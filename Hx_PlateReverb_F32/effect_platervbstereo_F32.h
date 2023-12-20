/*  Stereo plate reverb for Teensy 4
 *  32bit float version for OpenAudio_ArduinoLibrary:
 *  https://github.com/chipaudette/OpenAudio_ArduinoLibrary
 *  Author: Piotr Zapart
 *          www.hexefx.com
 *
 * Copyright (c) 2023 by Piotr Zapart
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/***
 * Algorithm based on plate reverbs developed for SpinSemi FV-1 DSP chip
 * 
 * Allpass + modulated delay line based lush plate reverb
 * 
 * Input parameters are float in range 0.0 to 1.0:
 * 
 * size - reverb time
 * hidamp - hi frequency loss in the reverb tail
 * lodamp - low frequency loss in the reverb tail
 * lowpass - output/master lowpass filter, useful for darkening the reverb sound 
 * diffusion - lower settings will make the reverb tail more "echoey".
 * freeze - infinite reverb tail effect
 * 
 */
#ifndef _EFFECT_PLATERVBSTEREO_F32_H
#define _EFFECT_PLATERVBSTEREO_F32_H

#include <Arduino.h>
#include "Audio.h"
#include "AudioStream_F32.h"
#include "arm_math.h"

// if uncommented will place all the buffers in the DMAMEM section ofd the memory
// works with single instance of the reverb only
#define REVERB_F32_USE_DMAMEM

/***
 * Loop delay modulation: comment/uncomment to switch sin/cos 
 * modulation for the 1st or 2nd tap, 3rd tap is always modulated
 * more modulation means more chorus type sounding reverb tail
 */
//#define TAP1_MODULATED
#define TAP2_MODULATED

class AudioEffectPlateReverb_F32 : public AudioStream_F32
{
public:
    AudioEffectPlateReverb_F32();
    virtual void update();

    /**
     * @brief reverb time. Please not the hidamp/lodamp params also control how
     *  fast the reverb tail dacays.
     * 
     * @param n time, range 0.0f to 1.0f
     */
    void size(float n)
    {
        n = constrain(n, 0.0f, 1.0f);
        n = map (n, 0.0f, 1.0f, 0.2f, rv_time_k_max);
        float32_t attn = map(n, 0.0f, rv_time_k_max, 0.5f, 0.25f);
        __disable_irq();
        rv_time_k = n;
        input_attn = attn;
        __enable_irq();
    }
    /**
     * @brief amount treble loss in the reverb tail
     * 
     * @param n range 0.0f to 1.0f
     */
    void hidamp(float n)
    {
        n = constrain(n, 0.0f, 1.0f);
        __disable_irq();
        lp_hidamp_k = 1.0f - n;
        __enable_irq();
    }
    /**
     * @brief amount og bass lost in the reverb tail
     * 
     * @param n range 0.0f to 1.0f
     */
    void lodamp(float n)
    {
        n = constrain(n, 0.0f, 1.0f);
        __disable_irq();
        lp_lodamp_k = -n;
        rv_time_scaler = 1.0f - n * 0.12f;        // limit the max reverb time, otherwise it will clip
        __enable_irq();
    }
    /**
     * @brief lowpass filter applied to the reverb output 
     *  use it to darken the reverb tail
     * 
     * @param n range 0.0f to 1.0f
     */
    void lowpass(float n)
    {
        n = constrain(n, 0.0f, 1.0f);
        n = map(n*n*n, 0.0f, 1.0f, 1.0f, 0.05f);
        master_lowpass_f = n;
    }
    /**
     * @brief "echoiness" of the reverb sound. Lower values produce more 
     *  echo like reflections. 1.0 produces a lush plate reverb.
     *  Use this parameter together with the "size" to produce different type
     *  of reverb sounds. Ie, small size (0) + low diffusion create a room type
     *  reverb sound.
     * 
     * @param n 
     */
    void diffusion(float n)
    {
        n = constrain(n, 0.0f, 1.0f);
        n = map(n, 0.0f, 1.0f, 0.005f, 0.65f);
        __disable_irq();
        in_allp_k = n;
        loop_allp_k = n;
         __enable_irq();
    }
    /**
     * @brief Freezes the reverb tank by cutting off the input signal
     *  and increasing the reverb time coeff to 1.0 giving an infinite
     *  tail.
     * 
     * @param state freeze on (true) or off (false)
     */
    void freeze(bool state)
    {
        flags.freeze = state;
        if (state)
        {
            rv_time_k_tmp = rv_time_k;      // store the settings
            lp_lodamp_k_tmp = lp_lodamp_k;
            lp_hidamp_k_tmp = lp_hidamp_k;
            
            __disable_irq();
            rv_time_k = freeze_rvtime_k;                                      
            input_attn = freeze_ingain;
            rv_time_scaler = 1.0f;
            lp_lodamp_k = freeze_lodamp_k;
            lp_hidamp_k = freeze_hidamp_k;

            __enable_irq();
        }
        else
        {
            float32_t attn = map(rv_time_k_tmp, 0.0f, rv_time_k_max, 0.5f, 0.25f);    // recalc the in attenuation
            float32_t sc = 1.0f + lp_lodamp_k_tmp * 0.12f;
            __disable_irq();
            rv_time_k = rv_time_k_tmp;                                      // restore the value
            input_attn = attn;
            rv_time_scaler = sc;
            lp_hidamp_k = lp_hidamp_k_tmp;
            lp_lodamp_k = lp_lodamp_k_tmp;
            __enable_irq();
        }
    }
    /**
     * @brief Toggles the freeze function and returns the current state
     * 
     * @return true     toggle resulted in freeze on
     * @return false    toggle resulted in freeze off
     */
    bool freeze_tgl() {flags.freeze ^= 1; freeze(flags.freeze); return flags.freeze;}
    
    bool freeze_get() {return flags.freeze;}

    float32_t size_get(void) {return rv_time_k;}

    bool bypass_get(void) {return flags.bypass;}
    void bypass_set(bool state) 
    {
        flags.bypass = state;
        if (state) freeze(false);       // disable freeze in bypass mode
    }
    bool bypass_tgl(void) 
    {
        flags.bypass ^= 1; 
        if (flags.bypass) freeze(false);       // disable freeze in bypass mode
        return flags.bypass;
    }

private:
    struct flags_t
    {
        unsigned bypass:           1;
        unsigned freeze:            1;
        unsigned shimmer:           1; // maybe will be added at some point
        unsigned cleanup_done:      1;
    }flags;

    audio_block_f32_t *inputQueueArray_f32[2];
    float32_t input_attn;           
    float32_t in_allp_k;            // input allpass coeff 
#ifndef REVERB_F32_USE_DMAMEM
    float32_t in_allp1_bufL[224];   // input allpass buffers
    float32_t in_allp2_bufL[420];
    float32_t in_allp3_bufL[856];
    float32_t in_allp4_bufL[1089];
#endif
    uint16_t in_allp1_idxL;
    uint16_t in_allp2_idxL;
    uint16_t in_allp3_idxL;
    uint16_t in_allp4_idxL;
    float32_t in_allp_out_L;    // L allpass chain output
#ifndef REVERB_F32_USE_DMAMEM
    float32_t in_allp1_bufR[156]; // input allpass buffers
    float32_t in_allp2_bufR[520];
    float32_t in_allp3_bufR[956];
    float32_t in_allp4_bufR[1289];
#endif
    uint16_t in_allp1_idxR;
    uint16_t in_allp2_idxR;
    uint16_t in_allp3_idxR;
    uint16_t in_allp4_idxR;
    float32_t in_allp_out_R;    // R allpass chain output
#ifndef REVERB_F32_USE_DMAMEM
    float32_t lp_allp1_buf[2303]; // loop allpass buffers
    float32_t lp_allp2_buf[2905];
    float32_t lp_allp3_buf[3175];
    float32_t lp_allp4_buf[2398];
#endif
    uint16_t lp_allp1_idx;
    uint16_t lp_allp2_idx;
    uint16_t lp_allp3_idx;
    uint16_t lp_allp4_idx;
    float32_t loop_allp_k;         // loop allpass coeff
    float32_t lp_allp_out;
#ifndef REVERB_F32_USE_DMAMEM
    float32_t lp_dly1_buf[3423];
    float32_t lp_dly2_buf[4589];
    float32_t lp_dly3_buf[4365];
    float32_t lp_dly4_buf[3698];
#endif
    uint16_t lp_dly1_idx;
    uint16_t lp_dly2_idx;
    uint16_t lp_dly3_idx;
    uint16_t lp_dly4_idx;

    const uint16_t lp_dly1_offset_L = 201;      // delay line tap offets
    const uint16_t lp_dly2_offset_L = 145;
    const uint16_t lp_dly3_offset_L = 1897;
    const uint16_t lp_dly4_offset_L = 280;

    const uint16_t lp_dly1_offset_R = 1897;
    const uint16_t lp_dly2_offset_R = 1245;
    const uint16_t lp_dly3_offset_R = 487;
    const uint16_t lp_dly4_offset_R = 780;  

    float32_t lp_hidamp_k;       // loop high band damping coeff
    float32_t lp_hidamp_k_tmp;  
    float32_t lp_lodamp_k;       // loop low baand damping coeff
    float32_t lp_lodamp_k_tmp;

    float32_t lpf1;             // lowpass filters
    float32_t lpf2;
    float32_t lpf3;
    float32_t lpf4;

    float32_t hpf1;             // highpass filters
    float32_t hpf2;
    float32_t hpf3;
    float32_t hpf4;

    float32_t lp_lowpass_f;      // loop lowpass scaled frequency
    float32_t lp_hipass_f;       // loop highpass scaled frequency 

    float32_t master_lowpass_f;
    float32_t master_lowpass_l;
    float32_t master_lowpass_r;

    const float32_t rv_time_k_max = 0.95f;
    float32_t rv_time_k;         // reverb time coeff
    float32_t rv_time_k_tmp;     // temp for restoring original value after freeze_off
    float32_t rv_time_scaler;    // with high lodamp settings lower the max reverb time to avoid clipping

    uint32_t lfo1_phase_acc;     // LFO 1
    uint32_t lfo1_adder;

    uint32_t lfo2_phase_acc;    // LFO 2
    uint32_t lfo2_adder;

    const float32_t freeze_rvtime_k = 1.0f;
    const float32_t freeze_ingain = 0.0f;
    const float32_t freeze_lodamp_k = 0.0f;
    const float32_t freeze_hidamp_k = 1.0f;
};

#endif // _EFFECT_PLATEREV_H

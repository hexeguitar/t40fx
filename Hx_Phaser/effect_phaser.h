/*  Mono Phaser/Vibrato effect for Teensy Audio library
 *
 *  Author: Piotr Zapart
 *          www.hexefx.com
 *
 * Copyright (c) 2021 by Piotr Zapart
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

#ifndef _EFFECT_PHASER_H
#define _EFFECT_PHASER_H

#include <Arduino.h>
#include "Audio.h"
#include "AudioStream.h"
#include "arm_math.h"

#define PHASER_STEREO_STAGES	12

class AudioEffectPhaser : public AudioStream
{
    public:
    AudioEffectPhaser();
    ~AudioEffectPhaser();
    virtual void update();

    /**
     * @brief Scale and offset the modulation signal. It can be the internal LFO
     *          or the incomig routed modulation AudioSignal.
     *          LFO will oscillate between these two max and min values. 
     * 
     * @param top       top level of the LFO
     * @param bottom     bottom level of the LFO
     */
    void depth(float32_t top, float32_t bottom)
    {
        float32_t a, b;
        a = constrain(top, 0.0f, 1.0f);
        b = constrain(bottom, 0.0f, 1.0f);
        __disable_irq();
        lfo_top = a;
        lfo_btm = b;
        __enable_irq();
    }
    /**
     * @brief Controls the internal LFO, or if a control signal is used, scales it
     *          Use this function to update all lfo parameteres at once
     * 
     * @param f_Hz  lfo frequency, use 0.0f for manual phaser control
     * @param top   lfo top level 
     * @param btm   lfo bottm level
     */
    void lfo(float32_t f_Hz, float32_t top, float32_t btm)
    {
        float32_t a, b, c;
        uint32_t add;
        a = constrain(top, 0.0f, 1.0f);
        b = constrain(btm, 0.0f, 1.0f);
        c = constrain(f_Hz, 0.0f, AUDIO_SAMPLE_RATE_EXACT/2);
        add = c * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
        __disable_irq();
        lfo_top = a;
        lfo_btm = b;
        lfo_add = add;
        __enable_irq();
    }
    /**
     * @brief Set the rate of the internal LFO
     * 
     * @param f_Hz lfo frequency, use 0.0f for manual phaser control
     */
    void lfo_rate(float32_t f_Hz)
    {
        float32_t c;
        uint32_t add;
        c = constrain(f_Hz, 0.0f, AUDIO_SAMPLE_RATE_EXACT/2);
        add = c * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
        __disable_irq();
        lfo_add = add;
        __enable_irq();
    }
    /**
     * @brief Controls the feedback parameter
     * 
     * @param fdb ffedback value in range 0.0f to 1.0f
     */
    void feedback(float32_t fdb)
    {
        feedb = constrain(fdb, 0.0f, 1.0f);
    }
    /**
     * @brief Dry / Wet mixer ratio. Classic Phaser sound uses 0.5f for 50% dry and 50%Wet
     *        1.0f will produce 100% wet signal craeting a vibrato effect
     * 
     * @param ratio mixing ratio, range 0.0f (full dry) to 1.0f (full wet)
     */
    void mix(float32_t ratio)
    {
        mix_ratio = constrain(ratio, 0.0f, 1.0f);
    }
    /**
     * @brief Sets the number of stages used in the phaser
     *        Allowed values are: 2, 4, 6, 8, 10, 12
     * 
     * @param st number of stages, even value <= 12
     */
    void stages(uint8_t st)
    {
        if (st && st == ((st >> 1) << 1) && st <= PHASER_STEREO_STAGES) // only 2, 4, 6, 8, 12 allowed
        {
            stg = st;
        }
    }
    /**
     * @brief Use to bypass the effect (true)
     * 
     * @param state true = bypass on, false = phaser on
     */
    void bypass(bool state) {bps = state;}
    void tgl_bypass(void) {bps ^= 1;}

private:
    uint8_t stg;                                    // number of stages
    bool bps;                                       // bypass
    audio_block_t *inputQueueArray[2];      
    float32_t allpass_x[PHASER_STEREO_STAGES];      // allpass inputs
	float32_t allpass_y[PHASER_STEREO_STAGES];      // allpass outputs
	float32_t mix_ratio;                            // 0 = dry. 1.0 = wet
    float32_t feedb;                                // feedback 
    float32_t last_sample;
    uint32_t lfo_phase_acc;                         // interfnal lfo 
    uint32_t lfo_add;
    float32_t lfo_top;
    float32_t lfo_btm;
};

#endif // _EFFECT_PHASER_H

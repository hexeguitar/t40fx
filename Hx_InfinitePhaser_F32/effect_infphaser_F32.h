/*  Mono Shephard/Barberpole Phaser/Vibrato effect for Teensy Audio library
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

#ifndef _EFFECT_INFPHASER_F32_H
#define _EFFECT_INFPHASER_F32_H

#include <Arduino.h>
#include "Audio.h"
#include "AudioStream_F32.h"
#include "arm_math.h"

// ################ SHEPARD/BARBERPOLE INFINITE PHASER ################
#define INFINITE_PHASER_STAGES	6
#define INFINITE_PHASER_PATHS   6   // 6 parallel paths

#define INFINITE_PHASER_MAX_LFO_HZ  (0.25f) // maximum LFO rate range

class AudioEffectInfinitePhaser_F32 : public AudioStream_F32
{
public:
    AudioEffectInfinitePhaser_F32();
    ~AudioEffectInfinitePhaser_F32();
    virtual void update();
/**
     * @brief Scale and offset the modulation signal. 
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
    void depth_top(float32_t top)
    {
        float32_t a = constrain(top, 0.0f, 1.0f);
        __disable_irq();
        lfo_top = a;
        __enable_irq();
    }
    void depth_btm(float32_t btm)
    {
        float32_t a = constrain(btm, 0.0f, 1.0f);
        __disable_irq();
        lfo_btm = a;
        __enable_irq();
    }
    /**
     * @brief Controls the internal LFO.
     *          Use this function to update all lfo parameteres at once
     * 
     * @param rate  scaled lfo frequency -1.0f to 1.0f, use 0.0f for manual phaser control
     * @param top   lfo top level, range 0.0f to 1.0f
     * @param btm   lfo bottm level, range 0.0f to 1.0f
     */
    void lfo(float32_t rate, float32_t top, float32_t btm)
    {
        int32_t add;
        if (rate < 0.0f) rate = rate*rate*(-1.0f);
        else rate = rate*rate;
        top = constrain(top, 0.0f, 1.0f);
        btm = constrain(btm, 0.0f, 1.0f);
        add = rate * (4294967296.0 / AUDIO_SAMPLE_RATE_EXACT);
        __disable_irq();
        lfo_top = top;
        lfo_btm = btm;
        lfo_add = add;
        __enable_irq();
    }
    /**
     * @brief Set the rate of the internal LFO
     * 
     * @param rate lfo frequency, use 0.0f for manual phaser control
     *      Range -1.0f to 1.0f for reverse and forward modulation
     */
    void lfo_rate(float32_t rate)
    {
        if (rate < 0.0f) rate = rate*rate*(-1.0f);
        else rate = rate*rate;
        int32_t add;
        rate = map(rate, -1.0f, 1.0f, -INFINITE_PHASER_MAX_LFO_HZ, INFINITE_PHASER_MAX_LFO_HZ);
        add = rate * (4294967296.0f / AUDIO_SAMPLE_RATE_EXACT);
        __disable_irq();
        lfo_add = add;
        __enable_irq();
    }
    /**
     * @brief Controls the feedback parameter
     * 
     * @param fdb feedback value in range 0.0f to 1.0f
     */
    void feedback(float32_t fdb)
    {
        fdb = map(fdb, 0.0f, 1.0f, 0.5f, 0.999f);
        __disable_irq();
        feedb = fdb;
        __enable_irq();
    }
    /**
     * @brief Dry / Wet mixer ratio. Classic Phaser sound uses 0.5f for 50% dry and 50%Wet
     *        1.0f will produce 100% wet signal craeting a vibrato effect
     * 
     * @param ratio mixing ratio, range 0.0f (full dry) to 1.0f (full wet)
     */
    void mix(float32_t ratio)
    {
        ratio = constrain(ratio, 0.0f, 1.0f);
        __disable_irq();
        mix_ratio = ratio;
        __enable_irq();
    }
    /**
     * @brief Sets the number of stages used in the phaser
     *        Allowed values are: 2, 4, 6
     * 
     * @param st number of stages, even value <= 6
     */
    void stages(uint8_t st)
    {
        if (st && st == ((st >> 1) << 1) && st <= INFINITE_PHASER_STAGES) // only 2, 4, 6, 8, 12 allowed
        {
            __disable_irq();
            stg = st;
            __enable_irq();
        }
    }
    /**
     * @brief Use to bypass the effect (true)
     * 
     * @param state true = bypass on, false = phaser on
     */
    void set_bypass(bool state) {bps = state;}
    bool get_bypass(void) {return bps;}
    bool tgl_bypass(void) {bps ^= 1; return bps;}
private:
    uint8_t stg;                             // number of stages
    bool bps;                                // bypass
    audio_block_f32_t *inputQueueArray_f32[1];      
    float32_t allpass_x[INFINITE_PHASER_PATHS][INFINITE_PHASER_STAGES];      // allpass inputs
	float32_t allpass_y[INFINITE_PHASER_PATHS][INFINITE_PHASER_STAGES];      // allpass outputs
	float32_t mix_ratio;                     // 0 = dry. 1.0 = wet
    float32_t feedb;                         // feedback 
    float32_t last_sample[INFINITE_PHASER_PATHS];
    uint32_t lfo_phase_acc;                  // interfnal lfo 
    int32_t lfo_add;
    float32_t lfo_top;
    float32_t lfo_btm;
};

#endif // _EFFECT_INFPHASER_H

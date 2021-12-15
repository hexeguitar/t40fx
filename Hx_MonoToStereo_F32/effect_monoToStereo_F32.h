/*  mono to stereo expander for Teensy 4
 * 32bit float version for OpenAudio_ArduinoLibrary:
 * https://github.com/chipaudette/OpenAudio_ArduinoLibrary
 *
 *  Author: Piotr Zapart
 *          www.hexefx.com
 *
 * Copyright (c) 2021 by Piotr Zapart
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _EFFECT_MONOTOSTEREO_F32_H
#define _EFFECT_MONOTOSTEREO_F32_H

#include <Arduino.h>
#include "AudioStream_F32.h"
#include "arm_math.h"

#define ALLP_NETWORK_LEN    21

class AudioEffectMonoToStereo_F32 : public AudioStream_F32
{
public:
    AudioEffectMonoToStereo_F32();
    ~AudioEffectMonoToStereo_F32();
    virtual void update();
    void setSpread(float32_t val)
    { 
        val = constrain(val, 0.0f, 1.0f);
        __disable_irq();
        width = val;
        __enable_irq();
    }
    void setPan(float32_t val)
    {
        float32_t a, b;
        val = constrain(val, -1.0f, 1.0f);
        a = map(val, -1.0f, 1.0f, -0.707f, 0.707f);
        b = 1.0f - abs(val*0.293f);
        __disable_irq();
        pansin = a;
        pancos = b;
        __enable_irq();
    }
    void setBypass(bool state) {bypass = state;}
    void tglBypass(void) {bypass ^= 1;}
    bool getBypass(void) { return bypass;}
private:
    bool bypass;
    float32_t width;
    float32_t pancos, pansin;
    float32_t do_allp_netw(float32_t inSig, float32_t *x, float32_t *y);
    float32_t allpass_netw_1x[ALLP_NETWORK_LEN];
    float32_t allpass_netw_1y[ALLP_NETWORK_LEN];
    float32_t allpass_netw_2x[ALLP_NETWORK_LEN];
    float32_t allpass_netw_2y[ALLP_NETWORK_LEN];

    audio_block_f32_t *inputQueueArray_f32[1];   
};

#endif // _EFFECT_MONOTOSTEREO_F32_H

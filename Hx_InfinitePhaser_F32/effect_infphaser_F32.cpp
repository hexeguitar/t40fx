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
#include "effect_infphaser_F32.h"

// ---------------------------- INFINITE PHASER MODULATION -----------------------
#define INF_PHASER_STEP     (0x100000000u / INFINITE_PHASER_PATHS)

#define USE_TABLE


AudioEffectInfinitePhaser_F32::AudioEffectInfinitePhaser_F32() : AudioStream_F32(1, inputQueueArray_f32)
{
	memset(allpass_x, 0, INFINITE_PHASER_STAGES * INFINITE_PHASER_PATHS * sizeof(float32_t));
	memset(allpass_y, 0, INFINITE_PHASER_STAGES * INFINITE_PHASER_PATHS * sizeof(float32_t));
    bps = false;
    lfo_top = 1.0f;
    lfo_btm = 0.0f;
    lfo_phase_acc = 0;
    lfo_add = 0;
    feedb = 0.5f;              // effect is hard noticable with low feedback settings, hence the range is limited to 0.5-0.999
    mix_ratio = 0.5f;         // start with classic phaser sound 
    stg = INFINITE_PHASER_STAGES;
}
AudioEffectInfinitePhaser_F32::~AudioEffectInfinitePhaser_F32()
{
}

void AudioEffectInfinitePhaser_F32::update()
{

#if defined(__ARM_ARCH_7EM__)
    audio_block_f32_t *blockIn; 
    uint16_t i = 0;
    float32_t modSig;
    uint32_t phaseAcc = lfo_phase_acc;
    int32_t phaseAdd = lfo_add;
    float32_t top = lfo_top;
    float32_t btm = lfo_btm;
    uint32_t phase_acc_local;
    uint32_t y0, y1;
    float32_t inSig, drySig, wetSig;
    float32_t fdb = feedb;
    float32_t ampl;

    blockIn = AudioStream_F32::receiveWritable_f32(0);       // audio data
    
    if (!blockIn)
    {
        return;
    }
    if (bps)
    {
        AudioStream_F32::transmit(blockIn);
        AudioStream_F32::release(blockIn);
        return;
    }

	for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) 
    {
        wetSig = 0.0f;
        drySig = blockIn->data[i] * (1.0f - fdb*0.25f);  // attenuate the input if using feedback
        
        y1 = INFINITE_PHASER_PATHS;
        while (y1)
        {
            y1--;
            phase_acc_local = phaseAcc + y1*INF_PHASER_STEP;
            modSig =  1.0f - ((float32_t)phase_acc_local / 4294967295.0f);
            ampl = modSig * 2.0f; 
            
            if (ampl > 1.0f)  ampl = -2.0f * modSig + 2.0f;
            modSig = modSig*modSig * abs(top - btm) + min(top, btm);

            inSig = drySig + last_sample[y1] * fdb;
            y0 = stg;
            while (y0)  // process allpass filters in pairs
            {
                y0--;
                allpass_y[y1][y0] = modSig * (allpass_y[y1][y0] + inSig) - allpass_x[y1][y0];
                allpass_x[y1][y0] = inSig;
                y0--;
                allpass_y[y1][y0] = modSig * (allpass_y[y1][y0] + allpass_y[y1][y0+1]) - allpass_x[y1][y0];
                allpass_x[y1][y0] = allpass_y[y1][y0+1];
                inSig = allpass_y[y1][y0];
            }
            last_sample[y1] = inSig;
            wetSig += ((drySig * (1.0f - mix_ratio) + inSig * mix_ratio)* ampl)/2.0f;
        }
        blockIn->data[i] = wetSig;

        phaseAcc += phaseAdd;   
    }
    lfo_phase_acc = phaseAcc;
    AudioStream_F32::transmit(blockIn);
	AudioStream_F32::release(blockIn);
#endif
}


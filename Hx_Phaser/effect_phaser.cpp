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

#include <Arduino.h>
#include "effect_phaser.h"
#include "utility/dspinst.h"

// ---------------------------- INTERNAL LFO -------------------------------------
#define LFO_LUT_BITS					8
#define LFO_MAX_F						(AUDIO_SAMPLE_RATE_EXACT / 2.0f)		
#define LFO_INTERP_INT_SHIFT			(32-LFO_LUT_BITS)
#define LFO_INTERP_FRACT_MASK			((1<<LFO_INTERP_INT_SHIFT)-1)

// parabollic/hypertriangular waveform used for the internal LFO
const uint16_t AudioWaveformHyperTri[257] = 
{
     0,    804,   1608,   2412,   3216,   4019,   4821,   5623,   6424,   7223, 
  8022,   8820,   9616,  10411,  11204,  11996,  12785,  13573,  14359,  15142, 
 15924,  16703,  17479,  18253,  19024,  19792,  20557,  21319,  22078,  22834, 
 23586,  24334,  25079,  25820,  26557,  27291,  28020,  28745,  29465,  30181, 
 30893,  31600,  32302,  32999,  33692,  34379,  35061,  35738,  36409,  37075, 
 37736,  38390,  39039,  39682,  40319,  40950,  41575,  42194,  42806,  43411, 
 44011,  44603,  45189,  45768,  46340,  46905,  47464,  48014,  48558,  49095, 
 49624,  50145,  50659,  51166,  51664,  52155,  52638,  53113,  53580,  54039, 
 54490,  54933,  55367,  55794,  56211,  56620,  57021,  57413,  57797,  58171, 
 58537,  58895,  59243,  59582,  59913,  60234,  60546,  60850,  61144,  61429, 
 61704,  61970,  62227,  62475,  62713,  62942,  63161,  63371,  63571,  63762, 
 63943,  64114,  64276,  64428,  64570,  64703,  64826,  64939,  65042,  65136, 
 65219,  65293,  65357,  65412,  65456,  65491,  65515,  65530,  65535,  65530, 
 65515,  65491,  65456,  65412,  65357,  65293,  65219,  65136,  65042,  64939, 
 64826,  64703,  64570,  64428,  64276,  64114,  63943,  63762,  63571,  63371, 
 63161,  62942,  62713,  62475,  62227,  61970,  61704,  61429,  61144,  60850, 
 60546,  60234,  59913,  59582,  59243,  58895,  58537,  58171,  57797,  57413, 
 57021,  56620,  56211,  55794,  55367,  54933,  54490,  54039,  53580,  53113, 
 52638,  52155,  51664,  51166,  50659,  50145,  49624,  49095,  48558,  48014, 
 47464,  46905,  46340,  45768,  45189,  44603,  44011,  43411,  42806,  42194, 
 41575,  40950,  40319,  39682,  39039,  38390,  37736,  37075,  36409,  35738, 
 35061,  34379,  33692,  32999,  32302,  31600,  30893,  30181,  29465,  28745, 
 28020,  27291,  26557,  25820,  25079,  24334,  23586,  22834,  22078,  21319, 
 20557,  19792,  19024,  18253,  17479,  16703,  15924,  15142,  14359,  13573, 
 12785,  11996,  11204,  10411,   9616,   8820,   8022,   7223,   6424,   5623, 
  4821,   4019,   3216,   2412,   1608,    804,      0
 };
// ---------------------------- /INTERNAL LFO ------------------------------------

static float32_t bf_f32[AUDIO_BLOCK_SAMPLES];

AudioEffectPhaser::AudioEffectPhaser() : AudioStream(2, inputQueueArray)
{
	memset(allpass_x, 0, PHASER_STEREO_STAGES * sizeof(float32_t));
	memset(allpass_y, 0, PHASER_STEREO_STAGES * sizeof(float32_t));
    bps = false;
    lfo_phase_acc = 0;
    lfo_add = 0;
    feedb = 0.0f;
    mix_ratio = 0.5f;         // start with classic phaser sound 
    stg = PHASER_STEREO_STAGES;
}
AudioEffectPhaser::~AudioEffectPhaser()
{
}


void AudioEffectPhaser::update()
{

#if defined(__ARM_ARCH_7EM__)
    audio_block_t *blockIn; 
    const audio_block_t *blockMod;    // inputs
    bool internalLFO = false;                    // use internal LFO of no modulation input
    uint16_t i = 0;
    float32_t modSig;
    uint32_t phaseAcc = lfo_phase_acc;
    uint32_t phaseAdd = lfo_add;
    float32_t top = lfo_top;
    float32_t btm = lfo_btm;
    uint32_t y0, y1, fract;
    uint64_t y;
    float32_t inSig, drySig;
    float32_t fdb = feedb;

    blockIn = receiveWritable(0);       // audio data
    blockMod = receiveReadOnly(1);      // bipolar/int16_t control input
    
    if (!blockIn)
    {
        if (blockMod) release((audio_block_t *)blockMod);
        return;
    }
    if (!blockMod)  internalLFO = true;         // no modulation input provided -> use internal LFO
    if (bps)
    {
        transmit((audio_block_t *)blockIn);
        release((audio_block_t *)blockIn);
        if (blockMod) release((audio_block_t *)blockMod);
        return;
    }
    arm_q15_to_float((q15_t *)blockIn->data, bf_f32, AUDIO_BLOCK_SAMPLES);   

	for (i=0; i < AUDIO_BLOCK_SAMPLES; i++) 
    {
        if(internalLFO)
        {
            uint32_t LUTaddr = phaseAcc >> LFO_INTERP_INT_SHIFT;	//8 bit address
            fract = phaseAcc & LFO_INTERP_FRACT_MASK;				// fractional part mask
            y0 = AudioWaveformHyperTri[LUTaddr];
            y1 = AudioWaveformHyperTri[LUTaddr+1];
            y = ((int64_t) y0 * (LFO_INTERP_FRACT_MASK - fract));
            y += ((int64_t) y1 * (fract));
            modSig = (float32_t)(y>>LFO_INTERP_INT_SHIFT) / 65535.0f;
            phaseAcc += phaseAdd;
        }
        else
        {
            modSig = ((float32_t)blockMod->data[i] + 32768.0f) / 65535.0f;    // mod signal is 0.0 to 1.0
        }
        // apply scale/offset to the modulation wave
        modSig = modSig * abs(top - btm) + min(top, btm);
        drySig = bf_f32[i] * (1.0f - fdb*0.25f);  // attenuate the input is using feedback
        inSig = drySig + last_sample * fdb;
        y0 = stg;
        while (y0)  // process allpass filters in pairs
        {
            y0--;
		    allpass_y[y0] = modSig * (allpass_y[y0] + inSig) - allpass_x[y0];
		    allpass_x[y0] = inSig;
            y0--;
		    allpass_y[y0] = modSig * (allpass_y[y0] + allpass_y[y0+1]) - allpass_x[y0];
		    allpass_x[y0] = allpass_y[y0+1];
            inSig = allpass_y[y0];
        }
        last_sample = inSig;
        drySig = drySig * (1.0f - mix_ratio) + last_sample * mix_ratio;     // dry/wet mixer
        blockIn->data[i] = (int16_t)(drySig * 32767.0f);   
    }
    lfo_phase_acc = phaseAcc;
    transmit(blockIn);
	release(blockIn);
    if (blockMod) release((audio_block_t *)blockMod);
#endif


}

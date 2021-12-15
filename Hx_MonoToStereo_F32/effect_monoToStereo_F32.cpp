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
#include "effect_monoToStereo_F32.h"

const float32_t allpass_k_table[ALLP_NETWORK_LEN] = 
{
    -0.9823311567306519f, -0.9838343858718872f, -0.9838343858718872f, 
    -0.9843953251838684f, -0.9843953251838684f, -0.9850407838821411f, 
    -0.9850407838821411f, -0.9856590032577515f, -0.9856590032577515f, 
    -0.9868955612182617f, -0.9868955612182617f, -0.9894036054611206f, 
    -0.9894036054611206f, -0.9902338981628418f, -0.9902338981628418f, 
    -0.9911531209945679f, -0.9911531209945679f, -0.9911531209945679f, 
    -0.9911531209945679f, -0.9928167462348938f, -0.9928167462348938f
};

AudioEffectMonoToStereo_F32::AudioEffectMonoToStereo_F32() : AudioStream_F32(1, inputQueueArray_f32)
{
    pancos = 1.0f;
    pansin= 0.0f;
    width = 0.0f;
    bypass = false;
}
AudioEffectMonoToStereo_F32::~AudioEffectMonoToStereo_F32()
{
}

void AudioEffectMonoToStereo_F32::update()
{
#if defined(__ARM_ARCH_7EM__)

    audio_block_f32_t *blockIn;
    uint16_t i;
    float32_t _width = width;
    float32_t _pancos = pancos;
    float32_t _pansin = pansin;
    float32_t allp1Out, allp2Out, stereoL, stereoR;

    blockIn = AudioStream_F32::receiveReadOnly_f32(0);
    if (!blockIn) return;

    audio_block_f32_t *blockOutL = AudioStream_F32::allocate_f32();
    audio_block_f32_t *blockOutR = AudioStream_F32::allocate_f32();
    if (!blockOutL || !blockOutR)
    {
        
        if (blockOutL)
            AudioStream_F32::release(blockOutL);
        if (blockOutR)
            AudioStream_F32::release(blockOutR);
        return;
    }
    if (bypass)
    {
        AudioStream_F32::transmit(blockIn, 0); // transmit input on both
        AudioStream_F32::transmit(blockIn, 1); // out channels
        AudioStream_F32::release(blockIn);
        AudioStream_F32::release(blockOutL);
        AudioStream_F32::release(blockOutR);
        return;
    }
    for (i = 0; i < blockIn->length; i++)
    {
        allp1Out = do_allp_netw(blockIn->data[i], allpass_netw_1x, allpass_netw_1y);
        allp2Out = do_allp_netw(allp1Out, allpass_netw_2x, allpass_netw_2y);
        stereoL = blockIn->data[i] * _width + allp1Out;
        stereoR = allp1Out - (allp2Out * _width);
        blockOutL->data[i] = (stereoL * _pancos) + (stereoR * _pansin);
        blockOutR->data[i] = (stereoR * _pancos) - (stereoL * _pansin);
    }
    AudioStream_F32::transmit(blockOutL, 0);
    AudioStream_F32::transmit(blockOutR, 1);
    AudioStream_F32::release(blockOutL);
    AudioStream_F32::release(blockOutR);
    AudioStream_F32::release(blockIn);

#endif
}


// y[n] = c*x[n] + x[n-1] - c*y[n-1]
// y[n] = c*(x[n] - y[n-1]) + x[n-1]
// c = (tan(pi*fc/fs)-1) / (tan(pi*fc/fs)+1)
float32_t AudioEffectMonoToStereo_F32::do_allp_netw(float32_t inSig, float32_t *x, float32_t *y)
{
    uint32_t stg = ALLP_NETWORK_LEN;
    while (stg)
    {
        stg--;
        y[stg] = allpass_k_table[stg] * (inSig - y[stg]) + x[stg];
        x[stg] = inSig;
        inSig = y[stg];
                stg--;
        y[stg] = allpass_k_table[stg] * (inSig - y[stg]) + x[stg];
        x[stg] = inSig;
        inSig = y[stg];
                stg--;
        y[stg] = allpass_k_table[stg] * (inSig - y[stg]) + x[stg];
        x[stg] = inSig;
        inSig = y[stg];
    }
    return y[0];
}
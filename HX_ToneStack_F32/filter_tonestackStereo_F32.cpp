/*
	ToneStack.h
	
	Copyright 2006-7
		David Yeh <dtyeh@ccrma.stanford.edu> 
	2006-14
		Tim Goetze <tim@quitte.de> (cosmetics)
	
	Tone Stack emulation for Teensy 4.x
	Ported for 32bit float version for OpenAudio_ArduinoLibrary:
	https://github.com/chipaudette/OpenAudio_ArduinoLibrary

	12.2023 Piotr Zapart www.hexefx.com

*/
/*
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 3
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
	02111-1307, USA or point your web browser to http://www.gnu.org.
*/
#include "filter_tonestackStereo_F32.h"

/**
 * @brief EQ models based on various guitar amplifiers
 */
AudioFilterToneStackStereo_F32::toneStackParams_t AudioFilterToneStackStereo_F32::presets[] = {
/* for convenience, */
#define k *1e3
#define M *1e6
#define nF *1e-9
#define pF *1e-12
	/* parameter order is R1 - R4, C1 - C3 */
	/* R1=treble R2=Bass R3=Mid, C1-3 related caps, R4 = parallel resistor */
	/* { 250000, 1000000, 25000, 56000, 0.25e-9, 20e-9, 20e-9 }, DY */
	{250 k, 1 M, 25 k, 56 k, 250 pF, 20 nF, 20 nF, "Bassman"},	   /* 59 Bassman 5F6-A */
	{250 k, 250 k, 4.8 k, 100 k, 250 pF, 100 nF, 47 nF, "Prince"}, /* 64 Princeton AA1164 */
	{250 k, 1 M, 25 k, 47 k, 600 pF, 20 nF, 20 nF, "Mesa"},		   /* Mesa Dual Rect. 'Orange' */
	/* Vox -- R3 is fixed (circuit differs anyway) */
	{1 M, 1 M, 10 k, 100 k, 50 pF, 22 nF, 22 nF, "Vox"}, /* Vox "top boost" */

	{220 k, 1 M, 22 k, 33 k, 470 pF, 22 nF, 22 nF, "JCM800"},	/* 59/81 JCM-800 Lead 100 2203 */
	{250 k, 250 k, 10 k, 100 k, 120 pF, 100 nF, 47 nF, "Twin"}, /* 69 Twin Reverb AA270 */

	{500 k, 1 M, 25 k, 47 k, 150 pF, 22 nF, 22 nF, "HK"},	   /* Hughes & Kettner Tube 20 */
	{250 k, 250 k, 10 k, 100 k, 150 pF, 82 nF, 47 nF, "Jazz"}, /* Roland Jazz Chorus */
	{250 k, 1 M, 50 k, 33 k, 100 pF, 22 nF, 22 nF, "Pignose"}, /* Pignose G40V */
#undef k
#undef M
#undef nF
#undef pF
};

AudioFilterToneStackStereo_F32 :: AudioFilterToneStackStereo_F32() : AudioStream_F32(2, inputQueueArray_f32)
{
	gain = 1.0f;
	setModel(TONESTACK_OFF);
}

void AudioFilterToneStackStereo_F32::setModel(toneStack_presets_e m)
{
	if (m >= TONE_STACK_MAX_MODELS) return;
	if (m == TONESTACK_OFF)
	{
		bp = true;
		filterL.reset();
		filterR.reset();
		return;
	}
	bp = false;
	currentModel = m - 1; 

	float32_t 	R1 = presets[currentModel].R1, \
				R2 = presets[currentModel].R2, \
				R3 = presets[currentModel].R3, \
				R4 = presets[currentModel].R4;
	float32_t 	C1 = presets[currentModel].C1, \
				C2 = presets[currentModel].C2, \
				C3 = presets[currentModel].C3;

	b1t = C1 * R1;
	b1m = C3 * R3;
	b1l = C1 * R2 + C2 * R2;
	b1d = C1 * R3 + C2 * R3;
	b2t = C1 * C2 * R1 * R4 + C1 * C3 * R1 * R4;
	b2m2 = -(C1 * C3 * R3 * R3 + C2 * C3 * R3 * R3);
	b2m = C1 * C3 * R1 * R3 + C1 * C3 * R3 * R3 + C2 * C3 * R3 * R3;
	b2l = C1 * C2 * R1 * R2 + C1 * C2 * R2 * R4 + C1 * C3 * R2 * R4;
	b2lm = C1 * C3 * R2 * R3 + C2 * C3 * R2 * R3;
	b2d = C1 * C2 * R1 * R3 + C1 * C2 * R3 * R4 + C1 * C3 * R3 * R4;
	b3lm = C1 * C2 * C3 * R1 * R2 * R3 + C1 * C2 * C3 * R2 * R3 * R4;
	b3m2 = -(C1 * C2 * C3 * R1 * R3 * R3 + C1 * C2 * C3 * R3 * R3 * R4);
	b3m = C1 * C2 * C3 * R1 * R3 * R3 + C1 * C2 * C3 * R3 * R3 * R4;
	b3t = C1 * C2 * C3 * R1 * R3 * R4;
	b3tm = -b3t;
	b3tl = C1 * C2 * C3 * R1 * R2 * R4;
	a0 = 1.0f;
	a1d = C1 * R1 + C1 * R3 + C2 * R3 + C2 * R4 + C3 * R4;
	a1m = C3 * R3;
	a1l = C1 * R2 + C2 * R2;
	a2m = C1 * C3 * R1 * R3 - C2 * C3 * R3 * R4 + C1 * C3 * R3 * R3 + C2 * C3 * R3 * R3;
	a2lm = C1 * C3 * R2 * R3 + C2 * C3 * R2 * R3;
	a2m2 = -(C1 * C3 * R3 * R3 + C2 * C3 * R3 * R3);
	a2l = C1 * C2 * R2 * R4 + C1 * C2 * R1 * R2 + C1 * C3 * R2 * R4 + C2 * C3 * R2 * R4;
	a2d = C1 * C2 * R1 * R4 + C1 * C3 * R1 * R4 + C1 * C2 * R3 * R4 + C1 * C2 * R1 * R3 + C1 * C3 * R3 * R4 + C2 * C3 * R3 * R4;
	a3lm = C1 * C2 * C3 * R1 * R2 * R3 + C1 * C2 * C3 * R2 * R3 * R4;
	a3m2 = -(C1 * C2 * C3 * R1 * R3 * R3 + C1 * C2 * C3 * R3 * R3 * R4);
	a3m = C1 * C2 * C3 * R3 * R3 * R4 + C1 * C2 * C3 * R1 * R3 * R3 - C1 * C2 * C3 * R1 * R3 * R4;
	a3l = C1 * C2 * C3 * R1 * R2 * R4;
	a3d = C1 * C2 * C3 * R1 * R3 * R4;

	filterL.reset();
	filterR.reset();
}

void AudioFilterToneStackStereo_F32::update()
{
#if defined(__ARM_ARCH_7EM__)
	audio_block_f32_t *blockL, *blockR; 
    blockL = AudioStream_F32::receiveWritable_f32(0);       // audio data
    blockR = AudioStream_F32::receiveWritable_f32(1);       // audio data
    if (!blockL || !blockR)
    {
		if (blockL) release((audio_block_f32_t *)blockL);
        if (blockR) release((audio_block_f32_t *)blockR);		
        return;
    }
	if (bp) // bypass mode
	{
        AudioStream_F32::transmit((audio_block_f32_t *)blockL,0);
        AudioStream_F32::transmit((audio_block_f32_t *)blockR,1);
        AudioStream_F32::release((audio_block_f32_t *)blockL);
        AudioStream_F32::release((audio_block_f32_t *)blockR);
		return;		
	}
	filterL.process(blockL->data, blockL->data, blockL->length);
	filterR.process(blockR->data, blockR->data, blockR->length);
	if (gain != 1.0f)
	{
		arm_scale_f32(blockL->data, gain, blockL->data, blockL->length);
		arm_scale_f32(blockR->data, gain, blockR->data, blockR->length);
	}
    AudioStream_F32::transmit(blockL, 0);
    AudioStream_F32::transmit(blockR, 1);
	AudioStream_F32::release(blockL);
    AudioStream_F32::release(blockR);	
#endif
}

/*
	TDFII.h
	
	Copyright 2006-7
		David Yeh <dtyeh@ccrma.stanford.edu> (implementation)
	2006-14
		Tim Goetze <tim@quitte.de> (cosmetics)

	transposed Direct Form II digital filter.
	Assumes order of b = order of a.
	Assumes a0 = 1.

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
#ifndef _FILTER_TDF2_H_
#define _FILTER_TDF2_H_

#include "arm_math.h"

template <int N>
class AudioFilterTDF2
{
public:
	float32_t a[N + 1];
	float32_t b[N + 1];
	float32_t h[N + 1];

	void reset()
	{
		for (int i = 0; i <= N; ++i)
			h[i] = 0; // zero state
	}

	void init()
	{
		reset();
		clear();
	}

	void clear()
	{
		for (int i = 0; i <= N; i++)
			a[i] = b[i] = 0;
		b[0] = 1;
	}

	void process(float32_t *src, float32_t *dst, uint32_t blockSize)
	{
		for (uint16_t i = 0; i<blockSize; i++)
		{
			float32_t in = *src++;
			float32_t y = h[0] + b[0] * in;

			for (uint16_t j = 1; j < N; ++j)
				h[j - 1] = h[j] + b[j] * in- a[j] * y;

			h[N - 1] = b[N] * in - a[N] * y;
			*dst++ = y;
		}
	}
};

#endif // _FILTER_TDF2_H_

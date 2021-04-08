/*
	SimpleSynthesizer V0.2
	Echo effect processor.
	Copyright (C) 2021 Feng Dai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include "echo.h"

FxEcho::FxEcho(bool _wetOnly)
{
	echoes[0].delay = 350;
	echoes[0].decay = 0.5;
	echoes[0].pan = 64;
	echoes[1].delay = 700;
	echoes[1].decay = 0.3;
	echoes[1].pan = 64;
	echoes[2].delay = 1050;
	echoes[2].decay = 0.15;
	echoes[2].pan = 64;

	wetOnly = _wetOnly;
}

FxEcho::~FxEcho()
{
	delete[] bufferLeft;
	delete[] bufferRight;
}

void FxEcho::Start(int depth)
{
	if (depth == 0)
	{
		delete[] bufferLeft;
		delete[] bufferRight;
		bufferLeft = bufferRight = nullptr;
		isEnabled = false;
		return;
	}
	else
		isEnabled = true;

	if (bufferLeft == nullptr)
	{
		bufferLeft = new double[EchoBufferSize];
		bufferRight = new double[EchoBufferSize];
		pos = 0;
		for (int i = 0; i < EchoBufferSize; i++)
		{
			bufferLeft[i] = bufferRight[i] = 0;
		}
		for (auto& item : echoes)
		{
			item.delayPulse = static_cast<int>(item.delay * SAMPLE_RATE / 1000.0);
		}
	}

	//For the time being.
	echoes[0].decay = depth / 127.0 * 0.5;
	echoes[1].decay = depth / 127.0 * 0.2;
	echoes[2].decay = depth / 127.0 * 0.1;
/*	echoes[0].delay = depth * 4;
	echoes[1].delay = depth * 8;
	echoes[2].delay = depth * 16;
*/
}

void FxEcho::TriggerPulse(const double inLeft, const double inRight, double& outLeft, double& outRight)
{
	if (!isEnabled)
	{
		if (wetOnly)
			outLeft = outRight = 0;
		else
		{
			outLeft = inLeft;
			outRight = inRight;
		}
		return;
	}

	bufferLeft[pos] = inLeft;
	bufferRight[pos] = inRight;

	double l = 0;
	double r = 0;

	for (auto& item : echoes)
	{
		size_t echoPos = ((int)EchoBufferSize + (int)pos - item.delayPulse) % EchoBufferSize;
		l += bufferLeft[echoPos] * item.decay;
		r += bufferRight[echoPos] * item.decay;
	}

	outLeft = l + (wetOnly ? 0 : inLeft);
	outRight = r + (wetOnly ? 0 : inRight);

	pos = (pos + 1) % EchoBufferSize;
}

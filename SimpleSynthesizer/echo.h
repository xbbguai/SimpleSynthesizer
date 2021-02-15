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

#pragma once

#include <iostream>
#include "Tone.h"

#define ECHO_TIMES 3
#define MAX_DELAY 1000	//maximum delay of the echo, in milliseconds

constexpr size_t EchoBufferSize = static_cast<size_t>(ECHO_TIMES * MAX_DELAY / 1000 * SAMPLE_RATE);

class FxEcho
{
	struct EchoParam
	{
		double delay;		//Delay of this echo. in milliseconds. < MAX_DELAY * ECHO_TIMES
		double decay;		//Deday of this echo. 0 - 1.0
		int pan;			//0 - 127

		int delayPulse;		// = delay  * SAMPLE_RATE / 1000.0
	};

	EchoParam echoes[ECHO_TIMES]{};

	double* bufferLeft{ nullptr };
	double* bufferRight{ nullptr };

	size_t pos;

	bool isEnabled{ false };
public:
	FxEcho();
	~FxEcho();

	void Start(int depth);

	void TriggerPulse(const double inLeft, const double inRight, double& outLeft, double& outRight);
};

/*
	SimpleSynthesizer V0.2
	Chorus effect processor.
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

#define MAX_CHORUS      7

class FxChorus
{
	struct ChorusParam
	{
		double delay;		//Delay of this chorus. 20 - 100ms
		double decay;		//Deday of this chorus. 0 - 1.0
		int pan;			//0 - 127

		int phase;			//Modulation position
		double modulationFrequency;		// modulation frequency. 0.1 - 5Hz
		double modulationDepth;	//0 = none, 1 = max
		int delayPulse;		// = delay  * SIGNAL_RATE / 1000.0
	};

	ChorusParam chorus[MAX_CHORUS]{};
	int numChorus{ 5 };

	double* bufferLeft{ nullptr };
	double* bufferRight{ nullptr };

	int bufferSize{ 0 };
	int pos{ 0 };

	bool isEnabled{ false };
	bool wetOnly{ false };
public:
	FxChorus(bool _wetOnly = false);
	~FxChorus();

	void Start(int depth);

	void TriggerPulse(const double inLeft, const double inRight, double& outLeft, double& outRight);
};

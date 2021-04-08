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
#include "chorus.h"

FxChorus::FxChorus(bool _wetOnly)
{
	wetOnly = _wetOnly;
	chorus[0] = { 35, 0.5, 0, 0, 10, 0.5, 0 };
	chorus[1] = { 25, 0.5, 32, 0, 20, 0.5, 0 };
	chorus[2] = { 45, 0.5, 64, 0, 10, 0.5, 0 };
	chorus[3] = { 65, 0.5, 96, 0, 20, 0.5, 0 };
	chorus[4] = { 50, 0.5, 127, 0, 10, 0.5, 0 };
}

FxChorus::~FxChorus()
{
	//It is safe to delete them at any time.
	delete[] bufferLeft;
	delete[] bufferRight;
}

void FxChorus::Start(int depth)
{
	int bufferSizeOld = bufferSize;

	//For the time being, chorus depth is mapped to decay param of echo chorus.
	for (int i = 0; i < numChorus; i++)
	{
		chorus[i].decay = static_cast<double>(depth) / 127;
	}

	for (int i = 0; i < numChorus; i++)
	{
		chorus[i].delayPulse = static_cast<int>(chorus[i].delay * SAMPLE_RATE / 1000);
		bufferSize = std::max(bufferSize, chorus[i].delayPulse);
	}

	if (bufferSize > bufferSizeOld)
	{
		delete[] bufferLeft;
		delete[] bufferRight;
	}
	bufferLeft = new double[bufferSize];
	bufferRight = new double[bufferSize];

	for (int i = 0; i < bufferSize; i++)
		bufferLeft[i] = bufferRight[i] = 0;

	isEnabled = (depth > 0);
}

void FxChorus::TriggerPulse(const double inLeft, const double inRight, double& outLeft, double& outRight)
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

	double oL = 0;
	double oR = 0;

	for (int i = 0; i < numChorus; i++)
	{
		int modulationCycle = static_cast<int>(SAMPLE_RATE / chorus[i].modulationFrequency);
		double modulationValue = static_cast<double>(chorus[i].phase) / modulationCycle;
		double modulationGain = (modulationValue > 0.5 ? 1 - modulationValue : modulationValue) * chorus[i].modulationDepth + 1 - chorus[i].modulationDepth;

		int chorusStartFrom = (bufferSize + pos - chorus[i].delayPulse) % bufferSize;
		oL += bufferLeft[chorusStartFrom] * chorus[i].decay * (127 - static_cast<double>(chorus[i].pan)) / 127 * modulationGain;
		oR += bufferRight[chorusStartFrom] * chorus[i].decay * static_cast<double>(chorus[i].pan) / 127 * modulationGain;

		chorus[i].phase = (chorus[i].phase + 1) % modulationCycle;
	}

	pos = (pos + 1) % bufferSize;

	outLeft = oL + (wetOnly ? 0 : inLeft);
	outRight = oR + (wetOnly ? 0 : inRight);
}
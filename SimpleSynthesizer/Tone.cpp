/*
	SimpleSynthesizer V0.2
	Tone generators for piano, squarewave(GM80) and trianglewave(GM81).
	The piano's harmonic wave params are taken from https://github.com/yuriecyx/merrychristmas
	Piano tone generator is not used for playing MIDI files instead a piano waveform is used.
	
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
#include <math.h>
#include <vector>
#include <iostream>
#include "Tone.h"
#include "WaveformTone.h"


//Static
Tone* Tone::CreateTone(int bank, int GMInstrument, const double _pitch, const uint8_t _velocity /*= 127*/)
{
//	return new WaveformTone(_track, _channel, bank, 35, _pitch, _velocity);
	if (bank < 512)
	{
		WaveformTone::MapGMInstrument(GMInstrument);
		if (GMInstrument == 80)
			return new GM080_Square(bank, 80, _pitch, _velocity);
		else if (GMInstrument == 81)
			return new GM081_Triangle(bank, 81, _pitch, _velocity);
		else
			return new WaveformTone(bank, GMInstrument, _pitch, _velocity);
	}
	else
	{
		Tone* pTone = new WaveformTone(bank, 0, _pitch, _velocity);
		pTone->SetSustain(true);
		return pTone;
	}
}

bool GM001_GrandPiano::Envelope(double& g)
{
	if (releaseVelocity >= 0 && !GetSustain())
	{
		return false;
		g *= lastEvelope / 100 * evlpSampleCount / releaseVelocity;
		evlpSampleCount--;
		return evlpSampleCount > 0;
	}
	else
	{
		int durationTickBase;
		//Higher pitch has a shorter duration.
		durationTickBase = (std::max)(5, static_cast<int>(pitch / 10 / 12.f * 40 + 5));

		size_t startPos = static_cast<size_t>(static_cast<double>(evlpSampleCount) / SAMPLE_RATE * durationTickBase);
		if (startPos >= std::size(envelopeData) - 1)
		{
			//Has been over the last evelope position
			g = 0;
			return false;
		}
		else
		{
			//Linear
			size_t interPos = evlpSampleCount - static_cast<size_t>(startPos * (SAMPLE_RATE / durationTickBase)); // static_cast<size_t>((static_cast<double>(evlpSampleCount) / SAMPLE_RATE - static_cast<int>(evlpSampleCount / SAMPLE_RATE)) * durationTickBase);
			evlpSampleCount++;
			lastEvelope = (envelopeData[startPos + 1] - envelopeData[startPos]) * interPos / (SAMPLE_RATE / durationTickBase) + envelopeData[startPos];
			g *= lastEvelope / 100;
			return true;
		}
	}
}

double GM001_GrandPiano::ToneGenerator()
{
	double t = toneSampleCount / SAMPLE_RATE;
	toneSampleCount++;

	double lenBase = pi2 * frequency * t;
	double len = lenBase;
	//Base
	double g1 = sin(len);
	//Harmonic waves :
/*	double g0 = 0.437 * (sin(2 * pi * 0.5 * frequency * t) * cos(0.2792) + cos(2 * pi * 0.5 * frequency * t) * sin(0.2792));	//half
	double g2 = 0.260 * (sin(2 * pi * 2 * frequency * t) * cos(2.5382) + cos(2 * pi * 2 * frequency * t) * sin(2.5382));		//
	double g3 = 0.182 * (sin(2 * pi * 3 * frequency * t) * cos(-0.866) + cos(2 * pi * 3 * frequency * t) * sin(-0.866));
	double g4 = 0.121 * (sin(2 * pi * 4 * frequency * t) * cos(3.553) + cos(2 * pi * 4 * frequency * t) * sin(3.553));
	double g5 = 0.144 * (sin(2 * pi * 5 * frequency * t) * cos(0.777) + cos(2 * pi * 5 * frequency * t) * sin(0.777));
	double g6 = 0.136 * (sin(2 * pi * 6 * frequency * t) * cos(1.593) + cos(2 * pi * 6 * frequency * t) * sin(1.593));
	double g7 = 0.016 * (sin(2 * pi * 7 * frequency * t) * cos(4.9) + cos(2 * pi * 7 * frequency * t) * sin(4.9));
	double g8 = 0.054 * (sin(2 * pi * 8 * frequency * t) * cos(-0.074) + cos(2 * pi * 8 * frequency * t) * sin(-0.074));
	double g9 = 0.092 * (sin(2 * pi * 9 * frequency * t) * cos(0.2) + cos(2 * pi * 9 * frequency * t) * sin(0.2));
	double g10 = 0.052 * (sin(2 * pi * 10 * frequency * t) * cos(-0.3) + cos(2 * pi * 10 * frequency * t) * sin(-0.3));
*/
	double g0 = 0.437 * (sin(len * 0.5) * 0.9613 + cos(len * 0.5) * 0.2756);	//half
	len += lenBase;
	double g2 = 0.260 * (sin(len) * -0.8234 + cos(len) * 0.7057);		//
	len += lenBase;
	double g3 = 0.182 * (sin(len) * 0.4679 + cos(len) * -0.7617);
	len += lenBase;
	double g4 = 0.121 * (sin(len) * -0.9166 + cos(len) * -0.3999);
	len += lenBase;
	double g5 = 0.144 * (sin(len) * 0.7130 + cos(len) * 0.7011);
	len += lenBase;
	double g6 = 0.136 * (sin(len) * -0.022 + cos(len) * 0.9999);
	len += lenBase;
	double g7 = 0.016 * (sin(len) * 0.1865 + cos(len) * -0.9825);
	len += lenBase;
	double g8 = 0.054 * (sin(len) * 0.9973 + cos(len) * -0.0739);
	len += lenBase;
	double g9 = 0.092 * (sin(len) * 0.9800 + cos(len) * 0.1987);
	len += lenBase;
	double g10= 0.052 * (sin(len) * 0.9553 + cos(len) * -0.2955);

	// Combine all the harmonic waves together
	double g = g0 + g1 + g2 + g3 + g4 + g5 + g6 + g7 + g8 + g9 + g10;
	//Then normalize to 60% of maximum volume
	g *= 32767 * 0.6 / ((1 + 0.437 + 0.260 + 0.182 + 0.121 + 0.144 + 0.136 + 0.016 + 0.054 + 0.092 + 0.052) * 2);
	return g;
}

bool GM001_GrandPiano::TriggerPulse(double& gl, double& gr)
{
	double g = ToneGenerator();
	if (Envelope(g))
	{
		g *= static_cast<double>(velocity) / 127;
		if (soft)
			g *= 0.5;
		if (autoStereo)
		{
			gl = (1 - (pitch / 127 * 0.6 + 0.2)) * g;
			gr = (pitch / 127 * 0.6 + 0.2) * g;
		}
		else
			gl = gr = g;
		return true;
	}
	else
	{
		gl = gr = 0;
		return false;
	}
}

bool GM080_Square::TriggerPulse(double& gl, double& gr)
{
	if (portamentoEnable)
	{
		PortamentoAdjust();	//portamentoEnable will be set to false when done.
	}

	double t = toneSampleCount / SAMPLE_RATE;
	toneSampleCount++;

	//Base only
	double fractionPart = frequency * t - static_cast<long long>(frequency * t);
	double g = (fractionPart > 0.5 ? 1 - fractionPart : fractionPart) * 64 - 16;
	if (g > 1)
		g = 1;
	if (g < -1)
		g = -1;

	//Attack
	if (releaseVelocity == -1)
	{
		evlpSampleCount++;
		if (evlpSampleCount < 50)
		{
			g *= static_cast<double>(evlpSampleCount) / 50;
		}
	}

	//Resonance
	g = bandPassFilter.TriggerPulse(g) * 2 + g;
	//Cutoff
	g = lowPassFilter.TriggerPulse(g);

	//Then normalize to 20% of maximum volume
	g *= 32767 * 0.2;
	//Velocity
	g *= static_cast<double>(velocity) / 127;
	if (soft)
		g *= 0.5;
	if (autoStereo)
	{
		gl = (1 - (pitch / 127 * 0.6 + 0.2)) * g;
		gr = (pitch / 127 * 0.6 + 0.2) * g;
	}
	else
		gl = gr = g;
	if (releaseVelocity >= 0 && !GetSustain())
	{
		gl *= static_cast<double>(evlpSampleCount) / releaseVelocity;
		gr *= static_cast<double>(evlpSampleCount) / releaseVelocity;
		evlpSampleCount -= 5;
		if (evlpSampleCount <= 5)
			return false;
	}
	return true;
}

bool GM081_Triangle::TriggerPulse(double& gl, double& gr)
{
	if (portamentoEnable)
	{
		PortamentoAdjust();	//portamentoEnable will be set to false when done.
	}

	double t = toneSampleCount / SAMPLE_RATE;
	toneSampleCount++;

	//Base only
	double fractionPart = frequency * t - static_cast<long long>(frequency * t);
//	double g = (fractionPart > 0.5 ? 1 - fractionPart : fractionPart) * 4 - 1;
	double g = (fractionPart > 0.2 ? (1 - fractionPart) * 5 / 8 : fractionPart * 2.5) * 4 - 1;
	//Resonance
	g = bandPassFilter.TriggerPulse(g) * 2 + g;
	//Cutoff
	g = lowPassFilter.TriggerPulse(g);
	//Then normalize to 50% of maximum volume
	g *= 32767 * 0.5;
	//Velocity
	g *= static_cast<double>(velocity) / 127;

	//Attack
	if (releaseVelocity == -1)
	{
		evlpSampleCount++;
		if (evlpSampleCount < 50)
		{
			g *= static_cast<double>(evlpSampleCount) / 50;
		}
	}

	if (soft)
		g *= 0.5;
	if (autoStereo)
	{
		gl = (1 - (pitch / 127 * 0.6 + 0.2)) * g;
		gr = (pitch / 127 * 0.6 + 0.2) * g;
	}
	else
		gl = gr = g;

	if (releaseVelocity >= 0 && !GetSustain())
	{
		gl *= static_cast<double>(evlpSampleCount) / releaseVelocity;
		gr *= static_cast<double>(evlpSampleCount) / releaseVelocity;
		evlpSampleCount -= 2;
		if (evlpSampleCount == 1 || evlpSampleCount == 0)
			return false;
	}

	return true;

}

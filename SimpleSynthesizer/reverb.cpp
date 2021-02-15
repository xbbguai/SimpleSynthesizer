/*
    SimpleSynthesizer V0.2
    Reverb effect processor.
    Copyright (C) 2021 Feng Dai

    The reverb effect is based on the C codes from libSox.
    It is transported to C++ and modified for this synthesizer.

    Information on libSox:
        libSox Copyright (c) 2007 robs@users.sourceforge.net
        Filter design based on freeverb by Jezar at Dreampoint.

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

#include "reverb.h"

void Filter::CreateBuffer(int bufferSize)
{
    size = bufferSize;
    buffer = new double[size];
    ptr = buffer;
    for (int i = 0; i < bufferSize; i++)
        buffer[i] = 0;
}

Filter::~Filter()
{
    //It is safe to delete anyway.
    delete[] buffer;
}

void FilterArray::CreateFilters(double rate, double scale, double offset)
{
    size_t i;
    double r = rate * (1 / 44100.); // Compensate for actual sample-rate 

    for (i = 0; i < std::size(comb_lengths); ++i, offset = -offset)
    {
        combs[i].CreateBuffer((size_t)(scale * r * (comb_lengths[i] + stereo_adjust * offset) + 0.5));
    }
    for (i = 0; i < std::size(allpass_lengths); ++i, offset = -offset)
    {
        allpasses[i].CreateBuffer((size_t)(r * (allpass_lengths[i] + stereo_adjust * offset) + 0.5));
    }
}

void Reverb::Create(double sample_rate_Hz,
                    double wet_gain_dB,
                    double room_scale,     //%
                    double reverberance,   //%
                    double hf_damping,     //%
                    double pre_delay_ms,
                    double stereo_depth,
                    size_t buffer_size)
{
    size_t i;
    size_t delay = static_cast<size_t>(pre_delay_ms / 1000 * sample_rate_Hz + 0.5);
    double scale = room_scale / 100 * 0.9;
    double depth = stereo_depth / 100;
    double a = -1 / log(1 - 0.3);           //Set minimum feedback
    double b = 100 / (log(1 - 0.98) * a + 1);  // Set maximum feedback

    feedback = 1 - exp((reverberance - b) / (a * b));
    hf_damping = hf_damping / 100 * 0.3 + 0.2;
    gain = dB_to_linear(wet_gain_dB) * 0.015;

//    for (i = 0; i <= ceil(depth); ++i)
    for (i = 0; i < 2; ++i)
    {
        filters[i].CreateFilters(sample_rate_Hz, scale, depth * i);
    }
}

void FxReverb::Start(int depth)
{
    if (isEnabled || depth == 0)  //Reverb params cannot be changed when it is enabled.
        return;

    size_t bufsiz = static_cast<size_t>(SAMPLE_RATE * 2);

    //for the time being.
    reverberance = static_cast<double>(depth) / 127.0 * 50 + 20; //0-127 maps to 0-60

    if (depth > 0)
    {
        for (size_t i = 0; i < ichannels; ++i)
        {
            reverbs[i].Create(
                SAMPLE_RATE,
                wet_gain_dB, room_scale, reverberance, hf_damping, pre_delay_ms, stereo_depth,
                bufsiz / ochannels);
        }
        isEnabled = true;
    }
    else
        isEnabled = false;
}
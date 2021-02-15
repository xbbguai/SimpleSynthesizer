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

#pragma once

#include <vector>
#include <iostream>

#define M_LN10 2.30258509299404568402
#define dB_to_linear(x) exp((x) * M_LN10 * 0.05)

class Filter
{
protected:
    size_t size{ 0 };
    double* buffer{ nullptr };
    double* ptr{ nullptr };
    void Advance()
    {
        if (--ptr < buffer)
            ptr += size;
    }
public:
    void CreateBuffer(int bufferSize);
    ~Filter();
};

class CombFilter : public Filter
{
protected:
    double  store{};
public:
    CombFilter() : Filter() {}

    inline double Process(const double& input, const double& feedback, const double& hf_damping)
    {
        double output = *ptr;
        store = output + (store - output) * hf_damping;
        *ptr = input + store * feedback;
        Advance();
        return output;
    }
};

class AllpassFilter : public Filter
{
public:
    AllpassFilter() : Filter() {}

    inline double Process(const double& input)
    {
        double output = *ptr;
        *ptr = input + output * 0.5;
        Advance();
        return output - input;
    }
};

//Filter delay lengths in samples (44100Hz sample-rate)
static const size_t comb_lengths[]{ 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
static const size_t allpass_lengths[]{ 225, 341, 441, 556 };

constexpr int stereo_adjust = 12;

class FilterArray
{
protected:
    CombFilter combs[std::size(comb_lengths)];
    AllpassFilter allpasses[std::size(allpass_lengths)];
public:
    FilterArray()
    {
    }

    void CreateFilters(double rate, double scale, double offset);

    inline void Process(const double& input, double& output,
        const double& feedback, const double& hf_damping, const double& gain)
    {
        output = 0;

        size_t i = std::size(comb_lengths) - 1;
        do
        {
            output += combs[i].Process(input, feedback, hf_damping);
        } while (i--);

        i = std::size(allpass_lengths) - 1;
        do
        {
            output = allpasses[i].Process(output);
        } while (i--);

        output *= gain;
    }
};

class Reverb
{
protected:
    double feedback{};
    double hf_damping{};
    double gain{};
    FilterArray filters[2]{};

public:
    void Create(double sample_rate_Hz,
        double wet_gain_dB,
        double room_scale,     //%
        double reverberance,   //%
        double hf_damping,     //%
        double pre_delay_ms,
        double stereo_depth,
        size_t buffer_size);

    inline void Process(const double& inLeft, const double& inRight, double& outLeft, double& outRight)
    {
        filters[0].Process(inLeft, outLeft, feedback, hf_damping, gain);
        filters[1].Process(inRight, outRight, feedback, hf_damping, gain);
    }
};

constexpr int ichannels = 2;
constexpr int ochannels = 2;
class FxReverb
{
    double reverberance;
    double hf_damping;
    double pre_delay_ms;
    double stereo_depth;
    double wet_gain_dB;
    double room_scale;
    bool wet_only;

    Reverb reverbs[2];

    bool isEnabled{ false };
public:
    FxReverb()
    {
        reverberance = 60;   //0-100
        hf_damping = 50;     //0-100
        stereo_depth = 100;  //0-100
        room_scale = 100;    //0-100
        wet_gain_dB = 0;     //-10 - 10
        pre_delay_ms = 5;
        wet_only = false;
    }

    void Start(int depth);

    inline void TriggerPulse(const double& inLeft, const double& inRight,
        double& outLeft, double& outRight)
    {
        if (!isEnabled)
        {
            outLeft = inLeft;
            outRight = inRight;
            return;
        }

        double oL0 = 0;
        double oR0 = 0;
        double oL1 = 0;
        double oR1 = 0;

        reverbs[0].Process(inLeft, inRight, oL0, oR0);
        reverbs[1].Process(inLeft, inRight, oL1, oR1);

        double oL = (1 - wet_only) * inLeft + 0.5 * (oL0 + oL1);
        double oR = (1 - wet_only) * inRight + 0.5 * (oR0 + oR1);

        outLeft = oL;
        outRight = oR;
    }
};

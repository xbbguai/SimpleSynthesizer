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

#pragma once

constexpr double SAMPLE_RATE = 44100;   //Standard CD quality.
constexpr double pi = 3.141592654;
constexpr double pi2 = pi * 2;

constexpr double MAX_MODULATION_FREQ = 10;
constexpr double MAX_MODULATION_PITCH = 1;
constexpr double PORTAMENTO_SPEED_CONST = 5;

#include "Filters.h"
class Tone
{
protected:
    //Counters reserved for tone generators and envelope generators.
    //Not used in this base class.
    double toneSampleCount; //increase 1 per sampling tick. It is a double because pitch bends and modulations require precise sample position calculation.
    size_t evlpSampleCount;

    double pitch;           //69 = A4, increase or decrease by one per semitone.
    double pitchBend;       //pitch bend. from -2 to 2, initialized to 0. this value will be added to pitch.
    double modulationBend; //current pitch bend value caused by modulation
    double frequency;       //Fundamental frequency according to pitch
    uint8_t velocity;       //max = 127, mute = 0
    int modulationDepth;    //max = 127. 0 = no modulation, 127 = +- 2 semitone
    int modulationSpeed;    //max = 127. 0 = 1Hz, 127 = MAX_MODULATION_FREQ
    double modulationPitchChangePerSample;
    double modulationDirection; // -1 or 1. multiply by modulationPitchChangePerSample.
    int releaseVelocity;    //64 = Neutral, 127 = hardest, 0 = softest
    bool sustain;           //Sustain until the note fade out without considering duration.
    bool sustainFlipped;    //Sustain status can change from true to false for only once.
    bool soft;              //Volume decrease by 50%
    bool autoStereo;        //[Deleted] Generate stereo signal in accordance with the piano keyboard arrangement -- bass on the left, treble on the right.
    double portamentoStep;  //How many pitch (in double) should change per pulse during portamento. 
    double portamentoPitchDiff; //The pitch difference between portamento target pitch and start pitch.
    bool portamentoEnable;  //Set true when should do portamento. Will be set false when done.

    //Filters
    LowPassFilter_1Order lowPassFilter; 
    BandPassFilter bandPassFilter;      //For resonance.

    //Calculate fundamental frequency with pitch, pitch bend and modulation bend value.
    virtual void SetFrequency()
    {
        frequency = 440 * pow(2, ((pitch + pitchBend + modulationBend + portamentoPitchDiff - 69) / 12));
    }
    virtual void ReCalibrateFrequency()
    {
        double frequencySave = frequency;
        SetFrequency();
        //Adjust toneSampleCount so that the frequency change does not affect the wave alignments.
        double t = toneSampleCount / SAMPLE_RATE;
        double len = pi2 * frequencySave * t;
        toneSampleCount = len / pi2 / frequency * SAMPLE_RATE;
    }
    virtual void PortamentoAdjust()
    {
        //portamentoEnable will be set to false when done.
        if (std::fabs(portamentoPitchDiff) <= std::fabs(portamentoStep))
        {
            portamentoPitchDiff = 0;
            portamentoEnable = false;
        }
        else
            portamentoPitchDiff += portamentoStep;
        ReCalibrateFrequency();
    }
public:
    double GetPitch() const { return pitch; }
    virtual void SetPitch(const double _pitch) 
    {
        pitch = _pitch; 
        SetFrequency();
    }
    virtual void SetPortamentoPitch(const int fromPitch, const int portamentoTime)
    {
        portamentoPitchDiff = fromPitch - pitch;
        portamentoStep = (portamentoPitchDiff < 0 ? PORTAMENTO_SPEED_CONST : -PORTAMENTO_SPEED_CONST) / (SAMPLE_RATE * 0.2 * portamentoTime / 127 + 1); //
        portamentoEnable = (portamentoPitchDiff != 0);
    }

    uint8_t GetVelocity() const { return velocity; }
    virtual void SetVelocity(const uint8_t _velocity) { velocity = _velocity; }

    bool GetSustain() const { return sustain; }
    virtual void SetSustain(const bool _sustain) 
    {
        if (!sustainFlipped)
        {
            if (sustain == true && _sustain == false)
                sustainFlipped = true;
            sustain = _sustain;
        }
    }

    const bool GetSoft() const { return soft; }
    virtual void SetSoft(const bool _soft) { soft = _soft; }

    const int GetModulationDepth() const { return modulationDepth; }
    const int GetModulationSpeed() const { return modulationSpeed; }
    virtual void SetModulation(const int depth, const int speed)
    {
        modulationDepth = depth; 
        modulationSpeed = speed;
        if (modulationDepth == 0)
        {
            modulationBend = 0;
            modulationPitchChangePerSample = 0;
        }
        else
        {
            modulationPitchChangePerSample = (static_cast<double>(modulationDepth) + 1) / 128 * 4 * (128 - static_cast<double>(modulationSpeed)) / 128 * MAX_MODULATION_FREQ / SAMPLE_RATE;
            modulationBend += modulationPitchChangePerSample * modulationDirection;
            if (modulationBend > static_cast<double>(modulationDepth) / (128 / MAX_MODULATION_PITCH))
                modulationDirection = -1;
            else if (modulationBend < -static_cast<double>(modulationDepth) / (128 / MAX_MODULATION_PITCH))
                modulationDirection = 1;
            ReCalibrateFrequency();
        }
    }

    bool GetAutoStereo() const { return autoStereo; }
    virtual void SetAutoStereo(const bool _autoStereo) { autoStereo = _autoStereo; }
 
    Tone() : velocity{ 127 },  
             sustain{ false }, 
             sustainFlipped{ false },
             soft{ false }, 
             autoStereo{ false }, 
             toneSampleCount{ 0 },
             evlpSampleCount{ 0 },
             releaseVelocity{ -1 },
             pitchBend{ 0 },
             modulationDepth{ 0 },
             modulationSpeed{ 0 },
             modulationBend{ 0 },
             modulationPitchChangePerSample{ 0 },
             modulationDirection{ 1 },
             portamentoEnable{ false },
             portamentoPitchDiff{ 0 }
    {
        SetPitch(69);
    }

    Tone(const Tone& copy)
    {
        toneSampleCount = copy.toneSampleCount;
        evlpSampleCount = copy.evlpSampleCount;
        pitch = copy.pitch;
        frequency = copy.frequency;
        velocity = copy.velocity;
        sustain = copy.sustain;
        sustainFlipped = copy.sustainFlipped;
        soft = copy.soft;
        autoStereo = copy.autoStereo;
        releaseVelocity = copy.releaseVelocity;
        pitchBend = copy.pitchBend;
        modulationDepth = copy.modulationDepth;
        modulationSpeed = copy.modulationSpeed;
        modulationBend = copy.modulationBend;
        modulationPitchChangePerSample = copy.modulationPitchChangePerSample;
        modulationDirection = copy.modulationDirection;
        portamentoEnable = copy.portamentoEnable;
        portamentoPitchDiff = copy.portamentoPitchDiff;
    }

    Tone& operator = (const Tone& copy)
    {
        toneSampleCount = copy.toneSampleCount;
        evlpSampleCount = copy.evlpSampleCount;
        pitch = copy.pitch;
        frequency = copy.frequency;
        velocity = copy.velocity;
        sustain = copy.sustain;
        sustainFlipped = copy.sustainFlipped;
        soft = copy.soft;
        autoStereo = copy.autoStereo;
        releaseVelocity = copy.releaseVelocity;
        pitchBend = copy.pitchBend;
        modulationDepth = copy.modulationDepth;
        modulationSpeed = copy.modulationSpeed;
        modulationBend = copy.modulationBend;
        modulationPitchChangePerSample = copy.modulationPitchChangePerSample;
        modulationDirection = copy.modulationDirection;
        portamentoEnable = copy.portamentoEnable;
        portamentoPitchDiff = copy.portamentoPitchDiff;
        return *this;
    }

    Tone(const double _pitch, const uint8_t _velocity = 127)
    {
        toneSampleCount = 0;
        evlpSampleCount = 0;
        velocity = _velocity;
        sustain = false;
        sustainFlipped = false;
        soft = false;
        releaseVelocity = -1;
        autoStereo = false;
        pitchBend = 0;
        modulationDepth = 0;
        modulationSpeed = 0;
        modulationBend = 0;
        modulationPitchChangePerSample = 0;
        modulationDirection = 1;
        portamentoEnable = false;
        portamentoPitchDiff = 0;
        //SetPitch should be called after pitchBend being initialized.
        SetPitch(_pitch);
    }

    //Get one group of data every pulse.
    //gl = Left channel
    //gr = Right channel
    virtual bool TriggerPulse(double& gl, double& gr) = 0;
    virtual void ReleaseKey(int velocity)
    {
        releaseVelocity = (128 - velocity) * 32;
        evlpSampleCount = releaseVelocity;
    }

    bool IsReleasing()
    {
        return releaseVelocity > 0;
    }

    virtual void PitchBend(int value, int depth)
    {
        //The pitch bend wheel value from midi is from 0 to 16383(-8192 to 8191), which means +- one whole tone.
        //The pitchBend value stores -depth to depth, pitch changes one tone every 2 depth value.
        pitchBend = (static_cast<double>(value) - 8192) / 8192 * depth;
        ReCalibrateFrequency();
    }

    virtual void SetFilterCutoffFreq(double freq)
    {
        lowPassFilter.UpdateParam(freq, SAMPLE_RATE);
    }

    virtual void SetResonanceFreq(double freq)
    {
        bandPassFilter.UpdateParam(freq, 200, 1, SAMPLE_RATE);
    }

    static Tone* CreateTone(int bank, int GMInstrument, const double _pitch, const uint8_t _velocity = 127);
};

class GM001_GrandPiano : public Tone
{
protected:
    const double envelopeData[22]{ 100, 80, 60, 40, 30, 25, 23, 21, 19, 17, 16, 13, 10, 8, 8, 6, 6, 5, 4, 3, 2, 0 };
    double lastEvelope; //So that I can do a little fading effect for KeyOff velocity

    bool Envelope(double& g);
    double ToneGenerator();
public:
    bool TriggerPulse(double& gl, double& gr);
 
    GM001_GrandPiano() : Tone(), lastEvelope(0)
    {
        
    }

    GM001_GrandPiano(const GM001_GrandPiano& copy) : Tone(copy)
    {
        lastEvelope = copy.lastEvelope;
        releaseVelocity = copy.releaseVelocity;
    }
    GM001_GrandPiano& operator = (const GM001_GrandPiano& copy)
    {
        Tone::operator = (copy);
        releaseVelocity = copy.releaseVelocity;
        return *this;
    }

    GM001_GrandPiano(const int _bank, const int _instrumentID, const double _pitch, const uint8_t _velocity = 127)
        : Tone(_pitch, _velocity), lastEvelope(0)
    {
        //bank and instrument id are omitted.
    }
};

class GM080_Square : public Tone
{
public:
    bool TriggerPulse(double& gl, double& gr);

    GM080_Square() : Tone()
    {
    }

    GM080_Square(const GM080_Square& copy) : Tone(copy)
    {
    }
    GM080_Square& operator = (const GM080_Square& copy)
    {
        Tone::operator = (copy);
        return *this;
    }

    GM080_Square(const int _bank, const int _instrumentID, const double _pitch, const uint8_t _velocity = 127)
        : Tone(_pitch, _velocity)
    {
        //bank and instrument id are omitted.
    }
};

class GM081_Triangle : public Tone
{
public:
    bool TriggerPulse(double& gl, double& gr);

    GM081_Triangle() : Tone()
    {
    }

    GM081_Triangle(const GM080_Square& copy) : Tone(copy)
    {
    }
    GM081_Triangle& operator = (const GM080_Square& copy)
    {
        Tone::operator = (copy);
        return *this;
    }

    GM081_Triangle(const int _bank, const int _instrumentID, const double _pitch, const uint8_t _velocity = 127)
        : Tone(_pitch, _velocity)
    {
        //bank and instrument id are omitted.
    }
};
/*
    SimpleSynthesizer V0.2
    Waveform tone processor.

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

class WaveformTone : public Tone
{
public:
    //-------------PUBLIC TO ALL WAVEFORM INSTRUMENTS------------------------
    struct InstrumentInfo
    {
        int bank;
        int instrumentID;
        std::string name;
    };
    static std::vector<InstrumentInfo> instrumentInfos;

    //The structure to store waveform sample data.
    struct WaveformType
    {
        int bank;
        int instrumentID;
        double pitch;           //0 - 127. stored as double type for convenience when doing calculations. 
        double pitchFrom;       //There may be many waveforms for different pitch ranges in one instrument.
        double pitchTo;
        double frequencyBase;   //The calculated frequency of the pitch.
        bool loop;              //If set true, loop from loopStartAt to loopEndAt while playing back.
        bool alwaysSustain;     //Some instruments should play all the samples without responding to NoteOff
        double loopStartAt;
        double loopEndAt;
        size_t size;            //Size of sample data.
        int16_t* leftChannel;   //16bit sample data of left channel
        int16_t* rightChannel;  //16bit sample data of right channel
    };
    
    //waveForms is static, the waveforms are loaded only once.
    //That is, waveForms is public to every note of the instrument.
    static std::vector<WaveformType> waveForms;
    
    //Load wave forms. All waveforms of one instrument are loaded into the memory only when it is needed.
    static bool LoadWaveform(int bank, int instrumentID);

    //Map those not sampled general midi instruments to a sampled one.
    static void MapGMInstrument(int& instrumentID);
    //------------------END OF PUBLIC-----------------------------------------

    //When pitch is set, I should select a waveform from the waveForms which pitch range includes the pitch.
    int selectedWaveform{ -1 };
    //Used to resample the wave for a different pitch
    double frequencyRatio{ 1 };
    //Waveform instrument is organized in banks and should have an instrument id.
    int bank{ 0 };
    int instrumentID{ 0 };

    virtual void ReCalibrateFrequency();
public:
    //Free wave forms' memory. Call only once when system shuts down.
    static void FreeWaveforms();

    virtual void SetPitch(const double _pitch)
    {
        Tone::SetPitch(_pitch);
        //Find the proper waveform to fit the pitch.
        for (size_t i = 0; i < waveForms.size(); i++)
        {
            if (waveForms[i].bank == bank && waveForms[i].instrumentID == instrumentID && waveForms[i].pitchFrom <= pitch && waveForms[i].pitchTo >= pitch)
            {
                selectedWaveform = i;
                frequencyRatio = frequency / waveForms[i].frequencyBase;
                return;
            }
        }
    }

    virtual bool TriggerPulse(double& gl, double& gr);
    virtual void ReleaseKey(int velocity);

    WaveformTone() : Tone()
    {
    }

    WaveformTone(const int _bank, const int _instrumentID, const double _pitch, const uint8_t _velocity = 127)
        : Tone(_pitch, _velocity)
    {
        bank = _bank;
        instrumentID = _instrumentID;
        SetPitch(_pitch);
    }

    WaveformTone(const WaveformTone& copy) : Tone(copy)
    {
        selectedWaveform = copy.selectedWaveform;
    }

    WaveformTone& operator = (const WaveformTone& copy)
    {
        Tone::operator = (copy);
        return *this;
    }

};
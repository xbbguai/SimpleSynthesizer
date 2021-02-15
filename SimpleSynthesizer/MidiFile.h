/*
    SimpleSynthesizer V0.2
    MIDI File reader
    Read a MIDI file and construct the data structure for MIDI tracks and events
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

#pragma pack(push)
#pragma pack(2)
struct MidiHeader
{
    uint32_t midiMark;
    uint32_t headerSize;
    uint16_t midiType;      //0 = single track, 1 = multiple tracks and synchronized, 2 - multiple tracks without synchronization
    uint16_t trackCount;
    uint16_t timeBase;      //MIDI tick count that a quater note lasts.
};
#pragma pack(pop)

//Events
constexpr uint8_t E_NoteOff = 0x80;
constexpr uint8_t E_NoteOn = 0x90;
constexpr uint8_t E_KeyAfterTouch = 0xa0;
constexpr uint8_t E_Controller = 0xb0;
constexpr uint8_t E_Program = 0xc0;         //Instrument setting/change event is called "program" in midi.
constexpr uint8_t E_TouchPressure = 0xd0;
constexpr uint8_t E_PitchBend = 0xe0;
constexpr uint8_t E_SystemCode = 0xf0;
constexpr uint8_t E_NonMidi = 0xff;

//Non-midi events (meta events)
constexpr uint8_t M_Text = 0x1;
constexpr uint8_t M_CopyRight = 0x02;
constexpr uint8_t M_TrackName = 0x03;
constexpr uint8_t M_InstrumentName = 0x04;
constexpr uint8_t M_Lyrics = 0x05;
constexpr uint8_t M_Mark = 0x06;
constexpr uint8_t M_Remark = 0x07;
constexpr uint8_t M_EndOfTrack = 0x2f;
constexpr uint8_t M_PlaybackSpeed = 0x51;

//Midi control codes
constexpr uint8_t C_BankSelectMSB = 0;
constexpr uint8_t C_ModulationWheelCoarse = 1;
constexpr uint8_t C_BreathControllerCoarse = 2;
constexpr uint8_t C_FootPedalCoarse = 4;
constexpr uint8_t C_PortamentoTimeCoarse = 5;
constexpr uint8_t C_DataEntryMSB = 6;
constexpr uint8_t C_VolumeCoarse = 7;
constexpr uint8_t C_BalanceCoarse = 8;
constexpr uint8_t C_PanCoarse = 10;
constexpr uint8_t C_ExpressionCoarse = 11;
constexpr uint8_t C_EffectControl1Coarse = 12;
constexpr uint8_t C_EffectControl2Coarse = 13;
constexpr uint8_t C_GeneralPurposeSlider1 = 16;
constexpr uint8_t C_GeneralPurposeSlider2 = 17;
constexpr uint8_t C_GeneralPurposeSlider3 = 18;
constexpr uint8_t C_GeneralPurposeSlider4 = 19;
constexpr uint8_t C_BankSelectLSB = 32;
constexpr uint8_t C_ModulationWheelFine = 33;
constexpr uint8_t C_BreathControllerFine = 34;
constexpr uint8_t C_FootPedalFine = 36;
constexpr uint8_t C_PortamentoTimeFine = 37;
constexpr uint8_t C_DataEntryLSB = 38;
constexpr uint8_t C_VolumeFine = 39;
constexpr uint8_t C_BalanceFine = 40;
constexpr uint8_t C_PanFine = 42;
constexpr uint8_t C_ExpressionFine = 43;
constexpr uint8_t C_EffectControl1Fine = 44;
constexpr uint8_t C_EffectControl2Fine = 45;
constexpr uint8_t C_HoldPedal = 64;
constexpr uint8_t C_Portamento = 65;
constexpr uint8_t C_SustenutoPedal = 66;
constexpr uint8_t C_SoftPedal = 67;
constexpr uint8_t C_LegatoPedal = 68;
constexpr uint8_t C_HoldPedal2 = 69;
constexpr uint8_t C_SoundVariation = 70;
constexpr uint8_t C_SoundTimbre = 71;
constexpr uint8_t C_SoundReleaseTime = 72;
constexpr uint8_t C_SoundAttackTime = 73;
constexpr uint8_t C_SoundBrightness = 74;
constexpr uint8_t C_SoundControl6 = 75;
constexpr uint8_t C_SoundControl7 = 76;
constexpr uint8_t C_SoundControl8 = 77;
constexpr uint8_t C_SoundControl9 = 78;
constexpr uint8_t C_SoundControl10 = 79;
constexpr uint8_t C_GPButton1 = 80;
constexpr uint8_t C_GPButton2 = 81;
constexpr uint8_t C_GPButton3 = 82;
constexpr uint8_t C_GPButton4 = 83;
constexpr uint8_t C_84 = 84;
constexpr uint8_t C_EffectsLevel = 91;  //reverb?
constexpr uint8_t C_TremuloLevel = 92;  //²üÒô
constexpr uint8_t C_ChorusDepth = 93;
constexpr uint8_t C_CelesteLevel = 94;  //·çÇÙµÄÒô½ÚË¨
constexpr uint8_t C_PhaserLevel = 95;
constexpr uint8_t C_DataButtonIncr = 96;
constexpr uint8_t C_DataButtonDecr = 97;
constexpr uint8_t C_NRPN_LSB = 98;
constexpr uint8_t C_NRPN_MSB = 99;
constexpr uint8_t C_RPN_LSB = 100;
constexpr uint8_t C_RPN_MSB = 101;
constexpr uint8_t C_AllSoundOff = 120;
constexpr uint8_t C_SAllControllersOff = 121;
constexpr uint8_t C_LocalKeyboard = 122;
constexpr uint8_t C_AllNotesOff = 123;
constexpr uint8_t C_OmniModeOff = 124;
constexpr uint8_t C_OmniModeOn = 125;
constexpr uint8_t C_MonoOperation = 126;
constexpr uint8_t C_PolyOperation = 127;

struct MidiEvent
{
    size_t timeTicks;
    uint8_t event;
    std::vector<uint8_t> params;
};

constexpr int MAX_MIDI_CHANNELS = 16;
struct MidiTrack
{
    std::vector<MidiEvent> channels[MAX_MIDI_CHANNELS]{};
};

class MidiDataCore
{
private:
    template<typename T>
    void BigEndian(T* data)
    {
        uint8_t ex;
        for (int i = 0; i < sizeof(T) / 2; i++)
        {
            ex = ((char*)data)[i];
            ((char*)data)[i] = ((char*)data)[sizeof(T) - i - 1];
            ((char*)data)[sizeof(T) - i - 1] = ex;
        }
    }
public:
    MidiHeader header{};
    std::vector<MidiTrack> tracks{};

    MidiDataCore() {};

    void LoadMidiFile(std::string fileName);
};

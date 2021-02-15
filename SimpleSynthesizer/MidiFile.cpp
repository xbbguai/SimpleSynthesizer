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

#include <vector>
#include <string>
#include <fstream>
#include "MidiFile.h"

//Load a midi file and interpret its content to tracks.
//May throw file i/o exception when reading file fails.
void MidiDataCore::LoadMidiFile(std::string fileName)
{
    //Clear all data
    tracks.clear();

    std::ifstream file;
    file.open(fileName, std::ios::in | std::ios::binary);
    
    //Read and check header
    file.read((char*)(&header), sizeof(header));
    BigEndian<uint32_t>(&header.headerSize);
    BigEndian<uint16_t>(&header.midiType);
    BigEndian<uint16_t>(&header.trackCount);
    BigEndian<uint16_t>(&header.timeBase);
    if (header.midiMark != 0x6468544d)  //MThd --> dhTM (Big endian)
        throw 0;
    if (header.headerSize != 6)
        throw 0;

    //Read track chuncks
    uint32_t temp32{};
    uint8_t temp8{};
    int32_t trackSize{};
    for (int i = 0; i < header.trackCount; i++)
    {
        if (file.eof()) //Never expect eof here.
            throw 0;

        MidiTrack track{};

        //Check track marker
        file.read((char*)&temp32, sizeof(uint32_t));
        if (temp32 != 0x6b72544d)   //MTrk --> krTM (Big endian)
            throw 0;

        //Track track size
        file.read((char*)&trackSize, sizeof(trackSize));
        BigEndian<int32_t>(&trackSize);

        //Events
        uint8_t lastEventCode{};
        size_t timeTicks = 0;
        while (trackSize > 0)
        {
            MidiEvent midiEvent{};
            size_t deltaTime = 0;
            //Read and parse delta time
            do
            {
                file.read((char*)&temp8, sizeof(temp8));
                deltaTime <<= 7;
                deltaTime += (temp8 & 0x7f);
                trackSize--;
            } while (trackSize > 0 && temp8 > 0x7f);
            timeTicks += deltaTime;
            midiEvent.timeTicks = timeTicks;

            //Event
            if (trackSize > 0)
            {
                file.read((char*)&temp8, sizeof(temp8));
                trackSize--;

                int channel{}; //Tone that SystemCode and NonMidi will be put to channel 0.
                if (temp8 < 0x80)
                {
                    midiEvent.event = lastEventCode & 0xf0;
                    channel = lastEventCode & 0x0f;
                    //When using the lastEventCode, it should not be SystemCode(0xf0) nor NonMidi(0xff)
                    if ((midiEvent.event & 0xf0) == 0xf0)
                        throw 0;
                    midiEvent.params.push_back(temp8);  //First byte of the param
                }
                else
                {
                    if (temp8 == E_SystemCode || temp8 == E_NonMidi)
                    {
                        midiEvent.event = temp8;
                        channel = 0;
                    }
                    else
                    {
                        midiEvent.event = temp8 & 0xf0;
                        channel = temp8 & 0x0f;
                    }
                    //Read one param byte
                    file.read((char*)&temp8, sizeof(temp8));
                    trackSize--;
                    midiEvent.params.push_back(temp8);  //First byte of the param
                }
                //Read in the rest param bytes
                size_t paramSize = 0;   //for E_SystemCode and E_NonMidi
                switch (midiEvent.event)
                {
                case E_NoteOn:
                case E_NoteOff:
                case E_KeyAfterTouch:
                case E_Controller:
                case E_PitchBend:
                    //These events have two bytes of param.
                    file.read((char*)&temp8, sizeof(temp8));
                    trackSize--;
                    midiEvent.params.push_back(temp8);  //First byte of the param
                    if (trackSize < 0)
                        throw 0;
                    break;
                case E_SystemCode:
                    //Param0 is the count of data bytes, with the end byte of 0xf7
                    //But should take care because if Param0 > 0x7f, the length is encoded. I should read in the rest of the encoded length.
                    while ((midiEvent.params.back() & 0x80) > 0)
                    {
                        paramSize <<= 7;
                        paramSize += (midiEvent.params.back() & 0x7f);
                        file.read((char*)&temp8, sizeof(temp8));
                        midiEvent.params.push_back(temp8);
                        trackSize--;
                        if (trackSize < 0)
                            throw 0;
                    }
                    paramSize <<= 7;
                    paramSize += (midiEvent.params.back() & 0x7f);
                    for (size_t n = 0; n < paramSize; n++)
                    {
                        file.read((char*)&temp8, sizeof(temp8));
                        trackSize--;
                        midiEvent.params.push_back(temp8);
                        if (trackSize < 0)
                            throw 0;
                    }
                    if (midiEvent.params.back() != 0xf7)
                        throw 0;
                    break;
                case E_NonMidi:
                    //Param0 is the program code, param1 is the count of data bytes.
                    file.read((char*)&temp8, sizeof(temp8));
                    trackSize--;
                    midiEvent.params.push_back(temp8);
                    if (trackSize < 0)
                        throw 0;
                    for (int n = 0; n < midiEvent.params[1]; n++)
                    {
                        file.read((char*)&temp8, sizeof(temp8));
                        trackSize--;
                        midiEvent.params.push_back(temp8);
                        if (trackSize < 0)
                            throw 0;
                    }
                    break;
                //Instrument and TouchPressure have only one param. No need to do anything here.
                case E_Program:
                case E_TouchPressure:
                    break;
                }

                track.channels[channel].push_back(midiEvent);
                lastEventCode = midiEvent.event + channel;
            }
            else
                throw 0;
        }

        tracks.push_back(track);
    }

    file.close();
}
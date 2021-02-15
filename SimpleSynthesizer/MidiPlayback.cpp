/*
	SimpleSynthesizer V0.2
	Interpret MIDI events and generate PCM data to a buffer.
	Call PrepareBuffer to generate PCM data to the buffer and then let OS play it out.
	PrepareBuffer should be called all the way until it returns false at the end of MIDI events.
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
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include "MidiFile.h"
#include "Tone.h"
#include "MidiPlayback.h"
#include "WaveformTone.h"

#if (TRACE_PROCESS_TIME)
#include <chrono>
#endif

void MidiPlayback::ChannelStatus::ParseEvent(const uint8_t& event, const std::vector<uint8_t>& params)
{
	if (event == E_NoteOn || event == E_NoteOff)
	{
		if (params[1] > 0 && event == E_NoteOn)
		{
			for (Tone*& p : pTones)
			{
				if (p == nullptr)
				{
					if (percussionBank >= 0)	//Percussion channel
						p = Tone::CreateTone(percussionBank + 512, instrumentID, params[0], params[1]);
					else 	//other channel
						p = Tone::CreateTone(instrumentBank, instrumentID, params[0], params[1]);
					p->SetSoft(soft);
					break;
				}
			}
		}
		else
		{
			for (Tone*& p : pTones)
			{
				if (p && p->GetPitch() == static_cast<double>(params[0]))
					p->ReleaseKey(params[1]);
			}
		}
	}
	else if (event == E_Program)
	{
		instrumentID = params[0];
	}
	else if (event == E_Controller)
	{
		switch (params[0])
		{
		case C_VolumeCoarse:
			volume = params[1];
			break;
		case C_PanCoarse:
			pan = params[1];
			break;
		case C_ExpressionCoarse:
			expression = params[1];
			break;
		case C_HoldPedal:
			sustain = (params[1] > 63);
			break;
		case C_SoftPedal:
			soft = (params[1] > 63);
			break;
		case C_ModulationWheelCoarse:
			modulationDepth = params[1];
			break;
		case C_ModulationWheelFine:
			modulationSpeed = params[1];
			break;
		case C_EffectsLevel:
			//Yamaha XG seems to use EffectsLevel to control reverb depth.
			reverbDepth = params[1];
			reverbProcessor.Start(reverbDepth);
			break;
		case C_ChorusDepth:
			chorusDepth = params[1];
			chorusProcessor.Start(chorusDepth);
			break;
		case C_CelesteLevel:
			//Yamaha XG seems to use CelesteLevel to control echo depth.
			echoDepth = params[1];
			echoProcessor.Start(echoDepth);
			break;
		case C_RPN_LSB:
			SetRPNLSB(params[1]);
			std::cout << " R L: " << (int)params[1] << std::endl;
			break;
		case C_RPN_MSB:
			SetRPNMSB(params[1]);
			std::cout << " R M: " << (int)params[1] << std::endl;
			break;
		case C_NRPN_LSB:
			SetNRPNLSB(params[1]);
			std::cout << " NR L: " << " " << (int)params[1] << std::endl;
			break;
		case C_NRPN_MSB:
			SetNRPNMSB(params[1]);
			std::cout << " NR M: " << " " << (int)params[1] << std::endl;
			break;
		case C_DataEntryMSB:
			WriteRPNDataMSB(params[1]);
			std::cout << " D M: " << " " << (int)params[1] << std::endl;
			break;
		case C_DataEntryLSB:
			WriteRPNDataLSB(params[1]);
			std::cout << " D L: " << " " << (int)params[1] << std::endl;
			break;
		default:
			//Not implemented, just print out.
			std::cout << " + " << (int)params[0] << " " << (int)params[1] << std::endl;
			break;
		}
	}
	else if (event == E_PitchBend)
	{
		short value = (static_cast<short>(params[0]) & 0x7f) | (params[1] << 7);
		for (int n = MAX_POLYPHONICS - 1; n >= 0; n--)
		{
			Tone*& p = pTones[n];
			if (p)
			{
				p->PitchBend(value, pitchBendDepth);
				break;
			}
		}
	}
}

void MidiPlayback::ChannelStatus::TriggerPulse(double& outLeft, double& outRight)
{
	double left{ 0 };
	double right{ 0 };
	double leftTotal{ 0 };
	double rightTotal{ 0 };
	for (int n = 0; n < MAX_POLYPHONICS; n++)
	{
		if (pTones[n])
		{
			pTones[n]->SetSustain(sustain);
			pTones[n]->SetModulation(modulationDepth, modulationSpeed);
			if (pTones[n]->TriggerPulse(left, right))
			{
				double vol = static_cast<double>(volume) / 127;
				double expr = static_cast<double>(expression) / 127;
				double panPos = static_cast<double>(pan) / 127;
				leftTotal += left * vol * expr * (1 - panPos);
				rightTotal += right * vol * expr * panPos;
			}
			else
			{
				delete pTones[n];
				int m;
				for (m = n; m < MAX_POLYPHONICS - 1; m++)
					pTones[m] = pTones[m + 1];
				pTones[m] = nullptr;
				n--;
			}
		}
		else
			break;
	}

	chorusProcessor.TriggerPulse(leftTotal, rightTotal, leftTotal, rightTotal);
	echoProcessor.TriggerPulse(leftTotal, rightTotal, leftTotal, rightTotal);
	reverbProcessor.TriggerPulse(leftTotal, rightTotal, leftTotal, rightTotal);

	outLeft = leftTotal;
	outRight = rightTotal;
#if (TRACE_PEAK)
	peakPulseCounter++;
	if (peakPulseCounter > SAMPLE_RATE / 100)
	{
		peakWritePos = (peakWritePos + 1) % 10;
		peakPulseCounter = 0;
	}
	int vol = static_cast<int>((std::abs(outLeft) + std::abs(outRight)) / 327.68 / 2);
	peaksFIFO[peakWritePos] = std::max(peaksFIFO[peakWritePos], vol);
#endif
}


MidiPlayback::~MidiPlayback()
{
	WaveformTone::FreeWaveforms();
}

void MidiPlayback::LoadMidiFile(std::string fileName)
{
	Rewind();

	midiData.LoadMidiFile(fileName);
	//Prepare tracks for timing.
	tracksStatus.clear();
	for (size_t i = 0; i < midiData.tracks.size(); i++)
	{
		TrackStatus tpb{};
		tracksStatus.push_back(tpb);
	}

	WaveformTone::FreeWaveforms();
	//Scan the midi events and get program events, 
	//load waveforms of these instruments.
	for (auto& track : midiData.tracks)
	{
		for (auto& channel : track.channels)
		{
			for (auto& evt : channel)
			{
				if ((evt.event & 0xf0) == E_Program)
				{
					WaveformTone::LoadWaveform(0, evt.params[0]);
				}
			}
		}
	}
	//Load at lease one drum set
	WaveformTone::LoadWaveform(512, 0);
	//Load at lease the piano
	WaveformTone::LoadWaveform(0, 0);
}

bool MidiPlayback::PrepareBuffer(char* pBuffer, size_t bufferSize)
{
#if (TRACE_PROCESS_TIME)
	auto timeNow = std::chrono::system_clock::now();
#endif
	for (size_t i = 0; i < bufferSize; i += 4)
	{
		midiTick = static_cast<int>(currentSampleIdx / samplesPerMidiTick);
		currentSampleIdx++;

		//If it's a new midi tick
		if (midiTick != lastMidiTick)
		{
			lastMidiTick = midiTick;

			//Get each event from midi data
			for (size_t t = 0; t < midiData.tracks.size(); t++)
			{
				for (uint8_t ch = 0; ch < MAX_MIDI_CHANNELS; ch++)
				{
					auto& events = midiData.tracks[t].channels[ch];
					auto& currentEventIdx = tracksStatus[t].channels[ch].currentEventIdx;
					//If current midi tick is the event's expected tick...
					while (currentEventIdx < events.size() && events[currentEventIdx].timeTicks == midiTick)
					{
						//Parse the event
						tracksStatus[t].channels[ch].ParseEvent(events[currentEventIdx].event, events[currentEventIdx].params);
						if (events[currentEventIdx].event == E_SystemCode)
						{
/*							std::cout << " S ";
							for (auto& item : events[currentEventIdx].params)
							{
								std::cout << (int)item << " ";
							}
							std::cout << std::endl;
*/
							//07 7f 7f 04 01 00 xx f7 means master volume change.
							//I don't know why.
							if (events[currentEventIdx].params[0] == 7 && events[currentEventIdx].params[1] == 0x7f && events[currentEventIdx].params[2] == 0x7f)
							{
								if (events[currentEventIdx].params[3] == 4 && events[currentEventIdx].params[4] == 1 && events[currentEventIdx].params[5] == 0)
									masterVolume = static_cast<double>(events[currentEventIdx].params[6]) / 127.;
							}
						}
						else if (events[currentEventIdx].event == E_NonMidi)
						{
							switch (events[currentEventIdx].params[0])
							{
							case M_Text:
							case M_CopyRight:
							case M_TrackName:
							case M_InstrumentName:
							case M_Lyrics:
							case M_Mark:
							case M_Remark:
								for (int n = 0; n < events[currentEventIdx].params[1]; n++)
								{
									std::cout << events[currentEventIdx].params[2 + n];
								}
								std::cout << std::endl;
								break;
							case M_EndOfTrack:
								tracksStatus[t].trackEnd = true;
								break;
							case M_PlaybackSpeed:
								samplesPerMidiTick = 0;
								for (int co = 0; co < events[currentEventIdx].params[1]; co++)
								{
									samplesPerMidiTick *= 256;
									samplesPerMidiTick += events[currentEventIdx].params[2 + co];
								}
								samplesPerMidiTick = samplesPerMidiTick * SAMPLE_RATE / 1000000 / midiData.header.timeBase;
								//Recalibrate currentSampleIdx when speed changed during playing back
								currentSampleIdx = midiTick * samplesPerMidiTick + 1;
							}
						}
						currentEventIdx++;
					}
				}
			}
		}

		double leftTotal = 0, rightTotal = 0;
		double left = 0, right = 0;
		int16_t vLeft = 0, vRight = 0;

		for (auto& track : tracksStatus)
		{
			for (auto& chn : track.channels)
			{
				chn.TriggerPulse(left, right);
				leftTotal += left;
				rightTotal += right;
			}
		}

		//main volume
		leftTotal *= masterVolume;
		rightTotal *= masterVolume;

		vLeft = leftTotal > 32767 ? 32767 : (leftTotal < -32768 ? -32768 : static_cast<int16_t>(leftTotal));
		vRight = rightTotal > 32767 ? 32767 : (rightTotal < -32768 ? -32768 : static_cast<int16_t>(rightTotal));
#if (TRACE_PEAK)
		peakPulseCounter++;
		if (peakPulseCounter > SAMPLE_RATE / 100)
		{
			peakWritePos = (peakWritePos + 1) % 10;
			peakPulseCounter = 0;
		}
		peaksLeftFIFO[peakWritePos] = peaksLeftFIFO[peakWritePos] > vLeft ? peaksLeftFIFO[peakWritePos] : vLeft;
		peaksRightFIFO[peakWritePos] = peaksRightFIFO[peakWritePos] > vRight ? peaksLeftFIFO[peakWritePos] : vRight;
#endif

		pBuffer[i + 0] = (vLeft & 0xff);
		pBuffer[i + 1] = (vLeft >> 8);
		pBuffer[i + 2] = (vRight & 0xff);
		pBuffer[i + 3] = (vRight >> 8);
	}

	bool eof = true;
	for (size_t i = 0; i < midiData.tracks.size(); i++)
	{
		eof &= tracksStatus[i].trackEnd;
	}

#if (TRACE_PROCESS_TIME)
	auto span = std::chrono::system_clock::now() - timeNow;
	cpuPercentage = (std::chrono::duration_cast<std::chrono::milliseconds>(span)).count() / (SAMPLE_RATE / bufferSize * 1000);
#endif

	return !eof;
}

void MidiPlayback::Rewind()
{
	//Reset all status for the next playback.
	currentSampleIdx = 0;
	midiTick = 0;
	lastMidiTick = -1;
	samplesPerMidiTick = 183.75 / 3 ;
	masterVolume = 1.;
	peakReadPos = 6;
	peakWritePos = 0;

	for (auto& item : tracksStatus)
	{
		item.trackEnd = false;
		for (int i = 0; i < MAX_MIDI_CHANNELS; i++)
			item.channels[i].ResetAll();
	}
}
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
#pragma once

#include "chorus.h"
#include "echo.h"
#include "reverb.h"

#define TRACE_PROCESS_TIME true
#define TRACE_PEAK true
#define USE_GLOBAL_EFFECT_PROCESSOR true	//If set false, every channel has its independent reverb, chorus and echo processors.

constexpr int MAX_POLYPHONICS = 64;	//max polyphonics per channel.

class MidiPlayback
{
public:
	struct ChannelStatus
	{
		int instrumentBank{ 0 };
		int percussionBank{ -1 };	//This will be set to a number >= 0 if it is a percussion channel.
		int instrumentID{ 0 };

		int volume{ 127 };
		int pan{ 64 };
		int expression{ 127 };
		bool sustain{ false };
		bool soft{ false };

		int modulationDepth{ 0 };
		int modulationSpeed{ 64 };
		int pitchBendDepth{ 2 };

		int lastPitch{ -1 };					//For portamento
		int portamentoTime{ 0 };
		bool portamentoEnable{ false };

		int reverbDepth{ 0 };
		int chorusDepth{ 0 };
		int echoDepth{ 0 };

		int cutOff{ 0 };
		int resonance{ 0 };

#if (!USE_GLOBAL_EFFECT_PROCESSOR)
		FxReverb reverbProcessor;
		FxChorus chorusProcessor;
		FxEcho echoProcessor;

		//Set up chorus params and call UpdateChorus
		void UpdateChorus()
		{
			chorusProcessor.Start(chorusDepth);
		}

		//Set up reverb params and call UpdateReverb
		void UpdateReverb()
		{
			reverbProcessor.Start(reverbDepth);
		}

		//Set up echo params and call UpdateEcho
		void UpdateEcho()
		{
			echoProcessor.Start(echoDepth);
		}
#endif

		int RPNLSB{ 0x7f };
		int RPNMSB{ 0x7f };
		int NRPNLSB{ 0x7f };
		int NRPNMSB{ 0x7f };
		int dataMSBSave{ 0 };

		int drumPan[128];		//For NRPN to adjust drum panpot. Valid only when percussion bank >= 0
		int drumReverb[128];	//[Not implemented] For NRPN to adjust drum reverb. Valid only when percussion bank >= 0

		size_t currentEventIdx{ 0 };


		void ResetAll()
		{
			ResetRPN();
			volume = 127;
			pan = 64;
			sustain = false;
			soft = false;
			modulationDepth = 0;
			modulationSpeed = 64;
			pitchBendDepth = 2;
			lastPitch = -1;
			portamentoEnable = false;
			portamentoTime = 0;
			currentEventIdx = 0;
			expression = 127;
			peakReadPos = 6;
			peakWritePos = 0;
			cutOff = 0;
			resonance = 0;

			for (auto& item : pTones)
			{
				if (item != nullptr)
				{
					delete item;
					item = nullptr;
				}
			}

			for (auto& item : drumPan)
				item = 64;
			for (auto& item : drumReverb)
				item = 64;
		}

		void ResetRPN()
		{
			RPNLSB = 0x7f;
			RPNMSB = 0x7f;
			NRPNMSB = NRPNLSB = 0x7f;
		}

		void SetRPNMSB(int msb)
		{
			RPNMSB = msb;
		}
		void SetRPNLSB(int lsb)
		{
			RPNLSB = lsb;
		}
		void SetNRPNMSB(int msb)
		{
			NRPNMSB = msb;
		}
		void SetNRPNLSB(int lsb)
		{
			NRPNLSB = lsb;
		}

		void WriteDataMSB(int data)
		{
			dataMSBSave = data;
			if (RPNLSB != 0x7f && RPNMSB != 0x7f)
			{
				//So, its RPN
				if (RPNLSB == 0 && RPNMSB == 0)	//Pitch bend sensitivity
				{
					pitchBendDepth = data;
				}
				else if (RPNLSB == 1 && RPNMSB == 0)	//Fine tuning
				{
				}
				else if (RPNLSB == 2 && RPNMSB == 0)	//Coarse tuning
				{
				}
				else if (RPNLSB == 3 && RPNMSB == 0)	//Tuning program select
				{
				}
				else if (RPNLSB == 4 && RPNMSB == 0)	//Tuning banck select
				{
				}
				//Done. Reset LSB and MSB and prevent useless msb written.
				RPNLSB = RPNMSB = 0x7f;
			}
			else if (NRPNLSB != 0x7f && NRPNMSB != 0x7f)
			{
				//So, its NRPN
//				NRPNMSB = NRPNLSB = 0x7f;
			}
		}

		void WriteDataLSB(int data)
		{
			if (NRPNLSB != 0x7f && NRPNMSB != 0x7f)
			{
				//XG doesn't use NRPN?
				//GS uses NRPN.
				switch (NRPNMSB)
				{
				case 1:
					switch (NRPNLSB)
					{
					case 8:	//vibrato rate
						break;
					case 9: //vibrato depth
						break;
					case 10: //vibrato delay
						break;
					case 32:	//filter cutoff frequency
						cutOff = (dataMSBSave << 7) + data;
						break;
					case 33:	//filter resonance
						resonance = (dataMSBSave << 7) + data;
						break;
					case 99:	//EG Attack time
						break;
					case 100:	//EG delay time
						break;
					case 102:	//EG release time
						break;
					}
					break;
				case 0x18:	//Drum instrument pitch coarse
					break;
				case 0x1a:	//Drum instrument TVA level
					break;
				case 0x1c:	//Drum instrument panpot
					drumPan[NRPNLSB] = data;
					break;
				case 0x1d:	//Drum instrument reverb send level
					drumReverb[NRPNLSB] = data;
					break;
				case 0x1e:	//Drum chorus send level
					break;
				case 0x1f:	//Drum instrument delay (variation) send level
					break;
				}
				NRPNMSB = NRPNLSB = 0x7f;
			}
		}

		Tone* pTones[MAX_POLYPHONICS]{};

#if (TRACE_PEAK)
		int peaksFIFO[10]{};
		int peakReadPos{ 6 };
		int peakWritePos{ 0 };
		int peakPulseCounter{ 0 };

		int GetPeak()
		{
			int peak = peaksFIFO[peakReadPos];
			peaksFIFO[peakReadPos] = 0;
			peakReadPos = (peakReadPos + 1) % 10;
			return peak;
		}
#endif
		void ParseEvent(const uint8_t& event, const std::vector<uint8_t>& params);
		void TriggerPulse(double& outLeft, double& outRight);

		//If this channel is in use.
		bool IsInUse()
		{
			return currentEventIdx != 0;
		}
	};

	struct TrackStatus
	{
		bool trackEnd{ false };
		bool autoStereo{ false };

		ChannelStatus channels[MAX_MIDI_CHANNELS];

		TrackStatus()
		{
			//Channel 9 is defaultly set to percussion channel.
			channels[9].percussionBank = 0;
		}
	};
	std::vector<TrackStatus> tracksStatus{};

	double currentSampleIdx{ 0 };
	double samplesPerMidiTick{ 183.75 / 3 };
	int midiTick{ 0 };
	int lastMidiTick{ -1 };

	MidiDataCore midiData;

	double masterVolume{ 1.0 };	//change by system code of: 7f 7f 04 01 00 xx (xx = 0 - 7f)
#if (TRACE_PROCESS_TIME)
	double cpuPercentage{ 0 };
#endif
#if (TRACE_PEAK)
	int peaksLeftFIFO[10]{};
	int peaksRightFIFO[10]{};
	int peakReadPos{ 6 };
	int peakWritePos{ 0 };
	int peakPulseCounter{ 0 };

	void GetPeak(int& left, int& right)
	{
		left = peaksLeftFIFO[peakReadPos] / 327;
		right = peaksRightFIFO[peakReadPos] / 327;
		peaksLeftFIFO[peakReadPos] = 0;
		peaksRightFIFO[peakReadPos] = 0;
		peakReadPos = (peakReadPos + 1) % 10;
	}
#endif

	//Global effect processors
#if (USE_GLOBAL_EFFECT_PROCESSOR)
	FxChorus chorusProcessor{ true };
	FxEcho echoProcessor{ true };
	FxReverb reverbProcessor{ true };
#endif

public:
	MidiPlayback()
	{
#if (USE_GLOBAL_EFFECT_PROCESSOR)
		chorusProcessor.Start(127);
		echoProcessor.Start(127);
		reverbProcessor.Start(127);
#endif
	}

	~MidiPlayback();

	void LoadMidiFile(std::string fileName);
	void Rewind();
	bool PrepareBuffer(char* pBuffer, size_t bufferSize);
};


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

constexpr int MAX_POLYPHONICS = 64;

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

		int reverbDepth{ 0 };
		FxReverb reverbProcessor;
		int chorusDepth{ 0 };
		FxChorus chorusProcessor;
		int echoDepth{ 0 };
		FxEcho echoProcessor;

		int RPNLSB{ 0 };
		int RPNMSB{ 0 };
		int NRPNLSB{ 0 };
		int NRPNMSB{ 0 };

		size_t currentEventIdx{ 0 };

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
			currentEventIdx = 0;
			expression = 127;
			peakReadPos = 6;
			peakWritePos = 0;

			for (auto& item : pTones)
			{
				if (item != nullptr)
				{
					delete item;
					item = nullptr;
				}
			}
		}

		void ResetRPN()
		{
			pitchBendDepth = 2;
			RPNLSB = 0;
			RPNMSB = 0;
		}

		void SetRPNMSB(int msb)
		{
			RPNMSB = msb;
			if (RPNMSB == 0x7f && RPNLSB == 0x7f)
				ResetRPN();
		}
		void SetRPNLSB(int lsb)
		{
			RPNLSB = lsb;
			if (RPNMSB == 0x7f && RPNLSB == 0x7f)
				ResetRPN();
		}
		void SetNRPNMSB(int msb)
		{
			NRPNMSB = msb;
		}
		void SetNRPNLSB(int lsb)
		{
			NRPNLSB = lsb;
		}

		void WriteRPNDataMSB(int data)
		{
			if (RPNLSB == 0 && RPNMSB == 0)	//Change pitch bend depth
				pitchBendDepth = data;
		}

		void WriteRPNDataLSB(int data)
		{
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
public:
	MidiPlayback()
	{
	}

	~MidiPlayback();

	void LoadMidiFile(std::string fileName);
	void Rewind();
	bool PrepareBuffer(char* pBuffer, size_t bufferSize);
};


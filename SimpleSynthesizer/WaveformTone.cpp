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

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <io.h>
#include <stdlib.h>
#include "Tone.h"
#include "WaveformTone.h"

//Static members of WaveformTone
std::vector<WaveformTone::WaveformType> WaveformTone::waveForms;
std::vector<WaveformTone::InstrumentInfo> WaveformTone::instrumentInfos;
//---------------------------------------


void WaveformTone::MapGMInstrument(int& instrumentID)
{
	//The instruments not sampled are mapped to sampled ones.
	int map[128] = 
	{
		0,		//0Acoustic Grand Piano   大钢琴（声学钢琴）
		1,		//1 Bright AcousticPiano  明亮的钢琴
		2,		//2 Electric GrandPiano   电钢琴
		3,		//3 Honky - tonkPiano     酒吧钢琴
		4,		//4 RhodesPiano           柔和的电钢琴
		5,		//5 ChorusedPiano         加合唱效果的电钢琴
		5,		//6Harpsichord            羽管键琴（拨弦古钢琴）
		5,		//7Clavichord			  科拉维科特琴（击弦古钢琴）
				//
				//色彩打击乐器
		10,		//8Celesta                 钢片琴
		9,		//9Glockenspiel            钟琴
		10,		//10 Musicbox              八音盒
		11,		//11Vibraphone             颤音琴
		12,		//12Marimba                马林巴
		12,		//13Xylophone              木琴
		14,		//14 TubularBells          管钟
		46,		//15Dulcimer               大扬琴
				//
				//风琴
		16,		//16 HammondOrgan          击杆风琴
		17,		//17 PercussiveOrgan       打击式风琴
		17,		//18 RockOrgan             摇滚风琴
		17,		//19 ChurchOrgan           教堂风琴
		21,		//20 ReedOrgan             簧管风琴
		21,		//21Accordian              手风琴
		22,		//22Harmonica              口琴
		21,		//23 TangoAccordian        探戈手风琴
				//
				//吉他
		24,		//24 Acoustic Guitar(nylon) 尼龙弦吉他
		25,		//25 Acoustic Guitar(steel) 钢弦吉他
		25,		//26 Electric Guitar(jazz)  爵士电吉他
		25,		//27 Electric Guitar(clean) 清音电吉他
		28,		//28 Electric Guitar(muted) 闷音电吉他
		29,		//29 OverdrivenGuitar      加驱动效果的电吉他
		30,		//30 DistortionGuitar      加失真效果的电吉他
		31,		//31 GuitarHarmonics       吉他和音
				//
				//贝司
		32,		//32 AcousticBass          大贝司（声学贝司）
		33,		//33 ElectricBass(finger)  电贝司（指弹）
		34,		//34 Electric Bass(pick)   电贝司（拨片）
		35,		//35 FretlessBass          无品贝司
		36,		//36 Slap Bass1            掌击Bass 1
		36,		//37 Slap Bass2            掌击Bass 2
		34,		//38 Synth Bass1           电子合成Bass 1
		34,		//39 Synth Bass2           电子合成Bass 2
				//
				//弦乐
		40,		//40Violin                 小提琴
		40,		//41Viola                  中提琴
		42,		//42Cello                  大提琴
		42,		//43Contrabass             低音大提琴
		45,		//44 TremoloStrings        弦乐群颤音音色
		45,		//45 PizzicatoStrings      弦乐群拨弦音色
		46,		//46 OrchestralHarp        竖琴
		46,		//47Timpani                定音鼓
				//
				//合奏 / 合唱
		48,		//48 String Ensemble1      弦乐合奏音色1
		48,		//49 String Ensemble2      弦乐合奏音色2
		48,		//50 Synth Strings1        合成弦乐合奏音色1
		48,		//51 Synth Strings2        合成弦乐合奏音色2
		52,		//52 ChoirAahs             人声合唱“啊”
		53,		//53 VoiceOohs             人声“嘟”
		53,		//54 SynthVoice            合成人声
		55,		//55 OrchestraHit          管弦乐敲击齐奏
				//
				//铜管
		56,		//56Trumpet                小号
		56,		//57Trombone               长号
		56,		//58Tuba                   大号
		56,		//59 MutedTrumpet          加弱音器小号
		56,		//60 FrenchHorn            法国号（圆号）
		61,		//61 Brass Section		   铜管组（铜管乐器合奏音色）
		61,		//62 Synth Brass1          合成铜管音色1
		61,		//63 Synth Brass2          合成铜管音色2
				//簧管
		64,		//64 SopranoSax            高音萨克斯风
		65,		//65 AltoSax               次中音萨克斯风
		65,		//66 TenorSax              中音萨克斯风
		65,		//67 BaritoneSax           低音萨克斯风
		68,		//68Oboe                   双簧管
		68,		//69 EnglishHorn           英国管
		68,		//70Bassoon                巴松（大管）
		71,		//71Clarinet               单簧管（黑管）
				//
				//笛
		73,		//72Piccolo                短笛
		73,		//73Flute                  长笛
		73,		//74Recorder               竖笛
		75,		//75 PanFlute              排箫
		73,		//76 BottleBlow            吹瓶
		10,		//77Shakuhachi             日本尺八
		73,		//78Whistle                口哨声
		73,		//79Ocarina                奥卡雷那
				//
				//合成主音
		80,		//80 Lead 1(square)合成主音1（方波）
		81,		//81 Lead 2(sawtooth)合成主音2（锯齿波）
		36,		//82 Lead 3 (caliopelead)合成主音3
		36,		//83 Lead 4 (chifflead)合成主音4
		29,		//84 Lead 5(charang)合成主音5
		52,		//85 Lead 6(voice)合成主音6（人声）
		80,		//86 Lead 7(fifths)合成主音7（平行五度）
		36,		//87 Lead 8 (bass + lead)合成主音8（贝司加主音）
				//
				//合成音色
		53,		//88 Pad 1 (newage)合成音色1（新世纪）
		89,		//89 Pad 2(warm)合成音色2 （温暖）
		99,		//90 Pad 3(polysynth)合成音色3
		99,		//91 Pad 4(choir)合成音色4 （合唱）
		99,		//92 Pad 5(bowed)合成音色5
		99,		//93 Pad 6(metallic)合成音色6 （金属声）
		99,		//94 Pad 7(halo)合成音色7 （光环）
		99,		//95 Pad 8(sweep)合成音色8
				//
				//合成效果
		99,		//96 FX 1(rain)合成效果 1 雨声
		99,		//97 FX 2(soundtrack)合成效果 2 音轨
		99,		//98 FX 3(crystal)合成效果 3 水晶
		99,		//99 FX 4(atmosphere)合成效果 4 大气
		100,	//100 FX 5(brightness)合成效果 5 明亮
		99,		//101 FX 6(goblins)合成效果 6 鬼怪
		99,		//102 FX 7(echoes)合成效果 7 回声
		99,		//103 FX 8(sci - fi)  合成效果 8 科幻
				//
				//民间乐器
		12,		//104Sitar           西塔尔（印度）
		12,		//105Banjo           班卓琴（美洲）
		12,		//106Shamisen        三昧线（日本）
		46,		//107Koto            十三弦筝（日本）
		12,		//108Kalimba         卡林巴
		73,		//109Bagpipe         风笛
		40,		//110Fiddle          民族提琴
		40,		//111Shanai          山奈
				//
				//打击乐器
		10,		//112 TinkleBell                        叮当铃
		113,	//113Agogo[中文名称暂缺](某种打击乐器)
		114,	//114 SteelDrums                        钢鼓
		115,	//115Woodblock                          木鱼
		117,	//116 TaikoDrum                         太鼓
		117,	//117 MelodicTom                        通通鼓
		117,	//118 SynthDrum                         合成鼓
		10,		//119 ReverseCymbal                     铜钹
				//
				//Sound Effects 声音效果
		120,	//120 Guitar FretNoise                吉他换把杂音
		121,	//121 BreathNoise                     呼吸声
		10,		//122Seashore                         海浪声
		10,		//123 BirdTweet                       鸟鸣
		10,		//124 TelephoneRing                   电话铃
		10,		//125Helicopter                       直升机
		10,		//126Applause                         鼓掌声
		10		//127 Gunshot

	};

	instrumentID = map[instrumentID % 128];
}


bool WaveformTone::LoadWaveform(int bank, int instrumentID)
{
	//Waveform files are organized in folders and with folder names.
	//  Waveform			---- Main folder for all waveforms
	//		Bank0			---- Instruments are organized in banks because GM only has 128 instruments. Each bank has 128 instruments.
	//        0				---- The waveforms of instrument ID 0 stored here
	//          69_0_127[_1].PCM	---- The file name should be: pitch_pitchFrom_pitchTo.PCM or .WAV
	//        1				---- The waveforms of instrument ID 1 stored here
	//			......
	//		Bank1			---- Bank1
	//		......
	//		Bank512			---- Percussion bank 0
	//		   0			---- Percussion set 0
	//			 69_69_69_2.PCM
	//			 70_70_70_2.PCM
	//			  ......
	//		Bank513			---- Percussion bank 1
	//			......
	//
	//A pitch waveform file is a raw PCM file with 44.1kHz sampling frequency, 2-channel stereo, 16bit small endian interger.
	//The format of a pitch waveform sample file can also be RIFF .wav file with 44.1kHz sampling frequency, 16bit stereo.
	//
	//File name convention is:
	//    S_F_T_L.pcm/.wav
	//    S: the pitch of this sample file.
	//    F, T: from F pitch to T pitch, use this sample file.
	//    L: L part is optional. L == 1 means there is a loop part in the sample. L == 2 means always sustain(play the whole wave file without responding to NoteOff)

	//-------------------
	//Not a full GM bank.
	if (bank == 0)
	{
		//Map the unsampled instruments to sampled ones
		MapGMInstrument(instrumentID);
	}
	//-------------------
	
	//Do not load the same instrument twice
	for (auto& tone : waveForms)
	{
		if (tone.bank == bank && tone.instrumentID == instrumentID)
			return true;
	}
	try
	{
		char dir[_MAX_PATH];
		sprintf_s(dir, ".\\Waveform\\Bank%d\\%d\\*.*", bank, instrumentID);

		//Walk the directory and search for a proper sample file.
		//Matches pitch first. If no pitch matches perfectly, try matching a range.
		int namePitch{ -1 };
		int namePitchFrom{ -1 };
		int namePitchTo{ -1 };
		bool nameLoop{ false };
		bool nameAlwaysSutain{ false };
		_finddata_t dirInfo;
		long dirHandle = _findfirst(dir, &dirInfo);
		if (dirHandle == -1)
			throw 0;	//Error reading directory.
		do
		{
			if ((dirInfo.attrib & _A_SUBDIR) == 0 && (dirInfo.attrib & _A_SYSTEM) == 0)
			{
				//split the file name into 3 parts.
				int pos = 0;
				namePitch = atoi(dirInfo.name);
				while (dirInfo.name[pos] != '_' && dirInfo.name[pos] != NULL)
					pos++;
				if (dirInfo.name[pos] == '_')
				{
					pos++;
					namePitchFrom = atoi(dirInfo.name + pos);
				}
				while (dirInfo.name[pos] != '_' && dirInfo.name[pos] != NULL)
					pos++;
				if (dirInfo.name[pos] == '_')
				{
					pos++;
					namePitchTo = atoi(dirInfo.name + pos);
				}
				while (dirInfo.name[pos] != '_' && dirInfo.name[pos] != NULL)
					pos++;
				if (dirInfo.name[pos] == '_')
				{
					nameLoop = (dirInfo.name[pos + 1] == '1');
					nameAlwaysSutain = (dirInfo.name[pos + 1] == '2');
				}

				if (namePitch != 0)
				{
					sprintf_s(dir, ".\\Waveform\\Bank%d\\%d\\%s", bank, instrumentID, dirInfo.name);

					WaveformType waveForm{ bank,
											instrumentID,
											static_cast<double>(namePitch),
											static_cast<double>(namePitchFrom),
											static_cast<double>(namePitchTo),
											440 * pow(2, (namePitch - 69) / 12),
											nameLoop,
											nameAlwaysSutain,
											0,
											0,
											0,
											nullptr, nullptr
					};

					std::ifstream file;
					file.open(dir, std::ios::in | std::ios::binary);

					size_t length = 0;	//Length of wave data

					//see if it is an RIFF wave file
					RIFFHeader riffHeader{};
					WaveFormat waveFormat{};
					WaveDataHeader waveDataHeader{};
					file.read((char*)&riffHeader, sizeof(RIFFHeader));
					if (riffHeader.id == 0x46464952 && riffHeader.type == 0x45564157)	//id == "RIFF" and type == "WAVE"
					{
						//RIFF file.
						//Read wave format chunck.
						file.read((char*)&waveFormat, sizeof(WaveFormat));
						//id == "fmt ", audioFormat == PCM, sample rate should be 44100, 16 bits per sample, 2 channels stereo.
						//Other formats are not supported.
						if (waveFormat.id == 0x20746d66 && waveFormat.audioFormat == 1 && waveFormat.sampleRate == 44100 && waveFormat.bitsPerSample == 16)// && waveFormat.numChannels == 2)
						{
							//In case this chunk has a larger size than sizeof(WaveFormat).
							waveFormat.size -= sizeof(WaveFormat) - sizeof(waveFormat.id) - sizeof(waveFormat.size);
							if (waveFormat.size > 0)
								file.seekg(waveFormat.size, std::ios::cur);

							//Trying reading data header
							file.read((char*)&waveDataHeader, sizeof(WaveDataHeader));
							if (waveDataHeader.id == 0x61746164)	//id == "data"?
								length = waveDataHeader.size / (waveFormat.bitsPerSample / 8) / waveFormat.numChannels;
							else if (waveDataHeader.id == 0x74636166)	//If it's a "fact" chunk
							{
								if (waveDataHeader.size > 0)
									file.seekg(waveDataHeader.size, std::ios::cur);
								//Try again
								file.read((char*)&waveDataHeader, sizeof(WaveDataHeader));
								if (waveDataHeader.id == 0x61746164)
									length = waveDataHeader.size / (waveFormat.bitsPerSample / 8) / waveFormat.numChannels;
								else
									throw 0;	//Invalid format
							}
							else
								throw 0;	//Invalid format.
						}
					}
					else
					{
						//Not an RIFF wave file.
						//Interpret it as a raw PCM sample file.
						file.seekg(0, std::ios::end);
						length = static_cast<size_t>(file.tellg() / sizeof(int16_t) / 2);
						file.seekg(0, std::ios::beg);
					}

					waveForm.size = length;
					if (length > 0)
					{
						waveForm.leftChannel = new int16_t[length];
						waveForm.rightChannel = new int16_t[length];

						for (size_t i = 0; i < length; i++)
						{
							file.read((char*)&(waveForm.leftChannel[i]), sizeof(int16_t));
							if (waveFormat.numChannels == 2)
								file.read((char*)&(waveForm.rightChannel[i]), sizeof(int16_t));
							else
								waveForm.rightChannel[i] = waveForm.leftChannel[i];
						}
					}

					file.close();

					//If it is a loop, find out the start and end position of the loop
					if (waveForm.loop)
					{
						size_t pos = waveForm.size - 1;
						size_t posMin = pos;
						int16_t left = waveForm.leftChannel[pos];
						int cyclePointsCount = static_cast<int>(SAMPLE_RATE / waveForm.frequencyBase) * 2;
						//back to a lowest point
						while (cyclePointsCount > 0)
						{
							if (waveForm.leftChannel[pos] < left)
							{
								posMin = pos;
								left = waveForm.leftChannel[pos];
							}
							pos--;
							cyclePointsCount--;
						}
						pos = posMin;
						//then, still go back, find the nearest zero point
						while (pos > 0 && waveForm.leftChannel[pos] < 0)
						{
							pos--;
						}
						//Linear
						waveForm.loopEndAt = pos + static_cast<double>(waveForm.leftChannel[pos]) / (static_cast<double>(waveForm.leftChannel[pos]) - waveForm.leftChannel[pos + 1]);
						//back some cycles
						size_t spos = static_cast<size_t>(waveForm.loopEndAt - SAMPLE_RATE / waveForm.frequencyBase * 200);
						//find the precise start position
						//and, it measures the actual frequency
						if (waveForm.leftChannel[spos] > 0)
						{
							while (waveForm.leftChannel[spos] > 0)
								spos++;
							//Linear
							waveForm.loopStartAt = spos - static_cast<double>(-waveForm.leftChannel[spos]) / (-static_cast<double>(waveForm.leftChannel[spos]) + waveForm.leftChannel[spos - 1]);
						}
						else
						{
							while (spos > 0 && waveForm.leftChannel[spos] < 0)
								spos--;
							//Linear
							waveForm.loopStartAt = spos + static_cast<double>(waveForm.leftChannel[spos]) / (static_cast<double>(waveForm.leftChannel[spos]) - waveForm.leftChannel[spos + 1]);
						}
					}

					waveForms.push_back(waveForm);
				}
			}
		} while (_findnext(dirHandle, &dirInfo) == 0);
		_findclose(dirHandle);
	}
	catch (...)
	{
		return false;
	}

	return true;
}

void WaveformTone::FreeWaveforms()
{
	for (auto& item : waveForms)
	{
		delete[] item.leftChannel;
		delete[] item.rightChannel;

		item.leftChannel = item.rightChannel = nullptr;
	}
	waveForms.clear();
}

bool WaveformTone::TriggerPulse(double& gl, double& gr)
{
	if (portamentoEnable)
	{
		PortamentoAdjust();	//portamentoEnable will be set to false when done.
	}

	if (selectedWaveform == -1)
	{
		gl = gr = 0;
		return false;
	}
	else
	{
		double pos = frequencyRatio * toneSampleCount;

		//If I should loop
		if (waveForms[selectedWaveform].loop && pos >= waveForms[selectedWaveform].loopEndAt)
		{
			double span = pos - waveForms[selectedWaveform].loopEndAt;
			double timesOf = span / (waveForms[selectedWaveform].loopEndAt - waveForms[selectedWaveform].loopStartAt);
			double fractionPart = timesOf - static_cast<size_t>(timesOf);
			pos = waveForms[selectedWaveform].loopStartAt + (waveForms[selectedWaveform].loopEndAt - waveForms[selectedWaveform].loopStartAt) * fractionPart;
		}

		size_t linearPos = static_cast<size_t>(pos);
		if (linearPos >= waveForms[selectedWaveform].size - 1)
		{
			return false;
		}
		else
		{
			//With linear interpolation between two sample values.
			double linear = pos - linearPos;	//This should be in 0 - 1
			gl = waveForms[selectedWaveform].leftChannel[linearPos] * (1 - linear) + waveForms[selectedWaveform].leftChannel[linearPos + 1] * linear;
			gr = waveForms[selectedWaveform].rightChannel[linearPos] * (1 - linear) + waveForms[selectedWaveform].rightChannel[linearPos + 1] * linear;
			toneSampleCount++;

			if (soft)
			{
				gl *= 0.5;
				gr *= 0.5;
			}
			if (!autoStereo)
			{
//				gl = (1 - (pitch / 127 * 0.6 + 0.2)) * gl;
//				gr = (pitch / 127 * 0.6 + 0.2) * gr;
//				gl = gr = (gl + gr) / 2;
			}
			//Normalize to 60%
			gl *= 0.6 * GetVelocity() / 127.0;
			gr *= 0.6 * GetVelocity() / 127.0;

			if (releaseVelocity >= 0 && !GetSustain())
			{
				gl *= static_cast<double>(evlpSampleCount) / releaseVelocity;
				gr *= static_cast<double>(evlpSampleCount) / releaseVelocity;
				evlpSampleCount--;
				if (evlpSampleCount == 0)
					selectedWaveform = -1;
			}

			return true;
		}
	}
}

void WaveformTone::ReleaseKey(int velocity)
{
	if (selectedWaveform != -1 && !waveForms[selectedWaveform].alwaysSustain)
		Tone::ReleaseKey(velocity);
}

void WaveformTone::ReCalibrateFrequency()
{
	if (selectedWaveform != -1)
	{
		double frequencyRatioSave = frequencyRatio;	//Old ratio
		SetFrequency();
		frequencyRatio = frequency / waveForms[selectedWaveform].frequencyBase;	//New ratio

		//Adjust toneSampleCount so that the change in frequency does not affect the wave alignments.
		toneSampleCount = frequencyRatioSave * toneSampleCount / frequencyRatio;
	}
}


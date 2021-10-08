# SimpleSynthesizer
An experimental synthesizer.

This is a waveform synthesizer.
The core synthesizer codes are in SimpleSynthesizer folder. 
There is a GUI shell for the core, which can be used to play a MIDI file and debug the core. It runs under Windows 7/8/10.

All projects and files are compiled with Visual Studio 2019 in Windows 10.
The core synthesizer codes are written in standard C++ and is to be compiled for Linux/macOS hopefully.

The core consists of:
1) MIDI file reader. Read a MIDI file, parse the data and store the data into a data structure.
2) Tone generators. Including a mathematical wave generator and a waveform tone generator.
3) Effects processor. Including chorus, echo and reverb generators.
3) MIDI playback.

MIDI commands are not all implemented but the most important events and control commands are included in this version.

The sample waveforms of each instruments should be placed in the Release folder for the core to load them.
Please read /SimpleSynthesizer/WaveformTone.cpp for the details of a waveform folder.

There is a waveform data bank in the V0.2 release. You can use these data to debug and try SimpleSynthesizer. Please download the release and unpack the Waveform folder to your Release folder inside the solution folder.


# Introducing GEM S2 Turbo VST3

This is an open development project to recreate the sound-making workflow of the early-1990s GEM S2 Turbo as a modern VST3 instrument. The aim is not to present a finished emulator. It is to make the reconstruction visible, testable and open to contributions from synthesizer owners, sound designers and developers.

The current build has an experimental two-layer sound engine, two serial filters, an amplitude envelope, basic LFO and MIDI wheel response, reverb, chorus, stereo and dry output routing, and a hardware-inspired interface. It can load one external audio sample. The SysEx and floppy-image tools currently inspect or extract data; they do not convert a complete GEM bank into playable presets.

The main work ahead is understanding native Sound, Performance and Waveset data; mapping samples and envelopes accurately; validating MIDI and audio behavior in DAWs; and comparing the reconstruction against a real S2 Turbo. Pull requests, format research, listening tests and UI feedback are welcome. Please share only material you have permission to redistribute, and label conclusions as documented facts, measurements or hypotheses.

This project is independent and unaffiliated with General Music/GEM. It contains no original firmware, ROMs, factory sample libraries, manuals or commercial sound banks. The source code is released under the MIT License; JUCE is a separate dependency under its own license terms.


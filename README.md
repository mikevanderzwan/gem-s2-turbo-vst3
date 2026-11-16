# GEM S2 Turbo VST3

An independent, open-source reconstruction project inspired by the documented sound architecture and hands-on workflow of the General Music (GEM) S2 Turbo. The goal is to build a useful modern VST3 instrument, understand the original format where evidence allows, and make the work easy for other developers and S2/S3 owners to inspect and improve.

This is an experimental work in progress. It is not an official GEM product, does not contain original firmware or factory sound data, and is not yet a faithful hardware emulator. Native GEM sound-bank compatibility is still a research goal.

## What is here today

- A JUCE-based Windows VST3 with a 32-voice development engine.
- Two playable layers with level, transpose and detune controls. The built-in source is a simple sine approximation.
- One imported WAV/AIFF/FLAC sample source, shared across the layers. This is not a Waveset or multisample-bank loader.
- Two serial filter stages with low-pass, high-pass, band-pass, and approximate parametric cut/boost choices.
- An amplitude ADSR, pitch LFO, MIDI pitch and modulation wheels, pan, master level, reverb and chorus controls.
- A stereo wet output and optional mono dry outputs, with layer A/B routing controls.
- A hardware-inspired editor with a signal-flow display, sound controls, performance sliders and an on-screen keyboard.
- File utilities: sample loading; extraction of recognized GEM SysEx DATA_DUMP packets; and read-only inspection of a 1.44 MB FAT12 disk image.

The SysEx utility extracts recognized packets; it does not decode them into playable GEM programs. Disk-image inspection reports directory and file information; it does not import a complete native bank.

## What needs contributors

- A tested native Sound / Performance / Waveset bank decoder with fixtures that contributors are allowed to share.
- Multisample zones, key and velocity ranges, loops, and correct sample mapping.
- The original envelope, modulation, voice allocation, multi-part, and effects behavior, based on documented or measured evidence.
- DAW testing of MIDI input, audible output, state recall, routing, and the editor at different host sizes.
- Better reference captures and notes that separate documented behavior from educated guesses.

Please open an issue before starting a large parser or synthesis change so the file formats and parameter IDs can stay compatible.

## Build on Windows

Requirements:

- Windows 10 or later, x64.
- Visual Studio 2022 with the Desktop development with C++ workload and a Windows SDK.
- CMake 3.22 or later.
- A local JUCE 9.x checkout. JUCE is a separate dependency and is not included here.

Configure and build from PowerShell, replacing the JUCE path with your local checkout:

```powershell
cmake -S . -B build -DJUCE_PATH="C:/path/to/JUCE" -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target GemS2TurboVST_VST3
```

The VST3 bundle is written under:

```text
build/GemS2TurboVST_artefacts/Release/VST3/GEM S2 Turbo.vst3
```

Add the bundle to a VST3 scan location and rescan it in your DAW. Build and host validation are separate: the project is still experimental, and reports of silent or host-specific behavior should include the DAW, version, operating system, MIDI setup, and audio routing.

## Reference material and data

The reconstruction notes in this repository summarize the project team's observations and known gaps. The `jmechnich/s3turbo` project is a separate public reference for S2/S3 SysEx and disk-image research; none of its source files are bundled here.

Do not commit GEM factory samples, ROMs, firmware, commercial sound banks, service manuals, demo disks, or dumps containing data you do not have permission to redistribute. Small, newly created synthetic test fixtures are preferred. Keep notes explicit about whether a claim comes from a manual, a dump, a measurement, or an inference.

## Contributing

Bug reports, format research, sound comparisons, UI feedback and code contributions are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md). Please keep submissions focused, document how changes were verified, and avoid claiming exact hardware behavior without a comparison.

## License and naming

The original code in this repository is released under the MIT License; see [LICENSE](LICENSE). JUCE remains a separate dependency under its own terms. GEM and S2 Turbo are names of the original product and are used here only to identify the instrument this independent project studies.


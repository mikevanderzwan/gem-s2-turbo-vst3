# Contributing

Thanks for helping reconstruct the GEM S2 Turbo as an independent VST3 project.

## Before opening a pull request

- Open an issue first for large changes to parsing, parameter IDs, or synthesis behavior.
- Keep original GEM firmware, ROMs, factory samples, commercial banks, manuals and demo disks out of commits.
- Only add sample or SysEx fixtures that you created or are clearly permitted to redistribute.
- Distinguish manual-documented facts, observations from a real instrument, and hypotheses in code comments or notes.
- Preserve existing parameter IDs and state compatibility when changing the engine.

## Reports and comparisons

For audio or host bugs, include the DAW and version, Windows version, sample rate, buffer size, MIDI routing, and audio output routing. For sound comparisons, note the patch setup and control values, and state whether the reference is hardware or another recording.

## Building

Follow the Windows build steps in the README. Pull requests should include a concise build result and describe any manual host checks. Do not commit generated build directories or VST3 binaries.


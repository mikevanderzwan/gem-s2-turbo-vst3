# GEM S2 Turbo: reconstruction notes

This is an independent reconstruction plan based on the supplied owner's manual and service manual. It is not GEM firmware or a dump of copyrighted factory waveforms.

## Documented sound path

The S2 is a sample-playback instrument. A Sound selects a Waveset / waveform (multisample), then applies pitch and modulation, two serial digital state-variable filters, amplitude, stereo pan and effects. Each filter is two-pole / 12 dB per octave. With both in series the combined slope is four-pole / 24 dB per octave. Turbo documentation specifies up to 32 voices, 16 multitimbral parts, ROM and RAM waveforms, two real-time effects processors, dynamic allocation, and polyphonic aftertouch.

The service schematics identify a central sound processor, separate filter circuitry, converters, analog output stages and effect outputs. These establish the hardware signal categories, but do not disclose the exact DSP algorithms or source code. Therefore an emulation can reproduce documented behavior and measured response, not claim bit-exact hardware identity without comparison recordings and waveform data.

The supplied hardware note identifies 18-bit DACs (including AD1865N) feeding a wet stereo Left/Right output plus four assignable dry sub-outs (Out 1–4). The plugin now exposes a stereo wet bus and four optional mono dry buses; the two layer taps can be assigned to any of those four buses. This models the documented routing topology, not DAC nonlinearities or the original DSP's exact wet/dry tap point.

The supplied internal-board photograph visibly shows a Motorola MC68302 and four VLSI-marked large DSP devices on the main board, consistent with the service diagram's dedicated sound processor/filter/effects sections. Chip identities beyond the visible markings should be confirmed against the service schematic/part list; the photo alone does not establish DSP instruction sets or algorithms.

## Sound model to implement

- Waveset with multisample zones, root key, key/velocity ranges, loop points and sample interpolation.
- Pitch stage with transpose, tuning, detune, pitch envelope, LFO and controller modulation.
- Filter 1 then Filter 2 in series; each offers low-pass, high-pass, band-pass, parametric boost and parametric cut. Each has cutoff, resonance, envelope amount, keyboard tracking and modulation inputs.
- Separate pitch, filter, amplitude and pan envelopes. Envelopes are multi-segment with key-on/key-off phases and optional loop behavior where supported by the Sound data.
- Amplitude, dynamic stereo pan, velocity/key tracking and aftertouch modulation.
- Two effect units with independent library/program selection and send/return levels. Avoid asserting exact effect algorithms until the effect parameter tables are transcribed.
- 32-voice allocation and 16-part MIDI operation, with per-part bank/program, channel, level, pan, key/velocity range and priority.

## Compatibility boundary

MIDI Bank Select CC0 plus Program Change can be implemented in the S-series 1–16 bank / 1–128 program convention (translate the host's zero-based values). The manuals describe native disk libraries, Waveform import formats and SysEx/file dumps. That is enough to design a parser, but not enough to promise exact sound-patch compatibility until the SysEx packet layout, checksums, packed parameter fields and Turbo-specific extensions have been transcribed and validated against actual dumps. Audio waveform imports (WAV and documented legacy formats) are distinct from native sound-bank parsing.

The first release should provide its own documented preset format and an importer that reports unsupported native fields instead of silently discarding them. Native bank conversion is a separate milestone.

## Interface changes from the first sketch

- Replace the incorrect generic oscillator framing with **Waveset / Multisample** and two sample layers.
- Draw the serial signal path explicitly: **Waveform → Filter 1 → Filter 2 → Amp → Pan**.
- Label each filter as 2-pole / 12 dB per octave; show the 4-pole result only when both serial filters are active.
- Show separate multi-segment pitch, filter, amplitude and pan envelope editors, rather than only an ADSR.
- Present the two DSP effect processors separately, with library/program selection.
- Add Sound bank/program selection, Performance and Multi views, 16-part status, voice allocation, two wheels, seven assignable sliders and poly-aftertouch.

## Evidence vs inference

The signal chain, filter types/slopes, multitimbrality, voice ceiling, controllers, waveform/RAM concept and two effect processors are documented. The modern tab layout, exact control grouping, colors and editable modulation matrix are interface decisions. Any recreated sound character and undocumented DSP response remain approximations until measured against hardware.

## Current implementation status

The current source builds a VST3 with a 32-voice JUCE synth, two basic layers, two serial state-variable filter stages, an amplitude ADSR, a pitch LFO, MIDI pitch/mod wheel handling, global pan, reverb and chorus. WAV/AIFF loading uses one external audio sample across notes; this is not a native Waveset or multisample loader. Filter choices include low-pass, high-pass, band-pass, and approximate cut/boost responses. The plugin declares a wet stereo output and four optional mono dry buses with Layer A/B routing selectors.

The GEM SysEx utility scans `F0 2F 5C 0D` DATA_DUMP packets, checks the XOR checksum, and extracts valid packet bytes to a sibling `.gemdump` file. It does not decode the packets into playable Sound records or presets. The FAT12 disk-image utility reports directories and selected S-series file information; it does not import a complete native sound bank.

The 0.6 editor uses a hardware-inspired single sound page: layers, serial filter controls, amplitude ADSR, two effects, LFO, output routing, seven parameter-linked performance sliders, pitch/mod controls, and an on-screen keyboard. Its waveform panel is an illustrative sine graphic, not a display of GEM ROM or loaded sample data. Multi-part performances, native program selection, independent pitch/filter/pan envelopes, polyphonic aftertouch, accurate Waveset zones, and original effect algorithms are not implemented.

The VST3 target compiles, but playback and interface behavior have not yet been confirmed in a DAW. Treat host playback as unresolved until verified on a test system. This is an experimental approximation, not a faithful or bit-exact hardware emulator.


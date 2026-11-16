# DemoDisk01 analysis

Source: user-provided `DemoDisk.img` (1,474,560 bytes). The image is a standard 1.44 MB FAT12 floppy image. It was read-only parsed and its files were extracted to a working copy; the original image was not modified.

## Inventory

- 10 banks under `BANKS`, with 61 performance files and 10 song files.
- `SETUP/SOUNDS`: 13 S-series Sound records.
- `SETUP/SAMPLES`: 13 sample files: BIRDS, BOW_GT, COPTER, CUCU, HANDCLAP, JAM_1, LAUGHT, PHONE, RADIO_F3, RAIN, SEA, THUNDER and VOICEEND.
- Setup records: GENERAL, EFFECT1, EFFECT2 and SOUNDMAP.
- Total extracted files: 105.

## Format observations

- The 13 sample files start with `01 01 01 18 02 01`; subsequent bytes are GEM-specific sample data, not a standard RIFF/WAV stream. Current plugin WAV/AIFF loading cannot directly read `.SAM` yet.
- Sound files start with `01 02 01 01 02 01`. The first example then carries a variable length and a sound name before opaque parameter bytes. That header is useful to identify records, but the complete field map still needs reverse-engineering.
- SOUNDMAP starts with `01 02 02 09 02 02` and consists of 18-byte records. The public s3turbo notes identify name, layer/source flag, bank, program, date and time fields for that record type.
- EFFECT1 starts with `01 02 02 06 02 01`; its 34-byte map entries include names, effect index/type and parameter words. EFFECT2 uses its own table.
- Performance files vary in size from 380 bytes upward and begin `01 02 01 02 02 02`; Songs are much larger. The reference project labels the Performance payload layout incomplete, so this demo disk is valuable reverse-engineering evidence but does not by itself establish every field meaning.

## Import/reconstruction consequence

The disk supplies sound records, sound-map indices and original samples together, enabling cross-reference work by name and sample family. Next importer work should:

1. Parse FAT12 directories and cluster chains from `.img` files.
2. Decode `.SAM` sample framing/encoding into playable PCM and preserve loop/root-note metadata where present.
3. Parse 18-byte Soundmap entries to build bank/program/name lookup.
4. Differentially map Sound and Performance parameter bytes against the 13 example sounds and their referenced sample data.

The current VST3 importer does not yet perform those four steps. Its existing WAV loader and SysEx packet validator are preliminary paths only.

## Test-build update

The current VST3 test build includes a FAT12 image inspector wired to the plugin's Load button. Select the `.img` file to validate the 1.44 MB FAT12 geometry and display counts for banks, performances, songs, Sound files and samples, plus the first Soundmap name/bank/program entries. The refreshed UI lays out sample layers, serial Filter 1/2, envelope/modulation, reverb, pan and the S2 key range. This is a read-only metadata import: the plugin does not modify the disk image, extract or decode `.SAM` sample payloads into voices, or instantiate Sound/Performance settings yet.


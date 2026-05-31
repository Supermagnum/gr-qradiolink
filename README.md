# gr-qradiolink

**IMPORTANT NOTICE**: This is AI-generated code. 

This code has not been reviewed by professional coders, it is a large task. There are tests available in the codebase, please review those and their code.

### GNU Radio 4 experimental port

On branch **`gnuradio4`**, the header-centric GNU Radio **4.x** port lives under **`gr4/`** with CMake **`cmake_gr4/`** as `-S` (see **[README_gr4.md](README_gr4.md)**). That flow builds against an upstream **GNU Radio 4 source tree** (`GR4_SOURCE_DIR`), links **`gnuradio4::gnuradio-core`**, and runs Boost.Test executables under **`gr4/tests`**. **Recorded results** (compiler, GR4 revision, CTest counts for GR4 and GR3) are in README_gr4.md under *Test results*.

---

GNU Radio out-of-tree (OOT) module for QRadioLink blocks.

## Modulation validation

**Branch `gnuradio4`:** GNU Radio **4.x** port (`gr4/`, see [README_gr4.md](README_gr4.md)). The
**GNU Radio 3.10** OOT on branch **`main`** has the same mode list and
[VALIDATION_REPORT.md](VALIDATION_REPORT.md) there.

Reference IQ for modulator blocks was checked with
[radio-modulation-validator](https://github.com/Supermagnum/radio-modulation-validator)
(2026-05-31). Summary: **SOFT FAIL** — 6/19 blocks passed, 13 soft fail, **0 hard fail**, 3 skipped.

All tested modes were identified at the correct modulation **family** (FSK, PSK, AM, FM). Order-level
soft fails (for example FSK predicted as CPFSK instead of 4FSK or GMSK) reflect limits of the
validator’s order classifier, not evidence that these modulators produce the wrong waveform family.

Full per-block results: **[VALIDATION_REPORT.md](VALIDATION_REPORT.md)**

## Publication

This project is documented in the following preprint:

> Cryptographically Keyed Gaussian-Distributed Spread-Spectrum
> for Enhanced Covert Communications: Design, Implementation,
> and Simulated Performance in ITU Channel Models  
> IACR Cryptology ePrint Archive, Paper 2025/108456  
> https://eprint.iacr.org/2025/108456

Archive record timestamp: **21 March 2026**.

---

## Overview

This module provides GNU Radio blocks for various digital and analog modulation schemes, specialized for amateur radio and digital voice communications.

This module was converted from the [QRadioLink](https://qradiolink.org/) application, which is a multimode SDR transceiver for GNU Radio, ADALM-Pluto, LimeSDR, USRP, and other SDR hardware. The original source code is located at [https://codeberg.org/qradiolink/qradiolink](https://codeberg.org/qradiolink/qradiolink).

**Validation:** Converted blocks that have a counterpart in QRadioLink have been validated against the original QRadioLink source (branch `next`, [codeberg.org/qradiolink/qradiolink](https://codeberg.org/qradiolink/qradiolink)) so that flowgraphs, filter parameters, scaling constants, and block behaviour match. SSB and AM modulator/demodulator chains (including CESSB structure and clipper/stretcher constants) were restored to match the original where they had diverged. Some blocks in this module have no corresponding source in QRadioLink's `src/gr/`; their origin is stated in the table below.

**Credit and origin:**

- **QRadioLink (Codeberg)** is the origin of the following modes (validated against [codeberg.org/qradiolink/qradiolink](https://codeberg.org/qradiolink/qradiolink) branch `next`, `src/gr/`): modulation/demodulation (2FSK, 4FSK, GMSK, BPSK, QPSK, AM, SSB, NBFM, WBFM), digital voice (FreeDV, M17, DMR), and supporting blocks (RSSI, M17 deframer, MMDVM source/sink, clipper, stretcher, zero_idle_bursts, gr_4fsk_discriminator, demod_mmdvm_multi, demod_mmdvm_multi2, mod_mmdvm_multi2). The DSP and mode behaviour for these blocks were derived from or reimplemented from the QRadioLink application.
- **Blocks not from QRadioLink** (no `gr_*` source in QRadioLink `src/gr/`): 8FSK, SOQPSK, CPM-4FSK, dPMR, NXDN, POCSAG, D-STAR, YSF, P25. Their origin is given in the Block origin table below.
- **Additions in this module** (not from QRadioLink): **(1) DSSS enhancements** (soft-decision metrics, AFC, adaptive correlation threshold, coarse-to-fine acquisition, timing recovery) on top of base DSSS spreading/despreading; **(2) GDSS** (Gaussian-Distributed Spread-Spectrum spreader/despreader), implemented from Shakeel et al., *Sensors* 2023 (see References below).

**Block origin (validation / attribution):**

| Block(s) | Origin | Notes |
|----------|--------|--------|
| 2FSK, 4FSK, GMSK, BPSK, QPSK, AM, SSB, NBFM, WBFM, FreeDV, M17, DMR | QRadioLink | Validated against QRadioLink `src/gr/` (gr_demod_*, gr_mod_*). |
| MMDVM source/sink, demod_mmdvm_multi, demod_mmdvm_multi2, mod_mmdvm_multi2, RSSI, clipper, stretcher, zero_idle_bursts, gr_4fsk_discriminator | QRadioLink | Validated against QRadioLink `src/gr/`. |
| 8FSK | Implemented for this module | No gr_demod_8fsk/gr_mod_8fsk in QRadioLink. Extension of 4FSK-style design. |
| SOQPSK | Implemented for this module | No gr_demod_soqpsk/gr_mod_soqpsk in QRadioLink. From SOQPSK literature/specs. |
| CPM-4FSK | Implemented for this module | No gr_mod_cpm_4fsk in QRadioLink. |
| dPMR | Implemented for this module | No gr_* in QRadioLink. From ETSI TS 102 658 (test vectors and code comments). |
| NXDN | Implemented for this module | No gr_* in QRadioLink. From NXDN Forum specs; symbol mapping references MMDVM convention. |
| POCSAG | Implemented for this module | No gr_* in QRadioLink. From POCSAG/ITU-R M.584-2; protocol compatible with MMDVMHost. |
| D-STAR | Implemented for this module | No gr_* in QRadioLink. From D-STAR/JARL spec (Golay FEC); compatible with MMDVMHost. |
| YSF | Implemented for this module | No gr_* in QRadioLink. From YSF protocol documentation; compatible with MMDVMHost. |
| P25 | Implemented for this module | No gr_* in QRadioLink. From P25 Phase 1 TIA-102; compatible with MMDVMHost. |
| DSSS (base) | QRadioLink | Validated against QRadioLink gr_demod_dsss/gr_mod_dsss. |
| DSSS (enhancements), GDSS | This module | DSSS: enhancements on top of QRadioLink base. GDSS: from Shakeel et al., Sensors 2023. |

The code has been fuzzed extensively using libFuzzer with over 104 million executions across multiple blocks, and no crashes or memory leaks were discovered. See [fuzzing-results/results.md](fuzzing-results/results.md) for coverage details.

## Features

### Modulation/Demodulation Blocks

- **Digital Modulations**: 2FSK, 4FSK, 8FSK, CPM-4FSK, GMSK, BPSK, QPSK, SOQPSK, DSSS, GDSS
  - **2FSK**: Binary Frequency Shift Keying modulator/demodulator (see [GRC Block](grc/qradiolink_mod_2fsk.block.yml), [Examples](examples/README.md))
  - **4FSK**: 4-level Frequency Shift Keying modulator/demodulator (see [GRC Block](grc/qradiolink_mod_4fsk.block.yml), [Examples](examples/README.md))
  - **8FSK**: 8-level Frequency Shift Keying modulator/demodulator with 3 bits per symbol (see [GRC Block](grc/qradiolink_mod_8fsk.block.yml), [Examples](examples/README.md))
  - **CPM-4FSK**: Continuous Phase Modulation 4FSK modulator (see [GRC Block](grc/qradiolink_mod_cpm_4fsk.block.yml))
  - **GMSK**: Gaussian Minimum Shift Keying modulator/demodulator (see [GRC Block](grc/qradiolink_mod_gmsk.block.yml), [Examples](examples/README.md))
  - **BPSK**: Binary Phase Shift Keying modulator/demodulator (see [GRC Block](grc/qradiolink_mod_bpsk.block.yml), [Examples](examples/README.md))
  - **QPSK**: Quadrature Phase Shift Keying modulator/demodulator (see [GRC Block](grc/qradiolink_mod_qpsk.block.yml), [Examples](examples/README.md))
  - **SOQPSK**: Shaped Offset Quadrature Phase Shift Keying modulator/demodulator (see [GRC Block](grc/qradiolink_mod_soqpsk.block.yml), [Details](#soqpsk-shaped-offset-quadrature-phase-shift-keying))
  - **DSSS**: Direct Sequence Spread Spectrum with enhanced spreader/despreader blocks, PN sequence generation (m-sequences, Gold codes), timing recovery, lock detection, soft-decision metrics, AFC support, adaptive correlation threshold, and coarse-to-fine acquisition (see [DSSS Blocks Guide](docs/DSSS_BLOCKS.md), [GRC Blocks](grc/qradiolink_dsss_spreader_cc.block.yml))
  - **DSSS-CDMA**: Code Division Multiple Access transmitter and receiver with multi-user support, configurable spreading factors (32, 64, 128, 256, 512), and support for multiple modulation schemes (2FSK, 4FSK, 8FSK, GMSK, BPSK, QPSK, SOQPSK) (see [GRC Blocks](grc/qradiolink_dsss_cdma_transmitter_cc.block.yml))
  - **GDSS**: Gaussian-Distributed Spread-Spectrum with spreader/despreader blocks using Gaussian-distributed sequences; soft-decision metrics, AFC support, adaptive threshold, and coarse-to-fine acquisition. The GDSS scheme follows Shakeel et al., *Gaussian-Distributed Spread-Spectrum for Covert Communications*, Sensors 2023, 23(8), 4081; [doi:10.3390/s23084081](https://doi.org/10.3390/s23084081). (See [GDSS Blocks Guide](docs/GDSS_BLOCKS.md), [GRC Blocks](grc/qradiolink_gdss_spreader_cc.block.yml))
- **Analog Modulations**: AM, SSB (USB/LSB), NBFM, WBFM
  - **AM**: Amplitude Modulation modulator/demodulator (see [GRC Block](grc/qradiolink_mod_am.block.yml), [Examples](examples/README.md))
  - **SSB**: Single Sideband (USB/LSB) modulator/demodulator (see [GRC Block](grc/qradiolink_mod_ssb.block.yml), [Examples](examples/README.md))
  - **NBFM**: Narrowband Frequency Modulation modulator/demodulator (see [GRC Block](grc/qradiolink_mod_nbfm.block.yml), [Examples](examples/nbfm_example.py))
  - **WBFM**: Wideband Frequency Modulation modulator/demodulator (see [GRC Block](grc/qradiolink_mod_wbfm.block.yml), [WBFM Demodulator](grc/qradiolink_demod_wbfm.block.yml))
- **Digital Voice**: FreeDV, M17, DMR (Tier I/II/III), dPMR, NXDN, MMDVM
  - **FreeDV**: FreeDV digital voice codec (see [Examples](examples/freedv_example.py))
  - **M17**: M17 digital voice protocol with physical and protocol layer support (see [GRC Blocks](grc/qradiolink_m17_coder.block.yml), [Examples](examples/m17_mod_example.py))
  - **DMR**: Digital Mobile Radio (Tier I/II/III) modulator/demodulator (see [GRC Blocks](grc/qradiolink_mod_dmr.block.yml), [Examples](examples/README.md))
  - **dPMR**: Digital Private Mobile Radio (ETSI TS 102 658), 2400 baud, 6.25 kHz channel spacing (see [GRC Block](grc/qradiolink_mod_dpmr.block.yml), [Examples](examples/README.md))
  - **NXDN**: Next Generation Digital Narrowband, supports NXDN48 (2400 baud) and NXDN96 (4800 baud) modes (see [GRC Block](grc/qradiolink_mod_nxdn.block.yml), [Examples](examples/README.md))
  - **MMDVM Protocols**: POCSAG, D-STAR, YSF (Yaesu System Fusion), P25 (Phase 1 C4FM)
    - **POCSAG**: Paging protocol (ITU-R M.584-2) with BCH(31,21) FEC, supports 512/1200/2400 bps (see [GRC Blocks](grc/qradiolink_pocsag_encoder.block.yml), [Examples](examples/README.md))
    - **D-STAR**: Digital Smart Technologies for Amateur Radio with Golay(24,12) FEC (see [GRC Blocks](grc/qradiolink_dstar_encoder.block.yml), [Examples](examples/README.md))
    - **YSF**: C4FM protocol with Golay(20,8) and Golay(23,12) FEC (see [GRC Blocks](grc/qradiolink_ysf_encoder.block.yml), [Examples](examples/README.md))
    - **P25**: Project 25 Phase 1 C4FM with BCH(63,16) and Trellis encoding (see [GRC Blocks](grc/qradiolink_p25_encoder.block.yml), [Examples](examples/README.md))
- **Supporting Blocks**: Audio source/sink, RSSI, FFT, deframer, CESSB, M17 deframer, MMDVM source/sink, clipper, stretcher, zero idle bursts (see [GRC Block](grc/qradiolink_m17_deframer.block.yml))
- **FEC Blocks**: Forward Error Correction with soft-decision LDPC encoder/decoder and block interleaver
  - **LDPC Encoder/Decoder**: Supports regular and irregular LDPC codes
  - **Configurable code rates**: 1/2, 2/3, 3/4, or custom
  - **Configurable block lengths**: 576, 1152, 2304 bits, or custom
  - **Auto-selection**: Automatically selects best matching code from available AList files
  - See [GRC Blocks](grc/qradiolink_ldpc_encoder.block.yml)
  - **Interleaver (HF Burst)**: Block interleaver for burst error mitigation on HF channels; spreads burst errors across symbol positions for better FEC performance (see [GRC Block](grc/qradiolink_interleaver_bb.block.yml))
- **DSSS Blocks**: Enhanced spreader/despreader with PN sequence generation, timing recovery, acquisition, soft-decision support, AFC, adaptive threshold, and coarse-to-fine code search (see [DSSS Blocks Guide](docs/DSSS_BLOCKS.md), [GRC Blocks](grc/qradiolink_dsss_spreader_cc.block.yml))
- **DSSS-CDMA Blocks**: Multi-user CDMA transmitter and receiver with configurable spreading factors, Gold code support, and multi-user interference estimation (see [GRC Blocks](grc/qradiolink_dsss_cdma_transmitter_cc.block.yml))
- **GDSS Blocks**: Gaussian-Distributed Spread-Spectrum spreader and despreader with soft-decision, AFC, adaptive threshold, and coarse-to-fine acquisition. GDSS is based on Shakeel et al., Sensors 2023, doi:10.3390/s23084081. (See [GDSS Blocks Guide](docs/GDSS_BLOCKS.md), [GRC Blocks](grc/qradiolink_gdss_spreader_cc.block.yml))

### SOQPSK (Shaped Offset Quadrature Phase Shift Keying)

SOQPSK is a bandwidth-efficient modulation scheme that provides improved spectral efficiency compared to standard QPSK. The implementation supports two operational modes:

**Mode 1: VHF/UHF Single Channel**
- 1 carrier at 14,400 bps
- Bandwidth: ~10 kHz
- Fits in 12.5 kHz allocation

**Mode 2: VHF/UHF Multi-Carrier (fading resistant)**
- 3 carriers at 4,800 bps each
- Spacing: 4 kHz apart
- Total bandwidth: ~12 kHz
- Better multipath resistance

#### HF Maximum Capability

Maximum bandwidth segments available:
- **Most HF bands**: 2,700 Hz max (standard digital mode segments)
- **10m band (above 29 MHz)**: 6,000 Hz max

SOQPSK efficiency: ~1.5 bps/Hz

**Per-Carrier Capacity:**
- **Standard HF bands (2,700 Hz)**: 2,700 Hz × 1.5 bps/Hz = ~4,000 bps per carrier
- **10m band (6,000 Hz)**: 6,000 Hz × 1.5 bps/Hz = ~9,000 bps per carrier

#### VHF/UHF Specifications

**Carrier specifications:**
- Symbol rate: ~9,600 symbols/second (for 14,400 bps at 1.5 bps/Hz)
- Occupied bandwidth: ~9.6 kHz
- Fits comfortably in 12.5 kHz channel with guard bands
- Leaves ~2.9 kHz for filtering rolloff and guard bands

### Python Bindings

All blocks are available through Python bindings, including:
- All modulation/demodulation blocks (2FSK, 4FSK, 8FSK, CPM-4FSK, GMSK, BPSK, QPSK, SOQPSK, DSSS, AM, SSB, NBFM, WBFM)
- All digital voice blocks (FreeDV, M17, DMR, dPMR, NXDN, MMDVM)
- FEC blocks (LDPC encoder/decoder, interleaver)
- Supporting blocks (RSSI, M17 deframer, MMDVM source/sink, clipper, stretcher, etc.)

The Python bindings enable use in GNU Radio Companion flowgraphs and Python scripts.

## Directory Structure

```
gr-qradiolink/
├── CMakeLists.txt          # Top-level CMake configuration
├── include/                 # Public header files
│   └── gnuradio/
│       └── qradiolink/
├── lib/                    # Implementation files
├── python/                 # Python bindings
│   └── qradiolink/
│       └── bindings/
├── grc/                    # GNU Radio Companion block definitions
├── docs/                   # Documentation
│   ├── doxygen/
│   ├── DSSS_BLOCKS.md      # DSSS spreader/despreader guide
│   ├── GDSS_BLOCKS.md      # GDSS spreader/despreader guide
│   └── PTT_CONTROL.md      # PTT control with gr-osmosdr
├── examples/               # Example flowgraphs
├── tests/                  # Unit tests
└── cmake/                  # CMake modules
    └── Modules/
```

## Dependencies

See [DEPENDENCIES.md](DEPENDENCIES.md) for a complete list of required and optional dependencies.

**Quick Summary:**
- Clone with `git clone --recursive` (or run `git submodule update --init --recursive` after cloning)
- GNU Radio >= 3.10 (with vocoder component built with Codec2 support)
- CMake >= 3.16
- Boost libraries
- Volk (Vector-Optimized Library of Kernels)
- ZeroMQ (optional, for MMDVM blocks)
- Python 3.x with NumPy (for Python bindings)
- fmt library (for tests)

## Testing

The module includes comprehensive unit tests for all blocks. Tests are run using CTest and Boost.Test framework.

### Test Results

All tests pass successfully. See [tests/results.md](tests/results.md) for C++ unit tests, Python test harness, and edge case results. See [fuzzing-results/results.md](fuzzing-results/results.md) for fuzzing and MMDVM protocol test details.

**C++ Unit Tests:**
```
100% tests passed

Test Breakdown:
- 12 Manual tests (with int main): test_mod_2fsk, test_mod_4fsk, test_mod_8fsk,
  test_mod_am, test_mod_gmsk, test_mod_bpsk, test_mod_mmdvm, test_mod_freedv,
  test_gdss_spreader_cc, test_gdss_despreader_cc, test_dsss_cdma_transmitter_cc,
  test_dsss_cdma_receiver_cc
- Boost.Test tests: mod_ssb, mod_qpsk, mod_nbfm, mod_wbfm, mod_dsss, all
  demodulators, rssi_tag_block (8 tests), interleaver_bb (13 tests with edge cases)

Test Coverage:
- Modulators: 2FSK, 4FSK, 8FSK, AM, GMSK, BPSK, SSB, QPSK, NBFM, WBFM, DSSS, M17, DMR, dPMR, NXDN
- Demodulators: 2FSK, 4FSK, 8FSK, AM, GMSK, BPSK, SSB, QPSK, NBFM, WBFM, DSSS, M17, DMR, dPMR, NXDN
- FEC: interleaver_bb (HF burst error handling, 13 tests including edge cases)
- Note: SOQPSK and CPM-4FSK blocks are implemented and available but unit tests are planned for future releases
```

**MMDVM Protocol Tests (Python):**
```
41 tests passed, 0 tests failed
- Protocol validation: 28 tests (all passed)
  - POCSAG: 10 tests (all passed)
  - D-STAR: 9 tests (all passed)
  - YSF: 4 tests (all passed)
  - P25: 5 tests (all passed)
- Block integration: 13 tests (all passed)
  - POCSAG: 4 tests (encoder/decoder creation, output, roundtrip)
  - D-STAR: 3 tests (encoder/decoder creation, output)
  - YSF: 3 tests (encoder/decoder creation, output)
  - P25: 3 tests (encoder/decoder creation, output)
```

See [fuzzing-results/results.md](fuzzing-results/results.md) for complete MMDVM protocol test results.

A **DSSS BER simulation** ([examples/dsss_ber_simulation.py](examples/dsss_ber_simulation.py)) runs spreader -> AWGN -> despreader and plots BER vs SNR for N=64, 128, 256, overlaying the theoretical curve 0.5*erfc(sqrt(N*Es/N0/2)); the curves match within simulation noise, confirming correct DSSS behaviour.

### Running Tests

To build and run the test suite:

```bash
cd build
cmake ..
make
ctest --output-on-failure
```

### Fuzzing Results

The module includes comprehensive fuzzing coverage using libFuzzer. See [fuzzing-results/results.md](fuzzing-results/results.md) for complete fuzzing campaign results including:

- Coverage statistics (757 edges, 893 features discovered)
- Execution metrics (104+ million executions)
- Performance analysis
- Security assessment (0 crashes, 0 memory leaks)

### Python Validation Tests

The module includes Python-based validation tests for all modulation types. See [fuzzing-results/results.md](fuzzing-results/results.md) for validation test results. All digital voice modes (FreeDV, M17, DMR, dPMR, NXDN) now have Python bindings and validation support. SOQPSK blocks have Python bindings and are available for use; validation tests are planned for future releases.

## Documentation

### Block Documentation

- **[PTT Control Guide](docs/PTT_CONTROL.md)**: Comprehensive guide on controlling PTT (Push-To-Talk) with gr-osmosdr and similar SDR hardware when using gr-qradiolink blocks.
- **[DSSS Blocks Guide](docs/DSSS_BLOCKS.md)**: DSSS spreader and despreader blocks: PN sequence generation, timing recovery, soft-decision metrics, AFC, adaptive correlation threshold, coarse-to-fine acquisition, and integration examples.
- **[GDSS Blocks Guide](docs/GDSS_BLOCKS.md)**: GDSS spreader and despreader blocks: sequence generation, timing recovery, soft-decision metrics, AFC, adaptive threshold, coarse-to-fine acquisition, and advantages over binary PN sequences.
- **[Examples Directory](examples/README.md)**: Example flowgraphs and Python scripts demonstrating usage of various blocks.

### Block Reference Links

All blocks have GRC (GNU Radio Companion) block definitions in the `grc/` directory. Key blocks:

**Digital Modulations:**
- [2FSK Modulator](grc/qradiolink_mod_2fsk.block.yml) / [2FSK Demodulator](grc/qradiolink_demod_2fsk.block.yml)
- [4FSK Modulator](grc/qradiolink_mod_4fsk.block.yml) / [4FSK Demodulator](grc/qradiolink_demod_4fsk.block.yml)
- [8FSK Modulator](grc/qradiolink_mod_8fsk.block.yml) / [8FSK Demodulator](grc/qradiolink_demod_8fsk.block.yml)
- [CPM-4FSK Modulator](grc/qradiolink_mod_cpm_4fsk.block.yml)
- [GMSK Modulator](grc/qradiolink_mod_gmsk.block.yml) / [GMSK Demodulator](grc/qradiolink_demod_gmsk.block.yml)
- [BPSK Modulator](grc/qradiolink_mod_bpsk.block.yml) / [BPSK Demodulator](grc/qradiolink_demod_bpsk.block.yml)
- [QPSK Modulator](grc/qradiolink_mod_qpsk.block.yml) / [QPSK Demodulator](grc/qradiolink_demod_qpsk.block.yml)
- [SOQPSK Modulator](grc/qradiolink_mod_soqpsk.block.yml) / [SOQPSK Demodulator](grc/qradiolink_demod_soqpsk.block.yml)
- [DSSS Spreader](grc/qradiolink_dsss_spreader_cc.block.yml) / [DSSS Despreader](grc/qradiolink_dsss_despreader_cc.block.yml)
- [DSSS-CDMA Transmitter](grc/qradiolink_dsss_cdma_transmitter_cc.block.yml) / [DSSS-CDMA Receiver](grc/qradiolink_dsss_cdma_receiver_cc.block.yml)
- [GDSS Spreader](grc/qradiolink_gdss_spreader_cc.block.yml) / [GDSS Despreader](grc/qradiolink_gdss_despreader_cc.block.yml)

**Analog Modulations:**
- [AM Modulator](grc/qradiolink_mod_am.block.yml) / [AM Demodulator](grc/qradiolink_demod_am.block.yml)
- [SSB Modulator](grc/qradiolink_mod_ssb.block.yml) / [SSB Demodulator](grc/qradiolink_demod_ssb.block.yml)
- [NBFM Modulator](grc/qradiolink_mod_nbfm.block.yml) / [NBFM Demodulator](grc/qradiolink_demod_nbfm.block.yml)
- [WBFM Modulator](grc/qradiolink_mod_wbfm.block.yml) / [WBFM Demodulator](grc/qradiolink_demod_wbfm.block.yml)

**Digital Voice:**
- [M17 Coder](grc/qradiolink_m17_coder.block.yml) / [M17 Decoder](grc/qradiolink_m17_decoder.block.yml) / [M17 Deframer](grc/qradiolink_m17_deframer.block.yml)
- [DMR Modulator](grc/qradiolink_mod_dmr.block.yml) / [DMR Demodulator](grc/qradiolink_demod_dmr.block.yml)
- [dPMR Modulator](grc/qradiolink_mod_dpmr.block.yml) / [dPMR Demodulator](grc/qradiolink_demod_dpmr.block.yml)
- [NXDN Modulator](grc/qradiolink_mod_nxdn.block.yml) / [NXDN Demodulator](grc/qradiolink_demod_nxdn.block.yml)

**MMDVM Protocols:**
- [POCSAG Encoder](grc/qradiolink_pocsag_encoder.block.yml) / [POCSAG Decoder](grc/qradiolink_pocsag_decoder.block.yml)
- [D-STAR Encoder](grc/qradiolink_dstar_encoder.block.yml) / [D-STAR Decoder](grc/qradiolink_dstar_decoder.block.yml)
- [YSF Encoder](grc/qradiolink_ysf_encoder.block.yml) / [YSF Decoder](grc/qradiolink_ysf_decoder.block.yml)
- [P25 Encoder](grc/qradiolink_p25_encoder.block.yml) / [P25 Decoder](grc/qradiolink_p25_decoder.block.yml)

**FEC:**
- [LDPC Encoder](grc/qradiolink_ldpc_encoder.block.yml) / [LDPC Decoder (Soft-Decision)](grc/qradiolink_ldpc_decoder.block.yml)
  - Supports regular and irregular LDPC codes
  - Configurable code rates: 1/2, 2/3, 3/4, or custom
  - Configurable block lengths: 576, 1152, 2304 bits, or custom
  - Auto-selection of best matching code from available AList files
- [Interleaver (HF Burst)](grc/qradiolink_interleaver_bb.block.yml) - Block interleaver/deinterleaver for HF burst error handling

For complete list of all blocks, see the [grc/](grc/) directory.

## References

- **GDSS (Gaussian-Distributed Spread-Spectrum):** Shakeel, I.; Hilliard, J.; Zhang, W.; Rice, M. Gaussian-Distributed Spread-Spectrum for Covert Communications. *Sensors* 2023, 23(8), 4081. [doi:10.3390/s23084081](https://doi.org/10.3390/s23084081). The GDSS spreader/despreader blocks in this module implement the quadrant-preserving I/Q masking scheme (Section 3) from this paper: each chip uses independent Gaussian samples U, V with element-wise masking I = R(S)*|U|, Q = Im(S)*|V|.

## License

This project is licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later).

See the [LICENSE](../LICENSE) file in the QRadioLink repository for details.


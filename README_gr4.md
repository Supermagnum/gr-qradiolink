# gr-qradiolink — GNU Radio 4.0 headers (`gnuradio4` branch)

This branch adds **header-only GR4 blocks** under `gnuradio4/` (full port; `gr4/` remains as the earlier stub tree). The GNU Radio 3.10 OOT on **`main`** is unchanged on this branch.

**Documentation:** [docs/CODE_MAP.md](docs/CODE_MAP.md) (function index), [docs/GRIDENT_ZMQ.md](docs/GRIDENT_ZMQ.md) (gr-ident ZMQ mode routing; shared with `main`).

## Requirements

- C++23 compiler with `<print>` (typically **GCC 14+**; GCC 13 lacks libstdc++ `<print>` required by upstream GR4)
- CMake 3.27+ (matches upstream GR4)
- A checkout of [GNU Radio 4.0](https://github.com/gnuradio/gnuradio4)
- Network access on **first** configure (GR4 uses `FetchContent` for `boost-ext/ut`, `vir-simd`, `cpp-httplib`, `libsoundio`, etc.)
- Optional: `codec2`, `libm17`, `libzmq` (via pkg-config; required for `GrIdentPreambleSub`)

## Build

Point `GR4_SOURCE_DIR` at your GR4 clone, then configure from **`cmake_gr4/`** (CMake requires `CMakeLists.txt` in the source directory):

```bash
cmake -S cmake_gr4 -B build_gr4 \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DCMAKE_CXX_COMPILER=g++-14 \
  -DGR4_SOURCE_DIR=/path/to/gnuradio4

cmake --build build_gr4 --parallel $(nproc)

cd build_gr4 && ctest --output-on-failure
```

Set `CMAKE_CXX_COMPILER` to **GCC 14+** (or another toolchain whose libstdc++ provides `<print>`). The default `g++` on many distributions is still GCC 13.

`CMakeLists_gr4.txt` at the repository root mirrors the same logic when `GR_QRL_ROOT` is the repo root; the supported entry point remains `cmake_gr4/`.

## Test results (recorded 2026-05-10)

Environment for the run below: **Ubuntu 24.04**, **CMake 3.28.3**, **`g++-14` 14.2.0**, GNU Radio 4 clone at **`08028ca`** (2026-05-08), gr-qradiolink branch **`gnuradio4`** at **`13d120d`**.

| Suite | Scope | Result |
|-------|--------|--------|
| **GR4 overlay** | `cmake_gr4` + `gnuradio4/test` (8 suites incl. `qa_GrIdent`, `qa_DigitalMods`, …) | Run `ctest` after build (see below) |
| **GR3 OOT (CTest)** | Existing GNU Radio 3.10 build in `build/` (34 tests registered) | **33/33 passed** when excluding `qradiolink_test_demod_dsss`; **full `ctest` does not complete** on that test (flowgraph waits indefinitely in `tests/test_demod_dsss.cc`; GNU Radio may log `pthread_setschedparam failed with return code 22`) |

Reproduce GR4 tests after a clean configure:

```bash
cmake -S cmake_gr4 -B build_gr4 -DCMAKE_CXX_COMPILER=g++-14 -DGR4_SOURCE_DIR=/path/to/gnuradio4
cmake --build build_gr4 --parallel "$(nproc)"
(cd build_gr4 && ctest --output-on-failure)
```

## GR4 CMake target

The GR4 core library target is **`gnuradio4::gnuradio-core`** (alias of `gnuradio-core`). This port links it through **`gnuradio-qradiolink-gr4`** (`INTERFACE`).

## Reflection

Upstream GR4 uses **`GR_MAKE_REFLECTABLE(ClassName, ...)`** inside the block struct (see `meta/reflection.hpp`). There is **no** `ENABLE_REFLECTION_FOR_TEMPLATE` in this GR4 revision; do not use legacy macro names from older drafts.

## Blocks (`gnuradio4/include/gnuradio-4.0/qradiolink/`)

| Area | Headers |
|------|---------|
| Digital modem | `Mod2Fsk`, `Demod2Fsk`, `Mod4Fsk`, `Demod4Fsk`, `Mod8Fsk`, `Demod8Fsk`, `ModCpm4Fsk`, `ModGmsk`, `DemodGmsk`, `ModBpsk`, `DemodBpsk`, `ModQpsk`, `DemodQpsk`, `ModSoqpsk`, `DemodSoqpsk` |
| Analog | `ModAm`, `DemodAm`, `ModSsb`, `DemodSsb`, `ModNbfm`, `DemodNbfm`, `ModWbfm`, `DemodWbfm` |
| Digital voice | `ModDmr`, `DemodDmr`, `ModDpmr`, `DemodDpmr`, `ModNxdn`, `DemodNxdn`, `ModM17`, `DemodM17`, `M17Coder`, `M17Decoder`, `M17Deframer` |
| Protocol / MMDVM | `PocsagEncoder`, `PocsagDecoder`, `DstarEncoder`, `DstarDecoder`, `YsfEncoder`, `YsfDecoder`, `P25Encoder`, `P25Decoder`, `MmdvmSource`, `MmdvmSink` |
| DSSS / GDSS | `DsssSpreaderCc`, `DsssDespreaderCc`, `DsssCdmaTransmitterCc`, `DsssCdmaReceiverCc`, `GdssSpreaderCc`, `GdssDespreaderCc` |
| gr-ident ZMQ | `GrIdentModeControl`, `GrIdentPreambleSub` (ZMQ), `ModDemodSwitchRx`, `ModDemodSwitchTx`; see [GRIDENT_ZMQ.md](docs/GRIDENT_ZMQ.md) |
| Utility | `InterleaverBb`, `RssiTagBlock`, `ClipperCc`, `StretcherCc`, `ZeroIdleBursts`, `Gr4FskDiscriminator` |

Legacy stubs under `gr4/include/` are still present; **`cmake_gr4` builds `gnuradio4/`** only.

**Status:** Several blocks include reference DSP (`Mod2Fsk`/`Demod2Fsk`, analog helpers in `detail/SimpleDigitalAnalog.hpp`); many PDU/voice blocks are deterministic shims for QA until full GR3 ports land.

## Limitations

| Feature | Status |
|---------|--------|
| C++ headers | Partial (stubs + 2FSK reference) |
| Python bindings | Not available in GR4 yet |
| GNU Radio Companion | GR4 not supported in GRC yet |

## GR3 tree

For GNU Radio 3.10 builds, flowgraphs, and Python APIs, use **`main`** and `README.md`.

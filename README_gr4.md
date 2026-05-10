# gr-qradiolink — GNU Radio 4.0 headers (`gnuradio4` branch)

This branch adds **header-only GR4 blocks** under `gr4/` next to the existing GNU Radio 3.10 tree (`lib/`, `include/`, `python/`, `grc/`). Nothing in the GR3 layout is modified.

## Requirements

- C++23 compiler with `<print>` (typically **GCC 14+**; GCC 13 lacks libstdc++ `<print>` required by upstream GR4)
- CMake 3.27+ (matches upstream GR4)
- A checkout of [GNU Radio 4.0](https://github.com/gnuradio/gnuradio4)
- Network access on **first** configure (GR4 uses `FetchContent` for `boost-ext/ut`, `vir-simd`, `cpp-httplib`, `libsoundio`, etc.)
- Optional: `codec2`, `libm17`, `libzmq` (via pkg-config)

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
| **GR4 overlay** | `cmake_gr4` + `gr4/tests` (`qa_mod_2fsk`, `qa_demod_2fsk`, `qa_gr4_smoke`) | **3/3 passed** (`ctest`, ~0.05 s total) |
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

## Blocks (`gr4/include/gnuradio-4.0/qradiolink/`)

| Area | Headers |
|------|---------|
| Digital modem | `mod_2fsk.hpp`, `demod_2fsk.hpp`, `mod_4fsk.hpp`, `demod_4fsk.hpp`, `mod_8fsk.hpp`, `demod_8fsk.hpp`, `mod_cpm_4fsk.hpp`, `mod_gmsk.hpp`, `demod_gmsk.hpp`, `mod_bpsk.hpp`, `demod_bpsk.hpp`, `mod_qpsk.hpp`, `demod_qpsk.hpp`, `mod_soqpsk.hpp`, `demod_soqpsk.hpp` |
| Analog | `mod_am.hpp`, `demod_am.hpp`, `mod_ssb.hpp`, `demod_ssb.hpp`, `mod_nbfm.hpp`, `demod_nbfm.hpp`, `mod_wbfm.hpp`, `demod_wbfm.hpp` |
| Digital voice | `mod_dmr.hpp`, `demod_dmr.hpp`, `mod_dpmr.hpp`, `demod_dpmr.hpp`, `mod_nxdn.hpp`, `demod_nxdn.hpp`, `m17_coder.hpp`, `m17_decoder.hpp`, `m17_deframer.hpp` |
| Protocol / MMDVM | `pocsag_encoder.hpp`, `pocsag_decoder.hpp`, `dstar_encoder.hpp`, `dstar_decoder.hpp`, `ysf_encoder.hpp`, `ysf_decoder.hpp`, `p25_encoder.hpp`, `p25_decoder.hpp`, `mmdvm_source.hpp`, `mmdvm_sink.hpp` |
| DSSS / GDSS | `dsss_spreader.hpp`, `dsss_despreader.hpp`, `dsss_cdma_tx.hpp`, `dsss_cdma_rx.hpp`, `gdss_spreader.hpp`, `gdss_despreader.hpp` |
| Utility | `ldpc_encoder.hpp`, `ldpc_decoder.hpp`, `interleaver_bb.hpp`, `rssi_tag.hpp`, `clipper.hpp`, `stretcher.hpp` |

**Status:** `mod_2fsk.hpp` / `demod_2fsk.hpp` contain usable reference signal processing; the remaining headers are **stubs** (compile-time placeholders) until DSP is ported line-by-line from the matching `lib/*_impl.cc` files.

## Limitations

| Feature | Status |
|---------|--------|
| C++ headers | Partial (stubs + 2FSK reference) |
| Python bindings | Not available in GR4 yet |
| GNU Radio Companion | GR4 not supported in GRC yet |

## GR3 tree

For GNU Radio 3.10 builds, flowgraphs, and Python APIs, use **`main`** and `README.md`.

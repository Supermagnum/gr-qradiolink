#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# DSSS BER simulation: spreader -> AWGN -> despreader
# Plots simulated BER vs SNR for N=64, 128, 256 and overlays theoretical
# BER = 0.5 * erfc(sqrt(N * Es/N0 / 2)) (QPSK DSSS, Sensors 2023 Eq. 1).
# Requires: numpy, matplotlib, gnuradio, gr-qradiolink (built and on PYTHONPATH).

from __future__ import print_function
import sys
import math
import numpy as np

try:
    from gnuradio import gr, blocks, analog
    from gnuradio import qradiolink
except ImportError as e:
    print("Import failed:", e, file=sys.stderr)
    print("Ensure GNU Radio and gr-qradiolink are built and on PYTHONPATH.", file=sys.stderr)
    sys.exit(1)

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
except ImportError:
    plt = None


def m_sequence(n_bits, taps):
    """Generate m-sequence (maximal-length LFSR): length 2^n - 1, values 0/1."""
    reg = [1] * n_bits
    out = []
    for _ in range((1 << n_bits) - 1):
        out.append(reg[-1])
        feedback = 0
        for t in taps:
            feedback ^= reg[t - 1]
        reg = [feedback] + reg[:-1]
    return out


def pn_sequence_for_N(N):
    """Return PN sequence of length N (0/1) for spreader/despreader.
    Uses m-sequence if N = 2^n - 1, else repeats Barker-13 or short m-seq.
    """
    if N == 63:
        return m_sequence(6, [6, 5])
    if N == 127:
        return m_sequence(7, [7, 6])
    if N == 255:
        return m_sequence(8, [8, 6, 5, 4])
    barker13 = [1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1]
    if N <= len(barker13):
        return barker13[:N]
    out = []
    while len(out) < N:
        out.extend(barker13)
    return out[:N]


def theoretical_ber_qpsk_dsss(snr_linear, N):
    """BER = 0.5 * erfc(sqrt(N * Es/N0 / 2)); Es/N0 in linear (Sensors 2023 Eq. 1)."""
    x = np.asarray(snr_linear, dtype=float)
    return 0.5 * np.vectorize(lambda e: math.erfc(np.sqrt(N * e / 2)))(x)


def run_one_snr(snr_db, N, num_symbols, seed):
    """Run spreader -> AWGN -> despreader for one SNR; return BER."""
    np.random.seed(seed)

    pn = pn_sequence_for_N(N)
    symbols = 2 * (np.random.rand(num_symbols) > 0.5).astype(np.float32) - 1
    symbols_c = symbols + 0j

    tb = gr.top_block()
    src = blocks.vector_source_c(list(symbols_c), False)
    spreader = qradiolink.dsss_spreader_cc(pn, N)
    Es_N0_linear = 10.0 ** (snr_db / 10.0)
    noise_std = np.sqrt(1.0 / (2.0 * Es_N0_linear))
    noise = analog.fastnoise_source_c(analog.GR_GAUSSIAN, noise_std, seed)
    add = blocks.add_cc(1)
    despreader = qradiolink.dsss_despreader_cc(pn, N, 0.3, 2)
    sink = blocks.vector_sink_c()
    null1 = blocks.null_sink(gr.sizeof_float)
    null2 = blocks.null_sink(gr.sizeof_float)

    tb.connect(src, spreader)
    tb.connect(spreader, (add, 0))
    tb.connect(noise, (add, 1))
    tb.connect(add, despreader)
    tb.connect((despreader, 0), sink)
    tb.connect((despreader, 1), null1)
    tb.connect((despreader, 2), null2)
    tb.run()

    out = np.array(sink.data())
    if len(out) == 0:
        return 1.0
    n = min(len(out), num_symbols)
    bits_rx = (np.real(out[:n]) > 0).astype(int)
    bits_tx = (symbols[:n] > 0).astype(int)
    err = np.sum(bits_rx != bits_tx)
    return err / float(n)


def main():
    spreading_factors = [64, 128, 256]
    snr_db_range = np.arange(-12, 6, 1.0)
    num_symbols = 4000
    num_trials = 2

    print("DSSS BER simulation (spreader -> AWGN -> despreader)")
    print("Spreading factors N =", spreading_factors)
    print("SNR range (dB):", snr_db_range[0], "...", snr_db_range[-1])
    print("Symbols per run:", num_symbols, "  Trials per (N,SNR):", num_trials)

    results = {}
    for N in spreading_factors:
        ber_list = []
        for snr_db in snr_db_range:
            bers = [run_one_snr(snr_db, N, num_symbols, seed=12345 + int(snr_db) + N * 100) for _ in range(num_trials)]
            ber_list.append(np.mean(bers))
        results[N] = np.array(ber_list)

    snr_linear = 10.0 ** (snr_db_range / 10.0)
    theory = {}
    for N in spreading_factors:
        theory[N] = theoretical_ber_qpsk_dsss(snr_linear, N)

    if plt is not None:
        plt.figure(figsize=(8, 5))
        for N in spreading_factors:
            plt.semilogy(snr_db_range, results[N], "o-", label="Sim N=%d" % N, markersize=4)
            plt.semilogy(snr_db_range, theory[N], "--", label="Theory N=%d" % N)
        plt.xlabel("Es/N0 (dB)")
        plt.ylabel("BER")
        plt.title("DSSS BER: spreader -> AWGN -> despreader (theory: 0.5*erfc(sqrt(N*Es/N0/2)))")
        plt.legend()
        plt.grid(True, which="both")
        out_path = "dsss_ber_curves.png"
        plt.savefig(out_path, dpi=120)
        print("Plot saved to", out_path)
    else:
        print("matplotlib not available; skipping plot.")
        for N in spreading_factors:
            print("N=%d BER at -6 dB:" % N, results[N][np.argmin(np.abs(snr_db_range + 6))])
            print("  Theory at -6 dB:", theory[N][np.argmin(np.abs(snr_db_range + 6))])

    print("Done.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
Test script for new blocks: 8FSK, GDSS, and DSSS-CDMA
"""

import sys
import numpy as np
import math

try:
    from gnuradio import gr
    from gnuradio import blocks
    from gnuradio import qradiolink
except ImportError as e:
    print(f"ERROR: Cannot import GNU Radio modules: {e}")
    sys.exit(1)


def test_8fsk_modulator():
    """Test 8FSK modulator"""
    print("Testing 8FSK Modulator...", end=" ")
    try:
        tb = gr.top_block()
        
        # Generate test data (bytes)
        test_data = [0, 1, 2, 3, 4, 5, 6, 7] * 100
        source = blocks.vector_source_b(test_data, False)
        
        mod = qradiolink.mod_8fsk(125, 250000, 1700, 8000, True)
        sink = blocks.null_sink(gr.sizeof_gr_complex)
        
        tb.connect(source, mod)
        tb.connect(mod, sink)
        
        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()
        
        print("✓ PASSED")
        return True
    except Exception as e:
        print(f"✗ FAILED: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_8fsk_demodulator():
    """Test 8FSK demodulator"""
    print("Testing 8FSK Demodulator...", end=" ")
    try:
        tb = gr.top_block()
        
        # Generate test signal
        samples = 10000
        signal = np.zeros(samples, dtype=np.complex64)
        for i in range(samples):
            phase = 2 * math.pi * 1700 * i / 250000
            signal[i] = np.exp(1j * phase) * 0.5
        
        source = blocks.vector_source_c(signal.tolist(), False)
        demod = qradiolink.demod_8fsk(125, 250000, 1700, 8000, True)
        
        sink1 = blocks.null_sink(gr.sizeof_gr_complex)
        sink2 = blocks.null_sink(gr.sizeof_gr_complex)
        sink3 = blocks.null_sink(gr.sizeof_char)
        
        tb.connect(source, demod)
        tb.connect(demod, sink1)
        tb.connect((demod, 1), sink2)
        tb.connect((demod, 2), sink3)
        
        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()
        
        print("✓ PASSED")
        return True
    except Exception as e:
        print(f"✗ FAILED: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_gdss_spreader():
    """Test GDSS spreader"""
    print("Testing GDSS Spreader...", end=" ")
    try:
        tb = gr.top_block()
        
        # Generate test symbols
        samples = 100
        signal = np.zeros(samples, dtype=np.complex64)
        for i in range(samples):
            signal[i] = np.complex64(0.5 + 0.3j)
        
        source = blocks.vector_source_c(signal.tolist(), False)
        spreader = qradiolink.gdss_spreader_cc(127, 42, 1.0, 12345)
        sink = blocks.null_sink(gr.sizeof_gr_complex)
        
        tb.connect(source, spreader)
        tb.connect(spreader, sink)
        
        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()
        
        print("✓ PASSED")
        return True
    except Exception as e:
        print(f"✗ FAILED: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_gdss_despreader():
    """Test GDSS despreader"""
    print("Testing GDSS Despreader...", end=" ")
    try:
        tb = gr.top_block()
        
        # Generate test signal (spread spectrum)
        samples = 4200  # 100 symbols * 42 chips
        signal = np.zeros(samples, dtype=np.complex64)
        for i in range(samples):
            signal[i] = np.complex64(0.1 + 0.1j)
        
        source = blocks.vector_source_c(signal.tolist(), False)
        
        # Generate same sequence as spreader would use
        import random
        random.seed(12345)
        np.random.seed(12345)
        spreading_sequence = np.random.normal(0.0, 1.0, 127).tolist()
        
        despreader = qradiolink.gdss_despreader_cc(spreading_sequence, 42, 0.7, 2)
        
        sink1 = blocks.null_sink(gr.sizeof_gr_complex)
        sink2 = blocks.null_sink(gr.sizeof_float)
        sink3 = blocks.null_sink(gr.sizeof_float)
        
        tb.connect(source, despreader)
        tb.connect(despreader, sink1)
        tb.connect((despreader, 1), sink2)
        tb.connect((despreader, 2), sink3)
        
        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()
        
        print("✓ PASSED")
        return True
    except Exception as e:
        print(f"✗ FAILED: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_dsss_cdma_transmitter():
    """Test DSSS-CDMA transmitter"""
    print("Testing DSSS-CDMA Transmitter...", end=" ")
    try:
        tb = gr.top_block()
        
        # Generate spreading codes for 2 users
        code1 = [1, -1, 1, -1] * 32  # 128 chips
        code2 = [-1, 1, -1, 1] * 32  # 128 chips
        spreading_codes = [code1, code2]
        
        # Generate test symbols for 2 users
        samples = 50
        signal1 = np.zeros(samples, dtype=np.complex64)
        signal2 = np.zeros(samples, dtype=np.complex64)
        for i in range(samples):
            signal1[i] = np.complex64(0.5 + 0.3j)
            signal2[i] = np.complex64(0.3 + 0.5j)
        
        source1 = blocks.vector_source_c(signal1.tolist(), False)
        source2 = blocks.vector_source_c(signal2.tolist(), False)
        
        transmitter = qradiolink.dsss_cdma_transmitter_cc(
            spreading_codes, 128, 2, True)
        sink = blocks.null_sink(gr.sizeof_gr_complex)
        
        tb.connect(source1, (transmitter, 0))
        tb.connect(source2, (transmitter, 1))
        tb.connect(transmitter, sink)
        
        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()
        
        print("✓ PASSED")
        return True
    except Exception as e:
        print(f"✗ FAILED: {e}")
        import traceback
        traceback.print_exc()
        return False


def test_dsss_cdma_receiver():
    """Test DSSS-CDMA receiver"""
    print("Testing DSSS-CDMA Receiver...", end=" ")
    try:
        tb = gr.top_block()
        
        # Generate test signal (combined CDMA)
        samples = 6400  # 50 symbols * 128 chips
        signal = np.zeros(samples, dtype=np.complex64)
        for i in range(samples):
            signal[i] = np.complex64(0.1 + 0.1j)
        
        source = blocks.vector_source_c(signal.tolist(), False)
        
        # Use same spreading code as transmitter
        spreading_code = [1, -1, 1, -1] * 32  # 128 chips
        
        receiver = qradiolink.dsss_cdma_receiver_cc(
            spreading_code, 128, 0.7, 2)
        
        sink1 = blocks.null_sink(gr.sizeof_gr_complex)
        sink2 = blocks.null_sink(gr.sizeof_float)
        sink3 = blocks.null_sink(gr.sizeof_float)
        sink4 = blocks.null_sink(gr.sizeof_float)
        
        tb.connect(source, receiver)
        tb.connect(receiver, sink1)
        tb.connect((receiver, 1), sink2)
        tb.connect((receiver, 2), sink3)
        tb.connect((receiver, 3), sink4)
        
        tb.start()
        tb.wait()
        tb.stop()
        tb.wait()
        
        print("✓ PASSED")
        return True
    except Exception as e:
        print(f"✗ FAILED: {e}")
        import traceback
        traceback.print_exc()
        return False


def main():
    """Run all tests"""
    print("=" * 70)
    print("Testing New Blocks: 8FSK, GDSS, and DSSS-CDMA")
    print("=" * 70)
    print()
    
    results = {"passed": 0, "failed": 0}
    
    tests = [
        ("8FSK Modulator", test_8fsk_modulator),
        ("8FSK Demodulator", test_8fsk_demodulator),
        ("GDSS Spreader", test_gdss_spreader),
        ("GDSS Despreader", test_gdss_despreader),
        ("DSSS-CDMA Transmitter", test_dsss_cdma_transmitter),
        ("DSSS-CDMA Receiver", test_dsss_cdma_receiver),
    ]
    
    for test_name, test_func in tests:
        if test_func():
            results["passed"] += 1
        else:
            results["failed"] += 1
        print()
    
    # Summary
    print("=" * 70)
    print("Test Summary")
    print("=" * 70)
    print(f"Passed: {results['passed']}")
    print(f"Failed: {results['failed']}")
    print(f"Total: {results['passed'] + results['failed']}")
    
    return 0 if results["failed"] == 0 else 1


if __name__ == "__main__":
    sys.exit(main())


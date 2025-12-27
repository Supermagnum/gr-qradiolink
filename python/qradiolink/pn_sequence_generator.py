"""
PN Sequence Generator - Python interface for generating PN sequences

This module provides Python bindings for PN sequence generation functions
used in DSSS communication systems.
"""

import numpy as np

def generate_msequence_127():
    """
    Generate standard 127-chip m-sequence (polynomial [7,1])
    
    Returns:
        numpy.ndarray: Array of +1/-1 values
    """
    # Polynomial: x^7 + x + 1
    taps = [7, 1]
    return _lfsr_sequence(taps, 127)

def generate_msequence_511():
    """
    Generate standard 511-chip m-sequence (polynomial [9,4])
    
    Returns:
        numpy.ndarray: Array of +1/-1 values
    """
    # Polynomial: x^9 + x^4 + 1
    taps = [9, 4]
    return _lfsr_sequence(taps, 511)

def generate_msequence_1023():
    """
    Generate standard 1023-chip m-sequence (polynomial [10,3])
    
    Returns:
        numpy.ndarray: Array of +1/-1 values
    """
    # Polynomial: x^10 + x^3 + 1
    taps = [10, 3]
    return _lfsr_sequence(taps, 1023)

def generate_gold_code(poly1_taps, poly2_taps, code_number, length):
    """
    Generate a Gold code from two preferred m-sequences
    
    Args:
        poly1_taps: First polynomial tap positions
        poly2_taps: Second polynomial tap positions
        code_number: Which code from the Gold code family (0 to length-1)
        length: Sequence length
    
    Returns:
        numpy.ndarray: Array of +1/-1 values
    """
    seq1 = _lfsr_sequence(poly1_taps, length)
    seq2 = _lfsr_sequence(poly2_taps, length)
    
    # Circularly shift seq2
    seq2_shifted = np.roll(seq2, code_number)
    
    # XOR (multiply in +1/-1 representation)
    gold_code = seq1 * seq2_shifted
    
    return gold_code

def generate_gold_code_family(length, num_codes):
    """
    Generate Gold code family for multi-user operation
    
    Args:
        length: Sequence length (127, 511, or 1023)
        num_codes: Number of codes to generate
    
    Returns:
        list: List of numpy arrays, each containing a Gold code
    """
    if length == 127:
        poly1_taps = [7, 1]
        poly2_taps = [7, 3]
    elif length == 511:
        poly1_taps = [9, 4]
        poly2_taps = [9, 6]
    elif length == 1023:
        poly1_taps = [10, 3]
        poly2_taps = [10, 8]
    else:
        raise ValueError(f"Unsupported length: {length}")

    family = []
    for i in range(num_codes):
        family.append(generate_gold_code(poly1_taps, poly2_taps, i, length))
    
    return family

def _lfsr_sequence(taps, length, initial_state=1):
    """
    Internal LFSR implementation for m-sequence generation
    
    Args:
        taps: Polynomial tap positions
        length: Sequence length
        initial_state: Initial LFSR state
    
    Returns:
        numpy.ndarray: Array of +1/-1 values
    """
    if len(taps) == 0 or length <= 0:
        return np.array([])
    
    n = taps[0]  # First tap is register length
    state = initial_state
    sequence = []
    
    for i in range(length):
        # Output the LSB
        sequence.append(1 if (state & 1) else -1)
        
        # Compute feedback (XOR of tap positions)
        feedback = 0
        for tap in taps:
            if tap > 0 and tap <= n:
                feedback ^= (state >> (n - tap)) & 1
        
        # Shift and insert feedback
        state = (state >> 1) | (feedback << (n - 1))
    
    return np.array(sequence)


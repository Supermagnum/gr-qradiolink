"""
LDPC Code Helper Module

Provides functions to select appropriate LDPC codes based on code rate and block length.
Supports both regular and irregular LDPC codes.
"""

import os
import glob
from typing import Optional, Tuple, List

# Standard LDPC code directory
DEFAULT_LDPC_DIR = "/usr/share/gnuradio/fec/ldpc/"

# Code rate mappings (fraction to decimal)
CODE_RATES = {
    "1/2": 0.5,
    "2/3": 0.6666667,
    "3/4": 0.75,
}

# Standard block lengths
STANDARD_BLOCK_LENGTHS = [576, 1152, 2304]


def get_code_info(alist_file: str) -> Optional[Tuple[int, int, float]]:
    """
    Read code information from an AList file.
    
    Args:
        alist_file: Path to AList file
        
    Returns:
        Tuple of (n, k, rate) or None if file cannot be read
    """
    try:
        with open(alist_file, 'r') as f:
            line = f.readline().strip().split()
            if len(line) >= 2:
                n = int(line[0])
                k = int(line[1])
                rate = k / n if n > 0 else 0.0
                return (n, k, rate)
    except (IOError, ValueError, IndexError):
        pass
    return None


def find_codes(ldpc_dir: str = DEFAULT_LDPC_DIR) -> List[Tuple[str, int, int, float]]:
    """
    Scan LDPC directory and return list of available codes.
    
    Args:
        ldpc_dir: Directory containing AList files
        
    Returns:
        List of tuples: (filename, n, k, rate)
    """
    codes = []
    pattern = os.path.join(ldpc_dir, "*.alist")
    
    for alist_file in glob.glob(pattern):
        info = get_code_info(alist_file)
        if info:
            n, k, rate = info
            codes.append((os.path.basename(alist_file), n, k, rate))
    
    return codes


def select_code_by_params(
    block_length: int,
    code_rate: str,
    ldpc_dir: str = DEFAULT_LDPC_DIR,
    tolerance: float = 0.1
) -> Optional[str]:
    """
    Select an AList file based on block length and code rate.
    
    Args:
        block_length: Desired block length (n) in bits
        code_rate: Code rate as string ("1/2", "2/3", "3/4") or decimal (0.5, 0.667, 0.75)
        ldpc_dir: Directory containing AList files
        tolerance: Maximum acceptable rate difference (default: 0.1)
        
    Returns:
        Path to selected AList file, or None if no suitable code found
    """
    # Convert code rate string to float
    if isinstance(code_rate, str):
        if code_rate in CODE_RATES:
            target_rate = CODE_RATES[code_rate]
        else:
            try:
                target_rate = float(code_rate)
            except ValueError:
                return None
    else:
        target_rate = float(code_rate)
    
    codes = find_codes(ldpc_dir)
    
    if not codes:
        return None
    
    # Filter codes by rate tolerance
    suitable_codes = [
        (fname, n, k, rate) for fname, n, k, rate in codes
        if abs(rate - target_rate) <= tolerance
    ]
    
    if not suitable_codes:
        # If no codes match rate, use all codes
        suitable_codes = codes
    
    # Find closest block length match
    best_match = min(
        suitable_codes,
        key=lambda x: abs(x[1] - block_length)
    )
    
    return os.path.join(ldpc_dir, best_match[0])


def get_code_selection(
    use_custom: bool,
    custom_file: str = "",
    block_length: int = 576,
    code_rate: str = "1/2",
    ldpc_dir: str = DEFAULT_LDPC_DIR
) -> str:
    """
    Get the appropriate AList file path based on configuration.
    
    Args:
        use_custom: If True, use custom_file; otherwise, select by params
        custom_file: Path to custom AList file (if use_custom is True)
        block_length: Desired block length in bits
        code_rate: Code rate as string ("1/2", "2/3", "3/4")
        ldpc_dir: Directory containing AList files
        
    Returns:
        Path to AList file to use
    """
    if use_custom and custom_file:
        if os.path.exists(custom_file):
            return custom_file
        else:
            # Fallback to auto-selection if custom file doesn't exist
            return select_code_by_params(block_length, code_rate, ldpc_dir) or custom_file
    else:
        selected = select_code_by_params(block_length, code_rate, ldpc_dir)
        return selected or custom_file


def list_available_codes(ldpc_dir: str = DEFAULT_LDPC_DIR) -> List[dict]:
    """
    List all available LDPC codes with their parameters.
    
    Args:
        ldpc_dir: Directory containing AList files
        
    Returns:
        List of dictionaries with code information
    """
    codes = find_codes(ldpc_dir)
    return [
        {
            "file": fname,
            "block_length": n,
            "info_bits": k,
            "parity_bits": n - k,
            "code_rate": f"{k}/{n}",
            "rate_decimal": rate,
            "path": os.path.join(ldpc_dir, fname)
        }
        for fname, n, k, rate in codes
    ]


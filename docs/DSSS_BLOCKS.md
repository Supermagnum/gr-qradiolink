# DSSS Spreader and Despreader Blocks

## Overview

This document describes the enhanced Direct Sequence Spread Spectrum (DSSS) blocks for gr-qradiolink. These blocks provide advanced spreading and despreading capabilities with timing recovery, code acquisition, and lock detection.

## System Specifications

- **Chip rate**: 500 kHz (500,000 chips per second)
- **Symbol rate**: 12 kHz (12,000 symbols per second)
- **Spreading factor**: ~42 chips per symbol (500k / 12k)
- **PN sequence**: Gold code or m-sequence, configurable length (127, 511, or 1023 chips)
- **Input modulation**: QPSK/SOQPSK (complex samples)
- **Output**: Spread spectrum signal (complex samples)

## Blocks

### dsss_spreader_cc

**DSSS Spreader Block** - Transmitter side

Accepts complex symbol stream at symbol rate and spreads each symbol using a PN sequence to produce a wideband signal at chip rate.

**Parameters:**
- `pn_sequence`: Vector of 0/1 chip values (internally mapped to -1/+1 for correlation)
- `chips_per_symbol`: Number of chips per symbol (default: 42)

**Input:**
- Complex symbols at symbol rate (e.g., 12 ksps)

**Output:**
- Spread spectrum signal at chip rate (e.g., 500 ksps)

**Features:**
- Real-time streaming operation
- Thread-safe parameter updates
- Seamless PN sequence wraparound

### dsss_despreader_cc

**DSSS Despreader/Correlator Block** - Receiver side

Accepts spread spectrum signal at chip rate and despreads it to recover the original symbols at symbol rate.

**Parameters:**
- `pn_sequence`: PN sequence (must match transmitter)
- `chips_per_symbol`: Number of chips per symbol (default: 42)
- `correlation_threshold`: Threshold for valid correlation (default: 0.7)
- `timing_error_tolerance`: Samples tolerance for timing errors (default: 2)

**Input:**
- Spread spectrum signal at chip rate (e.g., 500 ksps)

**Outputs:**
- 0: Despread complex symbols at symbol rate (e.g., 12 ksps)
- 1: Lock status (float, 0.0 = unlocked, 1.0 = locked)
- 2: SNR estimate (float, dB)

**Features:**
- PN code correlation and despreading
- Timing synchronization with early-late gate
- Code phase acquisition
- Lock detection
- SNR estimation
- State machine: Acquisition → Tracking → Locked

## Verification

- **Barker-13:** The hierarchical blocks `mod_dsss`/`demod_dsss` use the standard Barker-13 code `{1,1,1,1,1,0,0,1,1,0,1,0,1}`. The base spreader/despreader map 0 to -1 and 1 to +1 so that correlation matches standard DSSS.
- **BER simulation:** [examples/dsss_ber_simulation.py](../examples/dsss_ber_simulation.py) runs spreader -> AWGN -> despreader and plots BER vs SNR for N=64, 128, 256, overlaying the theoretical curve 0.5*erfc(sqrt(N*Es/N0/2)); simulated curves match theory within simulation noise.
- **Enhancements:** Timing recovery, lock detection, adaptive threshold, and coarse-to-fine acquisition are additive and do not change the base correlation/despreading behaviour.

## PN Sequence Generator

The `pn_sequence_generator` utility provides functions for generating standard PN sequences:

### M-Sequences (Maximal Length Sequences)

- `generate_msequence_127()`: 127-chip m-sequence (polynomial [7,1])
- `generate_msequence_511()`: 511-chip m-sequence (polynomial [9,4])
- `generate_msequence_1023()`: 1023-chip m-sequence (polynomial [10,3])

### Gold Codes

- `generate_gold_code(poly1_taps, poly2_taps, code_number, length)`: Generate a specific Gold code
- `generate_gold_code_family(length, num_codes)`: Generate a family of Gold codes for multi-user operation

### Usage Example

```python
from gnuradio import qradiolink
from gnuradio.qradiolink import pn_sequence_generator

# Generate 127-chip m-sequence
pn_seq = pn_sequence_generator.generate_msequence_127()

# Create spreader
spreader = qradiolink.dsss_spreader_cc(pn_seq, chips_per_symbol=42)

# Create despreader
despreader = qradiolink.dsss_despreader_cc(pn_seq, chips_per_symbol=42, 
                                          correlation_threshold=0.7,
                                          timing_error_tolerance=2)
```

## Soft-Decision Decoding Support

The despreader provides a per-symbol reliability metric for soft-decision FEC (e.g. LDPC):

- **`get_last_soft_metric()`**: Returns normalized correlation magnitude in [0, ~1]. Poll after each symbol (or use the value corresponding to the last output symbol). Use this as a soft input to a soft-decision LDPC decoder or other FEC.

## Automatic Frequency Control (AFC) Support

The despreader estimates residual frequency error from the phase drift of the prompt correlation:

- **`get_frequency_error()`**: Returns estimated frequency error in **rad/symbol**. Use this to drive an upstream NCO or rotator for AFC. The estimate is low-pass filtered; apply correction in your carrier recovery loop.

## Parallel Code Search (Faster Acquisition)

Acquisition uses a **coarse-to-fine** code phase search instead of a full linear sweep:

- **Coarse search**: Evaluates correlation at a subset of phases (e.g. 32 bins) to find the best coarse bin.
- **Fine search**: Refines the phase in a small window around the best coarse bin.

This reduces the number of correlation operations per symbol during acquisition and speeds up lock.

## Adaptive Correlation Threshold

Lock detection uses an **adaptive threshold** derived from the running correlation statistics:

- Threshold = max(0.2, user_threshold * (correlation_avg / correlation_peak)).
- In weak channels the threshold relaxes; in strong channels it tightens, improving acquisition and lock stability.

## State Machine

The despreader implements a three-state machine:

1. **STATE_ACQUISITION**: Searching for code alignment
   - Coarse-to-fine search over code phases
   - Finds best correlation peak
   - Transitions to TRACKING when threshold exceeded

2. **STATE_TRACKING**: Tracking code phase
   - Uses early-late gate for timing recovery
   - Monitors correlation quality
   - Transitions to LOCKED after sustained good correlation

3. **STATE_LOCKED**: Locked and despreading
   - Maintains code synchronization
   - Provides despread symbols
   - Falls back to TRACKING if correlation degrades

## Performance Characteristics

### Acquisition Time

- Typical: < 100 ms at moderate SNR (> 5 dB)
- Maximum: Depends on code length and search strategy
- Acquisition timeout: 10,000 samples

### Lock Detection

- Lock threshold: 10 consecutive good correlations
- Correlation threshold: Configurable (default 0.7), applied adaptively (see Adaptive Correlation Threshold)
- Lock counter: Increments on good correlation, decrements on poor

### Timing Recovery

- Early-late gate: ±1 sample offset
- Timing error tolerance: Configurable (default ±2 samples)
- Damping factor: 0.1 for stability

### SNR Estimation

- Signal power: Estimated from correlation peak
- Noise power: Estimated from correlation sidelobes
- Update rate: Exponential moving average (95% old, 5% new)

## Using Lock Status and SNR Estimate Outputs

The `dsss_despreader_cc` block provides two additional float outputs that can be connected to various GNU Radio blocks for monitoring, control, and analysis:

### Lock Status Output (float: 0.0 = unlocked, 1.0 = locked)

**Visualization and Monitoring:**
- **QT GUI Number Sink**: Display current lock status as a numeric value
- **QT GUI Time Sink**: Plot lock status over time to visualize acquisition and lock transitions
- **QT GUI Range**: Use as an indicator (0.0 = red/unlocked, 1.0 = green/locked)

**Logging and Recording:**
- **File Sink**: Log lock status to a file for post-processing analysis
- **Message Debug**: Print lock status changes to console for debugging

**Control and Triggering:**
- **Threshold**: Trigger actions when lock is achieved (e.g., enable data processing)
- **Multiply Const**: Scale the lock status for use with other blocks
- **Stream to Tagged Stream**: Tag the data stream when lock state changes

**Statistics:**
- **Probe Signal**: Sample lock status values programmatically
- **Moving Average**: Smooth lock status for hysteresis control

### SNR Estimate Output (float: dB)

**Visualization and Monitoring:**
- **QT GUI Number Sink**: Display current SNR value in dB
- **QT GUI Time Sink**: Plot SNR over time to monitor signal quality
- **QT GUI Waterfall Sink**: Visualize SNR alongside frequency/time data

**Logging and Recording:**
- **File Sink**: Record SNR measurements for performance analysis
- **Message Debug**: Print SNR values to console

**Adaptive Processing:**
- **Multiply Const**: Scale SNR for gain control applications
- **Threshold**: Trigger actions based on SNR thresholds (e.g., low SNR warning)
- **Conditional Blocks**: Adjust processing parameters based on SNR (adaptive modulation, coding rate, etc.)

**Statistics and Analysis:**
- **Moving Average**: Smooth SNR values to reduce noise in measurements
- **Probe Signal**: Sample SNR values for programmatic control
- **Stream to Tagged Stream**: Tag data when SNR crosses specific thresholds

### Common Use Cases

**1. Lock Indicator Display:**
```
DSSS Despreader → Lock Status → QT GUI Number Sink
```
Shows 0.0 (unlocked) or 1.0 (locked) in real-time.

**2. SNR Monitoring:**
```
DSSS Despreader → SNR Estimate → QT GUI Time Sink
```
Plots SNR over time to monitor signal quality and fading conditions.

**3. Adaptive Gain Control:**
```
DSSS Despreader → SNR Estimate → Threshold → Multiply Const (for AGC)
```
Automatically adjusts gain based on received signal quality.

**4. Data Logging:**
```
DSSS Despreader → Lock Status → File Sink (lock.log)
DSSS Despreader → SNR Estimate → File Sink (snr.log)
```
Records lock status and SNR measurements for post-processing analysis.

**5. Conditional Processing:**
```
DSSS Despreader → Lock Status → Stream to Tagged Stream → (process only when locked)
```
Only processes data when the despreader has achieved lock, improving system reliability.

**6. Performance Monitoring:**
```
DSSS Despreader → SNR Estimate → Moving Average → QT GUI Number Sink
```
Displays smoothed SNR values for better readability.

**Note:** Both outputs are float streams at the symbol rate (not chip rate), matching the despread symbols output rate. This ensures proper synchronization when connecting to downstream blocks.

**Note for GDSS Blocks:** The `gdss_despreader_cc` block provides identical Lock Status and SNR Estimate outputs with the same characteristics and can be used with the same downstream blocks as described above.

## Integration with Existing System

### Transmit Chain

```
Audio → Opus → Encrypt → LDPC → PSK Mod → DSSS Spreader → RRC Filter → USRP
```

### Receive Chain

```
USRP → RRC Filter → DSSS Despreader → PSK Demod → LDPC Decode → Decrypt → Opus → Audio
```

### Compatibility

- Works with SOQPSK modulator/demodulator
- Compatible with LDPC encoder/decoder
- Proper sample rate handling
- Tagged stream support for frame boundaries

## Example Flowgraphs

### Basic Loopback Test

```
Signal Source → DSSS Spreader → Channel (AWGN) → DSSS Despreader → Constellation Sink
```

### Performance Test

```
Signal Source → DSSS Spreader → AWGN Channel (variable SNR) → DSSS Despreader → BER Measurement
```

### Multi-User Test

```
Multiple Spreaders (different Gold codes) → Combiner → Multiple Despreaders → Verify Isolation
```

## Known Limitations

1. **Frequency Offset**: Limited tolerance for frequency offset between transmitter and receiver
   - Recommended: Use frequency offset compensation (Costas loop) for offsets > 500 Hz
   - Current implementation assumes carrier recovery handled upstream

2. **Timing Drift**: Tracks timing drift within ±2 samples (configurable)
   - For larger drift, increase `timing_error_tolerance` parameter

3. **Code Length**: Maximum code length limited by correlation buffer size
   - Current implementation supports up to 1023 chips
   - Can be extended if needed

## Future Enhancements

- Enhanced multipath handling
- Optional stream output for soft-decision metrics (currently available via `get_last_soft_metric()`)

## References

- Direct Sequence Spread Spectrum (DSSS) theory
- Gold code generation and properties
- M-sequence (maximal length sequence) theory
- Early-late gate timing recovery


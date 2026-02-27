# GDSS Spreader and Despreader Blocks

## Overview

This document describes the Gaussian-Distributed Spread-Spectrum (GDSS) blocks for gr-qradiolink. These blocks provide spreading and despreading capabilities using Gaussian-distributed sequences instead of binary PN sequences, offering improved spectral properties and better interference rejection.

## Key Differences from DSSS

Unlike traditional DSSS which uses binary PN sequences (+1/-1), GDSS uses Gaussian-distributed spreading sequences:

- **Better spectral properties**: Smoother spectral characteristics with reduced sidelobes
- **Improved interference rejection**: Better performance in multi-user and interference-limited scenarios
- **Continuous values**: Gaussian sequences use continuous floating-point values rather than discrete binary values
- **Configurable variance**: Control over the Gaussian distribution variance for different applications

## System Specifications

- **Chip rate**: 500 kHz (500,000 chips per second)
- **Symbol rate**: 12 kHz (12,000 symbols per second)
- **Spreading factor**: ~42 chips per symbol (500k / 12k)
- **Spreading sequence**: Gaussian-distributed sequence, configurable length (default: 127 chips)
- **Input modulation**: QPSK/SOQPSK (complex samples)
- **Output**: Spread spectrum signal (complex samples)

## Blocks

### gdss_spreader_cc

**GDSS Spreader Block** - Transmitter side

Accepts complex symbol stream at symbol rate and spreads each symbol using a Gaussian-distributed spreading sequence to produce a wideband signal at chip rate.

**Parameters:**
- `sequence_length`: Length of the Gaussian spreading sequence (default: 127)
- `chips_per_symbol`: Number of chips per symbol (default: 42)
- `variance`: Variance of the Gaussian distribution (default: 1.0)
- `seed`: Random seed for sequence generation (0 = use current time)

**Input:**
- Complex symbols at symbol rate (e.g., 12 ksps)

**Output:**
- Spread spectrum signal at chip rate (e.g., 500 ksps)

**Features:**
- Real-time streaming operation
- Thread-safe parameter updates
- Automatic sequence generation from Gaussian distribution
- Runtime sequence regeneration support

**Sequence Generation:**

The spreading sequence is generated from a Gaussian distribution with:
- Mean: 0.0
- Variance: Configurable (default: 1.0)
- Distribution: Normal (Gaussian) distribution

The sequence is generated once at initialization and can be regenerated using `regenerate_sequence()` if needed.

### gdss_despreader_cc

**GDSS Despreader/Correlator Block** - Receiver side

Accepts spread spectrum signal at chip rate and despreads it to recover the original symbols at symbol rate using correlation with the Gaussian spreading sequence.

**Parameters:**
- `spreading_sequence`: Gaussian spreading sequence vector (must match transmitter)
- `chips_per_symbol`: Number of chips per symbol (must match transmitter, default: 42)
- `correlation_threshold`: Threshold for valid correlation (default: 0.7)
- `timing_error_tolerance`: Samples tolerance for timing errors (default: 2)

**Input:**
- Spread spectrum signal at chip rate (e.g., 500 ksps)

**Outputs:**
- 0: Despread complex symbols at symbol rate (e.g., 12 ksps)
- 1: Lock status (float, 0.0 = unlocked, 1.0 = locked)
- 2: SNR estimate (float, dB)

**Features:**
- Gaussian sequence correlation and despreading
- Timing synchronization with early-late gate
- Code phase acquisition
- Lock detection
- SNR estimation
- State machine: Acquisition → Tracking → Locked

**Important:** The spreading sequence must be regenerated at the receiver using the same parameters (sequence_length, variance, seed) as the transmitter. The GRC block definition includes code to regenerate the sequence automatically.

## Usage Example

### Python Script

```python
from gnuradio import qradiolink
import numpy as np

# Parameters (must match between transmitter and receiver)
sequence_length = 127
chips_per_symbol = 42
variance = 1.0
seed = 12345  # Use same seed for tx and rx

# Generate Gaussian spreading sequence
np.random.seed(seed)
spreading_sequence = np.random.normal(0.0, np.sqrt(variance), sequence_length).tolist()

# Create spreader
spreader = qradiolink.gdss_spreader_cc(
    sequence_length=sequence_length,
    chips_per_symbol=chips_per_symbol,
    variance=variance,
    seed=seed
)

# Create despreader (using the same sequence)
despreader = qradiolink.gdss_despreader_cc(
    spreading_sequence=spreading_sequence,
    chips_per_symbol=chips_per_symbol,
    correlation_threshold=0.7,
    timing_error_tolerance=2
)
```

### GNU Radio Companion (GRC)

The GRC blocks automatically handle sequence generation. For the despreader, the GRC template regenerates the sequence using the same parameters as the transmitter:

1. Set matching parameters (sequence_length, variance, seed) in both spreader and despreader
2. The despreader will automatically regenerate the sequence using NumPy
3. Ensure the seed matches exactly between transmitter and receiver

## State Machine

The despreader implements a three-state machine identical to DSSS:

1. **STATE_ACQUISITION**: Searching for code alignment
   - Searches over all code phases
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
- Maximum: Depends on sequence length and search strategy
- Acquisition timeout: 10,000 samples

### Lock Detection

- Lock threshold: 10 consecutive good correlations
- Correlation threshold: Configurable (default 0.7)
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

The `gdss_despreader_cc` block provides two additional float outputs identical to `dsss_despreader_cc`:

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
GDSS Despreader → Lock Status → QT GUI Number Sink
```
Shows 0.0 (unlocked) or 1.0 (locked) in real-time.

**2. SNR Monitoring:**
```
GDSS Despreader → SNR Estimate → QT GUI Time Sink
```
Plots SNR over time to monitor signal quality and fading conditions.

**3. Adaptive Gain Control:**
```
GDSS Despreader → SNR Estimate → Threshold → Multiply Const (for AGC)
```
Automatically adjusts gain based on received signal quality.

**4. Data Logging:**
```
GDSS Despreader → Lock Status → File Sink (lock.log)
GDSS Despreader → SNR Estimate → File Sink (snr.log)
```
Records lock status and SNR measurements for post-processing analysis.

**5. Conditional Processing:**
```
GDSS Despreader → Lock Status → Stream to Tagged Stream → (process only when locked)
```
Only processes data when the despreader has achieved lock, improving system reliability.

**6. Performance Monitoring:**
```
GDSS Despreader → SNR Estimate → Moving Average → QT GUI Number Sink
```
Displays smoothed SNR values for better readability.

**Note:** Both outputs are float streams at the symbol rate (not chip rate), matching the despread symbols output rate. This ensures proper synchronization when connecting to downstream blocks.

## Integration with Existing System

### Transmit Chain

```
Audio → Opus → Encrypt → LDPC → PSK Mod → GDSS Spreader → RRC Filter → USRP
```

### Receive Chain

```
USRP → RRC Filter → GDSS Despreader → PSK Demod → LDPC Decode → Decrypt → Opus → Audio
```

### Compatibility

- Works with SOQPSK modulator/demodulator
- Compatible with LDPC encoder/decoder
- Proper sample rate handling
- Tagged stream support for frame boundaries
- Can be used alongside DSSS blocks in the same system

## Parameter Selection Guidelines

### Sequence Length

- **127 chips**: Good balance of performance and complexity (default)
- **511 chips**: Better correlation properties, longer acquisition time
- **1023 chips**: Best correlation properties, longest acquisition time

Choose based on:
- Required acquisition time
- Available processing power
- Desired correlation properties

### Variance

- **1.0**: Standard variance (default)
- **Lower values (< 1.0)**: Reduced spreading gain, smoother spectrum
- **Higher values (> 1.0)**: Increased spreading gain, more dynamic range

Choose based on:
- Desired spreading gain
- Signal dynamic range requirements
- Interference environment

### Chips per Symbol

- **42**: Standard for 500 kHz chip rate, 12 kHz symbol rate (default)
- **Higher values**: More spreading gain, wider bandwidth
- **Lower values**: Less spreading gain, narrower bandwidth

Choose based on:
- Available bandwidth
- Required spreading gain
- Symbol rate requirements

## Example Flowgraphs

### Basic Loopback Test

```
Signal Source → GDSS Spreader → Channel (AWGN) → GDSS Despreader → Constellation Sink
```

### Performance Test

```
Signal Source → GDSS Spreader → AWGN Channel (variable SNR) → GDSS Despreader → BER Measurement
```

### Comparison Test (GDSS vs DSSS)

```
Signal Source → Split → GDSS Spreader ──┐
                          DSSS Spreader ─┼→ Combiner → GDSS Despreader → Compare
                                         └→ DSSS Despreader ──┘
```

## Advantages of GDSS over DSSS

1. **Spectral Properties**: Gaussian sequences produce smoother spectral characteristics with reduced sidelobes compared to binary PN sequences
2. **Interference Rejection**: Better performance in multi-user scenarios and interference-limited environments
3. **Correlation Properties**: Gaussian sequences can provide better autocorrelation properties for certain applications
4. **Flexibility**: Configurable variance allows tuning for different scenarios

## Known Limitations

1. **Sequence Synchronization**: Transmitter and receiver must use identical sequences (same seed and variance)
   - Solution: Use deterministic seed values or share sequence parameters via control channel

2. **Frequency Offset**: Limited tolerance for frequency offset between transmitter and receiver
   - Recommended: Use frequency offset compensation (Costas loop) for offsets > 500 Hz
   - Current implementation assumes carrier recovery handled upstream

3. **Timing Drift**: Tracks timing drift within ±2 samples (configurable)
   - For larger drift, increase `timing_error_tolerance` parameter

4. **Sequence Length**: Maximum sequence length limited by correlation buffer size
   - Current implementation supports practical lengths up to 1023 chips
   - Can be extended if needed

5. **Memory Usage**: Gaussian sequences require floating-point storage (vs binary for DSSS)
   - Slightly higher memory usage compared to binary PN sequences

## Future Enhancements

- Soft-decision decoding support
- Automatic Frequency Control (AFC)
- Enhanced multipath handling
- Parallel code search for faster acquisition
- Adaptive correlation threshold
- Sequence sharing mechanisms for multi-user scenarios
- Optimized sequence generation algorithms

## References

- Gaussian-Distributed Spread-Spectrum (GDSS) theory
- Spread spectrum communication principles
- Correlation-based synchronization techniques
- Early-late gate timing recovery
- Signal processing for spread spectrum systems

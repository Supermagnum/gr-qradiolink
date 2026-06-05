# gr-ident ZeroMQ Integration

gr-qradiolink can switch modulation and demodulation based on **gr-ident** preamble results published over ZeroMQ. This matches the wire format documented in the [gr-ident zeromq-protocol](https://github.com/Supermagnum/gr-ident/blob/main/docs/zeromq-protocol.md) and the [SDR-repeater ZMQ reference](https://github.com/Supermagnum/SDR-repeater/blob/main/zeromq-messages.md) (Section 6).

For a full file and function index, see **[CODE_MAP.md](CODE_MAP.md#gr-ident-zmq-mode-routing-gnu-radio-310)**.

---

## What gr-ident sends (receive path)

After Golay decode, gr-ident publishes on a PUB socket (flowgraph binds, clients connect):

| Item | Value |
|------|--------|
| Endpoint | `tcp://127.0.0.1:5560` (or `ipc:///run/ht-module/grident` on repeaters) |
| Pattern | PUB / SUB |
| Frames | Multipart: `[topic, json_utf8]` |
| Topic | `grident`, or `grident.A` ... `grident.D`, or `grident.<band>` |
| JSON | `{"mode_id":N,"digital":bool,"encrypted":bool,"metadata_present":bool}` |

Example:

```json
{"mode_id":20,"digital":false,"encrypted":false,"metadata_present":false}
```

gr-ident does **not** send arbitrary "set mode" commands on the air chain. On **transmit**, mode is configured in the gr-ident flowgraph (`PreambleOnPtt` `mode_id`); ZMQ port **5561** carries **PTT on/off** only (`grident.tx`), not mode changes.

---

## gr-qradiolink blocks (GNU Radio 3.10, branch `main`)

| GRC block | Python class | Role |
|-----------|--------------|------|
| `qradiolink_grident_preamble_sub` | `qradiolink.grident_preamble_sub` | SUB to `:5560`, forwards frames on `preamble_out` |
| `qradiolink_grident_mode_control` | `qradiolink.grident_mode_control` | Parses JSON, publishes route dict on `route_out` |

Build requirement: `libzmq3-dev` for `grident_preamble_sub`. `grident_mode_control` builds without ZMQ.

### Message port wiring

```
grident_preamble_sub:preamble_out  -->  grident_mode_control:preamble_in
grident_mode_control:route_out     -->  (your selector / callback / second MSG block)
```

### `route_out` PMT dictionary keys

| Key | Type | Meaning |
|-----|------|---------|
| `mode_id` | uint | gr-ident mode 0-511 |
| `digital` | bool | Bit 11 (digital vs analog) |
| `encrypted` | bool | Bit 10 |
| `metadata_present` | bool | Bit 9 |
| `route_valid` | bool | Known mapping in gr-qradiolink |
| `active_demod_route` | uint | Internal `modem_route` enum value |
| `active_mod_route` | uint | Internal mod route enum value |
| `demod_block` | string | Factory name, e.g. `demod_nbfm`, `ysf_decoder` |
| `mod_block` | string | Factory name, e.g. `mod_m17`, `mod_dmr` |

Use `demod_block` / `mod_block` to enable the correct block bank in your application or a custom Python message handler.

### Python example (manual JSON)

```python
from gnuradio import gr, blocks, qradiolink
import pmt

tb = gr.top_block()
ctrl = qradiolink.grident_mode_control()
null = blocks.null_sink(gr.sizeof_char)

tb.msg_connect(ctrl, "route_out", null, "in")  # replace with your handler

# Inject gr-ident JSON (as if from ZMQ SUB)
json_body = '{"mode_id":20,"digital":false,"encrypted":false,"metadata_present":false}'
ctrl.message_port_pub(pmt.intern("preamble_in"), pmt.intern(json_body))

tb.start()
print("demod:", ctrl.demod_block())  # expect demod_nbfm
tb.stop()
```

### Python example (live ZMQ)

```python
from gnuradio import gr, qradiolink

tb = gr.top_block()
sub = qradiolink.grident_preamble_sub(
    endpoint="tcp://127.0.0.1:5560",
    topic_filter="grident",
    bind_socket=False,
)
ctrl = qradiolink.grident_mode_control()

tb.msg_connect(sub, "preamble_out", ctrl, "preamble_in")
# Connect ctrl route_out to your logic

tb.start()
# Run gr-ident detect flowgraph or repeater publishing preambles
tb.stop()
```

---

## Switching demodulators in a flowgraph

gr-qradiolink does not ship a single "switch" hier block on `main`. Typical patterns:

1. **Message handler (Python):** Subscribe to `route_out`, call `tb.lock()`, disconnect/reconnect the active `demod_*` branch, `tb.unlock()`.
2. **Multiple banks + valve:** One `demod_nbfm`, one `demod_dmr`, etc., with `blocks valve` or manual connection driven by `demod_block` string.
3. **Application-level:** Use route dict only in your UI/service; rebuild flowgraph when mode changes.

On branch **`gnuradio4`**, header blocks `ModDemodSwitchRx` / `ModDemodSwitchTx` plus `GrIdentModeControl` provide in-process switching for development (see [CODE_MAP.md](CODE_MAP.md#gnu-radio-4-branch-gnuradio4)).

---

## Mode ID quick reference

Aligned with gr-ident `PROFILE_BY_MODE_ID` (subset). See [gr-ident README mode table](https://github.com/Supermagnum/gr-ident/blob/main/README.md#mode-id-table) for the full list.

| mode_id | Service / mode | gr-qradiolink `demod_block` |
|---------|----------------|----------------------------|
| 20 | NFM 12.5 kHz | `demod_nbfm` |
| 100-102, 106, 109 | DMR family | `demod_dmr` |
| 103, 115 | D-STAR | `dstar_decoder` |
| 104, 105, 114 | C4FM / Fusion | `ysf_decoder` |
| 107 | NXDN | `demod_nxdn` |
| 108 | dPMR | `demod_dpmr` |
| 120-121 | M17 | `demod_m17` |
| 122-124 | FreeDV | `demod_freedv` |
| 150-151 | AX.25 / APRS | `demod_2fsk` |
| 158 | PSK31 | `demod_bpsk` |
| 159 | RTTY | `demod_2fsk` |

If `route_valid` is false, the mode ID is not mapped; do not route audio to a demod bank.

---

## Related documentation

- [CODE_MAP.md](CODE_MAP.md) -- function and file index
- [DEPENDENCIES.md](../DEPENDENCIES.md) -- `libzmq` package names
- [PTT_CONTROL.md](PTT_CONTROL.md) -- hardware/software PTT (separate from gr-ident `:5561` gating)
- [gr-ident zeromq-protocol.md](https://github.com/Supermagnum/gr-ident/blob/main/docs/zeromq-protocol.md) -- canonical wire spec

---

## Tests

```bash
cd build
cmake ..
make test_grident_zmq
./tests/test_grident_zmq
```

Or: `ctest -R test_grident_zmq --output-on-failure`

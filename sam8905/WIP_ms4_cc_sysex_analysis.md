# MS4 CC0-4 and SysEx Analysis

## Summary

**Can sound programs be sent via SysEx?** No. The MS4 SysEx implementation only supports a simple voice enable/disable command. There is no bulk dump, parameter editing, or program transfer capability.

**Can CC0-4 control sound parameters?** No. Only CC1 (Modulation Wheel) is recognized. CC0, CC2, CC3, CC4 are not in the CC lookup table and are silently ignored.

## CC Handling Details

### CC Lookup Table (CODE:D405)

The firmware uses a lookup table at CODE:D405 to map CC numbers to handler indices:

```
06 60 61 64 65 01 05 41 07 0A 40 42 43 7B 7C 7D 7E 7F
```

Decoded: CC 6, 96, 97, 100, 101, 1, 5, 65, 7, 10, 64, 66, 67, 123, 124, 125, 126, 127

### CC0-4 Status

| CC# | Name | Status | Notes |
|-----|------|--------|-------|
| 0 | Bank Select MSB | **NOT IMPLEMENTED** | Not in lookup table, silently ignored |
| 1 | Modulation Wheel | Implemented | Handler at 0xC2CA, affects LFO depth |
| 2 | Breath Controller | **NOT IMPLEMENTED** | Not in lookup table, silently ignored |
| 3 | Undefined | **NOT IMPLEMENTED** | Not in lookup table, silently ignored |
| 4 | Foot Controller | **NOT IMPLEMENTED** | Not in lookup table, silently ignored |

### CC Dispatch Logic (CODE:C1A0)

```
cc_dispatch:
  1. Call table_lookup(CC_TABLE, cc_number)
  2. If result == 0xFF: return (unrecognized CC)
  3. If result >= 5: jump to cc_switch_handler (performance CCs)
  4. If result < 5: use RPN jump table (CC 6, 96, 97, 100, 101)
```

Unrecognized CC numbers (including 0, 2, 3, 4) are silently discarded with no error or feedback.

## SysEx Implementation

### State Machine (CODE:C689-C6D9)

The SysEx parser is a simple 5-state machine:

| State | Trigger | Action |
|-------|---------|--------|
| 0 | Initial | Fall through to channel message processing |
| 1 | F0 received | Check next byte for manufacturer ID 0x31 |
| 2 | ID matched | Store first data byte to 0x1D18 (mask1) |
| 3 | mask1 stored | Store second data byte to 0x1D19 (mask2), call sysex_voice_enable |
| 4 | Complete/Error | Ignore remaining bytes until next status byte |

### Supported SysEx Command

**Voice Enable/Disable** - the ONLY supported SysEx command:

```
F0 31 <mask1> <mask2> F7
```

| Byte | Value | Description |
|------|-------|-------------|
| F0 | - | SysEx Start |
| 31 | - | Manufacturer ID (Solton/Hohner) |
| mask1 | 0x00-0x3F | Voice enable bits for voices 0-4 (bits 0-4), bits 6-7 must be 0 |
| mask2 | 0x00-0x3F | Voice enable bits for voices 5-9 (bits 0-4), bits 6-7 must be 0 |
| F7 | - | SysEx End |

### sysex_voice_enable Function (CODE:B59B)

```
For each voice 0-4:
  If mask2 & (1 << voice): write 0xC0 to XRAM[0x1D1A + voice]
  Else: write 0x00 to XRAM[0x1D1A + voice]

For each voice 0-4:
  If mask1 & (1 << voice): write 0xC0 to XRAM[0x1D20 + voice]
  Else: write 0x00 to XRAM[0x1D20 + voice]
```

This appears to control which voice slots are active/muted, but does NOT modify sound parameters or programs.

### What SysEx Does NOT Support

- **Program dump/load**: No way to transfer sound program data
- **Parameter editing**: No way to modify envelope, filter, or oscillator parameters
- **Bulk dump**: No dump request or response capability
- **Bank switching**: No bank select functionality
- **Identity request**: Standard F0 7E ... F7 identity request not implemented

## RPN Implementation

### Supported RPNs

Only two standard RPNs are implemented:

| RPN MSB | RPN LSB | Name | Handler | Range |
|---------|---------|------|---------|-------|
| 0 | 0 | Pitch Bend Range | CODE:C1CE | 0-12 semitones (clamped) |
| 0 | 1 | Fine Tuning | CODE:C1CE | Centered at 64, calls 0x9B16 |

### RPN State Variables

| Address | Purpose |
|---------|---------|
| 0x1D15 | RPN MSB value |
| 0x1D16 | RPN LSB value |
| 0x1D17 | Computed tuning offset |
| 0x11E5 | Pitch bend range (semitones) |

### Data Entry Handler (CC#6, CODE:C1CE)

```
If RPN == 0:0 (Pitch Bend Range):
  Store min(value, 12) to 0x11E5

If RPN == 0:1 (Fine Tuning):
  Compute (value - 64) * 2
  Store to 0x1D17
  Call tuning update function (0x9B16)
```

## MIDI Output Capability

The ISR at CODE:B630 has TX buffer handling code, but:
- No functions were found that queue SysEx responses
- No bulk dump request handling exists
- The TX functionality appears unused or reserved for MIDI Thru

## Conclusions

1. **No program editing via MIDI**: Sound programs are stored in ROM and cannot be modified or uploaded via SysEx.

2. **CC0-4 cannot control parameters**: Only CC1 (Mod Wheel) affects sound. The others are ignored.

3. **Minimal SysEx**: Only voice mute/unmute is supported. This may have been used for a multi-timbral mute function in the original accordion.

4. **Standard RPN only**: Pitch bend range and fine tuning are the only data entry targets.

5. **No real-time parameter control**: Unlike modern synthesizers, the MS4 has no CC-controllable filter cutoff, resonance, envelope times, etc.

## Potential Extensions (If Modifying Firmware)

To add real-time parameter control, one would need to:

1. Add CC numbers (e.g., CC74 for filter cutoff) to the lookup table at CODE:D405
2. Write handlers that modify the appropriate D-RAM voice parameters
3. Ensure the periodic_voice_update function respects these changes

For SysEx program loading:
1. Extend the state machine beyond state 4
2. Parse program data format (header + algorithm + parameters)
3. Write to the appropriate RAM/SAM areas
4. This would require significant reverse engineering of the program data format

## Related Documents

- WIP_solton_ms4.md - Main firmware analysis
- sam8905_programmers_guide.md - SAM8905 D-RAM parameter reference

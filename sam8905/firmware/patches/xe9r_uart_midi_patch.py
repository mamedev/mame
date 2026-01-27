#!/usr/bin/env python3
"""
XE9R UART MIDI Patch

Patches the XE9R firmware to enable UART-based MIDI input instead of
the parallel P1 port input used in production.

Changes:
- Adds UART ISR at 0xA050 that reads SBUF
- Enables Timer 2 (TR2) for baud rate generation
- Sets SCON for UART Mode 1 with receive enable
- Enables serial interrupt (ES)

Note: The serial vector at 0x0023 is in the middle of "SILENCE " string
data that gets copied to IRAM during init. However, IRAM 0x23-0x24 get
overwritten by mainloop anyway, so patching CODE 0x0023 is safe.

Baud rate: 31250 (MIDI standard) with 16 MHz crystal

Key differences from XE9L:
- Buffer base: 0x136B (vs 0x1361)
- Init addresses: SCON@0x9361, T2CON@0x9395, IE@0x93AF
- Uses inline DPTR arithmetic (no helper function available)
"""

import sys
from pathlib import Path


def create_patch():
    """Return list of (address, bytes) patches."""
    patches = []

    # --- Vector area patches ---

    # 0x0023: Serial vector jumps to new UART ISR at 0xA085
    # (overwrites part of "SILENCE " string, but that's OK)
    patches.append((0x0023, bytes([0x02, 0xA0, 0x85])))  # LJMP 0xA085

    # --- Initialization patches ---

    # 0x9361: SCON value 0x00 -> 0x5C (UART mode 1, REN=1, TI=1)
    patches.append((0x9361, bytes([0x5C])))

    # 0x9395: T2CON value 0x00 -> 0x34 (enable TR2, RCLK, TCLK)
    patches.append((0x9395, bytes([0x34])))

    # 0x93AF: IE value 0x89 -> 0x99 (enable ES serial interrupt)
    patches.append((0x93AF, bytes([0x99])))

    # --- New code at 0xA050 ---

    new_code = bytearray()

    # Standalone UART handler (0xA050-0xA084) - 53 bytes
    # Based on XE9R's INT0 handler - handles both RI and TI
    # Uses XE9R's buffer: 0x136B (data), IRAM 0xB8 (count), IRAM 0xB9 (write ptr)
    # Uses IRAM 0x23 as temp storage for SBUF byte
    # Uses inline DPTR arithmetic (no helper function available)
    uart_handler = bytes([
        # 0xA050: Check RI
        0x30, 0x98, 0x2F,  # JNB RI, clear_ti (-> 0xA082)
        0xC2, 0x98,        # CLR RI
        0xE5, 0x99,        # MOV A, SBUF
        0xB4, 0xFF, 0x02,  # CJNE A, #0xFF, check_buf (-> 0xA05C)
        0x80, 0x26,        # SJMP clear_ti (-> 0xA082)
        # 0xA05C: check_buf - save byte and check buffer space
        0xF5, 0x23,        # MOV 0x23, A (save SBUF byte)
        0x78, 0xB8,        # MOV R0, #0xB8 (buffer count in IRAM)
        0xE6,              # MOV A, @R0
        0xB4, 0xFF, 0x02,  # CJNE A, #0xFF, store (-> 0xA066)
        0x80, 0x1C,        # SJMP clear_ti (-> 0xA082)
        # 0xA066: store - increment count and store byte
        0x06,              # INC @R0 (count++)
        0x78, 0xB9,        # MOV R0, #0xB9 (write pointer in IRAM)
        0xE6,              # MOV A, @R0 (get write index)
        0x90, 0x13, 0x6B,  # MOV DPTR, #0x136B (buffer base - XE9R specific)
        # Inline DPTR += A (no helper available)
        0x25, 0x82,        # ADD A, DPL
        0xF5, 0x82,        # MOV DPL, A
        0xE4,              # CLR A
        0x35, 0x83,        # ADDC A, DPH
        0xF5, 0x83,        # MOV DPH, A
        0xE5, 0x23,        # MOV A, 0x23 (get saved byte)
        0xF0,              # MOVX @DPTR, A (store to buffer)
        0x78, 0xB9,        # MOV R0, #0xB9 (write pointer)
        0x06,              # INC @R0 (write_ptr++)
        0xE6,              # MOV A, @R0 (get new write ptr)
        0xB4, 0xFF, 0x02,  # CJNE A, #0xFF, clear_ti (wrap check -> 0xA082)
        0xE4,              # CLR A
        0xF6,              # MOV @R0, A (write ptr = 0)
        # 0xA082: clear_ti - always clear TI and return
        0xC2, 0x99,        # CLR TI
        0x22,              # RET
    ])
    new_code.extend(uart_handler)

    # UART ISR wrapper (0xA085-0xA09F) - 27 bytes
    uart_isr = bytes([
        0xC0, 0xE0,        # PUSH ACC
        0xC0, 0xF0,        # PUSH B
        0xC0, 0x83,        # PUSH DPH
        0xC0, 0x82,        # PUSH DPL
        0xC0, 0xD0,        # PUSH PSW
        0x75, 0xD0, 0x12,  # MOV PSW,#0x12 (bank 2, same as INT0)
        0x12, 0xA0, 0x50,  # LCALL 0xA050 (UART handler)
        0xD0, 0xD0,        # POP PSW
        0xD0, 0x82,        # POP DPL
        0xD0, 0x83,        # POP DPH
        0xD0, 0xF0,        # POP B
        0xD0, 0xE0,        # POP ACC
        0x32,              # RETI
    ])
    new_code.extend(uart_isr)

    patches.append((0xA050, bytes(new_code)))

    return patches


def apply_patches(data: bytearray, patches: list) -> None:
    """Apply patches to binary data."""
    for addr, patch_bytes in patches:
        print(f"  0x{addr:04X}: {data[addr:addr+len(patch_bytes)].hex()} -> {patch_bytes.hex()}")
        data[addr:addr + len(patch_bytes)] = patch_bytes


def verify_original(data: bytes | bytearray) -> bool:
    """Verify this is the expected original firmware."""
    checks = [
        (0x0023, bytes([0x43, 0x45, 0x20])),              # Part of "SILENCE " (CE )
        (0x9361, bytes([0x00])),                          # Original SCON value
        (0x9395, bytes([0x00])),                          # Original T2CON value
        (0x93AF, bytes([0x89])),                          # Original IE value
        (0xA050, bytes([0xFF, 0xFF, 0xFF, 0xFF])),        # Free space
    ]

    for addr, expected in checks:
        actual = data[addr:addr + len(expected)]
        if actual != expected:
            print(f"Warning: Unexpected data at 0x{addr:04X}")
            print(f"  Expected: {expected.hex()}")
            print(f"  Actual:   {actual.hex()}")
            return False
    return True


def main():
    if len(sys.argv) < 2:
        print("Usage: xe9r_uart_midi_patch.py <input.bin> [output.bin]")
        print("")
        print("If output is not specified, creates <input>_uart.bin")
        sys.exit(1)

    input_path = Path(sys.argv[1])
    if len(sys.argv) >= 3:
        output_path = Path(sys.argv[2])
    else:
        output_path = input_path.with_stem(input_path.stem + "_uart")

    print(f"Reading: {input_path}")
    data = bytearray(input_path.read_bytes())

    print(f"Size: {len(data)} bytes (0x{len(data):X})")

    print("Verifying original firmware...")
    if not verify_original(data):
        print("Warning: Firmware verification failed, continuing anyway...")

    print("Applying patches:")
    patches = create_patch()
    apply_patches(data, patches)

    print(f"Writing: {output_path}")
    output_path.write_bytes(data)

    print("Done!")
    print("")
    print("Patch summary:")
    print("  - UART handler at 0xA050 (standalone, handles RI+TI, filters 0xFF)")
    print("  - UART ISR wrapper at 0xA085")
    print("  - SCON: 0x00 -> 0x5C (UART mode 1, REN=1)")
    print("  - T2CON: 0x00 -> 0x34 (TR2 enabled)")
    print("  - IE: 0x89 -> 0x99 (ES enabled)")
    print("  - Buffer base: 0x136B (XE9R specific)")
    print("")
    print("MIDI input now uses UART at 31250 baud (16 MHz crystal)")


if __name__ == "__main__":
    main()

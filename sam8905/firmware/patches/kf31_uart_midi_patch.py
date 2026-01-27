#!/usr/bin/env python3
"""
KF31 UART MIDI Patch

Patches the KF31 firmware to enable UART-based MIDI input instead of
the parallel P1 port input used in production.

Changes:
- Relocates INT0 handler to 0xA0AB
- Adds UART ISR at 0xA0CE that reads SBUF
- Enables Timer 2 (TR2) for baud rate generation
- Enables serial interrupt (ES)

Baud rate: 31250 (MIDI standard) with 16 MHz crystal
"""

import sys
from pathlib import Path


def create_patch():
    """Return list of (address, bytes) patches."""
    patches = []

    # --- Vector area patches ---

    # 0x001E: INT0 now jumps to relocated handler at 0xA0AB
    patches.append((0x001E, bytes([0x02, 0xA0, 0xAB])))  # LJMP 0xA0AB

    # 0x0023: Serial vector jumps to new UART ISR at 0xA0FA
    patches.append((0x0023, bytes([0x02, 0xA0, 0xFA])))  # LJMP 0xA0FA

    # --- Initialization patches ---

    # 0x9ED9: T2CON value 0x30 -> 0x34 (enable TR2)
    patches.append((0x9ED9, bytes([0x34])))

    # 0x9EFA: IE value 0x89 -> 0x99 (enable ES serial interrupt)
    patches.append((0x9EFA, bytes([0x99])))

    # --- New code at 0xA0AB ---

    new_code = bytearray()

    # Relocated INT0 handler (0xA0AB-0xA0C5) - 27 bytes
    int0_handler = bytes([
        0xC0, 0xE0,        # PUSH ACC
        0xC0, 0xF0,        # PUSH B
        0xC0, 0x83,        # PUSH DPH
        0xC0, 0x82,        # PUSH DPL
        0xC0, 0xD0,        # PUSH PSW
        0x75, 0xD0, 0x18,  # MOV PSW,#0x18 (bank 3)
        0x12, 0x74, 0x04,  # LCALL 0x7404 (P1 read handler)
        0xD0, 0xD0,        # POP PSW
        0xD0, 0x82,        # POP DPL
        0xD0, 0x83,        # POP DPH
        0xD0, 0xF0,        # POP B
        0xD0, 0xE0,        # POP ACC
        0x32,              # RETI
    ])
    new_code.extend(int0_handler)

    # Standalone UART handler (0xA0C6-0xA0F9) - 52 bytes
    # Based on MS4's ISR_UART_HANDLER - handles both RI and TI
    # Uses KF31's buffer: 0x1414 (data), 0x130E (count), 0x130F (write ptr)
    # Uses IRAM 0x23 as temp storage for SBUF byte
    uart_handler = bytes([
        # 0xA0C6: Check RI
        0x30, 0x98, 0x2E,  # JNB RI, clear_ti (-> 0xA0F7)
        0xC2, 0x98,        # CLR RI
        0xE5, 0x99,        # MOV A, SBUF
        0xB4, 0xFF, 0x02,  # CJNE A, #0xFF, check_buf (-> 0xA0D2)
        0x80, 0x25,        # SJMP clear_ti (-> 0xA0F7)
        # 0xA0D2: check_buf - save byte and check buffer space
        0xF5, 0x23,        # MOV 0x23, A (save SBUF byte)
        0x90, 0x13, 0x0E,  # MOV DPTR, #0x130E (buffer count)
        0xE0,              # MOVX A, @DPTR
        0xB4, 0xFF, 0x02,  # CJNE A, #0xFF, store (-> 0xA0DD)
        0x80, 0x1A,        # SJMP clear_ti (-> 0xA0F7)
        # 0xA0DD: store - increment count and store byte
        0x04,              # INC A (count++)
        0xF0,              # MOVX @DPTR, A
        0x90, 0x13, 0x0F,  # MOV DPTR, #0x130F (write pointer)
        0xE0,              # MOVX A, @DPTR
        0x90, 0x14, 0x14,  # MOV DPTR, #0x1414 (buffer base)
        0x12, 0xA0, 0x91,  # LCALL 0xA091 (DPTR += A)
        0xE5, 0x23,        # MOV A, 0x23 (get saved byte)
        0xF0,              # MOVX @DPTR, A (store to buffer)
        0x90, 0x13, 0x0F,  # MOV DPTR, #0x130F (write pointer)
        0xE0,              # MOVX A, @DPTR
        0x04,              # INC A
        0xF0,              # MOVX @DPTR, A
        0xB4, 0xFF, 0x02,  # CJNE A, #0xFF, clear_ti (wrap check)
        0xE4,              # CLR A
        0xF0,              # MOVX @DPTR, A (write ptr = 0)
        # 0xA0F7: clear_ti - always clear TI and return
        0xC2, 0x99,        # CLR TI
        0x22,              # RET
    ])
    new_code.extend(uart_handler)

    # UART ISR wrapper (0xA0FA-0xA114) - 27 bytes
    uart_isr = bytes([
        0xC0, 0xE0,        # PUSH ACC
        0xC0, 0xF0,        # PUSH B
        0xC0, 0x83,        # PUSH DPH
        0xC0, 0x82,        # PUSH DPL
        0xC0, 0xD0,        # PUSH PSW
        0x75, 0xD0, 0x18,  # MOV PSW,#0x18 (bank 3)
        0x12, 0xA0, 0xC6,  # LCALL 0xA0C6 (UART handler)
        0xD0, 0xD0,        # POP PSW
        0xD0, 0x82,        # POP DPL
        0xD0, 0x83,        # POP DPH
        0xD0, 0xF0,        # POP B
        0xD0, 0xE0,        # POP ACC
        0x32,              # RETI
    ])
    new_code.extend(uart_isr)

    patches.append((0xA0AB, bytes(new_code)))

    return patches


def apply_patches(data: bytearray, patches: list) -> None:
    """Apply patches to binary data."""
    for addr, patch_bytes in patches:
        print(f"  0x{addr:04X}: {data[addr:addr+len(patch_bytes)].hex()} -> {patch_bytes.hex()}")
        data[addr:addr + len(patch_bytes)] = patch_bytes


def verify_original(data: bytes | bytearray) -> bool:
    """Verify this is the expected original firmware."""
    checks = [
        (0x001E, bytes([0xC0, 0xE0, 0xC0, 0xF0, 0xC0])),  # Original INT0 start
        (0x0023, bytes([0x83, 0xC0, 0x82])),              # Original (broken) serial vector area
        (0x9ED9, bytes([0x30])),                          # Original T2CON value
        (0x9EFA, bytes([0x89])),                          # Original IE value
        (0xA0AB, bytes([0xFF, 0xFF, 0xFF, 0xFF])),        # Free space
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
        print("Usage: kf31_uart_midi_patch.py <input.bin> [output.bin]")
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
    print("  - INT0 handler relocated to 0xA0AB")
    print("  - UART handler at 0xA0C6 (standalone, handles RI+TI, filters 0xFF)")
    print("  - UART ISR wrapper at 0xA0FA")
    print("  - T2CON: 0x30 -> 0x34 (TR2 enabled)")
    print("  - IE: 0x89 -> 0x99 (ES enabled)")
    print("")
    print("MIDI input now uses UART at 31250 baud (16 MHz crystal)")


if __name__ == "__main__":
    main()

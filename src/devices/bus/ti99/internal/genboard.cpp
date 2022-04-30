// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Geneve 9640 Gate Array, PAL, and Genmod daughterboard

    This file contains the emulation of the gate array and of the PAL chip
    that is used to control wait state generation.

    Pins of the Gate Array:
    in:  A0..A15: Address bus
    in:  CLKOUT
    in:  IAQ/HOLDA
    in?: NMI*
    in?: RESET*

    i/o: D0..D7: Data bus

    out: KBDINT*:  Keyboard interrupt
    i/o: KBDCLK:   Keyboard clock line
    i/o: KBDDATA:  Keyboard data line

    out: SNDEN*: Sound chip select
    out: RTCEN*: RTC chip select

    out: RAS*
    out: CAS* (2x for two banks)
    out: DRA0..DRA8: Address bus for DRAM (1+18 bit = 512K)

    out: PSIEN*:   9901 enable
    out: CRUCLK*

    out: CSW*: v9938 write
    out: CSR*: v9938 read

    out: RAMENX*: SRAM expansion
    out: RAMEN*:  SRAM
    out: ROMEN*:  EPROM
    out: AB0, AB1, AB2: Mapped address bits (2^15, 2^14, 2^13)
    out: AMC, AMB, AMA: Higher address bits (2^18, 2^17, 2^16)
    in:  DBIN*
    ? :  ABUS* / HOLDA
    out: DBIN
    ? :  HOLD*
    ? :  READY*
    out: DBEN*: External data bus enable
    in:  MEMEN*
    in:  SNDRDY
    out: WE* / CRUCLK
    out: PhiCLK: System clock for 9901

    Onboard SRAM configuration
    ==========================

    Earlier versions of this emulation allowed for up to 384 KiB of SRAM.
    However, this did not reflect the technical options of the real device.
    In fact, there are only two select lines (RAMEN*, RAMEN-X*) that each
    select one SRAM chip of 32 KiB capacity. Accordingly, we now only offer
    the optional 32K expansion.

    Measurements on the real system proved that the zero waitstate access
    is only available for the banks of stock and expansion SRAM, not for
    the other areas (including the EPROM).

    The later operating systems of the Geneve (starting with 2.50s)
    assume the 32K expansion to be available. For this reason, this option is
    selected by default.

    Higher amounts of SRAM require extensive changes to the hardware (also
    with respect to the wait state generation).

    Geneve mapper
    ============

    In the Gate Array, a set of 8 map registers is used to expand the logical
    address to a physical address of 21 bits length. This is done by defining
    frames of 8 KiB size (13 bits); the most significant 3 bits select the
    map register, and its 8 bits are then prepended to the offset.

    Logical address:
    fffx xxxx xxxx xxxx
    \|/
     |    Map registers
     +--> 000: pppp pppp    ---> Physical address (21 bits):
          001: pppp pppp               p pppp pppx xxxx xxxx xxxx
          010: pppp pppp
          ...
          111: pppp pppp               = 000000 ... 1FFFFF (2 MiB space)

    The map registers are memory-mapped into the logical address space
    (Geneve mode: F110..F117, TI mode: 8000..8007) and set by writing the
    bytes into them.

    AMA/B/C decoding
    ================

    Since TI decided, for some obscure reason, to order the address bits in the
    direction from MSB to LSB, the highest address bit is called A0. This
    raises the problem that higher address bits lack a proper number (you
    don't want to use negative numbers for sure). The Peripheral Expansion Box
    already has three additional address lines: AMA (for the 2^16 position),
    AMB (2^17), AMC (2^18).

    The Flex Cable Interface (the card that allows the TI-99/4A console to
    be connected to the Peribox) sets these bits to 1. The Geneve, however,
    may use these lines to expand the usual 64K address space.

    The classic Peribox cards check ABA/B/C for being set to 1. Since all
    addresses starting with 00 or 01 are routed to the Geneve main board
    itself, and the 11 prefix is reserved for SRAM and the Boot EPROM, the
    64K address range of the Peribox cards is on map values 10 111 xxx, which
    means pages b8..bf. As the DSR (Device Service Routine, the card firmware)
    is expected on logical addresses 4000-5FFF, this corresponds to the page 0xba.

    A problem occurs with some 3rd party expansion cards which do not check
    AMA/B/C=1. In a normal TI system, this would have no effect anyway, but
    with the Geneve this leads to mirroring. The usual DSR space at 4000-5fff
    which would be reachable via page 0xba is then mirrored on a number of
    other pages:

    10 xxx 010x = 82, 8a, 92, 9a, a2, aa, b2, ba

    Another block to take care of is 0xbc which covers 8000-9fff since this
    area contains the speech synthesizer port at 9000/9400.

    Address map
    ===========

    p,q = page value bit (q = AMC, AMB, AMA)
    c = address offset within 8 KiB page

    p pqqq pppc cccc cccc cccc

    0 .... .... .... .... .... on-board bus (external drivers inactive)
    0 0... .... .... .... .... on-board DRAM 512 KiB
    0 1... .... .... .... .... on-board future expansion 512 KiB or Memex with Genmod

    1 0... .... .... .... .... external bus (p-box)
    1 0111 .... .... .... .... p-box (AMA/B/C=1)
    1 0111 000. .... .... ....   address block 0xxx
    1 0111 001. .... .... ....   address block 2xxx
    1 0111 010. .... .... ....   address block 4xxx (DSR)
    1 0111 011. .... .... ....   address block 6xxx
    1 0111 100. .... .... ....   address block 8xxx (Speech at 0x9000)
    1 0111 101. .... .... ....   address block axxx
    1 0111 110. .... .... ....   address block cxxx
    1 0111 111. .... .... ....   address block exxx

    1 1... .... .... .... .... on-board bus or external bus (unclear)
    1 10.. .... .... .... .... Future expansion
    1 1100 .... .... .... .... Future expansion

    1 1101 0... .... .... .... on-board sram (32K) - Optional 32 KiB expansion, 0 WS
    1 1101 1... .... .... .... on-board sram (32K) - stock 32 KiB SRAM, 0 WS

    1 111. ..0. .... .... .... on-board boot1
    1 111. ..1. .... .... .... on-board boot2


    Address operation
    =================

    For DRAM access, a separate address bus between the Gate Array and the
    DRAM circuits is used. The address bus has a width of 9 bits, which makes
    it 18 bit for the whole address (row/column). Also, two CAS* lines are
    used, selecting one set of DRAMs.

    For SRAM access, the least significant two map value bits (AB1, AB2)
    are prepended to the 13 bits of the offset from the logical address.
    The next bit (AB0) controls the RAMEN* / RAMENX* lines.

    For the EPROM access, the least significant bit of the map value is
    prepended to the 13 bits of the offset from the logical address. This
    yields a boot ROM size of 16K, mirrored on pages f0, f2, ..., fe, and
    f1, f3, ..., ff. Bigger EPROMs (or flash memory, see PFM) require to use
    the remaining bits AB1, AB0, and AMA.

    The external bus is selected by the two most significant map bits 10.


    GROM emulation and cartridge ROM space
    ======================================

    The Gate Array emulates a 64K GROM space in order to allow TI-99/4A
    cartridge images to be run on the Geneve in TI mode. Also, two 8K ROM
    pages are reserved to allow for emulating Extended Basic type cartridges.

    In TI mode, pages 38 to 3f constitute the 64K GROM space. Pages 36 and 37
    are the two 8K ROM banks. All are located in DRAM with 1 WS. Page 36
    (or page 37) is mapped to logical space 6000-7FFF, regardless of the
    mapper value at 8003 (can be set to any value without effect).

    CRU address >1EF8 determines the ROM size (1 or 2 banks):
        0 = 1 bank (page 36, fixed)
        1 = 2 banks (page 36 when writing a byte to 6000, 6004, ...,
                     page 37 when writing a byte to 6002, 6006, ...)

    CRU address >1EFA write-protects 6000-6FFF when set to 0.
    CRU address >1EFC write-protects 7000-7FFF when set to 0.
    Both do not apply for 2-bank settings.

    As with every page, the GROM pages may be mapped to any other memory
    area as well so that they may be randomly accessed.

    Unlike the real GROM, the GROM emulation allows free access to the whole
    8K of its page. Also, the emulation allows writing, so we essentially
    have a GRAM emulation.

    The Gate Array contains a 16-bit counter that represents the current
    GROM address. It wraps at 8K boundaries (>3FFE->3FFF->2000->2001). Reading
    from the GROM read port delivers the byte at the current address and
    increases the counter. Writing to the GROM port stores the byte at that
    address, respectively, and then increases the counter.

    Reading the address counter delivers first the MSB, then on every
    following access, the LSB. The counter itself contains the LSB in both
    bytes after the first read operation. Thus, its value must be restored
    after reading.

    Setting the address counter copies its LSB to the MSB and then writes
    the new byte into the LSB. After the second write (without intermediate
    data transfer), the counter is increased by one.


    Genmod expansion
    ================

    The objective of the Genmod is to allow the Geneve to access the full
    2 MiB physical address space, in conjunction with the MEMEX card that
    must be plugged into the p-box. There are actually two lines in the box
    that are unused; those are now defined as AMD and AME. They are located
    on the p-box bus on pins 8 and 9.

    Since the Gate Array does not output the first two address bits, they have
    to be reconstructed by two GAL chips on a daughterboard that must be
    soldered to the backside of the Gate Array.

    When ROMEN is active, the on-board EPROM is accessed, and the external
    bus is inactive.

    The MEMEX card allows for using 0 waitstate accesses. This means that
    the wait state generation for box accesses must be inhibited; this is
    done by cutting the trace from the Gate Array pin READY to the PAL.

    Some peripheral cards must now be modified to check for AMD and AME
    as well, or they will be mirrored into other memory areas.
    This GenMod feature is automatically applied to all peripheral cards in
    this emulation.

    The only remaining issue is that GROM access in the TI mode is under
    full control of the Gate Array; it will always activate one of the DRAM
    banks. To change this, the GA would need to be fully replaced. Instead,
    the real GenMod contains a small box with two switches, which is also
    emulation here:

    - Turbo mode: Activates or deactivates the wait state logic on the Geneve
      board. This switch may be changed at any time.
    - TI mode: Selects between the on-board memory, which is required
      for the GROM access, and the external Memex memory. This switch
      triggers a reset when changed.


    Logical address space layout
    ============================

    Geneve mode
    -----------
    Video:    F100 (port 0, rw),
              F102 (port 1, rw),
              F104 (port 2, w),
              F106 (port 3, w)
                                  1111 0001 0000 .xx0
    Mapper:   F110 - F117         1111 0001 0001 0xxx
    Keyboard: F118                1111 0001 0001 1...
    Clock:    F130 - F13F         1111 0001 0011 xxxx
    Sound:    F120                1111 0001 0010 ...0

    TI mode
    -------
    Video:    8800 (port 0, r), 8c00 (port 0, w),
              8802 (port 1, r), 8c02 (port 0, w),
                                8c04 (port 2, w),
                                8c06 (port 3, w)

                                  1000 1w.. .... .xx0
    Mapper:   8000 - 8007         1000 0000 0000 0xxx
    Keyboard: 8008 - 800F         1000 0000 0000 1...
    Clock:    8010 - 801F         1000 0000 0001 xxxx
    Sound:    8400 - 87FE         1000 01.. .... ...0
    Speech:   9000 / 9400         1001 0w.. .... ...0
    Grom:     9800 / 9802         1001 1w.. .... ..x0
              9c00 / 9c02

    Physical address space layout
    -----------------------------

    Start    End      Phys.pages
    000000 - 07FFFF   00-3F   512 KiB DRAM on-board
       06C000 - 06DFFF   36     Cartridge space first 8K
       06E000 - 06FFFF   37     Cartridge space second 8K
    080000 - 0FFFFF   40-7F   512 KiB on-board expansion (never used)
    100000 - 16FFFF   80-B7   448 KiB P-Box space (special cards, like MEMEX)
    170000 - 17FFFF   B8-BF    64 KiB P-Box space (current cards)
    180000 - 1CFFFF   C0-E7    Future expansion
    1D0000 - 1D7FFF   E8-EB    32 KiB SRAM expansion
    1D8000 - 1DFFFF   EC-EF    32 KiB stock SRAM
    1E0000 - 1FFFFF   F0-FF   128 KiB EPROM space; 16 KiB actually used, 8 mirrors


    GenMod modification
    -------------------
    TI mode
    000000 - 07FFFF   00-3F   512 KiB DRAM on-board
       06C000 - 06DFFF   36     Cartridge space first 8K
       06E000 - 06FFFF   37     Cartridge space second 8K
    080000 - 1DFFFF   40-EF  1408 KiB P-Box space
    1E0000 - 1FFFFF   F0-FF   128 KiB EPROM space; 16 KiB actually used, 8 mirrors

    Non-TI mode
    000000 - 1DFFFF   00-EF  1920 KiB P-Box space
    1E0000 - 1FFFFF   F0-FF   128 KiB EPROM space; 16 KiB actually used, 8 mirrors

    Waitstate handling
    ==================
    Waitstates are caused by a cleared READY line of the TMS9995 processor
    during an external memory cycle. That means that waitstates have no effect
    for operations within the on-chip memory, and only when an access to the
    external memory or other devices occurs, a delay will be noticed.

    The waitstates are generated by the custom Gate Array chip on the board
    and the PAL 16R4, both lacking proper documentation. All of the following
    numbers have been determined by experiments with the real machine.

    Waitstates are generated for all accesses except for the SRAM in pages
    E8..EF. Video accesses produce 15 waitstates, created by the counter
    in the PAL. However, these video wait states are effective after the video
    access has been completed. Wait states are not effective when the
    execution is running in on-chip RAM.
    Without additional wait states, the video access takes the usual 1 or 2 WS.

    Additional waitstates are created when one of the CRU bits is set. In that
    case, all delays are extended to 2 WS (including SRAM).

    Sound waitstates depend on the clock of the sound chip; the READY line is
    pulled down until the next clock pulse.


    CRU map
    =======
    ---- TMS 9901 ----
    0000: flag Clock mode
    0002: int  INTA* (pbox pin 17)
    0004: int  Video interrupt
    0006: in   Joystick button
    0008: in      left
    000A: in      right
    000C: in      down
    000E: in      up
    0010: int  Keyboard scancode available
    0012: in   (mirror of 003A)
    0014: in   Left mouse button
    0016: in   Real-time clock interrupt
    0018: in   INTB* (pbox pin 18)
    001A: in   (reflects 0032)
    001C: -
    001E: in   (reflects 002E)
    0020: out  Pbox reset
    0022: out  Video reset
    0024: out  Joystick select (0=Joystick 1, 1=Joystick 2)
    0026: -
    0028: out  PFM bank switch LSB
    002A: out  PFM output enable
    002C: out  Keyboard reset
    002E: out  System clock speed (external memory cycles)
    0030: -
    0032: out  Video wait state enable
    0034: in   (mirror of 0018)
    0036: in   (mirror of 0016)
    0038: in   (mirror of 0014)
    003A: out  PFM bank switch MSB
    003C: in   (mirror of 0010)
    003E: in   (mirror of 000E)

    ----Gate Array----
    13C0 - 13FE: Single step execution (not implemented, not available for most systems)

    ----TMS9995-internal flag registers----
    The values are all latched inside the CPU, but output values are visible
    on the bus outside the CPU
    1EE0-1EE8, 1FDA:     see tms9995.cpp
    1EEA:   PAL/NTSC flag
    1EEC:   (unused)
    1EEE:   CapsLock flag
    1EF0:   Keyboard clock
    1EF2:   Keyboard shift register enable
    1EF4:   Operation mode (native, GPL)
    1EF6:   Memory mode (unmapped, mapped)
    1EF8:   Cartridge size (0: 16 KiB, 1: 8 KiB)
    1EFA:   Protect 6xxx
    1EFC:   Protect 7xxx
    1EFE:   Additional wait state per memory access

    -------------------------------------------------------------------------

    Michael Zapf, October 2011
    February 2012: rewritten as class, restructured
    Aug 2015: PFM added
    Nov 2019: Extensive rewrite to provide a true emulation of the Gate Array
              and the PAL, and also to allow for different kinds of external
              keyboards.

***************************************************************************/
#include "emu.h"

#define LOG_WARN     (1U<<1)
#define LOG_DETAIL   (1U<<2)
#define LOG_READ     (1U<<3)
#define LOG_WRITE    (1U<<4)
#define LOG_KEYBOARD (1U<<5)
#define LOG_CLOCK    (1U<<6)
#define LOG_READY    (1U<<7)
#define LOG_SETTING  (1U<<8)
#define LOG_CRU      (1U<<9)
#define LOG_CRUKEY   (1U<<10)
#define LOG_DECODE   (1U<<11)
#define LOG_ADDRESS  (1U<<12)
#define LOG_LINES    (1U<<13)
#define LOG_WAIT     (1U<<14)
#define LOG_GROM      (1U<<15)
#define LOG_MAPPER    (1U<<16)

// Minimum log should be warnings
#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "genboard.h"
#include "logmacro.h"

DEFINE_DEVICE_TYPE(GENEVE_GATE_ARRAY, bus::ti99::internal::geneve_gate_array_device, "geneve_gate_array", "Geneve Gate Array")
DEFINE_DEVICE_TYPE(GENMOD_DECODER,    bus::ti99::internal::genmod_decoder_device, "genmod_decoder", "GenMod decoder circuit")
DEFINE_DEVICE_TYPE(GENEVE_PAL,        bus::ti99::internal::geneve_pal_device, "geneve_pal", "Geneve PAL circuit")

namespace bus::ti99::internal {

geneve_gate_array_device::geneve_gate_array_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_have_waitstate(false),
	m_have_extra_waitstate(false),
	m_enable_extra_waitstates(false),
	m_extready(true),
	m_sndready(true),

	m_grom_address(0),
	m_cartridge_banked(false),
	m_cartridge_secondpage(false),
	m_cartridge6_writable(false),
	m_cartridge7_writable(false),
	m_load_lsb(false),

	m_geneve_mode(false),
	m_direct_mode(false),

	m_keyint(*this),
	m_keyb_clk(*this),
	m_keyb_data(*this),
	m_keyboard_shift_reg(0),
	m_keyboard_last_clock(CLEAR_LINE),
	m_keyboard_data_in(CLEAR_LINE),
	m_shift_reg_enabled(false),

	m_pal(*owner, GENEVE_PAL_TAG),
	m_peribox(*owner, TI_PERIBOX_TAG),

	m_debug(false)
{
}

geneve_gate_array_device::geneve_gate_array_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: geneve_gate_array_device(mconfig, GENEVE_GATE_ARRAY, tag, owner, clock)
{
}

void geneve_gate_array_device::cru_sstep_write(offs_t offset, uint8_t data)
{
	// Single step
	// 13c0 - 13fe: 0001 0011 11xx xxx0  (offset << 1)
	LOGMASKED(LOG_WARN, "Single step not implemented; bit %d set to %d\n", offset & 0x001f, data);
}

void geneve_gate_array_device::cru_ctrl_write(offs_t offset, uint8_t data)
{
	// This is just mirroring the internal flags of the 9995
	int bit = (offset & 0x000f);
	switch (bit)
	{
	case 5:
		// Unknown effect
		LOGMASKED(LOG_CRU, "Set PAL flag = %02x\n", data);
		// m_palvideo = (data!=0);
		break;
	case 7:
		// Capslock flag; just to keep track of the current state
		break;
	case 8:
		LOGMASKED(LOG_CRUKEY, "Set keyboard clock = %02x\n", data);
		set_keyboard_clock(data);
		break;
	case 9:
		LOGMASKED(LOG_CRUKEY, "Enable keyboard shift reg = %02x\n", data);
		enable_shift_register(data);
		break;
	case 10:
		LOGMASKED(LOG_CRU, "Operation mode = %s\n", (data!=0)? "native" : "GPL");
		m_geneve_mode = (data!=0);
		break;
	case 11:
		LOGMASKED(LOG_CRU, "Addressing mode = %s\n", (data!=0)? "unmapped" : "mapped");
		m_direct_mode = (data!=0);
		break;
	case 12:
		LOGMASKED(LOG_CRU, "Cartridge ROM: %s\n", (data==0)? "banked" : "unbanked");
		m_cartridge_banked = (data==0);
		break;
	case 13:
		LOGMASKED(LOG_CRU, "Cartridge ROM 6000-6fff %s\n", (data!=0)? "writable" : "protected");
		m_cartridge6_writable = (data!=0);
		break;
	case 14:
		LOGMASKED(LOG_CRU, "Cartridge ROM 7000-7fff %s\n", (data!=0)? "writable" : "protected");
		m_cartridge7_writable = (data!=0);
		break;
	case 15:
		LOGMASKED(LOG_CRU, "Extra wait states %s\n", (data==0)? "enabled" : "disabled");
		m_enable_extra_waitstates = (data==0);
		break;
	default:
		break;
	}
}

/******************************************************************
   Keyboard support
   XT protocol:

    Original XT: 0 1 bit0 bit1 bit2 bit3 bit4 bit5 bit6 bit7
    Some clones:   1 bit0 bit1 bit2 bit3 bit4 bit5 bit6 bit7

    bit0 = LSB, bit7 = MSB

   For now we assume that the Geneve needs the original XT keyboard.

   We can use the start 1 bit to control bit reception. When it reaches the
   rightmost position, we suspend transfer and raise the interrupt.

   With the flagging of the interrupt, the data line towards the keyboard
   is held low until the interrupt is cleared. This is done by clearing the
   shift register by setting the CRU address 1EF2 to 0.

   Note that lowering the clock line to 0 for more than 20ms will trigger
   a keyboard reset.

******************************************************************/

/*
    Pull down or release the clock line.
    Called by setting CRU bit 1EF0 to 0 or 1.
*/
WRITE_LINE_MEMBER( geneve_gate_array_device::set_keyboard_clock)
{
	m_keyb_clk(state);
}

/*
    Enable the shift register. Setting to 0 will clear the register and
    lock it. At the same time, the interrupt is cleared, and the data line
    is released. If further scancodes are expected, the shift register should
    immediately be enabled again.

    Called by setting CRU bit 1EF2 to 0 or 1
*/
WRITE_LINE_MEMBER( geneve_gate_array_device::enable_shift_register)
{
	m_shift_reg_enabled = (state==ASSERT_LINE);

	if (!m_shift_reg_enabled)
	{
		LOGMASKED(LOG_KEYBOARD, "Clear shift register, disable\n");
		m_keyboard_shift_reg = 0;
		shift_reg_changed();
	}
	else
		LOGMASKED(LOG_KEYBOARD, "Enable shift register\n");
}

void geneve_gate_array_device::shift_reg_changed()
{
	// The level of the data line is the inverse of the rightmost bit of
	// the shift register. This means that once the start bit reaches that
	// position, it will pull down the data line and stop the transfer.
	m_keyb_data(1 - (m_keyboard_shift_reg & 1));
	m_keyint((m_keyboard_shift_reg & 1)? ASSERT_LINE : CLEAR_LINE);
	if (m_keyboard_shift_reg & 1)
		LOGMASKED(LOG_KEYBOARD, "Scan code complete; raise interrupt, hold down data line\n");
	else
		LOGMASKED(LOG_KEYBOARD, "Clear keyboard interrupt, release data line\n");
}

/*
    Incoming keyboard strobe. When 0, push the current data line level into
    the shift register at the leftmost position.
*/
WRITE_LINE_MEMBER( geneve_gate_array_device::kbdclk )
{
	LOGMASKED(LOG_KEYBOARD, "Keyboard clock: %d\n", state);
	bool clock_falling_edge = (m_keyboard_last_clock == ASSERT_LINE && state == CLEAR_LINE);

	if (m_shift_reg_enabled && clock_falling_edge)
	{
		m_keyboard_shift_reg = (m_keyboard_shift_reg>>1) | (m_keyboard_data_in? 0x100 : 0x00);
		LOGMASKED(LOG_KEYBOARD, "Shift register = %02x\n", m_keyboard_shift_reg>>1);
		shift_reg_changed();
	}
	m_keyboard_last_clock = (line_state)state;
}

/*
    Latch the value of the incoming data line.
*/
WRITE_LINE_MEMBER( geneve_gate_array_device::kbddata )
{
	LOGMASKED(LOG_KEYBOARD, "Keyboard data: %d\n", state);
	m_keyboard_data_in = (line_state)state;
}

// sysspeed has no effect for vwait/speed

// TODO: Video write timing

/*****************************************************************
    Decoding functions
******************************************************************/

const geneve_gate_array_device::logentry_t geneve_gate_array_device::s_logmap[10] =
{
	{ 0xf100, 0x000e, 0x8800, 0x03fe, 0x0400, MLVIDEO,  "video" },
	{ 0xf110, 0x0007, 0x8000, 0x0007, 0x0000, MLMAPPER, "mapper" },
	{ 0xf118, 0x0007, 0x8008, 0x0007, 0x0000, MLKEY,    "keyboard" },
	{ 0xf120, 0x000e, 0x8400, 0x01ff, 0x0000, MLSOUND,  "sound" },
	{ 0x0000, 0x0000, 0x8600, 0x01ff, 0x0000, MLEXT,    "external bus" },
	{ 0xf130, 0x000f, 0x8010, 0x000f, 0x0000, MLCLOCK,  "clock" },
	{ 0x0000, 0x0000, 0x9000, 0x03fe, 0x0400, MLEXT,    "speech (ext. bus)" },
	{ 0x0000, 0x0000, 0x9800, 0x03fc, 0x0400, MLGROM,   "GROM data" },
	{ 0x0000, 0x0000, 0x9802, 0x03fc, 0x0400, MLGROMAD, "GROM address" },
	{ 0x0000, 0x0000, 0x6000, 0x1fff, 0x0000, MLCARTROM, "Cartridge ROM" }
};

void geneve_gate_array_device::decode_logical(geneve_gate_array_device::decdata* dec)
{
	dec->function = MUNDEF;
	int index = -1;

	for (int i = 0; (i < 10) && (index == -1); i++)
	{
		if (m_geneve_mode)
		{
			// Skip when genbase is 0
			if ((s_logmap[i].genbase != 0) && ((dec->offset & ~s_logmap[i].genmask) == s_logmap[i].genbase))
				index = i;
		}
		else
		{
			if (dec->read)
			{
				if ((dec->offset & ~s_logmap[i].timask) == s_logmap[i].tibase)
					index = i;
			}
			else
			{
				if ((dec->offset & ~s_logmap[i].timask) == (s_logmap[i].tibase | s_logmap[i].writeoff))
					index = i;
			}
		}
	}

	if (index != -1)
	{
		LOGMASKED(LOG_DECODE, "Decoded as %s: %04x\n", s_logmap[index].description, dec->offset);
		dec->function = s_logmap[index].function;
	}

	// Handle write operations to the cartridge ROM
	if (dec->function == MLCARTROM && !dec->read)
	{
		if (m_cartridge_banked) dec->function = MLCARTBANK;
		else
		{
			if ((((dec->offset & 0x1000)==0) && !m_cartridge6_writable)
				|| (((dec->offset & 0x1000)!=0) && !m_cartridge7_writable))
			{
				dec->function = CARTPROT;
				LOGMASKED(LOG_WARN, "Writing to protected cartridge space %04x ignored\n", dec->offset);
			}
		}
	}
}

/*
    Look up the mapper value (page) for the given logical address.
    Cartridges need special handling.
*/
void geneve_gate_array_device::get_page(geneve_gate_array_device::decdata* dec)
{
	int logpage = (dec->offset & 0xe000) >> 13;
	dec->physpage = 0;

	// Determine physical address
	if (m_direct_mode) dec->physpage = 0xf8; // points to boot eprom
	else
	{
		if (dec->function == MLCARTROM)
			dec->physpage = (m_cartridge_banked && m_cartridge_secondpage)? 0x37 : 0x36;
		else
		{
			if (dec->function == MLGROM)
				dec->physpage = ((m_grom_address >> 13) & 0x07) | 0x38;
			else
			{
				if (dec->function == MLEXT)
				{
					dec->physpage = 0xb8 | logpage;
				}
				else
					dec->physpage = m_map[logpage];
			}
		}
	}
}

const geneve_gate_array_device::physentry_t geneve_gate_array_device::s_physmap[7] =
{
	{ 0x00, 0x3f, MPDRAM,  "DRAM" },
	{ 0x40, 0x3f, MPEXP,  "Future exp (on-board)" },
	{ 0xe8, 0x03, MPSRAMX,  "SRAM exp" },
	{ 0xec, 0x03, MPSRAM,  "SRAM" },
	{ 0xf0, 0x0f, MPEPROM,  "Boot ROM/PFM" },
	{ 0x80, 0x7f, MBOX,  "external" }          // catch-all
};

/*
    Try to decode the physical address.
*/
void geneve_gate_array_device::decode_physical(geneve_gate_array_device::decdata* dec)
{
	// Search the map
	if (dec->function == MUNDEF || dec->function == MLCARTROM || dec->function == MLGROM)
	{
		if (dec->function == MLGROM)
		{
			// If map byte 6 is not set to 03, ignore the access
			// This seems to be a safety feature, or a glitch in the design
			// of the Gate array.
			if (m_map[6] != 0x03)
			{
				dec->function = MUNDEF;
				return;
			}

			// GROM is actually DRAM; substitute the address and update the address counter
			// The page has already been set in get_page
			LOGMASKED(LOG_GROM, "GROM address = %04x\n", m_grom_address);
			dec->offset = m_grom_address;  // Do not wipe the GROM number
			// Auto-increment here (not in clock_in, as we have two clock cycles by wait states)
			increase_grom_address();
			return;
		}

		// Search for the matching page interval. Cartridge space will be decoded as DRAM.
		bool found = false;
		for (int i = 0; (i < 6) && !found; i++)
		{
			if ((dec->physpage & ~s_physmap[i].mask) == s_physmap[i].base)
			{
				dec->function = s_physmap[i].function;
				LOGMASKED(LOG_DECODE, "Decoded as %s, page %02x\n", s_physmap[i].description, dec->physpage);
				found = true;
			}
		}

		if (!found)
			LOGMASKED(LOG_DECODE, "Unmapped address %06x\n", (get_prefix(0xff) | (dec->offset & 0x1fff)) & 0x1fffff);
	}
}

void geneve_gate_array_device::increase_grom_address()
{
	if (!m_debug)
	{
		m_grom_address = ((m_grom_address + 1) & 0x1fff) | (m_grom_address & 0xe000);
		m_load_lsb = false;
	}
}

/*
    Are we addressing DRAM?
*/
bool geneve_gate_array_device::accessing_dram()
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	return accessing_dram_s(dec->function);
}

bool geneve_gate_array_device::accessing_dram_s(int function)
{
	return (function == MPDRAM) || (function == MLGROM) || (function == MLCARTROM);
}

bool geneve_gate_array_device::accessing_sram_s(int function)
{
	return (function == MPSRAM) || (function == MPSRAMX);
}

bool geneve_gate_array_device::accessing_devs_s(int function)
{
	return (function == MLMAPPER) || (function == MLKEY) || (function == MLCLOCK) || (function == MLGROMAD);
}

bool geneve_gate_array_device::accessing_grom()
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	return (dec->function == MLGROM);
}

bool geneve_gate_array_device::accessing_box_s(int function, bool genmod)
{
	return (function == MLEXT) || (function == MBOX) || (genmod && function == MPEXP);
}

/*
    The Gate Array has a private address bus to the DRAM.
*/
offs_t geneve_gate_array_device::get_dram_address()
{
	offs_t physaddr = 0;
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	int addr13 = dec->offset & 0x1fff;

	// Cartridge access is also done in DRAM; the pages are fixed to
	// 36, 37 (ROM), 38...3f (GROM), regardless of the mapper.

	if (dec->function == MLGROM)
	{
		physaddr = (0x38 << 13) | dec->offset;
		// The GROM address counter is updated in decode_physical
	}
	else
	{
		if (dec->function == MPDRAM || dec->function == MLCARTROM)
		{
			physaddr = (get_prefix(0x3f) | addr13) & 0x7ffff;
		}
		else
			LOGMASKED(LOG_WARN, "Unknown decoding %d for DRAM\n", dec->function);
	}
	return physaddr;
}

/*
    Accept the address passed over the address bus and decode it appropriately.
    This decoding will later be used in the READ/WRITE member functions. Also,
    we initiate wait state creation here.
*/
void geneve_gate_array_device::setaddress(offs_t address, uint8_t busctrl)
{
	LOGMASKED(LOG_ADDRESS, "setaddress = %04x%s\n", address, m_debug? " (debug)" : "");
	decdata& dec = (m_debug)? m_decdebug : m_decoded;

	dec.offset = address;
	dec.read = ((busctrl & TMS99xx_BUS_DBIN)!=0);

	decode_logical(&dec);
	get_page(&dec);
	decode_physical(&dec);

	if (!m_debug)
	{
		m_have_extra_waitstate = m_enable_extra_waitstates;
		m_have_waitstate = (dec.function != MPSRAM && dec.function != MPSRAMX) || m_have_extra_waitstate;
	}
}

/*
   The Gate Array uses the clock to operate the wait state flags. The actual
   wait state generation is up to the PAL chip.
*/
WRITE_LINE_MEMBER( geneve_gate_array_device::clock_in )
{
	// Falling CLK
	if (state == CLEAR_LINE)
	{
		if (!m_have_extra_waitstate)
			m_have_waitstate = false;
		else
			m_have_extra_waitstate = false;
	}
}

/*
    READY line from the peribox. Together with the sndready and the READY output
    of the Gate Array itself, this forms a wired AND.
*/
WRITE_LINE_MEMBER( geneve_gate_array_device::extready_in )
{
	LOGMASKED(LOG_READY, "External READY = %d\n", state);
	m_extready = (state==ASSERT_LINE);
}

WRITE_LINE_MEMBER( geneve_gate_array_device::sndready_in )
{
	LOGMASKED(LOG_READY, "Sound READY = %d\n", state);
	m_sndready = (state==ASSERT_LINE);
}

READ_LINE_MEMBER(geneve_gate_array_device::csw_out)
{
	// Do not access a port-based device in debugger mode
	if (m_debug) return CLEAR_LINE;
	return ((m_decoded.function == MLVIDEO) && !m_decoded.read)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER(geneve_gate_array_device::csr_out)
{
	// Do not access a port-based device in debugger mode
	if (m_debug) return CLEAR_LINE;
	return ((m_decoded.function == MLVIDEO) && m_decoded.read)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER(geneve_gate_array_device::romen_out)
{
	// Do not restrict to read-only, as we could have a PFM here
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	return (dec->function == MPEPROM)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER(geneve_gate_array_device::ramen_out)
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	return (dec->function == MPSRAM)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER(geneve_gate_array_device::ramenx_out)
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	return (dec->function == MPSRAMX)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER(geneve_gate_array_device::rtcen_out)
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	return (dec->function == MLCLOCK)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER(geneve_gate_array_device::snden_out)
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	return ((dec->function == MLSOUND) && !dec->read)? ASSERT_LINE : CLEAR_LINE;
}

READ_LINE_MEMBER(geneve_gate_array_device::dben_out)
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	return accessing_box_s(dec->function, false)? ASSERT_LINE : CLEAR_LINE;
}

// After setaddress, pull down READY if
// - we have an extra waitstate  OR
// - we do not access SRAM(X) OR
// - extready = 0

// In Genmod, pull down READY if
// - we have an extra waitstate OR
// - we access the box and have turbo==0 OR
// - we access DRAM and have timode==1 OR
// - extready = 0

READ_LINE_MEMBER(geneve_gate_array_device::gaready_out)
{
	if (m_debug) return ASSERT_LINE;  // Always READY when debugging
	// Return true (READY=1) when we are accessing SRAM/SRAMX and when we do not have extra waitstates
	// return ((m_decoded.function == MPSRAM || m_decoded.function == MPSRAMX) && !m_have_extra_waitstate)? ASSERT_LINE : CLEAR_LINE;
	return (m_have_waitstate || m_have_extra_waitstate || !m_extready || !m_sndready)? CLEAR_LINE : ASSERT_LINE;
}


/**********************************************************
    Gate Array-internal functions
***********************************************************/

/*
    Changes the value of the parameter if one of the functions applies.
*/
void geneve_gate_array_device::readz(uint8_t& value)
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	uint8_t lsb = 0;

	switch (dec->function)
	{
	case MLMAPPER:
		value = m_map[dec->offset & 0x0007];
		LOGMASKED(LOG_READ, "Read mapper %04x -> %02x\n", dec->offset, value);
		break;
	case MLKEY:
		value = m_keyboard_shift_reg>>1;
		LOGMASKED(LOG_READ, "Read keyboard %04x -> %02x\n", dec->offset, value);
		break;
	case MLGROMAD:
		if (!m_debug)       // don't let the debugger mess with the address counter
		{
			value = (m_grom_address & 0xff00)>>8;
			lsb = (m_grom_address & 0xff);
			m_grom_address = lsb << 8 | lsb;
			m_load_lsb = false;
			LOGMASKED(LOG_READ, "Read GROM address %04x -> %02x\n", dec->offset, value);
		}
		break;
	case MLCARTBANK:
		break;
	default:
		break;
	}
}

/*
    Internal functions of the Gate Array. Returns without changes if none
    of the function applies.
*/
void geneve_gate_array_device::write(uint8_t data)
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;

	switch (dec->function)
	{
	case MLMAPPER:
		m_map[dec->offset & 0x0007] = data;
		// LOGMASKED(LOG_MAPPER, "Write mapper %04x <- %02x\n", dec->offset, data);
		LOGMASKED(LOG_MAPPER, "Set %04x mapper[%02x %02x %02x %02x %02x %02x %02x %02x]\n",
			dec->offset, m_map[0], m_map[1], m_map[2], m_map[3], m_map[4], m_map[5], m_map[6], m_map[7]);
		break;
	case MLGROMAD:
		if (!m_debug)   // don't let the debugger mess with the address counter
		{
			m_grom_address = (m_grom_address << 8 | data) & 0xffff;
			if (m_load_lsb) increase_grom_address();
			else m_load_lsb = true;
			LOGMASKED(LOG_GROM, "Write GROM address %04x <- %02x\n", dec->offset, data);
		}
		break;
	case MLCARTBANK:
		m_cartridge_secondpage = ((dec->offset & 0x0002)!=0);
		LOGMASKED(LOG_WRITE, "Set cartridge bank %04x <- %02x\n", dec->offset, data);
		break;
	case MLKEY:
		LOGMASKED(LOG_WRITE, "Write to keyboard ignored\n");
		break;
	default:
		break;
	}
}

/*
    Address lines that the Gate Array offers. They reflect the 8 bits of
    the mapper byte. AME and AMD are only used by GenMod.
    (AME, AMD,) AMC, AMB, AMA, AB0, AB1, AB2
*/
int geneve_gate_array_device::get_prefix(int lines)
{
	decdata* dec = (m_debug)? &m_decdebug : &m_decoded;
	return (dec->physpage & lines) << 13;
}

//====================================================================
//  Common device lifecycle
//====================================================================

void geneve_gate_array_device::device_start()
{
	m_keyint.resolve_safe();
	m_keyb_clk.resolve_safe();
	m_keyb_data.resolve_safe();

	m_geneve_mode = false;
	m_direct_mode = true;

	// State registration
	save_item(NAME(m_have_extra_waitstate));
	save_item(NAME(m_enable_extra_waitstates));
	save_item(NAME(m_extready));
	save_item(NAME(m_sndready));

	save_item(NAME(m_grom_address));
	save_item(NAME(m_cartridge_banked));
	save_item(NAME(m_cartridge_secondpage));
	save_item(NAME(m_cartridge6_writable));
	save_item(NAME(m_cartridge7_writable));
	save_item(NAME(m_load_lsb));

	save_item(NAME(m_geneve_mode));
	save_item(NAME(m_direct_mode));

	save_pointer(NAME(m_map), 8);

	save_item(NAME(m_decoded.read));
	save_item(NAME(m_decoded.function));
	save_item(NAME(m_decoded.offset));
	save_item(NAME(m_decoded.physpage));

	save_item(NAME(m_keyboard_shift_reg));
	save_item(NAME(m_shift_reg_enabled));
}

void geneve_gate_array_device::common_reset()
{
	m_have_extra_waitstate = false;
	m_enable_extra_waitstates = false;

	m_grom_address = 0;
	m_cartridge_banked = false;
	m_cartridge_secondpage = false;
	m_cartridge6_writable = false;
	m_cartridge7_writable = false;

	m_geneve_mode =false;
	m_direct_mode = true;

	// Clear map
	for (auto & elem : m_map) elem = 0;
}

void geneve_gate_array_device::device_reset()
{
	common_reset();
}

/* ========================================================================

    The PAL circuit on the Geneve main board. It is the actual waitstate
    generator, and its task is to control the READY line depending on the
    accessed device, and to control the outgoing MEMEN* and WE* lines into
    the peribox.

    The chip is a PAL16R4ACN

    Pin    Dir   Meaning
    ---------------------
    1      in    CLK (assert=H)
    2      in    WE* (write)
    3      in    READYIN (from Gate Array and sound chip)
    4      in    CSR* (Video read)
    5      in    CRU bit 23 (>002E) ("System clock speed")
    6      in    MEMEN* (memory access)
    7      in    DBEN* (external data bus enable)
    8      in    (Gate Array bit 36)
    9      in    CSW* (video write)
    10           GND
    11     in    OE*, hardwired to H
    12     out   WE*
    13     out   MEMEN*
    14           (output of FF1, disabled by pin 11)
    15           (output of FF2, disabled by pin 11)
    16           (output of FF3, disabled by pin 11)
    17           (output of FF4, disabled by pin 11)
    18     out   READYOUT
    19     in    VDPWAITEN
    20           Vcc

    ======================================================================== */

geneve_pal_device::geneve_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GENEVE_PAL, tag, owner, clock),
	  m_pin3(true),
	  m_pin4(true),
	  m_pin5(true),
	  m_pin9(true),
	  m_pin19(false),
	  m_pin14d(false),m_pin14q(false),
	  m_pin15d(false),m_pin15q(false),
	  m_pin16d(false),m_pin16q(false),
	  m_pin17d(false),m_pin17q(false),
	  m_prev_ready(CLEAR_LINE),
	  m_peribox(*owner, TI_PERIBOX_TAG),
	  m_ready(*this)
{
}

/*
    READY input from the Gate Array.
*/
WRITE_LINE_MEMBER(geneve_pal_device::gaready_in)
{
	bool prev = m_pin3;
	m_pin3 = (state==ASSERT_LINE);
	if (prev != m_pin3)
	{
		LOGMASKED(LOG_LINES, "READY(ga) <- %d\n", state);
		// When GAREADY=L, the video lines have immediate effect on the ready line
		set_ready();
	}
}

/*
    Video read (ASSERT=low).
*/
WRITE_LINE_MEMBER(geneve_pal_device::csr_in)
{
	bool prev = m_pin4;
	m_pin4 = (state==CLEAR_LINE);
	if (prev != m_pin4)
	{
		LOGMASKED(LOG_LINES, "CSR <- %d\n", state);
		set_ready();
	}
}

/*
    Video write (ASSERT=low).
*/
WRITE_LINE_MEMBER(geneve_pal_device::csw_in)
{
	bool prev = m_pin9;
	m_pin9 = (state==CLEAR_LINE);
	if (prev != m_pin9)
	{
		LOGMASKED(LOG_LINES, "CSW <- %d\n", state);
		set_ready();
	}
}

/*
    Memory enable (ASSERT=low); pass through
*/
WRITE_LINE_MEMBER(geneve_pal_device::memen)
{
	LOGMASKED(LOG_LINES, "MEMEN -> %d\n", state);
	m_peribox->memen_in(state);
}

/*
    Write external mem cycles (0=long, 1=short)
    System clock speed (PAL pin 5). (ASSERT=high)
    The function from the equations could not be verified on a real machine.
    This function seems to have no effect. So either the equations are
    wrong, or something else is going on.
*/
WRITE_LINE_MEMBER(geneve_pal_device::sysspeed)
{
	bool prev = m_pin5;
	m_pin5 = (state == ASSERT_LINE);
	if (prev != m_pin5)
	{
		LOGMASKED(LOG_SETTING, "System clock speed set to %d\n", state);
		set_ready();
	}
}

/*
    Write vdp wait cycles (1=add 14 cycles, 0=add none)
    see above for waitstate handling
*/
WRITE_LINE_MEMBER(geneve_pal_device::vwaiten)
{
	bool prev = m_pin19;
	m_pin19 = (state==ASSERT_LINE);
	if (prev != m_pin19)
	{
		LOGMASKED(LOG_SETTING, "Video wait states %s\n", (state!=0)? "enabled" : "disabled");
		set_ready();
	}
}

/*
    Clock input. This controls the state of the waitstate counter.
*/
WRITE_LINE_MEMBER(geneve_pal_device::clock_in)
{
	LOGMASKED(LOG_CLOCK, "CLK%s\n", state? "?" : "?");
	// Set the FF
	if (state==ASSERT_LINE)
	{
		m_pin14q = m_pin14d;
		m_pin15q = m_pin15d;
		m_pin16q = m_pin16d;
		m_pin17q = m_pin17d;
		set_ready();
	}
}

/*
    Called from clock_in and input pin functions. Set the state of the
    READY line. Since this is the only source for the state, we always have
    0 or 1, but no Z state.

    FIXME: The video write wait state handling is still not correct. Unfortunately,
    the equations of the PAL did not help, in contrast, the problem got worse.
    Problem: After initiating a video write (CSW*=0), the READY line must
    remain high for the next falling clock edge so that the CPU can complete
    the command cycle. By these equations, the READY line immediately goes
    low when the clock line rises, and thus on the falling edge, a wait state
    is produced.
    The effect is that in the real machine, if all code runs in on-chip memory,
    a video write does not cause any wait state, while in this emulation, it
    always causes the full 14 cycles of wait states. In rare situations, this
    may heavily slow down the processing.
*/
void geneve_pal_device::set_ready()
{
	line_state ready_line;

	// Original equations from the PAL
/*
    ready_line = ((!m_pin4 && !m_pin3) ||
                 (!m_pin9 && !m_pin3) ||
                 (m_pin9 && m_pin4 && !m_pin14q) ||
                 (m_pin5 && m_pin4 && !m_pin15q) ||
                 (m_pin5 && m_pin4 && !m_pin16q) ||
                 (m_pin9 && m_pin4 && !m_pin17q))? CLEAR_LINE : ASSERT_LINE;

    m_pin17d = (m_pin9 && m_pin4 && !m_pin17q && !m_pin14q) ||
             (m_pin9 && m_pin4 && !m_pin17q && !m_pin15q) ||
             (m_pin9 && m_pin4 && !m_pin17q && !m_pin16q) ||
             (m_pin9 && m_pin4 && m_pin3 && m_pin16q && m_pin15q && m_pin14q) ||
             (!m_pin19 && !m_pin17q && !m_pin14q) ||
             (!m_pin19 && !m_pin17q && !m_pin15q) ||
             (!m_pin19 && !m_pin17q && !m_pin16q) ||
             (m_pin3 && !m_pin19 && m_pin16q && m_pin15q && m_pin14q);

    m_pin16d = (m_pin9 && m_pin4 && m_pin17q && !m_pin16q) ||
             (m_pin9 && m_pin4 && !m_pin17q && m_pin16q) ||
             (m_pin9 && m_pin4 && m_pin16q && m_pin15q && m_pin14q) ||
             !m_pin19;

    m_pin15d = (m_pin9 && m_pin4 && m_pin15q && m_pin14q) ||
             (m_pin9 && m_pin4 && !m_pin16q && m_pin15q) ||
             (m_pin9 && m_pin4 && !m_pin17q && m_pin15q) ||
             (m_pin9 && m_pin4 && m_pin17q && m_pin16q && !m_pin15q) ||
             !m_pin19;

    m_pin14d = (m_pin9 && m_pin4 && m_pin14q) ||
              (m_pin9 && m_pin4 && m_pin17q && m_pin16q && m_pin15q) ||
              !m_pin19;
*/

	// Simplified equations for better performance

	bool pin4_9 = m_pin9 && m_pin4;

	ready_line = ((!m_pin3 && !pin4_9) ||
				  (pin4_9 && (!m_pin14q || !m_pin17q)) ||
				  (m_pin5 && m_pin4 && (!m_pin15q || !m_pin16q)))? CLEAR_LINE : ASSERT_LINE;

	m_pin17d = (pin4_9 && !m_pin17q && (!m_pin14q || !m_pin15q || !m_pin16q)) ||
			 (((pin4_9 && m_pin3) || (m_pin3 && !m_pin19)) && m_pin16q && m_pin15q && m_pin14q) ||
			 (!m_pin19 && !m_pin17q && (!m_pin14q || !m_pin15q || !m_pin16q));

	m_pin16d = (pin4_9 && m_pin17q && !m_pin16q) ||
			 (pin4_9 && !m_pin17q && m_pin16q) ||
			 (pin4_9 && m_pin16q && m_pin15q && m_pin14q) ||
			 !m_pin19;

	m_pin15d = (pin4_9 && m_pin15q && (m_pin14q || !m_pin16q || !m_pin17q)) ||
			 (pin4_9 && m_pin17q && m_pin16q && !m_pin15q) ||
			 !m_pin19;

	m_pin14d = (pin4_9 && m_pin14q) ||
			  (pin4_9 && m_pin17q && m_pin16q && m_pin15q) ||
			  !m_pin19;

	if (m_prev_ready != ready_line)
	{
		LOGMASKED(LOG_WAIT, "READY = %d (%d %d %d %d, %d %d %d, %d %d)\n", ready_line, m_pin14d, m_pin15d, m_pin16d, m_pin17d, m_pin3, m_pin4, m_pin9, m_pin5, m_pin19);
		m_prev_ready = ready_line;
	}
	m_ready(ready_line);
}

void geneve_pal_device::device_start()
{
	m_ready.resolve_safe();

	save_item(NAME(m_pin3));
	save_item(NAME(m_pin4));
	save_item(NAME(m_pin5));
	save_item(NAME(m_pin9));
	save_item(NAME(m_pin19));
	save_item(NAME(m_pin17q));
	save_item(NAME(m_pin16q));
	save_item(NAME(m_pin15q));
	save_item(NAME(m_pin14q));
}

/********************************************************************
  Genmod daughterboard, soldered to the back of the Gate Array
  The main task of the Genmod is to route all memory accesses to the
  Memex card in the peripheral box.

  Also, the Genmod may inhibit wait states by flipping on the TURBO switch.

  The TIMODE switch blocks the box access for the DRAM space (pages
  00-3F) so that the GROM emulator remains functional. Without this GROM
  emulation (which is hardwired to the board DRAM), the TI-99/4A
  software cannot run.

********************************************************************/

genmod_decoder_device::genmod_decoder_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GENMOD_DECODER, tag, owner, clock),
	m_debug(false),
	m_turbo(false),
	m_timode(false),
	m_function(0),
	m_function_debug(0),
	m_page(0),
	m_page_debug(0),
	m_gaready(0),
	m_extready(ASSERT_LINE),
	m_sndready(ASSERT_LINE)
{
}

void genmod_decoder_device::set_function(int func, int page)
{
	if (m_debug)
	{
		m_function_debug = func;
		m_page_debug = page;
	}
	else
	{
		m_function = func;  // values from logical map or physical map
		m_page = page;
	}
}

WRITE_LINE_MEMBER(genmod_decoder_device::gaready_in)
{
	m_gaready = state;
}

/*
    READY line from the box. Do not ignore this line, as it is important
    for device operation.
*/
WRITE_LINE_MEMBER(genmod_decoder_device::extready_in)
{
	m_extready = state;
}

/*
    READY line from the sound chip.
*/
WRITE_LINE_MEMBER(genmod_decoder_device::sndready_in)
{
	m_sndready = state;
}

/*
    Wait state generation
    The Genmod board does not look inside the Gate Array. The call we are using
    is just a shorthand for evaluating the states of the select lines.
*/
READ_LINE_MEMBER(genmod_decoder_device::gaready_out)
{
	int func = m_debug? m_function_debug : m_function;
	int page = m_debug? m_page_debug : m_page;

	// External or sound READY must always be respected
	if (m_extready==CLEAR_LINE || m_sndready==CLEAR_LINE) return CLEAR_LINE;

	// When TURBO is off, pass through
	if (!m_turbo) return m_gaready;

	// When accessing internal devices, pass through
	if (geneve_gate_array_device::accessing_devs_s(func)) return m_gaready;

	// In TURBO mode:
	// When TIMODE is active, and we access the DRAM area, pass through
	if (m_timode && ((page & 0xc0)==0)) return m_gaready;

	// When accessing SRAM, SRAMX, EPROM, or page BA, pass through
	if ((((page & 0xf0)==0xf0) || ((page&0xf8)==0xe8) || page == 0xba)) return m_gaready;

	// else no wait states
	return ASSERT_LINE;
}

/*
    Genmod accesses the box also for the DRAM range, but only if timode==0.
    (This includes GROM and cartridge ROM access.)
    Note: It is not sufficient to check for the page area; we need to check
    the select lines (via the static functions).
*/
READ_LINE_MEMBER(genmod_decoder_device::dben_out)
{
	int func = m_debug? m_function_debug : m_function;

	if (geneve_gate_array_device::accessing_box_s(func, true))
		return ASSERT_LINE;

	if (!m_timode && geneve_gate_array_device::accessing_dram_s(func))
		return ASSERT_LINE;

	// This needs to be verified with a real device.
	if (geneve_gate_array_device::accessing_sram_s(func))
		return ASSERT_LINE;

	return CLEAR_LINE;
}

void genmod_decoder_device::device_start()
{
	save_item(NAME(m_debug));
	save_item(NAME(m_turbo));
	save_item(NAME(m_timode));
	save_item(NAME(m_function));
	save_item(NAME(m_function_debug));
	save_item(NAME(m_page));
	save_item(NAME(m_page_debug));
	save_item(NAME(m_gaready));
	save_item(NAME(m_extready));
	save_item(NAME(m_sndready));
}

} // end namespace bus::ti99::internal

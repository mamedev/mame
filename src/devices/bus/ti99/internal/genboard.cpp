// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Geneve 9640 Gate Array and more components

    This file contains the emulation of the gate array and of the PAL chip
    that is used to control wait state generation.

    Pins of the Gate Array:

    in:  AMC, AMB, AMA, A0..A15: Address bus
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
    out: AB0, AB1, AB2: Mapped address bits
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

    Onboard SRAM configuration:
    There is an adjustable SRAM configuration on board, representing the
    various enhancements by users.

    The standard memory configuration as reported by chkdsk (32 KiB):
    557056 bytes of total memory

    With 64 KiB SRAM:
    589824 bytes of total memory

    With 384 KiB SRAM:
    917504 bytes of total memory

    The original 32 KiB SRAM memory needs to be expanded to 64 KiB for
    MDOS 2.50s and higher, or the system will lock up. Therefore the emulation
    default is 64 KiB.

    The ultimate expansion is a 512 KiB SRAM circuit wired to the gate array
    to provide 48 pages of fast static RAM. This also requires to build an
    adapter for a larger socket. From the 512 KiB, only 384 KiB will be
    accessed, since the higher pages are hidden behind the EPROM pages.

    === Address map ===
    p,q = page value bit (q = AMC, AMB, AMA)
    c = address offset within 8 KiB page

    p pqqq pppc cccc cccc cccc

    0 0... .... .... .... .... on-board dram 512 KiB

    0 1... .... .... .... .... on-board future expansion 512 KiB or Memex with Genmod

    1 00.. .... .... .... .... p-box AMA=0 (256 KiB)
    1 010. .... .... .... .... p-box AMA=1 AMB=0 (128 KiB)
    1 0110 .... .... .... .... p-box AMA=1 AMB=1 AMC=0 (64 KiB)

    1 0111 00.. .... .... .... p-box address block 0xxx, 2xxx
    1 0111 010. .... .... .... p-box address block 4xxx (DSR)
    1 0111 011. .... .... .... p-box address block 6xxx
    1 0111 100. .... .... .... p-box address block 8xxx (Speech at 0x9000)
    1 0111 101. .... .... .... p-box address block axxx
    1 0111 11.. .... .... .... p-box address block cxxx, exxx

    1 100. .... .... .... .... on-board sram (128K) -\
    1 101. .... .... .... .... on-board sram (128K) --+- maximum SRAM expansion
    1 1100 .... .... .... .... on-board sram (64K) --/
    1 1101 0... .... .... .... on-board sram (32K) - additional 32 KiB required for MDOS 2.50s and higher
    1 1101 1... .... .... .... on-board sram (32K) - standard setup

    1 111. ..0. .... .... .... on-board boot1
    1 111. ..1. .... .... .... on-board boot2

    The TI console (or more precise, the Flex Cable Interface) sets the AMA/B/C
    lines to 1. Most cards actually check for AMA/B/C=1. However, this decoding
    was forgotten in third party cards which cause the card address space
    to be mirrored. The usual DSR space at 4000-5fff which would be reachable
    via page 0xba is then mirrored on a number of other pages:

    10 xxx 010x = 82, 8a, 92, 9a, a2, aa, b2, ba

    Another block to take care of is 0xbc which covers 8000-9fff since this
    area contains the speech synthesizer port at 9000/9400.

    For the standard Geneve, only prefix 10 is routed to the P-Box. The Genmod
    modification wires these address lines to pins 8 and 9 in the P-Box as AMD and
    AME. This requires all cards to be equipped with an additional selection logic
    to detect AMD=0, AME=1. Otherwise these cards, although completely decoding the
    19-bit address, would reappear at 512 KiB distances.

    Genmod's double switch box is also emulated. There are two switches:
    - Turbo mode: Activates or deactivates the wait state logic on the Geneve
      board. This switch may be changed at any time.
    - TI mode: Selects between the on-board memory, which is required
      for the GPL interpreter, and the external Memex memory. This switch
      triggers a reset when changed.


    ===================
    Mapping
    ===================

    Logical address space: 64 KiB

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
    Speech:   9000 / 9400         1001 0w.. .... ...0
    Grom:     9800 / 9802         1001 1w.. .... ..x0
              9c00 / 9c02

    Physical address space
    ----------------------
    Address space size = 2 MiB

    Start    End      Phys.pages
    000000 - 07FFFF   00-3F   512 KiB DRAM on-board
       06C000 - 06DFFF   36     Cartridge space first 8K
       06E000 - 06FFFF   37     Cartridge space second 8K
    080000 - 0FFFFF   40-7F   512 KiB on-board expansion (never used)
    100000 - 16FFFF   80-B7   448 KiB P-Box space (special cards, like MEMEX)
    170000 - 17FFFF   B8-BF    64 KiB P-Box space (current cards)
    180000 - 1DFFFF   C0-EF   384 KiB SRAM space on-board; stock Geneve comes with 32 KiB
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
    ------------------
    Waitstates are caused by a cleared READY line of the TMS9995 processor
    during an external memory cycle. That means that waitstates have no effect
    for operations within the on-chip memory, and only when an access to the
    external memory or other devices occurs, a delay will be noticed.

    The waitstates are generated by the custom Gate Array chip on the board
    and the PAL 16R4, both lacking proper documentation. All of the following
    numbers have been determined by experiments with the real machine.

    Waitstates are generated for:
    - memory-mapped devices (mapper, clock, keyboard): 1 WS
    - accesses to the peripheral expansion box: 1 WS
    - accesses to on-board DRAM: 1 WS
    - accesses to video: 15 WS
    - accesses to SRAM: 0 WS

    Additional waitstates are created when one of the CRU bits is set. In that
    case, all delays are extended to 2 WS (including SRAM).

    Sound waitstates are somewhat unpredictable. It seems as if they depend
    on the clock of the sound chip; the READY line is pulled down until the
    next clock pulse, which may take some value between 18 CPU cycles and
    30 CPU cycles.

    The gate array is able to create wait states for video accesses. However,
    these wait states are effective after the video access has been completed.
    Wait states are not effective when the execution is running in on-chip
    RAM. Additional wait states are requested by m_video_waitstates = true.
    Without additional wait states, the video access takes the usual 1 or 2 WS.

    Waitstate behavior (Nov 2013)
       Almost perfect. Only video read access from code in DRAM is too fast
       by one WS

    ==========================
    PFM expansion
    ==========================

    The "Programmable Flash Memory expansion" is a replacement for the boot
    EPROM.

    PFM: Original version, 128 KiB
    PFM+: Expansion of the original version, piggybacked, adds another 128KiB
    PFM512: Using an AT29C040 (not A), 512 KiB

    The PFM is visible as four banks in memory pages 0xF0 - 0xFF.

    Bank switching is done by four 9901 pins:

    0028: LSB of bank number
    003A: MSB of bank number

    Bank 0 is the boot code, while banks 1-3 can be used as flash drives

    Michael Zapf, October 2011
    February 2012: rewritten as class, restructured
    Aug 2015: PFM added

***************************************************************************/
#include "emu.h"

#define LOG_WARN     (1U<<1)
#define LOG_DETAIL   (1U<<2)
#define LOG_READ     (1U<<3)
#define LOG_WRITE    (1U<<4)
#define LOG_KEYBOARD (1U<<5)
#define LOG_CLOCK    (1U<<6)
#define LOG_LINES    (1U<<7)
#define LOG_SETTING  (1U<<8)
#define LOG_VIDEOWS  (1U<<9)
#define LOG_PFM      (1U<<10)
#define LOG_DECODE   (1U<<11)
#define LOG_INVADDR  (1U<<12)

// Minimum log should be settings and warnings
#define VERBOSE ( LOG_SETTING | LOG_WARN )

#include "genboard.h"
#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(GENEVE_GATE_ARRAY, bus::ti99::internal, geneve_gate_array_device, "geneve_gate_array", "Geneve Gate Array")
DEFINE_DEVICE_TYPE_NS(GENMOD_GATE_ARRAY, bus::ti99::internal, genmod_gate_array_device, "genmod_gate_array", "Geneve Mod Gate Array")

namespace bus { namespace ti99 { namespace internal {

geneve_gate_array_device::geneve_gate_array_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock),
	m_boot_rom(0),
	m_gromwaddr_LSB(false),
	m_gromraddr_LSB(false),
	m_grom_address(0),
	m_video_waitstates(false),
	m_extra_waitstates(false),
	m_ready_asserted(false),
	m_read_mode(false),
	m_debug_no_ws(false),
	m_geneve_mode(false),
	m_direct_mode(false),
	m_cartridge_size(0),
	m_cartridge_secondpage(false),
	m_cartridge6_writable(false),
	m_cartridge7_writable(false),
	m_pfm_bank(0),
	m_pfm_output_enable(false),
	m_sram_mask(0),
	m_sram_val(0),
	m_ready(*this),
	m_keyint(*this),
	m_waitcount(0),
	m_video_waitcount(0),
	m_keyboard_shift_reg(0),
	m_keyboard_last_clock(CLEAR_LINE),
	m_keyboard_data_in(CLEAR_LINE),
	m_shift_reg_enabled(false),
	m_cpu(*owner, "maincpu"),
	m_sound(*owner, TI_SOUNDCHIP_TAG),
	m_video(*owner, TI_VDP_TAG),
	m_rtc(*owner, GENEVE_CLOCK_TAG),
	m_sram(*this, GENEVE_SRAM_PAR_TAG),
	m_dram(*this, GENEVE_DRAM_PAR_TAG),
	m_peribox(*owner, TI_PERIBOX_TAG),
	m_pfm512(*owner, GENEVE_PFM512_TAG),
	m_pfm512a(*owner, GENEVE_PFM512A_TAG),
	m_keyb_conn(*owner, GENEVE_KEYBOARD_CONN_TAG)
{
}

geneve_gate_array_device::geneve_gate_array_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: geneve_gate_array_device(mconfig, GENEVE_GATE_ARRAY, tag, owner, clock)
{
	m_eprom = nullptr;
	m_pbox_prefix = 0x070000;
}

genmod_gate_array_device::genmod_gate_array_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: geneve_gate_array_device(mconfig, GENMOD_GATE_ARRAY, tag, owner, clock),
	m_gm_timode(false),
	m_turbo(false)
{
	m_eprom = nullptr;
	m_pbox_prefix = 0x170000;
}

INPUT_CHANGED_MEMBER( geneve_gate_array_device::settings_changed )
{
	// Used when switching the boot ROMs during runtime, especially the PFM
	m_boot_rom = newval;
}

INPUT_CHANGED_MEMBER( genmod_gate_array_device::setgm_changed )
{
	int number = int(param&0x03);
	int value = newval;

	switch (number)
	{
	case 1:
		// Turbo switch. May be changed at any time.
		LOGMASKED(LOG_SETTING, "Setting turbo flag to %d\n", value);
		m_turbo = (value!=0);
		break;
	case 2:
		// TIMode switch. Causes reset when changed.
		LOGMASKED(LOG_SETTING, "Setting timode flag to %d\n", value);
		m_gm_timode = (value!=0);
		machine().schedule_hard_reset();
		break;
	case 3:
		// Used when switching the boot ROMs during runtime, especially the PFM
		m_boot_rom = value;
		break;
	default:
		LOGMASKED(LOG_WARN, "Unknown setting %d ignored\n", number);
	}
}

/****************************************************************************
    GROM simulation. The Geneve board simulated GROM circuits within its gate
    array.
*****************************************************************************/

/*
    Simulates GROM. The real Geneve does not use GROMs but simulates them
    within the gate array. Unlike with real GROMs, no address wrapping occurs,
    and the complete 64K space is available.
*/
uint8_t geneve_gate_array_device::read_grom(offs_t offset)
{
	uint8_t reply;
	if (offset & 0x0002)
	{
		// GROM address handling
		m_gromwaddr_LSB = false;

		if (m_gromraddr_LSB)
		{
			reply = m_grom_address & 0xff;
			m_gromraddr_LSB = false;
		}
		else
		{
			reply = (m_grom_address >> 8) & 0xff;
			m_gromraddr_LSB = true;
		}
	}
	else
	{
		// GROM data handling
		// GROMs are stored in pages 38..3f
		int physpage = 0x38;
		reply = m_dram->pointer()[(physpage<<13) + m_grom_address];
		m_grom_address = (m_grom_address + 1) & 0xffff;
		m_gromraddr_LSB = m_gromwaddr_LSB = false;
	}
	return reply;
}

/*
    Simulates GROM. The real Geneve does not use GROMs but simulates them
    within the gate array.
*/
void geneve_gate_array_device::write_grom(offs_t offset, uint8_t data)
{
	if (offset & 0x0002)
	{
		// set address
		m_gromraddr_LSB = false;
		if (m_gromwaddr_LSB)
		{
			m_grom_address = (m_grom_address & 0xff00) | data;
			m_grom_address = (m_grom_address + 1) & 0xffff;
			m_gromwaddr_LSB = false;
		}
		else
		{
			m_grom_address = (m_grom_address & 0x00ff) | ((uint16_t)data<<8);
			m_gromwaddr_LSB = true;
		}
	}
	else
	{   // write GPL data
		// The Geneve GROM simulator allows for GROM writing (verified with a real system)
		int physpage = 0x38;
		m_dram->pointer()[(physpage<<13) + m_grom_address] = data;

		m_grom_address = (m_grom_address + 1) & 0xffff;
		m_gromraddr_LSB = m_gromwaddr_LSB = false;
	}
}

void geneve_gate_array_device::set_wait(int min)
{
	if (m_extra_waitstates && min < 2) min = 2;

	// if we still have video wait states, do not set this counter
	// (or it will assert READY when expiring)
	if (m_video_waitcount > min) return;

	// need one more pass so that READY will be asserted again
	m_waitcount = min + 1;
	if (m_waitcount > 1)
	{
		LOGMASKED(LOG_LINES, "Pulling down READY line for %d cycles\n", min);
		m_ready(CLEAR_LINE);
		m_ready_asserted = false;
	}
}

void geneve_gate_array_device::set_video_waitcount(int min)
{
	if (m_debug_no_ws) return;
	m_video_waitcount = min;
}

void geneve_gate_array_device::set_geneve_mode(bool geneve)
{
	LOGMASKED(LOG_SETTING, "Setting Geneve mode = %d\n", geneve);
	m_geneve_mode = geneve;
}

void geneve_gate_array_device::set_direct_mode(bool direct)
{
	LOGMASKED(LOG_SETTING, "Setting direct mode = %d\n", direct);
	m_direct_mode = direct;
}

void geneve_gate_array_device::set_cartridge_size(int size)
{
	LOGMASKED(LOG_SETTING, "Setting cartridge size to %d\n", size);
	m_cartridge_size = size;
}

void geneve_gate_array_device::set_cartridge_writable(int base, bool write)
{
	LOGMASKED(LOG_SETTING, "Cartridge %04x space writable = %d\n", base, write);
	if (base==0x6000) m_cartridge6_writable = write;
	else m_cartridge7_writable = write;
}

void geneve_gate_array_device::set_video_waitstates(bool wait)
{
	// Tends to be called repeatedly
	if (m_video_waitstates != wait)
	{
		LOGMASKED(LOG_SETTING, "Setting video waitstates = %d\n", wait);
	}
	m_video_waitstates = wait;
}

void geneve_gate_array_device::set_extra_waitstates(bool wait)
{
	LOGMASKED(LOG_SETTING, "Setting extra waitstates = %d\n", wait);
	m_extra_waitstates = wait;
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
	m_keyb_conn->clock_write_from_mb(state);
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
	m_keyb_conn->data_write_from_mb(1 - (m_keyboard_shift_reg & 1));
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

/************************************************************************
    Called by the address map
************************************************************************/

/*
    Read a byte via the data bus. The decoding has already been done in the
    SETADDRESS method, and we re-use the values stored there to quickly
    access the appropriate component.
*/
uint8_t geneve_gate_array_device::readm(offs_t offset)
{
	uint8_t value = 0;

	decdata *dec;
	decdata debug;

	// For the debugger, do the decoding here with no wait states
	if (machine().side_effects_disabled())
	{
		if (m_cpu->is_onchip(offset)) return m_cpu->debug_read_onchip_memory(offset&0xff);
		dec = &debug;
		m_debug_no_ws = true;
		dec->offset = offset;
		decode_logical(true, dec);
		if (dec->function == MUNDEF)
		{
			map_address(m_read_mode, dec);
			decode_physical(dec);
			decode_mod(dec);
		}
		if (dec->function == MBOX)
		{
			m_peribox->memen_in(ASSERT_LINE);
			m_peribox->setaddress_dbin(dec->physaddr, true);
		}
	}
	else
	{
		// Use the values found in the setaddress phase
		dec = &m_decoded;
		m_debug_no_ws = false;
	}

	// Logical space

	switch (dec->function)
	{
	case MLVIDEO:
		if (!machine().side_effects_disabled())
		{
			value = m_video->read(dec->offset>>1);
			LOGMASKED(LOG_READ, "Read video %04x -> %02x\n", dec->offset, value);
			// Video wait states are created *after* the access
			// Accordingly, they have no effect when execution is in onchip RAM
			if (m_video_waitstates) set_video_waitcount(15);
		}
		break;

	case MLMAPPER:
		// mapper
		value = m_map[dec->offset & 0x0007];
		LOGMASKED(LOG_READ, "Read mapper %04x -> %02x\n", dec->offset, value);
		break;

	case MLKEY:
		// key
		value = m_keyboard_shift_reg>>1;
		LOGMASKED(LOG_KEYBOARD, "Read keyboard -> %02x\n", value);
		break;

	case MLCLOCK:
		// clock
		// Tests on the real machine showed that the upper nibble is 0xf
		// (probably because of the location at f130-f13f?)
		// In TI mode, however, the upper nibble is 1, unless we read 801f,
		// in which case the nibble is 2. Here the location is 8010-801f.
		// Needs more investigation. We might as well ignore this,
		// as the high nibble is obviously undefined and takes some past
		// value floating around.
		value = m_rtc->read(dec->offset & 0x000f);
		if (m_geneve_mode) value |= 0xf0;
		else value |= ((dec->offset & 0x000f)==0x000f)? 0x20 : 0x10;
		LOGMASKED(LOG_READ, "Read clock %04x -> %02x\n", dec->offset, value);
		break;

	case MLGROM:
		// grom simulation
		// ++++ ++-- ---- ---+
		// 1001 1000 0000 00x0
		if (!machine().side_effects_disabled()) value = read_grom(dec->offset);
		LOGMASKED(LOG_READ, "Read GROM %04x -> %02x\n", dec->offset, value);
		break;

	case MLSOUND:
		value = 0;
		break;

	case MPDRAM:
		// DRAM. One wait state.
		value = m_dram->pointer()[dec->physaddr];
		LOGMASKED(LOG_READ, "Read DRAM %04x (%06x) -> %02x\n", dec->offset, dec->physaddr, value);
		break;

	case MPEXP:
		// On-board memory expansion for standard Geneve (never used)
		LOGMASKED(LOG_READ, "Read on-board expansion (not available) %06x -> 00\n", dec->physaddr);
		value = 0;
		break;

	case MPEPROM:
		// 1 111. ..xx xxxx xxxx xxxx on-board eprom (16K)
		// mirrored for f0, f2, f4, ...; f1, f3, f5, ...
		value = boot_rom(dec->physaddr);
		break;

	case MPSRAM:
		if ((dec->physaddr & m_sram_mask)==m_sram_val)
		{
			value = m_sram->pointer()[dec->physaddr & ~m_sram_mask];
			LOGMASKED(LOG_READ, "Read SRAM %04x (%06x) -> %02x\n", dec->offset, dec->physaddr, value);
		}
		else
		{
			LOGMASKED(LOG_INVADDR, "Decoded as SRAM read, but no SRAM at %06x\n", dec->physaddr);
			value = 0;
		}
		// Return in any case
		break;

	case MBOX:
		// Route everything else to the P-Box
		//   0x000000-0x07ffff for the stock Geneve (AMC,AMB,AMA,A0 ...,A15)
		//   0x000000-0x1fffff for the GenMod.(AME,AMD,AMC,AMB,AMA,A0 ...,A15)

		m_peribox->readz(dec->physaddr, &value);
		m_peribox->memen_in(CLEAR_LINE);
		LOGMASKED(LOG_READ, "Read P-Box %04x (%06x) -> %02x\n", dec->offset, dec->physaddr, value);
		break;

	default:
		LOGMASKED(LOG_WARN, "Unknown decoding result type: %d\n", dec->function);
		break;
	}
	return value;
}

void geneve_gate_array_device::writem(offs_t offset, uint8_t data)
{
	decdata *dec;
	decdata debug;

	// For the debugger, do the decoding here with no wait states
	if (machine().side_effects_disabled())
	{
		// TODO: add debug_write_onchip_memory
		dec = &debug;
		m_debug_no_ws = true;
		dec->offset = offset;
		decode_logical(false, dec);
		if (dec->function == MUNDEF)
		{
			map_address(m_read_mode, dec);
			decode_physical(dec);
			decode_mod(dec);
		}
		if (dec->function == MBOX)
		{
			m_peribox->memen_in(ASSERT_LINE);
			m_peribox->setaddress_dbin(dec->physaddr, false);
		}
	}
	else
	{
		// Use the values found in the setaddress phase
		dec = &m_decoded;
		m_debug_no_ws = false;
	}


	// Logical space

	switch (dec->function)
	{
	case MLVIDEO:
		// video
		// ++++ ++++ ++++ ---+
		// 1111 0001 0000 .cc0
		// Initialize waitstate timer

		if (!machine().side_effects_disabled())
		{
			m_video->write(dec->offset>>1, data);
			LOGMASKED(LOG_WRITE, "Write video %04x <- %02x\n", offset, data);
			// See above
			if (m_video_waitstates) set_video_waitcount(15);
		}
		break;

	case MLMAPPER:
		// mapper
		m_map[dec->offset & 0x0007] = data;
		LOGMASKED(LOG_WRITE, "Write mapper %04x <- %02x\n", offset, data);
		break;

	case MLCLOCK:
		// clock
		// ++++ ++++ ++++ ----
		m_rtc->write(dec->offset & 0x000f, data);
		LOGMASKED(LOG_WRITE, "Write clock %04x <- %02x\n", offset, data);
		break;

	case MLSOUND:
		// sound
		// ++++ ++++ ++++ ---+
		m_sound->write(data);
		LOGMASKED(LOG_WRITE, "Write sound <- %02x\n", data);
		break;

	case MLGROM:
		// The GROM simulator is only available in TI Mode
		write_grom(dec->offset, data);
		LOGMASKED(LOG_WRITE, "Write GROM %04x <- %02x\n", offset, data);
		break;

	// Physical space
	case MPDRAM:
		// DRAM write
		m_dram->pointer()[dec->physaddr] = data;
		LOGMASKED(LOG_WRITE, "Write DRAM %04x (%06x) <- %02x\n", offset, dec->physaddr, data);
		break;

	case MPEXP:
		// On-board memory expansion for standard Geneve
		// Actually never built, so we show it as unmapped
		LOGMASKED(LOG_WRITE, "Write on-board expansion (not available) %06x <- %02x\n", dec->physaddr, data);
		break;

	case MPEPROM:
		// 1 111. ..xx xxxx xxxx xxxx on-board eprom (16K)
		// mirrored for f0, f2, f4, ...; f1, f3, f5, ...
		// Ignore EPROM write (unless PFM)
		if (m_boot_rom != GENEVE_EPROM) write_to_pfm(dec->physaddr, data);
		else
			LOGMASKED(LOG_INVADDR, "Write EPROM %04x (%06x) <- %02x, ignored\n", offset, dec->physaddr, data);
		break;

	case MPSRAM:
		if ((dec->physaddr & m_sram_mask)==m_sram_val)
		{
			m_sram->pointer()[dec->physaddr & ~m_sram_mask] = data;
			LOGMASKED(LOG_WRITE, "Write SRAM %04x (%06x) <- %02x\n", offset, dec->physaddr, data);
		}
		else
		{
			LOGMASKED(LOG_INVADDR, "Decoded as SRAM write, but no SRAM at %06x\n", dec->physaddr);
		}
		break;

	case MBOX:
		// Route everything else to the P-Box
		LOGMASKED(LOG_WRITE, "Write P-Box %04x (%06x) <- %02x\n", offset, dec->physaddr, data);
		m_peribox->write(dec->physaddr, data);
		m_peribox->memen_in(CLEAR_LINE);
		break;

	default:
		LOGMASKED(LOG_WARN, "Unknown decoding result type: %d\n", dec->function);
		break;
	}
}

const geneve_gate_array_device::logentry_t geneve_gate_array_device::s_logmap[7] =
{
	{ 0xf100, 0x000e, 0x8800, 0x03fe, 0x0400, MLVIDEO,  "video" },
	{ 0xf110, 0x0007, 0x8000, 0x0007, 0x0000, MLMAPPER, "mapper" },
	{ 0xf118, 0x0007, 0x8008, 0x0007, 0x0000, MLKEY,    "keyboard" },
	{ 0xf120, 0x000e, 0x8400, 0x03fe, 0x0000, MLSOUND,  "sound" },
	{ 0xf130, 0x000f, 0x8010, 0x000f, 0x0000, MLCLOCK,  "clock" },
	{ 0x0000, 0x0000, 0x9000, 0x03fe, 0x0400, MBOX,     "speech (in P-Box)" },
	{ 0x0000, 0x0000, 0x9800, 0x03fe, 0x0400, MLGROM,   "GROM" },
};

void geneve_gate_array_device::decode_logical(bool reading, geneve_gate_array_device::decdata* dec)
{
	dec->function = MUNDEF;
	dec->physaddr = m_pbox_prefix | dec->offset;
	dec->wait = 1;

	int i = 0;
	while (i < 7)
	{
		if (m_geneve_mode)
		{
			// Skip when genbase is 0
			if ((s_logmap[i].genbase != 0) && ((dec->offset & ~s_logmap[i].genmask) == s_logmap[i].genbase))
				break;
		}
		else
		{
			if (reading)
			{
				if ((dec->offset & ~s_logmap[i].timask) == s_logmap[i].tibase)
					break;
			}
			else
			{
				if ((dec->offset & ~s_logmap[i].timask) == (s_logmap[i].tibase | s_logmap[i].writeoff))
					break;
			}
		}
		i++;
	}
	if (i != 7)
	{
		LOGMASKED(LOG_DECODE, "Decoded as %s: %04x\n", s_logmap[i].description, dec->offset);
		dec->function = s_logmap[i].function;
	}
}

void geneve_gate_array_device::map_address(bool reading, geneve_gate_array_device::decdata* dec)
{
	int logpage = (dec->offset & 0xe000) >> 13;
	int physpage = 0;

	// Determine physical address
	if (m_direct_mode) physpage = 0xf8; // points to boot eprom
	else
	{
		// TI mode, accessing logical addresses 6000-7fff
		if (!m_geneve_mode && logpage==3)
		{
			if (reading)
			{
				physpage = (m_cartridge_size==0x4000 && m_cartridge_secondpage)? 0x37 : 0x36;
			}
			else
			{
				// Emulate the cartridge bank switch feature of Extended Basic
				// TODO: Is this the right place? Or writem()?
				if (m_cartridge_size==0x4000)
				{
					m_cartridge_secondpage = ((dec->offset & 0x0002)!=0);
					LOGMASKED(LOG_WRITE, "Set cartridge page %02x\n", m_cartridge_secondpage);
				}
				else
				{
					// writing into cartridge rom space (no bank switching)
					if ((((dec->offset & 0x1000)==0x0000) && !m_cartridge6_writable)
						|| (((dec->offset & 0x1000)==0x1000) && !m_cartridge7_writable))
					{
						LOGMASKED(LOG_WARN, "Writing to protected cartridge space %04x ignored\n", dec->offset);
					}
					else
						// TODO: Check whether secondpage is really ignored
						physpage = 0x36;
				}
			}
		}
		else
			physpage = m_map[logpage];
	}
	dec->physaddr = ((physpage << 13) | (dec->offset & 0x1fff)) & 0x1fffff;
}

const geneve_gate_array_device::physentry_t geneve_gate_array_device::s_physmap[4] =
{
	{ 0x000000, 0x07ffff, MPDRAM,  1, "DRAM" },
	{ 0x080000, 0x07ffff, MPEXP,   1, "on-board expansion" },
	{ 0x1e0000, 0x01ffff, MPEPROM, 0, "EPROM" },
	{ 0x180000, 0x07ffff, MPSRAM,  0, "SRAM" }
};

void geneve_gate_array_device::decode_physical(geneve_gate_array_device::decdata* dec)
{
	dec->function = MUNDEF;

	int i = 0;
	while (i < 4)
	{
		if ((dec->physaddr & ~s_physmap[i].mask) == s_physmap[i].base)
			break;
		i++;
	}
	if (i != 4)
	{
		LOGMASKED(LOG_DECODE, "Decoded as %s: %06x\n", s_physmap[i].description, dec->physaddr);
		dec->function = s_physmap[i].function;
		dec->wait = s_physmap[i].wait;
	}
	else
	{
		// Route everything else to the P-Box
		dec->function = MBOX;
		dec->wait = 1;
	}
}

void genmod_gate_array_device::decode_mod(geneve_gate_array_device::decdata* dec)
{
	// GenMod mode
	// The TI Mode switch activates the DRAM on the board (1 WS)
	// for the first 512K (000000-07ffff)
	if (((dec->function == MPDRAM) && !m_gm_timode) || dec->function==MPSRAM || dec->function==MPEXP)
	{
		dec->function = MBOX;
	}

	if ((dec->function != MPDRAM) && m_turbo)
		dec->wait = 0;
}

/*
    Boot ROM handling, from EPROM or PFM.
*/
uint8_t geneve_gate_array_device::boot_rom(offs_t offset)
{
	uint8_t value;
	int pfmaddress = (offset & 0x01ffff) | (m_pfm_bank<<17);

	switch (m_boot_rom)
	{
	case GENEVE_EPROM:
		value = m_eprom[offset & 0x003fff];
		LOGMASKED(LOG_READ, "Read EPROM %04x -> %02x\n", offset & 0x003fff, value);
		return value;
	case GENEVE_PFM512:
		value = m_pfm512->read(pfmaddress);
		break;
	case GENEVE_PFM512A:
		value = m_pfm512a->read(pfmaddress);
		break;
	default:
		LOGMASKED(LOG_WARN, "Illegal mode for reading boot ROM: %d\n", m_boot_rom);
		value = 0;
	}

	if (!m_pfm_output_enable) value = 0;
	LOGMASKED(LOG_PFM, "Reading from PFM at address %05x -> %02x\n", pfmaddress, value);
	return value;
}

void geneve_gate_array_device::write_to_pfm(offs_t offset, uint8_t data)
{
	// Nota bene: The PFM must be write protected on startup, or the RESET
	// of the 9995 will attempt to write the return vector into the flash EEPROM
	int address = (offset & 0x01ffff) | (m_pfm_bank<<17);
	LOGMASKED(LOG_PFM, "Writing to PFM at address %05x <- %02x\n", address, data);

	switch (m_boot_rom)
	{
	case GENEVE_PFM512:
		m_pfm512->write(address, data);
		break;
	case GENEVE_PFM512A:
		m_pfm512a->write(address, data);
		break;
	default:
		LOGMASKED(LOG_WARN, "Illegal mode for writing to PFM: %d\n", m_boot_rom);
	}
}


/*
    Accept the address passed over the address bus and decode it appropriately.
    This decoding will later be used in the READ/WRITE member functions. Also,
    we initiate wait state creation here.
*/
void geneve_gate_array_device::setaddress(offs_t address, uint8_t busctrl)
{
	LOGMASKED(LOG_DETAIL, "setaddress = %04x\n", address);
	m_debug_no_ws = false;
	m_decoded.offset = address;

	m_read_mode = ((busctrl & TMS99xx_BUS_DBIN)!=0);

	decode_logical(m_read_mode, &m_decoded);
	if (m_decoded.function == MUNDEF)
	{
		map_address(m_read_mode, &m_decoded);
		decode_physical(&m_decoded);
		decode_mod(&m_decoded);
	}

	set_wait(m_decoded.wait);

	if (m_decoded.function == MBOX)
	{
		m_peribox->memen_in(ASSERT_LINE);
		m_peribox->setaddress_dbin(m_decoded.physaddr, m_read_mode);
	}
}

/*
    The mapper is connected to the clock line in order to operate
    the wait state counter.
    The wait counter is decremented on each rising clock edge; when 0, the
    READY line is asserted. However, there is a second counter which is used for
    video wait states.
    The READY line must be asserted when the wait counter reaches 0, but must be
    cleared immediately again if the video counter has not reached 0.
    (See comments at the file header: The additional video wait states do not
    affect the video access itself but become effective after the access; if
    the code runs on the chip, these wait states are ignored.)
*/
WRITE_LINE_MEMBER( geneve_gate_array_device::clock_in )
{
	if (state==ASSERT_LINE)
	{
		// Rising edge
		if (!m_ready_asserted)
		{
			if (m_waitcount > 0)
			{
				m_waitcount--;
				if (m_waitcount == 0)
				{
					LOGMASKED(LOG_CLOCK, "clock, READY asserted\n");
					m_ready(ASSERT_LINE);
					m_ready_asserted = true;
				}
				else
				{
					LOGMASKED(LOG_CLOCK, "clock\n");
				}
			}
			else
			{
				if (m_video_waitcount > 0)
				{
					m_video_waitcount--;
					if (m_video_waitcount == 0)
					{
						LOGMASKED(LOG_CLOCK, "clock, READY asserted after video\n");
						m_ready(ASSERT_LINE);
						m_ready_asserted = true;
					}
					else
					{
						LOGMASKED(LOG_CLOCK, "vclock, ew=%d\n", m_video_waitcount);
					}
				}
			}
		}
	}
	else
	{
		// Falling edge
		// Do we have video wait states? In that case, clear the line again
		if ((m_waitcount == 0) && (m_video_waitcount > 0) && m_ready_asserted)
		{
			LOGMASKED(LOG_CLOCK, "clock, READY cleared for video\n");
			m_ready(CLEAR_LINE);
			m_ready_asserted = false;
		}
	}
}

/*
    PFM expansion: Setting the bank.
*/
WRITE_LINE_MEMBER( geneve_gate_array_device::pfm_select_lsb )
{
	if (state==ASSERT_LINE) m_pfm_bank |= 1;
	else m_pfm_bank &= 0xfe;
	LOGMASKED(LOG_PFM, "Setting bank (l) = %d\n", m_pfm_bank);
}

WRITE_LINE_MEMBER( geneve_gate_array_device::pfm_select_msb )
{
	if (state==ASSERT_LINE) m_pfm_bank |= 2;
	else m_pfm_bank &= 0xfd;
	LOGMASKED(LOG_PFM, "Setting bank (u) = %d\n", m_pfm_bank);
}

WRITE_LINE_MEMBER( geneve_gate_array_device::pfm_output_enable )
{
	// Negative logic
	m_pfm_output_enable = (state==CLEAR_LINE);
	LOGMASKED(LOG_PFM, "PFM output %s\n", m_pfm_output_enable? "enable" : "disable");
}

//====================================================================
//  Common device lifecycle
//====================================================================

void geneve_gate_array_device::device_start()
{
	m_ready.resolve_safe();
	m_keyint.resolve_safe();

	m_geneve_mode = false;
	m_direct_mode = true;

	// State registration
	save_item(NAME(m_gromwaddr_LSB));
	save_item(NAME(m_gromraddr_LSB));
	save_item(NAME(m_grom_address));
	save_item(NAME(m_video_waitstates));
	save_item(NAME(m_extra_waitstates));
	save_item(NAME(m_ready_asserted));
	save_item(NAME(m_read_mode));
	save_item(NAME(m_debug_no_ws));
	save_item(NAME(m_geneve_mode));
	save_item(NAME(m_direct_mode));
	save_item(NAME(m_cartridge_size));
	save_item(NAME(m_cartridge_secondpage));
	save_item(NAME(m_cartridge6_writable));
	save_item(NAME(m_cartridge7_writable));
	save_pointer(NAME(m_map), 8);
	save_item(NAME(m_decoded.function));
	save_item(NAME(m_decoded.offset));
	save_item(NAME(m_decoded.physaddr));
	save_item(NAME(m_boot_rom));
	save_item(NAME(m_pfm_bank));
	save_item(NAME(m_pfm_output_enable));
	save_item(NAME(m_sram_mask));
	save_item(NAME(m_sram_val));
	save_item(NAME(m_waitcount));
	save_item(NAME(m_video_waitcount));
}

void geneve_gate_array_device::common_reset()
{
	m_extra_waitstates = false;
	m_video_waitstates = true;
	m_read_mode = false;
	m_waitcount = 0;
	m_video_waitcount = 0;
	m_ready_asserted = true;

	m_geneve_mode =false;
	m_direct_mode = true;
	m_cartridge_size = 0x4000;
	m_cartridge_secondpage = false;
	m_cartridge6_writable = false;
	m_cartridge7_writable = false;
	m_grom_address = 0;
	m_pfm_bank = 0;
	m_pfm_output_enable = true;

	// Clear map
	for (auto & elem : m_map) elem = 0;

	// Check which boot EPROM we are using (or PFM)
	m_boot_rom = machine().root_device().ioport("BOOTROM")->read();
	m_eprom = machine().root_device().memregion("maincpu")->base();

	// Allow for configuring the VRAM size
	uint32_t videoram = (machine().root_device().ioport("VRAM")->read()!=0)? 0x30000 : 0x20000;
	downcast<v99x8_device &>(*m_video.target()).set_vram_size(videoram);
	LOGMASKED(LOG_SETTING, "Video RAM set to %d KiB\n", videoram / 1024);
}

void geneve_gate_array_device::device_reset()
{
	common_reset();

	// SRAM is only separately handled for the standard Geneve; Genmod uses
	// the Memex instead
	switch (machine().root_device().ioport("SRAM")->read())
	{
/*  1 100. .... .... .... .... on-board sram (128K) -+
    1 101. .... .... .... .... on-board sram (128K) -+-- maximum SRAM expansion
    1 1100 .... .... .... .... on-board sram (64K) --+
    1 1101 0... .... .... .... on-board sram (32K) - additional 32 KiB required for MDOS 2.50s and higher
    1 1101 1... .... .... .... on-board sram (32K) - standard setup
*/
	case 0: // 32 KiB
		m_sram_mask =   0x1f8000;
		m_sram_val =    0x1d8000;
		break;
	case 1: // 64 KiB
		m_sram_mask =   0x1f0000;
		m_sram_val =    0x1d0000;
		break;
	case 2: // 384 KiB (actually 512 KiB, but the EPROM masks the upper 128 KiB)
		m_sram_mask =   0x180000;
		m_sram_val =    0x180000;
		break;
	}
}

void genmod_gate_array_device::device_reset()
{
	common_reset();
	LOGMASKED(LOG_SETTING, "Using GenMod modification\n");
	m_turbo = ((machine().root_device().ioport("GENMODDIPS")->read() & GENEVE_GM_TURBO)!=0);
	m_gm_timode = ((machine().root_device().ioport("GENMODDIPS")->read() & GENEVE_GM_TIM)!=0);
}


} } } // end namespace bus::ti99::internal


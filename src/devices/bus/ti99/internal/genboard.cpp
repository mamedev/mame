// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Geneve 9640 mapper and more components

    This file contains 2 classes:
    - mapper: main function of the Gate Array on the Geneve board. Maps logical
        memory accesses to a wider address space using map registers.
    - keyboard: an implementation of a XT-style keyboard. This should be dropped
        and replaced by a proper XT keyboard implementation.

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

// Minimum log should be settings and warnings
#define VERBOSE ( LOG_SETTING | LOG_WARN )

#include "genboard.h"
#include "logmacro.h"

DEFINE_DEVICE_TYPE_NS(GENEVE_KEYBOARD, bus::ti99::internal, geneve_keyboard_device, "geneve_keyboard", "Geneve XT-style keyboard")
DEFINE_DEVICE_TYPE_NS(GENEVE_MAPPER, bus::ti99::internal, geneve_mapper_device, "geneve_mapper", "Geneve Gate Array")
DEFINE_DEVICE_TYPE_NS(GENMOD_MAPPER, bus::ti99::internal, genmod_mapper_device, "genmod_mapper", "Geneve Mod Gate Array")

namespace bus { namespace ti99 { namespace internal {

geneve_mapper_device::geneve_mapper_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock), m_gromwaddr_LSB(false),
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
	m_boot_rom(0),
	m_pfm_bank(0),
	m_pfm_output_enable(false),
	m_sram_mask(0),
	m_sram_val(0),
	m_ready(*this),
	m_waitcount(0),
	m_video_waitcount(0),
	m_clock(*owner, GENEVE_CLOCK_TAG),
	m_cpu(*owner, "maincpu"),
	m_pfm512(*owner, GENEVE_PFM512_TAG),
	m_pfm512a(*owner, GENEVE_PFM512A_TAG),
	m_sound(*owner, TI_SOUNDCHIP_TAG),
	m_keyboard(*owner, GENEVE_KEYBOARD_TAG),
	m_video(*owner, TI_VDP_TAG),
	m_peribox(*owner, TI_PERIBOX_TAG),
	m_sram(*this, GENEVE_SRAM_PAR_TAG),
	m_dram(*this, GENEVE_DRAM_PAR_TAG)
{
}

geneve_mapper_device::geneve_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: geneve_mapper_device(mconfig, GENEVE_MAPPER, tag, owner, clock)
{
	m_eprom = nullptr;
	m_pbox_prefix = 0x070000;
}

genmod_mapper_device::genmod_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: geneve_mapper_device(mconfig, GENMOD_MAPPER, tag, owner, clock),
	m_gm_timode(false),
	m_turbo(false)
{
	m_eprom = nullptr;
	m_pbox_prefix = 0x170000;
}

INPUT_CHANGED_MEMBER( geneve_mapper_device::settings_changed )
{
	// Used when switching the boot ROMs during runtime, especially the PFM
	m_boot_rom = newval;
}

INPUT_CHANGED_MEMBER( genmod_mapper_device::setgm_changed )
{
	int number = (int)((uint64_t)param&0x03);
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
READ8_MEMBER( geneve_mapper_device::read_grom )
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
WRITE8_MEMBER( geneve_mapper_device::write_grom )
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

void geneve_mapper_device::set_wait(int min)
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

void geneve_mapper_device::set_video_waitcount(int min)
{
	if (m_debug_no_ws) return;
	m_video_waitcount = min;
}

void geneve_mapper_device::set_geneve_mode(bool geneve)
{
	LOGMASKED(LOG_SETTING, "Setting Geneve mode = %d\n", geneve);
	m_geneve_mode = geneve;
}

void geneve_mapper_device::set_direct_mode(bool direct)
{
	LOGMASKED(LOG_SETTING, "Setting direct mode = %d\n", direct);
	m_direct_mode = direct;
}

void geneve_mapper_device::set_cartridge_size(int size)
{
	LOGMASKED(LOG_SETTING, "Setting cartridge size to %d\n", size);
	m_cartridge_size = size;
}

void geneve_mapper_device::set_cartridge_writable(int base, bool write)
{
	LOGMASKED(LOG_SETTING, "Cartridge %04x space writable = %d\n", base, write);
	if (base==0x6000) m_cartridge6_writable = write;
	else m_cartridge7_writable = write;
}

void geneve_mapper_device::set_video_waitstates(bool wait)
{
	// Tends to be called repeatedly
	if (m_video_waitstates != wait)
	{
		LOGMASKED(LOG_SETTING, "Setting video waitstates = %d\n", wait);
	}
	m_video_waitstates = wait;
}

void geneve_mapper_device::set_extra_waitstates(bool wait)
{
	LOGMASKED(LOG_SETTING, "Setting extra waitstates = %d\n", wait);
	m_extra_waitstates = wait;
}


/************************************************************************
    Called by the address map
************************************************************************/

/*
    Read a byte via the data bus. The decoding has already been done in the
    SETOFFSET method, and we re-use the values stored there to quickly
    access the appropriate component.
*/
READ8_MEMBER( geneve_mapper_device::readm )
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
			m_peribox->setaddress_dbin(space, dec->physaddr, true);
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
			value = m_video->read(space, dec->offset>>1);
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
		if (!machine().side_effects_disabled()) value = m_keyboard->get_recent_key();
		LOGMASKED(LOG_READ, "Read keyboard -> %02x\n", value);
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
		value = m_clock->read(space, dec->offset & 0x000f);
		if (m_geneve_mode) value |= 0xf0;
		else value |= ((dec->offset & 0x000f)==0x000f)? 0x20 : 0x10;
		LOGMASKED(LOG_READ, "Read clock %04x -> %02x\n", dec->offset, value);
		break;

	case MLGROM:
		// grom simulation
		// ++++ ++-- ---- ---+
		// 1001 1000 0000 00x0
		if (!machine().side_effects_disabled()) value = read_grom(space, dec->offset, 0xff);
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
		value = boot_rom(space, dec->physaddr, 0xff);
		break;

	case MPSRAM:
		if ((dec->physaddr & m_sram_mask)==m_sram_val)
		{
			value = m_sram->pointer()[dec->physaddr & ~m_sram_mask];
			LOGMASKED(LOG_READ, "Read SRAM %04x (%06x) -> %02x\n", dec->offset, dec->physaddr, value);
		}
		else
		{
			LOGMASKED(LOG_WARN, "Decoded as SRAM read, but no SRAM at %06x\n", dec->physaddr);
			value = 0;
		}
		// Return in any case
		break;

	case MBOX:
		// Route everything else to the P-Box
		//   0x000000-0x07ffff for the stock Geneve (AMC,AMB,AMA,A0 ...,A15)
		//   0x000000-0x1fffff for the GenMod.(AME,AMD,AMC,AMB,AMA,A0 ...,A15)

		m_peribox->readz(space, dec->physaddr, &value, 0xff);
		m_peribox->memen_in(CLEAR_LINE);
		LOGMASKED(LOG_READ, "Read P-Box %04x (%06x) -> %02x\n", dec->offset, dec->physaddr, value);
		break;

	default:
		LOGMASKED(LOG_WARN, "Unknown decoding result type: %d\n", dec->function);
		break;
	}
	return value;
}

WRITE8_MEMBER( geneve_mapper_device::writem )
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
			m_peribox->setaddress_dbin(space, dec->physaddr, false);
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
			m_video->write(space, dec->offset>>1, data);
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
		m_clock->write(space, dec->offset & 0x000f, data);
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
		write_grom(space, dec->offset, data, 0xff);
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
		if (m_boot_rom != GENEVE_EPROM) write_to_pfm(space, dec->physaddr, data, 0xff);
		else
			LOGMASKED(LOG_WARN, "Write EPROM %04x (%06x) <- %02x, ignored\n", offset, dec->physaddr, data);
		break;

	case MPSRAM:
		if ((dec->physaddr & m_sram_mask)==m_sram_val)
		{
			m_sram->pointer()[dec->physaddr & ~m_sram_mask] = data;
			LOGMASKED(LOG_WRITE, "Write SRAM %04x (%06x) <- %02x\n", offset, dec->physaddr, data);
		}
		else
		{
			LOGMASKED(LOG_WARN, "Decoded as SRAM write, but no SRAM at %06x\n", dec->physaddr);
		}
		break;

	case MBOX:
		// Route everything else to the P-Box
		LOGMASKED(LOG_WRITE, "Write P-Box %04x (%06x) <- %02x\n", offset, dec->physaddr, data);
		m_peribox->write(space, dec->physaddr, data, 0xff);
		m_peribox->memen_in(CLEAR_LINE);
		break;

	default:
		LOGMASKED(LOG_WARN, "Unknown decoding result type: %d\n", dec->function);
		break;
	}
}

void geneve_mapper_device::decode_logical(bool reading, geneve_mapper_device::decdata* dec)
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
			if ((m_logmap[i].genbase != 0) && ((dec->offset & ~m_logmap[i].genmask) == m_logmap[i].genbase))
				break;
		}
		else
		{
			if (reading)
			{
				if ((dec->offset & ~m_logmap[i].timask) == m_logmap[i].tibase)
					break;
			}
			else
			{
				if ((dec->offset & ~m_logmap[i].timask) == (m_logmap[i].tibase | m_logmap[i].writeoff))
					break;
			}
		}
		i++;
	}
	if (i != 7)
	{
		LOGMASKED(LOG_DECODE, "Decoded as %s: %04x\n", m_logmap[i].description, dec->offset);
		dec->function = m_logmap[i].function;
	}
}

void geneve_mapper_device::map_address(bool reading, geneve_mapper_device::decdata* dec)
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

void geneve_mapper_device::decode_physical(geneve_mapper_device::decdata* dec)
{
	dec->function = MUNDEF;

	int i = 0;
	while (i < 4)
	{
		if ((dec->physaddr & ~m_physmap[i].mask) == m_physmap[i].base)
			break;
		i++;
	}
	if (i != 4)
	{
		LOGMASKED(LOG_DECODE, "Decoded as %s: %06x\n", m_physmap[i].description, dec->physaddr);
		dec->function = m_physmap[i].function;
		dec->wait = m_physmap[i].wait;
	}
	else
	{
		// Route everything else to the P-Box
		dec->function = MBOX;
		dec->wait = 1;
	}
}

void genmod_mapper_device::decode_mod(geneve_mapper_device::decdata* dec)
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
READ8_MEMBER( geneve_mapper_device::boot_rom )
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
		value = m_pfm512->read(space, pfmaddress, mem_mask);
		break;
	case GENEVE_PFM512A:
		value = m_pfm512a->read(space, pfmaddress, mem_mask);
		break;
	default:
		LOGMASKED(LOG_WARN, "Illegal mode for reading boot ROM: %d\n", m_boot_rom);
		value = 0;
	}

	if (!m_pfm_output_enable) value = 0;
	LOGMASKED(LOG_PFM, "Reading from PFM at address %05x -> %02x\n", pfmaddress, value);
	return value;
}

WRITE8_MEMBER( geneve_mapper_device::write_to_pfm )
{
	// Nota bene: The PFM must be write protected on startup, or the RESET
	// of the 9995 will attempt to write the return vector into the flash EEPROM
	int address = (offset & 0x01ffff) | (m_pfm_bank<<17);
	LOGMASKED(LOG_PFM, "Writing to PFM at address %05x <- %02x\n", address, data);

	switch (m_boot_rom)
	{
	case GENEVE_PFM512:
		m_pfm512->write(space, address, data, mem_mask);
		break;
	case GENEVE_PFM512A:
		m_pfm512a->write(space, address, data, mem_mask);
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
READ8_MEMBER( geneve_mapper_device::setoffset )
{
	LOGMASKED(LOG_DETAIL, "setoffset = %04x\n", offset);
	m_debug_no_ws = false;
	m_decoded.offset = offset;

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
		m_peribox->setaddress_dbin(space, m_decoded.physaddr, m_read_mode);
	}
	return 0;
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
WRITE_LINE_MEMBER( geneve_mapper_device::clock_in )
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
    We need the DBIN line for the setoffset operation.
*/
WRITE_LINE_MEMBER( geneve_mapper_device::dbin_in )
{
	m_read_mode = (state==ASSERT_LINE);
	LOGMASKED(LOG_DETAIL, "dbin = %02x\n", m_read_mode? 1:0);
}

/*
    PFM expansion: Setting the bank.
*/
WRITE_LINE_MEMBER( geneve_mapper_device::pfm_select_lsb )
{
	if (state==ASSERT_LINE) m_pfm_bank |= 1;
	else m_pfm_bank &= 0xfe;
	LOGMASKED(LOG_PFM, "Setting bank (l) = %d\n", m_pfm_bank);
}

WRITE_LINE_MEMBER( geneve_mapper_device::pfm_select_msb )
{
	if (state==ASSERT_LINE) m_pfm_bank |= 2;
	else m_pfm_bank &= 0xfd;
	LOGMASKED(LOG_PFM, "Setting bank (u) = %d\n", m_pfm_bank);
}

WRITE_LINE_MEMBER( geneve_mapper_device::pfm_output_enable )
{
	// Negative logic
	m_pfm_output_enable = (state==CLEAR_LINE);
	LOGMASKED(LOG_PFM, "PFM output %s\n", m_pfm_output_enable? "enable" : "disable");
}

//====================================================================
//  Common device lifecycle
//====================================================================

void geneve_mapper_device::device_start()
{
	m_ready.resolve();

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

void geneve_mapper_device::common_reset()
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

void geneve_mapper_device::device_reset()
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

void genmod_mapper_device::device_reset()
{
	common_reset();
	LOGMASKED(LOG_SETTING, "Using GenMod modification\n");
	m_turbo = ((machine().root_device().ioport("GENMODDIPS")->read() & GENEVE_GM_TURBO)!=0);
	m_gm_timode = ((machine().root_device().ioport("GENMODDIPS")->read() & GENEVE_GM_TIM)!=0);
}

/****************************************************************************
    Keyboard support

    The XT keyboard interface is described in various places on the internet,
    like (http://www-2.cs.cmu.edu/afs/cs/usr/jmcm/www/info/key2.txt).  It is a
    synchronous unidirectional serial interface: the data line is driven by the
    keyboard to send data to the CPU; the CTS/clock line has a pull up resistor
    and can be driven low by both keyboard and CPU.  To send data to the CPU,
    the keyboard pulses the clock line low 9 times, and the Geneve samples all
    8 bits of data (plus one start bit) on each falling edge of the clock.
    When the key code buffer is full, the Geneve gate array asserts the kbdint*
    line (connected to 9901 int8_t*).  The Geneve gate array will hold the
    CTS/clock line low as long as the keyboard buffer is full or CRU bit @>F78
    is 0.  Writing a 0 to >F79 will clear the Geneve keyboard buffer, and
    writing a 1 will resume normal operation: you need to write a 0 to >F78
    before clearing >F79, or the keyboard will be enabled to send data the gate
    array when >F79 is is set to 0, and any such incoming data from the
    keyboard will be cleared as soon as it is buffered by the gate array.

****************************************************************************/

static const uint8_t MF1_CODE[0xe] =
{
	/* extended keys that are equivalent to non-extended keys */
	0x1c,   /* keypad enter */
	0x1d,   /* right control */
	0x38,   /* alt gr */
	// extra codes are 0x5b for Left Windows, 0x5c for Right Windows, 0x5d
	// for Menu, 0x5e for power, 0x5f for sleep, 0x63 for wake, but I doubt
	// any Geneve program would take advantage of these. */

	// extended key that is equivalent to a non-extended key
	// with shift off
	0x35,   /* pad slash */

	// extended keys that are equivalent to non-extended keys
	// with numlock off
	0x47,   /* home */
	0x48,   /* up */
	0x49,   /* page up */
	0x4b,   /* left */
	0x4d,   /* right */
	0x4f,   /* end */
	0x50,   /* down */
	0x51,   /* page down */
	0x52,   /* insert */
	0x53    /* delete */
};

geneve_keyboard_device::geneve_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, GENEVE_KEYBOARD, tag, owner, clock),
	m_interrupt(*this),
	m_keys(*this, "KEY%u", 0),
	m_key_reset(false), m_key_queue_length(0), m_key_queue_head(0), m_key_in_buffer(false), m_key_numlock_state(false), m_key_ctrl_state(0), m_key_alt_state(0),
	m_key_real_shift_state(0), m_key_fake_shift_state(false), m_key_fake_unshift_state(false), m_key_autorepeat_key(0), m_key_autorepeat_timer(0), m_keep_keybuf(false),
	m_keyboard_clock(false), m_timer(nullptr)
{
}

void geneve_keyboard_device::post_in_key_queue(int keycode)
{
	m_key_queue[(m_key_queue_head + m_key_queue_length) % KEYQUEUESIZE] = keycode;
	m_key_queue_length++;

	LOGMASKED(LOG_KEYBOARD, "Posting keycode %02x\n", keycode);
}

void geneve_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	poll();
}

void geneve_keyboard_device::poll()
{
	uint32_t keystate;
	uint32_t key_transitions;
	int i, j;
	int keycode;
	int pressed;
	LOGMASKED(LOG_KEYBOARD, "Poll keyboard\n");
	if (m_key_reset) return;

	/* Poll keyboard */
	for (i = 0; (i < 4) && (m_key_queue_length <= (KEYQUEUESIZE-MAXKEYMSGLENGTH)); i++)
	{
		keystate = m_keys[2*i]->read() | (m_keys[2*i + 1]->read() << 16);
		key_transitions = keystate ^ m_key_state_save[i];
		if (key_transitions)
		{
			for (j = 0; (j < 32) && (m_key_queue_length <= (KEYQUEUESIZE-MAXKEYMSGLENGTH)); j++)
			{
				if ((key_transitions >> j) & 1)
				{
					keycode = (i << 5) | j;
					pressed = ((keystate >> j) & 1);
					if (pressed)
						m_key_state_save[i] |= (1 << j);
					else
						m_key_state_save[i] &= ~ (1 << j);

					/* Update auto-repeat */
					if (pressed)
					{
						m_key_autorepeat_key = keycode;
						m_key_autorepeat_timer = KEYAUTOREPEATDELAY+1;
					}
					else /*if (keycode == m_key_autorepeat_key)*/
						m_key_autorepeat_key = 0;

					// Release Fake Shift/Unshift if another key is pressed
					// We do so if a key is released, though it is actually
					// required only if it is a modifier key
					/*if (pressed)*/
					//{
					if (m_key_fake_shift_state)
					{
						/* Fake shift release */
						post_in_key_queue(0xe0);
						post_in_key_queue(0xaa);
						m_key_fake_shift_state = false;
					}
					if (m_key_fake_unshift_state)
					{
						/* Fake shift press */
						post_in_key_queue(0xe0);
						post_in_key_queue(0x2a);
						m_key_fake_unshift_state = false;
					}
					//}

					/* update shift and numlock state */
					if ((keycode == 0x2a) || (keycode == 0x36))
						m_key_real_shift_state = m_key_real_shift_state + (pressed ? +1 : -1);
					if ((keycode == 0x1d) || (keycode == 0x61))
						m_key_ctrl_state = m_key_ctrl_state + (pressed ? +1 : -1);
					if ((keycode == 0x38) || (keycode == 0x62))
						m_key_alt_state = m_key_alt_state + (pressed ? +1 : -1);
					if ((keycode == 0x45) && pressed)
						m_key_numlock_state = !m_key_numlock_state;

					if ((keycode >= 0x60) && (keycode < 0x6e))
					{   /* simpler extended keys */
						/* these keys are emulated */

						if ((keycode >= 0x63) && pressed)
						{
							/* Handle shift state */
							if (keycode == 0x63)
							{   /* non-shifted key */
								if (m_key_real_shift_state!=0)
									/* Fake shift unpress */
									m_key_fake_unshift_state = true;
							}
							else /*if (keycode >= 0x64)*/
							{   /* non-numlock mode key */
								if (m_key_numlock_state & (m_key_real_shift_state==0))
									/* Fake shift press if numlock is active */
									m_key_fake_shift_state = true;
								else if ((!m_key_numlock_state) & (m_key_real_shift_state!=0))
									/* Fake shift unpress if shift is down */
									m_key_fake_unshift_state = true;
							}

							if (m_key_fake_shift_state)
							{
								post_in_key_queue(0xe0);
								post_in_key_queue(0x2a);
							}

							if (m_key_fake_unshift_state)
							{
								post_in_key_queue(0xe0);
								post_in_key_queue(0xaa);
							}
						}

						keycode = MF1_CODE[keycode-0x60];
						if (!pressed) keycode |= 0x80;
						post_in_key_queue(0xe0);
						post_in_key_queue(keycode);
					}
					else if (keycode == 0x6e)
					{   /* emulate Print Screen / System Request (F13) key */
						/* this is a bit complex, as Alt+PrtScr -> SysRq */
						/* Additionally, Ctrl+PrtScr involves no fake shift press */
						if (m_key_alt_state!=0)
						{
							/* SysRq */
							keycode = 0x54;
							if (!pressed) keycode |= 0x80;
							post_in_key_queue(keycode);
						}
						else
						{
							/* Handle shift state */
							if (pressed && (m_key_real_shift_state==0) && (m_key_ctrl_state==0))
							{   /* Fake shift press */
								post_in_key_queue(0xe0);
								post_in_key_queue(0x2a);
								m_key_fake_shift_state = true;
							}

							keycode = 0x37;
							if (!pressed) keycode |= 0x80;
							post_in_key_queue(0xe0);
							post_in_key_queue(keycode);
						}
					}
					else if (keycode == 0x6f)
					{   // emulate pause (F15) key
						// this is a bit complex, as Pause -> Ctrl+NumLock and
						// Ctrl+Pause -> Ctrl+ScrLock.  Furthermore, there is no
						// repeat or release.
						if (pressed)
						{
							if (m_key_ctrl_state!=0)
							{
								post_in_key_queue(0xe0);
								post_in_key_queue(0x46);
								post_in_key_queue(0xe0);
								post_in_key_queue(0xc6);
							}
							else
							{
								post_in_key_queue(0xe1);
								post_in_key_queue(0x1d);
								post_in_key_queue(0x45);
								post_in_key_queue(0xe1);
								post_in_key_queue(0x9d);
								post_in_key_queue(0xc5);
							}
						}
					}
					else
					{
						if (!pressed) keycode |= 0x80;
						post_in_key_queue(keycode);
					}
					signal_when_key_available();
				}
			}
		}
	}

	/* Handle auto-repeat */
	if ((m_key_queue_length <= (KEYQUEUESIZE-MAXKEYMSGLENGTH)) && (m_key_autorepeat_key!=0) && (--m_key_autorepeat_timer == 0))
	{
		if ((m_key_autorepeat_key >= 0x60) && (m_key_autorepeat_key < 0x6e))
		{
			post_in_key_queue(0xe0);
			post_in_key_queue(MF1_CODE[m_key_autorepeat_key-0x60]);
		}
		else if (m_key_autorepeat_key == 0x6e)
		{
			if (m_key_alt_state!=0)
				post_in_key_queue(0x54);
			else
			{
				post_in_key_queue(0xe0);
				post_in_key_queue(0x37);
			}
		}
		else if (m_key_autorepeat_key == 0x6f)
			;
		else
		{
			post_in_key_queue(m_key_autorepeat_key);
		}
		signal_when_key_available();
		m_key_autorepeat_timer = KEYAUTOREPEATRATE;
	}
}

uint8_t geneve_keyboard_device::get_recent_key()
{
	if (m_key_in_buffer) return m_key_queue[m_key_queue_head];
	else return 0;
}

void geneve_keyboard_device::signal_when_key_available()
{
	// if keyboard reset is not asserted, and key clock is enabled, and key
	// buffer clear is disabled, and key queue is not empty. */
	if ((!m_key_reset) && (m_keyboard_clock) && (m_keep_keybuf) && (m_key_queue_length != 0))
	{
		LOGMASKED(LOG_KEYBOARD, "Key available\n");
		m_interrupt(ASSERT_LINE);
		m_key_in_buffer = true;
	}
}

WRITE_LINE_MEMBER( geneve_keyboard_device::clock_control )
{
	bool rising_edge = (!m_keyboard_clock && (state==ASSERT_LINE));
	m_keyboard_clock = (state==ASSERT_LINE);
	LOGMASKED(LOG_KEYBOARD, "Keyboard clock_control state=%d\n", m_keyboard_clock);
	if (rising_edge)
		signal_when_key_available();
}

WRITE_LINE_MEMBER( geneve_keyboard_device::send_scancodes )
{
	bool rising_edge = (!m_keep_keybuf && (state==ASSERT_LINE));
	bool falling_edge = (m_keep_keybuf && (state==CLEAR_LINE));
	m_keep_keybuf = (state==ASSERT_LINE);

	if (rising_edge) signal_when_key_available();
	else
	{
		if (falling_edge)
		{
			if (m_key_queue_length != 0)
			{
				m_key_queue_head = (m_key_queue_head + 1) % KEYQUEUESIZE;
				m_key_queue_length--;
			}
			/* clear keyboard interrupt */
			m_interrupt(CLEAR_LINE);
			m_key_in_buffer = false;
		}
	}
}

WRITE_LINE_MEMBER( geneve_keyboard_device::reset_line )
{
	m_key_reset = !(state==ASSERT_LINE);

	if (m_key_reset)
	{
		/* reset -> clear keyboard key queue, but not geneve key buffer */
		m_key_queue_length = (m_key_in_buffer)? 1 : 0;
		m_key_queue_head = 0;
		memset(m_key_state_save, 0, sizeof(m_key_state_save));
		m_key_numlock_state = false;
		m_key_ctrl_state = 0;
		m_key_alt_state = 0;
		m_key_real_shift_state = 0;
		m_key_fake_shift_state = false;
		m_key_fake_unshift_state = false;
		m_key_autorepeat_key = 0;
	}
}

void geneve_keyboard_device::device_start()
{
	m_timer = timer_alloc(0);
	m_interrupt.resolve();

	// State registration
	save_item(NAME(m_key_reset));
	save_item(NAME(m_key_queue_length));
	save_item(NAME(m_key_queue_head));
	save_item(NAME(m_key_in_buffer));
	save_item(NAME(m_key_numlock_state));
	save_item(NAME(m_key_ctrl_state));
	save_item(NAME(m_key_alt_state));
	save_item(NAME(m_key_real_shift_state));
	save_item(NAME(m_key_fake_shift_state));
	save_item(NAME(m_key_fake_unshift_state));
	save_item(NAME(m_key_autorepeat_key));
	save_item(NAME(m_key_autorepeat_timer));
	save_item(NAME(m_keep_keybuf));
	save_item(NAME(m_keyboard_clock));
	save_pointer(NAME(m_key_queue),KEYQUEUESIZE);
	save_pointer(NAME(m_key_state_save),4);
}

void geneve_keyboard_device::device_reset()
{
	m_key_in_buffer = false;
	reset_line(CLEAR_LINE);
	m_key_queue_length = 0;
	m_key_reset = true;
	m_keyboard_clock = false;
	m_keep_keybuf = false;
	m_timer->adjust(attotime::from_usec(1), 0, attotime::from_hz(120));
}

INPUT_PORTS_START( genkeys )
	PORT_START("KEY0")  /* IN3 */
	PORT_BIT ( 0x0001, 0x0000, IPT_UNUSED )     /* unused scancode 0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) /* Esc                       01  81 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1 !") PORT_CODE(KEYCODE_1) /* 1                           02  82 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2 @") PORT_CODE(KEYCODE_2) /* 2                           03  83 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3 #") PORT_CODE(KEYCODE_3) /* 3                           04  84 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4 $") PORT_CODE(KEYCODE_4) /* 4                           05  85 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5 %") PORT_CODE(KEYCODE_5) /* 5                           06  86 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6 ^") PORT_CODE(KEYCODE_6) /* 6                           07  87 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7 &") PORT_CODE(KEYCODE_7) /* 7                           08  88 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8 *") PORT_CODE(KEYCODE_8) /* 8                           09  89 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 (") PORT_CODE(KEYCODE_9) /* 9                           0A  8A */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 )") PORT_CODE(KEYCODE_0) /* 0                           0B  8B */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("- _") PORT_CODE(KEYCODE_MINUS) /* -                           0C  8C */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("= +") PORT_CODE(KEYCODE_EQUALS) /* =                          0D  8D */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) /* Backspace                 0E  8E */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) /* Tab                       0F  8F */

	PORT_START("KEY1")  /* IN4 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) /* Q                         10  90 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) /* W                         11  91 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) /* E                         12  92 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) /* R                         13  93 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) /* T                         14  94 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) /* Y                         15  95 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) /* U                         16  96 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) /* I                         17  97 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) /* O                         18  98 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) /* P                         19  99 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("[ {") PORT_CODE(KEYCODE_OPENBRACE) /* [                           1A  9A */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("] }") PORT_CODE(KEYCODE_CLOSEBRACE) /* ]                          1B  9B */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) /* Enter                     1C  9C */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L-Ctrl") PORT_CODE(KEYCODE_LCONTROL) /* Left Ctrl                 1D  9D */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) /* A                         1E  9E */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) /* S                         1F  9F */

	PORT_START("KEY2")  /* IN5 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) /* D                         20  A0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) /* F                         21  A1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) /* G                         22  A2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) /* H                         23  A3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) /* J                         24  A4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) /* K                         25  A5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) /* L                         26  A6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("; :") PORT_CODE(KEYCODE_COLON) /* ;                           27  A7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("' \"") PORT_CODE(KEYCODE_QUOTE) /* '                          28  A8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("` ~") PORT_CODE(KEYCODE_TILDE) /* `                           29  A9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("L-Shift") PORT_CODE(KEYCODE_LSHIFT) /* Left Shift                 2A  AA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("\\ |") PORT_CODE(KEYCODE_BACKSLASH) /* \                          2B  AB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) /* Z                         2C  AC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) /* X                         2D  AD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) /* C                         2E  AE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) /* V                         2F  AF */

	PORT_START("KEY3")  /* IN6 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) /* B                         30  B0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) /* N                         31  B1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) /* M                         32  B2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(", <") PORT_CODE(KEYCODE_COMMA) /* ,                           33  B3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(". >") PORT_CODE(KEYCODE_STOP) /* .                            34  B4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/ ?") PORT_CODE(KEYCODE_SLASH) /* /                           35  B5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("R-Shift") PORT_CODE(KEYCODE_RSHIFT) /* Right Shift                36  B6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP * (PrtScr)") PORT_CODE(KEYCODE_ASTERISK    ) /* Keypad *  (PrtSc)          37  B7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt") PORT_CODE(KEYCODE_LALT) /* Left Alt                 38  B8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) /* Space                     39  B9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) /* Caps Lock                   3A  BA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1) /* F1                          3B  BB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2) /* F2                          3C  BC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3) /* F3                          3D  BD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4) /* F4                          3E  BE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5) /* F5                          3F  BF */

	PORT_START("KEY4")  /* IN7 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6) /* F6                          40  C0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7) /* F7                          41  C1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8) /* F8                          42  C2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F9") PORT_CODE(KEYCODE_F9) /* F9                          43  C3 */
	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F10") PORT_CODE(KEYCODE_F10) /* F10                       44  C4 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("NumLock") PORT_CODE(KEYCODE_NUMLOCK) /* Num Lock                  45  C5 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("ScrLock (F14)") PORT_CODE(KEYCODE_SCRLOCK) /* Scroll Lock             46  C6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 7 (Home)") PORT_CODE(KEYCODE_7_PAD     ) /* Keypad 7  (Home)           47  C7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 8 (Up)") PORT_CODE(KEYCODE_8_PAD       ) /* Keypad 8  (Up arrow)       48  C8 */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 9 (PgUp)") PORT_CODE(KEYCODE_9_PAD     ) /* Keypad 9  (PgUp)           49  C9 */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP -") PORT_CODE(KEYCODE_MINUS_PAD) /* Keypad -                   4A  CA */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 4 (Left)") PORT_CODE(KEYCODE_4_PAD     ) /* Keypad 4  (Left arrow)     4B  CB */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 5") PORT_CODE(KEYCODE_5_PAD) /* Keypad 5                   4C  CC */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 6 (Right)") PORT_CODE(KEYCODE_6_PAD        ) /* Keypad 6  (Right arrow)    4D  CD */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP +") PORT_CODE(KEYCODE_PLUS_PAD) /* Keypad +                    4E  CE */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 1 (End)") PORT_CODE(KEYCODE_1_PAD      ) /* Keypad 1  (End)            4F  CF */

	PORT_START("KEY5")  /* IN8 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 2 (Down)") PORT_CODE(KEYCODE_2_PAD     ) /* Keypad 2  (Down arrow)     50  D0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 3 (PgDn)") PORT_CODE(KEYCODE_3_PAD     ) /* Keypad 3  (PgDn)           51  D1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP 0 (Ins)") PORT_CODE(KEYCODE_0_PAD      ) /* Keypad 0  (Ins)            52  D2 */
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("KP . (Del)") PORT_CODE(KEYCODE_DEL_PAD        ) /* Keypad .  (Del)            53  D3 */
	PORT_BIT ( 0x0030, 0x0000, IPT_UNUSED )
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(84/102)\\") PORT_CODE(KEYCODE_BACKSLASH2) /* Backslash 2             56  D6 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)F11") PORT_CODE(KEYCODE_F11) /* F11                      57  D7 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)F12") PORT_CODE(KEYCODE_F12) /* F12                      58  D8 */
	PORT_BIT ( 0xfe00, 0x0000, IPT_UNUSED )

	PORT_START("KEY6")  /* IN9 */
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)KP Enter") PORT_CODE(KEYCODE_ENTER_PAD) /* PAD Enter                 60  e0 */
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)R-Control") PORT_CODE(KEYCODE_RCONTROL) /* Right Control             61  e1 */
	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)ALTGR") PORT_CODE(KEYCODE_RALT) /* ALTGR                     64  e4 */

	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)KP /") PORT_CODE(KEYCODE_SLASH_PAD) /* PAD Slash                 62  e2 */

	PORT_BIT(0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Home") PORT_CODE(KEYCODE_HOME) /* Home                       66  e6 */
	PORT_BIT(0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Cursor Up") PORT_CODE(KEYCODE_UP) /* Up                          67  e7 */
	PORT_BIT(0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Page Up") PORT_CODE(KEYCODE_PGUP) /* Page Up                 68  e8 */
	PORT_BIT(0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Cursor Left") PORT_CODE(KEYCODE_LEFT) /* Left                        69  e9 */
	PORT_BIT(0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Cursor Right") PORT_CODE(KEYCODE_RIGHT) /* Right                     6a  ea */
	PORT_BIT(0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)End") PORT_CODE(KEYCODE_END) /* End                      6b  eb */
	PORT_BIT(0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Cursor Down") PORT_CODE(KEYCODE_DOWN) /* Down                        6c  ec */
	PORT_BIT(0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Page Down") PORT_CODE(KEYCODE_PGDN) /* Page Down                 6d  ed */
	PORT_BIT(0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Insert") PORT_CODE(KEYCODE_INSERT) /* Insert                     6e  ee */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Delete") PORT_CODE(KEYCODE_DEL) /* Delete                        6f  ef */

	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)PrtScr (F13)") PORT_CODE(KEYCODE_PRTSCR) /* Print Screen             63  e3 */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("(101)Pause (F15)") PORT_CODE(KEYCODE_PAUSE) /* Pause                      65  e5 */

	PORT_START("KEY7")  /* IN10 */
	PORT_BIT ( 0xffff, 0x0000, IPT_UNUSED )
#if 0
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Print Screen") PORT_CODE(KEYCODE_PRTSCR) /* Print Screen alternate        77  f7 */
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Left Win") /* Left Win                    7d  fd */
	PORT_BIT(0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Right Win") /* Right Win                  7e  fe */
	PORT_BIT(0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Menu") /* Menu                        7f  ff */
#endif
INPUT_PORTS_END

ioport_constructor geneve_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( genkeys );
}

} } } // end namespace bus::ti99::internal


// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/***************************************************************************
    Geneve 9640 mapper and more components

    This file contains 3 classes:
    - mapper: main function of the Gate Array on the Geneve board. Maps logical
        memory accesses to a wider address space using map registers.
    - keyboard: an implementation of a XT-style keyboard. This should be dropped
        and replaced by a proper XT keyboard implementation.
    - mouse: an implementation of an Atari-style mouse connected to the v9938.

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
    - TI mode: Selects between the on-board memory, which is obviously required
      for the GPL interpreter, and the external Memex memory. This switch
      triggers a reset when changed.


    ===================
    Mapping
    ===================

    Logical address space: 64 KiB

    Geneve mode:

    Video:    F100, F102, F104, F106 (mirror: +8)
    Mapper:   F110 - F117
    Keyboard: F118
    Clock:    F130 - F13F
    Sound:    F120

    TI mode:

    Video:    8800, 8802, 8804, 8806
    Mapper:   8000 - 8007
    Keyboard: 8008 - 800F
    Clock:    8010 - 801F
    Speech:   9000 / 9400
    Grom:     9800 / 9802

    Physical address space: 2 MiB

    Start    End      Banks
    000000 - 07FFFF   00-3F   512 KiB DRAM on-board
    080000 - 0FFFFF   40-7F   512 KiB on-board expansion (never used)
    100000 - 16FFFF   80-B7   448 KiB P-Box space (special cards, like MEMEX)
    170000 - 17FFFF   B8-BF    64 KiB P-Box space (current cards)
    180000 - 1DFFFF   C0-EF   384 KiB SRAM space on-board; stock Geneve comes with 32 KiB
    1E0000 - 1FFFFF   F0-FF   128 KiB EPROM space; 16 KiB actually used, 8 mirrors


    GenMod modification:

    TI mode
    000000 - 07FFFF   00-3F   512 KiB DRAM on-board
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
    - accesses to sound: ~25 WS
    - accesses to SRAM: 0 WS

    Additional waitstates are created when one of the CRU bits is set. In that
    case, all delays are extended to 2 WS (including SRAM).

    Sound waitstates are somewhat unpredictable. It seems as if they depend
    on the clock of the sound chip; the theory is that the READY line is
    pulled down until the next clock pulse, which may take some value between
    18 CPU cycles and 30 CPU cycles.

    The gate array is able to create wait states for video accesses. However,
    these wait states are effective after the video access has been completed.
    Wait states are not effective when the execution is running in on-chip
    RAM. Additional wait states are requested by m_video_waitstates = true.
    Without additional wait states, the video access takes the usual 1 or 2 WS.

    Waitstate behavior (Nov 2013)
       Almost perfect. Only video read access from code in DRAM is too fast by one WS

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

#include "genboard.h"

#define TRACE_READ 0
#define TRACE_WRITE 0
#define TRACE_DETAIL 0
#define TRACE_KEYBOARD 0
#define TRACE_CLOCK 0
#define TRACE_LINES 0
#define TRACE_SETTING 1
#define TRACE_PFM 0

geneve_mapper_device::geneve_mapper_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, GENEVE_MAPPER, "Geneve Gate Array", tag, owner, clock, "geneve_mapper", __FILE__), m_gromwaddr_LSB(false), m_gromraddr_LSB(false), m_grom_address(0), m_video_waitstates(false), m_extra_waitstates(false), m_ready_asserted(false), m_read_mode(false), m_debug_no_ws(false), m_geneve_mode(false), m_direct_mode(false), m_cartridge_size(0), m_cartridge_secondpage(false), m_cartridge6_writable(false), m_cartridge7_writable(false), m_turbo(false), m_genmod(false), m_timode(false), m_pfm_mode(0), m_pfm_bank(0), m_pfm_output_enable(false), m_sram_mask(0), m_sram_val(0),
	m_ready(*this), m_waitcount(0), m_ext_waitcount(0), m_clock(nullptr), m_cpu(nullptr), m_pfm512(nullptr), m_pfm512a(nullptr), m_keyboard(nullptr), m_video(nullptr), m_peribox(nullptr), m_sound(nullptr), m_sram(nullptr), m_dram(nullptr)
{
	m_eprom = NULL;
}

INPUT_CHANGED_MEMBER( geneve_mapper_device::settings_changed )
{
	int number = (int)((UINT64)param&0x03);
	int value = newval;

	switch (number)
	{
	case 1:
		// Turbo switch. May be changed at any time.
		if (TRACE_SETTING) logerror("%s: Setting turbo flag to %d\n", tag(), value);
		m_turbo = (value!=0);
		break;
	case 2:
		// TIMode switch. Causes reset when changed.
		if (TRACE_SETTING) logerror("%s: Setting timode flag to %d\n", tag(), value);
		m_timode = (value!=0);
		machine().schedule_hard_reset();
		break;
	case 3:
		// Used when switching the boot ROMs during runtime, especially the PFM
		set_boot_rom(value);
		break;
	default:
		logerror("%s: Unknown setting %d ignored\n", tag(), number);
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
	UINT8 reply;
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
		int page = 0x38;
		reply = m_dram[(page<<13) + m_grom_address];
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
			m_grom_address = (m_grom_address & 0x00ff) | ((UINT16)data<<8);
			m_gromwaddr_LSB = true;
		}
	}
	else
	{   // write GPL data
		// The Geneve GROM simulator allows for GROM writing (verified with a real system)
		int page = 0x38;
		m_dram[(page<<13) + m_grom_address] = data;

		m_grom_address = (m_grom_address + 1) & 0xffff;
		m_gromraddr_LSB = m_gromwaddr_LSB = false;
	}
}

void geneve_mapper_device::set_wait(int min)
{
	if (m_debug_no_ws) return;
	if (m_extra_waitstates && min < 2) min = 2;

	// if we still have video wait states, do not set this counter
	// (or it will assert READY when expiring)
	if (m_ext_waitcount > min) return;

	// need one more pass so that READY will be asserted again
	m_waitcount = min + 1;
	if (m_waitcount > 1)
	{
		if (TRACE_LINES) logerror("%s: Pulling down READY line for %d cycles\n", tag(), min);
		m_ready(CLEAR_LINE);
		m_ready_asserted = false;
	}
}

void geneve_mapper_device::set_ext_wait(int min)
{
	if (m_debug_no_ws) return;
	m_ext_waitcount = min;
}

void geneve_mapper_device::set_boot_rom(int selection)
{
	switch (selection)
	{
	case GENEVE_098:
		logerror("%s: Using 0.98 boot eprom\n", tag());
		m_eprom = machine().root_device().memregion("maincpu")->base() + 0x4000;
		m_pfm_mode = 0;
		break;
	case GENEVE_100:
		logerror("%s: Using 1.00 boot eprom\n", tag());
		m_eprom = machine().root_device().memregion("maincpu")->base();
		m_pfm_mode = 0;
		break;
	case GENEVE_PFM512:
		logerror("%s: Using PFM512 (AT29C040)\n", tag());
		m_pfm_mode = 1;
		break;
	case GENEVE_PFM512A:
		logerror("%s: Using PFM512A (AT29C040A)\n", tag());
		m_pfm_mode = 2;
		break;
	default:
		logerror("%s: Unknown boot ROM selection\n", tag());
	}
}

void geneve_mapper_device::set_geneve_mode(bool geneve)
{
	if (TRACE_SETTING) logerror("%s: Setting Geneve mode = %d\n", tag(), geneve);
	m_geneve_mode = geneve;
}

void geneve_mapper_device::set_direct_mode(bool direct)
{
	if (TRACE_SETTING) logerror("%s: Setting direct mode = %d\n", tag(), direct);
	m_direct_mode = direct;
}

void geneve_mapper_device::set_cartridge_size(int size)
{
	if (TRACE_SETTING) logerror("%s: Setting cartridge size to %d\n", tag(), size);
	m_cartridge_size = size;
}

void geneve_mapper_device::set_cartridge_writable(int base, bool write)
{
	if (TRACE_SETTING) logerror("%s: Cartridge %04x space writable = %d\n", tag(), base, write);
	if (base==0x6000) m_cartridge6_writable = write;
	else m_cartridge7_writable = write;
}

void geneve_mapper_device::set_video_waitstates(bool wait)
{
	if (TRACE_SETTING) logerror("%s: Setting video waitstates = %d\n", tag(), wait);
	m_video_waitstates = wait;
}

void geneve_mapper_device::set_extra_waitstates(bool wait)
{
	if (TRACE_SETTING) logerror("%s: Setting extra waitstates = %d\n", tag(), wait);
	m_extra_waitstates = wait;
}


/************************************************************************
    Called by the address map
************************************************************************/
/*
    Constants for mapper decoding. Naming scheme:
    M=mapper
    L=Logical space; P=Physical space
    G=Geneve mode;  T=TI mode; GM=GenMod
*/
enum
{
	MLGVIDEO=1,
	MLGMAPPER,
	MLGKEY,
	MLGCLOCK,
	MLGSOUND,
	MLTMAPPER,
	MLTKEY,
	MLTCLOCK,
	MLTVIDEO,
	MLTSPEECH,
	MLTGROM,
	MLTSOUND,
	MPGDRAM,
	MPGEXP,
	MPGEPROM,
	MPGSRAM,
	MPGBOX,
	MPGMDRAM,
	MPGMEPROM,
	MPGMBOX
};

/*
    Read a byte via the data bus. The decoding has already been done in the
    SETOFFSET method, and we re-use the values stored there to quickly
    access the appropriate component.
*/
READ8_MEMBER( geneve_mapper_device::readm )
{
	UINT8 value = 0;

	decdata *dec;
	decdata debug;

	// For the debugger, do the decoding here with no wait states
	if (space.debugger_access())
	{
		if (m_cpu->is_onchip(offset)) return m_cpu->debug_read_onchip_memory(offset&0xff);
		dec = &debug;
		m_debug_no_ws = true;
		decode(space, offset, true, dec);
	}
	else
	{
		// Use the values found in the setaddress phase
		dec = &m_decoded;
		m_debug_no_ws = false;
	}

	switch (dec->function)
	{
	case MLGVIDEO:
		m_video->readz(space, dec->offset, &value, 0xff);
		if (TRACE_READ) logerror("%s: Read video %04x -> %02x\n", tag(), dec->offset, value);
		// Video wait states are created *after* the access
		// Accordingly, they have no effect when execution is in onchip RAM
		if (m_video_waitstates) set_ext_wait(15);
		break;

	case MLGMAPPER:
		// mapper
		value = m_map[dec->offset];
		if (TRACE_READ) logerror("%s: read mapper %04x -> %02x\n", tag(), dec->offset, value);
		break;

	case MLGKEY:
		// key
		if (!space.debugger_access()) value = m_keyboard->get_recent_key();
		if (TRACE_READ) logerror("%s: Read keyboard -> %02x\n", tag(), value);
		break;

	case MLGCLOCK:
		// clock
		// tests on the real machine showed that
		// upper nibble is 0xf (probably because of the location at 0xf130?)
		value = m_clock->read(space, dec->offset) | 0xf0;
		if (TRACE_READ) logerror("%s: Read clock %04x -> %02x\n", tag(), dec->offset, value);
		break;

	case MLTMAPPER:
		// mapper
		value = m_map[dec->offset];
		if (TRACE_READ) logerror("%s: Read mapper %04x -> %02x\n", tag(), dec->offset, value);
		break;

	case MLTKEY:
		// key
		if (!space.debugger_access()) value = m_keyboard->get_recent_key();
		if (TRACE_READ) logerror("%s: Read keyboard -> %02x\n", tag(), value);
		break;

	case MLTCLOCK:
		// clock
		// upper nibble is 1, only last byte gets a 2
		// probably because of the location at 8010...8020?
		// (TI mode used swapped byte order)
		// unless we use a workspace at >F000, in which case we get 8x values
		// Obscure, needs more investigation. We might as well ignore this,
		// as the high nibble is obviously undefined and takes some past
		// value floating around.
		value = m_clock->read(space, dec->offset);
		value |= (dec->offset==0x000f)? 0x20 : 0x10;
		if (TRACE_READ) logerror("%s: Read clock %04x -> %02x\n", tag(), dec->offset, value);
		break;

	case MLTVIDEO:
		// video
		// ++++ ++-- ---- ---+
		// 1000 1000 0000 00x0
		m_video->readz(space, dec->offset, &value, 0xff);
		if (TRACE_READ) logerror("%s: Read video %04x -> %02x\n", tag(), dec->offset, value);
		// See above
		if (m_video_waitstates) set_ext_wait(15);
		break;

	case MLTSPEECH:
		// speech
		// ++++ ++-- ---- ---+
		// 1001 0000 0000 0000
		// We need to add the address prefix bits
		m_peribox->readz(space, dec->offset, &value, 0xff);
		if (TRACE_READ) logerror("%s: Read speech -> %02x\n", tag(), value);
		break;

	case MLTGROM:
		// grom simulation
		// ++++ ++-- ---- ---+
		// 1001 1000 0000 00x0
		if (!space.debugger_access()) value = read_grom(space, dec->offset, 0xff);
		if (TRACE_READ) logerror("%s: Read GROM %04x -> %02x\n", tag(), dec->offset, value);
		break;

	case MLGSOUND:
	case MLTSOUND:
		value = 0;
		break;


	case MPGDRAM:
		// DRAM.
		value = m_dram[dec->physaddr];
//          LOG("dram read physaddr = %06x logaddr = %04x value = %02x\n", dec->physaddr, dec->offset, value);
		if (TRACE_READ) logerror("%s: Read DRAM %04x (%06x) -> %02x\n", tag(), dec->offset, dec->physaddr, value);
		break;

	case MPGEXP:
		// On-board memory expansion for standard Geneve (never used)
		if (TRACE_READ) logerror("%s: Read unmapped area %06x\n", tag(), dec->physaddr);
		value = 0;
		break;

	case MPGEPROM:
		// 1 111. ..xx xxxx xxxx xxxx on-board eprom (16K)
		// mirrored for f0, f2, f4, ...; f1, f3, f5, ...
		if (m_pfm_mode == 0)
		{
			value = m_eprom[dec->physaddr & 0x003fff];
			if (TRACE_READ) logerror("%s: Read EPROM %04x (%06x) -> %02x\n", tag(), dec->offset, dec->physaddr, value);
		}
		else value = read_from_pfm(space, dec->physaddr, 0xff);

		break;

	case MPGSRAM:
		if ((dec->physaddr & m_sram_mask)==m_sram_val)
		{
			value = m_sram[dec->physaddr & ~m_sram_mask];
		}
		else value = 0;
		// Return in any case
//          LOG("sram read physaddr = %06x logaddr = %04x value = %02x\n", dec->physaddr, dec->offset, value);
		if (TRACE_READ) logerror("%s: Read SRAM %04x (%06x) -> %02x\n", tag(), dec->offset, dec->physaddr, value);
		break;

	case MPGBOX:
		// Route everything else to the P-Box
		//   0x000000-0x07ffff for the stock Geneve (AMC,AMB,AMA,A0 ...,A15)
		//   0x000000-0x1fffff for the GenMod.(AME,AMD,AMC,AMB,AMA,A0 ...,A15)

		m_peribox->readz(space, dec->physaddr, &value, 0xff);
		if (TRACE_READ) logerror("%s: Read P-Box %04x (%06x) -> %02x\n", tag(), dec->offset, dec->physaddr, value);
		break;

	case MPGMDRAM:
		// DRAM. One wait state.
		value = m_dram[dec->physaddr];
		break;

	case MPGMEPROM:
		// 1 111. ..xx xxxx xxxx xxxx on-board eprom (16K)
		// mirrored for f0, f2, f4, ...; f1, f3, f5, ...
		if (m_pfm_mode == 0)
		{
			value = m_eprom[dec->physaddr & 0x003fff];
			if (TRACE_READ) logerror("%s: Read EPROM %04x (%06x) -> %02x\n", tag(), dec->offset, dec->physaddr, value);
		}
		else value = read_from_pfm(space, dec->physaddr, 0xff);
		break;

	case MPGMBOX:
		// Route everything else to the P-Box
		m_peribox->readz(space, dec->physaddr, &value, 0xff);
		break;
	}
	return value;
}

WRITE8_MEMBER( geneve_mapper_device::writem )
{
	decdata *dec;
	decdata debug;

	// For the debugger, do the decoding here with no wait states
	if (space.debugger_access())
	{
		dec = &debug;
		m_debug_no_ws = true;
		decode(space, offset, false, dec);
	}
	else
	{
		// Use the values found in the setaddress phase
		m_debug_no_ws = false;
		dec = &m_decoded;
	}

	switch (dec->function)
	{
	case MLGVIDEO:
		// video
		// ++++ ++++ ++++ ---+
		// 1111 0001 0000 .cc0
		m_video->write(space, dec->offset, data, 0xff);
		if (TRACE_WRITE) logerror("%s: Write video %04x <- %02x\n", tag(), offset, data);
		// See above
		if (m_video_waitstates) set_ext_wait(15);
		break;

	case MLGMAPPER:
		// mapper
		m_map[dec->offset] = data;
		if (TRACE_WRITE) logerror("%s: Write mapper %04x <- %02x\n", tag(), offset, data);
		break;

	case MLGCLOCK:
		// clock
		// ++++ ++++ ++++ ----
		m_clock->write(space, dec->offset, data);
		if (TRACE_WRITE) logerror("%s: Write clock %04x <- %02x\n", tag(), offset, data);
		break;

	case MLGSOUND:
		// sound
		// ++++ ++++ ++++ ---+
		m_sound->write(space, 0, data, 0xff);
		if (TRACE_WRITE) logerror("%s: Write sound <- %02x\n", tag(), data);
		break;

	case MLTMAPPER:
		// mapper
		m_map[dec->offset] = data;
		if (TRACE_WRITE) logerror("%s: Write mapper %04x <- %02x\n", tag(), offset, data);
		break;

	case MLTCLOCK:
		// clock
		m_clock->write(space, dec->offset, data);
		if (TRACE_WRITE) logerror("%s: Write clock %04x <- %02x\n", tag(), offset, data);
		break;

	case MLTVIDEO:
		// video
		// ++++ ++-- ---- ---+
		// 1000 1100 0000 00c0
		// Initialize waitstate timer
		m_video->write(space, dec->offset, data, 0xff);
		if (TRACE_WRITE) logerror("%s: Write video %04x <- %02x\n", tag(), offset, data);
		// See above
		if (m_video_waitstates) set_ext_wait(15);
		break;

	case MLTSPEECH:
		// speech
		// ++++ ++-- ---- ---+
		// 1001 0100 0000 0000
		// We need to add the address prefix bits
		m_peribox->write(space, dec->offset, data, 0xff);
		if (TRACE_WRITE) logerror("%s: Write speech <- %02x\n", tag(), data);
		break;

	case MLTGROM:
		// grom simulation
		// ++++ ++-- ---- ---+
		// 1001 1100 0000 00c0
		write_grom(space, dec->offset, data, 0xff);
		if (TRACE_WRITE) logerror("%s: Write GROM %04x <- %02x\n", tag(), offset, data);
		break;

	case MLTSOUND:
		// sound
		// ++++ ++-- ---- ---+
		// 1000 0100 0000 0000
		m_sound->write(space, 0, data, 0xff);
		if (TRACE_WRITE) logerror("%s: Write sound <- %02x\n", tag(), data);
		break;

	case MLTKEY:
	case MLGKEY:
		break;

	case MPGDRAM:
		// DRAM write. One wait state. (only for normal Geneve)
		m_dram[dec->physaddr] = data;
		if (TRACE_WRITE) logerror("%s: Write DRAM %04x (%06x) <- %02x\n", tag(), offset, dec->physaddr, data);
		break;

	case MPGEXP:
		// On-board memory expansion for standard Geneve (never used)
		if (TRACE_WRITE) logerror("%s: Write unmapped area %06x\n", tag(), dec->physaddr);
		break;

	case MPGEPROM:
		// 1 111. ..xx xxxx xxxx xxxx on-board eprom (16K)
		// mirrored for f0, f2, f4, ...; f1, f3, f5, ...
		// Ignore EPROM write (unless PFM)
		if (m_pfm_mode != 0) write_to_pfm(space, dec->physaddr, data, 0xff);
		else
			logerror("%s: Write EPROM %04x (%06x) <- %02x, ignored\n", tag(), offset, dec->physaddr, data);
		break;

	case MPGSRAM:
		if ((dec->physaddr & m_sram_mask)==m_sram_val)
		{
			m_sram[dec->physaddr & ~m_sram_mask] = data;
		}
		if (TRACE_WRITE) logerror("%s: Write SRAM %04x (%06x) <- %02x\n", tag(), offset, dec->physaddr, data);
		break;

	case MPGBOX:
		dec->physaddr = (dec->physaddr & 0x0007ffff);  // 19 bit address
		if (TRACE_WRITE) logerror("%s: Write P-Box %04x (%06x) <- %02x\n", tag(), offset, dec->physaddr, data);
		m_peribox->write(space, dec->physaddr, data, 0xff);
		break;

	case MPGMDRAM:
		// DRAM. One wait state.
		m_dram[dec->physaddr] = data;
		break;

	case MPGMEPROM:
		// 1 111. ..xx xxxx xxxx xxxx on-board eprom (16K)
		// mirrored for f0, f2, f4, ...; f1, f3, f5, ...
		// Ignore EPROM write
		if (m_pfm_mode != 0) write_to_pfm(space, dec->physaddr, data, 0xff);
		else
			logerror("%s: Write EPROM %04x (%06x) <- %02x, ignored\n", tag(), offset, dec->physaddr, data);
		break;

	case MPGMBOX:
		// Route everything else to the P-Box
		m_peribox->write(space, dec->physaddr, data, 0xff);
		break;
	}
}

void geneve_mapper_device::decode(address_space& space, offs_t offset, bool read_mode, geneve_mapper_device::decdata* dec)
{
	dec->function = 0;
	dec->offset = offset;
	dec->physaddr = 0;

	int page;

	if (read_mode)    // got this from DBIN
	{
		// Logical addresses
		if (m_geneve_mode)
		{
			// TODO: shortcut offset & 0xffc0 = 0xf100
			if ((offset & 0xfff5)==0xf100)
			{
				// video
				// ++++ ++++ ++++ -+-+
				// 1111 0001 0000 0000
				// 1111 0001 0000 0010
				// 1111 0001 0000 1000
				// 1111 0001 0000 1010

				dec->function = MLGVIDEO;
				set_wait(1);
				return;
			}
			if ((offset & 0xfff8)==0xf110)
			{
				// mapper
				dec->function = MLGMAPPER;
				dec->offset = dec->offset & 0x0007;
				set_wait(1);
				return;
			}
			if ((offset & 0xfff8) == 0xf118)
			{
				// key
				dec->function = MLGKEY;
				set_wait(1);
				return;
			}
			if ((offset & 0xfff0)==0xf130)
			{
				// clock
				// tests on the real machine showed that
				// upper nibble is 0xf (probably because of the location at 0xf130?)
				dec->function = MLGCLOCK;
				dec->offset = dec->offset & 0x000f;
				set_wait(1);
				return;
			}
		}
		else
		{
			if ((offset & 0xfff8)==0x8000)
			{
				// mapper
				dec->function = MLTMAPPER;
				dec->offset = dec->offset & 0x0007;
				set_wait(1);
				return;
			}
			if ((offset & 0xfff8)== 0x8008)
			{
				// key
				dec->function = MLTKEY;
				set_wait(1);
				return;
			}
			if ((offset & 0xfff0)==0x8010)
			{
				// clock
				dec->function = MLTCLOCK;
				dec->offset = dec->offset & 0x000f;
				set_wait(1);
				return;
			}
			if ((offset & 0xfc01)==0x8800)
			{
				// video
				// ++++ ++-- ---- ---+
				// 1000 1000 0000 00x0
				// 1 WS is always added; any pending video waitstates are canceled
				dec->function = MLTVIDEO;
				set_wait(1);
				return;
			}
			if ((offset & 0xfc01)==0x9000)
			{
				// speech
				// ++++ ++-- ---- ---+
				// 1001 0000 0000 0000
				// We need to add the address prefix bits
				dec->function = MLTSPEECH;
				dec->offset = offset | ((m_genmod)? 0x170000 : 0x070000);
				m_peribox->setaddress_dbin(space, dec->offset, read_mode);
				set_wait(1);
				return;
			}
			if ((offset & 0xfc01)==0x9800)
			{
				// grom simulation
				// ++++ ++-- ---- ---+
				// 1001 1000 0000 00x0
				dec->function = MLTGROM;
				set_wait(1);
				return;
			}
		}
		// still here? Then go via mapping.
		page = (offset & 0xe000) >> 13;

		// Determine physical address
		if (m_direct_mode)
		{
			dec->physaddr = 0x1f0000; // points to boot eprom (page F8)
		}
		else
		{
			if (!m_geneve_mode && page==3)
			{
				if (m_cartridge_size==0x4000 && m_cartridge_secondpage) dec->physaddr = 0x06e000;
				else dec->physaddr = 0x06c000;
			}
			else
			{
				dec->physaddr = (m_map[page] << 13);
			}
		}
		dec->physaddr |= (offset & 0x1fff);

		if (!m_genmod)      // Standard Geneve
		{
			if ((dec->physaddr & 0x180000)==0x000000)
			{
				// DRAM.
				dec->physaddr = dec->physaddr & 0x07ffff;
				dec->function = MPGDRAM;
				set_wait(1);
				return;
			}

			if ((dec->physaddr & 0x180000)==0x080000)
			{
				// On-board memory expansion for standard Geneve (never used)
				dec->function = MPGEXP;
				set_wait(1);
				return;
			}

			if ((dec->physaddr & 0x1e0000)==0x1e0000)
			{
				// 1 111. ..xx xxxx xxxx xxxx on-board eprom (16K)
				// mirrored for f0, f2, f4, ...; f1, f3, f5, ... unless using PFM
				dec->function = MPGEPROM;
				set_wait(0);
				return;
			}

			if ((dec->physaddr & 0x180000)==0x180000)
			{
				dec->function = MPGSRAM;
				set_wait(0);
				return;
			}

			// Route everything else to the P-Box
			//   0x000000-0x07ffff for the stock Geneve (AMC,AMB,AMA,A0 ...,A15)
			//   0x000000-0x1fffff for the GenMod.(AME,AMD,AMC,AMB,AMA,A0 ...,A15)
			// Add a wait state
			set_wait(1);
			dec->function = MPGBOX;

			dec->physaddr = (dec->physaddr & 0x0007ffff);  // 19 bit address (with AMA..AMC)
			m_peribox->setaddress_dbin(space, dec->physaddr, read_mode);
			return;
		}
		else
		{
			// GenMod mode
			if ((m_timode) && ((dec->physaddr & 0x180000)==0x000000))
			{
				// DRAM. One wait state.
				dec->function = MPGMDRAM;
				dec->physaddr = dec->physaddr & 0x07ffff;
				if (!m_turbo) set_wait(1);
				return;
			}

			if ((dec->physaddr & 0x1e0000)==0x1e0000)
			{
				// 1 111. ..xx xxxx xxxx xxxx on-board eprom (16K)
				// mirrored for f0, f2, f4, ...; f1, f3, f5, ... unless using PFM
				dec->function = MPGMEPROM;
				set_wait(0);
				return;
			}

			// Route everything else to the P-Box
			dec->physaddr = (dec->physaddr & 0x001fffff);  // 21 bit address for Genmod
			dec->function = MPGMBOX;

			if (!m_turbo) set_wait(1);
			// Check: Are waitstates completely turned off for turbo mode, or
			// merely the waitstates for DRAM memory access and box access?

			m_peribox->setaddress_dbin(space, dec->physaddr, read_mode);
			return;
		}
	}
	else
	{   // Write access
		// Logical addresses
		if (m_geneve_mode)
		{
			if ((offset & 0xfff1)==0xf100)
			{
				// 1 WS is always added; any pending video waitstates are canceled
				dec->function = MLGVIDEO;
				set_wait(1);
				return;
			}
			if ((offset & 0xfff8)==0xf110)
			{
				dec->function = MLGMAPPER;
				dec->offset = dec->offset & 0x0007;
				set_wait(1);
				return;
			}
			if ((offset & 0xfff1)==0xf120)
			{
				// Add 24 waitstates. This is an average value, as the
				// waitstate generation seems to depend on an external timer of
				// the sound chip
				// TODO: do it properly with the use of READY
				dec->function = MLGSOUND;
				set_wait(24);
				return;
			}
			if ((offset & 0xfff0)==0xf130)
			{
				dec->function = MLGCLOCK;
				dec->offset = dec->offset & 0x00f;
				set_wait(1);
				return;
			}
		}
		else
		{
			// TI mode
			if ((offset & 0xfff8)==0x8000)
			{
				dec->function = MLTMAPPER;
				dec->offset = dec->offset & 0x0007;
				set_wait(1);
				return;
			}
			if ((offset & 0xfff0)==0x8010)
			{
				dec->function = MLTCLOCK;
				dec->offset = dec->offset & 0x00f;
				set_wait(1);
				return;
			}
			if ((offset & 0xfc01)==0x9c00)
			{
				dec->function = MLTGROM;
				set_wait(1);
				return;
			}
			if ((offset & 0xfc01)==0x8400)
			{
				// Add 24 waitstates. This is an approximation, as the
				// waitstate generation seems to depend on an external timer of
				// the sound chip
				// TODO: do it properly with the use of READY-
				dec->function = MLTSOUND;
				set_wait(24);
				return;
			}
			if ((offset & 0xfc01)==0x8c00)
			{
				// 1 WS is always added; any pending video waitstates are canceled
				dec->function = MLTVIDEO;
				set_wait(1);
				return;
			}

			if ((offset & 0xfc01)==0x9400)
			{
				dec->function = MLTSPEECH;
				dec->offset = dec->offset | ((m_genmod)? 0x170000 : 0x070000);
				m_peribox->setaddress_dbin(space, dec->offset, read_mode);
				set_wait(1);
				return;
			}
		}

		// Determine physical address
		page = (dec->offset & 0xe000) >> 13;

		if (m_direct_mode)
		{
			dec->physaddr = 0x1e0000; // points to boot eprom
		}
		else
		{
			if (!m_geneve_mode && page==3)
			{
				if (m_cartridge_size==0x4000)
				{
					m_cartridge_secondpage = ((dec->offset & 0x0002)!=0);
					if (TRACE_WRITE) logerror("%s: Set cartridge page %02x\n", tag(), m_cartridge_secondpage);
					set_wait(1);
					return;
				}
				else
				{
					// writing into cartridge rom space (no bankswitching)
					if ((((dec->offset & 0x1000)==0x0000) && !m_cartridge6_writable)
						|| (((dec->offset & 0x1000)==0x1000) && !m_cartridge7_writable))
					{
						logerror("%s: Writing to protected cartridge space %04x ignored\n", tag(), dec->offset);
						return;
					}
					else
						// TODO: Check whether secondpage is really ignored
					dec->physaddr = 0x06c000;
				}
			}
			else
				dec->physaddr = (m_map[page] << 13);
		}

		dec->physaddr |= dec->offset & 0x1fff;

		if (!m_genmod)
		{
			if ((dec->physaddr & 0x180000)==0x000000)
			{
				dec->function = MPGDRAM;
				dec->physaddr = dec->physaddr & 0x07ffff;
				set_wait(1);
				return;
			}
			if ((dec->physaddr & 0x180000)==0x080000)
			{
				dec->function = MPGEXP;
				set_wait(1);
				return;
			}

			if ((dec->physaddr & 0x1e0000)==0x1e0000)
			{
				dec->function = MPGEPROM;
				set_wait(0); // EPROM
				return;
			}
			if ((dec->physaddr & 0x180000)==0x180000)
			{
				dec->function = MPGSRAM;
				set_wait(0); // SRAM
				return;
			}

			// Route everything else to the P-Box
			// Add a wait state

			// only AMA, AMB, AMC are used; AMD and AME are not used
			dec->function = MPGBOX;
			dec->physaddr = (dec->physaddr & 0x0007ffff);  // 19 bit address
			m_peribox->setaddress_dbin(space, dec->physaddr, read_mode);
			set_wait(1);
		}
		else
		{
			// GenMod mode
			if ((dec->physaddr & 0x1e0000)==0x1e0000)
			{   // EPROM, ignore (unless PFM)
				dec->function = MPGMEPROM;
				set_wait(0);
				return;
			}

			if (m_timode && ((dec->physaddr & 0x180000)==0x000000))
			{
				dec->function = MPGMDRAM;
				dec->physaddr = dec->physaddr & 0x07ffff;
				if (!m_turbo) set_wait(1);
				return;
			}

			// Route everything else to the P-Box
			dec->function = MPGMBOX;
			dec->physaddr = (dec->physaddr & 0x001fffff);  // 21 bit address for Genmod
			m_peribox->setaddress_dbin(space, dec->physaddr, read_mode);
			if (!m_turbo) set_wait(1);
		}
	}
}

/*
    Read from PFM.
*/
READ8_MEMBER( geneve_mapper_device::read_from_pfm )
{
	UINT8 value = 0;
	if (!m_pfm_output_enable) return 0;

	int address = (offset & 0x01ffff) | (m_pfm_bank<<17);

	switch (m_pfm_mode)
	{
	case 1:
		value = m_pfm512->read(space, address, mem_mask);
		break;
	case 2:
		value = m_pfm512a->read(space, address, mem_mask);
		break;
	default:
		logerror("%s: Illegal mode for reading PFM: %d\n", tag(), m_pfm_mode);
		return 0;
	}

	if (TRACE_PFM) logerror("%s: Reading from PFM at address %05x -> %02x\n", tag(), address, value);
	return value;
}

WRITE8_MEMBER( geneve_mapper_device::write_to_pfm )
{
	// Nota bene: The PFM must be write protected on startup, or the RESET
	// of the 9995 will attempt to write the return vector into the flash EEPROM
	int address = (offset & 0x01ffff) | (m_pfm_bank<<17);
	if (TRACE_PFM) logerror("%s: Writing to PFM at address %05x <- %02x\n", tag(), address, data);

	switch (m_pfm_mode)
	{
	case 1:
		m_pfm512->write(space, address, data, mem_mask);
		break;
	case 2:
		m_pfm512a->write(space, address, data, mem_mask);
		break;
	default:
		logerror("%s: Illegal mode for writing to PFM: %d\n", tag(), m_pfm_mode);
	}
}


/*
    Accept the address passed over the address bus and decode it appropriately.
    This decoding will later be used in the READ/WRITE member functions. Also,
    we initiate wait state creation here.
*/
SETOFFSET_MEMBER( geneve_mapper_device::setoffset )
{
	if (TRACE_DETAIL) logerror("%s: setoffset = %04x\n", tag(), offset);
	m_debug_no_ws = false;
	decode(space, offset, m_read_mode, &m_decoded);
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
					if (TRACE_CLOCK) logerror("%s: clock, READY asserted\n", tag());
					m_ready(ASSERT_LINE);
					m_ready_asserted = true;
				}
				else
				{
					if (TRACE_CLOCK) logerror("%s: clock\n", tag());
				}
			}
			else
			{
				if (m_ext_waitcount > 0)
				{
					m_ext_waitcount--;
					if (m_ext_waitcount == 0)
					{
						if (TRACE_CLOCK) logerror("%s: clock, READY asserted after video\n", tag());
						m_ready(ASSERT_LINE);
						m_ready_asserted = true;
					}
					else
					{
						if (TRACE_CLOCK) logerror("%s: vclock, ew=%d\n", tag(), m_ext_waitcount);
					}
				}
			}
		}
	}
	else
	{
		// Falling edge
		// Do we have video wait states? In that case, clear the line again
		if ((m_waitcount == 0) && (m_ext_waitcount > 0) && m_ready_asserted)
		{
			if (TRACE_CLOCK) logerror("%s: clock, READY cleared for video\n", tag());
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
	if (TRACE_DETAIL) logerror("%s: dbin = %02x\n", tag(), m_read_mode? 1:0);
}

/*
    PFM expansion: Setting the bank.
*/
WRITE_LINE_MEMBER( geneve_mapper_device::pfm_select_lsb )
{
	if (state==ASSERT_LINE) m_pfm_bank |= 1;
	else m_pfm_bank &= 0xfe;
	if (TRACE_PFM) logerror("%s: Setting bank (l) = %d\n", tag(), m_pfm_bank);
}

WRITE_LINE_MEMBER( geneve_mapper_device::pfm_select_msb )
{
	if (state==ASSERT_LINE) m_pfm_bank |= 2;
	else m_pfm_bank &= 0xfd;
	if (TRACE_PFM) logerror("%s: Setting bank (u) = %d\n", tag(), m_pfm_bank);
}

WRITE_LINE_MEMBER( geneve_mapper_device::pfm_output_enable )
{
	// Negative logic
	m_pfm_output_enable = (state==CLEAR_LINE);
	if (TRACE_PFM) logerror("%s: PFM output %s\n", tag(), m_pfm_output_enable? "enable" : "disable");
}

//====================================================================
//  Common device lifecycle
//====================================================================

void geneve_mapper_device::device_start()
{
	// Get pointers
	m_peribox = machine().device<bus8z_device>(PERIBOX_TAG);
	m_keyboard = machine().device<geneve_keyboard_device>(GKEYBOARD_TAG);
	m_video = machine().device<bus8z_device>(VIDEO_SYSTEM_TAG);
	m_sound = machine().device<bus8z_device>(TISOUND_TAG);
	m_clock = machine().device<mm58274c_device>(GCLOCK_TAG);

	// PFM expansion
	m_pfm512 = machine().device<at29c040_device>(PFM512_TAG);
	m_pfm512a = machine().device<at29c040a_device>(PFM512A_TAG);

	m_ready.resolve();

	m_sram = machine().root_device().memregion(SRAM_TAG)->base();
	m_dram = machine().root_device().memregion(DRAM_TAG)->base();
	m_cpu = static_cast<tms9995_device*>(machine().device("maincpu"));

	m_geneve_mode = false;
	m_direct_mode = true;
}

void geneve_mapper_device::device_reset()
{
	m_extra_waitstates = false;
	m_video_waitstates = true;
	m_read_mode = false;
	m_waitcount = 0;
	m_ext_waitcount = 0;
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
	for (int i=0; i < 8; i++) m_map[i] = 0;

	m_genmod = false;

	// Check which boot EPROM we are using (or PFM)
	set_boot_rom(machine().root_device().ioport("BOOTROM")->read());

	// Check for GenMod. We assume that GenMod can be combined with PFM.
	if (machine().root_device().ioport("MODE")->read()!=0)
	{
		logerror("%s: Using GenMod modification\n", tag());
		m_eprom = machine().root_device().memregion("maincpu")->base() + 0x8000;
		if (m_eprom[0] != 0xf0)
		{
			fatalerror("genboard: GenMod boot ROM missing\n");
		}
		m_genmod = true;
		m_turbo = ((machine().root_device().ioport("GENMODDIPS")->read() & GM_TURBO)!=0);
		m_timode = ((machine().root_device().ioport("GENMODDIPS")->read() & GM_TIM)!=0);
	}

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

const device_type GENEVE_MAPPER = &device_creator<geneve_mapper_device>;

/****************************************************************************
    Keyboard support
****************************************************************************/

static const char *const KEYNAMES[] = { "KEY0", "KEY1", "KEY2", "KEY3", "KEY4", "KEY5", "KEY6", "KEY7" };

static const UINT8 MF1_CODE[0xe] =
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

geneve_keyboard_device::geneve_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, GENEVE_KEYBOARD, "Geneve XT-style keyboard", tag, owner, clock, "geneve_keyboard", __FILE__),
	m_interrupt(*this), m_key_reset(false), m_key_queue_length(0), m_key_queue_head(0), m_key_in_buffer(false), m_key_numlock_state(false), m_key_ctrl_state(0), m_key_alt_state(0),
	m_key_real_shift_state(0), m_key_fake_shift_state(false), m_key_fake_unshift_state(false), m_key_autorepeat_key(0), m_key_autorepeat_timer(0), m_keep_keybuf(false),
	m_keyboard_clock(false), m_timer(nullptr)
{
}

void geneve_keyboard_device::post_in_key_queue(int keycode)
{
	m_key_queue[(m_key_queue_head + m_key_queue_length) % KEYQUEUESIZE] = keycode;
	m_key_queue_length++;

	if (TRACE_KEYBOARD) logerror("%s: Posting keycode %02x\n", tag(), keycode);
}

void geneve_keyboard_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	poll();
}

void geneve_keyboard_device::poll()
{
	UINT32 keystate;
	UINT32 key_transitions;
	int i, j;
	int keycode;
	int pressed;
	if (TRACE_KEYBOARD) logerror("%s: Poll keyboard\n", tag());
	if (m_key_reset) return;

	/* Poll keyboard */
	for (i = 0; (i < 4) && (m_key_queue_length <= (KEYQUEUESIZE-MAXKEYMSGLENGTH)); i++)
	{
		keystate = ioport(KEYNAMES[2*i])->read() | (ioport(KEYNAMES[2*i + 1])->read() << 16);
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

UINT8 geneve_keyboard_device::get_recent_key()
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
		if (TRACE_KEYBOARD) logerror("%s: Signalling key available\n", tag());
		m_interrupt(ASSERT_LINE);
		m_key_in_buffer = true;
	}
}

WRITE_LINE_MEMBER( geneve_keyboard_device::clock_control )
{
	bool rising_edge = (!m_keyboard_clock && (state==ASSERT_LINE));
	m_keyboard_clock = (state==ASSERT_LINE);
	if (TRACE_KEYBOARD) logerror("%s: Keyboard clock_control state=%d\n", tag(), m_keyboard_clock);
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

const device_type GENEVE_KEYBOARD = &device_creator<geneve_keyboard_device>;

/****************************************************************************
    Mouse support
****************************************************************************/

geneve_mouse_device::geneve_mouse_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: device_t(mconfig, GENEVE_MOUSE, "Geneve mouse", tag, owner, clock, "geneve_mouse", __FILE__), m_v9938(nullptr), m_last_mx(0), m_last_my(0)
{
}

line_state geneve_mouse_device::left_button()
{
	return ((ioport("MOUSE0")->read() & 0x04)!=0)? ASSERT_LINE : CLEAR_LINE;
}

void geneve_mouse_device::poll()
{
	int new_mx, new_my;
	int delta_x, delta_y, buttons;

	buttons = ioport("MOUSE0")->read();
	new_mx = ioport("MOUSEX")->read();
	new_my = ioport("MOUSEY")->read();

	/* compute x delta */
	delta_x = new_mx - m_last_mx;

	/* check for wrap */
	if (delta_x > 0x80)
		delta_x = 0x100-delta_x;
	if  (delta_x < -0x80)
		delta_x = -0x100-delta_x;

	m_last_mx = new_mx;

	/* compute y delta */
	delta_y = new_my - m_last_my;

	/* check for wrap */
	if (delta_y > 0x80)
		delta_y = 0x100-delta_y;
	if  (delta_y < -0x80)
		delta_y = -0x100-delta_y;

	m_last_my = new_my;

	// only middle and right button go to V9938
	m_v9938->update_mouse_state(delta_x, delta_y, buttons & 0x03);
}

INPUT_PORTS_START( genmouse )
	PORT_START("MOUSEX") /* Mouse - X AXIS */
		PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSEY") /* Mouse - Y AXIS */
		PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("MOUSE0") /* mouse buttons */
		PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left mouse button")
		PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right mouse button")
		PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Middle mouse button")
INPUT_PORTS_END

void geneve_mouse_device::device_start()
{
	m_v9938 = machine().device<v9938_device>(VDP_TAG);
}

void geneve_mouse_device::device_reset()
{
	m_last_mx = 0;
	m_last_my = 0;
}

ioport_constructor geneve_mouse_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( genmouse );
}

const device_type GENEVE_MOUSE = &device_creator<geneve_mouse_device>;

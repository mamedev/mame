// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/******************************************************************************

    ======  Original system =========

    Myarc Geneve 9640
    =================

    +--------------------------------------------------------------------+
    |                                   /---\                        +---+
    |  +-----+-----+                    |Bat|                        |Vid()
    |  |     |     |  +-HM62256---+ ____\---/                        +---+
    |  +-----+-----+  | 32K SRAM  ||RTC_|+-------------+             +---+
    |  |   512K    |  +-----------+ ____ |    V9938    |             |Mou[]
    |  +-  DRAM ---+  +-----------+|Snd_||     VDP     |             +---+
    |  | ... | ... |  | 16K EPROM |      +-------------+             +---+
    |  |    16*    |  +-----------+     +----+         +--+   +----+ |Joy[]
    |  +--HM50256--+  +--------------+  |    | +-----+ |  |   |Keyb| +---+
    |  |   256Kx1  |  |   TMS9995    |  |    | |128K | |  +---+(  )+----+
    |  +-----+-----+  |     CPU      |  |TMS | +Video+ |
    |  |     |     |  +--------------+  |9901| |RAM  | |  outside of box
    |  +-----+-----+   +--------+  +-+  |    | +- - -+ |
    |                  |        |  |P|  |    | |4*HM | |
    |                  | Gate   |  |A|  |    | +50464+ |
    |                  | Array  |  |L|  +----+ |64Kx4| |
   O= LED              |        |  +-+         +-----+ |
    |              +-+ +--------+          +-+         |
    +--------------+ |                     | +---------+
    Front            |||||||||||||||||||||||       Back
                     +---------------------+

    The Geneve 9640 is a card for the Peripheral Expansion Box of the TI-99/4A
    system, equipped with a complete computing architecture and thus replaces
    the TI console. It was created by the company Myarc Inc. in 1987, who also
    manufactured several expansion cards for the TI system (like floppy disk
    controllers, hard disk controllers, serial interfaces, memory expansions).

    The Geneve 9640 got its name purportedly from a picture at the wall of
    one of its creators, showing the Swiss city Geneva (Geneve). The number
    9640 should be read as 9-640, with the 9 being a reference to the TI-99
    family (TI did not agree to use the 99 as a reference), and the 640 as
    the amount of built-in RAM (CPU + video).

    It is equipped with a TMS9995 microprocessor, which is downward compatible
    with the TMS9900 used in the TI console. It is clocked at 12 MHz,
    internally divided by 4, which makes it compatible with the expected clock
    rate of external devices, but internally it is much more efficient and
    can execute TMS programs about 2-3 times faster. For more details, see
    tms9995.cpp.

    General architecture
    --------------------
    - CPU: TMS9995, 12 Mhz, with 256 bytes on-chip RAM and decrementer
    - RAM: SRAM (32 KiB, 0 wait states, HM62256 32Kx8),
           DRAM (512 KiB, 1 wait state, 16*HM50256 256Kx1)
    - Video: Yamaha V9938 (compatible to TMS9928, also used in MSX2)
    - Video memory: 128 KiB (4*HM50464 64Kx4)
    - Sound: SN76496 (compat. to TMS9919)
    - System interface: TMS9901
    - Real time clock: MM58274, battery-backed
    - Keyboard: Connector for external XT-compatible keyboard
    - Mouse: Bus mouse connector, going to the color bus of the V9938
    - Joysticks: TI-99/4A joystick connector
    - Video output: RGB or composite via DIN plug

    A Gate Array circuit (labeled Myarc M60014-1004J 715500) contains most of
    logic circuitry for device and memory selection, a memory mapper, and the
    keyboard interface. Unfortunately, the details of its implementation must
    be considered as lost. We can only guess its implementation by its
    behavior.

    Also, a PAL circuit (PAL16R4ACN) is mounted on the board whose task is
    to drive the READY line to the processor and selection lines to the
    peribox. The equations of the PAL are available. The PAL controls
    the creation of wait states according to the device or memory selected
    by the Gate Array.


    Operating modes
    ---------------
    To achieve a maximum degree of compatibility, the Geneve offers two
    operation modes: native and GPL.

    In the GPL mode, memory space layout is largely equal to the TI-99/4A
    layout, thus allowing programs to run without any adaptation.

    The native mode rearranges the memory layout in order to exploit all
    capabilities of the enhanced architecture. However, this mode cannot run
    older TI-99 programs.

    Memory Map
    ----------
    The memory map is described in genboard.cpp where the emulation of the
    Gate Array and the PAL are implemented.

    CRU map
    -------
    The CRU space contains the selectable devices and flags of the system.
    For more details, see genboard.cpp.


    ======  Modifications  ===========

    32K SRAM expansion
    ------------------
    The schematics prove that Myarc already planned for a 32K expansion of
    the stock 32K SRAM memory. One additional chip is soldered on top
    of the base 32K chip, with its CE* line (pin 20) wired to pin 48 of the
    Gate Array (RAMEN-X*).

    The 32K expansion is required for releases of the operating system (MDOS)
    later than version 2.5.


    PFM: Boot EPROM replacement
    ---------------------------
    The Programmable Flash Memory mod replaces the stock EPROM (16K) by a
    flash memory chip (AT29C040(A) for 512K). It is enabled by selecting
    the machine configuration switch BOOTROM. When set to EPROM, the mod is
    inactive.

    There were several versions. Only the third one is emulated, since it
    functionally subsumes the other two:

    PFM: Original version, 128 KiB
    PFM+: Expansion of the original version, piggybacked, adds another 128KiB
    PFM512: Using an AT29C040 (not A), 512 KiB

    The PFM512 is visible as four banks in memory pages 0xF0-0xFF.
    Bank 0 is the boot code, while banks 1-3 can be used as flash drives.

    The lower 13 bits (A3-A15 by TI counting) of its memory space are set by
    the main address bus, provided that the Gate Array enables the access.
    This is true when the EPROM bank area is accessed (pages F0-FF) or in
    direct mode (unmapped mode, used during boot).

    The next four bits (AMA, AB0, AB1, AB2) are set by the Gate Array as the
    lower four bits of the page number (F0-FF, i.e. 0-F). Note that the AMB
    and further lines cannot be used, because the boot ROM area is restricted
    to that range by the Gate Array.

    To set the most significant two bits of the flash memory chip (A17 and A18
    in the usual counting), two dedicated CRU bits are used, set and latched
    by the 9901 chip. Also, the output can be disabled by another CRU bit.

    CRU 0028: LSB of bank number
    CRU 003A: MSB of bank number
    CRU 002A: PFM output enable

    Genmod
    ------
    A special modification was published for the Geneve around 1990. The goal
    was to fill all memory space with physical memory from a memory
    expansion card, and also to make use of 0-wait state SRAM from this card.
    The mod required some few changes on the Geneve card. The Geneve itself
    never had a large user base, compared to the 99/4A, and this modification
    was even much rarer. Nevertheless, its interesting point is that it
    in fact expands the Geneve architecture to its maximum.

    For more details, see genboard.cpp.

    Genmod is treated as a separate system in MAME. Beside the modified
    hardware, it also requires a modified boot ROM.

    ============================================

    2003: Original version by Raphael Nabet
    2012: Rewritten by Michael Zapf (functionally, high level)
    2019: Rewritten by Michael Zapf (closer to the actual hardware)

******************************************************************************/

#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "machine/tms9901.h"
#include "machine/mm58274c.h"
#include "sound/sn76496.h"

#include "bus/ti99/internal/genboard.h"
#include "bus/ti99/internal/genkbd.h"

#include "bus/ti99/colorbus/colorbus.h"
#include "bus/ti99/joyport/joyport.h"
#include "bus/ti99/peb/peribox.h"

#include "bus/pc_kbd/keyboards.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/pcxt83.h"
#include "bus/pc_kbd/keytro.h"

#include "speaker.h"

#define LOG_WARN     (1U << 1)
#define LOG_CRU      (1U << 2)
#define LOG_CRUKEY   (1U << 3)
#define LOG_READ     (1U << 4)
#define LOG_READG    (1U << 5)
#define LOG_WRITE    (1U << 6)
#define LOG_CONFIG   (1U << 7)
#define LOG_PFM      (1U << 8)

// Minimum log should be settings and warnings
#define VERBOSE ( LOG_GENERAL | LOG_CONFIG | LOG_WARN )

#include "logmacro.h"


namespace {

#define GENEVE_SRAM_TAG  "sram"
#define GENEVE_SRAMX_TAG "sramexp"
#define GENEVE_SRAMU_TAG "sramult"
#define GENEVE_DRAM_TAG  "dram"
#define GENEVE_CLOCK_TAG "mm58274c"
#define GENEVE_SOUNDCHIP_TAG   "soundchip"
#define GENEVE_TMS9901_TAG     "tms9901"
#define GENEVE_SCREEN_TAG      "screen"

enum
{
	AB2 = 1,
	AB1 = 2,
	AB0 = 4,
	AMA = 8,
	FULLGEN = 63,  // AMC,AMB,AMA,AB0,AB1,AB2
	FULLGNM = 255  // AME,AMD,FULLGEN
};

enum
{
	SRAM32 = 0,
	SRAM64 = 1,
	SRAM384 = 2
};

void geneve_xt_keyboards(device_slot_interface &device)
{
	device.option_add(STR_KBD_KEYTRONIC_PC3270, PC_KBD_KEYTRONIC_PC3270);
	device.option_add(STR_KBD_IBM_PC_XT_83, PC_KBD_IBM_PC_XT_83);
	device.option_add(STR_KBD_GENEVE_XT_101_HLE, KBD_GENEVE_XT_101_HLE);
}

class geneve_state : public driver_device
{
public:
	geneve_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_cpu(*this, "maincpu"),
		m_tms9901(*this, GENEVE_TMS9901_TAG),
		m_sound(*this, GENEVE_SOUNDCHIP_TAG),
		m_video(*this, TIGEN_V9938_TAG),
		m_rtc(*this, GENEVE_CLOCK_TAG),
		m_dram(*this, GENEVE_DRAM_TAG),
		m_sram(*this, GENEVE_SRAM_TAG),
		m_sramx(*this, GENEVE_SRAMX_TAG),
		m_sramu(*this, GENEVE_SRAMU_TAG),
		m_gatearray(*this, GENEVE_GATE_ARRAY_TAG),
		m_genmod_decoder(*this, GENMOD_DECODER_TAG),
		m_pal(*this, GENEVE_PAL_TAG),
		m_joyport(*this, TI_JOYPORT_TAG),
		m_colorbus(*this, COLORBUS_TAG),
		m_kbdconn(*this, "kbd"),
		m_peribox(*this, TI_PERIBOX_TAG),
		m_pfm512(*this, GENEVE_PFM512_TAG),
		m_pfm512a(*this, GENEVE_PFM512A_TAG),
		m_left_button(0),
		m_pfm_prefix(0),
		m_pfm_oe(true),
		m_sram_size(SRAM64),
		m_genmod(false)
	{
	}

	void geneve_common(machine_config &config);
	void geneve(machine_config &config);
	void genmod(machine_config &config);
	void init_geneve();
	void init_genmod();

	DECLARE_INPUT_CHANGED_MEMBER( settings_changed );
	DECLARE_INPUT_CHANGED_MEMBER( setgm_changed );

private:
	// CRU (Communication Register Unit) handling
	uint8_t cruread(offs_t offset);
	void cruwrite(offs_t offset, uint8_t data);

	// Connections with the system interface TMS9901
	uint8_t psi_input(offs_t offset);
	void peripheral_bus_reset(int state);
	void VDP_reset(int state);
	void joystick_select(int state);
	void keyboard_reset(int state);
	void video_wait_states(int state);
	void left_mouse_button(int state);

	void keyboard_clock_line(int state);
	void keyboard_data_line(int state);

	void clock_out(int state);

	void external_operation(offs_t offset, uint8_t data);

	void tms9901_interrupt(offs_t offset, uint8_t data);

	void keyboard_interrupt(int state);

	required_device<tms9995_device>     m_cpu;
	required_device<tms9901_device>     m_tms9901;
	required_device<sn76496_device>     m_sound;
	required_device<v9938_device>       m_video;
	required_device<mm58274c_device>    m_rtc;
	required_device<ram_device>         m_dram;
	required_device<ram_device>         m_sram;
	required_device<ram_device>         m_sramx;
	required_device<ram_device>         m_sramu;

	required_device<bus::ti99::internal::geneve_gate_array_device> m_gatearray;
	optional_device<bus::ti99::internal::genmod_decoder_device> m_genmod_decoder;
	required_device<bus::ti99::internal::geneve_pal_device>        m_pal;
	required_device<bus::ti99::joyport::joyport_device>            m_joyport;
	required_device<bus::ti99::colorbus::v9938_colorbus_device>    m_colorbus;
	required_device<pc_kbdc_device>                                m_kbdconn;
	required_device<bus::ti99::peb::peribox_device>                m_peribox;

	uint8_t* m_eprom = nullptr;  // Pointer to the EPROM

	// PFM expansion
	required_device<at29c040_device>     m_pfm512;
	required_device<at29c040a_device>    m_pfm512a;
	void read_eprom_or_pfm(offs_t offset, uint8_t& value);
	void write_pfm(offs_t offset, uint8_t data);

	void pfm_a17(int state);
	void pfm_a18(int state);
	void pfm_oe(int state);

	// Interrupts
	void inta(int state);
	void intb(int state);
	void keyboard_int(int state);
	void int2_from_v9938(int state);

	// READY line contributors
	void extready(int state);
	void sndready(int state);

	// Memory bus
	void setaddress_debug(bool debug, offs_t address, uint8_t busctrl);
	void setaddress(offs_t address, uint8_t busctrl);
	uint8_t memread(offs_t offset);
	void memwrite(offs_t offset, uint8_t data);

	void crumap(address_map &map) ATTR_COLD;
	void memmap(address_map &map) ATTR_COLD;
	void memmap_setaddress(address_map &map) ATTR_COLD;

	// General device lifecycle
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// Members
	int  m_inta = 0;
	int  m_intb = 0;
	int  m_int2 = 0;
	int  m_keyint = 0;

	int     m_left_button;   // Left mouse button, not wired to the 9938
	int     m_pfm_prefix;
	bool    m_pfm_oe;

	// Settings
	int m_boot_rom = 0;     // Kind of boot ROM (EPROM or PFM512 or PFM512A)
	int m_sram_size = SRAM64;

	// Genmod modifications
	bool m_genmod;
};

/*
    Memory map
*/

void geneve_state::memmap(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(geneve_state::memread), FUNC(geneve_state::memwrite));
}

void geneve_state::memmap_setaddress(address_map &map)
{
	map(0x0000, 0xffff).w(FUNC(geneve_state::setaddress));
}

/*
    CRU map
    The TMS9901 is fully decoded, no mirroring, so we have 32 bits for it,
    and the rest goes to the board (and from there to the PEB)
    TMS9995 has a full 15-bit CRU bit address space (attached to A0-A14)

    We cannot use the map because there is a least one card (sidmaster) that
    activates itself when no other device is selected.
*/
void geneve_state::crumap(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(geneve_state::cruread), FUNC(geneve_state::cruwrite));
}

static INPUT_PORTS_START(geneve_common)

	PORT_START( "BOOTROM" )
	PORT_CONFNAME( 0x03, GENEVE_EPROM, "Boot from" ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(geneve_state::settings_changed), 3)
		PORT_CONFSETTING( GENEVE_EPROM, "EPROM" )
		PORT_CONFSETTING( GENEVE_PFM512, "PFM 512" )
		PORT_CONFSETTING( GENEVE_PFM512A, "PFM 512A" )

	PORT_START( "VRAM" )
	PORT_CONFNAME( 0x01, 0x00, "Video RAM" )
		PORT_CONFSETTING( 0x00, "128 KiB" )
		PORT_CONFSETTING( 0x01, "192 KiB" )

INPUT_PORTS_END

static INPUT_PORTS_START(geneve)
	PORT_INCLUDE(geneve_common)

	PORT_START( "SRAM" )
	PORT_CONFNAME( 0x03, 0x01, "SRAM size" )
		PORT_CONFSETTING( SRAM32, "32 KiB" )
		PORT_CONFSETTING( SRAM64, "64 KiB" )
		PORT_CONFSETTING( SRAM384, "384 KiB" )

INPUT_PORTS_END

static INPUT_PORTS_START(genmod)
	PORT_INCLUDE(geneve_common)

	PORT_START( "GENMODDIPS" )
	PORT_DIPNAME( GENEVE_GM_TURBO, GENEVE_GM_TURBO, "Genmod Turbo mode") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(geneve_state::setgm_changed), 1)
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( GENEVE_GM_TURBO, DEF_STR( On ))
	PORT_DIPNAME( GENEVE_GM_TIM, 0, "Genmod TI mode") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(geneve_state::setgm_changed), 2)
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( GENEVE_GM_TIM, DEF_STR( On ))

INPUT_PORTS_END

INPUT_CHANGED_MEMBER( geneve_state::settings_changed )
{
	// Used when switching the boot ROMs during runtime, especially the PFM
	m_boot_rom = newval;
}

INPUT_CHANGED_MEMBER( geneve_state::setgm_changed )
{
	int number = int(param&0x03);
	int value = newval;

	switch (number)
	{
	case 1:
		// Turbo switch. May be changed at any time.
		LOGMASKED(LOG_CONFIG, "Setting turbo flag to %d\n", value);
		m_genmod_decoder->set_turbo(value!=0);
		break;
	case 2:
		// TIMode switch. Causes reset when changed.
		LOGMASKED(LOG_CONFIG, "Setting timode flag to %d\n", value);
		m_genmod_decoder->set_timode(value!=0);
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

/*
    Propagate the address bus levels to the Gate Array. This will make it
    create waitstates before the read operation and assert the selector lines.
*/
void geneve_state::setaddress(offs_t address, uint8_t busctrl)
{
	setaddress_debug(false, address, busctrl);
}

void geneve_state::setaddress_debug(bool debug, offs_t address, uint8_t busctrl)
{
	m_gatearray->set_debug(debug);
	m_gatearray->setaddress(address, busctrl);

	// Genmod
	if (m_genmod)
	{
		m_genmod_decoder->set_debug(debug);
		m_genmod_decoder->set_function(m_gatearray->get_function(), m_gatearray->get_prefix(FULLGNM)>>13);
	}

	// Insert a wait state except for SRAM (not by debugger)
	if (!debug)
	{
		// Note that the CSR/CSW inputs must not be asserted for more than
		// 2 us; i.e. they cannot remain asserted for the whole 14 wait cycles
		// (which are 4.6 us). Accordingly, we have to assume that the CSW/CSR
		// signals are only asserted for a single clock cycle (333ns, which
		// is more than the minimum pulse width of 186 ns).
		// See V9938 specs
		m_pal->csw_in(m_gatearray->csw_out());
		m_pal->csr_in(m_gatearray->csr_out());

		// Trigger the 9901 clock when A10=1
		if ((address & 0x0020) != 0)
			m_tms9901->update_clock();
	}

	// Going to the box
	int extbus = m_genmod? m_genmod_decoder->dben_out() : m_gatearray->dben_out();

	if (extbus==ASSERT_LINE)
	{
		offs_t addr13 = address & 0x1fff;
		m_peribox->memen_in(ASSERT_LINE);
		m_peribox->setaddress_dbin(m_gatearray->get_prefix(m_genmod? FULLGNM : FULLGEN) | addr13, ((busctrl & TMS99xx_BUS_DBIN)!=0));
	}
}

uint8_t geneve_state::memread(offs_t offset)
{
	uint8_t value = 0;
	offs_t sramadd;
	offs_t dramadd;
	offs_t pboxadd;
	offs_t addr13 = offset & 0x1fff;
	// For debugging
	int page = m_gatearray->get_prefix(FULLGNM) >> 13;

	if (machine().side_effects_disabled())
	{
		if (m_cpu->is_onchip(offset))
			return m_cpu->debug_read_onchip_memory(offset);

		// The debugger does not call setaddress, so we do it here
		// Also, the decode result is replaced by the debugger version
		setaddress_debug(true, offset, TMS99xx_BUS_DBIN);
	}

	// Video read (never by debugger)
	if (m_gatearray->csr_out()==ASSERT_LINE)
	{
		value = m_video->read((offset>>1)&3);   // M1=A13, M0=A14
		LOGMASKED(LOG_READ, "Video %04x -> %02x\n", offset, value);
	}

	// All of the following parts are accessed in parallel and can set the
	// value on the data bus. In reality, if several of them did, this would
	// be a bug and likely damage the machine.

	// Gate array itself (Keyboard, mapper, GROM address register)
	// If not addressed, value remains unchanged
	m_gatearray->readz(value);

	// Clock
	// The clock is connected with only 4 data bits. Tests on the real machine
	// showed that the upper nibble is 0xf (probably because of the location
	// at f130-f13f?)
	// In TI mode, however, the upper nibble is 1, unless we read 801f,
	// in which case the nibble is 2. Here the location is 8010-801f.
	// Needs more investigation. Simply clearing the upper nibble will not work
	// for some software that assumes these bits to be set.
	if (m_gatearray->rtcen_out()==ASSERT_LINE)
	{
		value = m_rtc->read(offset & 0x000f);
		if (m_gatearray->geneve_mode())
			value |= 0xf0;
		else
			value |= ((offset & 0x001f)+1) & 0xf0;

		LOGMASKED(LOG_READ, "Clock %04x -> %02x\n", offset, value);
	}

	// DRAM (also for GROM simulator)
	// Genmod uses the box, but also the DRAM in parallel. Only for
	// TIMODE, the box access is suppressed.
	if (m_gatearray->accessing_dram())
	{
		dramadd = m_gatearray->get_dram_address();
		value = m_dram->pointer()[dramadd];
		int dpage = dramadd >> 13;

		const char* ramtype = (m_gatearray->accessing_grom())? "GROM" : "DRAM";
		LOGMASKED(LOG_READ, "%s %02x:%04x -> %02x\n", ramtype, dpage, addr13, value);
	}

	// Boot ROM or PFM (normal and Genmod)
	if (m_gatearray->romen_out()==ASSERT_LINE)
		read_eprom_or_pfm(offset, value);

	// Stock SRAM 32K (normal and Genmod)
	if (m_gatearray->ramen_out()==ASSERT_LINE)
	{
		sramadd = m_gatearray->get_prefix(AB1 | AB2) | addr13;
		value = m_sram->pointer()[sramadd];
		LOGMASKED(LOG_READ, "SRAM %02x:%04x -> %02x\n", page, addr13, value);
	}

	// Expanded SRAM 32K (not in Genmod)
	if (!m_genmod && m_gatearray->ramenx_out()==ASSERT_LINE)
	{
		if (m_sram_size != SRAM32)
		{
			sramadd = m_gatearray->get_prefix(AB1 | AB2) | addr13;
			value = m_sramx->pointer()[sramadd];
			LOGMASKED(LOG_READ, "SRAMX %02x:%04x -> %02x\n", page, addr13, value);
		}
		else
			LOGMASKED(LOG_WARN, "Access to SRAMX page %02x, but no SRAM expansion available\n", page);
	}

	// Ultimate SRAM expansion (not in Genmod)
	if (!m_genmod && m_gatearray->ramenu_out()==ASSERT_LINE)
	{
		if (m_sram_size == SRAM384)
		{
			sramadd = m_gatearray->get_prefix(FULLGEN) | addr13;
			value = m_sramu->pointer()[sramadd];
			LOGMASKED(LOG_READ, "SRAMU %02x:%04x -> %02x\n", page, addr13, value);
		}
		else
			LOGMASKED(LOG_WARN, "Access to SRAMU page %02x, but no 384K SRAM expansion available\n", page);
	}

	// Peripheral box
	if ((m_genmod && m_genmod_decoder->dben_out())
		|| (!m_genmod && m_gatearray->dben_out()==ASSERT_LINE))
	{
		pboxadd = m_gatearray->get_prefix(m_genmod? FULLGNM : FULLGEN) | addr13;
		m_peribox->readz(pboxadd, &value);
		m_peribox->memen_in(CLEAR_LINE);
		LOGMASKED(LOG_READ, "PEB %02x:%04x -> %02x\n", page, addr13, value);
	}

	// In case we had a debugger read, reset the flag.
	m_gatearray->set_debug(false);
	if (m_genmod) m_genmod_decoder->set_debug(false);

	return value;
}

void geneve_state::memwrite(offs_t offset, uint8_t data)
{
	offs_t sramadd = 0;
	offs_t dramadd = 0;
	offs_t pboxadd = 0;

	offs_t addr13 = offset & 0x1fff;
	// For debugging
	int page = m_gatearray->get_prefix(FULLGNM) >> 13;

	if (machine().side_effects_disabled())
	{
		if (m_cpu->is_onchip(offset))
		{
			m_cpu->debug_write_onchip_memory(offset, data);
			return;
		}

		// The debugger does not call setaddress, so we do it here
		// Also, the decode result is replaced by the debugger version
		setaddress_debug(true, offset, 0);
	}

	// Video write (never by debugger)
	if (m_gatearray->csw_out()==ASSERT_LINE)
	{
		LOGMASKED(LOG_WRITE, "Video %04x <- %02x\n", offset, data);
		m_video->write((offset>>1)&3, data);   // M1=A13, M0=A14
	}

	// Gate array itself (Keyboard, mapper, GROM address (not by debugger))
	// Has no effect when not addressed
	m_gatearray->write(data);

	// Clock
	if (m_gatearray->rtcen_out()==ASSERT_LINE)
	{
		LOGMASKED(LOG_WRITE, "Clock %04x <- %02x\n", offset, data);
		m_rtc->write(offset & 0x000f, data);
	}

	// DRAM (also for GROM simulator)
	if (m_gatearray->accessing_dram())
	{
		// We block the write access to the DRAM for the Genmod when not in TI mode
		// This is not verified to happen on the real machine, but if we do not
		// block, page 3A will be declared as available, which is wrong.
		if (!m_genmod || !m_genmod_decoder->dben_out())
		{
			dramadd = m_gatearray->get_dram_address();
			m_dram->pointer()[dramadd] = data;
			const char* ramtype = (m_gatearray->accessing_grom())? "GROM" : "DRAM";
			LOGMASKED(LOG_WRITE, "%s %02x:%04x <- %02x\n", ramtype, page, addr13, data);
		}
	}

	// Sound
	if (m_gatearray->snden_out()==ASSERT_LINE)
	{
		LOGMASKED(LOG_WRITE, "Sound %04x <- %02x\n", offset, data);
		m_sound->write(data);
	}

	// Boot ROM or PFM (normal and Genmod)
	if (m_gatearray->romen_out()==ASSERT_LINE)
		write_pfm(offset, data);

	// Stock SRAM 32K
	if (m_gatearray->ramen_out()==ASSERT_LINE)
	{
		sramadd = m_gatearray->get_prefix(AB1 | AB2) | addr13;
		LOGMASKED(LOG_WRITE, "SRAM %02x:%04x <- %02x\n", page, addr13, data);
		m_sram->pointer()[sramadd] = data;
	}

	// Expanded SRAM (not in Genmod)
	if (!m_genmod && m_gatearray->ramenx_out()==ASSERT_LINE)
	{
		if (m_sram_size != SRAM32)
		{
			sramadd = m_gatearray->get_prefix(AB1 | AB2) | addr13;
			LOGMASKED(LOG_WRITE, "SRAMX %02x:%04x <- %02x\n", page, addr13, data);
			m_sramx->pointer()[sramadd] = data;
		}
		else
			LOGMASKED(LOG_WARN, "Access to SRAMX page %02x, but no SRAM expansion available\n", page);
	}

	// Ultimate SRAM expansion (not in Genmod)
	if (!m_genmod && m_gatearray->ramenu_out()==ASSERT_LINE)
	{
		if (m_sram_size == SRAM384)
		{
			sramadd = m_gatearray->get_prefix(FULLGEN) | addr13;
			LOGMASKED(LOG_READ, "SRAMU %02x:%04x -> %02x\n", page, addr13, data);
			m_sramu->pointer()[sramadd] = data;
		}
		else
			LOGMASKED(LOG_WARN, "Access to SRAMU page %02x, but no 384K SRAM expansion available\n", page);
	}

	// Peripheral box
	if ((m_genmod && m_genmod_decoder->dben_out())
		|| (!m_genmod && m_gatearray->dben_out()==ASSERT_LINE))
	{
		pboxadd = m_gatearray->get_prefix(m_genmod? FULLGNM : FULLGEN) | addr13;
		LOGMASKED(LOG_WRITE, "PEB %02x:%04x <- %02x\n", page, addr13, data);
		m_peribox->write(pboxadd, data);
		m_peribox->memen_in(CLEAR_LINE);
	}

	// In case we had a debugger write, reset the flag.
	m_gatearray->set_debug(false);
	if (m_genmod) m_genmod_decoder->set_debug(false);
}

/****************************************************************************
    PFM handling
*****************************************************************************/

void geneve_state::pfm_a17(int state)
{
	if (state==ASSERT_LINE) m_pfm_prefix |= 0x20000;
	else m_pfm_prefix &= ~0x20000;
}

void geneve_state::pfm_a18(int state)
{
	if (state==ASSERT_LINE) m_pfm_prefix |= 0x40000;
	else m_pfm_prefix &= ~0x40000;
}

void geneve_state::pfm_oe(int state)
{
	// Negative logic
	LOGMASKED(LOG_PFM, "PFM output %s\n", (state==0)? "enable" : "disable");
	m_pfm_oe = (state==0);
}

/*
    Boot ROM handling, from EPROM or PFM.
*/
void geneve_state::read_eprom_or_pfm(offs_t offset, uint8_t& value)
{
	int pfmaddress;
	offs_t addr13 = offset & 0x1fff;
	int page = m_gatearray->get_prefix(FULLGNM) >> 13;

	switch (m_boot_rom)
	{
	case GENEVE_EPROM:
		// Mirrors at pages F0, F2, F4,... FE, and F1, F3, ... FF.
		value = m_eprom[addr13 | m_gatearray->get_prefix(AB2)];
		LOGMASKED(LOG_READ, "EPROM %02x:%04x -> %02x\n", page, addr13, value);
		break;
	case GENEVE_PFM512:
		pfmaddress = addr13 | m_gatearray->get_prefix(AMA | AB0 | AB1 | AB2) | m_pfm_prefix;
		if (m_pfm_oe)
		{
			value = m_pfm512->read(pfmaddress);
			LOGMASKED(LOG_PFM, "PFM %02x:%04x -> %02x\n", page, addr13, value);
		}
		else LOGMASKED(LOG_PFM, "PFM512 disabled\n");
		break;
	case GENEVE_PFM512A:
		pfmaddress = addr13 | m_gatearray->get_prefix(AMA | AB0 | AB1 | AB2) | m_pfm_prefix;
		if (m_pfm_oe)
		{
			value = m_pfm512a->read(pfmaddress);
			LOGMASKED(LOG_PFM, "PFM %02x:%04x -> %02x\n", page, addr13, value);
		}
		else LOGMASKED(LOG_PFM, "PFM512a disabled\n");
		break;
	default:
		LOGMASKED(LOG_WARN, "Illegal mode for reading boot ROM: %d\n", m_boot_rom);
	}
}

void geneve_state::write_pfm(offs_t offset, uint8_t data)
{
	// Nota bene: The PFM must be write protected on startup, or the RESET
	// of the 9995 will attempt to write the return vector into the flash EEPROM
	offs_t addr13 = offset & 0x1fff;
	int pfmaddress = addr13 | m_gatearray->get_prefix(AMA | AB0 | AB1 | AB2) | m_pfm_prefix;
	int page = m_gatearray->get_prefix(FULLGNM) >> 13;

	switch (m_boot_rom)
	{
	case GENEVE_EPROM:
		LOGMASKED(LOG_WARN, "Write to EPROM at %02x:%04x ignored\n", page, addr13);
		break;
	case GENEVE_PFM512:
		m_pfm512->write(pfmaddress, data);
		LOGMASKED(LOG_PFM, "PFM %02x:%04x <- %02x\n", page, addr13, data);
		break;
	case GENEVE_PFM512A:
		m_pfm512a->write(pfmaddress, data);
		LOGMASKED(LOG_PFM, "PFMa %02x:%04x <- %02x\n", page, addr13, data);
		break;
	default:
		LOGMASKED(LOG_WARN, "Illegal mode for writing to PFM: %d\n", m_boot_rom);
	}
}

/****************************************************************************
    CRU handling
*****************************************************************************/

void geneve_state::cruwrite(offs_t offset, uint8_t data)
{
	offs_t cruaddr = offset << 1;

	// 9901 access: 0000..003e (fully decoded)
	if ((cruaddr & 0xffc0)==0)
		m_tms9901->write(offset & 0x1f, data);

	// Gate array: 13c0..13ce (Single step), write only
	if ((cruaddr & 0xfff0)==0x13c0)
		m_gatearray->cru_sstep_write(offset, data);

	// Gate array: 1ee0..1efe (mirror of 9995-internal flags), write only
	if ((cruaddr & 0xffe0)==0x1ee0)
		m_gatearray->cru_ctrl_write(offset, data);

	// Rest of the system
	m_peribox->cruwrite(cruaddr, data);
}

uint8_t geneve_state::cruread(offs_t offset)
{
	offs_t cruaddr = offset << 1;
	uint8_t value = 0;
	// 9901 access: 0000..003e (fully decoded)
	if ((cruaddr & 0xffc0)==0)
		value = m_tms9901->read(offset & 0x3f);

	// Propagate the CRU access to external devices
	m_peribox->crureadz(cruaddr, &value);
	return value;
}

/***********************************************************************
    CRU callbacks
***********************************************************************/

uint8_t geneve_state::psi_input(offs_t offset)
{
	switch (offset)
	{
	// External interrupt (INTA)
	case tms9901_device::INT1:
		return (m_inta==CLEAR_LINE)? 1 : 0;

	// Video interrupt
	case tms9901_device::INT2:
		return (m_int2==CLEAR_LINE)? 1 : 0;

	// Joystick port
	case tms9901_device::INT3:
	case tms9901_device::INT4:
	case tms9901_device::INT5:
	case tms9901_device::INT6:
	case tms9901_device::INT7_P15:
		return BIT(m_joyport->read_port(), offset-tms9901_device::INT3);

	// Keyboard interrupt
	case tms9901_device::INT8_P14:
		return (m_keyint==CLEAR_LINE)? 1 : 0;

	// Left mouse button
	case tms9901_device::INT10_P12:
		LOGMASKED(LOG_CRU, "Mouse button = %d\n", m_left_button);
		return (m_left_button==CLEAR_LINE)? 1 : 0;

	// TODO: Real time clock interrupt
	case tms9901_device::INT11_P11:
		return 1;

	// INTB interrupt
	case tms9901_device::INT12_P10:
		return (m_intb==CLEAR_LINE)? 1 : 0;

	default:
		// Pin 9 seems to be queried although there is no connection, maybe
		// by CRU multi-bit operation (STCR)
		// LOGMASKED(LOG_WARN, "Unknown pin %d\n", offset);
		return 1;
	}
}

void geneve_state::left_mouse_button(int state)
{
	m_left_button = state;
}

/*
    Write PE bus reset line
*/
void geneve_state::peripheral_bus_reset(int state)
{
	m_peribox->reset_in(state);
}

/*
    Write VDP reset line
*/
void geneve_state::VDP_reset(int state)
{
	m_video->reset_line(state);
}

/*
    Write joystick select line. 1 selects joystick 1 (pin 7), 0 selects joystick 2 (pin 2)
*/
void geneve_state::joystick_select(int state)
{
	m_joyport->write_port((state==ASSERT_LINE)? 1:2);
}

/*
   Keyboard reset (active low). Most keyboards do not use a dedicated reset
   line but trigger a reset when the clock line is held low for some time.
*/
void geneve_state::keyboard_reset(int state)
{
	if (state==CLEAR_LINE)
		LOG("Keyboard reset (line not connected)\n");
}

/*
    Called by the 9901 core whenever the state of INTREQ and IC0-3 changes.
    As with the TI-99/4A, the interrupt level is delivered as the offset,
    but again it is ignored. Anyway, the TMS9995 has only two external inputs
    (INT1 and INT4).
*/
void geneve_state::tms9901_interrupt(offs_t offset, uint8_t data)
{
	/* INTREQ is connected to INT1. */
	m_cpu->set_input_line(INT_9995_INT1, data);
}

/*******************************************************************
    Signal lines
*******************************************************************/

/*
    inta is connected to both tms9901 IRQ1 line and to tms9995 INT4/EC line.
*/
void geneve_state::inta(int state)
{
	m_inta = (state!=0)? ASSERT_LINE : CLEAR_LINE;
	m_tms9901->set_int_line(1, state);
	m_cpu->set_input_line(INT_9995_INT4, state);
}

/*
    intb is connected to tms9901 IRQ12 line.
*/
void geneve_state::intb(int state)
{
	m_intb = (state!=0)? ASSERT_LINE : CLEAR_LINE;
	m_tms9901->set_int_line(12, state);
}

/*
    set the state of int2 (called by the v9938 core)
*/
void geneve_state::int2_from_v9938(int state)
{
	// This method is frequently called without level change, so we only
	// react on changes
	if (state != m_int2)
	{
		m_int2 = (state!=0)? ASSERT_LINE : CLEAR_LINE;
		m_tms9901->set_int_line(2, state);
	}
}

/*
    Interrupt from the keyboard.
*/
void geneve_state::keyboard_interrupt(int state)
{
	m_keyint = (state!=0)? ASSERT_LINE : CLEAR_LINE;
	m_tms9901->set_int_line(8, state);
}

/*
    READY from the box is connected to the Gate Array and the Genmod board.
*/
void geneve_state::extready(int state)
{
	m_gatearray->extready_in(state);
	if (m_genmod)
		m_genmod_decoder->extready_in(state);
}

/*
    READY from the sound chip is connected to the Gate Array and the Genmod board.
*/
void geneve_state::sndready(int state)
{
	m_gatearray->sndready_in(state);
	if (m_genmod)
		m_genmod_decoder->sndready_in(state);
}

void geneve_state::external_operation(offs_t offset, uint8_t data)
{
	static char const *const extop[8] = { "inv1", "inv2", "IDLE", "RSET", "inv3", "CKON", "CKOF", "LREX" };
	if (offset != IDLE_OP)
		LOGMASKED(LOG_WARN, "External operation %s not implemented on Geneve board\n", extop[offset]);
}

/*
    Clock line from the CPU. Used to control wait state generation.
*/
void geneve_state::clock_out(int state)
{
	m_tms9901->phi_line(state);
	m_gatearray->clock_in(state);


	if (state==ASSERT_LINE)
	{
		// Video also has a GA waitstate, in addition to the video wait states
		// obviously, when comparing to the real machine

		int readyin = m_gatearray->gaready_out();
		if (m_genmod)
		{
			m_genmod_decoder->gaready_in(readyin);
			readyin = m_genmod_decoder->gaready_out();
		}
		m_pal->gaready_in(readyin);
	}
	else
	{
		// Stop the pulse after one cycle for video write. Video read
		// will be reset by the next access.
		if (m_gatearray->csw_out()==ASSERT_LINE)
		{
			m_pal->csw_in(CLEAR_LINE);
		}  // see pal_device::set_ready

		// The pulse must be active so that the READY line is asserted after
		// the ext waitstate
	}

	m_pal->clock_in(state);
}

void geneve_state::init_geneve()
{
	m_genmod = false;
}

void geneve_state::init_genmod()
{
	m_genmod = true;
}

void geneve_state::machine_start()
{
	save_item(NAME(m_inta));
	save_item(NAME(m_intb));
	save_item(NAME(m_int2));
	save_item(NAME(m_keyint));
	save_item(NAME(m_left_button));
	save_item(NAME(m_pfm_prefix));
	save_item(NAME(m_pfm_oe));
}

/*
    Reset the machine.
*/
void geneve_state::machine_reset()
{
	m_inta = CLEAR_LINE;    // flag reflecting the INTA line
	m_intb = CLEAR_LINE;    // flag reflecting the INTB line
	m_int2 = CLEAR_LINE;    // flag reflecting the INT2 line
	m_keyint = CLEAR_LINE;

	// READY=ASSERT; RESET -> no additional wait states
	// READY=CLEAR; RESET -> create wait state in every memory cycle
	m_cpu->ready_line(ASSERT_LINE);
	m_cpu->hold_line(CLEAR_LINE);
	m_cpu->reset_line(ASSERT_LINE);

	m_joyport->write_port(0x01);    // select Joystick 1

	// Configuring the VRAM size
	uint32_t videoram = (ioport("VRAM")->read()!=0)? 0x30000 : 0x20000;
	m_video->set_vram_size(videoram);
	LOGMASKED(LOG_CONFIG, "Video RAM set to %d KiB\n", videoram / 1024);

	// Check which boot EPROM we are using (or PFM)
	m_eprom = memregion("maincpu")->base();
	m_boot_rom = ioport("BOOTROM")->read();

	if (m_genmod)
	{
		m_genmod_decoder->set_turbo((ioport("GENMODDIPS")->read() & GENEVE_GM_TURBO)!=0);
		m_genmod_decoder->set_timode((ioport("GENMODDIPS")->read() & GENEVE_GM_TIM)!=0);
	}
	else
	{
		// SRAM expansion
		// Only applies to the standard Geneve; Genmod uses the Memex instead
		m_sram_size = (ioport("SRAM")->read());
	}
}

void geneve_state::geneve(machine_config &config)
{
	geneve_common(config);

	// Gate array
	GENEVE_GATE_ARRAY(config, m_gatearray, 0);
	m_gatearray->kbdint_cb().set(FUNC(geneve_state::keyboard_interrupt));
	m_gatearray->kbdclk_cb().set(m_kbdconn, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_gatearray->kbddata_cb().set(m_kbdconn, FUNC(pc_kbdc_device::data_write_from_mb));

	// Peripheral expansion box (Geneve composition)
	TI99_PERIBOX_GEN(config, m_peribox, 0);
	m_peribox->inta_cb().set(FUNC(geneve_state::inta));
	m_peribox->intb_cb().set(FUNC(geneve_state::intb));
	m_peribox->ready_cb().set(FUNC(geneve_state::extready));
}

void geneve_state::genmod(machine_config &config)
{
	geneve_common(config);
	GENMOD_DECODER(config, m_genmod_decoder, 0);

	// Gate Array
	GENEVE_GATE_ARRAY(config, m_gatearray, 0);
	m_gatearray->kbdint_cb().set(FUNC(geneve_state::keyboard_interrupt));
	m_gatearray->kbdclk_cb().set(m_kbdconn, FUNC(pc_kbdc_device::clock_write_from_mb));
	m_gatearray->kbddata_cb().set(m_kbdconn, FUNC(pc_kbdc_device::data_write_from_mb));

	// Peripheral expansion box (Geneve composition with Genmod and plugged-in Memex)
	TI99_PERIBOX_GENMOD(config, m_peribox, 0);
	m_peribox->inta_cb().set(FUNC(geneve_state::inta));
	m_peribox->intb_cb().set(FUNC(geneve_state::intb));
	m_peribox->ready_cb().set(FUNC(geneve_state::extready));
}

void geneve_state::geneve_common(machine_config &config)
{
	// basic machine hardware
	// TMS9995 CPU @ 12.0 MHz
	TMS9995(config, m_cpu, 12000000);
	m_cpu->set_addrmap(AS_PROGRAM, &geneve_state::memmap);
	m_cpu->set_addrmap(AS_IO, &geneve_state::crumap);
	m_cpu->set_addrmap(tms9995_device::AS_SETADDRESS, &geneve_state::memmap_setaddress);
	m_cpu->extop_cb().set(FUNC(geneve_state::external_operation));
	m_cpu->clkout_cb().set(FUNC(geneve_state::clock_out));

	// Video hardware
	v99x8_device& video(V9938(config, TIGEN_V9938_TAG, XTAL(21'477'272))); // typical 9938 clock, not verified
	video.set_vram_size(0x20000);
	video.int_cb().set(FUNC(geneve_state::int2_from_v9938));
	video.set_screen(GENEVE_SCREEN_TAG);
	screen_device& screen(SCREEN(config, GENEVE_SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(21'477'272),
		v99x8_device::HTOTAL,
		0,
		v99x8_device::HVISIBLE - 1,
		v99x8_device::VTOTAL_NTSC * 2,
		v99x8_device::VERTICAL_ADJUST * 2,
		v99x8_device::VVISIBLE_NTSC * 2 - 1 - v99x8_device::VERTICAL_ADJUST * 2);
	screen.set_screen_update(TIGEN_V9938_TAG, FUNC(v99x8_device::screen_update));

	// Main board components
	TMS9901(config, m_tms9901, 0);
	m_tms9901->read_cb().set(FUNC(geneve_state::psi_input));
	m_tms9901->p_out_cb(0).set(FUNC(geneve_state::peripheral_bus_reset));
	m_tms9901->p_out_cb(1).set(FUNC(geneve_state::VDP_reset));
	m_tms9901->p_out_cb(2).set(FUNC(geneve_state::joystick_select));
	m_tms9901->p_out_cb(6).set(FUNC(geneve_state::keyboard_reset));
	m_tms9901->p_out_cb(7).set(GENEVE_PAL_TAG, FUNC(bus::ti99::internal::geneve_pal_device::sysspeed));
	m_tms9901->p_out_cb(9).set(GENEVE_PAL_TAG, FUNC(bus::ti99::internal::geneve_pal_device::vwaiten));
	m_tms9901->intreq_cb().set(FUNC(geneve_state::tms9901_interrupt));

	// PFM expansion: Select the 2^17 and 2^18 bit and the output enable
	m_tms9901->p_out_cb(4).set(FUNC(geneve_state::pfm_a17));
	m_tms9901->p_out_cb(5).set(FUNC(geneve_state::pfm_oe));
	m_tms9901->p_out_cb(13).set(FUNC(geneve_state::pfm_a18));

	// Clock
	MM58274C(config, GENEVE_CLOCK_TAG, 0).set_mode_and_day(1, 0); // 24h, sunday

	// PAL
	GENEVE_PAL(config, m_pal, 0);
	m_pal->ready_cb().set("maincpu", FUNC(tms9995_device::ready_line));

	// Sound hardware
	SPEAKER(config, "sound_out").front_center();
	SN76496(config, m_sound, XTAL(21'477'272)/6); // Delivered by the CLKOUT of the V9938
	m_sound->add_route(ALL_OUTPUTS, "sound_out", 0.75);
	m_sound->ready_cb().set(FUNC(geneve_state::sndready));

	// User interface devices: PC-style keyboard, joystick port, mouse connector
	PC_KBDC(config, m_kbdconn, geneve_xt_keyboards, STR_KBD_GENEVE_XT_101_HLE);
	m_kbdconn->out_clock_cb().set(GENEVE_GATE_ARRAY_TAG, FUNC(bus::ti99::internal::geneve_gate_array_device::kbdclk));
	m_kbdconn->out_data_cb().set(GENEVE_GATE_ARRAY_TAG, FUNC(bus::ti99::internal::geneve_gate_array_device::kbddata));

	TI99_JOYPORT(config, m_joyport, 0, ti99_joyport_options_plain, "twinjoy");
	V9938_COLORBUS(config, m_colorbus, 0, ti99_colorbus_options, nullptr);
	m_colorbus->extra_button_cb().set(FUNC(geneve_state::left_mouse_button));

	// PFM expansion
	AT29C040(config, GENEVE_PFM512_TAG);
	AT29C040A(config, GENEVE_PFM512A_TAG);

	// DRAM 512K
	RAM(config, GENEVE_DRAM_TAG).set_default_size("512K").set_default_value(0);

	// SRAM
	RAM(config, GENEVE_SRAM_TAG).set_default_size("32K").set_default_value(0);
	RAM(config, GENEVE_SRAMX_TAG).set_default_size("32K").set_default_value(0);

	// Ultimate SRAM expansion
	RAM(config, GENEVE_SRAMU_TAG).set_default_size("384K").set_default_value(0);
}

/*
    ROM loading
*/

ROM_START(geneve)
	/*CPU memory space*/
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_DEFAULT_BIOS("0.98")
	ROM_SYSTEM_BIOS(0, "0.98", "Geneve Boot ROM 0.98 (1987)")
	ROMX_LOAD("genbt098.bin", 0x0000, 0x4000, CRC(b2e20df9) SHA1(2d5d09177afe97d63ceb3ad59b498b1c9e2153f7), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "1.00", "Geneve Boot ROM 1.00 (1990)")
	ROMX_LOAD("genbt100.bin", 0x0000, 0x4000, CRC(8001e386) SHA1(b44618b54dabac3882543e18555d482b299e0109), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "2.00", "Geneve Boot ROM 2.00 (2021)")
	ROMX_LOAD("genbt200.bin", 0x0000, 0x4000, CRC(cc159fd6) SHA1(15d3bb48edb301364ecbd42025c4a2539cc3070d), ROM_BIOS(2))
ROM_END

ROM_START(genmod)
	/*CPU memory space*/
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_DEFAULT_BIOS("1.00")
	ROM_SYSTEM_BIOS(0, "1.00", "Geneve Mod Boot ROM 1.00 (1990)")
	ROMX_LOAD("gnmbt100.bin", 0x0000, 0x4000, CRC(19b89479) SHA1(6ef297eda78dc705946f6494e9d7e95e5216ec47), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "2.00", "Geneve Mod Boot ROM 2.00 (2021)")
	ROMX_LOAD("gnmbt200.bin", 0x0000, 0x4000, CRC(0a66c714) SHA1(139ed03d365b21123295cd99c73736ee424dbb74), ROM_BIOS(1))
ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT  COMPAT  MACHINE      INPUT   CLASS         INIT         COMPANY  FULLNAME       FLAGS
COMP( 1987, geneve, 0,      0,      geneve,      geneve, geneve_state, init_geneve, "Myarc", "Geneve 9640", MACHINE_SUPPORTS_SAVE)
COMP( 1990, genmod, 0,      0,      genmod,      genmod, geneve_state, init_genmod, "Myarc / Ron G. Walters", "Geneve 9640 Mod",  MACHINE_SUPPORTS_SAVE)

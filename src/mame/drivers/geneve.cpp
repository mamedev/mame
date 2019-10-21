// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/******************************************************************************
    Myarc Geneve 9640.

    The Geneve has two operation modes.  One is compatible with the TI-99/4a,
    the other is not.


    General architecture:

    TMS9995@12MHz (including 256 bytes of on-chip 16-bit RAM and a timer),
    V9938, SN76496 (compatible with TMS9919), TMS9901, MM58274 RTC, 512 kbytes
    of 1-wait-state CPU RAM (expandable to almost 2 Mbytes), 32 kbytes of
    0-wait-state CPU RAM (expandable to 64 kbytes), 128 kbytes of VRAM
    (expandable to 192 kbytes).


    Memory map:

    64-kbyte CPU address space is mapped to a 2-Mbyte virtual address space.
    8 8-kbyte pages are available simultaneously, out of a total of 256.

    Page map (regular console):
    >00->3f: 512kbytes of CPU RAM (pages >36 and >37 are used to emulate
      cartridge CPU ROMs in ti99 mode, and pages >38 through >3f are used to
      emulate console and cartridge GROMs in ti99 mode)
    >40->7f: optional Memex RAM (except >7a and >7c that are mirrors of >ba and
      >bc?)
    >80->b7: PE-bus space using spare address lines (AMA-AMC)?  Used by RAM
        extension (Memex or PE-Bus 512k card).
    >b8->bf: PE-bus space (most useful pages are >ba: DSR space and >bc: speech
      synthesizer page; other pages are used by RAM extension)
    >c0->e7: optional Memex RAM
    >e8->ef: 64kbytes of 0-wait-state RAM (with 32kbytes of SRAM installed,
      only even pages (>e8, >ea, >ec and >ee) are available)
    >f0->f1: boot ROM
    >f2->fe: optional Memex RAM? (except >fa and >fc that are mirrors of >ba
      and >bc?)
    >ff: always empty?

    Page map (genmod console):
    >00->39, >40->b9, >bb, >bd->ef, f2->fe: Memex RAM (except >3a, >3c, >7a,
      >7c, >fa and >fc that are mirrors of >ba and >bc?) (I don't know if
      >e8->ef is on the Memex board or the Geneve motherboard)
    >ba: DSR space
    >bc: speech synthesizer space
    >f0->f1: boot ROM
    >ff: always empty?

    >00->3f: switch-selectable(?): 512kbytes of onboard RAM (1-wait-state DRAM)
      or first 512kbytes of the Memex Memory board (0-wait-state SRAM).  The
      latter setting is incompatible with TI mode.

    Note that >bc is actually equivalent to >8000->9fff on the /4a bus,
    therefore the speech synthesizer is only available at offsets >1800->1bff
    (read port) and >1c00->1fff (write port).  OTOH, if you installed a FORTI
    sound card, it will be available in the same page at offsets >0400->7ff.


    Unpaged locations (ti99 mode):
    >8000->8007: memory page registers (>8000 for page 0, >8001 for page 1,
      etc.  register >8003 is ignored (this page is hard-wired to >36->37).
    >8008: key buffer
    >8009->800f: ???
    >8010->801f: clock chip
    >8400->9fff: sound, VDP, speech, and GROM registers (according to one
      source, the GROM and sound registers are only available if page >3
      is mapped in at location >c000 (register 6).  I am not sure the Speech
      registers are implemented, though I guess they are.)
    Note that >8020->83ff is mapped to regular CPU RAM according to map
    register 4.

    Unpaged locations (native mode):
    >f100: VDP data port (read/write)
    >f102: VDP address/status port (read/write)
    >f104: VDP port 2
    >f106: VDP port 3
    >f108->f10f: VDP mirror (used by Barry Boone's converted Tomy cartridges)
    >f110->f117: memory page registers (>f110 for page 0, >f111 for page 1,
      etc.)
    >f118: key buffer
    >f119->f11f: ???
    >f120: sound chip
    >f121->f12f: ???
    >f130->f13f: clock chip

    Unpaged locations (tms9995 locations):
    >f000->f0fb and >fffc->ffff: tms9995 RAM
    >fffa->fffb: tms9995 timer register
    Note: accessing tms9995 locations will also read/write corresponding paged
      locations.


    GROM emulator:

    The GPL interface is accessible only in TI99 mode.  GPL data is fetched
    from pages >38->3f.  It uses a straight 16-bit address pointer.  The
    address pointer is incremented when the read data and write data ports
    are accessed, and when the second byte of the GPL address is written.  A
    weird consequence of this is the fact that GPL data is always off by one
    byte, i.e. GPL bytes 0, 1, 2... are stored in bytes 1, 2, 3... of pages
    >38->3f (byte 0 of page >38 corresponds to GPL address >ffff).

    I think that I have once read that the geneve GROM emulator does not
    emulate wrap-around within a GROM, i.e. address >1fff is followed by >2000
    (instead of >0000 with a real GROM).

    There are two ways to implement GPL address load and store.
    One is maintaining 2 flags (one for address read and one for address write)
    that tell if you are accessing address LSB: these flags must be cleared
    whenever data is read, and the read flag must be cleared when the write
    address port is written to.
    The other is to shift the register and always read/write the MSByte of the
    address pointer.  The LSByte is copied to the MSbyte when the address is
    read, whereas the former MSByte is copied to the LSByte when the address is
    written.  The address pointer must be incremented after the address is
    written to.  It will not harm if it is incremented after the address is
    read, provided the LSByte has been cleared.


    Cartridge emulator:

    The cartridge interface is in the >6000->7fff area.

    If CRU bit @>F7C is set, the cartridge area is always mapped to virtual
    page >36.  Writes in the >6000->6fff area are ignored if the CRU bit @>F7D
    is 0, whereas writes in the >7000->7fff area are ignored if the CRU bit
    @>F7E is 0.

    If CRU bit @>F7C is clear, the cartridge area is mapped either to virtual
    page >36 or to page >37 according to which page is currently selected.
    Writing data to address >6000->7fff will select page >36 if A14 is 0,
    >37 if A14 is 1.


    CRU map:

    Base >0000: tms9901
    tms9901 pin assignment:
    int1: external interrupt (INTA on PE-bus) (connected to tms9995 (int4/EC)*
      pin as well)
    int2: VDP interrupt
    int3-int7: joystick port inputs (fire, left, right, down, up)
    int8: keyboard interrupt (asserted when key buffer full)
    int9/p13: unused
    int10/p12: third mouse button
    int11/p11: clock interrupt?
    int12/p10: INTB from PE-bus
    int13/p9: (used as output)
    int14/p8: unused
    int15/p7: (used as output)
    p0: PE-bus reset line
    p1: VDP reset line
    p2: joystick select (0 for joystick 1, 1 for joystick 2)
    p3-p5: unused
    p6: keyboard reset line
    p7/int15: external mem cycles 0=long, 1=short
    p9/int13: vdp wait cycles 1=add 15 cycles, 0=add none

    Base >1EE0 (>0F70): tms9995 flags and geneve mode register
    bits 0-1: tms9995 flags
    Bits 2-4: tms9995 interrupt state register
    Bits 5-15: tms9995 user flags - overlaps geneve mode, but hopefully the
      geneve registers are write-only, so no real conflict happens
    TMS9995 user flags:
    Bit 5: 0 if NTSC, 1 if PAL video
    Bit 6: unused???
    Bit 7: some keyboard flag, set to 1 if caps is on
    Geneve gate array + TMS9995 user flags:
    Bit 8: 1 = allow keyboard clock
    Bit 9: 0 = clear keyboard input buffer, 1 = allow keyboard buffer to be
      loaded
    Bit 10: 1 = geneve mode, 0 = ti99 mode
    Bit 11: 1 = ROM mode, 0 = map mode
    Bit 12: 0 = Enable cartridge paging
    Bit 13: 0 = protect cartridge range >6000->6fff
    Bit 14: 0 = protect cartridge range >7000->7fff
    bit 15: 1 = add 1 extra wait state when accessing 0-wait-state SRAM???

    Original version 2003 by Raphael Nabet

    Rewritten 2012 by Michael Zapf
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

#define LOG_WARN    (1U<<1)
#define LOG_READY   (1U<<2)
#define LOG_LINES   (1U<<3)
#define LOG_CRU     (1U<<4)
#define LOG_CRUKEY  (1U<<5)

// Minimum log should be settings and warnings
#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"


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
		m_tms9901(*this, TI_TMS9901_TAG),
		m_gatearray(*this, GENEVE_GATE_ARRAY_TAG),
		m_peribox(*this, TI_PERIBOX_TAG),
		m_joyport(*this, TI_JOYPORT_TAG),
		m_colorbus(*this, COLORBUS_TAG),
		m_kbdconn(*this, GENEVE_KEYBOARD_CONN_TAG),
		m_left_button(0)
	{
	}

	void geneve_common(machine_config &config);
	void geneve(machine_config &config);
	void genmod(machine_config &config);

	void init_geneve();

private:
	// CRU (Communication Register Unit) handling
	uint8_t cruread(offs_t offset);
	void cruwrite(offs_t offset, uint8_t data);

	// Connections with the system interface TMS9901
	uint8_t psi_input(offs_t offset);
	DECLARE_WRITE_LINE_MEMBER(peripheral_bus_reset);
	DECLARE_WRITE_LINE_MEMBER(VDP_reset);
	DECLARE_WRITE_LINE_MEMBER(joystick_select);
	DECLARE_WRITE_LINE_MEMBER(extbus_wait_states);
	DECLARE_WRITE_LINE_MEMBER(keyboard_reset);
	DECLARE_WRITE_LINE_MEMBER(video_wait_states);
	DECLARE_WRITE_LINE_MEMBER(left_mouse_button);

	DECLARE_WRITE_LINE_MEMBER(keyboard_clock_line);
	DECLARE_WRITE_LINE_MEMBER(keyboard_data_line);

	DECLARE_WRITE_LINE_MEMBER(clock_out);

	void external_operation(offs_t offset, uint8_t data);

	void tms9901_interrupt(offs_t offset, uint8_t data);

	DECLARE_WRITE_LINE_MEMBER( keyboard_interrupt );

	required_device<tms9995_device>         m_cpu;
	required_device<tms9901_device>         m_tms9901;
	required_device<bus::ti99::internal::geneve_gate_array_device>    m_gatearray;
	required_device<bus::ti99::peb::peribox_device>               m_peribox;
	required_device<bus::ti99::joyport::joyport_device>           m_joyport;
	required_device<bus::ti99::colorbus::v9938_colorbus_device>   m_colorbus;
	required_device<pc_kbdc_device>   m_kbdconn;

	DECLARE_WRITE_LINE_MEMBER( inta );
	DECLARE_WRITE_LINE_MEMBER( intb );
	DECLARE_WRITE_LINE_MEMBER( ext_ready );
	DECLARE_WRITE_LINE_MEMBER( mapper_ready );
	DECLARE_WRITE_LINE_MEMBER( keyboard_int );

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_WRITE_LINE_MEMBER(set_tms9901_INT2_from_v9938);

	int  m_inta;
	int  m_intb;
	int  m_int2;
	int  m_keyint;
	int  m_video_wait; // reflects the line to the mapper for CRU query

	int m_ready_line;
	int m_ready_line1;

	int m_left_button;

	void crumap(address_map &map);
	void memmap(address_map &map);
	void memmap_setaddress(address_map &map);
};

/*
    Memory map
*/

void geneve_state::memmap(address_map &map)
{
	map(0x0000, 0xffff).rw(GENEVE_GATE_ARRAY_TAG, FUNC(bus::ti99::internal::geneve_gate_array_device::readm), FUNC(bus::ti99::internal::geneve_gate_array_device::writem));
}

void geneve_state::memmap_setaddress(address_map &map)
{
	map(0x0000, 0xffff).w(GENEVE_GATE_ARRAY_TAG, FUNC(bus::ti99::internal::geneve_gate_array_device::setaddress));
}

/*
    CRU map
    The TMS9901 is fully decoded, no mirroring, so we have 32 bits for it,
    and the rest goes to the board (and from there to the PEB)
    TMS9995 has a full 15-bit CRU bit address space (attached to A0-A14)
    TODO: Check whether A0-A2 are available for CRU addressing since those
    bits are usually routed through the mapper first.
*/
void geneve_state::crumap(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(geneve_state::cruread), FUNC(geneve_state::cruwrite));
	map(0x0000, 0x003f).rw(m_tms9901, FUNC(tms9901_device::read), FUNC(tms9901_device::write));
}

static INPUT_PORTS_START(geneve_common)

	PORT_START( "BOOTROM" )
	PORT_CONFNAME( 0x03, GENEVE_EPROM, "Boot from" ) PORT_CHANGED_MEMBER(GENEVE_GATE_ARRAY_TAG, bus::ti99::internal::geneve_gate_array_device, settings_changed, 3)
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
	PORT_CONFNAME( 0x03, 0x01, "Onboard SRAM" )
		PORT_CONFSETTING( 0x00, "32 KiB" )
		PORT_CONFSETTING( 0x01, "64 KiB" )
		PORT_CONFSETTING( 0x02, "384 KiB" )

INPUT_PORTS_END

static INPUT_PORTS_START(genmod)
	PORT_INCLUDE(geneve_common)

	PORT_START( "GENMODDIPS" )
	PORT_DIPNAME( GENEVE_GM_TURBO, 0x00, "Genmod Turbo mode") PORT_CHANGED_MEMBER(GENEVE_GATE_ARRAY_TAG, bus::ti99::internal::genmod_gate_array_device, setgm_changed, 1)
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( GENEVE_GM_TURBO, DEF_STR( On ))
	PORT_DIPNAME( GENEVE_GM_TIM, GENEVE_GM_TIM, "Genmod TI mode") PORT_CHANGED_MEMBER(GENEVE_GATE_ARRAY_TAG, bus::ti99::internal::genmod_gate_array_device, setgm_changed, 2)
		PORT_CONFSETTING( 0x00, DEF_STR( Off ))
		PORT_CONFSETTING( GENEVE_GM_TIM, DEF_STR( On ))

INPUT_PORTS_END


/****************************************************************************
    CRU handling
*****************************************************************************/

#define CRU_CONTROL_BASE 0x1ee0
#define CRU_SSTEP_BASE 0x13c0

void geneve_state::cruwrite(offs_t offset, uint8_t data)
{
	int addroff = offset << 1;

	// Single step
	// 13c0 - 13fe: 0001 0011 11xx xxx0
	if ((addroff & 0xffc0) == CRU_SSTEP_BASE)
	{
		int bit = (addroff & 0x003e)>>1;
		LOGMASKED(LOG_WARN, "Single step not implemented; bit %d set to %d\n", bit, data);
		return;
	}

	// This is just mirroring the internal flags of the 9995
	if ((addroff & 0xffe0) == CRU_CONTROL_BASE)
	{
		int bit = (addroff & 0x001e)>>1;
		switch (bit)
		{
		case 5:
			// No one really cares...
			LOGMASKED(LOG_CRU, "Set PAL flag = %02x\n", data);
			// m_palvideo = (data!=0);
			break;
		case 7:
			// m_capslock = (data!=0);
			LOGMASKED(LOG_CRU, "Set capslock flag = %02x\n", data);
			break;
		case 8:
			LOGMASKED(LOG_CRUKEY, "Set keyboard clock = %02x\n", data);
			m_gatearray->set_keyboard_clock(data);
			break;
		case 9:
			LOGMASKED(LOG_CRUKEY, "Enable keyboard shift reg = %02x\n", data);
			m_gatearray->enable_shift_register(data);
			break;
		case 10:
			LOGMASKED(LOG_CRU, "Geneve mode = %02x\n", data);
			m_gatearray->set_geneve_mode(data!=0);
			break;
		case 11:
			LOGMASKED(LOG_CRU, "Direct mode = %02x\n", data);
			m_gatearray->set_direct_mode(data!=0);
			break;
		case 12:
			LOGMASKED(LOG_CRU, "Cartridge size 8K = %02x\n", data);
			m_gatearray->set_cartridge_size((data!=0)? 0x2000 : 0x4000);
			break;
		case 13:
			LOGMASKED(LOG_CRU, "Cartridge writable 6000 = %02x\n", data);
			m_gatearray->set_cartridge_writable(0x6000, (data!=0));
			break;
		case 14:
			LOGMASKED(LOG_CRU, "Cartridge writable 7000 = %02x\n", data);
			m_gatearray->set_cartridge_writable(0x7000, (data!=0));
			break;
		case 15:
			LOGMASKED(LOG_CRU, "Extra wait states = %02x\n", data==0);
			m_gatearray->set_extra_waitstates(data==0);  // let's use the inverse semantics
			break;
		default:
			LOGMASKED(LOG_WARN, "set CRU address %04x=%02x ignored\n", addroff, data);
			break;
		}
	}
	else
	{
		m_peribox->cruwrite(addroff, data);
	}
}

uint8_t geneve_state::cruread(offs_t offset)
{
	uint8_t value = 0;
	uint16_t addroff = offset << 1;

	// Single step
	// 13c0 - 13fe: 0001 0011 11xx xxx0
	if ((addroff & 0xffc0) == CRU_SSTEP_BASE)
	{
		int bit = (addroff & 0x003e)>>1;
		LOGMASKED(LOG_WARN, "Single step not implemented; attempting to read bit %d\n", bit);
		return value;
	}

	// TMS9995-internal CRU locations (1ee0-1efe) are handled within the processor
	// so we just don't arrive here

	// Propagate the CRU access to external devices
	m_peribox->crureadz(addroff, &value);
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

WRITE_LINE_MEMBER( geneve_state::left_mouse_button )
{
	m_left_button = state;
}

/*
    Write PE bus reset line
*/
WRITE_LINE_MEMBER( geneve_state::peripheral_bus_reset )
{
	LOGMASKED(LOG_WARN, "Peripheral bus reset request; not implemented yet.\n");
}

/*
    Write VDP reset line
*/
WRITE_LINE_MEMBER( geneve_state::VDP_reset )
{
	LOGMASKED(LOG_WARN, "Video reset request; not implemented yet.\n");
}

/*
    Write joystick select line. 1 selects joystick 1 (pin 7), 0 selects joystick 2 (pin 2)
*/
WRITE_LINE_MEMBER( geneve_state::joystick_select )
{
	m_joyport->write_port((state==ASSERT_LINE)? 1:2);
}


/*
    Write external mem cycles (0=long, 1=short)
*/
WRITE_LINE_MEMBER( geneve_state::extbus_wait_states )
{
	LOGMASKED(LOG_WARN, "External bus wait states set to %d, not implemented yet.\n", state);
}

/*
    Write vdp wait cycles (1=add 14 cycles, 0=add none)
    see above for waitstate handling
*/
WRITE_LINE_MEMBER( geneve_state::video_wait_states )
{
	LOGMASKED(LOG_LINES, "Video wait states set to %d\n", state);
	m_gatearray->set_video_waitstates(state==ASSERT_LINE);
	m_video_wait = (state!=0)? ASSERT_LINE : CLEAR_LINE;
}

/*
   Keyboard reset (active low).
*/
WRITE_LINE_MEMBER( geneve_state::keyboard_reset )
{
	LOGMASKED(LOG_CRUKEY, "Keyboard reset %d\n", state);
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
WRITE_LINE_MEMBER( geneve_state::inta )
{
	m_inta = (state!=0)? ASSERT_LINE : CLEAR_LINE;
	m_tms9901->set_int_line(1, state);
	m_cpu->set_input_line(INT_9995_INT4, state);
}

/*
    intb is connected to tms9901 IRQ12 line.
*/
WRITE_LINE_MEMBER( geneve_state::intb )
{
	m_intb = (state!=0)? ASSERT_LINE : CLEAR_LINE;
	m_tms9901->set_int_line(12, state);
}

WRITE_LINE_MEMBER( geneve_state::ext_ready )
{
	LOGMASKED(LOG_READY, "READY level (ext) = %02x\n", state);
	m_ready_line = state;
	m_cpu->ready_line((m_ready_line == ASSERT_LINE && m_ready_line1 == ASSERT_LINE)? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( geneve_state::mapper_ready )
{
	LOGMASKED(LOG_READY, "READY level (mapper) = %02x\n", state);
	m_ready_line1 = state;
	m_cpu->ready_line((m_ready_line == ASSERT_LINE && m_ready_line1 == ASSERT_LINE)? ASSERT_LINE : CLEAR_LINE);
}

/*
    set the state of int2 (called by the v9938 core)
*/
WRITE_LINE_MEMBER(geneve_state::set_tms9901_INT2_from_v9938)
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
WRITE_LINE_MEMBER( geneve_state::keyboard_interrupt )
{
	m_keyint = (state!=0)? ASSERT_LINE : CLEAR_LINE;
	m_tms9901->set_int_line(8, state);
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
WRITE_LINE_MEMBER( geneve_state::clock_out )
{
	m_tms9901->phi_line(state);
	m_gatearray->clock_in(state);
}

void geneve_state::init_geneve()
{
}

void geneve_state::machine_start()
{
	save_item(NAME(m_inta));
	save_item(NAME(m_intb));
	save_item(NAME(m_int2));
	save_item(NAME(m_keyint));
	save_item(NAME(m_video_wait)); // reflects the line to the mapper for CRU query
	save_item(NAME(m_ready_line));
	save_item(NAME(m_ready_line1));
	save_item(NAME(m_left_button));
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

	m_ready_line = m_ready_line1 = ASSERT_LINE;

	m_joyport->write_port(0x01);    // select Joystick 1
}

void geneve_state::geneve(machine_config &config)
{
	geneve_common(config);

	// Gate array
	GENEVE_GATE_ARRAY(config, m_gatearray, 0);
	m_gatearray->ready_cb().set(FUNC(geneve_state::mapper_ready));
	m_gatearray->kbdint_cb().set(FUNC(geneve_state::keyboard_interrupt));

	// Peripheral expansion box (Geneve composition)
	TI99_PERIBOX_GEN(config, m_peribox, 0);
	m_peribox->inta_cb().set(FUNC(geneve_state::inta));
	m_peribox->intb_cb().set(FUNC(geneve_state::intb));
	m_peribox->ready_cb().set(FUNC(geneve_state::ext_ready));
}

void geneve_state::genmod(machine_config &config)
{
	geneve_common(config);

	// Mapper
	GENMOD_GATE_ARRAY(config, m_gatearray, 0);
	m_gatearray->ready_cb().set(FUNC(geneve_state::mapper_ready));
	m_gatearray->kbdint_cb().set(FUNC(geneve_state::keyboard_interrupt));

	// Peripheral expansion box (Geneve composition with Genmod and plugged-in Memex)
	TI99_PERIBOX_GENMOD(config, m_peribox, 0);
	m_peribox->inta_cb().set(FUNC(geneve_state::inta));
	m_peribox->intb_cb().set(FUNC(geneve_state::intb));
	m_peribox->ready_cb().set(FUNC(geneve_state::ext_ready));
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
	v99x8_device& video(V9938(config, TI_VDP_TAG, XTAL(21'477'272))); // typical 9938 clock, not verified
	video.set_vram_size(0x20000);
	video.int_cb().set(FUNC(geneve_state::set_tms9901_INT2_from_v9938));
	video.set_screen(TI_SCREEN_TAG);
	screen_device& screen(SCREEN(config, TI_SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(21'477'272), \
		v99x8_device::HTOTAL, \
		0, \
		v99x8_device::HVISIBLE - 1, \
		v99x8_device::VTOTAL_NTSC * 2, \
		v99x8_device::VERTICAL_ADJUST * 2, \
		v99x8_device::VVISIBLE_NTSC * 2 - 1 - v99x8_device::VERTICAL_ADJUST * 2);
	screen.set_screen_update(TI_VDP_TAG, FUNC(v99x8_device::screen_update));

	// Main board components
	TMS9901(config, m_tms9901, 0);
	m_tms9901->read_cb().set(FUNC(geneve_state::psi_input));
	m_tms9901->p_out_cb(0).set(FUNC(geneve_state::peripheral_bus_reset));
	m_tms9901->p_out_cb(1).set(FUNC(geneve_state::VDP_reset));
	m_tms9901->p_out_cb(2).set(FUNC(geneve_state::joystick_select));
	m_tms9901->p_out_cb(4).set(GENEVE_GATE_ARRAY_TAG, FUNC(bus::ti99::internal::geneve_gate_array_device::pfm_select_lsb));
	m_tms9901->p_out_cb(5).set(GENEVE_GATE_ARRAY_TAG, FUNC(bus::ti99::internal::geneve_gate_array_device::pfm_output_enable));
	m_tms9901->p_out_cb(6).set(FUNC(geneve_state::keyboard_reset));
	m_tms9901->p_out_cb(7).set(FUNC(geneve_state::extbus_wait_states));
	m_tms9901->p_out_cb(9).set(FUNC(geneve_state::video_wait_states));
	m_tms9901->p_out_cb(13).set(GENEVE_GATE_ARRAY_TAG, FUNC(bus::ti99::internal::geneve_gate_array_device::pfm_select_msb));
	m_tms9901->intreq_cb().set(FUNC(geneve_state::tms9901_interrupt));

	// Clock
	MM58274C(config, GENEVE_CLOCK_TAG, 0).set_mode_and_day(1, 0); // 24h, sunday

	// Sound hardware
	SPEAKER(config, "sound_out").front_center();
	sn76496_device& soundgen(SN76496(config, TI_SOUNDCHIP_TAG, 3579545));
	soundgen.ready_cb().set(FUNC(geneve_state::ext_ready));
	soundgen.add_route(ALL_OUTPUTS, "sound_out", 0.75);

	// User interface devices: PC-style keyboard, joystick port, mouse connector
	PC_KBDC(config, m_kbdconn, 0);
	PC_KBDC_SLOT(config, "kbd", geneve_xt_keyboards, STR_KBD_GENEVE_XT_101_HLE).set_pc_kbdc_slot(m_kbdconn);
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

	// SRAM 384K (max; stock Geneve: 32K, but later MDOS releases require 64K)
	RAM(config, GENEVE_SRAM_TAG).set_default_size("384K").set_default_value(0);
}

/*
    ROM loading
*/

ROM_START(geneve)
	/*CPU memory space*/
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_DEFAULT_BIOS("0.98")
	ROM_SYSTEM_BIOS(0, "0.98", "Geneve Boot ROM 0.98")
	ROMX_LOAD("genbt098.bin", 0x0000, 0x4000, CRC(b2e20df9) SHA1(2d5d09177afe97d63ceb3ad59b498b1c9e2153f7), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "1.00", "Geneve Boot ROM 1.00")
	ROMX_LOAD("genbt100.bin", 0x0000, 0x4000, CRC(8001e386) SHA1(b44618b54dabac3882543e18555d482b299e0109), ROM_BIOS(1))
ROM_END

ROM_START(genmod)
	/*CPU memory space*/
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("gnmbt100.bin", 0x0000, 0x4000, CRC(19b89479) SHA1(6ef297eda78dc705946f6494e9d7e95e5216ec47))
ROM_END

//    YEAR  NAME    PARENT  COMPAT  MACHINE      INPUT   CLASS         INIT         COMPANY  FULLNAME       FLAGS
COMP( 1987, geneve, 0,      0,      geneve,      geneve, geneve_state, init_geneve, "Myarc", "Geneve 9640", MACHINE_SUPPORTS_SAVE)
COMP( 1990, genmod, 0,      0,      genmod,      genmod, geneve_state, init_geneve, "Myarc / Ron G. Walters", "Geneve 9640 Mod",  MACHINE_SUPPORTS_SAVE)

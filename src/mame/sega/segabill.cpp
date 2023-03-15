// license: BSD-3-Clause
// copyright-holders: Dirk Best
// thanks-to: Rob Krum
/***************************************************************************

    Sega Billboard

    TODO: Timing, vs298 needs a higher interrupt frequency, but then
    the animations seem to fast?
    Document and add support for Blast City Billboard with EPR-19158.IC3
    ( https://www.arcade-projects.com/forums/index.php?attachment/1477-blast-city-billboard-pcb-jpg/ )

***************************************************************************/

/*
Sega VERSUS CITY BILLBOARD CONTROL BD 837-11854 (C) 1991
( https://www.arcade-projects.com/forums/index.php?attachment/7044-vs-pcb-top-jpg/ )
|-----------------------------------------------------------------------------|
|      |---------------|  |---------------|    |---------------------------|  |
|--    |---------------|  |---------------|    |---------------------------|  |
|||S          CN5                CN4                        CN3               |
|||W                        PS2501  PS2501        M54583P   M54583P   M54583P |
|||1                                                        M54583P   M54583P |
|--      74F74N  M74LS393P  HD74HC4040P   315-5338A                           |
|                                                                             |
|     z                                                                    -- |
|     8  M27C512                   32.000MHz                              C|| |
|     0           LH5268AD                                                N|| |
|                                                                         2|| |
|                     GAL16V8B         SN74LS32N SN74LS04N MB3771          -- |
|    LED                                                                      |
|-----------------------------------------------------------------------------|
   Setup:
    Z80         ZILOG Z0840008PSC Z80 CPU (DIP40)
    LH5268A     SHARP LH5268AD-10LL 8k x8 SRAM (DIP28, labeled as MB8464A-15LL on PCB silkscreen)
    PS2501      NEC PS2501-4 ISOLATOR (DIP16)
    M54583P     MITSUBISHI M54583P DARLINGTON TRANSISTOR ARRAY (DIP18)
    74F74N      PHILLIPS 74F74N DUAL D-TYPE FLIP-FLIP (DIP14)
    M74LS393P   MITSUBISHI M74LS393P DUAL BINARY COUNTER (DIP14)
    HD74HC4040P RENESAS HD74HC4040P 12-STAGE BINARY COUNTER (DIP16)
    315-5338A   SEGA CUSTOM (QFP100)
    M27C512     ST MICROELECTRONICS M27C512-15F1 (DIP28, labeled 'EPR-18022', socketed)
    GAL16V8B    LATTICE GAL16V8B-25LP HIGH PERFORMANCE E2CMOS PLD GENERIC ARRAY LOGIC (DIP28, socketed)
    SN74LS32N   MOTOROLA SN74LS32N QUAD 2-INPUT OR GATE (DIP14)
    SN74LS04N   MOTOROLA SN74LS04N HEX INVERTER (DIP14)
    MB3771      FUJITSU MB3771 POWER SUPPLY VOLTAGE MONITOR (SOP8)
    Board:
        837-11854  Sega VERSUS CITY BILLBOARD CONTROL BD
    EEPROM:
        EPR-18022.IC2    Sega VERSUS CITY BILLBOARD
    Notes:
        SW1     8-DIP switch
        CN2     10 pin connector for power input (5v)
        CN3 J50 pin connector for two 7-SEG UNITs (VSC-0220, connects at VSC-0221 7SEG BASE)
    CN4 J20 pin connector to BLUE(U-P) PANEL
        (600-6770-005 WIRE HARN BILLBOARD FLT BD, CN7(?) and CN11 for Model3, CN6 and CN12 for Model2)
        CN5 has no connector soldered and as such is unused and has two silkscreened PS2501-4 ICs below,
        similar to the ICs under CN4
        171-6218B is printed to right of (C) SEGA 1991 on bottom of board
*/
#include "emu.h"
#include "segabill.h"
#include "cpu/z80/z80.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SEGA_BILLBOARD, sega_billboard_device, "segabill", "Sega Billboard")

//-------------------------------------------------
// mem_map - z80 memory map
//-------------------------------------------------

void sega_billboard_device::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xe000, 0xffff).ram();
}

//-------------------------------------------------
// io_map - z80 io map
//-------------------------------------------------

void sega_billboard_device::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x20, 0x2f).rw("io", FUNC(sega_315_5338a_device::read), FUNC(sega_315_5338a_device::write));
}

//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

static INPUT_PORTS_START( billboard )
	PORT_START("dsw")
	PORT_DIPNAME(0x01, 0x01, "Test Winner LED P1")
	PORT_DIPSETTING(   0x01, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPLOCATION("DSW:1")
	PORT_DIPNAME(0x02, 0x02, "Test Winner LED P2")
	PORT_DIPSETTING(   0x02, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPLOCATION("DSW:2")
	PORT_DIPNAME(0x04, 0x04, "Test 7-Segment P1")
	PORT_DIPSETTING(   0x04, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPLOCATION("DSW:3")
	PORT_DIPNAME(0x08, 0x08, "Test 7-Segment P2")
	PORT_DIPSETTING(   0x08, DEF_STR( Off ))
	PORT_DIPSETTING(   0x00, DEF_STR( On ))
	PORT_DIPLOCATION("DSW:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW:6")
	PORT_DIPNAME(0x40, 0x00, "Demo")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x40, DEF_STR( On ))
	PORT_DIPLOCATION("DSW:7")
	PORT_DIPNAME(0x80, 0x00, "Testmode")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x80, DEF_STR( On ))
	PORT_DIPLOCATION("DSW:8")
INPUT_PORTS_END

ioport_constructor sega_billboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(billboard);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( billboard )
	ROM_REGION(0x10000, "billcpu", 0)
	ROM_LOAD("epr-18022.ic2", 0x00000, 0x10000, CRC(0ca70f80) SHA1(edf5ade72d9fa2f4d5f83f9f89e6cecfadd77f56))
ROM_END

const tiny_rom_entry *sega_billboard_device::device_rom_region() const
{
	return ROM_NAME(billboard);
}

//-------------------------------------------------
// device_add_mconfig - add device configuration
//-------------------------------------------------

void sega_billboard_device::device_add_mconfig(machine_config &config)
{
	Z80(config, m_billcpu, 32_MHz_XTAL / 4); // Z0840008PSC 8MHz rated part - not verified
	m_billcpu->set_addrmap(AS_PROGRAM, &sega_billboard_device::mem_map);
	m_billcpu->set_addrmap(AS_IO, &sega_billboard_device::io_map);
	m_billcpu->set_periodic_int(FUNC(sega_billboard_device::irq0_line_hold), attotime::from_hz(32_MHz_XTAL/65536)); // timing?

	sega_315_5338a_device &io(SEGA_315_5338A(config, "io", 32_MHz_XTAL));
	io.in_pa_callback().set_ioport("dsw");
	io.in_pb_callback().set(FUNC(sega_billboard_device::cmd_r));
	io.out_pc_callback().set(FUNC(sega_billboard_device::digit_w<1>));
	io.out_pd_callback().set(FUNC(sega_billboard_device::digit_w<0>));
	io.out_pe_callback().set(FUNC(sega_billboard_device::digit_w<3>));
	io.out_pf_callback().set(FUNC(sega_billboard_device::digit_w<2>));
	io.out_pg_callback().set(FUNC(sega_billboard_device::led_w));
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sega_billboard_device - constructor
//-------------------------------------------------

sega_billboard_device::sega_billboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEGA_BILLBOARD, tag, owner, clock),
	m_billcpu(*this, "billcpu"),
	m_io(*this, "io"),
	m_digits(*this, "digit%u", 0U),
	m_leds(*this, "led_winner%u", 0U),
	m_cmd(0xff)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sega_billboard_device::device_start()
{
	// resolve output finders
	m_digits.resolve();
	m_leds.resolve();

	// register for save states
	save_item(NAME(m_cmd));
}

//-------------------------------------------------
//  device_start - device-specific reset
//-------------------------------------------------

void sega_billboard_device::device_reset()
{
	m_cmd = 0xff;
}


//**************************************************************************
//  INTERFACE
//**************************************************************************

void sega_billboard_device::irq0_line_hold(device_t &device)
{
	m_billcpu->set_input_line(INPUT_LINE_IRQ0, HOLD_LINE);
}

void sega_billboard_device::write(uint8_t data)
{
	m_cmd = data;
}

uint8_t sega_billboard_device::cmd_r()
{
	return m_cmd;
}

template<int N>
void sega_billboard_device::digit_w(uint8_t data)
{
	m_digits[N] = ~data & 0xff;
}

void sega_billboard_device::led_w(uint8_t data)
{
	m_leds[0] = BIT(~data, 0);
	m_leds[1] = BIT(~data, 1);
}

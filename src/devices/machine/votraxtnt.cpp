// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
*
*  Votrax Type 'N Talk
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  with loads of help (dumping, code disassembly, schematics, and other
*  documentation) from Kevin 'kevtris' Horton
*
*  The Votrax TNT was sold from some time in early 1981 until at least 1983.
*
*    ACIA status code is set to E2 for no data and E3 for data.
*
*    On the CPU, A12 and A15 are not connected. A13 and A14 are used
*    to select devices. A0-A9 address RAM. A0-A11 address ROM.
*    A0 switches the ACIA between status/command, and data in/out.
*
******************************************************************************/

#include "emu.h"
#include "votraxtnt.h"
#include "speaker.h"


DEFINE_DEVICE_TYPE(VOTRAXTNT, votraxtnt_device, "votraxtnt", "Votrax Type 'N Talk")


votraxtnt_device::votraxtnt_device(const machine_config &mconfig, const char* tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, VOTRAXTNT, tag, owner, clock)
	, m_maincpu(*this, "maincpu")
	, m_votrax(*this, "votrax")
	, m_acia(*this, "acia")
	, m_clock(*this, "acia_clock")
	, m_txd_handler(*this)
	, m_rts_handler(*this)
{
}


/******************************************************************************
 Address Maps
******************************************************************************/

/*  a15 a14 a13 a12   a11 a10  a9  a8    a7  a6  a5  a4    a3  a2  a1  a0
      x   0   0   x     x   x   *   *     *   *   *   *     *   *   *   *    RW RAM (2x 2114 1kx4 SRAM, wired in parallel)
      x   0   1   x     x   x   x   x     x   x   x   x     x   x   x   0    RW 6850 Status(R)/Control(W)
      x   0   1   x     x   x   x   x     x   x   x   x     x   x   x   1    RW 6850 Data(R)/Data(W)
      x   1   0   x     x   x   x   x     x   x   x   x     x   x   x   x    W  SC-01 Data(W)
                                                                                low 6 bits write to 6 bit input of sc-01-a;
                                                                                high 2 bits are ignored (but by adding a buffer chip could be made to control
                                                                                the inflection bits of the sc-01-a which are normally grounded on the TNT);
                                                                                upon any access to this area (even reads, which count effectively as a 'write
                                                                                of open bus value') the /STB line of the sc-01 is pulsed low using a 74123 monostable
                                                                                multivibrator with a capacitor of 120pf and a resistor to vcc of 22Kohm
      x   1   1   x     *   *   *   *     *   *   *   *     *   *   *   *    R  ROM (2332 4kx8 Mask ROM, inside potted brick)
*/

void votraxtnt_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).mirror(0x9c00).ram(); /* RAM, 2114*2 (0x400 bytes) mirrored 4x */
	map(0x2000, 0x2001).mirror(0x9ffe).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x4000, 0x4000).mirror(0x9fff).w(m_votrax, FUNC(votrax_sc01_device::write));
	map(0x6000, 0x6fff).mirror(0x9000).rom().region("maincpu",0); /* ROM in potted block */
}


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------
/** TODO: actually hook this up to the ACIA */
static INPUT_PORTS_START(votraxtnt)
	PORT_START("DSW1")
	/* not connected to cpu, each switch is connected directly to the output
	   of a 4040 counter dividing the cpu m1? clock to feed the 6850 ACIA.
	   Setting more than one switch on (downward is on, upward is off) is a bad
	   idea, as it will short together outputs of the 4040, possibly damaging it.
	   see tnt_schematic.jpg */
	PORT_DIPNAME( 0xff, 0x80, "Baud Select" ) PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0x01, "75" )
	PORT_DIPSETTING(    0x02, "150" )
	PORT_DIPSETTING(    0x04, "300" )
	PORT_DIPSETTING(    0x08, "600" )
	PORT_DIPSETTING(    0x10, "1200" )
	PORT_DIPSETTING(    0x20, "2400" )
	PORT_DIPSETTING(    0x40, "4800" )
	PORT_DIPSETTING(    0x80, "9600" )
INPUT_PORTS_END

ioport_constructor votraxtnt_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(votraxtnt);
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START(votraxtnt)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("cn49752n.bin", 0x0000, 0x1000, CRC(a44e1af3) SHA1(af83b9e84f44c126b24ee754a22e34ca992a8d3d)) /* 2332 mask rom inside potted brick */
ROM_END

const tiny_rom_entry *votraxtnt_device::device_rom_region() const
{
	return ROM_NAME(votraxtnt);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void votraxtnt_device::device_reset()
{
	// Read the dips, whichever one is found to be on first is accepted
	u8 dips = ioport("DSW1")->read();
	u8 speed = 1;
	for (u8 i = 0; i < 7; i++)
	{
		if (BIT(dips, i))
		{
			m_clock->set_unscaled_clock(75*speed*16);
			return;
		}
		speed *= 2;
	}
	// if none are on we'll leave the default which is 9600 baud
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void votraxtnt_device::device_add_mconfig(machine_config &config)
{
	/* basic machine hardware */
	M6802(config, m_maincpu, 2.4576_MHz_XTAL);  // 2.4576MHz XTAL, verified; divided by 4 inside the MC6802
	m_maincpu->set_ram_enable(false);
	m_maincpu->set_addrmap(AS_PROGRAM, &votraxtnt_device::mem_map);

	/* serial hardware */
	acia6850_device &acia(ACIA6850(config, "acia"));
	acia.txd_handler().set([this](int state) { m_txd_handler(state); });
	acia.rts_handler().set([this](int state) { m_rts_handler(state); });

	CLOCK(config, m_clock, 153600);
	m_clock->signal_handler().set("acia", FUNC(acia6850_device::write_txc));
	m_clock->signal_handler().append("acia", FUNC(acia6850_device::write_rxc));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	VOTRAX_SC01A(config, m_votrax, 720000); // 720kHz? needs verify
	m_votrax->ar_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_votrax->add_route(ALL_OUTPUTS, "mono", 1.00);
}

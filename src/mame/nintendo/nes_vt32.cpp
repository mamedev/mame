// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  nes_vt32.cpp

  VT32

   - new 444 palette mode
   - backwards compatibility with mappers other than MMC3

 ***************************************************************************/

#include "emu.h"
#include "nes_vt369_vtunknown_soc.h"
#include "nes_vt32_soc.h"


namespace {

class nes_vt32_base_state : public driver_device
{
public:
	nes_vt32_base_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_io0(*this, "IO0"),
		m_io1(*this, "IO1"),
		m_cartsel(*this, "CARTSEL"),
		m_exin(*this, "EXTRAIN%u", 0U),
		m_prgrom(*this, "mainrom")
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint8_t in0_r();
	virtual uint8_t in1_r();
	virtual void in0_w(uint8_t data);

	void nes_vt32_map(address_map &map) ATTR_COLD;

	optional_ioport m_io0;
	optional_ioport m_io1;

	uint8_t m_latch0;
	uint8_t m_latch1;
	uint8_t m_previous_port0;

	optional_ioport m_cartsel;
	optional_ioport_array<4> m_exin;

	/* Misc */
	uint32_t m_ahigh; // external banking bits
	uint8_t m_4242;
	uint8_t m_411c;
	uint8_t m_411d;

	required_region_ptr<uint8_t> m_prgrom;

	uint8_t vt_rom_r(offs_t offset);
	[[maybe_unused]] void vtspace_w(offs_t offset, uint8_t data);

	void configure_soc(nes_vt02_vt03_soc_device* soc);

	uint8_t upper_412c_r();
	uint8_t upper_412d_r();
	void upper_412c_w(uint8_t data);

private:
	/* Extra IO */
	template <uint8_t NUM> uint8_t extrain_r();
};

class nes_vt32_state : public nes_vt32_base_state
{
public:
	nes_vt32_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt32_base_state(mconfig, type, tag),
		m_soc(*this, "soc")
	{ }

	void vt_external_space_map_32mbyte(address_map &map) ATTR_COLD;

protected:
	required_device<nes_vt02_vt03_soc_device> m_soc;
};

class nes_vt32_unk_state : public nes_vt32_state
{
public:
	nes_vt32_unk_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt32_state(mconfig, type, tag)
	{ }

	void nes_vt32_fp(machine_config& config);
	void nes_vt32_32mb(machine_config& config);
	void nes_vt32_4x16mb(machine_config& config);

	void nes_vt32_pal_32mb(machine_config& config);

private:
	uint8_t vt_rom_banked_r(offs_t offset);
	void vt_external_space_map_fp_2x32mbyte(address_map &map) ATTR_COLD;

	uint8_t fcpocket_412d_r();
	void fcpocket_412c_w(uint8_t data);
};

uint8_t nes_vt32_base_state::vt_rom_r(offs_t offset)
{
	return m_prgrom[offset];
}

void nes_vt32_base_state::vtspace_w(offs_t offset, uint8_t data)
{
	logerror("%s: vtspace_w %08x : %02x", machine().describe_context(), offset, data);
}

// VTxx can address 25-bit address space (32MB of ROM) so use maps with mirroring in depending on ROM size
void nes_vt32_state::vt_external_space_map_32mbyte(address_map &map)
{
	map(0x0000000, 0x1ffffff).r(FUNC(nes_vt32_state::vt_rom_r));
}


uint8_t nes_vt32_unk_state::vt_rom_banked_r(offs_t offset)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt32_unk_state::vt_external_space_map_fp_2x32mbyte(address_map &map)
{
	map(0x0000000, 0x1ffffff).r(FUNC(nes_vt32_unk_state::vt_rom_banked_r));
}


template <uint8_t NUM> uint8_t nes_vt32_base_state::extrain_r()
{
	if (m_exin[NUM])
		return m_exin[NUM]->read();
	else
	{
		logerror("%s: extrain_r (port %d) (not hooked up)\n", NUM, machine().describe_context());
	}
	return 0x00;
}


/* Standard I/O handlers (NES Controller clone) */

uint8_t nes_vt32_base_state::in0_r()
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch0 & 1;
	m_latch0 >>= 1;
	return ret;
}

uint8_t nes_vt32_base_state::in1_r()
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch1 & 1;
	m_latch1 >>= 1;
	return ret;
}

void nes_vt32_base_state::in0_w(uint8_t data)
{
	//logerror("%s: in0_w %02x\n", machine().describe_context(), data);
	if ((data & 0x01) != (m_previous_port0 & 0x01))
	{
		if (data & 0x01)
		{
			m_latch0 = m_io0->read();
			m_latch1 = m_io1->read();
		}
	}

	m_previous_port0 = data;
}


void nes_vt32_base_state::machine_start()
{
	m_latch0 = 0;
	m_latch1 = 0;
	m_previous_port0 = 0;

	m_ahigh = 0;
	m_4242 = 0;
	m_411c = 0;
	m_411d = 0;

	save_item(NAME(m_latch0));
	save_item(NAME(m_latch1));
	save_item(NAME(m_previous_port0));

	save_item(NAME(m_ahigh));
	save_item(NAME(m_4242));
	save_item(NAME(m_411c));
	save_item(NAME(m_411d));
}

void nes_vt32_base_state::machine_reset()
{

}

void nes_vt32_base_state::configure_soc(nes_vt02_vt03_soc_device* soc)
{
	soc->set_addrmap(AS_PROGRAM, &nes_vt32_state::vt_external_space_map_32mbyte);
	soc->read_0_callback().set(FUNC(nes_vt32_base_state::in0_r));
	soc->read_1_callback().set(FUNC(nes_vt32_base_state::in1_r));
	soc->write_0_callback().set(FUNC(nes_vt32_base_state::in0_w));

	soc->extra_read_0_callback().set(FUNC(nes_vt32_base_state::extrain_r<0>));
	soc->extra_read_1_callback().set(FUNC(nes_vt32_base_state::extrain_r<1>));
	soc->extra_read_2_callback().set(FUNC(nes_vt32_base_state::extrain_r<2>));
	soc->extra_read_3_callback().set(FUNC(nes_vt32_base_state::extrain_r<3>));
}

uint8_t nes_vt32_base_state::upper_412c_r()
{
	logerror("%s: upper_412c_r\n", machine().describe_context());
	return 0x00;
}

uint8_t nes_vt32_base_state::upper_412d_r()
{
	logerror("%s: upper_412d_r\n", machine().describe_context());
	return 0x00;
}

void nes_vt32_base_state::upper_412c_w(uint8_t data)
{
	logerror("%s: upper_412c_w %02x\n", machine().describe_context(), data);
}




static INPUT_PORTS_START( nes_vt32 )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY

	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
INPUT_PORTS_END

uint8_t nes_vt32_unk_state::fcpocket_412d_r()
{
	if (m_cartsel)
		return m_cartsel->read();
	else
		return 0;
}

void nes_vt32_unk_state::fcpocket_412c_w(uint8_t data)
{
	// fcpocket
	logerror("%s: vtfp_412c_extbank_w %02x\n", machine().describe_context(), data);
	m_ahigh = (data & 0x01) ? (1 << 25) : 0x0;
}

void nes_vt32_unk_state::nes_vt32_fp(machine_config &config)
{
	/* basic machine hardware */
	NES_VT32_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt32_unk_state::upper_412c_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412d_callback().set(FUNC(nes_vt32_unk_state::upper_412d_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt32_unk_state::upper_412c_w));

	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB12);
	m_soc->force_bad_dma();
}

void nes_vt32_unk_state::nes_vt32_pal_32mb(machine_config& config)
{
	/* basic machine hardware */
	NES_VT32_SOC_PAL(config, m_soc, NTSC_APU_CLOCK); // TODO, proper clocks etc. for PAL
	configure_soc(m_soc);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt32_unk_state::upper_412c_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412d_callback().set(FUNC(nes_vt32_unk_state::upper_412d_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt32_unk_state::upper_412c_w));

	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB12);
	m_soc->force_bad_dma();

	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_unk_state::vt_external_space_map_32mbyte);
}

void nes_vt32_unk_state::nes_vt32_4x16mb(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_unk_state::vt_external_space_map_fp_2x32mbyte);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt32_unk_state::fcpocket_412c_w));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412d_callback().set(FUNC(nes_vt32_unk_state::fcpocket_412d_r));
}

void nes_vt32_unk_state::nes_vt32_32mb(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_unk_state::vt_external_space_map_32mbyte);
}


static INPUT_PORTS_START( nes_vt32_fp )
	PORT_INCLUDE(nes_vt32)

	PORT_START("CARTSEL")
	PORT_DIPNAME( 0x06, 0x00, "Cartridge Select" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x00, "472-in-1" )
	PORT_DIPSETTING(    0x06, "128-in-1" )
INPUT_PORTS_END


ROM_START( dgun2573 ) // this one lacked a DreamGear logo but was otherwise physically identical, is it a clone product or did DreamGear drop the logo in favor of just using the 'My Arcade' brand?
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "myarcadegamerportable_s29gl256p10tfi01_0001227e.bin", 0x00000, 0x2000000, CRC(8f8c8da7) SHA1(76a18458922e39abe1982f05f184babb5e65acf2) )
ROM_END

ROM_START( dgun2573a )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "myarcadegamerportabledreamgear_s29gl256p10tfi01_0001227e.bin", 0x00000, 0x2000000, CRC(928c41ad) SHA1(c0119a13a47a5b784d0b834d1451973ff0b4a84f) )
ROM_END

ROM_START( rminitv )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "29gl256.bin", 0x00000, 0x2000000, CRC(cb4048d4) SHA1(9877ce5716d13f8498abfc1cbfaefa9426205d3e) )
ROM_END

ROM_START( dgunl3201 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "s29gl256.u2", 0x00000, 0x2000000,  CRC(8174088a) SHA1(4854d83a5657f3043b9568b1356e54c7f8282491) )
ROM_END

ROM_START( dgunl3202 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "dg308n1_s29gl256p90tfcr1_0001227e.bin", 0x00000, 0x2000000, CRC(489c806f) SHA1(979b2c00eec459646de5a658863aff0eaacc2402) )
	ROM_IGNORE(0x100)
ROM_END

ROM_START( fcpocket )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "s29gl01gp.bin", 0x00000, 0x8000000, CRC(8703b18a) SHA1(07943443294e80ca93f83181c8bdbf950b87c52f) ) // 2nd half = 0x00 (so 64MByte of content)
ROM_END

ROM_START( matet300 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "tetris.bin", 0x00000, 0x2000000, CRC(73cbd40a) SHA1(5996c97cebd6cec42a0ba1fba9517adf1af00098) )
ROM_END

} // anonymous namespace


CONS( 2015, dgun2573,  0,         0,  nes_vt32_32mb,     nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Gamer V Portable Gaming System (DGUN-2573) (set 1, newer)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2015, dgun2573a, dgun2573,  0,  nes_vt32_32mb,     nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Gamer V Portable Gaming System (DGUN-2573) (set 2, older)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // some menu graphics haven't been updated to reflect 'Panda' theme to the sports games

CONS( 2015, rminitv,   0,  0,  nes_vt32_pal_32mb, nes_vt32, nes_vt32_unk_state, empty_init, "Orb Gaming", "Retro 'Mini TV' Console 300-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // single 32Mbyte bank!

// This was available in at least 3 other form factors, some of those have been shown to use different menu backgrounds
// Gamestation Wireless : https://youtu.be/rlX-LGO-ewM Fish background
// Pixel Classic (DGUNL-3201) : https://youtu.be/XOUtT_wRXa4 Plane background, note, different revision, the copyright text on some games (eg. Heavy Barrel) hasn't been updated as it has on the dgunl3201 set here
// However, sometimes the different models use the same background as this one (confirmed on one Pixel Classic at least), so there doesn't appear to be a clear way of knowing without powering them on
CONS( 201?, dgunl3201, 0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Data East Classics - Pixel Classic (308-in-1) (DGUNL-3201)", MACHINE_NOT_WORKING ) // from a UK unit, single 32Mbyte bank!
CONS( 201?, dgunl3202, 0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Data East Classics - Pixel Player (308-in-1) (DGUNL-3202)", MACHINE_NOT_WORKING ) // from a US unit single 32Mbyte bank!
// There was also a 34-in-1 version of the Data East Classics in a mini-cabinet, NOT running on VT hardware, but using proper arcade ROMs, that one is reportedly running an old MAME build on an ARM SoC (although some sources say FBA)

CONS( 2021, matet300,  0,         0,  nes_vt32_32mb,     nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Tetris (DGUNL-7029, Go Gamer, with 300 bonus games)", MACHINE_NOT_WORKING )

// Use DIP switch to select console or cartridge, as cartridge is fake and just toggles a GPIO
CONS( 2016, fcpocket,  0,  0,  nes_vt32_4x16mb,   nes_vt32_fp, nes_vt32_unk_state, empty_init, "<unknown>",   "FC Pocket 600 in 1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )  // has external banking (2x 32mbyte banks)

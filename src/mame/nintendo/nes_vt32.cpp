// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  nes_vt32.cpp

  VT32

   - new 444 palette mode
   - backwards compatibility with mappers other than MMC3

 ***************************************************************************/

#include "emu.h"
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

	optional_ioport m_io0;
	optional_ioport m_io1;

	uint8_t m_latch0;
	uint8_t m_latch1;
	uint8_t m_previous_port0;

	optional_ioport m_cartsel;
	optional_ioport_array<4> m_exin;

	/* Misc */
	uint32_t m_ahigh; // external banking bits

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

	[[maybe_unused]] void vt_external_space_map_1mbyte(address_map &map) ATTR_COLD;
	[[maybe_unused]] void vt_external_space_map_2mbyte(address_map &map) ATTR_COLD;
	[[maybe_unused]] void vt_external_space_map_4mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_8mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_16mbyte(address_map &map) ATTR_COLD;
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
	void nes_vt32_8mb(machine_config& config);
	void nes_vt32_16mb(machine_config& config);
	void nes_vt32_32mb(machine_config& config);
	void nes_vt32_4x16mb(machine_config& config);

	void nes_vt32_pal_32mb(machine_config& config);

	void init_rfcp168();
	void init_g9_666();
	void init_hhgc319();

protected:
	uint8_t vt_rom_banked_r(offs_t offset);

private:
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
void nes_vt32_state::vt_external_space_map_1mbyte(address_map &map)
{
	map(0x0000000, 0x00fffff).mirror(0x1f00000).r(FUNC(nes_vt32_state::vt_rom_r));
}

void nes_vt32_state::vt_external_space_map_2mbyte(address_map &map)
{
	map(0x0000000, 0x01fffff).mirror(0x1e00000).r(FUNC(nes_vt32_state::vt_rom_r));
}

void nes_vt32_state::vt_external_space_map_4mbyte(address_map &map)
{
	map(0x0000000, 0x03fffff).mirror(0x1c00000).r(FUNC(nes_vt32_state::vt_rom_r));
}

void nes_vt32_state::vt_external_space_map_8mbyte(address_map &map)
{
	map(0x0000000, 0x07fffff).mirror(0x1800000).r(FUNC(nes_vt32_state::vt_rom_r));
}

void nes_vt32_state::vt_external_space_map_16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt32_state::vt_rom_r));
}

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

	save_item(NAME(m_latch0));
	save_item(NAME(m_latch1));
	save_item(NAME(m_previous_port0));

	save_item(NAME(m_ahigh));
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

// TODO: these should be in the SoC devices - upper_412d_r gets read, compared against, and another register written based on the result (maybe detecting SoC type?)
uint8_t nes_vt32_base_state::upper_412c_r() { logerror("%s: nes_vt32_base_state:upper_412c_r\n", machine().describe_context()); return 0x00; }
uint8_t nes_vt32_base_state::upper_412d_r() { logerror("%s: nes_vt32_base_state:upper_412d_r\n", machine().describe_context()); return 0x00; }
void nes_vt32_base_state::upper_412c_w(uint8_t data) { logerror("%s: nes_vt32_base_state:upper_412c_w %02x\n", machine().describe_context(), data); }


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

void nes_vt32_unk_state::nes_vt32_8mb(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_unk_state::vt_external_space_map_8mbyte);
}

void nes_vt32_unk_state::nes_vt32_16mb(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_unk_state::vt_external_space_map_16mbyte);
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

ROM_START( myaass )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "s29gl256.u2", 0x00000, 0x2000000, CRC(71a3298d) SHA1(5a2441ae5a8bf3e5efe9f22843ad2b8ef2df0f40) )
ROM_END

ROM_START( myaasa )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "mx29gl256el.u2", 0x00000, 0x2000000, CRC(1882264c) SHA1(e594b5cea634fadc4aac217b6d651be72a3024c0) )
ROM_END

ROM_START( mymman )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "megaman_s29gl064n90tfi04_0001227e.bin", 0x00000, 0x800000, CRC(1954cc95) SHA1(be20d42d32d625ec7b3c5db983850763c0ceff73) )
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

ROM_START( matet220 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "gamervtetris_s29gl128n10tfi01_0001227e.bin", 0x00000, 0x1000000, CRC(ac244e56) SHA1(89897f5f65f55a46bf0d6b5ca534ca31c79a0658) )
	ROM_IGNORE(0x100)
ROM_END

ROM_START( matet100 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "picotetris_s29gl064n90tfi04_0001227e.bin", 0x00000, 0x800000, CRC(7d9296f2) SHA1(0db5883028d14783d0abff1f7672e59534b0e513) )
	ROM_IGNORE(0x100)
ROM_END

ROM_START( lxpcsp )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "s29gl512n11tfi02.u2", 0x00000, 0x4000000, CRC(113e22f2) SHA1(c57184131db3f3c82d09d7757f0977223698f62c) )
ROM_END

ROM_START( lxpcli )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "s29gl512n11tfi02.u2", 0x00000, 0x4000000, CRC(9df963c6) SHA1(e5cc7b48c31b761bb74b3e5e1563a16a0cefa272) )
ROM_END

ROM_START( lxpcpp )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "s29gl512n11tfi02.u2", 0x00000, 0x4000000, CRC(75728b87) SHA1(4b6d8cfd19ea5160f5486a4bd72c8cc708716d2c) )
ROM_END

ROM_START( typo240 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "240nes.u2", 0x00000, 0x1000000, CRC(d709f66c) SHA1(73ca34ce07a1a8782226bd74b1ae43fc6d7126e1) ) // s29gl128p90tfcr1
ROM_END

ROM_START( retror30 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "s29gl032n90tfi03.u2", 0x00000, 0x400000, CRC(dfb89ef7) SHA1(401539b73521e018e3af70b8019e6b59ba67fcad) )
ROM_END

ROM_START( k10_5l )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "js28f128m29ewh.u4", 0x00000, 0x1000000, CRC(69dba082) SHA1(bd6829b0339795876dd5b4eb5de8bbd124c64f77) )
	ROM_IGNORE(0x100)
ROM_END

ROM_START( k10_2l )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "s29gl128n10tfi01.u4", 0x00000, 0x1000000, CRC(3ca75ab7) SHA1(a4e4f939c26b4a2f361261fa8b5303c6eeee9c41) )
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

// many of the games don't work or have scrambled graphics, it writes 0xc0 to vtfp_411e_encryption_state_w in such cases
CONS( 201?, myaass,    0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade All Star Stadium - Pocket Player (307-in-1)", MACHINE_NOT_WORKING )
CONS( 201?, myaasa,    0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade All Star Arena - Pocket Player (307-in-1)", MACHINE_NOT_WORKING )

CONS( 201?, mymman,    0,  0,  nes_vt32_8mb, nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Mega Man (DGUNL-7011, Pico Player)", MACHINE_NOT_WORKING )

// most games work, a few minor graphical issues common to the same games in other units
CONS( 202?, typo240,   0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, empty_init, "Typo", "Vintage Gamer 240-in-1", MACHINE_IMPERFECT_GRAPHICS )

// speed challenge doesn't work
CONS( 2021, retror30,  0,  0,  nes_vt32_32mb,      nes_vt32, nes_vt32_unk_state, empty_init, "Orb Gaming", "Retro Racer (30-in-1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// Some games (eg F22) are scrambled like in myaass
// These use a 16x16x8bpp packed tile mode for the main menu which seems more like a VT3xx feature, but VT3xx extended video regs not written?
// also access 3e00 (not 3f00) for palette on said screens?
CONS( 2021, matet220,  0,         0,  nes_vt32_32mb,     nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Tetris (DGUNL-7030, Gamer V, with 220 bonus games)", MACHINE_NOT_WORKING )
CONS( 2021, matet300,  0,         0,  nes_vt32_32mb,     nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Tetris (DGUNL-7029, Go Gamer, with 300 bonus games)", MACHINE_NOT_WORKING )

// unknown tech level, uses vt32 style opcode scramble and palette, lots of unmapped accesses though
CONS( 2021, matet100,  0,        0,  nes_vt32_32mb,      nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Tetris (DGUNL-7027, Pico Player, with 100+ bonus games)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // box says 100+ bonus games

// Uses DIPs switch to select console or cartridge, as cartridge is fake and just toggles a GPIO
CONS( 2016, fcpocket,  0,  0,  nes_vt32_4x16mb,   nes_vt32_fp, nes_vt32_unk_state, empty_init, "<unknown>",   "FC Pocket 600 in 1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )  // has external banking (2x 32mbyte banks)

// aside from the boot screens these have no theming and all contain a barely disguised bootleg version of Nintendo's Pinball in the Games section
CONS( 2020, lxpcsp,    0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init,    "Lexibook", "Power Console - Marvel Spider-Man", MACHINE_NOT_WORKING )
CONS( 2020, lxpcli,    0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init,    "Lexibook", "Power Console - Lilo & Stitch", MACHINE_NOT_WORKING )
CONS( 2020, lxpcpp,    0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init,    "Lexibook", "Power Console - Paw Patrol", MACHINE_NOT_WORKING )
// Power Console - Gabby's Dollhouse
// Power Console - Disney Princess
// Power Console - Frozen
// Power Console - Generic EN/FR model
// Power Console - Generic EN/ES model
// Power Console - Generic EN/DE model

// unclear SoC types maybe even different
// Rush'n Attack has the raster split in the wrong place on the 5 language version (mountains in first stage) when using real hardware
// said game also requires either extra RAM on the PCB (none visible) or a SoC that natively supports that
// 
// not set as clones as each other because the games lists are different
CONS( 201?, k10_5l,    0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, empty_init, "<unknown>", "Games Power 500-in-1 Ultra Thin Handheld Game (K10) (5 languages)", MACHINE_NOT_WORKING )
CONS( 201?, k10_2l,    0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, empty_init, "<unknown>", "Games Power 500-in-1 Ultra Thin Handheld Game (K10) (2 languages)", MACHINE_NOT_WORKING )


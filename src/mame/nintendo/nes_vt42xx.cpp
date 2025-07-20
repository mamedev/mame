// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  nes_vt42xx.cpp

  unknown VT-based SOC

 ***************************************************************************/

#include "emu.h"
#include "nes_vt42xx_soc.h"

#include "multibyte.h"

#include <algorithm>

namespace {

class nes_vt42xx_base_state : public driver_device
{
public:
	nes_vt42xx_base_state(const machine_config& mconfig, device_type type, const char* tag) :
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

class nes_vt42xx_state : public nes_vt42xx_base_state
{
public:
	nes_vt42xx_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt42xx_base_state(mconfig, type, tag),
		m_soc(*this, "soc")
	{ }

	void vt_external_space_map_1mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_2mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_4mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_8mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_16mbyte(address_map &map) ATTR_COLD;

	void nes_vt42xx(machine_config& config);
	void nes_vt42xx_1mb(machine_config& config);
	void nes_vt42xx_2mb(machine_config& config);
	void nes_vt42xx_4mb(machine_config& config);
	void nes_vt42xx_8mb(machine_config& config);
	void nes_vt42xx_16mb(machine_config& config);

	void init_rfcp168();
	void init_g9_666();
	void init_hhgc319();

protected:
	uint8_t vt_rom_banked_r(offs_t offset);

	required_device<nes_vt02_vt03_soc_device> m_soc;
};

class nes_vt42xx_bitboy_state : public nes_vt42xx_state
{
public:
	nes_vt42xx_bitboy_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt42xx_state(mconfig, type, tag)
	{ }

	void nes_vt42xx_bitboy_2x16mb(machine_config& config);
	void nes_vt42xx_gprnrs16_2x16mb(machine_config& config);

	void vt_external_space_map_bitboy_2x16mbyte(address_map &map) ATTR_COLD;

private:
	void bittboy_412c_w(u8 data);
	void gprnrs16_412c_w(u8 data);
};

class nes_vt42xx_fapocket_state : public nes_vt42xx_state
{
public:
	nes_vt42xx_fapocket_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt42xx_state(mconfig, type, tag)
	{ }

	void nes_vt42xx_fa(machine_config& config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	void vt_external_space_map_fa_4x16mbyte(address_map &map) ATTR_COLD;

	u8 fapocket_412c_r();
	void fapocket_412c_w(u8 data);
};

uint8_t nes_vt42xx_base_state::vt_rom_r(offs_t offset)
{
	return m_prgrom[offset];
}

void nes_vt42xx_base_state::vtspace_w(offs_t offset, uint8_t data)
{
	logerror("%s: vtspace_w %08x : %02x", machine().describe_context(), offset, data);
}

// use maps with mirroring in depending on ROM size (this SoC can only access 16MB without banking?)
void nes_vt42xx_state::vt_external_space_map_1mbyte(address_map &map)
{
	map(0x0000000, 0x00fffff).mirror(0x1f00000).r(FUNC(nes_vt42xx_state::vt_rom_r));
}

void nes_vt42xx_state::vt_external_space_map_2mbyte(address_map &map)
{
	map(0x0000000, 0x01fffff).mirror(0x1e00000).r(FUNC(nes_vt42xx_state::vt_rom_r));
}

void nes_vt42xx_state::vt_external_space_map_4mbyte(address_map &map)
{
	map(0x0000000, 0x03fffff).mirror(0x1c00000).r(FUNC(nes_vt42xx_state::vt_rom_r));
}

void nes_vt42xx_state::vt_external_space_map_8mbyte(address_map &map)
{
	map(0x0000000, 0x07fffff).mirror(0x1800000).r(FUNC(nes_vt42xx_state::vt_rom_r));
}

void nes_vt42xx_state::vt_external_space_map_16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt42xx_state::vt_rom_r));
}


uint8_t nes_vt42xx_state::vt_rom_banked_r(offs_t offset)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt42xx_bitboy_state::vt_external_space_map_bitboy_2x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt42xx_bitboy_state::vt_rom_banked_r));
}

void nes_vt42xx_fapocket_state::vt_external_space_map_fa_4x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt42xx_fapocket_state::vt_rom_banked_r));
}


template <uint8_t NUM> uint8_t nes_vt42xx_base_state::extrain_r()
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

uint8_t nes_vt42xx_base_state::in0_r()
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch0 & 1;
	m_latch0 >>= 1;
	return ret;
}

uint8_t nes_vt42xx_base_state::in1_r()
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch1 & 1;
	m_latch1 >>= 1;
	return ret;
}

void nes_vt42xx_base_state::in0_w(uint8_t data)
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


void nes_vt42xx_base_state::machine_start()
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

void nes_vt42xx_base_state::machine_reset()
{

}

void nes_vt42xx_fapocket_state::machine_reset()
{
	nes_vt42xx_state::machine_reset();

	m_ahigh = 0;
}

void nes_vt42xx_base_state::configure_soc(nes_vt02_vt03_soc_device* soc)
{
	soc->set_addrmap(AS_PROGRAM, &nes_vt42xx_state::vt_external_space_map_16mbyte);
	soc->read_0_callback().set(FUNC(nes_vt42xx_base_state::in0_r));
	soc->read_1_callback().set(FUNC(nes_vt42xx_base_state::in1_r));
	soc->write_0_callback().set(FUNC(nes_vt42xx_base_state::in0_w));

	soc->extra_read_0_callback().set(FUNC(nes_vt42xx_base_state::extrain_r<0>));
	soc->extra_read_1_callback().set(FUNC(nes_vt42xx_base_state::extrain_r<1>));
	soc->extra_read_2_callback().set(FUNC(nes_vt42xx_base_state::extrain_r<2>));
	soc->extra_read_3_callback().set(FUNC(nes_vt42xx_base_state::extrain_r<3>));
}

// TODO: these should be in the SoC devices - upper_412d_r gets read, compared against, and another register written based on the result (maybe detecting SoC type?)
uint8_t nes_vt42xx_base_state::upper_412c_r() { logerror("%s: nes_vt42xx_base_state:upper_412c_r\n", machine().describe_context()); return 0x00; }
uint8_t nes_vt42xx_base_state::upper_412d_r() { logerror("%s: nes_vt42xx_base_state:upper_412d_r\n", machine().describe_context()); return 0x00; }
void nes_vt42xx_base_state::upper_412c_w(uint8_t data) { logerror("%s: nes_vt42xx_base_state:upper_412c_w %02x\n", machine().describe_context(), data); }


static INPUT_PORTS_START( nes_vt42xx )
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

void nes_vt42xx_state::nes_vt42xx(machine_config &config)
{
	/* basic machine hardware */
	NES_VT42XX_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt42xx_state::upper_412c_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412d_callback().set(FUNC(nes_vt42xx_state::upper_412d_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt42xx_state::upper_412c_w));

	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB12);
	m_soc->force_bad_dma();
}

void nes_vt42xx_state::nes_vt42xx_1mb(machine_config& config)
{
	nes_vt42xx(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt42xx_state::vt_external_space_map_1mbyte);
}

void nes_vt42xx_state::nes_vt42xx_2mb(machine_config& config)
{
	nes_vt42xx(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt42xx_state::vt_external_space_map_2mbyte);
}

void nes_vt42xx_state::nes_vt42xx_4mb(machine_config& config)
{
	nes_vt42xx(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt42xx_state::vt_external_space_map_4mbyte);
}

void nes_vt42xx_state::nes_vt42xx_8mb(machine_config& config)
{
	nes_vt42xx(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt42xx_state::vt_external_space_map_8mbyte);
}

void nes_vt42xx_state::nes_vt42xx_16mb(machine_config& config)
{
	nes_vt42xx(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt42xx_state::vt_external_space_map_16mbyte);
}


void nes_vt42xx_bitboy_state::bittboy_412c_w(u8 data)
{
	// bittboy (ok)
	logerror("%s: vt03_412c_extbank_w %02x\n", machine().describe_context(),  data);
	m_ahigh = (data & 0x04) ? (1 << 24) : 0x0;
}

void nes_vt42xx_bitboy_state::gprnrs16_412c_w(u8 data)
{
	logerror("%s: vt03_412c_extbank_w %02x\n", machine().describe_context(),  data);
	m_ahigh = (data & 0x02) ? (1 << 24) : 0x0;
}

void nes_vt42xx_bitboy_state::nes_vt42xx_bitboy_2x16mb(machine_config& config)
{
	nes_vt42xx(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt42xx_bitboy_state::vt_external_space_map_bitboy_2x16mbyte);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt42xx_bitboy_state::bittboy_412c_w));
}

void nes_vt42xx_bitboy_state::nes_vt42xx_gprnrs16_2x16mb(machine_config& config)
{
	nes_vt42xx(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt42xx_bitboy_state::vt_external_space_map_bitboy_2x16mbyte);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt42xx_bitboy_state::gprnrs16_412c_w));
}


u8 nes_vt42xx_fapocket_state::fapocket_412c_r()
{
	if (m_cartsel)
		return m_cartsel->read();
	else
		return 0;
}

void nes_vt42xx_fapocket_state::fapocket_412c_w(u8 data)
{
	// fapocket (ok?) (also uses bank from config switch for fake cartridge slot)
	logerror("%s: vtfa_412c_extbank_w %02x\n", machine().describe_context(), data);
	m_ahigh = 0;
	m_ahigh |= (data & 0x01) ? (1 << 25) : 0x0;
	m_ahigh |= (data & 0x02) ? (1 << 24) : 0x0;
}

void nes_vt42xx_fapocket_state::nes_vt42xx_fa(machine_config& config)
{
	nes_vt42xx(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt42xx_fapocket_state::vt_external_space_map_fa_4x16mbyte);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt42xx_fapocket_state::fapocket_412c_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt42xx_fapocket_state::fapocket_412c_w));
}


static INPUT_PORTS_START( nes_vt42xx_fa )
	PORT_INCLUDE(nes_vt42xx)

	PORT_START("CARTSEL")
	PORT_DIPNAME( 0x08, 0x00, "Cartridge Select" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x00, "508-in-1" )
	PORT_DIPSETTING(    0x08, "130-in-1" )
INPUT_PORTS_END


ROM_START( rfcp168 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "winbond_w29gl128c.bin", 0x00000, 0x1000000, CRC(d11caf71) SHA1(64b269cee30a51549a2d0491bbeed07751771559) ) // ROM verified on 2 units
ROM_END

ROM_START( g9_666 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "666in1.u1", 0x00000, 0x1000000, CRC(e3a98465) SHA1(dfec3e74e36aef9bfa57ec530c37642015569dc5) )
ROM_END

ROM_START( g5_500 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "s29gl128.u1", 0x00000, 0x1000000, CRC(de779dd7) SHA1(ac6d3fa6f18ceb795532ba9e85edffc040d74347) )
ROM_END

ROM_START( hhgc319 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "s29gl128n10tfi01.u3", 0x000000, 0x1000000, CRC(4b51125f) SHA1(bab3981ae1652cf6620c7c6769a6729a1e4d588f) )
ROM_END

ROM_START( bittboy )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "bittboy_flash_read_s29gl256n-tf-v2.bin", 0x00000, 0x2000000, CRC(24c802d7) SHA1(c1300ff799b93b9b53060b94d3985db4389c5d3a) )
ROM_END

ROM_START( mc_hh210 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "msp55lv128t.u4", 0x00000, 0x1000000, CRC(9ba520d4) SHA1(627f811b24314197e289a2ade668ff4115421bed) )
ROM_END

ROM_START( retro400 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "retro fc 400-in-1.bin", 0x00000, 0x1000000, CRC(4bf9991b) SHA1(ce9cac61cfc950d832d47afc76eb6c1488eeb2ca) ) // BGA on Subboard
ROM_END

ROM_START( gbox2019 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "fgb2019.bin", 0x00000, 0x1000000, CRC(7ef130d5) SHA1(00f45974494707fdac78153b13d8cfb503716ad0) ) // flash ROM
ROM_END

ROM_START( gprnrs1 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "gprnrs1.bin", 0x00000, 0x800000, CRC(c3ffcec8) SHA1(313a790fb51d0b155257f9de84726ed67da43a8f) )
ROM_END

ROM_START( gprnrs16 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "gprnrs16.bin", 0x00000, 0x2000000, CRC(bdffa40a) SHA1(3d01907211f18e8415171dfc6c1d23cf5952e7bb) )
ROM_END

ROM_START( mc_7x6ss )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "777777-in-1, 8 bit slim station, newpxp-dvt22-a pcb.bin", 0x00000, 0x100000, CRC(7790c21a) SHA1(f320f3dd18b88ae5f65bb51f58d4cb869997bab3) )
ROM_END

ROM_START( mc_8x6ss )
	ROM_REGION( 0x100000, "mainrom", 0 ) // odd size rom, does it need stripping?
	ROM_LOAD( "888888-in-1, 8 bit slim station, newpxp-dvt22-a pcb.bin", 0x00000, 0x100000, CRC(47149d0b) SHA1(5a8733886b550e3235dd90fb415b5a602e967f91) )
	ROM_IGNORE(0xce1)
ROM_END

// PXP2 8Bit Slim Station
ROM_START( mc_9x6ss )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "s29gl032.u3", 0x00000, 0x400000, CRC(9f4194e8) SHA1(bd2a356aea56188ea78169095cbbe603d00e0063) )
ROM_END

// same machine as above? is one of these bad?
ROM_START( mc_9x6sa )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "999999-in-1, 8 bit slim station, newpxp-dvt22-a pcb.bin", 0x00000, 0x200000, CRC(6a47c6a0) SHA1(b4dd376167a57dbee3dea70eb16f1a38e16bcdaa) )
ROM_END

ROM_START( fapocket )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "s29gl512n.bin", 0x00000, 0x4000000, CRC(37d0fb06) SHA1(0146a2fae32e23b65d4032c508f0d12cedd399c3) )
ROM_END

void nes_vt42xx_state::init_rfcp168()
{
	uint8_t *romdata = memregion("mainrom")->base();
	for (offs_t i = 0; i < 0x1000000; i += 0x10000)
	{
		// Swap A12 with A13 and A14 with A15
		std::swap_ranges(&romdata[i + 0x1000], &romdata[i + 0x2000], &romdata[i + 0x2000]);
		std::swap_ranges(&romdata[i + 0x4000], &romdata[i + 0x5000], &romdata[i + 0x8000]);
		std::swap_ranges(&romdata[i + 0x5000], &romdata[i + 0x6000], &romdata[i + 0xa000]);
		std::swap_ranges(&romdata[i + 0x6000], &romdata[i + 0x7000], &romdata[i + 0x9000]);
		std::swap_ranges(&romdata[i + 0x7000], &romdata[i + 0x8000], &romdata[i + 0xb000]);
		std::swap_ranges(&romdata[i + 0xd000], &romdata[i + 0xe000], &romdata[i + 0xe000]);
	}
}

void nes_vt42xx_state::init_g9_666()
{
	uint8_t *romdata = memregion("mainrom")->base();
	for (offs_t i = 0; i < 0x1000000; i += 2)
	{
		uint16_t w = get_u16le(&romdata[i]);
		put_u16le(&romdata[i], (w & 0xf9f9) | (w & 0x0600) >> 8 | (w & 0x0006) << 8);
	}
}

void nes_vt42xx_state::init_hhgc319()
{
	init_rfcp168();

	// Even more pairs of address and data lines to swap here...
	uint8_t *romdata = memregion("mainrom")->base();
	for (offs_t i = 0; i < 0x1000000; i += 0x800000)
	{
		std::swap_ranges(&romdata[i + 0x080000], &romdata[i + 0x100000], &romdata[i + 0x400000]);
		std::swap_ranges(&romdata[i + 0x180000], &romdata[i + 0x200000], &romdata[i + 0x500000]);
		std::swap_ranges(&romdata[i + 0x280000], &romdata[i + 0x300000], &romdata[i + 0x600000]);
		std::swap_ranges(&romdata[i + 0x380000], &romdata[i + 0x400000], &romdata[i + 0x700000]);
	}
	for (offs_t i = 0; i < 0x1000000; i += 0x800)
		std::swap_ranges(&romdata[i + 0x200], &romdata[i + 0x400], &romdata[i + 0x400]);
	for (offs_t i = 0; i < 0x1000000; i += 0x20)
		std::swap_ranges(&romdata[i + 0x08], &romdata[i + 0x10], &romdata[i + 0x10]);
	for (offs_t i = 0; i < 0x1000000; i += 2)
		put_u16le(&romdata[i], bitswap<16>(get_u16le(&romdata[i]), 15, 14, 6, 5, 3, 2, 9, 8, 7, 13, 12, 4, 11, 10, 1, 0));
}

} // anonymous namespace


CONS( 201?, rfcp168,  0,  0,  nes_vt42xx_16mb, nes_vt42xx, nes_vt42xx_state, init_rfcp168, "<unknown>", "Retro FC Plus 168 in 1 Handheld", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // "RETRO_FC_V3.5"

// many duplicates, real game count to be confirmed, graphical issues in some games
CONS( 202?, g9_666,   0,  0,  nes_vt42xx_16mb, nes_vt42xx, nes_vt42xx_state, init_g9_666, "<unknown>", "G9 Game Box 666 Games", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// same bitswap as above
CONS( 201?, g5_500,   0,  0,  nes_vt42xx_16mb, nes_vt42xx, nes_vt42xx_state, init_g9_666, "<unknown>", "G5 500 in 1 Handheld", MACHINE_NOT_WORKING )

// highly scrambled
CONS( 201?, hhgc319,  0,  0,  nes_vt42xx_16mb, nes_vt42xx, nes_vt42xx_state, init_hhgc319, "<unknown>", "Handheld Game Console 319-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// Runs well, only issues in SMB3 which crashes
CONS( 2017, bittboy,  0,  0,  nes_vt42xx_bitboy_2x16mb, nes_vt42xx, nes_vt42xx_bitboy_state, empty_init, "BittBoy",   "BittBoy Mini FC 300 in 1", MACHINE_IMPERFECT_GRAPHICS ) // has external banking (2x 16mbyte banks)

// No title screen, but press start and menu and games run fine. Makes odd
// memory accesses which probably explain broken title screen
CONS( 201?, mc_hh210, 0,  0,  nes_vt42xx_16mb, nes_vt42xx, nes_vt42xx_state, empty_init, "<unknown>", "Handheld 210 in 1", MACHINE_NOT_WORKING )

CONS( 201?, retro400, 0,  0,  nes_vt42xx_16mb, nes_vt42xx, nes_vt42xx_state, empty_init, "<unknown>", "Retro FC 400-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

CONS( 2019, gbox2019, 0,  0,  nes_vt42xx_16mb, nes_vt42xx, nes_vt42xx_state, empty_init, "Sup", "Game Box 400 in 1 (2019 PCB)", MACHINE_NOT_WORKING )

CONS( 200?, gprnrs1,  0,  0,  nes_vt42xx_8mb,  nes_vt42xx, nes_vt42xx_state, empty_init, "<unknown>", "Game Prince RS-1", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, gprnrs16, 0,  0,  nes_vt42xx_gprnrs16_2x16mb, nes_vt42xx, nes_vt42xx_bitboy_state, empty_init, "<unknown>", "Game Prince RS-16", MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, mc_9x6ss, 0,        0, nes_vt42xx_4mb, nes_vt42xx, nes_vt42xx_state, empty_init, "<unknown>", "999999 in 1 (PXP2 Slim Station)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_9x6sa, mc_9x6ss, 0, nes_vt42xx_2mb, nes_vt42xx, nes_vt42xx_state, empty_init, "<unknown>", "999999 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_7x6ss, 0,        0, nes_vt42xx_1mb, nes_vt42xx, nes_vt42xx_state, empty_init, "<unknown>", "777777 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_8x6ss, 0,        0, nes_vt42xx_1mb, nes_vt42xx, nes_vt42xx_state, empty_init, "<unknown>", "888888 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_IMPERFECT_GRAPHICS )

// Uses DIP switch to select console or cartridge, as cartridge is fake and just toggles a GPIO
CONS( 2017, fapocket,  0,  0,  nes_vt42xx_fa,     nes_vt42xx_fa, nes_vt42xx_fapocket_state, empty_init, "<unknown>",   "Family Pocket 638 in 1", MACHINE_IMPERFECT_GRAPHICS ) // has external banking (4x 16mbyte banks)

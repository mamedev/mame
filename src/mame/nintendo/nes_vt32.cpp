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

#include "multibyte.h"

#include <algorithm>

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

	void vt_external_space_map_1mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_2mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_4mbyte(address_map &map) ATTR_COLD;
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
	void nes_vt32_1mb(machine_config& config);
	void nes_vt32_2mb(machine_config& config);
	void nes_vt32_4mb(machine_config& config);
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

class nes_vt32_bitboy_state : public nes_vt32_unk_state
{
public:
	nes_vt32_bitboy_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt32_unk_state(mconfig, type, tag)
	{ }

	void nes_vt32_bitboy_2x16mb(machine_config& config);
	void nes_vt32_gprnrs16_2x16mb(machine_config& config);

	void vt_external_space_map_bitboy_2x16mbyte(address_map &map) ATTR_COLD;

private:
	void bittboy_412c_w(u8 data);
	void gprnrs16_412c_w(u8 data);
};

class nes_vt32_fapocket_state : public nes_vt32_unk_state
{
public:
	nes_vt32_fapocket_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt32_unk_state(mconfig, type, tag)
	{ }

	void nes_vt32_fa(machine_config& config);

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	void vt_external_space_map_fa_4x16mbyte(address_map &map) ATTR_COLD;

	u8 fapocket_412c_r();
	void fapocket_412c_w(u8 data);
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

void nes_vt32_bitboy_state::vt_external_space_map_bitboy_2x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt32_bitboy_state::vt_rom_banked_r));
}

void nes_vt32_fapocket_state::vt_external_space_map_fa_4x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt32_fapocket_state::vt_rom_banked_r));
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

void nes_vt32_fapocket_state::machine_reset()
{
	nes_vt32_unk_state::machine_reset();

	m_ahigh = 0;
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

void nes_vt32_unk_state::nes_vt32_1mb(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_unk_state::vt_external_space_map_1mbyte);
}

void nes_vt32_unk_state::nes_vt32_2mb(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_unk_state::vt_external_space_map_2mbyte);
}

void nes_vt32_unk_state::nes_vt32_4mb(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_unk_state::vt_external_space_map_4mbyte);
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


void nes_vt32_bitboy_state::bittboy_412c_w(u8 data)
{
	// bittboy (ok)
	logerror("%s: vt03_412c_extbank_w %02x\n", machine().describe_context(),  data);
	m_ahigh = (data & 0x04) ? (1 << 24) : 0x0;
}

void nes_vt32_bitboy_state::gprnrs16_412c_w(u8 data)
{
	logerror("%s: vt03_412c_extbank_w %02x\n", machine().describe_context(),  data);
	m_ahigh = (data & 0x02) ? (1 << 24) : 0x0;
}

void nes_vt32_bitboy_state::nes_vt32_bitboy_2x16mb(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_bitboy_state::vt_external_space_map_bitboy_2x16mbyte);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt32_bitboy_state::bittboy_412c_w));
}

void nes_vt32_bitboy_state::nes_vt32_gprnrs16_2x16mb(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_bitboy_state::vt_external_space_map_bitboy_2x16mbyte);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt32_bitboy_state::gprnrs16_412c_w));
}


u8 nes_vt32_fapocket_state::fapocket_412c_r()
{
	if (m_cartsel)
		return m_cartsel->read();
	else
		return 0;
}

void nes_vt32_fapocket_state::fapocket_412c_w(u8 data)
{
	// fapocket (ok?) (also uses bank from config switch for fake cartridge slot)
	logerror("%s: vtfa_412c_extbank_w %02x\n", machine().describe_context(), data);
	m_ahigh = 0;
	m_ahigh |= (data & 0x01) ? (1 << 25) : 0x0;
	m_ahigh |= (data & 0x02) ? (1 << 24) : 0x0;
}

void nes_vt32_fapocket_state::nes_vt32_fa(machine_config& config)
{
	nes_vt32_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt32_fapocket_state::vt_external_space_map_fa_4x16mbyte);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt32_fapocket_state::fapocket_412c_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt32_fapocket_state::fapocket_412c_w));
}


static INPUT_PORTS_START( nes_vt32_fp )
	PORT_INCLUDE(nes_vt32)

	PORT_START("CARTSEL")
	PORT_DIPNAME( 0x06, 0x00, "Cartridge Select" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x00, "472-in-1" )
	PORT_DIPSETTING(    0x06, "128-in-1" )
INPUT_PORTS_END

static INPUT_PORTS_START( nes_vt32_fa )
	PORT_INCLUDE(nes_vt32)

	PORT_START("CARTSEL")
	PORT_DIPNAME( 0x08, 0x00, "Cartridge Select" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x00, "508-in-1" )
	PORT_DIPSETTING(    0x08, "130-in-1" )
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

ROM_START( goretrop )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "goretroportable.bin", 0x00000, 0x2000000, CRC(e7279dd3) SHA1(5f096ce22e46f112c2cc6588cb1c527f4f0430b5) )
ROM_END

ROM_START( fcpocket )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "s29gl01gp.bin", 0x00000, 0x8000000, CRC(8703b18a) SHA1(07943443294e80ca93f83181c8bdbf950b87c52f) ) // 2nd half = 0x00 (so 64MByte of content)
ROM_END

ROM_START( fapocket )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "s29gl512n.bin", 0x00000, 0x4000000, CRC(37d0fb06) SHA1(0146a2fae32e23b65d4032c508f0d12cedd399c3) )
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

ROM_START( typo240 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "240nes.u2", 0x00000, 0x1000000, CRC(d709f66c) SHA1(73ca34ce07a1a8782226bd74b1ae43fc6d7126e1) ) // s29gl128p90tfcr1
ROM_END

void nes_vt32_unk_state::init_rfcp168()
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

void nes_vt32_unk_state::init_g9_666()
{
	uint8_t *romdata = memregion("mainrom")->base();
	for (offs_t i = 0; i < 0x1000000; i += 2)
	{
		uint16_t w = get_u16le(&romdata[i]);
		put_u16le(&romdata[i], (w & 0xf9f9) | (w & 0x0600) >> 8 | (w & 0x0006) << 8);
	}
}

void nes_vt32_unk_state::init_hhgc319()
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

// lots of accesses to $42xx (could this be a different SoC?)
CONS( 201?, rfcp168,  0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, init_rfcp168, "<unknown>", "Retro FC Plus 168 in 1 Handheld", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // "RETRO_FC_V3.5"

// many duplicates, real game count to be confirmed, graphical issues in some games, lots of accesses to $42xx
CONS( 202?, g9_666,   0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, init_g9_666, "<unknown>", "G9 Game Box 666 Games", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// same bitswap as above, lots of accesses to $42xx
CONS( 201?, g5_500,   0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, init_g9_666, "<unknown>", "G5 500 in 1 Handheld", MACHINE_NOT_WORKING )

// lots of accesses to $42xx, highly scrambled
CONS( 201?, hhgc319,  0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, init_hhgc319, "<unknown>", "Handheld Game Console 319-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// lots of accesses to $42xx
// Runs well, only issues in SMB3 which crashes
CONS( 2017, bittboy,  0,  0,  nes_vt32_bitboy_2x16mb, nes_vt32, nes_vt32_bitboy_state, empty_init, "BittBoy",   "BittBoy Mini FC 300 in 1", MACHINE_IMPERFECT_GRAPHICS ) // has external banking (2x 16mbyte banks)

// lots of accesses to $42xx
// No title screen, but press start and menu and games run fine. Makes odd
// memory accesses which probably explain broken title screen
CONS( 201?, mc_hh210, 0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, empty_init, "<unknown>", "Handheld 210 in 1", MACHINE_NOT_WORKING )

// lots of accesses to $42xx, M705-128A6 sub-board with BGA
CONS( 201?, retro400, 0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, empty_init, "<unknown>", "Retro FC 400-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// lots of accesses to $42xx
CONS( 2019, gbox2019, 0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, empty_init, "Sup", "Game Box 400 in 1 (2019 PCB)", MACHINE_NOT_WORKING )

// lots of accesses to $42xx
CONS( 200?, gprnrs1,  0,  0,  nes_vt32_8mb,  nes_vt32, nes_vt32_unk_state, empty_init, "<unknown>", "Game Prince RS-1", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, gprnrs16, 0,  0,  nes_vt32_gprnrs16_2x16mb, nes_vt32, nes_vt32_bitboy_state, empty_init, "<unknown>", "Game Prince RS-16", MACHINE_IMPERFECT_GRAPHICS )

// lots of accesses to $42xx
CONS( 200?, mc_9x6ss, 0,        0, nes_vt32_4mb, nes_vt32, nes_vt32_unk_state, empty_init, "<unknown>", "999999 in 1 (PXP2 Slim Station)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_9x6sa, mc_9x6ss, 0, nes_vt32_2mb, nes_vt32, nes_vt32_unk_state, empty_init, "<unknown>", "999999 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_7x6ss, 0,        0, nes_vt32_1mb, nes_vt32, nes_vt32_unk_state, empty_init, "<unknown>", "777777 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_8x6ss, 0,        0, nes_vt32_1mb, nes_vt32, nes_vt32_unk_state, empty_init, "<unknown>", "888888 in 1 (8 bit Slim Station, NEWPXP-DVT22-A PCB)", MACHINE_IMPERFECT_GRAPHICS )

// most games work, a few minor graphical issues common to the same games in other units
CONS( 202?, typo240,   0,  0,  nes_vt32_16mb, nes_vt32, nes_vt32_unk_state, empty_init, "Typo", "Vintage Gamer 240-in-1", MACHINE_IMPERFECT_GRAPHICS )

// there's also a 250+ version of the unit below at least
CONS( 2018, goretrop,    0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init,    "Retro-Bit", "Go Retro Portable 260+ Games", MACHINE_NOT_WORKING )

// Some games (eg F22) are scrambled like in myaass
// These use a 16x16x8bpp packed tile mode for the main menu which seems more like a VT3xx feature, but VT3xx extended video regs not written?
// also access 3e00 (not 3f00) for palette on said screens?
CONS( 2021, matet220,  0,         0,  nes_vt32_32mb,     nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Tetris (DGUNL-7030, Gamer V, with 220 bonus games)", MACHINE_NOT_WORKING )
CONS( 2021, matet300,  0,         0,  nes_vt32_32mb,     nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Tetris (DGUNL-7029, Go Gamer, with 300 bonus games)", MACHINE_NOT_WORKING )

// unknown tech level, uses vt32 style opcode scramble and palette, lots of unmapped accesses though
CONS( 2021, matet100,  0,        0,  nes_vt32_32mb,      nes_vt32, nes_vt32_unk_state, empty_init, "dreamGEAR", "My Arcade Tetris (DGUNL-7027, Pico Player, with 100+ bonus games)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // box says 100+ bonus games

// Use DIP switch to select console or cartridge, as cartridge is fake and just toggles a GPIO
// fapocket has lots of accesses to $42xx
CONS( 2016, fcpocket,  0,  0,  nes_vt32_4x16mb,   nes_vt32_fp, nes_vt32_unk_state, empty_init, "<unknown>",   "FC Pocket 600 in 1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )  // has external banking (2x 32mbyte banks)
CONS( 2017, fapocket,  0,  0,  nes_vt32_fa,       nes_vt32_fa, nes_vt32_fapocket_state, empty_init, "<unknown>",   "Family Pocket 638 in 1", MACHINE_IMPERFECT_GRAPHICS ) // has external banking (4x 16mbyte banks)

// aside from the boot screens these have no theming and all contain a barely disguised bootleg version of Nintendo's Pinball in the Games section
CONS( 2020, lxpcsp,    0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init,    "Lexibook", "Power Console - Marvel Spider-Man", MACHINE_NOT_WORKING )
CONS( 2020, lxpcli,    0,  0,  nes_vt32_32mb, nes_vt32, nes_vt32_unk_state, empty_init,    "Lexibook", "Power Console - Lilo & Stitch", MACHINE_NOT_WORKING )

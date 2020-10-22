// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  nes_vt369_vtunknown.cpp

  VT369 and unknown/higher

   - new screen modes
   - new registers for controlling banking
   - can run from SPI ROM
   - additional audio cpu (like VT1682) and multiplier on VT369 models

   (not all features are used all games, but anything that has an SPI ROM
    must at least be this tech level)

  TODO:
  this still needs significant cleanups before work is started on individual
  systems

  some of these might be older systems eg. fapocket

  ***************************************************************************/

#include "emu.h"
#include "machine/nes_vt369_vtunknown_soc.h"
#include "machine/nes_vt32_soc.h"

class nes_vt369_vtunknown_base_state : public driver_device
{
public:
	nes_vt369_vtunknown_base_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_io0(*this, "IO0"),
		m_io1(*this, "IO1"),
		m_cartsel(*this, "CARTSEL"),
		m_exin(*this, "EXTRAIN%u", 0U),
		m_prgrom(*this, "mainrom")
	{ }

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual uint8_t in0_r();
	virtual uint8_t in1_r();
	virtual void in0_w(uint8_t data);

	void nes_vt369_vtunknown_map(address_map& map);

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
	void vtspace_w(offs_t offset, uint8_t data);

	void configure_soc(nes_vt02_vt03_soc_device* soc);

	uint8_t upper_412c_r();
	uint8_t upper_412d_r();
	void upper_412c_w(uint8_t data);

private:
	/* Extra IO */
	template <uint8_t NUM> uint8_t extrain_r();
};

class nes_vt369_vtunknown_state : public nes_vt369_vtunknown_base_state
{
public:
	nes_vt369_vtunknown_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt369_vtunknown_base_state(mconfig, type, tag),
		m_soc(*this, "soc")
	{ }

	void nes_vt369_vtunknown_4k_ram(machine_config& config);
	void nes_vt369_vtunknown_4k_ram_16mb(machine_config& config);

	void vt_external_space_map_32mbyte(address_map& map);
	void vt_external_space_map_16mbyte(address_map& map);
	void vt_external_space_map_8mbyte(address_map& map);
	void vt_external_space_map_4mbyte(address_map& map);
	void vt_external_space_map_2mbyte(address_map& map);
	void vt_external_space_map_1mbyte(address_map& map);
	void vt_external_space_map_512kbyte(address_map& map);

	void init_lxcmcypp();

protected:
	required_device<nes_vt02_vt03_soc_device> m_soc;
};


class nes_vt369_vtunknown_swap_op_d5_d6_state : public nes_vt369_vtunknown_state
{
public:
	nes_vt369_vtunknown_swap_op_d5_d6_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt369_vtunknown_state(mconfig, type, tag)
	{ }

	void nes_vt369_vtunknown_vh2009_8mb(machine_config& config);
protected:
};


class nes_vt369_vtunknown_cy_state : public nes_vt369_vtunknown_state
{
public:
	nes_vt369_vtunknown_cy_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt369_vtunknown_state(mconfig, type, tag)
	{ }

	void nes_vt369_vtunknown_cy(machine_config& config);
	void nes_vt369_vtunknown_cy_bigger(machine_config& config);
	void nes_vt369_vtunknown_bt(machine_config& config);
	void nes_vt369_vtunknown_bt_2x16mb(machine_config& config);

	void vt_external_space_map_bitboy_2x16mbyte(address_map& map);

private:

	void bittboy_412c_w(uint8_t data);

	uint8_t vt_rom_banked_r(offs_t offset);
};



class nes_vt369_vtunknown_dg_fapocket_state : public nes_vt369_vtunknown_state
{
public:
	nes_vt369_vtunknown_dg_fapocket_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt369_vtunknown_state(mconfig, type, tag)
	{ }

	void nes_vt369_vtunknown_fa_4x16mb(machine_config& config);

protected:
	virtual void machine_reset() override;

private:
	uint8_t vt_rom_banked_r(offs_t offset);
	void vt_external_space_map_fapocket_4x16mbyte(address_map& map);

	uint8_t fapocket_412c_r();
	void fapocket_412c_w(uint8_t data);

};


class nes_vt369_vtunknown_unk_state : public nes_vt369_vtunknown_state
{
public:
	nes_vt369_vtunknown_unk_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt369_vtunknown_state(mconfig, type, tag)
	{ }

	void nes_vt369_vtunknown_hh(machine_config& config);
	void nes_vt369_vtunknown_hh_4mb(machine_config& config);
	void nes_vt369_vtunknown_hh_8mb(machine_config& config);

	void nes_vt369_vtunknown_unk(machine_config& config);
	void nes_vt369_vtunknown_unk_1mb(machine_config& config);
	void nes_vt369_vtunknown_unk_16mb(machine_config& config);

	void nes_vt369_vtunknown_fp(machine_config& config);
	void nes_vt369_vtunknown_fp_16mb(machine_config& config);

private:
	uint8_t vt_rom_banked_r(offs_t offset);
	void vt_external_space_map_fp_2x32mbyte(address_map& map);
};

uint8_t nes_vt369_vtunknown_base_state::vt_rom_r(offs_t offset)
{
	return m_prgrom[offset];
}

void nes_vt369_vtunknown_base_state::vtspace_w(offs_t offset, uint8_t data)
{
	logerror("%s: vtspace_w %08x : %02x", machine().describe_context(), offset, data);
}

// VTxx can address 25-bit address space (32MB of ROM) so use maps with mirroring in depending on ROM size
void nes_vt369_vtunknown_state::vt_external_space_map_32mbyte(address_map &map)
{
	map(0x0000000, 0x1ffffff).r(FUNC(nes_vt369_vtunknown_state::vt_rom_r));
}

void nes_vt369_vtunknown_state::vt_external_space_map_16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt369_vtunknown_state::vt_rom_r));
}

void nes_vt369_vtunknown_state::vt_external_space_map_8mbyte(address_map &map)
{
	map(0x0000000, 0x07fffff).mirror(0x1800000).r(FUNC(nes_vt369_vtunknown_state::vt_rom_r));
}

void nes_vt369_vtunknown_state::vt_external_space_map_4mbyte(address_map &map)
{
	map(0x0000000, 0x03fffff).mirror(0x1c00000).r(FUNC(nes_vt369_vtunknown_state::vt_rom_r));
}

void nes_vt369_vtunknown_state::vt_external_space_map_2mbyte(address_map &map)
{
	map(0x0000000, 0x01fffff).mirror(0x1e00000).r(FUNC(nes_vt369_vtunknown_state::vt_rom_r));
}

void nes_vt369_vtunknown_state::vt_external_space_map_1mbyte(address_map &map)
{
	map(0x0000000, 0x00fffff).mirror(0x1f00000).r(FUNC(nes_vt369_vtunknown_state::vt_rom_r));
}

void nes_vt369_vtunknown_state::vt_external_space_map_512kbyte(address_map &map)
{
	map(0x0000000, 0x007ffff).mirror(0x1f80000).r(FUNC(nes_vt369_vtunknown_state::vt_rom_r));
}

// bitboy is 2 16Mbyte banks
uint8_t nes_vt369_vtunknown_cy_state::vt_rom_banked_r(offs_t offset)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt369_vtunknown_cy_state::vt_external_space_map_bitboy_2x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt369_vtunknown_cy_state::vt_rom_banked_r));
}

// fapocket is 4 16Mbyte banks
uint8_t nes_vt369_vtunknown_dg_fapocket_state::vt_rom_banked_r(offs_t offset)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt369_vtunknown_dg_fapocket_state::vt_external_space_map_fapocket_4x16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt369_vtunknown_dg_fapocket_state::vt_rom_banked_r));
}

uint8_t nes_vt369_vtunknown_unk_state::vt_rom_banked_r(offs_t offset)
{
	return m_prgrom[m_ahigh | offset];
}

void nes_vt369_vtunknown_unk_state::vt_external_space_map_fp_2x32mbyte(address_map &map)
{
	map(0x0000000, 0x1ffffff).r(FUNC(nes_vt369_vtunknown_unk_state::vt_rom_banked_r));
}


template <uint8_t NUM> uint8_t nes_vt369_vtunknown_base_state::extrain_r()
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

uint8_t nes_vt369_vtunknown_base_state::in0_r()
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch0 & 1;
	m_latch0 >>= 1;
	return ret;
}

uint8_t nes_vt369_vtunknown_base_state::in1_r()
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch1 & 1;
	m_latch1 >>= 1;
	return ret;
}

void nes_vt369_vtunknown_base_state::in0_w(uint8_t data)
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


void nes_vt369_vtunknown_base_state::machine_start()
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

void nes_vt369_vtunknown_base_state::machine_reset()
{

}

void nes_vt369_vtunknown_dg_fapocket_state::machine_reset()
{
	nes_vt369_vtunknown_base_state::machine_reset();

	// fapocket needs this, fcpocket instead reads the switch in software?
	if (m_cartsel)
		m_ahigh = (m_cartsel->read() == 0x01) ? (1 << 25) : 0x0;
	else
		m_ahigh = 0;
}
void nes_vt369_vtunknown_base_state::configure_soc(nes_vt02_vt03_soc_device* soc)
{
	soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_state::vt_external_space_map_32mbyte);
	soc->read_0_callback().set(FUNC(nes_vt369_vtunknown_base_state::in0_r));
	soc->read_1_callback().set(FUNC(nes_vt369_vtunknown_base_state::in1_r));
	soc->write_0_callback().set(FUNC(nes_vt369_vtunknown_base_state::in0_w));

	soc->extra_read_0_callback().set(FUNC(nes_vt369_vtunknown_base_state::extrain_r<0>));
	soc->extra_read_1_callback().set(FUNC(nes_vt369_vtunknown_base_state::extrain_r<1>));
	soc->extra_read_2_callback().set(FUNC(nes_vt369_vtunknown_base_state::extrain_r<2>));
	soc->extra_read_3_callback().set(FUNC(nes_vt369_vtunknown_base_state::extrain_r<3>));
}

uint8_t nes_vt369_vtunknown_base_state::upper_412c_r()
{
	logerror("%s: upper_412c_r\n", machine().describe_context());
	return 0x00;
}

uint8_t nes_vt369_vtunknown_base_state::upper_412d_r()
{
	logerror("%s: upper_412d_r\n", machine().describe_context());
	return 0x00;
}

void nes_vt369_vtunknown_base_state::upper_412c_w(uint8_t data)
{
	logerror("%s: upper_412c_w %02x\n", machine().describe_context(), data);
}

void nes_vt369_vtunknown_state::nes_vt369_vtunknown_4k_ram(machine_config &config)
{
	/* basic machine hardware */
	NES_VT09_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt369_vtunknown_state::upper_412c_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412d_callback().set(FUNC(nes_vt369_vtunknown_state::upper_412d_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt369_vtunknown_state::upper_412c_w));
}

void nes_vt369_vtunknown_state::nes_vt369_vtunknown_4k_ram_16mb(machine_config &config)
{
	nes_vt369_vtunknown_4k_ram(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_state::vt_external_space_map_16mbyte);
}

void nes_vt369_vtunknown_cy_state::nes_vt369_vtunknown_cy(machine_config &config)
{
	nes_vt369_vtunknown_4k_ram(config);

	NES_VTUNKNOWN_SOC_CY(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
}

void nes_vt369_vtunknown_cy_state::nes_vt369_vtunknown_cy_bigger(machine_config &config)
{
	nes_vt369_vtunknown_cy(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_cy_state::vt_external_space_map_32mbyte); // must be some banking of this kind of VT can address over 32mb
}

void nes_vt369_vtunknown_cy_state::nes_vt369_vtunknown_bt(machine_config &config)
{
	nes_vt369_vtunknown_4k_ram(config);

	NES_VTUNKNOWN_SOC_BT(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
}

void nes_vt369_vtunknown_cy_state::bittboy_412c_w(uint8_t data)
{
	//bittboy (ok), mc_pg150 (not working)
	logerror("%s: vt03_412c_extbank_w %02x\n", machine().describe_context(),  data);
	m_ahigh = (data & 0x04) ? (1 << 24) : 0x0;
}

void nes_vt369_vtunknown_cy_state::nes_vt369_vtunknown_bt_2x16mb(machine_config& config)
{
	nes_vt369_vtunknown_bt(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_cy_state::vt_external_space_map_bitboy_2x16mbyte);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt369_vtunknown_cy_state::bittboy_412c_w));
}

void nes_vt369_vtunknown_unk_state::nes_vt369_vtunknown_unk(machine_config &config)
{
	nes_vt369_vtunknown_4k_ram(config);

	NES_VTUNKNOWN_SOC_DG(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->force_bad_dma();
}

void nes_vt369_vtunknown_unk_state::nes_vt369_vtunknown_unk_16mb(machine_config& config)
{
	nes_vt369_vtunknown_unk(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_unk_state::vt_external_space_map_16mbyte);
}

void nes_vt369_vtunknown_unk_state::nes_vt369_vtunknown_unk_1mb(machine_config& config)
{
	nes_vt369_vtunknown_unk(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_unk_state::vt_external_space_map_1mbyte);
}





// New mystery handheld architecture, VTxx derived
void nes_vt369_vtunknown_unk_state::nes_vt369_vtunknown_hh(machine_config &config)
{
	nes_vt369_vtunknown_4k_ram(config);

	NES_VT369_SOC(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB);
	m_soc->force_bad_dma();
}

void nes_vt369_vtunknown_unk_state::nes_vt369_vtunknown_hh_8mb(machine_config& config)
{
	nes_vt369_vtunknown_hh(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_unk_state::vt_external_space_map_8mbyte);
}

void nes_vt369_vtunknown_unk_state::nes_vt369_vtunknown_hh_4mb(machine_config& config)
{
	nes_vt369_vtunknown_hh(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_unk_state::vt_external_space_map_4mbyte);
}


static INPUT_PORTS_START( nes_vt369_vtunknown )
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


void nes_vt369_vtunknown_unk_state::nes_vt369_vtunknown_fp(machine_config &config)
{
	nes_vt369_vtunknown_4k_ram(config);

	NES_VT32_SOC(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB12);
	m_soc->force_bad_dma();
}


void nes_vt369_vtunknown_unk_state::nes_vt369_vtunknown_fp_16mb(machine_config& config)
{
	nes_vt369_vtunknown_fp(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_unk_state::vt_external_space_map_16mbyte);
}




uint8_t nes_vt369_vtunknown_dg_fapocket_state::fapocket_412c_r()
{
	if (m_cartsel)
		return m_cartsel->read();
	else
		return 0;
}

void nes_vt369_vtunknown_dg_fapocket_state::fapocket_412c_w(uint8_t data)
{
	// fapocket (ok?) (also uses bank from config switch for fake cartridge slot)
	logerror("%s: vtfa_412c_extbank_w %02x\n", machine().describe_context(), data);
	m_ahigh = 0;
	m_ahigh |= (data & 0x01) ? (1 << 25) : 0x0;
	m_ahigh |= (data & 0x02) ? (1 << 24) : 0x0;
}





void nes_vt369_vtunknown_dg_fapocket_state::nes_vt369_vtunknown_fa_4x16mb(machine_config& config) // fapocket
{
	nes_vt369_vtunknown_4k_ram(config);

	NES_VTUNKNOWN_SOC_FA(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_dg_fapocket_state::vt_external_space_map_fapocket_4x16mbyte);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt369_vtunknown_dg_fapocket_state::fapocket_412c_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt369_vtunknown_dg_fapocket_state::fapocket_412c_w));
}


void nes_vt369_vtunknown_swap_op_d5_d6_state::nes_vt369_vtunknown_vh2009_8mb(machine_config& config)
{
	NES_VT02_VT03_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	NES_VT02_VT03_SOC_SCRAMBLE(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_addrmap(AS_PROGRAM, &nes_vt369_vtunknown_swap_op_d5_d6_state::vt_external_space_map_8mbyte);
}


static INPUT_PORTS_START( nes_vt369_vtunknown_fa )
	PORT_INCLUDE(nes_vt369_vtunknown)

	PORT_START("CARTSEL")
	PORT_DIPNAME( 0x01, 0x00, "Cartridge Select" ) PORT_CODE(KEYCODE_3) PORT_TOGGLE
	PORT_DIPSETTING(    0x00, "508-in-1" )
	PORT_DIPSETTING(    0x01, "130-in-1" )
INPUT_PORTS_END




ROM_START( dgun2561 )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "dgun2561.bin", 0x00000, 0x4000000, CRC(a6e627b4) SHA1(2667d2feb02de349387f9dcfa5418e7ed3afeef6) )
ROM_END

ROM_START( dgun2593 )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "dreamgear300.bin", 0x00000, 0x8000000, CRC(4fe0ed02) SHA1(a55590557bacca65ed9a17c5bcf0a4e5cb223126) )
ROM_END

ROM_START( rtvgc300 )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	// some of the higher address lines might be swapped
	ROM_LOAD( "lexibook300.bin", 0x00000, 0x4000000, CRC(015c4067) SHA1(a12986c4a366a23c4c7ca7b3d33e421a8dfdffc0) )
ROM_END

ROM_START( rtvgc300fz )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	// some of the higher address lines might be swapped
	ROM_LOAD( "jg7800fz.bin", 0x00000, 0x4000000, CRC(c9d319d2) SHA1(9d0d1435b802f63ce11b94ce54d11f4065b324cc) )
ROM_END

// The maximum address space a VT chip can see is 32MB, so these 64MB roms are actually 2 programs (there are vectors in the first half and the 2nd half)
// there must be a bankswitch bit that switches the whole 32MB space.  Loading the 2nd half in Star Wars does actually boot straight to a game.
ROM_START( lxcmcy )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "lxcmcy.bin", 0x00000, 0x4000000, CRC(3f3af72c) SHA1(76127054291568fcce1431d21af71f775cfb05a6) )
ROM_END

ROM_START( lxcmcysw )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "jl2365swr-1.u2", 0x2000000, 0x2000000, CRC(60ece391) SHA1(655de6b36ba596d873de2839522b948ccf45e006) )
	ROM_CONTINUE(0x0000000, 0x2000000)
ROM_END

ROM_START( lxcmcyfz )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "jl2365_frozen.u1", 0x00000, 0x4000000, CRC(64d4c708) SHA1(1bc2d161326ce3039ab9ba46ad62695060cfb2e1) )
ROM_END

ROM_START( lxcmcydp )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "cyberarcade-disneyprincess.bin", 0x00000, 0x4000000, CRC(05946f81) SHA1(33eea2b70f5427e7613c836b8a08148731fac231) )
ROM_END

ROM_START( lxcmcysp )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "lexibookspiderman.bin", 0x00000, 0x4000000, CRC(ef6e8847) SHA1(0012df193c52fd48595d85886fd431619c5d5e3e) )
ROM_END

ROM_START( lxcmcycr )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "lexibook cars.bin", 0x00000, 0x4000000, CRC(198fe11b) SHA1(5e35caa3fc319ec69812c187a3ec89f01749f749) )
ROM_END

ROM_START( lxcmcypp )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// marked 512mbit, possible A22 / A23 are swapped as they were marked on the board in a different way.
	ROM_LOAD( "pawpatrol_compact.bin", 0x00000, 0x4000000, CRC(bf536762) SHA1(80dde8426a636bae33a82d779e564fa743eb3776) )
ROM_END

ROM_START( lxcmc250 )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "cca250in1.u1", 0x00000, 0x4000000, CRC(6ccd6ad6) SHA1(fafed339097c3d1538faa306021a8373c1b799b3) )
ROM_END

ROM_START( lxccminn )
	ROM_REGION( 0x4000000, "mainrom", 0 ) // sub-board was hardwired to only be able to address the lower 64MByte, was rewired to also dump upper half when dumping, upper half contains only garbage, hence ROM_IGNORE
	ROM_LOAD( "minnie_lexibook.bin", 0x00000, 0x4000000, CRC(3f8e5a69) SHA1(c9f11f3e5f9b73832a191f4d1620a85c1b70f79e) )
	ROM_IGNORE(0x4000000)
ROM_END

ROM_START( lxccplan )
	ROM_REGION( 0x4000000, "mainrom", 0 ) // sub-board was hardwired to only be able to address the lower 64MByte, was rewired to also dump upper half when dumping, upper half contains only garbage, hence ROM_IGNORE
	ROM_LOAD( "planes_lexibook.bin", 0x00000, 0x4000000, CRC(76e1a962) SHA1(83b801c0e0e941ceb1c93e565e833b07c09412c3))
	ROM_IGNORE(0x4000000)
ROM_END

ROM_START( red5mam )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "mam.u3", 0x00000, 0x8000000, CRC(0c0a0ecd) SHA1(2dfd8437de17fc9975698f1933dd81fbac78466d) )
ROM_END

ROM_START( lpgm240 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "w25q64jv.u1", 0x00000, 0x800000, CRC(b973e65b) SHA1(36ff137068ea56b4679c2db386ac0067de0a9eaf) )
ROM_END

ROM_START( sy889 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "sy889_w25q64.bin", 0x00000, 0x800000, CRC(fcdaa6fc) SHA1(0493747facf2172b8af22010851668bb18cbb3e4) )
ROM_END

ROM_START( sy888b )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "sy888b_f25q32.bin", 0x00000, 0x400000, CRC(a8298c33) SHA1(7112dd13d5fb5f9f9d496816758defd22773ec6e) )
ROM_END


ROM_START( bittboy )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "bittboy_flash_read_s29gl256n-tf-v2.bin", 0x00000, 0x2000000, CRC(24c802d7) SHA1(c1300ff799b93b9b53060b94d3985db4389c5d3a) )
ROM_END

ROM_START( mc_cb280 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "w25q32.u5", 0x00000, 0x400000, CRC(c9541bdf) SHA1(f0ce46f18658ca5dbed881e5a80460e59820bbd0) )
ROM_END

ROM_START( mc_pg150 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "pocketgames150-in1.bin", 0x00000, 0x2000000, CRC(32f1176b) SHA1(2cfd9b61ebdfc328f020ae9bd5e5e2219321e828) )
ROM_END

ROM_START( mc_hh210 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "msp55lv128t.u4", 0x00000, 0x1000000, CRC(9ba520d4) SHA1(627f811b24314197e289a2ade668ff4115421bed) )
ROM_END

ROM_START( dvnimbus )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "2012-7-4-v1.bin", 0x00000, 0x1000000, CRC(a91d7aa6) SHA1(9421b70b281bb630752bc352c3715258044c0bbe) )
ROM_END

ROM_START( unkra200 ) // "Winbond 25Q64FVSIG 1324" SPI ROM
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "retro_machine_rom", 0x00000, 0x800000, CRC(0e824aa7) SHA1(957e98868559ecc22b3fa42c76692417b76bf132) )
ROM_END


ROM_START( denv150 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "denver150in1.bin", 0x00000, 0x1000000, CRC(6b3819d7) SHA1(b0039945ce44a52ea224ab736d5f3c6980409b5d) ) // 2nd half is blank
ROM_END

ROM_START( mog_m320 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "w25q64fv.bin", 0x00000, 0x800000, CRC(3c5e1b36) SHA1(4bcbf35ebf2b1714ccde5de758a89a6a39528f89) )
ROM_END


ROM_START( fapocket )
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "s29gl512n.bin", 0x00000, 0x4000000, CRC(37d0fb06) SHA1(0146a2fae32e23b65d4032c508f0d12cedd399c3) )
ROM_END

ROM_START( otrail )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "g25q80cw.bin", 0x00000, 0x100000, CRC(b20a03ba) SHA1(c4ca8e590b07baaebed747537bc8f92e44bdd219) ) // dumped as QD25Q80C

	ROM_REGION( 0x200, "seeprom", 0 )
	ROM_LOAD( "t24c04a.bin", 0x000, 0x200, CRC(ce1fad6f) SHA1(82878996765739edba42042b6336460d5c8f8096) )
ROM_END

ROM_START( zonefusn )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "fusion.bin", 0x00000, 0x1000000, CRC(240bf970) SHA1(1b82d95a252c08e52fb8da6320276574a30b60db) )
ROM_END

void nes_vt369_vtunknown_state::init_lxcmcypp()
{
	int size = memregion("mainrom")->bytes()/2;
	uint16_t* ROM = (uint16_t*)memregion("mainrom")->base();

	for (int i = 0; i < size; i++)
	{
		ROM[i] = bitswap<16>(ROM[i], 4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11);
	}
}

// Runs well, only issues in SMB3 which crashes
CONS( 2017, bittboy,    0,        0,  nes_vt369_vtunknown_bt_2x16mb, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init, "BittBoy",   "BittBoy Mini FC 300 in 1", MACHINE_IMPERFECT_GRAPHICS ) // has external banking (2x 16mbyte banks)
// Broken GFX, investigate, is this really a system? research indicates it's a multicart for a regular NES?
CONS( 201?, mc_pg150,   0,        0,  nes_vt369_vtunknown_bt_2x16mb, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init, "<unknown>", "Pocket Games 150 in 1", MACHINE_NOT_WORKING ) // has external banking
// No title screen, but press start and menu and games run fine. Makes odd
// memory accesses which probably explain broken title screen
CONS( 201?, mc_hh210,   0,        0,  nes_vt369_vtunknown_4k_ram_16mb, nes_vt369_vtunknown, nes_vt369_vtunknown_state, empty_init, "<unknown>", "Handheld 210 in 1", MACHINE_NOT_WORKING )
// First half of games don't work, probably bad dump
CONS( 201?, dvnimbus,   0,        0,  nes_vt369_vtunknown_unk_16mb, nes_vt369_vtunknown, nes_vt369_vtunknown_unk_state, empty_init, "<unknown>", "DVTech Nimbus 176 in 1", MACHINE_NOT_WORKING )
 // probably another Thumbs Up product? cursor doesn't work unless nes_vt369_vtunknown_hh machine is used? possibly newer than VT02 as it runs from an SPI ROM, might just not use enhanced features.  Some minor game name changes to above (eg Smackdown just becomes Wrestling)
CONS( 201?, unkra200,   mc_tv200, 0,  nes_vt369_vtunknown_hh_8mb, nes_vt369_vtunknown, nes_vt369_vtunknown_unk_state, empty_init, "<unknown>", "200 in 1 Retro Arcade", MACHINE_IMPERFECT_GRAPHICS )

// is this vt09 or vt32?
// Use DIP switch to select console or cartridge, as cartridge is fake and just toggles a ROM high address bit
// (which can also be overriden by GPIO)
CONS( 2017, fapocket,   0,        0,  nes_vt369_vtunknown_fa_4x16mb, nes_vt369_vtunknown_fa, nes_vt369_vtunknown_dg_fapocket_state, empty_init, "<unknown>",   "Family Pocket 638 in 1", MACHINE_IMPERFECT_GRAPHICS ) // has external banking (4x 16mbyte banks)





// Runs well, minor GFX issues in intro
CONS( 2017, sy889,      0,        0,  nes_vt369_vtunknown_hh_8mb, nes_vt369_vtunknown, nes_vt369_vtunknown_unk_state, empty_init, "SY Corp",   "SY-889 300 in 1 Handheld", MACHINE_IMPERFECT_GRAPHICS )
CONS( 2016, sy888b,     0,        0,  nes_vt369_vtunknown_hh_4mb, nes_vt369_vtunknown, nes_vt369_vtunknown_unk_state, empty_init, "SY Corp",   "SY-888B 288 in 1 Handheld", MACHINE_IMPERFECT_GRAPHICS )

// Same hardware as SY-889
CONS( 201?, mc_cb280,   0,        0,  nes_vt369_vtunknown_hh_4mb, nes_vt369_vtunknown, nes_vt369_vtunknown_unk_state, empty_init, "CoolBoy",   "Coolboy RS-18 (280 in 1)", MACHINE_IMPERFECT_GRAPHICS )

// Plays intro music but then crashes. same hardware as SY-88x but uses more features
CONS( 2016, mog_m320,   0,        0,  nes_vt369_vtunknown_hh_8mb, nes_vt369_vtunknown, nes_vt369_vtunknown_unk_state, empty_init, "MOGIS",    "MOGIS M320 246 in 1 Handheld", MACHINE_NOT_WORKING )

/****************************************************************************************************************

    Things below seem on heavily enhanced hardware of unknown VT type

    It's possible some of these are the same as some of the ones above (sy889, rminitv, dgun2573 etc.) but with
    more features used.

    In some cases these might be almost entirely different, and it is likely a number don't belong in this
    driver at all.

****************************************************************************************************************/

// don't even get to menu. very enhanced chipset, VT368/9?
CONS( 2012, dgun2561,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init, "dreamGEAR", "My Arcade Portable Gaming System with 140 Games (DGUN-2561)", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme

CONS( 200?, lxcmcy,    0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmc250,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - 250-in-1 (JL2375)", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcysw,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Star Wars Rebels", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcyfz,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Frozen", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcydp,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Disney Princess", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcysp,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Marvel Ultimate Spider-Man", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxcmcycr,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Compact Cyber Arcade - Cars", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme
// the data order is swapped for this one, maybe other internal differences?
CONS( 200?, lxcmcypp,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, init_lxcmcypp, "Lexibook", "Lexibook Compact Cyber Arcade - Paw Patrol", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme


CONS( 200?, lxccminn,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Console Colour - Minnie Mouse", MACHINE_NOT_WORKING ) // 64Mbyte (used) ROM, must be externally banked, or different addressing scheme
CONS( 200?, lxccplan,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Console Colour - Disney's Planes", MACHINE_NOT_WORKING ) // 64Mbyte (used) ROM, must be externally banked, or different addressing scheme


// GB-NO13-Main-VT389-2 on PCBs
CONS( 2016, rtvgc300,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Retro TV Game Console - 300 Games", MACHINE_NOT_WORKING )  // 64Mbyte ROM, must be externally banked, or different addressing scheme
CONS( 2017, rtvgc300fz,0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init,    "Lexibook", "Lexibook Retro TV Game Console - Frozen - 300 Games", MACHINE_NOT_WORKING )  // 64Mbyte ROM, must be externally banked, or different addressing scheme


/* The following are also confirmed to be NES/VT derived units, most having a standard set of games with a handful of lazy graphic mods thrown in to fit the unit theme

    (handheld units, use standard AAA batteries)
    Lexibook Compact Cyber Arcade - Barbie
    Lexibook Compact Cyber Arcade - Finding Dory
    Lexibook Compact Cyber Arcade - PJ Masks

    (Handheld units, but different form factor to Compact Cyber Arcade, charged via USB)
    Lexibook Console Colour - Barbie

    (units for use with TV)
    Lexibook Retro TV Game Console (300 Games) - Cars
    Lexibook Retro TV Game Console (300 Games) - PJ Masks

    (more?)

    There are also updated 'Compact Cyber Arcade' branded units with a large + D-pad and internal battery / USB charger for
    Spiderman
    Frozen
    (generic)
    it isn't verified if these use the same ROMs as the original Compact Cyber Arcade releases, or if the software has been updated

*/

// confirmed VT369
CONS( 201?, denv150,   0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init, "Denver", "Denver Game Console GMP-240C 150-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// uncertain, uses SPI ROM
CONS( 200?, lpgm240,    0,  0,  nes_vt369_vtunknown_vh2009_8mb,        nes_vt369_vtunknown, nes_vt369_vtunknown_swap_op_d5_d6_state, empty_init, "<unknown>", "Let's Play! Game Machine 240 in 1", MACHINE_NOT_WORKING ) // mini 'retro-arcade' style cabinet

// incertain, uses SPI ROM
CONS( 2017, otrail,     0,        0,  nes_vt369_vtunknown_unk_1mb, nes_vt369_vtunknown, nes_vt369_vtunknown_unk_state, empty_init, "Basic Fun", "The Oregon Trail", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// uncertain, intial code isn't valid? scrambled?
CONS( 201?, red5mam,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init, "Red5", "Mini Arcade Machine (Red5)", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme
// uncertain, very similar to red5mam
CONS( 2016, dgun2593,  0,  0,  nes_vt369_vtunknown_cy_bigger, nes_vt369_vtunknown, nes_vt369_vtunknown_cy_state, empty_init, "dreamGEAR", "My Arcade Retro Arcade Machine - 300 Handheld Video Games (DGUN-2593)", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme

// uncertain, NOT SPI ROM
CONS( 200?, zonefusn,  0,         0,  nes_vt369_vtunknown_fp_16mb,     nes_vt369_vtunknown, nes_vt369_vtunknown_unk_state, empty_init, "Ultimate Products / Jungle's Soft", "Zone Fusion",  MACHINE_NOT_WORKING )

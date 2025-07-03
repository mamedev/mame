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

  ***************************************************************************/

#include "emu.h"
#include "nes_vt369_vtunknown_soc.h"

namespace {

class vt369_base_state : public driver_device
{
public:
	vt369_base_state(const machine_config& mconfig, device_type type, const char* tag) :
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

	virtual u8 in0_r();
	virtual u8 in1_r();
	virtual void in0_w(u8 data);

	void vt369_map(address_map &map) ATTR_COLD;

	optional_ioport m_io0;
	optional_ioport m_io1;

	u8 m_latch0;
	u8 m_latch1;
	u8 m_previous_port0;

	optional_ioport m_cartsel;
	optional_ioport_array<4> m_exin;

	/* Misc */
	u32 m_ahigh; // external banking bits
	u8 m_4242;
	u8 m_411c;
	u8 m_411d;

	required_region_ptr<u8> m_prgrom;

	u8 vt_rom_r(offs_t offset);
	void configure_soc(nes_vt02_vt03_soc_device* soc);

	void extbank_w(u8 data);
	void extbank_red5mam_w(u8 data);

private:
	/* Extra IO */
	template <u8 NUM> u8 extrain_r();
};

class vt369_state : public vt369_base_state
{
public:
	vt369_state(const machine_config& mconfig, device_type type, const char* tag) :
		vt369_base_state(mconfig, type, tag),
		m_soc(*this, "soc")
	{ }

	void vt_external_space_map_32mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_32mbyte_bank(address_map &map) ATTR_COLD;
	void vt_external_space_map_16mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_8mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_4mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_2mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_1mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_512kbyte(address_map &map) ATTR_COLD;

	void init_lxcmcypp();

protected:
	u8 vt_rom_banked_r(offs_t offset);

	required_device<nes_vt02_vt03_soc_device> m_soc;
};


class vt36x_state : public vt369_state
{
public:
	vt36x_state(const machine_config& mconfig, device_type type, const char* tag) :
		vt369_state(mconfig, type, tag)
	{ }

	void vt36x(machine_config& config);
	void vt36x_1mb(machine_config& config);
	void vt36x_4mb(machine_config& config);
	void vt36x_8mb(machine_config& config);
	void vt36x_16mb(machine_config& config);
	void vt36x_32mb(machine_config& config);
	void vt36x_32mb_2banks_lexi(machine_config& config);
	void vt36x_32mb_2banks_lexi300(machine_config& config);

	void vt36x_swap(machine_config& config);
	void vt36x_swap_2mb(machine_config& config);
	void vt36x_swap_4mb(machine_config& config);
	void vt36x_swap_8mb(machine_config& config);
	void vt36x_swap_16mb(machine_config& config);
	void vt36x_swap_512kb(machine_config& config);

	void vt36x_altswap(machine_config& config);
	void vt36x_altswap_2mb(machine_config& config);
	void vt36x_altswap_4mb(machine_config& config);
	void vt36x_altswap_16mb(machine_config& config);
	void vt36x_altswap_32mb_4banks_red5mam(machine_config& config);

	void vt36x_vibesswap_16mb(machine_config& config);
	void vt36x_gbox2020_16mb(machine_config& config);

	void vt369_unk(machine_config& config);
	void vt369_unk_1mb(machine_config& config);
	void vt369_unk_16mb(machine_config& config);

};

class vt36x_tetrtin_state : public vt36x_state
{
public:
	vt36x_tetrtin_state(const machine_config& mconfig, device_type type, const char* tag) :
		vt36x_state(mconfig, type, tag)
	{ }

protected:
	virtual void machine_reset() override;

};

void vt36x_tetrtin_state::machine_reset()
{
	vt36x_state::machine_reset();
	// the game appears to require code/data from an additional device (not just the standard internal ROM)
	// there's an 8-pin chip on the PCB which is likely responsible

	// simulate what that code might be doing
	// copy VT369 internal ROM 0x0000 to 0x4ff4 in CPU space (copying boot vectors for sound CPU, as other games do in code)
	int src = 0;
	u8 *introm = memregion("soc:internal")->base();
	for (int i = 0x4ff4; i < 0x5000; i++)
	{
		m_soc->write_byte_to_cpu(i, introm[src++]);
	}
	u8* gamerom = memregion("mainrom")->base();

	int patchaddress;


	// tetrtin - jump over a whole lot of code, this is crude, there might be other code still in the startup we could be executing
	patchaddress = 0x7f675;
	if ((gamerom[patchaddress] == 0x20) && (gamerom[patchaddress+1] == 0xcb) && (gamerom[patchaddress+2] == 0xf5))
	{
		gamerom[patchaddress] = 0x4c;
		gamerom[patchaddress+1] = 0xcb;
		gamerom[patchaddress+2] = 0xf6;
	}
	// same for pactin
	patchaddress = 0x7f5a3;
	if ((gamerom[patchaddress] == 0x20) && (gamerom[patchaddress+1] == 0x04) && (gamerom[patchaddress+2] == 0xf5))
	{
		gamerom[patchaddress] = 0x4c;
		gamerom[patchaddress+1] = 0xf9;
		gamerom[patchaddress+2] = 0xf5;
	}
	// lxcap (will show menu, but accesses device again afterwards)
	patchaddress = 0x7ecd4;
	if ((gamerom[patchaddress] == 0x20) && (gamerom[patchaddress+1] == 0x96) && (gamerom[patchaddress+2] == 0xeb))
	{
		gamerom[patchaddress] = 0x4c;
		gamerom[patchaddress+1] = 0x2a;
		gamerom[patchaddress+2] = 0xed;
	}
}

u8 vt369_base_state::vt_rom_r(offs_t offset)
{
	return m_prgrom[offset];
}

u8 vt369_state::vt_rom_banked_r(offs_t offset)
{
	return m_prgrom[m_ahigh | offset];
}

// VTxx can address 25-bit address space (32MB of ROM) so use maps with mirroring in depending on ROM size
void vt369_state::vt_external_space_map_32mbyte(address_map &map)
{
	map(0x0000000, 0x1ffffff).r(FUNC(vt369_state::vt_rom_r));
}

void vt369_state::vt_external_space_map_32mbyte_bank(address_map &map)
{
	map(0x0000000, 0x1ffffff).r(FUNC(vt369_state::vt_rom_banked_r));
}

void vt369_state::vt_external_space_map_16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(vt369_state::vt_rom_r));
}

void vt369_state::vt_external_space_map_8mbyte(address_map &map)
{
	map(0x0000000, 0x07fffff).mirror(0x1800000).r(FUNC(vt369_state::vt_rom_r));
}

void vt369_state::vt_external_space_map_4mbyte(address_map &map)
{
	map(0x0000000, 0x03fffff).mirror(0x1c00000).r(FUNC(vt369_state::vt_rom_r));
}

void vt369_state::vt_external_space_map_2mbyte(address_map &map)
{
	map(0x0000000, 0x01fffff).mirror(0x1e00000).r(FUNC(vt369_state::vt_rom_r));
}

void vt369_state::vt_external_space_map_1mbyte(address_map &map)
{
	map(0x0000000, 0x00fffff).mirror(0x1f00000).r(FUNC(vt369_state::vt_rom_r));
}

void vt369_state::vt_external_space_map_512kbyte(address_map &map)
{
	map(0x0000000, 0x007ffff).mirror(0x1f80000).r(FUNC(vt369_state::vt_rom_r));
}

template <u8 NUM> u8 vt369_base_state::extrain_r()
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

u8 vt369_base_state::in0_r()
{
	//logerror("%s: in0_r\n", machine().describe_context());
	u8 ret = 0x40;
	ret |= m_latch0 & 1;
	m_latch0 >>= 1;
	return ret;
}

u8 vt369_base_state::in1_r()
{
	//logerror("%s: in1_r\n", machine().describe_context());
	u8 ret = 0x40;
	ret |= m_latch1 & 1;
	m_latch1 >>= 1;
	return ret;
}

void vt369_base_state::in0_w(u8 data)
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


void vt369_base_state::machine_start()
{
	m_latch0 = 0;
	m_latch1 = 0;
	m_previous_port0 = 0;

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

void vt369_base_state::machine_reset()
{
	m_ahigh = 0;
}

void vt369_base_state::configure_soc(nes_vt02_vt03_soc_device* soc)
{
	soc->set_addrmap(AS_PROGRAM, &vt369_state::vt_external_space_map_32mbyte);
	soc->read_0_callback().set(FUNC(vt369_base_state::in0_r));
	soc->read_1_callback().set(FUNC(vt369_base_state::in1_r));
	soc->write_0_callback().set(FUNC(vt369_base_state::in0_w));

	soc->extra_read_0_callback().set(FUNC(vt369_base_state::extrain_r<0>));
	soc->extra_read_1_callback().set(FUNC(vt369_base_state::extrain_r<1>));
	soc->extra_read_2_callback().set(FUNC(vt369_base_state::extrain_r<2>));
	soc->extra_read_3_callback().set(FUNC(vt369_base_state::extrain_r<3>));
}




void vt36x_state::vt369_unk(machine_config &config)
{
	VT3XX_SOC_UNK_DG(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->force_bad_dma();
}

void vt36x_state::vt369_unk_16mb(machine_config& config)
{
	vt369_unk(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_16mbyte);
}

void vt36x_state::vt369_unk_1mb(machine_config& config)
{
	vt369_unk(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_1mbyte);
}


// New mystery handheld architecture, VTxx derived
void vt36x_state::vt36x(machine_config &config)
{
	VT369_SOC_INTROM_NOSWAP(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB);
	m_soc->force_bad_dma();
}

void vt36x_state::vt36x_swap(machine_config &config)
{
	VT369_SOC_INTROM_SWAP(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB);
	m_soc->force_bad_dma();
}

void vt36x_state::vt36x_swap_512kb(machine_config &config)
{
	vt36x_swap(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_512kbyte);
}

void vt36x_state::vt36x_swap_2mb(machine_config &config)
{
	vt36x_swap(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_2mbyte);
}

void vt36x_state::vt36x_swap_4mb(machine_config &config)
{
	vt36x_swap(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_4mbyte);
}

void vt36x_state::vt36x_swap_8mb(machine_config &config)
{
	vt36x_swap(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_8mbyte);
}

void vt36x_state::vt36x_swap_16mb(machine_config &config)
{
	vt36x_swap(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_16mbyte);
}

void vt36x_state::vt36x_altswap(machine_config &config)
{
	VT369_SOC_INTROM_ALTSWAP(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB);
	m_soc->force_bad_dma();
}

void vt36x_state::vt36x_altswap_2mb(machine_config &config)
{
	vt36x_altswap(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_2mbyte);
}

void vt36x_state::vt36x_altswap_4mb(machine_config &config)
{
	vt36x_altswap(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_4mbyte);
}

void vt36x_state::vt36x_altswap_16mb(machine_config& config)
{
	vt36x_altswap(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_16mbyte);
}

void vt369_base_state::extbank_red5mam_w(u8 data)
{
//  printf("extbank_red5mam_w %02x\n", data);
	m_ahigh = ((data & 0x03) << 25);
}

void vt36x_state::vt36x_altswap_32mb_4banks_red5mam(machine_config& config)
{
	vt36x_altswap(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_32mbyte_bank);
	m_soc->set_41e6_write_cb().set(FUNC(vt36x_state::extbank_red5mam_w));
}

void vt36x_state::vt36x_vibesswap_16mb(machine_config &config)
{
	vt36x_swap_16mb(config);

	VT369_SOC_INTROM_VIBESSWAP(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	//m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB);
	m_soc->force_bad_dma();
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_16mbyte);
}

void vt36x_state::vt36x_gbox2020_16mb(machine_config &config)
{
	vt36x_swap_16mb(config);

	VT369_SOC_INTROM_GBOX2020(config.replace(), m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);
	m_soc->set_default_palette_mode(PAL_MODE_NEW_RGB);
	m_soc->force_bad_dma();
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_16mbyte);
}


void vt36x_state::vt36x_1mb(machine_config& config)
{
	vt36x(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_1mbyte);
}

void vt36x_state::vt36x_4mb(machine_config& config)
{
	vt36x(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_4mbyte);
}

void vt36x_state::vt36x_8mb(machine_config& config)
{
	vt36x(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_8mbyte);
}

void vt36x_state::vt36x_16mb(machine_config& config)
{
	vt36x(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_16mbyte);
}

void vt36x_state::vt36x_32mb(machine_config& config)
{
	vt36x(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_32mbyte);
}

void vt369_base_state::extbank_w(u8 data)
{
	m_ahigh = (data & 0x01) ? (1 << 25) : 0x0;
}

void vt36x_state::vt36x_32mb_2banks_lexi(machine_config& config)
{
	vt36x_32mb(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_32mbyte_bank);
	m_soc->set_4150_write_cb().set(FUNC(vt36x_state::extbank_w));
}

void vt36x_state::vt36x_32mb_2banks_lexi300(machine_config& config)
{
	vt36x_32mb(config);
	m_soc->set_addrmap(AS_PROGRAM, &vt36x_state::vt_external_space_map_32mbyte_bank);
	m_soc->set_411e_write_cb().set(FUNC(vt36x_state::extbank_w)); // could be on 411d
}

static INPUT_PORTS_START( vt369 )
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

static INPUT_PORTS_START( vt369_rot )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY

	PORT_START("IO1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("B")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
INPUT_PORTS_END

// internal ROMs - these seem to be generic, but that isn't yet verified, if they are move them to device
//
// maps at 0x1000-0x1fff on main CPU, and can boot using vectors in 1ffx area
// can also be mapped at 0x4000-0x4fff on the sound CPU, typically when this is
// done the main CPU fetch the vectors from 0x4000 and writes them to the RAM
// shared with the sound CPU vector area before enabling the sound CPU

#define VT3XX_INTERNAL_NO_SWAP \
	ROM_REGION( 0x1000, "soc:internal", 0 ) \
	ROM_LOAD( "internal.bin", 0x0000, 0x1000, CRC(da5850f0) SHA1(39d674d965818922aad5993e9499170d3ebc43bf) )

#define VT3XX_INTERNAL_OPCODE_SWAP \
	ROM_REGION( 0x1000, "soc:internal", 0 ) \
	ROM_LOAD( "internal.bin", 0x0000, 0x1000, CRC(57c9cea9) SHA1(4f338e5ef87a66601014ad726cfefefbc20dc4be) )

// below use Flash ROMs

ROM_START( dgun2593 )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "dreamgear300.bin", 0x00000, 0x8000000, CRC(4fe0ed02) SHA1(a55590557bacca65ed9a17c5bcf0a4e5cb223126) )
ROM_END

ROM_START( 240in1ar )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "mw-106-2g.u3", 0x00000, 0x8000000, CRC(c46d2ca9) SHA1(0fff7d3461ff620c5b5e43f54f9e7badd089b951) )
ROM_END


ROM_START( rtvgc300 )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	// some of the higher address lines might be swapped
	ROM_LOAD( "lexibook300.bin", 0x00000, 0x0800000, CRC(015c4067) SHA1(a12986c4a366a23c4c7ca7b3d33e421a8dfdffc0) )
	ROM_CONTINUE(0x1000000, 0x0800000)
	ROM_CONTINUE(0x0800000, 0x0800000)
	ROM_CONTINUE(0x1800000, 0x0800000)
	ROM_CONTINUE(0x2000000, 0x0800000)
	ROM_CONTINUE(0x3000000, 0x0800000)
	ROM_CONTINUE(0x2800000, 0x0800000)
	ROM_CONTINUE(0x3800000, 0x0800000)

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( rtvgc300fz )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	// some of the higher address lines might be swapped
	ROM_LOAD( "jg7800fz.bin", 0x00000, 0x4000000, CRC(c9d319d2) SHA1(9d0d1435b802f63ce11b94ce54d11f4065b324cc) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END


ROM_START( dgun2561 ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "dgun2561.bin", 0x00000, 0x4000000, CRC(a6e627b4) SHA1(2667d2feb02de349387f9dcfa5418e7ed3afeef6) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxccatv ) // all games selectable
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "120n1.bin", 0x00000, 0x2000000, CRC(6b9cf537) SHA1(44276c3ef928c76a3ecf404d2e531cd3ce5561af) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcy ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "lxcmcy.bin", 0x00000, 0x4000000, CRC(3f3af72c) SHA1(76127054291568fcce1431d21af71f775cfb05a6) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcysw ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "jl2365swr-1.u2", 0x0000000, 0x0800000, CRC(60ece391) SHA1(655de6b36ba596d873de2839522b948ccf45e006) )
	ROM_CONTINUE(0x1000000, 0x0800000)
	ROM_CONTINUE(0x0800000, 0x0800000)
	ROM_CONTINUE(0x1800000, 0x0800000)
	ROM_CONTINUE(0x2000000, 0x0800000)
	ROM_CONTINUE(0x3000000, 0x0800000)
	ROM_CONTINUE(0x2800000, 0x0800000)
	ROM_CONTINUE(0x3800000, 0x0800000)

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcyfz ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "jl2365_frozen.u1", 0x00000, 0x0800000, CRC(64d4c708) SHA1(1bc2d161326ce3039ab9ba46ad62695060cfb2e1) )
	ROM_CONTINUE(0x1000000, 0x0800000)
	ROM_CONTINUE(0x0800000, 0x0800000)
	ROM_CONTINUE(0x1800000, 0x0800000)
	ROM_CONTINUE(0x2000000, 0x0800000)
	ROM_CONTINUE(0x3000000, 0x0800000)
	ROM_CONTINUE(0x2800000, 0x0800000)
	ROM_CONTINUE(0x3800000, 0x0800000)

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcydp ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "cyberarcade-disneyprincess.bin", 0x00000, 0x4000000, CRC(05946f81) SHA1(33eea2b70f5427e7613c836b8a08148731fac231) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcysp ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "lexibookspiderman.bin", 0x00000, 0x4000000, CRC(ef6e8847) SHA1(0012df193c52fd48595d85886fd431619c5d5e3e) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcycr ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "lexibook cars.bin", 0x00000, 0x4000000, CRC(198fe11b) SHA1(5e35caa3fc319ec69812c187a3ec89f01749f749) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcypj ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 1GB capacity (A0-A25 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "cob66-1g-new02.u4", 0x00000, 0x0800000, CRC(78149671) SHA1(00dab8c0919e909e910525c18142e6a195b364f8) )
	ROM_CONTINUE(0x1000000, 0x0800000)
	ROM_CONTINUE(0x0800000, 0x0800000)
	ROM_CONTINUE(0x1800000, 0x0800000)
	ROM_CONTINUE(0x2000000, 0x0800000)
	ROM_CONTINUE(0x3000000, 0x0800000)
	ROM_CONTINUE(0x2800000, 0x0800000)
	ROM_CONTINUE(0x3800000, 0x0800000)

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcyba ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "barbie.bin", 0x00000, 0x4000000, CRC(e38af9d0) SHA1(a978a4da61f007c152c70233e9628dbebb427743) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcybt ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "batman.bin", 0x00000, 0x4000000, CRC(9f8f15ce) SHA1(396122ce68008e9c8f35b98f5246e8dc7725df17) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcydpn ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "dp150.bin", 0x00000, 0x4000000, CRC(dce19f81) SHA1(e74190d5eea4c31ec0cdcc374b988db2dc1d37c6) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcyspn ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "lexi_sp_cca_150.u2", 0x00000, 0x4000000, CRC(f57ee9cf) SHA1(4c9a322439f0c255845167e5a2e3762e56665c4e) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmcypp ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// marked 512mbit, possible A22 / A23 are swapped as they were marked on the board in a different way.
	ROM_LOAD( "pawpatrol_compact.bin", 0x00000, 0x4000000, CRC(bf536762) SHA1(80dde8426a636bae33a82d779e564fa743eb3776) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcypkdp ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "ddp.u2", 0x00000, 0x4000000, CRC(ac5ce022) SHA1(450d11886385aeadc81e62090acd1d8ef8fedcd8) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcypksp ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	ROM_LOAD( "pocketspiderman.u2", 0x00000, 0x4000000, CRC(3e1af689) SHA1(e2ca78c35cd6d827928cf284ea3dcf8b397d347c) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxcmc250 ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 )
	// sub-board was marked for 2GB capacity (A0-A26 address lines), but only address lines A0-A24 are connected to the chip
	ROM_LOAD( "cca250in1.u1", 0x00000, 0x0800000, CRC(6ccd6ad6) SHA1(fafed339097c3d1538faa306021a8373c1b799b3) )
	ROM_CONTINUE(0x1000000, 0x0800000)
	ROM_CONTINUE(0x0800000, 0x0800000)
	ROM_CONTINUE(0x1800000, 0x0800000)
	ROM_CONTINUE(0x2000000, 0x0800000)
	ROM_CONTINUE(0x3000000, 0x0800000)
	ROM_CONTINUE(0x2800000, 0x0800000)
	ROM_CONTINUE(0x3800000, 0x0800000)

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxccminn ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 ) // sub-board was hardwired to only be able to address the lower 64MByte, was rewired to also dump upper half when dumping, upper half contains only garbage, hence ROM_IGNORE
	ROM_LOAD( "minnie_lexibook.bin", 0x00000, 0x4000000, CRC(3f8e5a69) SHA1(c9f11f3e5f9b73832a191f4d1620a85c1b70f79e) )
	ROM_IGNORE(0x4000000)

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( lxccplan ) // all games selectable
	ROM_REGION( 0x4000000, "mainrom", 0 ) // sub-board was hardwired to only be able to address the lower 64MByte, was rewired to also dump upper half when dumping, upper half contains only garbage, hence ROM_IGNORE
	ROM_LOAD( "planes_lexibook.bin", 0x00000, 0x4000000, CRC(76e1a962) SHA1(83b801c0e0e941ceb1c93e565e833b07c09412c3))
	ROM_IGNORE(0x4000000)

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END


ROM_START( red5mam )
	ROM_REGION( 0x8000000, "mainrom", 0 )
	ROM_LOAD( "mam.u3", 0x00000, 0x8000000, CRC(0c0a0ecd) SHA1(2dfd8437de17fc9975698f1933dd81fbac78466d) )
ROM_END

ROM_START( nubsupmf )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "w25q32fv.bin", 0x00000, 0x400000,  CRC(5ca234b2) SHA1(3eba3e690f68116fd3e5e914f8bd16b1dc2c0bc4) )
ROM_END

ROM_START( 36pcase )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "25q16.ic3", 0x00000, 0x200000, CRC(a8edb73e) SHA1(1028656530e411607ffa3b63788b42e41bf971d7) )
ROM_END


ROM_START( dvnimbus )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "2012-7-4-v1.bin", 0x00000, 0x1000000, CRC(a91d7aa6) SHA1(9421b70b281bb630752bc352c3715258044c0bbe) )
ROM_END

ROM_START( zonefusn ) // all games selectable
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "fusion.bin", 0x00000, 0x1000000, CRC(240bf970) SHA1(1b82d95a252c08e52fb8da6320276574a30b60db) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( sealvt ) // all games selectable
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "l157-44 v02.u1", 0x00000, 0x1000000, CRC(0fabced0) SHA1(3f8cd85b12b125b01c831c9f2f2937e29c1b6205) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( gcs2mgp ) // all games selectable
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "gcs2_v4.u3", 0x00000, 0x1000000, CRC(3b5be765) SHA1(c54f1a732d638b0ee582ca822715c9d3a3af5ef3) )
ROM_END

// VT369 using SPI ROMs

ROM_START( lpgm240 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "w25q64jv.u1", 0x00000, 0x800000, CRC(b973e65b) SHA1(36ff137068ea56b4679c2db386ac0067de0a9eaf) )

	VT3XX_INTERNAL_OPCODE_SWAP
ROM_END

ROM_START( tup240 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "mini_arcade240.bin", 0x00000, 0x800000, CRC(d4b4bf6c) SHA1(9cf4557e27bc8659079c62abdd22a311e1843047) )

	VT3XX_INTERNAL_OPCODE_SWAP
ROM_END

ROM_START( sy889 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "sy889_w25q64.bin", 0x00000, 0x800000, CRC(fcdaa6fc) SHA1(0493747facf2172b8af22010851668bb18cbb3e4) )
ROM_END

ROM_START( sy888b )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "sy888b_f25q32.bin", 0x00000, 0x400000, CRC(a8298c33) SHA1(7112dd13d5fb5f9f9d496816758defd22773ec6e) )
ROM_END

ROM_START( mc_cb280 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "w25q32.u5", 0x00000, 0x400000, CRC(c9541bdf) SHA1(f0ce46f18658ca5dbed881e5a80460e59820bbd0) )
ROM_END

ROM_START( unkra200 ) // "Winbond 25Q64FVSIG 1324" SPI ROM
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "retro_machine_rom", 0x00000, 0x800000, CRC(0e824aa7) SHA1(957e98868559ecc22b3fa42c76692417b76bf132) )
ROM_END

ROM_START( dgun2577 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "blackarcade_dump_dreambook-my_arcade.bin", 0x00000, 0x800000, CRC(9b95b912) SHA1(573c938a0f1acca8f3b75900fd0185bfe28d4fa5) )
ROM_END

ROM_START( lxcyber )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "lexibook_dump_correct.bin", 0x00000, 0x800000, CRC(74b71846) SHA1(e7dcfa7c53cc7d30678763c6e60f7a3250768849) )
ROM_END

ROM_START( gtct885 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "ct-885 g25q64c.bin", 0x00000, 0x800000, CRC(a5b2b568) SHA1(79de79364fa731e421627ec68e3bfa9d311aa7fc) )

	ROM_REGION( 0x100, "extra", 0 ) // data from additional 8-pin chip for protection
	ROM_LOAD( "mystery chip.bin", 0x00000, 0x100, CRC(8173c1c2) SHA1(7521a4676166a81a79209638491026b2d8e32895) )
ROM_END

ROM_START( rd5_240 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "red5.bin", 0x00000, 0x800000, CRC(0e564e73) SHA1(c29a927c830ab3876e9b63e2d41bef962c05518f) )

	ROM_REGION( 0x100, "extra", 0 ) // data from additional 8-pin chip for protection
	ROM_LOAD( "mystery chip.bin", 0x00000, 0x100, NO_DUMP )
ROM_END

ROM_START( myarccn )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "my_arcade_caveman_ninja.bin", 0x00000, 0x100000, CRC(dcc5590c) SHA1(a734cb9c81e58346ff5fa934347d7cb24a32cb39) )

	VT3XX_INTERNAL_NO_SWAP // verified for this set
ROM_END

ROM_START( hkb502 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "red console.bin", 0x00000, 0x400000, CRC(e4766383) SHA1(64b0c20592f38928b3a639fa42b468ff09664808) )

	VT3XX_INTERNAL_NO_SWAP // verified for this set
ROM_END

ROM_START( hkb502a )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "hkb-502.bin", 0x00000, 0x400000, CRC(970f54d2) SHA1(b45df00d85a2e29fe9418563927584a048db94b3) )

	VT3XX_INTERNAL_NO_SWAP // verified for this set
ROM_END

ROM_START( lxcap )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "lexibook_cyber_arcade_pocket.bin", 0x00000, 0x800000, CRC(245d0cd3) SHA1(d91cca2d0f99a6ca202fa9ba6d03587ea8af0cd9) )

	VT3XX_INTERNAL_NO_SWAP // verified for this set

	ROM_REGION( 0x100, "extra", 0 ) // data from additional 8-pin chip for protection
	ROM_LOAD( "mystery chip.bin", 0x00000, 0x100, CRC(491d206b) SHA1(a5411a7afe3b4df93b1b22e5533f5010bd3aaa93) )
ROM_END

ROM_START( denv150 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "denver150in1.bin", 0x00000, 0x800000, CRC(6b3819d7) SHA1(b0039945ce44a52ea224ab736d5f3c6980409b5d) )
	ROM_IGNORE(0x800000) // 2nd half is unused
ROM_END

ROM_START( egame150 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "rom.bin", 0x00000, 0x800000, CRC(a19644ea) SHA1(01c004d126edf792f71c1e9ed98b3c96d9278a69) )
ROM_END

ROM_START( mog_m320 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "w25q64fv.bin", 0x00000, 0x800000, CRC(3c5e1b36) SHA1(4bcbf35ebf2b1714ccde5de758a89a6a39528f89) )
ROM_END

ROM_START( otrail )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "g25q80cw.bin", 0x00000, 0x100000, CRC(b20a03ba) SHA1(c4ca8e590b07baaebed747537bc8f92e44bdd219) ) // dumped as QD25Q80C

	ROM_REGION( 0x200, "seeprom", 0 )
	ROM_LOAD( "t24c04a.bin", 0x000, 0x200, CRC(ce1fad6f) SHA1(82878996765739edba42042b6336460d5c8f8096) )
ROM_END

ROM_START( pactin )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "25q80a.u3", 0x00000, 0x100000, CRC(92935759) SHA1(2333e7dcab51fa34c8d875374371854121fff27a) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( tetrtin )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "25q80.u3", 0x00000, 0x100000, CRC(017a99b9) SHA1(e7f891762bbc3b80ae0f177654d8d066b7524bcd) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END


// GC31-369-20210702-V2 on PCB
ROM_START( unk128vt )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "w25q32.bin", 0x00000, 0x400000, CRC(35ccadf6) SHA1(80b25e374a097d1b9380b7e64013d7ac0d5aa2ca) )
ROM_END

ROM_START( 168pcase )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "25q32.u7", 0x000000, 0x400000, CRC(98e8e97a) SHA1(fd516ef2819a597130f5f7ace9a7838cb99ab08a) )
ROM_END

ROM_START( gbox2020 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "fgb2020.bin", 0x00000, 0x1000000, CRC(a685d943) SHA1(9b272daccd8fe244c910f031466a4fedd83d5236) ) // flash ROM
ROM_END

ROM_START( vibes240 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	// wouldn't read consistently
	ROM_LOAD( "s29gl128p11tfi01.bin", 0x000000, 0x1000000, BAD_DUMP CRC(7244d6e9) SHA1(951052f6cd8c873f85f79be9d64498a43e92fd10) )
	ROM_IGNORE(0x100)
ROM_END

ROM_START( lexi30 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "lexi30.u3", 0x00000, 0x800000, CRC(0d4307ea) SHA1(0d7cf492f796b0bb871deebaca38a3ff3b2ed1e6) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( matet10 )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "tetriskeychain_p25q16sh_856015.bin", 0x00000, 0x200000, CRC(7a7251ea) SHA1(7ace8482a54f6b06982a90328779c21266d864fa) )
	ROM_IGNORE(0x300)
ROM_END

ROM_START( matetsl )
	ROM_REGION( 0x80000, "mainrom", 0 )
	ROM_LOAD( "slurpeetetris_p25q40sh_856013.bin", 0x00000, 0x80000, CRC(d3b68de8) SHA1(97bcdfcd31bc536b626f9a369afe18de60a399da) )
	ROM_IGNORE(0x300)
ROM_END

ROM_START( nesvt270 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "w25q128jvs.u3", 0x00000, 0x1000300, CRC(fe189a90) SHA1(7f07ae89ae7ff49f139e936b08c9ef2a3467ea92) )
ROM_END

ROM_START( rbbrite )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "coleco_rainbowbrite_29dl800ba_000422cb.bin", 0x00000, 0x100000, CRC(d2ad0d7d) SHA1(4423a5aa2eda20b3621ab46e951ac08dc2d24789) )

	VT3XX_INTERNAL_NO_SWAP // not verified for this set, used for testing
ROM_END

ROM_START( mc_89in1 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "89in1.bin", 0x00000, 0x400000, CRC(b97f8ce5) SHA1(1a8e67f2b58a671ceec2b0ed18ec5954a71ae63a) )
ROM_END

ROM_START( mc_110cb )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "29w320dt.bin", 0x00000, 0x400000, CRC(a4bed7eb) SHA1(f1aa89916264ba781d3f1390a2336ef42129b607) )
ROM_END

ROM_START( mc_138cb )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "138-in-1 coolbaby, coolboy rs-5, pcb060-10009011v1.3.bin", 0x00000, 0x400000, CRC(6b5b1a1a) SHA1(2df0cd717bd0de0b0c973ac356426ddbb0d736fa) )
ROM_END

ROM_START( jl2050 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "jl2050.u5", 0x00000, 0x1000000, CRC(f96c5c02) SHA1(c7d0b57c2622b5213d3c7e6532495d9da74d4b01) )
ROM_END


void vt369_state::init_lxcmcypp()
{
	int size = memregion("mainrom")->bytes()/2;
	u16* ROM = (u16*)memregion("mainrom")->base();

	for (int i = 0; i < size; i++)
	{
		ROM[i] = bitswap<16>(ROM[i], 4, 5, 6, 7, 0, 1, 2, 3, 12, 13, 14, 15, 8, 9, 10, 11);
	}
}

} // anonymous namespace


// Might not be VT369
// First half of games don't work, probably bad dump
CONS( 201?, dvnimbus,   0,        0,  vt369_unk_16mb, vt369, vt36x_state, empty_init, "<unknown>", "DVTech Nimbus 176 in 1", MACHINE_NOT_WORKING )


/****************************************************************************************************************

    Things below seem on heavily enhanced hardware of unknown VT type

    It's possible some of these are the same as some of the ones above (sy889, rminitv, dgun2573 etc.) but with
    more features used.

    In some cases these might be almost entirely different, and it is likely a number don't belong in this
    driver at all.

****************************************************************************************************************/

CONS( 2012, lexi30,  0,0,  vt36x_8mb, vt369_rot, vt36x_state, empty_init, "Lexibook", "Arcade Center (JL1800_01)", MACHINE_NOT_WORKING | ROT270 )


CONS( 2012, lxccatv,   0,  0,  vt36x_32mb, vt369, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade TV - 120 in 1 (JL2370)", MACHINE_NOT_WORKING ) // 32MByte ROM, 2011 on case, 2012 on PCB

// All Lexibook units below have 64Mbyte ROMs, must be externally banked, or different addressing scheme
CONS( 2012, lxcmcysp,  0,  0,  vt36x_32mb_2banks_lexi, vt369_rot, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade - Spider-Man (120-in-1)", MACHINE_NOT_WORKING | ROT270) // renders vertically, but screen stretches it to horizontal
CONS( 200?, lxcmc250,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade - 250-in-1 (JL2375)", MACHINE_NOT_WORKING )
CONS( 2012, lxcmcydp,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade - Disney Princess (120-in-1)", MACHINE_NOT_WORKING )
// JL2365 models (150-in-1 versions)
CONS( 200?, lxcmcysw,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade - Star Wars Rebels (JL2365SW)", MACHINE_NOT_WORKING )
CONS( 200?, lxcmcyfz,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade - Frozen (JL2365FZ)", MACHINE_NOT_WORKING )
CONS( 2018, lxcmcypj,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade - PJ Masks (JL2365PJM)", MACHINE_NOT_WORKING )
CONS( 2014, lxcmcyba,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade - Barbie (JL2365BB)", MACHINE_NOT_WORKING )
CONS( 2014, lxcmcycr,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade - Cars (JL2365DC)", MACHINE_NOT_WORKING )
// JL2367 models (150-in-1 versions, newer case style) - the data order is swapped for these (is this common to the JL2367 shell types?)
CONS( 2018, lxcmcypp,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, init_lxcmcypp, "Lexibook", "Compact Cyber Arcade - Paw Patrol (JL2367PA)", MACHINE_NOT_WORKING )
CONS( 2020, lxcmcybt,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, init_lxcmcypp, "Lexibook", "Compact Cyber Arcade - Batman (JL2367BAT)", MACHINE_NOT_WORKING )
CONS( 2014, lxcmcydpn, 0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, init_lxcmcypp, "Lexibook", "Compact Cyber Arcade - Disney Princess (JL2367DP, 150-in-1)", MACHINE_NOT_WORKING )
CONS( 2014, lxcmcyspn, 0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, init_lxcmcypp, "Lexibook", "Compact Cyber Arcade - Spider-Man (JL2367SP, 150-in-1)", MACHINE_NOT_WORKING )

// JL1895 models, Cyber Arcade Pocket.  This make strange use of the LCDC, the menus are vertical (so must be copied to the LCD rotated) but the games are horizontal as usual
CONS( 201?, lxcypkdp,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Cyber Arcade Pocket - Disney Princess (JL1895DP)", MACHINE_NOT_WORKING )
CONS( 201?, lxcypksp,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Cyber Arcade Pocket - Spider-Man (JL1895SP-2)", MACHINE_NOT_WORKING )

CONS( 200?, lxccminn,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Console Colour - Minnie Mouse (JL2800MN)", MACHINE_NOT_WORKING )
CONS( 200?, lxccplan,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Console Colour - Disney's Planes (JL2800PL)", MACHINE_NOT_WORKING )
// similar menus to the lxccminn/lxccplan sets
CONS( 2013, lxcmcy,    0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init,    "Lexibook", "Compact Cyber Arcade - 200 in 1 (JL2355)", MACHINE_NOT_WORKING )
CONS( 2012, dgun2561,  0,  0,  vt36x_32mb_2banks_lexi, vt369, vt36x_state, empty_init, "dreamGEAR", "My Arcade Portable Gaming System with 140 Games (DGUN-2561)", MACHINE_NOT_WORKING ) // 64Mbyte ROM, must be externally banked, or different addressing scheme

// GB-NO13-Main-VT389-2 on PCBs - uses higher resolution mode (twice usual h-res?)
CONS( 2016, rtvgc300,  0,  0,  vt36x_32mb_2banks_lexi300, vt369, vt36x_state, empty_init,    "Lexibook", "Retro TV Game Console - 300 Games", MACHINE_NOT_WORKING )
CONS( 2017, rtvgc300fz,0,  0,  vt36x_32mb_2banks_lexi300, vt369, vt36x_state, empty_init,    "Lexibook", "Retro TV Game Console - Frozen - 300 Games", MACHINE_NOT_WORKING )


/* The following are also confirmed to be NES/VT derived units, most having a standard set of games with a handful of lazy graphic mods thrown in to fit the unit theme

    (handheld units, use standard AAA batteries)
    Lexibook Compact Cyber Arcade - Finding Dory

    (handheld units, use standard AAA batteries, smaller display)
    Lexibook Compact Cyber Arcade Pocket - Paw Patrol
    Lexibook Compact Cyber Arcade Pocket - Frozen

    (Handheld units, but different form factor to Compact Cyber Arcade, charged via USB, different menus)
    Lexibook Console Colour - Barbie

    (Handheld units, charged via USB-C, more educational focused, contain bootleg NES Pinball game in games section)
    Power Console - Gabby's Dollhouse
    Power Console - Disney Princess
    Power Console - Stitch
    Power Console - Frozen
    Power Console - Generic EN/FR model
    Power Console - Generic EN/ES model
    Power Console - Generic EN/DE model
    Power Console - Paw Patrol

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

// uncertain, NOT SPI ROM
CONS( 200?, zonefusn,  0,         0,  vt36x_16mb,     vt369, vt36x_state, empty_init, "Ultimate Products / Jungle's Soft", "Zone Fusion",  MACHINE_NOT_WORKING )
// same as above but without Jungle's Soft boot logo? model number taken from cover of manual
CONS( 200?, sealvt,    zonefusn,  0,  vt36x_16mb,     vt369, vt36x_state, empty_init, "Lexibook / Sit Up Limited / Jungle's Soft", "Seal 30-in-1 (VT based, Model FN098134)",  MACHINE_NOT_WORKING )

// NOT SPI roms, altswap sets code starts with '6a'

CONS( 201?, red5mam,  0,  0,  vt36x_altswap_32mb_4banks_red5mam, vt369, vt36x_state, empty_init, "Red5", "Mini Arcade Machine (Red5, 'Xtra Game')", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme

CONS( 2016, dgun2593,  0,  0,  vt36x_altswap_32mb_4banks_red5mam, vt369, vt36x_state, empty_init, "dreamGEAR", "My Arcade Retro Arcade Machine - 300 Handheld Video Games (DGUN-2593)", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme

CONS( 200?, gcs2mgp,   0,  0,  vt36x_altswap_16mb, vt369_rot, vt36x_state, empty_init, "Jungle's Soft", "Mini Game Player 48-in-1",  MACHINE_NOT_WORKING | ROT270 )

// Not the same as the other 240-in-1 machine from Thumbs Up below (tup240) This one makes greater use of newer VT features with most games having sampled music, not APU sound.
// Several of the games contained in here are buggy / broken on real hardware (see https://www.youtube.com/watch?v=-mgGNaDQ1HE )
CONS( 201?, 240in1ar,  0,  0,  vt36x_altswap_32mb_4banks_red5mam, vt369, vt36x_state, empty_init, "Thumbs Up", "Mini Arcade Machine (Thumbs Up, 240IN1ARC)", MACHINE_NOT_WORKING ) // 128Mbyte ROM, must be externally banked or different addressing scheme
// portable fan + famiclone combo handheld, very similar to 240in1ar
CONS( 2020, nubsupmf,   0,      0,  vt36x_altswap_4mb, vt369, vt36x_state, empty_init, "<unknown>", "NubSup Mini Game Fan", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// protected?
CONS( 202?, 36pcase,    0,      0,  vt36x_altswap_2mb, vt369, vt36x_state, empty_init, "<unknown>", "36-in-1 Classic Games phone case", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )


/*****************************************************************************
* below are VT369? games that use flash ROM
*****************************************************************************/

// different SoC (and language select music) from 2019 version, opcodes are scrambled
CONS( 2020, gbox2020, gbox2019, 0, vt36x_gbox2020_16mb, vt369, vt36x_state, empty_init, "Sup", "Game Box 400 in 1 (2020 PCB)", MACHINE_NOT_WORKING )

// unknown tech, probably from 2021, probably VT369, ROM wouldn't read consistently
// boots with bad colors
CONS( 202?, vibes240, 0,        0,  vt36x_vibesswap_16mb, vt369, vt36x_state, empty_init, "<unknown>", "Vibes Retro Pocket Gamer 240-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// has extra protection?
CONS( 2018, rbbrite,    0,        0,  vt369_unk_1mb, vt369, vt36x_state, empty_init, "Coleco", "Rainbow Brite (mini-arcade)", MACHINE_NOT_WORKING )

/*****************************************************************************
* below are VT369 games that use SQI / SPI ROM
*****************************************************************************/

// Runs well, minor GFX issues in intro
CONS( 2017, sy889,      0,        0,  vt36x_8mb, vt369, vt36x_state, empty_init, "SY Corp",   "SY-889 300 in 1 Handheld", MACHINE_IMPERFECT_GRAPHICS )
CONS( 2016, sy888b,     0,        0,  vt36x_4mb, vt369, vt36x_state, empty_init, "SY Corp",   "SY-888B 288 in 1 Handheld", MACHINE_IMPERFECT_GRAPHICS )

// Same hardware as SY-889
CONS( 201?, mc_cb280,   0,        0,  vt36x_swap_4mb, vt369, vt36x_state, empty_init, "CoolBoy",   "Coolboy RS-18 (280 in 1)", MACHINE_IMPERFECT_GRAPHICS )

// Plays intro music but then crashes. same hardware as SY-88x but uses more features
CONS( 2016, mog_m320,   0,        0,  vt36x_8mb, vt369, vt36x_state, empty_init, "MOGIS",    "MOGIS M320 246 in 1 Handheld", MACHINE_NOT_WORKING )

// VT369, but doesn't use most features
CONS( 200?, lpgm240,    0,        0,  vt36x_swap_8mb,        vt369, vt36x_state, empty_init, "<unknown>", "Let's Play! Game Machine 240 in 1", MACHINE_NOT_WORKING ) // mini 'retro-arcade' style cabinet
CONS( 200?, tup240,     lpgm240,  0,  vt36x_swap_8mb,        vt369, vt36x_state, empty_init, "Thumbs Up", "Thumbs Up 240-in-1 Mini Arcade Machine", MACHINE_NOT_WORKING )

// VT369, but doesn't use most features
CONS( 201?, unkra200,   mc_tv200, 0,  vt36x_8mb, vt369, vt36x_state, empty_init, "<unknown>",    "200 in 1 Retro Arcade", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
CONS( 201?, dgun2577,   mc_tv200, 0,  vt36x_8mb, vt369, vt36x_state, empty_init, "DreamGear",    "My Arcade Retro Machine 200-in-1 (DGUN-2577)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
CONS( 201?, lxcyber,    mc_tv200, 0,  vt36x_8mb, vt369, vt36x_state, empty_init, "Lexibook",     "Cyber Arcade 200-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
 // menu is protected with code from extra ROM
CONS( 201?, gtct885,    mc_tv200, 0,  vt36x_8mb, vt369, vt36x_state, empty_init, "Gaming Tech",  "Gaming Tech CT-885", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
 // similar to above, but with 40 extra games, menu is protected with code from extra ROM (although RTS opcodes seem to work)
CONS( 201?, rd5_240,    0,        0,  vt36x_8mb, vt369, vt36x_state, empty_init, "Red5",         "Mini Arcade Machine 240-in-1 (Red5)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

CONS( 201?, hkb502,   0,      0,  vt36x_4mb, vt369, vt36x_state, empty_init, "<unknown>", "HKB-502 268-in-1 (set 1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
CONS( 201?, hkb502a,  hkb502, 0,  vt36x_4mb, vt369, vt36x_state, empty_init, "<unknown>", "HKB-502 268-in-1 (set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
// similar to above, fewer games in menu
CONS( 2021, unk128vt, 0,      0,  vt36x_4mb, vt369, vt36x_state, empty_init, "<unknown>", "unknown VT369 based 128-in-1 (GC31-369-20210702-V2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// case was for an iPhone 11, but seems to be available for many different phones
CONS( 202?, 168pcase, 0,      0,  vt36x_4mb, vt369, vt36x_state, empty_init, "<unknown>", "Diier-D-10 168-in-1 phone case", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// uses a LCD with resolution of 160x128 (image scaled to fit for some games, others run natively at 160x128)
// contains a protection chip, command 80 XX returns a byte
CONS( 201?, lxcap,    0,      0,  vt36x_8mb, vt369, vt36x_tetrtin_state, empty_init, "Lexibook", "Cyber Arcade Pocket (JL1895)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// 2022 date on 'BL-867 PCB03' PCB, has extra protection?
CONS( 2022, nesvt270,    0,  0,  vt36x_16mb, vt369, vt36x_state, empty_init, "<unknown>", "unknown VT3xx based 270-in-1 (BL-867 PCB03)", MACHINE_NOT_WORKING )

// VT369, but doesn't use most features
CONS( 201?, myarccn,   0, 0,  vt36x_1mb, vt369, vt36x_state, empty_init, "DreamGear", "My Arcade Caveman Ninja", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// confirmed VT369, uses more features (including sound CPU)
CONS( 201?, denv150,   0,        0,  vt36x_8mb, vt369, vt36x_state, empty_init, "Denver", "Denver Game Console GMP-240C 150-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
CONS( 201?, egame150,  denv150,  0,  vt36x_swap_8mb, vt369, vt36x_state, empty_init, "<unknown>", "E-Game! 150-in-1", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// uncertain, uses SPI ROM so probably VT369, has extra protection? (but RAM test goes up to 0x2000, over the internal ROM area?)
CONS( 2017, otrail,     0,        0,  vt36x_1mb, vt369, vt36x_state, empty_init, "Basic Fun", "The Oregon Trail", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// seems to be running the NES version of Pac-Man with some extra splash screens, has extra protection
CONS( 2021, pactin,     0,        0,  vt36x_1mb, vt369, vt36x_tetrtin_state, empty_init, "Fizz Creations", "Pac-Man Arcade in a Tin", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
// has extra protection
CONS( 2021, tetrtin,    0,        0,  vt36x_1mb, vt369, vt36x_tetrtin_state, empty_init, "Fizz Creations", "Tetris Arcade in a Tin", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )

// uses a low res display (so vt3xx?)
CONS( 2021, matet10,   0,        0,  vt36x_swap_2mb, vt369, vt36x_state, empty_init, "dreamGEAR", "My Arcade Tetris (DGUNL-7083, Pixel Pocket, with 10 bonus games)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
CONS( 2021, matetsl,   0,        0,  vt36x_swap_512kb, vt369, vt36x_state, empty_init, "dreamGEAR", "My Arcade Tetris (Slurpee)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS ) // no bonus games on this model

// Runs well, all games seem to work
CONS( 201?, mc_89in1,  0,        0,  vt36x_4mb, vt369, vt36x_state, empty_init, "<unknown>", "89 in 1 Mini Game Console (060-92023011V1.0)", MACHINE_IMPERFECT_GRAPHICS )

// both offer chinese or english menus
CONS( 200?, mc_110cb,  0,        0,  vt36x_4mb, vt369, vt36x_state, empty_init, "CoolBoy", "110 in 1 CoolBaby (CoolBoy RS-1S)", MACHINE_IMPERFECT_GRAPHICS )
CONS( 200?, mc_138cb,  0,        0,  vt36x_4mb, vt369, vt36x_state, empty_init, "CoolBoy", "138 in 1 CoolBaby (CoolBoy RS-5, PCB060-10009011V1.3)", MACHINE_IMPERFECT_GRAPHICS )

CONS( 200?, jl2050,    0,        0,  vt36x_16mb, vt369, vt36x_state, empty_init, "LexiBook / JungleTac / NiceCode",  "Cyber Console Center 200-in-1 (JL2050)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

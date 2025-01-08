// license:BSD-3-Clause
// copyright-holders:David Haywood
/***************************************************************************

  nes_vt09.cpp

  VT09 and higher go in here

  - 4KB RAM
  - Optional alt 4bpp tile mode
  - DMA acts the same in both NTSC and PAL modes

  NON-bugs (same happens on real hardware)

  msisinv: Taito screen has bad palette, it's encoded as VT03 but hardware is VT09
  msimm2: Corruption on Metal Man boss

***************************************************************************/

#include "emu.h"
#include "nes_vt09_soc.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist_dev.h"


namespace {

class nes_vt09_common_base_state : public driver_device
{
public:
	nes_vt09_common_base_state(const machine_config& mconfig, device_type type, const char* tag) :
		driver_device(mconfig, type, tag),
		m_io0(*this, "IO0"),
		m_io1(*this, "IO1"),
		m_exin(*this, "EXTRAIN%u", 0U),
		m_prgrom(*this, "mainrom")
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	virtual uint8_t in0_r();
	virtual uint8_t in1_r();
	virtual void in0_w(uint8_t data);

	void nes_vt09_map(address_map &map) ATTR_COLD;

	optional_ioport m_io0;
	optional_ioport m_io1;

	uint8_t m_latch0;
	uint8_t m_latch1;
	uint8_t m_previous_port0;

	optional_ioport_array<4> m_exin;

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

class nes_vt09_common_state : public nes_vt09_common_base_state
{
public:
	nes_vt09_common_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt09_common_base_state(mconfig, type, tag),
		m_soc(*this, "soc")
	{ }

	void vt_external_space_map_32mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_16mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_8mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_4mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_2mbyte(address_map &map) ATTR_COLD;
	void vt_external_space_map_1mbyte(address_map &map) ATTR_COLD;
	[[maybe_unused]] void vt_external_space_map_512kbyte(address_map &map) ATTR_COLD;


protected:
	required_device<nes_vt02_vt03_soc_device> m_soc;
};

class nes_vt09_state : public nes_vt09_common_state
{
public:
	nes_vt09_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt09_common_state(mconfig, type, tag)
	{ }

	void nes_vt09(machine_config& config);
	void nes_vt09_1mb(machine_config& config);
	void nes_vt09_2mb(machine_config& config);
	void nes_vt09_4mb(machine_config& config);
	void nes_vt09_4mb_rasterhack(machine_config& config);
	void nes_vt09_8mb(machine_config& config);
	void nes_vt09_16mb(machine_config& config);

private:
};

class nes_vt09_cart_state : public nes_vt09_state
{
public:
	nes_vt09_cart_state(const machine_config& mconfig, device_type type, const char* tag) :
		nes_vt09_state(mconfig, type, tag),
		m_bank(*this, "cartbank"),
		m_cart(*this, "cartslot"),
		m_cart_region(nullptr)
	{ }

	void nes_vt09_cart(machine_config& config);

protected:
	void machine_start() override ATTR_COLD;

private:
	void vt_external_space_map_cart(address_map &map) ATTR_COLD;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	required_memory_bank m_bank;
	required_device<generic_slot_device> m_cart;
	memory_region *m_cart_region;
};

void nes_vt09_cart_state::machine_start()
{
	nes_vt09_state::machine_start();

	m_bank->configure_entries(0, 1, memregion("mainrom")->base(), 0x200000);
	m_bank->set_entry(0);

	// if there's a cart, override the standard banking
	if (m_cart && m_cart->exists())
	{
		m_cart_region = memregion(std::string(m_cart->tag()) + GENERIC_ROM_REGION_TAG);
		m_bank->configure_entries(0, 1, m_cart_region->base(), 0x200000);
		m_bank->set_entry(0);
	}
}

DEVICE_IMAGE_LOAD_MEMBER(nes_vt09_cart_state::cart_load)
{
	uint32_t size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}


uint8_t nes_vt09_common_base_state::vt_rom_r(offs_t offset)
{
	return m_prgrom[offset];
}

void nes_vt09_common_base_state::vtspace_w(offs_t offset, uint8_t data)
{
	logerror("%s: vtspace_w %08x : %02x", machine().describe_context(), offset, data);
}

// VTxx can address 25-bit address space (32MB of ROM) so use maps with mirroring in depending on ROM size
void nes_vt09_common_state::vt_external_space_map_32mbyte(address_map &map)
{
	map(0x0000000, 0x1ffffff).r(FUNC(nes_vt09_common_state::vt_rom_r));
}

void nes_vt09_common_state::vt_external_space_map_16mbyte(address_map &map)
{
	map(0x0000000, 0x0ffffff).mirror(0x1000000).r(FUNC(nes_vt09_common_state::vt_rom_r));
}

void nes_vt09_common_state::vt_external_space_map_8mbyte(address_map &map)
{
	map(0x0000000, 0x07fffff).mirror(0x1800000).r(FUNC(nes_vt09_common_state::vt_rom_r));
}

void nes_vt09_common_state::vt_external_space_map_4mbyte(address_map &map)
{
	map(0x0000000, 0x03fffff).mirror(0x1c00000).r(FUNC(nes_vt09_common_state::vt_rom_r));
}

void nes_vt09_common_state::vt_external_space_map_2mbyte(address_map &map)
{
	map(0x0000000, 0x01fffff).mirror(0x1e00000).r(FUNC(nes_vt09_common_state::vt_rom_r));
}

void nes_vt09_common_state::vt_external_space_map_1mbyte(address_map &map)
{
	map(0x0000000, 0x00fffff).mirror(0x1f00000).r(FUNC(nes_vt09_common_state::vt_rom_r));
}

void nes_vt09_common_state::vt_external_space_map_512kbyte(address_map &map)
{
	map(0x0000000, 0x007ffff).mirror(0x1f80000).r(FUNC(nes_vt09_common_state::vt_rom_r));
}

void nes_vt09_cart_state::vt_external_space_map_cart(address_map &map)
{
	map(0x0000000, 0x01fffff).mirror(0x1e00000).bankr("cartbank");
}


template <uint8_t NUM> uint8_t nes_vt09_common_base_state::extrain_r()
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

uint8_t nes_vt09_common_base_state::in0_r()
{
	//logerror("%s: in0_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch0 & 1;
	m_latch0 >>= 1;
	return ret;
}

uint8_t nes_vt09_common_base_state::in1_r()
{
	//logerror("%s: in1_r\n", machine().describe_context());
	uint8_t ret = 0x40;
	ret |= m_latch1 & 1;
	m_latch1 >>= 1;
	return ret;
}

void nes_vt09_common_base_state::in0_w(uint8_t data)
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


void nes_vt09_common_base_state::machine_start()
{
	m_latch0 = 0;
	m_latch1 = 0;
	m_previous_port0 = 0;

	save_item(NAME(m_latch0));
	save_item(NAME(m_latch1));
	save_item(NAME(m_previous_port0));
}

void nes_vt09_common_base_state::machine_reset()
{
}

void nes_vt09_common_base_state::configure_soc(nes_vt02_vt03_soc_device* soc)
{
	soc->set_addrmap(AS_PROGRAM, &nes_vt09_common_state::vt_external_space_map_32mbyte);
	soc->read_0_callback().set(FUNC(nes_vt09_common_base_state::in0_r));
	soc->read_1_callback().set(FUNC(nes_vt09_common_base_state::in1_r));
	soc->write_0_callback().set(FUNC(nes_vt09_common_base_state::in0_w));

	soc->extra_read_0_callback().set(FUNC(nes_vt09_common_base_state::extrain_r<0>));
	soc->extra_read_1_callback().set(FUNC(nes_vt09_common_base_state::extrain_r<1>));
	soc->extra_read_2_callback().set(FUNC(nes_vt09_common_base_state::extrain_r<2>));
	soc->extra_read_3_callback().set(FUNC(nes_vt09_common_base_state::extrain_r<3>));
}


uint8_t nes_vt09_common_base_state::upper_412c_r()
{
	logerror("%s: upper_412c_r\n", machine().describe_context());
	return 0x00;
}

uint8_t nes_vt09_common_base_state::upper_412d_r()
{
	logerror("%s: upper_412d_r\n", machine().describe_context());
	return 0x00;
}

void nes_vt09_common_base_state::upper_412c_w(uint8_t data)
{
	logerror("%s: upper_412c_w %02x\n", machine().describe_context(), data);
}



void nes_vt09_state::nes_vt09(machine_config &config)
{
	/* basic machine hardware */
	NES_VT09_SOC(config, m_soc, NTSC_APU_CLOCK);
	configure_soc(m_soc);

	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412c_callback().set(FUNC(nes_vt09_state::upper_412c_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_read_412d_callback().set(FUNC(nes_vt09_state::upper_412d_r));
	dynamic_cast<nes_vt09_soc_device&>(*m_soc).upper_write_412c_callback().set(FUNC(nes_vt09_state::upper_412c_w));

	m_soc->force_bad_dma();
}

void nes_vt09_state::nes_vt09_16mb(machine_config& config)
{
	nes_vt09(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt09_state::vt_external_space_map_16mbyte);
}

void nes_vt09_state::nes_vt09_8mb(machine_config& config)
{
	nes_vt09(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt09_state::vt_external_space_map_8mbyte);
}

void nes_vt09_state::nes_vt09_1mb(machine_config& config)
{
	nes_vt09(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt09_state::vt_external_space_map_1mbyte);
}

void nes_vt09_state::nes_vt09_2mb(machine_config& config)
{
	nes_vt09(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt09_state::vt_external_space_map_2mbyte);
}

void nes_vt09_state::nes_vt09_4mb(machine_config& config)
{
	nes_vt09(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt09_state::vt_external_space_map_4mbyte);
}

void nes_vt09_state::nes_vt09_4mb_rasterhack(machine_config& config)
{
	nes_vt09_4mb(config);
	m_soc->force_raster_timing_hack();
}

void nes_vt09_cart_state::nes_vt09_cart(machine_config& config)
{
	nes_vt09(config);
	m_soc->set_addrmap(AS_PROGRAM, &nes_vt09_cart_state::vt_external_space_map_cart);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "nes_vt_cart");
	m_cart->set_width(GENERIC_ROM8_WIDTH);
	m_cart->set_device_load(FUNC(nes_vt09_cart_state::cart_load));

	SOFTWARE_LIST(config, "cart_list").set_original("nes_vt_cart");
}

static INPUT_PORTS_START( nes_vt09 )
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

static INPUT_PORTS_START( nes_vt09_msi )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("B")
	//PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) // doesn't exist?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY

	PORT_START("IO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( nes_vt09_msi_mm2 )
	PORT_START("IO0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("B")
	//PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) // doesn't exist?
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY

	PORT_START("IO1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


ROM_START( msiwwe )
	ROM_REGION( 0x200000, "mainrom", 0 ) // the first half of this ROM is unused code from the Ms. Pac-Man game!
	ROM_LOAD( "wrestlemania_es29lv160fb_004a2249.bin", 0x00000, 0x200000, CRC(f524382d) SHA1(0c8d1c29c76e3e3c58018354f1eca9445c9ab945) )
ROM_END

ROM_START( msiwwea )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "wrestlemania_en29lv800bb_007f225b.bin", 0x00000, 0x100000, CRC(52102de3) SHA1(f858ad18e05d3de24dfab4c98798efb4d30e2987) )
ROM_END

ROM_START( msidd )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "doubledragon_m29w160eb_00202249.bin", 0x00000, 0x200000, CRC(44df5bb6) SHA1(a984aa1644d2d313d4263afdfed1cd64009f1137) )
ROM_END

ROM_START( msimm2 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "megaman2_s99jl032hbt1_001227e_readas_s29jl032h.bin", 0x00000, 0x400000, CRC(f537a053) SHA1(bd9353df34c0c0ee7d0e5e9808fc36f1a5eecc22) )
ROM_END

ROM_START( msinamco )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "msinamco3in1.bin", 0x00000, 0x200000, CRC(c69ad54a) SHA1(f12b9274d827e8a8a8f1bf2646fa426d9f8e6ece) )
ROM_END

ROM_START( msimpac )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "mspacman_29dl800ba_000422cb.bin", 0x00000, 0x100000, CRC(c66300e3) SHA1(3fc0bdfbf449d884151f1b581e848243cd2df3a5) )
ROM_END

ROM_START( msisinv )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "spaceinvaders_en29lv800bb_007f225b.bin", 0x00000, 0x100000, CRC(e444d129) SHA1(33742bc3a6250337cc42b73812e797023818282a) )
ROM_END

ROM_START( msifrog )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "frogger_39vf3201_00bf235b.bin", 0x00000, 0x400000, CRC(c46c29c0) SHA1(b8f26445c2086b97db8ee98bf36dff9d63ca414b) )
ROM_END

ROM_START( cybar120 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "m2500p-vt09-epson,20091222ver05,_30r-sx1067-01_pcb,_12r0cob128m_12001-3d05_fw.bin", 0x00000, 0x1000000, CRC(f7138980) SHA1(de31264ee3a5a5c77a86733b2e2d6845fee91ea5) )
ROM_END

ROM_START( jl2050 )
	ROM_REGION( 0x1000000, "mainrom", 0 )
	ROM_LOAD( "jl2050.u5", 0x00000, 0x1000000, CRC(f96c5c02) SHA1(c7d0b57c2622b5213d3c7e6532495d9da74d4b01) )
ROM_END

ROM_START( vsmaxtx2 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "tx2.bin", 0x00000, 0x400000, CRC(eddf0ca8) SHA1(b87c5c3e945d1efdcb953325425d4ddb0fded00a) )
ROM_END

ROM_START( senario25 )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "senario25.bin", 0x00000, 0x200000, CRC(270c4517) SHA1(c099096d1c86f55f2b0826484cd3d3f68c90c794) )
ROM_END

ROM_START( dturbogt )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "dgturbogt.bin", 0x00000, 0x800000, CRC(9532fb78) SHA1(cd188672f9b8e9c12069612ad0d0b70d3dd6c1b1) )
ROM_END

ROM_START( rcapnp )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "rcapnp_mx29lv160ab_00c22249.bin", 0x00000, 0x200000, CRC(8cc30a47) SHA1(815bfc26360b01ed3fa077016222939d2184408c) )
ROM_END

ROM_START( ventur25 )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "25games_m5m29gt320vp_001c0020.bin", 0x00000, 0x400000, CRC(3f78a45a) SHA1(3e97333c13e09c580e66518dd2e1e031371d399c) )
ROM_END

ROM_START( vgtablet )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "vgtablet.bin", 0x00000, 0x400000, CRC(99ef3978) SHA1(0074445708d66a04ab02b4993069ce1ae0514c2f) )
ROM_END

ROM_START( vgpocket )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "vgpocket.bin", 0x00000, 0x400000, CRC(843634c6) SHA1(c59dab0e43d364f59eb3a138abb585bc54e5d674) )
	// there was a dump of a 'secure' area with this, but it was just the top 0x10000 bytes of the existing rom.
ROM_END

ROM_START( vgpmini )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "vgpmini.bin", 0x00000, 0x400000, CRC(a1121843) SHA1(c96013ae6cf2f8173e65a167d45685cb61536d36) )
	// there was a dump of a 'secure' area with this, but it was just the bottom 0x10000 bytes of the existing rom.
ROM_END

ROM_START( joypad65 )
	ROM_REGION( 0x800000, "mainrom", 0 )
	ROM_LOAD( "joypad65.bin", 0x00000, 0x800000, CRC(b7f81c5f) SHA1(8579d9bc866415e0049979b7c3427d8dd0a60813) )
ROM_END

ROM_START( rbbrite )
	ROM_REGION( 0x100000, "mainrom", 0 )
	ROM_LOAD( "coleco_rainbowbrite_29dl800ba_000422cb.bin", 0x00000, 0x100000, CRC(d2ad0d7d) SHA1(4423a5aa2eda20b3621ab46e951ac08dc2d24789) )
ROM_END

ROM_START( timetp25 )
	ROM_REGION( 0x200000, "mainrom", 0 )
	ROM_LOAD( "s29al016d70tfi02.u2", 0x00000, 0x200000, CRC(6109816a) SHA1(e48699d48b72219d80b8d27b1337e8d09793f4da) )
	ROM_FILL(0x1fce36, 0x01, 0x04 | 0x40) // the code doesn't set the 'alt 4bpp' mode bit, but needs it? why? it isn't hardcoded as the system takes cartridges which don't want it
ROM_END

ROM_START( wfmotor )
	ROM_REGION( 0x400000, "mainrom", 0 )
	ROM_LOAD( "motorcycle.bin", 0x00000, 0x400000, CRC(978f12f0) SHA1(a0230cfe4398d3971d487ff5d4b7107341799424) )
ROM_END


} // anonymous namespace


// MSI Entertainment games (MSI previously operated as Majesco Entertainment)

// There are meant to be multiple revisions of this software, some with theme tunes for the new wrestlers, some without. This one appears to lack them.
// 2 box variations exist, one with Randy Savage in purple attire and another with green, this was dumped from a unit with purple on the box.
CONS( 2017, msiwwe,     0,      0,  nes_vt09_2mb, nes_vt09_msi, nes_vt09_state, empty_init, "MSI", "WWE Wrestlemania Steel Cage Challenge (Plug & Play) (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
// this one was dumped from the version with Randy Savage in green, the box was much larger than the other one.  This one also has new theme music for the adjusted roster.
CONS( 2017, msiwwea,    msiwwe, 0,  nes_vt09_1mb, nes_vt09_msi, nes_vt09_state, empty_init, "MSI", "WWE Wrestlemania Steel Cage Challenge (Plug & Play) (set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 2017, msidd,      0,  0,  nes_vt09_2mb, nes_vt09_msi, nes_vt09_state, empty_init, "MSI / Arc System Works", "Double Dragon - 30 Years Anniversary (Plug & Play)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 2016, msimpac,    0,  0,  nes_vt09_1mb, nes_vt09_msi, nes_vt09_state, empty_init, "MSI / Bandai Namco", "Ms. Pac-Man (MSI Plug & Play)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

CONS( 2017, msimm2,     0,  0,  nes_vt09_4mb, nes_vt09_msi_mm2, nes_vt09_state, empty_init, "MSI / Capcom", "Mega Man 2 (MSI Plug & Play)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // various issues (glitched Metal Man stage boss, missing 'ready' text) happen on real unit

CONS( 2016, msisinv,    0,  0,  nes_vt09_1mb, nes_vt09_msi, nes_vt09_state, empty_init, "MSI / Taito", "Space Invaders (MSI Plug & Play)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// This is from the version with the same case type as the above MSI units.
// MSI also issued a version in the original Majesco shell but with the updated case logos and boot logos in the software, the software on that revision might match this one.
CONS( 2016, msifrog,    0,  0,  nes_vt09_4mb_rasterhack, nes_vt09_msi, nes_vt09_state, empty_init, "MSI / Konami", "Frogger (MSI Plug & Play, white joystick)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) //  raster timing for need a hack

CONS( 2018, msinamco,   0,  0,  nes_vt09_1mb, nes_vt09_msi, nes_vt09_state, empty_init, "MSI / Bandai Namco", "Namco Classics Vol.1 (3-in-1) (MSI Plug & Play)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// MSI Midway (Joust+Gauntlet 2 + Defender 2) has 2x Globs, rather than Glob + Flash ROM

// this is VT09 based
CONS( 2009, cybar120,  0,  0,  nes_vt09_16mb,nes_vt09, nes_vt09_state, empty_init, "Defender / JungleTac",                      "Defender M2500P 120-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, vsmaxtx2,  0,  0,  nes_vt09_4mb, nes_vt09, nes_vt09_state, empty_init, "Senario / JungleTac",                       "Vs Maxx TX-2 50-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, senario25, 0,  0,  nes_vt09_2mb, nes_vt09, nes_vt09_state, empty_init, "Senario / JungleTac",                       "25 Video Games - All in 1 Video System (Senario)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // no Vs Maxx branding, newer style packaging
CONS( 200?, rcapnp,    0,  0,  nes_vt09_2mb, nes_vt09, nes_vt09_state, empty_init, "RCA / JungleTac",                           "RCA NS-500 30-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, dturbogt,  0,  0,  nes_vt09_8mb, nes_vt09, nes_vt09_state, empty_init, "dreamGEAR / JungleTac",                     "Turbo GT 50-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, ventur25,  0,  0,  nes_vt09_4mb, nes_vt09, nes_vt09_state, empty_init, "<unknown> / JungleTac",                     "Venturer '25 Games' 25-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, joypad65,  0,  0,  nes_vt09_8mb, nes_vt09, nes_vt09_state, empty_init, "WinFun / JungleTac",                        "Joypad 65", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, wfmotor,   0,  0,  nes_vt09_4mb, nes_vt09, nes_vt09_state, empty_init, "WinFun / JungleTac",                        "Motorcycle 30-in-1", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 2005, vgpocket,  0,  0,  nes_vt09_4mb, nes_vt09, nes_vt09_state, empty_init, "Performance Designed Products / JungleTac", "VG Pocket (VG-2000)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
CONS( 200?, vgpmini,   0,  0,  nes_vt09_4mb, nes_vt09, nes_vt09_state, empty_init, "Performance Designed Products / JungleTac", "VG Pocket Mini (VG-1500)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )
// VG Pocket Max (VG-2500) (blue case, 75 games)
// VG Pocket Max (VG-3000) (white case, 75 games) (does the game selection differ, or only the case?)
CONS( 2006, vgtablet,  0, 0,  nes_vt09_4mb_rasterhack,  nes_vt09, nes_vt09_state, empty_init, "Performance Designed Products (licensed by Konami) / JungleTac", "VG Pocket Tablet (VG-4000)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // raster timing for Frogger needs a hack
// VG Pocket Caplet is SunPlus hardware instead, see spg2xx_lexibook.cpp

CONS( 200?, jl2050,  0,  0,  nes_vt09_16mb,nes_vt09, nes_vt09_state, empty_init, "LexiBook / JungleTac / NiceCode",  "Cyber Console Center 200-in-1 (JL2050)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

// might be VT369 based, if so, move
CONS( 2018, rbbrite,    0,  0,  nes_vt09_1mb, nes_vt09, nes_vt09_state, empty_init, "Coleco", "Rainbow Brite (mini-arcade)", MACHINE_NOT_WORKING )

CONS( 200?, timetp25, 0,  0,  nes_vt09_cart, nes_vt09, nes_vt09_cart_state, empty_init, "Timetop", "Super Game 25-in-1 (GM-228)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND )

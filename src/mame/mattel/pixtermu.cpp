// license:BSD-3-Clause
// copyright-holders:QUFB
/******************************************************************************

    Skeleton driver for Pixter Multi-Media.

    Currently hangs after reset:

    - If executing boot ROM, with boot configuration set to load NOR Flash or SRAM, it will reach an infinite loop at 0x1f0;
    - If executing nCS1 ROM, it will wait indefinitely due to unimplemented timer controls;

    References:

    - https://elinux.org/Pixter_Multimedia
    - https://www.nxp.com/docs/en/data-sheet/LH79524_525_N.pdf
    - https://www.nxp.com/docs/en/user-guide/LH79524-LH79525_UG_V1_3.pdf
    - https://lh79525.wordpress.com/

    Hardware
    --------

    Model H4651/J4287/J4288:

    - PCB Revision: PT1543A-BGA-4F2C 2005/04/30 Rev 4.2b
    - CPU: Sharp LH79524-NOE (ARM720T)
    - RAM: ICSI IC42S16100-7T (2MB each, 4MB total)
    - ROM: Chip-On-Board, selected by pin CS1

    JTAG
    ----

    To place the LH79524 into ARM ICE debug mode you will need to connect TEST1 via a 4.7K Ohm resistor to ground.

    Pinout:

    - D2: nTRST (Reset Input)
    - P4: TMS (Mode Select Input)
    - T3: TCK (Clock Input)
    - T1: TDI (Serial Data Input)
    - P3: TDO (Data Serial Output)
    - TEST1: ARM ICE debug mode

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"

#include "softlist_dev.h"

namespace {

class pixter_multimedia_state : public driver_device
{
public:
	pixter_multimedia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_maincpu(*this, "maincpu")
		, m_bootrom(*this, "bootrom")
		, m_ndcs0(*this, "ndcs0")
		, m_ncs0(*this, "ncs0")
		, m_ncs1(*this, "ncs1")
		, m_internal_sram(*this, "internal_sram")
	{ }

	void pixter_multimedia(machine_config &config);

	required_device<generic_slot_device> m_cart;
	required_device<arm7_cpu_device> m_maincpu;
	required_shared_ptr<uint32_t> m_bootrom;
	required_shared_ptr<uint32_t> m_ndcs0;
	required_shared_ptr<uint32_t> m_ncs0;
	required_shared_ptr<uint32_t> m_ncs1;
	required_shared_ptr<uint32_t> m_internal_sram;

private:
	// Remap Control, mapped at 0xfffe2008, offset 0x22008/4
	static inline constexpr uint32_t APB_REMAP = 34818;
	// Power-up Boot Configuration, mapped at 0xfffe6000, offset 0x26000/4
	static inline constexpr uint32_t APB_PBC = 38912;
	// nCS1 Override, mapped at 0xffff6004, offset 0x26004/4
	static inline constexpr uint32_t APB_CS1OV = 38913;
	// External Peripheral Mapping, mapped at 0xffff6008, offset 0x26008/4
	static inline constexpr uint32_t APB_EPM = 38914;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void arm7_map(address_map &map);
	uint32_t apb_bridge_r(offs_t offset);
	void apb_bridge_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t remap_r(offs_t offset);

	std::unique_ptr<uint32_t[]> m_apb;
};

void pixter_multimedia_state::machine_start()
{
	memcpy(reinterpret_cast<uint32_t *>(m_bootrom.target()), memregion("bootrom")->base(), 0x2000);
	memcpy(reinterpret_cast<uint32_t *>(m_ncs1.target()), memregion("ncs1")->base(), 0x400000);
	if (m_cart->exists()) {
		std::string region_tag;
		memory_region *cart_rom = m_cart->memregion("cart:rom");
		m_maincpu->space(AS_PROGRAM).install_rom(0x48000000, 0x483fffff, 0x400000, cart_rom->base());
	}

	m_apb = make_unique_clear<uint32_t[]>(0x27000/4);
	save_pointer(NAME(m_apb), 0x27000/4);
}

void pixter_multimedia_state::machine_reset()
{
	memset(m_apb.get(), 0, 0x27000);
	m_apb[APB_PBC] = 0b0000; // Boot from NOR Flash or SRAM, 16-bit data bus width, nBLEx LOW for reads
	m_apb[APB_CS1OV] = 0b0; // nCS1 is routed for normal operation
	m_apb[APB_EPM] = 0b1111; // All external devices are accessible following reset
}

DEVICE_IMAGE_LOAD_MEMBER(pixter_multimedia_state::cart_load)
{
	uint32_t const size = m_cart->common_get_size("rom");

	m_cart->rom_alloc(size, GENERIC_ROM32_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

void pixter_multimedia_state::arm7_map(address_map &map)
{
	// Remap Bank
	map(0x00000000, 0x003fffff).r(FUNC(pixter_multimedia_state::remap_r));

	// External SRAM
	map(0x20000000, 0x203fffff).ram().share("ndcs0");
	// nCS0 (Unused NAND Flash?)
	map(0x40000000, 0x403fffff).rom().share("ncs0");
	// nCS1 (Chip-On-Board ROM)
	map(0x44000000, 0x443fffff).rom().share("ncs1");
	// nCS2 (Cart ROM)
	map(0x48000000, 0x483fffff).rom().share("ncs2");
	// nCS3 (Unused?)
	map(0x4c000000, 0x4c3fffff).rom().share("ncs3");

	// Internal SRAM
	map(0x60000000, 0x60003fff).mirror(0x0fffc000).ram().share("internal_sram");

	// Boot ROM
	map(0x80000000, 0x80001fff).rom().share("bootrom");

	// APB Bridge
	map(0xfffc0000, 0xfffe6fff).rw(FUNC(pixter_multimedia_state::apb_bridge_r), FUNC(pixter_multimedia_state::apb_bridge_w));
	// External Memory Control
	map(0xffff1000, 0xffff1fff).ram();
	// Color LCD Control
	map(0xffff4000, 0xffff4fff).ram();
	// USB Device
	map(0xffff5000, 0xffff5fff).ram();
	// Interrupt Vector Control
	map(0xfffff000, 0xffffffff).ram();
}

uint32_t pixter_multimedia_state::remap_r(offs_t offset)
{
	// User's Guide - 1.6 Memory Interface Architecture
	uint32_t *target = nullptr;
	switch (m_apb[APB_REMAP]) {
		case 0:
			target = (m_apb[APB_PBC] & 0b0100) && (m_apb[APB_CS1OV] & 0b1) ? m_bootrom.target() : m_ncs1.target();
			break;
		case 1:
			target = m_ndcs0.target();
			break;
		case 2:
			target = m_internal_sram.target();
			break;
		case 3:
			target = m_ncs0.target();
			break;
		default:
			return 0;
	}

	return target[offset];
}

uint32_t pixter_multimedia_state::apb_bridge_r(offs_t offset)
{
	return m_apb[offset];
}

void pixter_multimedia_state::apb_bridge_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_apb[offset]);
}

static INPUT_PORTS_START( pixter_multimedia )
INPUT_PORTS_END

void pixter_multimedia_state::pixter_multimedia(machine_config &config)
{
	// User's Guide - 1.3 Clock Strategy - AHB Fast CPU Clock (FCLK)
	ARM7(config, m_maincpu, 76'205'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &pixter_multimedia_state::arm7_map);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "pixter_cart");
	m_cart->set_endian(ENDIANNESS_LITTLE);
	m_cart->set_width(GENERIC_ROM32_WIDTH);
	m_cart->set_device_load(FUNC(pixter_multimedia_state::cart_load));
	m_cart->set_must_be_loaded(false);

	SOFTWARE_LIST(config, "cart_list").set_original("pixter_cart");
}

ROM_START( pixtermu )
	ROM_REGION( 0x400000, "ncs1", 0 )
	ROM_LOAD( "cs1.bin", 0x00000000, 0x400000, CRC(9d06745a) SHA1(c85ffd1777ffee4e99e5a208e3707a39b0dfc3aa) )

	ROM_REGION( 0x2000, "bootrom", 0 )
	ROM_LOAD( "lh79524.bootrom.bin", 0x00000000, 0x2000, CRC(5314a9e3) SHA1(23ed1914c7e7cc875cbb9f9b3d511a60a7324abd) )
ROM_END

} // anonymous namespace


//    year, name,     parent,  compat, machine,           input,             class,                   init,       company,  fullname,             flags
CONS( 2005, pixtermu, 0,       0,      pixter_multimedia, pixter_multimedia, pixter_multimedia_state, empty_init, "Mattel", "Pixter Multi-Media", MACHINE_IS_SKELETON )

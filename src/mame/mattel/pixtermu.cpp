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

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/arm7/arm7.h"

#include "softlist_dev.h"

namespace {

class pixter_multimedia_state : public driver_device
{
public:
	pixter_multimedia_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_maincpu(*this, "maincpu")
		, m_ndcs0(*this, "ndcs0")
		, m_internal_sram(*this, "internal_sram")
		, m_apb(*this, "apb", 0x27000, ENDIANNESS_LITTLE)
		, m_remap_view(*this, "remap")
	{ }

	void pixter_multimedia(machine_config &config);

private:
	// Remap Control, mapped at 0xfffe2008, offset 0x22008/4
	static inline constexpr uint32_t APB_REMAP = 34818;
	// Power-up Boot Configuration, mapped at 0xfffe6000, offset 0x26000/4
	static inline constexpr uint32_t APB_PBC = 38912;
	// nCS1 Override, mapped at 0xfffe6004, offset 0x26004/4
	static inline constexpr uint32_t APB_CS1OV = 38913;
	// External Peripheral Mapping, mapped at 0xfffe6008, offset 0x26008/4
	static inline constexpr uint32_t APB_EPM = 38914;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void arm7_map(address_map &map) ATTR_COLD;
	void apb_bridge_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void apb_remap(uint32_t data);

	required_device<generic_slot_device> m_cart;
	required_device<arm7_cpu_device> m_maincpu;
	required_shared_ptr<uint32_t> m_ndcs0;
	required_shared_ptr<uint32_t> m_internal_sram;
	memory_share_creator<uint32_t> m_apb;
	memory_view m_remap_view;
};

void pixter_multimedia_state::apb_remap(uint32_t data)
{
	// User's Guide - 1.6 Memory Interface Architecture
	if (data == 0) {
		m_remap_view.select((m_apb[APB_PBC] & 0b0100) && (m_apb[APB_CS1OV] & 0b1) ? 0 : 3);
	} else if (data < 3) {
		m_remap_view.select(data);
	} else {
		logerror("Unexpected remap %d\n", data);
	}
}

void pixter_multimedia_state::machine_start()
{
	if (m_cart->exists()) {
		memory_region *const cart_rom = m_cart->memregion("rom");
		device_generic_cart_interface::install_non_power_of_two<0>(
				cart_rom->bytes(),
				0x03ff'ffff,
				0,
				0x4800'0000,
				[this, cart_rom] (offs_t begin, offs_t end, offs_t mirror, offs_t src) {
					m_maincpu->space(AS_PROGRAM).install_rom(begin, end, mirror, cart_rom->base() + src);
				});
	}
}

void pixter_multimedia_state::machine_reset()
{
	m_apb[APB_PBC] = 0b0000; // Boot from NOR Flash or SRAM, 16-bit data bus width, nBLEx LOW for reads
	m_apb[APB_CS1OV] = 0b0; // nCS1 is routed for normal operation
	m_apb[APB_EPM] = 0b1111; // All external devices are accessible following reset

	m_apb[APB_REMAP] = 0b00; // Map nCS1
	apb_remap(m_apb[APB_REMAP]);
}

DEVICE_IMAGE_LOAD_MEMBER(pixter_multimedia_state::cart_load)
{
	uint64_t length;
	memory_region *cart_rom = nullptr;
	if (m_cart->loaded_through_softlist()) {
		cart_rom = m_cart->memregion("rom");
		if (!cart_rom) {
			return std::make_pair(image_error::BADSOFTWARE, "Software list item has no 'rom' data area");
		}
		length = cart_rom->bytes();
	} else {
		length = m_cart->length();
	}

	if (!length) {
		return std::make_pair(image_error::INVALIDLENGTH, "Cartridges must not be empty");
	}
	if (length & 3) {
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be a multiple of 4 bytes)");
	}

	if (!m_cart->loaded_through_softlist()) {
		cart_rom = machine().memory().region_alloc(m_cart->subtag("rom"), length, 4, ENDIANNESS_LITTLE);
		if (!cart_rom) {
			return std::make_pair(std::errc::not_enough_memory, std::string());
		}

		uint32_t *const base = reinterpret_cast<uint32_t *>(cart_rom->base());
		if (m_cart->fread(base, length) != length) {
			return std::make_pair(std::errc::io_error, "Error reading cartridge file");
		}

		if (ENDIANNESS_NATIVE != ENDIANNESS_LITTLE) {
			for (uint64_t i = 0; (length / 4) > i; ++i)
				base[i] = swapendian_int32(base[i]);
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

void pixter_multimedia_state::arm7_map(address_map &map)
{
	// Remap Bank
	map(0x0000'0000, 0x003f'ffff).view(m_remap_view);
	m_remap_view[0](0x0000'0000, 0x0000'1fff).rom().region("bootrom", 0);
	m_remap_view[1](0x0000'0000, 0x003f'ffff).ram().share("ndcs0");
	m_remap_view[2](0x0000'0000, 0x0000'3fff).ram().share("internal_sram");
	m_remap_view[3](0x0000'0000, 0x003f'ffff).rom().region("ncs1", 0);

	// External SRAM
	map(0x2000'0000, 0x203f'ffff).ram().share("ndcs0");
	// nCS0 (Unused NAND Flash?)
	// map(0x40000000, 0x403fffff)
	// nCS1 (Chip-On-Board ROM)
	map(0x4400'0000, 0x443f'ffff).mirror(0x03c0'0000).rom().region("ncs1", 0);
	// nCS2 (Cart ROM)
	// map(0x4800'0000, 0x483f'ffff)
	// nCS3 (Unused?)
	// map(0x4c00'0000, 0x4c3f'ffff)

	// Internal SRAM
	map(0x6000'0000, 0x6000'3fff).mirror(0x0fff'c000).ram().share("internal_sram");

	// Boot ROM
	map(0x8000'0000, 0x8000'1fff).rom().region("bootrom", 0);

	// APB Bridge
	map(0xfffc'0000, 0xfffe'6fff).ram().share("apb").w(FUNC(pixter_multimedia_state::apb_bridge_w));
	// External Memory Control
	map(0xffff'1000, 0xffff'1fff).ram();
	// Color LCD Control
	map(0xffff'4000, 0xffff'4fff).ram();
	// USB Device
	map(0xffff'5000, 0xffff'5fff).ram();
	// Interrupt Vector Control
	map(0xffff'f000, 0xffff'ffff).ram();
}

void pixter_multimedia_state::apb_bridge_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == m_apb[APB_REMAP]) {
		apb_remap(data);
	}

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
	ROM_REGION32_LE( 0x400000, "ncs1", 0 )
	ROM_LOAD( "cs1.bin", 0x00000000, 0x400000, CRC(9d06745a) SHA1(c85ffd1777ffee4e99e5a208e3707a39b0dfc3aa) )

	ROM_REGION32_LE( 0x2000, "bootrom", 0 )
	ROM_LOAD( "lh79524.bootrom.bin", 0x00000000, 0x2000, CRC(5314a9e3) SHA1(23ed1914c7e7cc875cbb9f9b3d511a60a7324abd) )
ROM_END

} // anonymous namespace


//    year, name,     parent,  compat, machine,           input,             class,                   init,       company,  fullname,             flags
CONS( 2005, pixtermu, 0,       0,      pixter_multimedia, pixter_multimedia, pixter_multimedia_state, empty_init, "Mattel", "Pixter Multi-Media", MACHINE_IS_SKELETON )

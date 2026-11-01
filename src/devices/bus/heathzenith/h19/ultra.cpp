// license:BSD-3-Clause
// copyright-holders:Mark Garlanger
/***************************************************************************

  Heathkit Terminal Logic Board (TLB) with the UltraROM

  Not Currently Implemented
    - Interlace mode

****************************************************************************/

#include "emu.h"
#include "ultra.h"

#include "igc.h"


namespace bus::heathzenith::h19 {

namespace {

/**
 * UltraROM and page 2 RAM, shared by the TLB and TLB plus IGC versions
 */
template <typename Base>
class heath_ultra_tlb_base : public Base
{
protected:
	heath_ultra_tlb_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;

	required_memory_region m_maincpu_region;
	required_shared_ptr<u8> m_page_2_ram;
	memory_view m_mem_view;
};

/**
 * Heath TLB with Ultra ROM
 */
class heath_ultra_tlb_device : public heath_ultra_tlb_base<heath_tlb_device>
{
public:
	heath_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

/**
 * Heath TLB with the UltraROM plus SigmaSoft Interactive Graphics Controller
 */
class heath_igc_ultra_tlb_device : public heath_ultra_tlb_base<heath_igc_tlb_device>
{
public:
	heath_igc_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


static INPUT_PORTS_START( ultra19 )
	PORT_INCLUDE( tlb )

	PORT_MODIFY("SW401")
	PORT_DIPNAME( 0x07, 0x05, "Baud Rate")              PORT_DIPLOCATION("SW401:1,2,3")
	PORT_DIPSETTING(    0x00, "110")
	PORT_DIPSETTING(    0x01, "300")
	PORT_DIPSETTING(    0x02, "1200")
	PORT_DIPSETTING(    0x03, "2400")
	PORT_DIPSETTING(    0x04, "4800")
	PORT_DIPSETTING(    0x05, "9600")
	PORT_DIPSETTING(    0x06, "19200")
	PORT_DIPSETTING(    0x07, "38400")
	PORT_DIPNAME( 0x08, 0x00, "Parity")                 PORT_DIPLOCATION("SW401:4")
	PORT_DIPSETTING(    0x00, "Disabled")
	PORT_DIPSETTING(    0x08, "Enabled")
	PORT_DIPNAME( 0x10, 0x00, "Parity Type")            PORT_DIPLOCATION("SW401:5")
	PORT_DIPSETTING(    0x00, "Odd")
	PORT_DIPSETTING(    0x10, "Even")
	PORT_DIPNAME( 0x20, 0x00, "Data Size")              PORT_DIPLOCATION("SW401:6")
	PORT_DIPSETTING(    0x00, "8-bit")
	PORT_DIPSETTING(    0x20, "7-bit")
	PORT_DIPNAME( 0x40, 0x40, "Duplex")                 PORT_DIPLOCATION("SW401:7")
	PORT_DIPSETTING(    0x00, "Half")
	PORT_DIPSETTING(    0x40, "Full")
	PORT_DIPNAME( 0x80, 0x80, "Software Handshaking")   PORT_DIPLOCATION("SW401:8")
	PORT_DIPSETTING(    0x00, "Enabled")
	PORT_DIPSETTING(    0x80, "Disabled")

	PORT_MODIFY("SW402")
	PORT_DIPNAME( 0x08, 0x00, "Keypad Shifted")         PORT_DIPLOCATION("SW402:4")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x00, "Default Key Values")     PORT_DIPLOCATION("SW402:5")
	PORT_DIPSETTING(    0x00, "HDOS Values")
	PORT_DIPSETTING(    0x10, "CP/M Values")
	PORT_DIPNAME( 0x60, 0x60, "Cursor Blink")           PORT_DIPLOCATION("SW402:6,7")
	PORT_DIPSETTING(    0x00, "Steady")
	PORT_DIPSETTING(    0x20, "Invisible")
	PORT_DIPSETTING(    0x40, "Fast Blink")
	PORT_DIPSETTING(    0x60, "Slow Blink")
	PORT_DIPNAME( 0x80, 0x00, "Interlace Scan Mode")    PORT_DIPLOCATION("SW402:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("CONFIG")
	PORT_CONFNAME(0x10, 0x10, "Page 2 RAM present")
	PORT_CONFSETTING(   0x00, DEF_STR( No ) )
	PORT_CONFSETTING(   0x10, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( igc_ultra )
	PORT_INCLUDE( ultra19 )

	PORT_INCLUDE( igc_common )
INPUT_PORTS_END


ROM_START( ultra19 )
	// Ultra ROM
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2532_h19_ultra_firmware.u437",     0x0000, 0x1000, CRC(8ad4cdb4) SHA1(d6e1fc37a1f52abfce5e9adb1819e0030bed1df3))

	// Original font
	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_LOAD( "2716_444-29_h19font.u420",         0x0000, 0x0800, CRC(d595ac1d) SHA1(130fb4ea8754106340c318592eec2d8a0deaf3d0))

	// Ultra keyboard
	ROM_REGION( 0x0800, "keyboard", 0 )
	ROM_LOAD( "2716_h19_ultra_keyboard.u445",     0x0000, 0x0800, CRC(76130c92) SHA1(ca39c602af48505139d2750a084b5f8f0e662ff7))
ROM_END


/**
 * UltraROM
 *
 * Developed by William G. Parrott, III, sold by Software Wizardry, Inc.
 */
template <typename Base>
heath_ultra_tlb_base<Base>::heath_ultra_tlb_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: Base(mconfig, type, tag, owner, clock)
	, m_maincpu_region(*this, "maincpu")
	, m_page_2_ram(*this, "page2ram")
	, m_mem_view(*this, "mem")
{
}

template <typename Base>
void heath_ultra_tlb_base<Base>::device_reset()
{
	Base::device_reset();

	ioport_value const cfg(this->m_config->read());

	// Page 2 memory
	m_mem_view.select(BIT(cfg, 4));
}

template <typename Base>
void heath_ultra_tlb_base<Base>::device_add_mconfig(machine_config &config)
{
	Base::device_add_mconfig(config);

	this->m_maincpu->set_addrmap(AS_PROGRAM, &heath_ultra_tlb_base::mem_map);
}

template <typename Base>
void heath_ultra_tlb_base<Base>::mem_map(address_map &map)
{
	Base::mem_map(map);

	map(0x0000, 0x3fff).view(m_mem_view);

	m_mem_view[0](0x0000, 0x0fff).mirror(0x3000).rom().region(m_maincpu_region, 0x0000).unmapw();
	m_mem_view[1](0x0000, 0x0fff).mirror(0x2000).rom().region(m_maincpu_region, 0x0000).unmapw();
	m_mem_view[1](0x1000, 0x1fff).mirror(0x2000).ram().share(m_page_2_ram);
}

template <typename Base>
const tiny_rom_entry *heath_ultra_tlb_base<Base>::device_rom_region() const
{
	return ROM_NAME(ultra19);
}


heath_ultra_tlb_device::heath_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_ultra_tlb_base<heath_tlb_device>(mconfig, HEATH_ULTRA, tag, owner, clock)
{
}

ioport_constructor heath_ultra_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ultra19);
}


/**
 * SigmaSoft and Systems IGC plus TLB with UltraROM
 */
heath_igc_ultra_tlb_device::heath_igc_ultra_tlb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: heath_ultra_tlb_base<heath_igc_tlb_device>(mconfig, HEATH_IGC_ULTRA, tag, owner, clock)
{
}

ioport_constructor heath_igc_ultra_tlb_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(igc_ultra);
}

} // anonymous namespace

} // namespace bus::heathzenith::h19

DEFINE_DEVICE_TYPE_PRIVATE(HEATH_ULTRA,     device_heath_tlb_card_interface, bus::heathzenith::h19::heath_ultra_tlb_device,     "heath_ultra_tlb",     "Heath Terminal Logic Board w/ Ultra ROM")
DEFINE_DEVICE_TYPE_PRIVATE(HEATH_IGC_ULTRA, device_heath_tlb_card_interface, bus::heathzenith::h19::heath_igc_ultra_tlb_device, "heath_igc_ultra_tlb", "Heath Terminal Logic Board w/ Ultra ROM plus SigmaSoft Interactive Graphics Controller")

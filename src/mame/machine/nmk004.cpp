// license:BSD-3-Clause
// copyright-holders:David Haywood,Alex Marshall
/***************************************************************************

 NMK004 emulation

***************************************************************************/

#include "emu.h"
#include "nmk004.h"

#include "sound/2203intf.h"
#include "sound/okim6295.h"


WRITE8_MEMBER( nmk004_device::write )
{
	machine().scheduler().synchronize();
	to_nmk004 = data;
}

READ8_MEMBER( nmk004_device::read )
{
	machine().scheduler().synchronize();
	return to_main;
}

WRITE8_MEMBER(nmk004_device::nmk004_port4_w)
{
	// bit 0x08 toggles frequently but is connected to nothing?

	// bit 0x01 is set to reset the 68k
	m_reset_cb(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER(nmk004_device::nmk004_oki0_bankswitch_w)
{
	data &= 3;
	membank(":okibank1")->set_entry(data);
}

WRITE8_MEMBER(nmk004_device::nmk004_oki1_bankswitch_w)
{
	data &= 3;
	membank(":okibank2")->set_entry(data);
}

READ8_MEMBER(nmk004_device::nmk004_tonmk004_r)
{
	machine().scheduler().synchronize();
	return to_nmk004;
}

WRITE8_MEMBER(nmk004_device::nmk004_tomain_w)
{
	machine().scheduler().synchronize();
	to_main = data;
}


void nmk004_device::ym2203_irq_handler(int irq)
{
	m_cpu->set_input_line(0, irq ? ASSERT_LINE : CLEAR_LINE);
}

void nmk004_device::nmk004_sound_mem_map(address_map &map)
{
	//map(0x0000, 0x1fff).rom(); /* 0x0000 - 0x1fff = internal ROM */
	map(0x2000, 0xefff).rom().region(":audiocpu", 0x2000);
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf801).rw(":ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xf900, 0xf900).rw(":oki1", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfa00, 0xfa00).rw(":oki2", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xfb00, 0xfb00).r(FUNC(nmk004_device::nmk004_tonmk004_r));    // from main cpu
	map(0xfc00, 0xfc00).w(FUNC(nmk004_device::nmk004_tomain_w));  // to main cpu
	map(0xfc01, 0xfc01).w(FUNC(nmk004_device::nmk004_oki0_bankswitch_w));
	map(0xfc02, 0xfc02).w(FUNC(nmk004_device::nmk004_oki1_bankswitch_w));
}


ROM_START( nmk004 )
	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "nmk004.bin", 0x00000, 0x02000, CRC(83b6f611) SHA1(bb7ddc00affe8a066002ecd6858dbd2854af8940) )
ROM_END


DEFINE_DEVICE_TYPE(NMK004, nmk004_device, "nmk004", "NMK004")

nmk004_device::nmk004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NMK004, tag, owner, clock),
	m_cpu(*this, "mcu"),
	m_reset_cb(*this),
	to_nmk004(0xff),
	to_main(0xff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void nmk004_device::device_start()
{
	m_reset_cb.resolve_safe();

	membank(":okibank1")->configure_entries(0, 4, memregion(":oki1")->base() + 0x20000, 0x20000);
	membank(":okibank2")->configure_entries(0, 4, memregion(":oki2")->base() + 0x20000, 0x20000);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void nmk004_device::device_add_mconfig(machine_config &config)
{
	TMP90840(config, m_cpu, DERIVED_CLOCK(1,1)); // Toshiba TMP90C840AF in QFP64 package with 8Kbyte internal ROM
	m_cpu->set_addrmap(AS_PROGRAM, &nmk004_device::nmk004_sound_mem_map);
	m_cpu->port_write<4>().set(FUNC(nmk004_device::nmk004_port4_w));
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------
const tiny_rom_entry *nmk004_device::device_rom_region() const
{
	return ROM_NAME(nmk004 );
}

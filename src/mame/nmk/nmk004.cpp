// license:BSD-3-Clause
// copyright-holders:David Haywood,Alex Marshall
/***************************************************************************

 NMK004 emulation

***************************************************************************/

#include "emu.h"
#include "nmk004.h"

#include "sound/okim6295.h"
#include "sound/ymopn.h"


void nmk004_device::write(uint8_t data)
{
	machine().scheduler().synchronize();
	to_nmk004 = data;
}

uint8_t nmk004_device::read()
{
	machine().scheduler().synchronize();
	return to_main;
}

void nmk004_device::port4_w(uint8_t data)
{
	// bit 0x08 toggles frequently but is connected to nothing?

	// bit 0x01 is set to reset the 68k
	m_reset_cb(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t nmk004_device::ym_r(offs_t offset)
{
	return m_ym_read_cb(offset);
}

void nmk004_device::ym_w(offs_t offset, uint8_t data)
{
	m_ym_write_cb(offset, data);
}

template <unsigned Which>
uint8_t nmk004_device::oki_r()
{
	return m_oki_read_cb[Which]();
}

template <unsigned Which>
void nmk004_device::oki_w(uint8_t data)
{
	m_oki_write_cb[Which](data);
}

template <unsigned Which>
void nmk004_device::oki_bankswitch_w(uint8_t data)
{
	data &= 3;
	m_okibank[Which]->set_entry(data);
}

uint8_t nmk004_device::tonmk004_r()
{
	machine().scheduler().synchronize();
	return to_nmk004;
}

void nmk004_device::tomain_w(uint8_t data)
{
	machine().scheduler().synchronize();
	to_main = data;
}


void nmk004_device::ym2203_irq_handler(int irq)
{
	m_cpu->set_input_line(0, irq ? ASSERT_LINE : CLEAR_LINE);
}

void nmk004_device::mem_map(address_map &map)
{
	//map(0x0000, 0x1fff).rom(); /* 0x0000 - 0x1fff = internal ROM */
	map(0x2000, 0xefff).rom().region(DEVICE_SELF, 0x2000);
	map(0xf000, 0xf7ff).ram();
	map(0xf800, 0xf801).rw(FUNC(nmk004_device::ym_r), FUNC(nmk004_device::ym_w));
	map(0xf900, 0xf900).rw(FUNC(nmk004_device::oki_r<0>), FUNC(nmk004_device::oki_w<0>));
	map(0xfa00, 0xfa00).rw(FUNC(nmk004_device::oki_r<1>), FUNC(nmk004_device::oki_w<1>));
	map(0xfb00, 0xfb00).r(FUNC(nmk004_device::tonmk004_r));    // from main cpu
	map(0xfc00, 0xfc00).w(FUNC(nmk004_device::tomain_w));  // to main cpu
	map(0xfc01, 0xfc01).w(FUNC(nmk004_device::oki_bankswitch_w<0>));
	map(0xfc02, 0xfc02).w(FUNC(nmk004_device::oki_bankswitch_w<1>));
}


ROM_START( nmk004 )
	ROM_REGION( 0x2000, "mcu", 0 )
	ROM_LOAD( "nmk004.bin", 0x00000, 0x02000, CRC(8ae61a09) SHA1(f55f9e6bb55bfa56f9f797518dca032aaa3f6a32) )
ROM_END


DEFINE_DEVICE_TYPE(NMK004, nmk004_device, "nmk004", "NMK004")

nmk004_device::nmk004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NMK004, tag, owner, clock)
	, m_cpu(*this, "mcu")
	, m_okirom(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
	, m_okibank(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
	, m_reset_cb(*this)
	, m_ym_read_cb(*this, 0)
	, m_ym_write_cb(*this)
	, m_oki_read_cb{{*this, 0}, {*this, 0}}
	, m_oki_write_cb{{*this}, {*this}}
	, to_nmk004(0xff)
	, to_main(0xff)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void nmk004_device::device_start()
{
	save_item(NAME(to_main));
	save_item(NAME(to_nmk004));

	for (int i = 0; i < 2; i++)
	{
		m_okibank[i]->configure_entries(0, 4, m_okirom[i] + 0x20000, 0x20000);
	}
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void nmk004_device::device_add_mconfig(machine_config &config)
{
	TMP90840(config, m_cpu, DERIVED_CLOCK(1,1)); // Toshiba TMP90C840AF in QFP64 package with 8Kbyte internal ROM
	m_cpu->set_addrmap(AS_PROGRAM, &nmk004_device::mem_map);
	m_cpu->port_write<4>().set(FUNC(nmk004_device::port4_w));
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------
const tiny_rom_entry *nmk004_device::device_rom_region() const
{
	return ROM_NAME(nmk004);
}

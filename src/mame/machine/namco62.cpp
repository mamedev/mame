// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Namco 62XX

    This custom chip is a Fujitsu MB8843 MCU programmed to act as an I/O
    device. It is used by just one game: Gaplus.

    TODO: Chip pin description/layout/notes

***************************************************************************/

#include "emu.h"
#include "machine/namco62.h"

#define VERBOSE 0
#include "logmacro.h"


/***************************************************************************
    DEVICE INTERFACE
***************************************************************************/

ROM_START( namco_62xx )
	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "62xx.bin", 0x0000, 0x0800, CRC(308dc115) SHA1(fe0a60fc339ac2eeed4879a64c1aab130f9d4cfe) )
ROM_END


DEFINE_DEVICE_TYPE(NAMCO_62XX, namco_62xx_device, "namco62", "Namco 62xx")

namco_62xx_device::namco_62xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_62XX, tag, owner, clock),
	m_cpu(*this, "mcu"),
	m_in{ { *this }, { *this }, { *this }, { *this } },
	m_out{ { *this }, { *this } }
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_62xx_device::device_start()
{
	/* resolve our read callbacks */
	for (devcb_read8 &cb : m_in)
		cb.resolve_safe(0);

	/* resolve our write callbacks */
	for (devcb_write8 &cb : m_out)
		cb.resolve_safe();
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void namco_62xx_device::device_add_mconfig(machine_config &config)
{
	MB8843(config, m_cpu, DERIVED_CLOCK(1,1)); /* parent clock, internally divided by 6 (TODO: Correct?) */
//  m_cpu->read_k().set(FUNC(namco_62xx_device::namco_62xx_K_r));
//  m_cpu->write_o().set(FUNC(namco_62xx_device::namco_62xx_O_w));
//  m_cpu->read_r<0>().set(FUNC(namco_62xx_device::namco_62xx_R0_r));
//  m_cpu->read_r<2>().set(FUNC(namco_62xx_device::namco_62xx_R2_r));
	m_cpu->set_disable();
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const tiny_rom_entry *namco_62xx_device::device_rom_region() const
{
	return ROM_NAME(namco_62xx );
}

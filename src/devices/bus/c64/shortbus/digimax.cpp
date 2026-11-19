// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    DigiMAX ShortBus card

***************************************************************************/

#include "emu.h"
#include "digimax.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE(IDE64_SHORTBUS_DIGIMAX, ide64_shortbus_digimax_device, "ide64_shortbus_digimax", "DigiMAX (ShortBus)")

static INPUT_PORTS_START( digimax )
	PORT_START("BASE")
	PORT_CONFNAME( 0x01, 0x00, "Base Address" )
	PORT_CONFSETTING(    0x00, "$DE40" )
	PORT_CONFSETTING(    0x01, "$DE48" )
INPUT_PORTS_END

ioport_constructor ide64_shortbus_digimax_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( digimax );
}

ide64_shortbus_digimax_device::ide64_shortbus_digimax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IDE64_SHORTBUS_DIGIMAX, tag, owner, clock),
	device_ide64_shortbus_interface(mconfig, *this),
	m_dac(*this, "%u", 0U),
	m_base(*this, "BASE")
{
}

void ide64_shortbus_digimax_device::device_start()
{
}

void ide64_shortbus_digimax_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	DAC_8BIT_R2R(config, m_dac[1], 0).add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	DAC_8BIT_R2R(config, m_dac[3], 0).add_route(ALL_OUTPUTS, "lspeaker", 0.25);

	SPEAKER(config, "rspeaker").front_right();
	DAC_8BIT_R2R(config, m_dac[0], 0).add_route(ALL_OUTPUTS, "rspeaker", 0.25);
	DAC_8BIT_R2R(config, m_dac[2], 0).add_route(ALL_OUTPUTS, "rspeaker", 0.25);
}

void ide64_shortbus_digimax_device::shortbus_w(offs_t offset, uint8_t data, int io1, int io2, int csel)
{
	if (!io1)
	{
		const bool upper = BIT(m_base->read(), 0);

		if (upper ? (offset >= 0x48 && offset < 0x58) : ((offset & 0xf8) == 0x40))
			m_dac[offset & 3]->write(data);
	}
}

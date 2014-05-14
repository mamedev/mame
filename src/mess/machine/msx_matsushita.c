#include "emu.h"
#include "msx_matsushita.h"


const device_type MSX_MATSUSHITA = &device_creator<msx_matsushita_device>;


msx_matsushita_device::msx_matsushita_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: msx_switched_device(mconfig, MSX_MATSUSHITA, "Matsushita switched device", tag, owner, clock, "msx_matsushita", __FILE__)
	, m_io_config(*this, "CONFIG")
	, m_turbo_out_cb(*this)
{
}


UINT8 msx_matsushita_device::get_id()
{
	return 0x08;
}


static INPUT_PORTS_START( matsushita )
	PORT_START("CONFIG")
	PORT_CONFNAME( 0x80, 0x00, "Firmware switch")
	PORT_CONFSETTING( 0x00, "On" )
	PORT_CONFSETTING( 0x80, "Off" )
	PORT_BIT(0x7F, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


ioport_constructor msx_matsushita_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( matsushita );
}


void msx_matsushita_device::device_start()
{
	msx_switched_device::device_start();
	m_turbo_out_cb.resolve_safe();
}

READ8_MEMBER(msx_matsushita_device::io_read)
{
	switch (offset)
	{
		case 0:
			return ~get_id();

		case 1:
			return m_io_config->read();

		default:
			printf("msx_matsushita: unhandled read from offset %02x\n", offset);
			break;
	}

	return 0xFF;
}


WRITE8_MEMBER(msx_matsushita_device::io_write)
{
	switch (offset)
	{
		// bit 0: CPU clock select
		//        0 - 5.369317 MHz
		//        1 - 3.579545 MHz
		case 0x01:
			m_turbo_out_cb((data & 1) ? ASSERT_LINE : CLEAR_LINE);
			break;

		default:
			printf("msx_matsushita: unhandled write %02x to offset %02x\n", data, offset);
			break;
	}
}


/***************************************************************************

    E05A03 Gate Array (used in the Epson LX-800)

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

***************************************************************************/

#include "emu.h"
#include "e05a03.h"


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

const device_type E05A03 = &device_creator<e05a03_device>;

e05a03_device::e05a03_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, E05A03, "E05A03", tag, owner, clock, "e05a03", __FILE__),
	m_shift(0),
	m_busy_leading(0),
	m_busy_software(0),
	m_nlqlp(0),
	m_cndlp(0),
	#if 0
	m_pe(0),
	m_pelp(0),
	#endif
	m_printhead(0),
	m_pf_motor(0),
	m_cr_motor(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void e05a03_device::device_config_complete()
{
	// inherit a copy of the static data
	const e05a03_interface *intf = reinterpret_cast<const e05a03_interface *>(static_config());
	if (intf != NULL)
		*static_cast<e05a03_interface *>(this) = *intf;

	// or initialize to defaults if none provided
	else
	{
		memset(&m_in_data_cb, 0 , sizeof(m_in_data_cb));
		memset(&m_out_nlq_lp_cb, 0 , sizeof(m_out_nlq_lp_cb));
		memset(&m_out_pe_lp_cb, 0 , sizeof(m_out_pe_lp_cb));
		memset(&m_out_pe_cb, 0 , sizeof(m_out_pe_cb));
		memset(&m_out_reso_cb, 0 , sizeof(m_out_reso_cb));
	}
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e05a03_device::device_start()
{
	/* resolve callbacks */
	m_out_nlq_lp_func.resolve(m_out_nlq_lp_cb, *this);
	m_out_pe_lp_func.resolve(m_out_pe_lp_cb, *this);
	m_out_reso_func.resolve(m_out_reso_cb, *this);
	m_out_pe_func.resolve(m_out_pe_cb, *this);
	m_in_data_func.resolve(m_in_data_cb, *this);

	/* register for state saving */
	save_item(NAME(m_shift));
	save_item(NAME(m_busy_leading));
	save_item(NAME(m_busy_software));
	save_item(NAME(m_nlqlp));
	save_item(NAME(m_cndlp));
	#if 0
	save_item(NAME(m_pe));
	save_item(NAME(m_pelp));
	#endif
	save_item(NAME(m_printhead));
	save_item(NAME(m_pf_motor));
	save_item(NAME(m_cr_motor));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e05a03_device::device_reset()
{
	m_printhead = 0x00;
	m_pf_motor = 0x00;
	m_cr_motor = 0x0f;

	m_out_pe_func(0);
	m_out_pe_lp_func(1);

	m_busy_software = 1;
	m_nlqlp = 1;
	m_cndlp = 1;
}



/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

WRITE8_MEMBER( e05a03_device::write )
{
	logerror("%s: e05a03_w(%02x): %02x\n", space.machine().describe_context(), offset, data);

	switch (offset)
	{
	/* shift register */
	case 0x00: m_shift = (m_shift & 0x00ffff) | (data << 16); break;
	case 0x01: m_shift = (m_shift & 0xff00ff) | (data << 8); break;
	case 0x02: m_shift = (m_shift & 0xffff00) | (data << 0); break;

	case 0x03:
		m_busy_leading = BIT(data, 7);
		m_busy_software = BIT(data, 6);
		m_nlqlp = BIT(data, 4);
		m_cndlp = BIT(data, 3);

		m_out_pe_func(BIT(data, 2));
		m_out_pe_lp_func(!BIT(data, 2));

#if 0
		m_pe = BIT(data, 2);
		m_pelp = !BIT(data, 2);
#endif

		break;

	/* printhead */
	case 0x04: m_printhead = (m_printhead & 0x100) | !data; break;
	case 0x05: m_printhead = (m_printhead & 0x0ff) | (!(BIT(data, 7) << 8)); break;

	/* paper feed and carriage motor phase data*/
	case 0x06: m_pf_motor = (data & 0xf0) >> 4; break;
	case 0x07: m_cr_motor = (data & 0x0f) >> 0; break;
	}
}

READ8_MEMBER( e05a03_device::read )
{
	UINT8 result = 0;

	logerror("%s: e05a03_r(%02x)\n", space.machine().describe_context(), offset);

	switch (offset)
	{
	case 0x00:
		break;

	case 0x01:
		break;

	case 0x02:
		result = m_in_data_func(0);
		break;

	case 0x03:
		result |= BIT(m_shift, 23) << 7;
		m_shift <<= 1;
		break;
	}

	return result;
}

/* home position signal */
WRITE_LINE_MEMBER( e05a03_device::home_w )
{
}

/* printhead solenoids trigger */
WRITE_LINE_MEMBER( e05a03_device::fire_w )
{
}

WRITE_LINE_MEMBER( e05a03_device::strobe_w )
{
}

READ_LINE_MEMBER( e05a03_device::busy_r )
{
	return 1;
}

WRITE_LINE_MEMBER( e05a03_device::resi_w )
{
	if (!state)
	{
		device_reset();
		m_out_reso_func(1);
	}
}

WRITE_LINE_MEMBER( e05a03_device::init_w )
{
	resi_w(state);
}

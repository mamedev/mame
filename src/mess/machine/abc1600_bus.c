/**********************************************************************

    Luxor ABC 1600 Expansion Bus emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "abc1600_bus.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ABC1600BUS_SLOT = &device_creator<abc1600bus_slot_device>;



//**************************************************************************
//  DEVICE ABC1600BUS CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_abc1600bus_card_interface - constructor
//-------------------------------------------------

device_abc1600bus_card_interface::device_abc1600bus_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~device_abc1600bus_card_interface - destructor
//-------------------------------------------------

device_abc1600bus_card_interface::~device_abc1600bus_card_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  abc1600bus_slot_device - constructor
//-------------------------------------------------

abc1600bus_slot_device::abc1600bus_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ABC1600BUS_SLOT, "ABC 1600 bus slot", tag, owner, clock),
		device_slot_interface(mconfig, *this),
		m_int(CLEAR_LINE),
		m_pren(1),
		m_trrq(1),
		m_nmi(CLEAR_LINE),
		m_xint2(CLEAR_LINE),
		m_xint3(CLEAR_LINE),
		m_xint4(CLEAR_LINE),
		m_xint5(CLEAR_LINE)
{
}


//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void abc1600bus_slot_device::device_config_complete()
{
	// inherit a copy of the static data
	const abc1600bus_interface *intf = reinterpret_cast<const abc1600bus_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<abc1600bus_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_out_int_cb, 0, sizeof(m_out_int_cb));
    	memset(&m_out_pren_cb, 0, sizeof(m_out_pren_cb));
    	memset(&m_out_trrq_cb, 0, sizeof(m_out_trrq_cb));
    	memset(&m_out_nmi_cb, 0, sizeof(m_out_nmi_cb));
    	memset(&m_out_xint2_cb, 0, sizeof(m_out_xint2_cb));
    	memset(&m_out_xint3_cb, 0, sizeof(m_out_xint3_cb));
    	memset(&m_out_xint4_cb, 0, sizeof(m_out_xint4_cb));
    	memset(&m_out_xint5_cb, 0, sizeof(m_out_xint5_cb));
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void abc1600bus_slot_device::device_start()
{
	m_card = dynamic_cast<device_abc1600bus_card_interface *>(get_card_device());

	// resolve callbacks
	m_out_int_func.resolve(m_out_int_cb, *this);
	m_out_pren_func.resolve(m_out_pren_cb, *this);
	m_out_trrq_func.resolve(m_out_trrq_cb, *this);
	m_out_nmi_func.resolve(m_out_nmi_cb, *this);
	m_out_xint2_func.resolve(m_out_xint2_cb, *this);
	m_out_xint3_func.resolve(m_out_xint3_cb, *this);
	m_out_xint4_func.resolve(m_out_xint4_cb, *this);
	m_out_xint5_func.resolve(m_out_xint5_cb, *this);

	// state saving
	save_item(NAME(m_int));
	save_item(NAME(m_pren));
	save_item(NAME(m_trrq));
	save_item(NAME(m_nmi));
	save_item(NAME(m_xint2));
	save_item(NAME(m_xint3));
	save_item(NAME(m_xint4));
	save_item(NAME(m_xint5));
}


//-------------------------------------------------
//  cs_w -
//-------------------------------------------------

void abc1600bus_slot_device::cs_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abc1600bus_cs(data);
	}
}


//-------------------------------------------------
//  csb_r -
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::csb_r )
{
	int data = 1;

	if (m_card != NULL)
	{
		data = m_card->abc1600bus_csb();
	}

	return data;
}


//-------------------------------------------------
//  brst_w - reset
//-------------------------------------------------

void abc1600bus_slot_device::brst_w()
{
	if (m_card != NULL)
	{
		m_card->abc1600bus_brst();
	}
}


//-------------------------------------------------
//  inp_r -
//-------------------------------------------------

UINT8 abc1600bus_slot_device::inp_r()
{
	UINT8 data = 0xff;

	if (m_card != NULL)
	{
		data &= m_card->abc1600bus_inp();
	}

	return data;
}


//-------------------------------------------------
//  out_w -
//-------------------------------------------------

void abc1600bus_slot_device::out_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abc1600bus_out(data);
	}
}


//-------------------------------------------------
//  stat_r -
//-------------------------------------------------

UINT8 abc1600bus_slot_device::stat_r()
{
	UINT8 data = 0xff;

	if (m_card != NULL)
	{
		data &= m_card->abc1600bus_stat();
	}

	return data;
}


//-------------------------------------------------
//  stat_r -
//-------------------------------------------------

UINT8 abc1600bus_slot_device::ops_r()
{
	UINT8 data = 0xff;

	if (m_card != NULL)
	{
		data &= m_card->abc1600bus_ops();
	}

	return data;
}


//-------------------------------------------------
//  c1_w -
//-------------------------------------------------

void abc1600bus_slot_device::c1_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abc1600bus_c1(data);
	}
}


//-------------------------------------------------
//  c2_w -
//-------------------------------------------------

void abc1600bus_slot_device::c2_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abc1600bus_c2(data);
	}
}


//-------------------------------------------------
//  c3_w -
//-------------------------------------------------

void abc1600bus_slot_device::c3_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abc1600bus_c3(data);
	}
}


//-------------------------------------------------
//  c4_w -
//-------------------------------------------------

void abc1600bus_slot_device::c4_w(UINT8 data)
{
	if (m_card != NULL)
	{
		m_card->abc1600bus_c4(data);
	}
}


//-------------------------------------------------
//  exp_r -
//-------------------------------------------------

UINT8 abc1600bus_slot_device::exp_r()
{
	UINT8 data = 0xff;

	if (m_card != NULL)
	{
		data &= m_card->abc1600bus_exp();
	}

	return data;
}


//-------------------------------------------------
//  xcsb2_r -
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::xcsb2_r )
{
	int data = 1;

	if (m_card != NULL)
	{
		data = m_card->abc1600bus_xcsb2();
	}

	return data;
}


//-------------------------------------------------
//  xcsb3_r -
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::xcsb3_r )
{
	int data = 1;

	if (m_card != NULL)
	{
		data = m_card->abc1600bus_xcsb3();
	}

	return data;
}


//-------------------------------------------------
//  xcsb4_r -
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::xcsb4_r )
{
	int data = 1;

	if (m_card != NULL)
	{
		data = m_card->abc1600bus_xcsb4();
	}

	return data;
}


//-------------------------------------------------
//  xcsb5_r -
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::xcsb5_r )
{
	int data = 1;

	if (m_card != NULL)
	{
		data = m_card->abc1600bus_xcsb5();
	}

	return data;
}


//-------------------------------------------------
//  tren_w - transfer enable
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::tren_w )
{
	if (m_card != NULL)
	{
		m_card->abc1600bus_tren(state);
	}
}


//-------------------------------------------------
//  prac_w - peripheral acknowledge
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::prac_w )
{
	if (m_card != NULL)
	{
		m_card->abc1600bus_prac(state);
	}
}


//-------------------------------------------------
//  int_w - interrupt request
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::int_w )
{
	m_int = state;
	m_out_int_func(state);
}


//-------------------------------------------------
//  pren_w - peripheral enable
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::pren_w )
{
	m_pren = state;
	m_out_pren_func(state);
}


//-------------------------------------------------
//  trrq_w - transfer request
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::trrq_w )
{
	m_trrq = state;
	m_out_trrq_func(state);
}


//-------------------------------------------------
//  nmi_w - non-maskable interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::nmi_w )
{
	m_nmi = state;
	m_out_nmi_func(state);
}


//-------------------------------------------------
//  xint2_w - expansion interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::xint2_w )
{
	m_xint2 = state;
	m_out_xint2_func(state);
}


//-------------------------------------------------
//  xint3_w - expansion interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::xint3_w )
{
	m_xint3 = state;
	m_out_xint3_func(state);
}


//-------------------------------------------------
//  xint4_w - expansion interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::xint4_w )
{
	m_xint4 = state;
	m_out_xint4_func(state);
}


//-------------------------------------------------
//  xint5_w - expansion interrupt
//-------------------------------------------------

WRITE_LINE_MEMBER( abc1600bus_slot_device::xint5_w )
{
	m_xint5 = state;
	m_out_xint5_func(state);
}


//-------------------------------------------------
//  int_r - interrupt request
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::int_r )
{
	return m_int;
}


//-------------------------------------------------
//  pren_r - peripheral enable
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::pren_r )
{
	return m_pren;
}


//-------------------------------------------------
//  trrq_r - transfer request
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::trrq_r )
{
	return m_trrq;
}


//-------------------------------------------------
//  nmi_r - non-maskable interrupt
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::nmi_r )
{
	return m_nmi;
}


//-------------------------------------------------
//  xint2_r - expansion interrupt
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::xint2_r )
{
	return m_xint2;
}


//-------------------------------------------------
//  xint3_r - expansion interrupt
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::xint3_r )
{
	return m_xint3;
}


//-------------------------------------------------
//  xint4_r - expansion interrupt
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::xint4_r )
{
	return m_xint4;
}


//-------------------------------------------------
//  xint5_r - expansion interrupt
//-------------------------------------------------

READ_LINE_MEMBER( abc1600bus_slot_device::xint5_r )
{
	return m_xint5;
}

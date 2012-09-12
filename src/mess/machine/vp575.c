/**********************************************************************

    RCA VIP Expansion Board VP-575 emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#include "vp575.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VP575 = &device_creator<vp575_device>;


//-------------------------------------------------
//  VIP_EXPANSION_INTERFACE( expansion_intf )
//-------------------------------------------------

void vp575_device::update_interrupts()
{
	int interrupt = CLEAR_LINE;
	int dma_out = CLEAR_LINE;
	int dma_in = CLEAR_LINE;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		interrupt |= m_int[i];
		dma_out |= m_dma_out[i];
		dma_in |= m_dma_in[i];
	}

	m_slot->interrupt_w(interrupt);
	m_slot->dma_out_w(dma_out);
	m_slot->dma_in_w(dma_in);
}

WRITE_LINE_MEMBER( vp575_device::exp1_int_w ) { m_int[0] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp2_int_w ) { m_int[1] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp3_int_w ) { m_int[2] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp4_int_w ) { m_int[3] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp5_int_w ) { m_int[4] = state; update_interrupts(); }

WRITE_LINE_MEMBER( vp575_device::exp1_dma_out_w ) { m_dma_out[0] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp2_dma_out_w ) { m_dma_out[1] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp3_dma_out_w ) { m_dma_out[2] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp4_dma_out_w ) { m_dma_out[3] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp5_dma_out_w ) { m_dma_out[4] = state; update_interrupts(); }

WRITE_LINE_MEMBER( vp575_device::exp1_dma_in_w ) { m_dma_in[0] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp2_dma_in_w ) { m_dma_in[1] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp3_dma_in_w ) { m_dma_in[2] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp4_dma_in_w ) { m_dma_in[3] = state; update_interrupts(); }
WRITE_LINE_MEMBER( vp575_device::exp5_dma_in_w ) { m_dma_in[4] = state; update_interrupts(); }

static VIP_EXPANSION_INTERFACE( expansion1_intf )
{
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp1_int_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp1_dma_out_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp1_dma_in_w)
};

static VIP_EXPANSION_INTERFACE( expansion2_intf )
{
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp2_int_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp2_dma_out_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp2_dma_in_w)
};

static VIP_EXPANSION_INTERFACE( expansion3_intf )
{
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp3_int_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp3_dma_out_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp3_dma_in_w)
};

static VIP_EXPANSION_INTERFACE( expansion4_intf )
{
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp4_int_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp4_dma_out_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp4_dma_in_w)
};

static VIP_EXPANSION_INTERFACE( expansion5_intf )
{
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp5_int_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp5_dma_out_w),
    DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, vp575_device, exp5_dma_in_w)
};


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( vp575 )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( vp575 )
    MCFG_VIP_EXPANSION_SLOT_ADD("exp1", XTAL_3_52128MHz/2, expansion1_intf, vip_expansion_cards, NULL, NULL)
    MCFG_VIP_EXPANSION_SLOT_ADD("exp2", XTAL_3_52128MHz/2, expansion2_intf, vip_expansion_cards, NULL, NULL)
    MCFG_VIP_EXPANSION_SLOT_ADD("exp3", XTAL_3_52128MHz/2, expansion3_intf, vip_expansion_cards, NULL, NULL)
    MCFG_VIP_EXPANSION_SLOT_ADD("exp4", XTAL_3_52128MHz/2, expansion4_intf, vip_expansion_cards, NULL, NULL)
    MCFG_VIP_EXPANSION_SLOT_ADD("exp5", XTAL_3_52128MHz/2, expansion5_intf, vip_expansion_cards, NULL, NULL)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor vp575_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( vp575 );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp575_device - constructor
//-------------------------------------------------

vp575_device::vp575_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VP575, "VP575", tag, owner, clock),
	device_vip_expansion_card_interface(mconfig, *this)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_int[i] = CLEAR_LINE;
		m_dma_out[i] = CLEAR_LINE;
		m_dma_in[i] = CLEAR_LINE;
	}
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void vp575_device::device_start()
{
	// find devices
	m_expansion_slot[0] = dynamic_cast<vip_expansion_slot_device *>(subdevice("exp1"));
	m_expansion_slot[1] = dynamic_cast<vip_expansion_slot_device *>(subdevice("exp2"));
	m_expansion_slot[2] = dynamic_cast<vip_expansion_slot_device *>(subdevice("exp3"));
	m_expansion_slot[3] = dynamic_cast<vip_expansion_slot_device *>(subdevice("exp4"));
	m_expansion_slot[4] = dynamic_cast<vip_expansion_slot_device *>(subdevice("exp5"));
}


//-------------------------------------------------
//  vip_program_r - program read
//-------------------------------------------------

UINT8 vp575_device::vip_program_r(address_space &space, offs_t offset, int cs, int cdef, int *minh)
{
	UINT8 data = 0xff;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		data &= m_expansion_slot[i]->program_r(space, offset, cs, cdef, minh);
	}

	return data;
}


//-------------------------------------------------
//  vip_program_w - program write
//-------------------------------------------------

void vp575_device::vip_program_w(address_space &space, offs_t offset, UINT8 data, int cdef, int *minh)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_expansion_slot[i]->program_w(space, offset, data, cdef, minh);
	}
}


//-------------------------------------------------
//  vip_io_r - I/O read
//-------------------------------------------------

UINT8 vp575_device::vip_io_r(address_space &space, offs_t offset)
{
	UINT8 data = 0xff;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		data &= m_expansion_slot[i]->io_r(space, offset);
	}

	return data;
}


//-------------------------------------------------
//  vip_io_w - I/O write
//-------------------------------------------------

void vp575_device::vip_io_w(address_space &space, offs_t offset, UINT8 data)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_expansion_slot[i]->io_w(space, offset, data);
	}
}


//-------------------------------------------------
//  vip_dma_r - DMA read
//-------------------------------------------------

UINT8 vp575_device::vip_dma_r(address_space &space, offs_t offset)
{
	UINT8 data = 0xff;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		data &= m_expansion_slot[i]->dma_r(space, offset);
	}

	return data;
}


//-------------------------------------------------
//  vip_dma_w - DMA write
//-------------------------------------------------

void vp575_device::vip_dma_w(address_space &space, offs_t offset, UINT8 data)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_expansion_slot[i]->dma_w(space, offset, data);
	}
}


//-------------------------------------------------
//  vip_screen_update - screen update
//-------------------------------------------------

UINT32 vp575_device::vip_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 data = 0;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		data |= m_expansion_slot[i]->screen_update(screen, bitmap, cliprect);
	}

	return data;
}


//-------------------------------------------------
//  vip_ef1_r - EF1 flag read
//-------------------------------------------------

int vp575_device::vip_ef1_r()
{
	int state = CLEAR_LINE;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		state |= m_expansion_slot[i]->ef1_r();
	}

	return state;
}


//-------------------------------------------------
//  vip_ef3_r - EF3 flag read
//-------------------------------------------------

int vp575_device::vip_ef3_r()
{
	int state = CLEAR_LINE;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		state |= m_expansion_slot[i]->ef3_r();
	}

	return state;
}


//-------------------------------------------------
//  vip_ef4_r - EF4 flag read
//-------------------------------------------------

int vp575_device::vip_ef4_r()
{
	int state = CLEAR_LINE;

	for (int i = 0; i < MAX_SLOTS; i++)
	{
		state |= m_expansion_slot[i]->ef4_r();
	}

	return state;
}


//-------------------------------------------------
//  vip_sc_w - status code write
//-------------------------------------------------

void vp575_device::vip_sc_w(int data)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_expansion_slot[i]->sc_w(data);
	}
}


//-------------------------------------------------
//  vip_q_w - Q write
//-------------------------------------------------

void vp575_device::vip_q_w(int state)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_expansion_slot[i]->q_w(state);
	}
}


//-------------------------------------------------
//  vip_run_w - RUN write
//-------------------------------------------------

void vp575_device::vip_run_w(int state)
{
	for (int i = 0; i < MAX_SLOTS; i++)
	{
		m_expansion_slot[i]->run_w(state);
	}
}

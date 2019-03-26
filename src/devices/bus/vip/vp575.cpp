// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    RCA VIP Expansion Board VP-575 emulation

**********************************************************************/

#include "emu.h"
#include "vp575.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VP575, vp575_device, "vp575", "VP-575 System Expansion")


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


//-------------------------------------------------
//  machine_config( vp575 )
//-------------------------------------------------

void vp575_device::device_add_mconfig(machine_config &config)
{
	VIP_EXPANSION_SLOT(config, m_expansion_slot[0], XTAL(3'521'280)/2, vip_expansion_cards, nullptr);
	m_expansion_slot[0]->int_wr_callback().set(FUNC(vp575_device::exp1_int_w));
	m_expansion_slot[0]->dma_out_wr_callback().set(FUNC(vp575_device::exp1_dma_out_w));
	m_expansion_slot[0]->dma_in_wr_callback().set(FUNC(vp575_device::exp1_dma_in_w));

	VIP_EXPANSION_SLOT(config, m_expansion_slot[1], XTAL(3'521'280)/2, vip_expansion_cards, nullptr);
	m_expansion_slot[1]->int_wr_callback().set(FUNC(vp575_device::exp2_int_w));
	m_expansion_slot[1]->dma_out_wr_callback().set(FUNC(vp575_device::exp2_dma_out_w));
	m_expansion_slot[1]->dma_in_wr_callback().set(FUNC(vp575_device::exp2_dma_in_w));

	VIP_EXPANSION_SLOT(config, m_expansion_slot[2], XTAL(3'521'280)/2, vip_expansion_cards, nullptr);
	m_expansion_slot[2]->int_wr_callback().set(FUNC(vp575_device::exp3_int_w));
	m_expansion_slot[2]->dma_out_wr_callback().set(FUNC(vp575_device::exp3_dma_out_w));
	m_expansion_slot[2]->dma_in_wr_callback().set(FUNC(vp575_device::exp3_dma_in_w));

	VIP_EXPANSION_SLOT(config, m_expansion_slot[3], XTAL(3'521'280)/2, vip_expansion_cards, nullptr);
	m_expansion_slot[3]->int_wr_callback().set(FUNC(vp575_device::exp4_int_w));
	m_expansion_slot[3]->dma_out_wr_callback().set(FUNC(vp575_device::exp4_dma_out_w));
	m_expansion_slot[3]->dma_in_wr_callback().set(FUNC(vp575_device::exp4_dma_in_w));

	VIP_EXPANSION_SLOT(config, m_expansion_slot[4], XTAL(3'521'280)/2, vip_expansion_cards, nullptr);
	m_expansion_slot[4]->int_wr_callback().set(FUNC(vp575_device::exp5_int_w));
	m_expansion_slot[4]->dma_out_wr_callback().set(FUNC(vp575_device::exp5_dma_out_w));
	m_expansion_slot[4]->dma_in_wr_callback().set(FUNC(vp575_device::exp5_dma_in_w));
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  vp575_device - constructor
//-------------------------------------------------

vp575_device::vp575_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VP575, tag, owner, clock),
	device_vip_expansion_card_interface(mconfig, *this),
	m_expansion_slot(*this, "exp%u", 1)
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
}


//-------------------------------------------------
//  vip_program_r - program read
//-------------------------------------------------

uint8_t vp575_device::vip_program_r(offs_t offset, int cs, int cdef, int *minh)
{
	uint8_t data = 0xff;

	for (auto & elem : m_expansion_slot)
	{
		data &= elem->program_r(offset, cs, cdef, minh);
	}

	return data;
}


//-------------------------------------------------
//  vip_program_w - program write
//-------------------------------------------------

void vp575_device::vip_program_w(offs_t offset, uint8_t data, int cdef, int *minh)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->program_w(offset, data, cdef, minh);
	}
}


//-------------------------------------------------
//  vip_io_r - I/O read
//-------------------------------------------------

uint8_t vp575_device::vip_io_r(offs_t offset)
{
	uint8_t data = 0xff;

	for (auto & elem : m_expansion_slot)
	{
		data &= elem->io_r(offset);
	}

	return data;
}


//-------------------------------------------------
//  vip_io_w - I/O write
//-------------------------------------------------

void vp575_device::vip_io_w(offs_t offset, uint8_t data)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->io_w(offset, data);
	}
}


//-------------------------------------------------
//  vip_dma_r - DMA read
//-------------------------------------------------

uint8_t vp575_device::vip_dma_r(offs_t offset)
{
	uint8_t data = 0xff;

	for (auto & elem : m_expansion_slot)
	{
		data &= elem->dma_r(offset);
	}

	return data;
}


//-------------------------------------------------
//  vip_dma_w - DMA write
//-------------------------------------------------

void vp575_device::vip_dma_w(offs_t offset, uint8_t data)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->dma_w(offset, data);
	}
}


//-------------------------------------------------
//  vip_screen_update - screen update
//-------------------------------------------------

uint32_t vp575_device::vip_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t data = 0;

	for (auto & elem : m_expansion_slot)
	{
		data |= elem->screen_update(screen, bitmap, cliprect);
	}

	return data;
}


//-------------------------------------------------
//  vip_ef1_r - EF1 flag read
//-------------------------------------------------

int vp575_device::vip_ef1_r()
{
	int state = CLEAR_LINE;

	for (auto & elem : m_expansion_slot)
	{
		state |= elem->ef1_r();
	}

	return state;
}


//-------------------------------------------------
//  vip_ef3_r - EF3 flag read
//-------------------------------------------------

int vp575_device::vip_ef3_r()
{
	int state = CLEAR_LINE;

	for (auto & elem : m_expansion_slot)
	{
		state |= elem->ef3_r();
	}

	return state;
}


//-------------------------------------------------
//  vip_ef4_r - EF4 flag read
//-------------------------------------------------

int vp575_device::vip_ef4_r()
{
	int state = CLEAR_LINE;

	for (auto & elem : m_expansion_slot)
	{
		state |= elem->ef4_r();
	}

	return state;
}


//-------------------------------------------------
//  vip_sc_w - status code write
//-------------------------------------------------

void vp575_device::vip_sc_w(int n, int sc)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->sc_w(n, sc);
	}
}


//-------------------------------------------------
//  vip_q_w - Q write
//-------------------------------------------------

void vp575_device::vip_q_w(int state)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->q_w(state);
	}
}


//-------------------------------------------------
//  vip_tpb_w - TPB write
//-------------------------------------------------

void vp575_device::vip_tpb_w(int state)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->tpb_w(state);
	}
}


//-------------------------------------------------
//  vip_run_w - RUN write
//-------------------------------------------------

void vp575_device::vip_run_w(int state)
{
	for (auto & elem : m_expansion_slot)
	{
		elem->run_w(state);
	}
}

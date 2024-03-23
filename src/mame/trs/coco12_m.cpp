// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco12.cpp

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#include "emu.h"
#include "coco12.h"

//-------------------------------------------------
//  device_start
//-------------------------------------------------

void coco12_state::device_start()
{
	coco_state::device_start();
	configure_sam();
}



//-------------------------------------------------
//  coco12_state::configure_sam
//-------------------------------------------------

void coco12_state::configure_sam()
{
	offs_t ramsize = m_ram->size();
	m_sam->space(0).install_ram(0, ramsize - 1, m_ram->pointer());
	if (ramsize < 65536)
		m_sam->space(0).nop_readwrite(ramsize, 0xffff);
}


//-------------------------------------------------
//  horizontal_sync
//-------------------------------------------------

void coco12_state::horizontal_sync(int state)
{
	pia_0().ca1_w(state);
	m_sam->hs_w(state);
}



//-------------------------------------------------
//  field_sync
//-------------------------------------------------

void coco12_state::field_sync(int state)
{
	pia_0().cb1_w(state);
}



//-------------------------------------------------
//  sam_read
//-------------------------------------------------

uint8_t coco12_state::sam_read(offs_t offset)
{
	uint8_t data = sam().display_read(offset);
	m_vdg->as_w(data & 0x80 ? ASSERT_LINE : CLEAR_LINE);
	m_vdg->inv_w(data & 0x40 ? ASSERT_LINE : CLEAR_LINE);
	return data;
}



//-------------------------------------------------
//  pia1_pb_changed
//-------------------------------------------------

void coco12_state::pia1_pb_changed(uint8_t data)
{
	/* call inherited function */
	coco_state::pia1_pb_changed(data);

	m_vdg->css_w(data & 0x08);
	m_vdg->intext_w(data & 0x10);
	m_vdg->gm0_w(data & 0x10);
	m_vdg->gm1_w(data & 0x20);
	m_vdg->gm2_w(data & 0x40);
	m_vdg->ag_w(data & 0x80);
}



//-------------------------------------------------
//  deluxecoco_state::device_start
//-------------------------------------------------

void deluxecoco_state::device_start()
{
	coco12_state::device_start();
	configure_sam();

	m_ram_view.disable();
	m_rom_view.select(0);
}



//-------------------------------------------------
//  deluxecoco_state::configure_sam
//-------------------------------------------------

void deluxecoco_state::configure_sam()
{
	m_sam->space(0).install_view(0x4000, 0x7fff, m_ram_view);

	m_ram_view[0].install_ram(0x4000, 0x7fff, m_ram->pointer() + 0x0000);
	m_ram_view[1].install_ram(0x4000, 0x7fff, m_ram->pointer() + 0x4000);
	m_ram_view[2].install_ram(0x4000, 0x7fff, m_ram->pointer() + 0x8000);
	m_ram_view[3].install_ram(0x4000, 0x7fff, m_ram->pointer() + 0xc000);
}


//-------------------------------------------------
//  deluxecoco::ff30_write
//-------------------------------------------------

void deluxecoco_state::ff30_write(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		if (BIT(data, 2))
			m_ram_view.select(data & 0x03);
		else
			m_ram_view.disable();

		m_rom_view.select(BIT(data, 7));

		if (BIT(data, 6))
		{
			m_timer->adjust(attotime::from_hz(60));
		}
		else
		{
			m_timer->adjust(attotime::never);
			m_irqs->in_w<3>(0);
		}
	}
}


//-------------------------------------------------
//  deluxecoco_state::perodic_timer
//-------------------------------------------------

TIMER_DEVICE_CALLBACK_MEMBER(deluxecoco_state::perodic_timer)
{
	m_irqs->in_w<3>(1);
}

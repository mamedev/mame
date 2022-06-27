// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco12.cpp

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#include "emu.h"
#include "includes/coco12.h"

//-------------------------------------------------
//  device_start
//-------------------------------------------------

void coco12_state::device_start()
{
	coco_state::device_start();
	configure_sam();
}



//-------------------------------------------------
//  configure_sam
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

WRITE_LINE_MEMBER( coco12_state::horizontal_sync )
{
	pia_0().ca1_w(state);
	m_sam->hs_w(state);
}



//-------------------------------------------------
//  field_sync
//-------------------------------------------------

WRITE_LINE_MEMBER( coco12_state::field_sync )
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

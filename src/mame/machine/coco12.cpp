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
	uint8_t *rom = memregion(MAINCPU_TAG)->base();

	m_sam->configure_bank(0, ram().pointer(), ram().size(), false); // $0000-$7FFF
	m_sam->configure_bank(1, &rom[0x0000], 0x2000, true);           // $8000-$9FFF
	m_sam->configure_bank(2, &rom[0x2000], 0x2000, true);           // $A000-$BFFF

	// $C000-$FEFF
	m_sam->configure_bank(3, read8_delegate(*m_cococart, FUNC(cococart_slot_device::cts_read)), write8_delegate(*m_cococart, FUNC(cococart_slot_device::cts_write)));

	// $FF00-$FF1F
	m_sam->configure_bank(4, read8_delegate(*this, FUNC(coco12_state::ff00_read)), write8_delegate(*this, FUNC(coco12_state::ff00_write)));

	// $FF20-$FF3F
	m_sam->configure_bank(5, read8_delegate(*this, FUNC(coco12_state::ff20_read)), write8_delegate(*this, FUNC(coco12_state::ff20_write)));

	// $FF40-$FF5F
	m_sam->configure_bank(6, read8_delegate(*this, FUNC(coco12_state::ff40_read)), write8_delegate(*this, FUNC(coco12_state::ff40_write)));

	// $FF60-$FFBF
	m_sam->configure_bank(7, read8_delegate(*this, FUNC(coco12_state::ff60_read)), write8_delegate(*this, FUNC(coco12_state::ff60_write)));
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

READ8_MEMBER( coco12_state::sam_read )
{
	uint8_t data = ram().read(offset);
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

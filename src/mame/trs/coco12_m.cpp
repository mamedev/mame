// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    coco12.cpp

    TRS-80 Radio Shack Color Computer 1/2 Family

***************************************************************************/

#include "emu.h"
#include "coco12.h"

//-------------------------------------------------
//  machine_start
//-------------------------------------------------

void coco12_state::machine_start()
{
	coco_state::machine_start();

	// right joystick
	m_joy_handlers[0] = std::make_unique<coco_joy_standard>(*this, 0, ioport(JOYSTICK_BUTTONS_TAG));
	// left joystick
	m_joy_handlers[1] = std::make_unique<coco_joy_standard>(*this, 2, ioport(JOYSTICK_BUTTONS_TAG));
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
//  vdg_flush
//-------------------------------------------------

void coco12_state::vdg_flush(int state)
{
	m_vdg->flush_pending_video();
}



//-------------------------------------------------
//  pia1_pb_changed
//-------------------------------------------------

void coco12_state::pia1_pb_w(uint8_t data)
{
	/* call inherited function */
	coco_state::pia1_pb_w(data);

	m_vdg->css_w(data & 0x08);
	m_vdg->intext_w(data & 0x10);
	m_vdg->gm0_w(data & 0x10);
	m_vdg->gm1_w(data & 0x20);
	m_vdg->gm2_w(data & 0x40);
	m_vdg->ag_w(data & 0x80);
}



//-------------------------------------------------
//  deluxecoco_state::machine_start
//-------------------------------------------------

void deluxecoco_state::machine_start()
{
	coco12_state::machine_start();

	// bit 2 points physical RAM $4000-$7FFF at one of the four 16K blocks
	if (m_ram->size() >= 0x10000)
	{
		std::vector<std::pair<uint8_t *, uint8_t *> > blocks;
		for (int i = 0; i < 4; i++)
			blocks.emplace_back(m_ram->pointer() + (i * 0x4000), m_ram->pointer() + (i * 0x4000));
		m_sam->configure_ram_bank(0x4000, 0x7fff, std::move(blocks));
		update_ram_bank();
	}

	save_item(NAME(m_ram_bank_select));
	save_item(NAME(m_rom_select));
	machine().save().register_postload(save_prepost_delegate(FUNC(deluxecoco_state::update_ram_bank), this));
}



//-------------------------------------------------
//  deluxecoco_state::update_ram_bank
//-------------------------------------------------

void deluxecoco_state::update_ram_bank()
{
	if (m_ram->size() < 0x10000)
		return;

	m_sam->select_ram_bank(0x4000, BIT(m_ram_bank_select, 2) ? (m_ram_bank_select & 0x03) : 1);
}


//-------------------------------------------------
//  deluxecoco::ff30_write
//-------------------------------------------------

void deluxecoco_state::ff30_write(offs_t offset, uint8_t data)
{
	if (offset == 0)
	{
		m_ram_bank_select = data & 0x07;
		update_ram_bank();

		m_rom_select = BIT(data, 7);

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

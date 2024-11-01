// license:BSD-3-Clause
// copyright-holders:Robbbert,Nigel Barnes
// thanks-to:Andrew Trotman
/***************************************************************************

    Poly 1/2 (New Zealand)

****************************************************************************/

#include "emu.h"
#include "poly.h"


inline offs_t poly_state::physical(offs_t offset)
{
	return (((~m_dat[(m_dat_bank << 3) | (offset >> 13)]) & 0x0f) << 13) | (offset & 0x1fff);
}

uint8_t poly_state::logical_mem_r(offs_t offset)
{
	return m_user->base()[physical(offset)];
}

void poly_state::logical_mem_w(offs_t offset, uint8_t data)
{
	m_user->base()[physical(offset)] = data;
}


uint8_t poly_state::vector_r(offs_t offset)
{
	/* system mode is selected by a vector fetch (interrupt and reset) */
	if (!machine().side_effects_disabled())
		m_memview.select(0);

	return m_system->base()[offset & 0x0fff];
}


TIMER_CALLBACK_MEMBER(poly_state::set_protect)
{
	m_memview.select(1);
}

void poly_state::set_protect_w(uint8_t data)
{
	/* set protect after 1 E cycle */
	m_protect_timer->adjust(m_maincpu->cycles_to_attotime(2));
}


uint8_t poly_state::select_map_r()
{
	return 0x00;
}

void poly_state::select_map1_w(uint8_t data)
{
	m_dat_bank = 0;
}

void poly_state::select_map2_w(uint8_t data)
{
	m_dat_bank = 1;
}


void poly_state::pia0_pa_w(uint8_t data)
{
	logerror("PIA0: Port A out: %02X\n", data);
	m_video_pa = data;
	m_ptm->set_g3(BIT(data, 7));
}

void poly_state::pia0_pb_w(uint8_t data)
{
	logerror("PIA0: Port B out: %02X\n", data);
	m_video_pb = data;
}


uint8_t poly_state::pia1_b_in()
{
	// B1 - D0
	// B2 - D1
	// B3 - D2
	// B4 - D3
	// B5 - D4
	// B6
	// B7
	// B8 - D6
	// STROBE - D7

	/* return key code */
	//data = m_kr2376->data_r(space, 0) & 0x7f;
	/* bit 5 is derived from data bits 5,6 and caps lock */
	//data |= !(!(BIT(m_kr2376->data_r(space, 0), 6) && !caps_lock) && !(caps_lock && BIT(m_kr2376->data_r(space, 0), 5))) << 5;
	/* bit 6 is data bit 7 */
	//data |= BIT(m_kr2376->data_r(space, 0), 7) << 6;
	/* caps lock needs verifying */
	//data |= ((caps_lock && BIT(m_kr2376->data_r(space, 0), 6)) ? 0 : BIT(m_kr2376->data_r(space, 0), 5)) << 5;
	/* bit 7 is the strobe value */
	//data |= m_kr2376->get_output_pin(kr2376_device::KR2376_SO) << 7;

	// return ascii key value, bit 7 is the strobe value
	uint8_t data = m_term_data;
	m_term_data &= 0x7f;
	return data;
}


uint8_t poly_state::videoram_1_r(offs_t offset)
{
	return m_videoram[offset];
}

uint8_t poly_state::videoram_2_r(offs_t offset)
{
	return m_videoram[offset + 0x400];
}


void poly_state::kbd_put(u8 data)
{
	m_term_data = data | 0x80;

	m_pia[1]->cb1_w(1);
	m_pia[1]->cb1_w(0);
}

int poly_state::kbd_shift_r()
{
	return BIT(m_modifiers->read(), 1);
}

int poly_state::kbd_control_r()
{
	return BIT(m_modifiers->read(), 0);
}


void poly_state::ptm_o2_callback(int state)
{
	m_ptm->set_c1(state);
}

void poly_state::ptm_o3_callback(int state)
{
	m_speaker->level_w(state);
}

void poly_state::baud_rate_w (uint8_t data)
{
	/* baud rate controller (0=9600,2=4800,4=2400,6=1200,8=600,A=300) */
	int selector = (data & 0x0e) >> 1;
	m_acia_clock->set_clock_scale((selector <= 5) ? 1.0 / (1 << selector) : 0.0);
}

uint8_t poly_state::network_r(offs_t offset)
{
	return m_adlc->read(offset >> 1);
}

void poly_state::network_w(offs_t offset, uint8_t data)
{
	m_adlc->write(offset >> 1, data);
}

void poly_state::network_clk_w(int state)
{
	m_adlc->rxc_w(state);
	m_adlc->txc_w(state);
}


void polydev_state::drive_register_w(uint8_t data)
{
	/* drive select */
	switch (data & 0x03)
	{
	case 0:
		m_current_floppy = m_floppy[0]->get_device();
		break;

	case 1:
		m_current_floppy = m_floppy[1]->get_device();
		break;

	default:
		m_current_floppy = nullptr;
		break;
	}
	m_fdc->set_floppy(m_current_floppy);

	if (m_current_floppy)
	{
		/* side select */
		m_current_floppy->ss_w(BIT(data, 6));
	}
}

uint8_t polydev_state::drive_register_r()
{
	/* disk change */
	return (m_current_floppy ? m_current_floppy->dskchg_r() : 1) << 1;
}

void polydev_state::motor_w(int state)
{
	if (m_current_floppy) m_current_floppy->mon_w(!state);
}

uint8_t polydev_state::fdc_inv_r(offs_t offset)
{
	return m_fdc->read(offset) ^ 0xff;
}

void polydev_state::fdc_inv_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset, data ^ 0xff);
}


uint32_t poly_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/*
	    PA0     Not used. Connected to CA2.
	    PA1  (  Mix colour  0) Blue   1) Green  0) Red    1) No
	    PA2  (  Graphics 1  0)        0)        1)        1) Mix
	    PA3     1 = Enable Teletext 1 screen
	    PA4     1 = 480 Graphics mode. 0 = 240 Graphics mode.
	    PA5     1 = Enable Graphics 1 screen or 480 Graphics screen.
	    PA6     1 = Mix Mode 0 = Priority mode
	    PA7     Must be 0. Connected to PA7 and timer G3.

	    PB0     1 = Enable Teletext 2 screen.
	    PB1     1 = Enable Graphics 2 (must be 0 for 480 Graphics)
	    PB2  (  Mix colour  0) Blue    1) Green  0) Red    1) No
	    PB3  (  Graphics 2  0)         0)        1)        1) Mix
	    PB4     1 = Enable Red Background
	    PB5     1 = Enable Green Background
	    PB6     1 = Enable Blue Background
	    PB7     Not used.

	    CA1     Connected to PA7 and timer G3. Must be 0.
	    CA2     Not used. Connected to PA0.
	    CB1     Not used.
	    CB2     Not used.
	*/

	/* Enable Teletext 1 screen */
	if (BIT(m_video_pa, 3))
		m_trom[0]->screen_update(screen, bitmap, cliprect);

	/* Enable Teletext 2 screen */
	if (BIT(m_video_pb, 0))
		m_trom[1]->screen_update(screen, bitmap, cliprect);

	return 0;
}

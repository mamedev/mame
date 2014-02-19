#include "emu.h"
#include "crsshair.h"
#include "video/315_5124.h"
#include "sound/2413intf.h"
#include "includes/sms.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define ENABLE_NONE      0
#define ENABLE_EXPANSION 1
#define ENABLE_CARD      2
#define ENABLE_CART      3
#define ENABLE_BIOS      4


void sms_state::lphaser_hcount_latch()
{
	/* A delay seems to occur when the Light Phaser latches the
	   VDP hcount, then an offset is added here to the hpos. */
	m_vdp->hcount_latch_at_hpos(m_main_scr->hpos() + m_lphaser_x_offs);
}


WRITE_LINE_MEMBER(sms_state::sms_ctrl1_th_input)
{
	// Check if TH of controller port 1 is set to input (1)
	if (m_io_ctrl_reg & 0x02)
	{
		if (state == 0)
		{
			m_ctrl1_th_latch = 1;
		}
		else
		{
			// State is 1. If changed from 0, hcount is latched.
			if (m_ctrl1_th_state == 0)
				lphaser_hcount_latch();
		}
		m_ctrl1_th_state = state;
	}
}


WRITE_LINE_MEMBER(sms_state::sms_ctrl2_th_input)
{
	// Check if TH of controller port 2 is set to input (1)
	if (m_io_ctrl_reg & 0x08)
	{
		if (state == 0)
		{
			m_ctrl2_th_latch = 1;
		}
		else
		{
			// State is 1. If changed from 0, hcount is latched.
			if (m_ctrl2_th_state == 0)
				lphaser_hcount_latch();
		}
		m_ctrl2_th_state = state;
	}
}


void sms_state::sms_get_inputs( address_space &space )
{
	UINT8 data1, data2;

	m_port_dc_reg = 0xff;
	m_port_dd_reg = 0xff;

	// The bit order of the emulated controller port tries to follow its
	// physical pins numbering. For register bits whose order differs,
	// it's necessary move the equivalent controller bits to match.

	data1 = m_port_ctrl1->port_r();
	m_port_dc_reg &= ~0x0f | data1; // Up, Down, Left, Right
	m_port_dc_reg &= ~0x10 | (data1 >> 1); // TL (Button 1)
	m_port_dc_reg &= ~0x20 | (data1 >> 2); // TR (Button 2)

	data2 = m_port_ctrl2->port_r();
	m_port_dc_reg &= ~0xc0 | (data2 << 6); // Up, Down
	m_port_dd_reg &= ~0x03 | (data2 >> 2); // Left, Right
	m_port_dd_reg &= ~0x04 | (data2 >> 3); // TL (Button 1)
	m_port_dd_reg &= ~0x08 | (data2 >> 4); // TR (Button 2)

	// Sega Mark III does not have TH line connected.
	if (!m_is_mark_iii)
	{
		m_port_dd_reg &= ~0x40 | data1; // TH ctrl1
		m_port_dd_reg &= ~0x80 | (data2 << 1); // TH ctrl2
	}
}


WRITE8_MEMBER(sms_state::sms_fm_detect_w)
{
	if (m_has_fm)
		m_fm_detect = (data & 0x01);
}


READ8_MEMBER(sms_state::sms_fm_detect_r)
{
	if (m_has_fm)
	{
		return m_fm_detect;
	}
	else
	{
		if (!m_is_mark_iii && (m_bios_port & IO_CHIP))
		{
			return 0xff;
		}
		else
		{
			sms_get_inputs(space);
			return m_port_dc_reg;
		}
	}
}


WRITE8_MEMBER(sms_state::sms_io_control_w)
{
	bool latch_hcount = false;
	UINT8 ctrl1_port_data = 0xff;
	UINT8 ctrl2_port_data = 0xff;

	// Controller Port 1:

	// check if TR or TH are set to output (0).
	if ((data & 0x03) != 0x03)
	{
		if (!(data & 0x01)) // TR set to output
		{
			ctrl1_port_data &= ~0x80 | (data << 3);
		}
		if (!(data & 0x02)) // TH set to output
		{
			ctrl1_port_data &= ~0x40 | (data << 1);
		}
		if (!m_is_gamegear)
			m_port_ctrl1->port_w(ctrl1_port_data);
	}
	// check if TH is set to input (1).
	if (data & 0x02)
	{
		if (!m_is_gamegear)
			ctrl1_port_data &= ~0x40 | m_port_ctrl1->port_r();

		// check if TH input level is high (1) and was output/low (0)
		if ((ctrl1_port_data & 0x40) && !(m_io_ctrl_reg & 0x22))
			latch_hcount = true;
	}

	// Controller Port 2:

	// check if TR or TH are set to output (0).
	if ((data & 0x0c) != 0x0c)
	{
		if (!(data & 0x04)) // TR set to output
		{
			ctrl2_port_data &= ~0x80 | (data << 1);
		}
		if (!(data & 0x08)) // TH set to output
		{
			ctrl2_port_data &= ~0x40 | (data >> 1);
		}
		if (!m_is_gamegear)
			m_port_ctrl2->port_w(ctrl2_port_data);
	}
	// check if TH is set to input (1).
	if (data & 0x08)
	{
		if (!m_is_gamegear)
			ctrl2_port_data &= ~0x40 | m_port_ctrl2->port_r();

		// check if TH input level is high (1) and was output/low (0)
		if ((ctrl2_port_data & 0x40) && !(m_io_ctrl_reg & 0x88))
			latch_hcount = true;
	}

	if (latch_hcount)
	{
		m_vdp->hcount_latch();
	}
	m_io_ctrl_reg = data;
}


READ8_MEMBER(sms_state::sms_count_r)
{
	if (offset & 0x01)
		return m_vdp->hcount_read(*m_space, offset);
	else
		return m_vdp->vcount_read(*m_space, offset);
}


/*
 Check if the pause button is pressed.
 If the gamegear is in sms mode, check if the start button is pressed.
 */
WRITE_LINE_MEMBER(sms_state::sms_pause_callback)
{
	if (m_is_gamegear && m_cartslot->m_cart && !m_cartslot->m_cart->get_sms_mode())
		return;

	if ((m_is_gamegear && !(m_port_start->read() & 0x80)) || (!m_is_gamegear && !(m_port_pause->read() & 0x80)))
	{
		if (!m_paused)
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);

		m_paused = 1;
	}
	else
		m_paused = 0;

	if (!m_is_mark_iii)
	{
		// clear TH latch of the controller ports
		m_ctrl1_th_latch = 0;
		m_ctrl2_th_latch = 0;
	}
}


READ8_MEMBER(sms_state::sms_input_port_dc_r)
{
	if (m_is_mark_iii)
	{
		sms_get_inputs(space);
		return m_port_dc_reg;
	}

	if (m_bios_port & IO_CHIP)
	{
		return 0xff;
	}
	else
	{
		sms_get_inputs(space);

		// Check if TR of controller port 1 is set to output (0)
		if (!(m_io_ctrl_reg & 0x01))
		{
			// Read TR state set through IO control port
			m_port_dc_reg &= ~0x20 | ((m_io_ctrl_reg & 0x10) << 1);
		}

		return m_port_dc_reg;
	}
}


READ8_MEMBER(sms_state::sms_input_port_dd_r)
{
	if (m_is_mark_iii)
	{
		sms_get_inputs(space);
		return m_port_dd_reg;
	}

	if (m_bios_port & IO_CHIP)
		return 0xff;

	sms_get_inputs(space);

	// Reset Button
	if ( m_port_reset )
	{
		m_port_dd_reg &= ~0x10 | (m_port_reset->read() & 0x01) << 4;
	}

	// Check if TR of controller port 2 is set to output (0)
	if (!(m_io_ctrl_reg & 0x04))
	{
		// Read TR state set through IO control port
		m_port_dd_reg &= ~0x08 | ((m_io_ctrl_reg & 0x40) >> 3);
	}

	// Check if TH of controller port 1 is set to output (0)
	if (!(m_io_ctrl_reg & 0x02))
	{
		// Read TH state set through IO control port
		m_port_dd_reg &= ~0x40 | ((m_io_ctrl_reg & 0x20) << 1);
	}
	else  // TH set to input (1)
	{
		if (m_ctrl1_th_latch)
		{
			m_port_dd_reg &= ~0x40;
			m_ctrl1_th_latch = 0;
		}
	}

	// Check if TH of controller port 2 is set to output (0)
	if (!(m_io_ctrl_reg & 0x08))
	{
		// Read TH state set through IO control port
		m_port_dd_reg &= ~0x80 | (m_io_ctrl_reg & 0x80);
	}
	else  // TH set to input (1)
	{
		if (m_ctrl2_th_latch)
		{
			m_port_dd_reg &= ~0x80;
			m_ctrl2_th_latch = 0;
		}
	}

	return m_port_dd_reg;
}


WRITE8_MEMBER(sms_state::sms_ym2413_register_port_w)
{
	if (m_has_fm)
		m_ym->write(space, 0, (data & 0x3f));
}


WRITE8_MEMBER(sms_state::sms_ym2413_data_port_w)
{
	if (m_has_fm)
	{
		//logerror("data_port_w %x %x\n", offset, data);
		m_ym->write(space, 1, data);
	}
}


READ8_MEMBER(sms_state::gg_input_port_2_r)
{
	//logerror("joy 2 read, val: %02x, pc: %04x\n", ((m_is_region_japan ? 0x00 : 0x40) | (m_port_start->read() & 0x80)), activecpu_get_pc());
	return ((m_is_region_japan ? 0x00 : 0x40) | (m_port_start->read() & 0x80));
}


READ8_MEMBER(sms_state::sms_sscope_r)
{
	int sscope = m_port_scope->read();

	if ( sscope )
	{
		// Scope is attached
		return m_sscope_state;
	}

	return m_mainram[0x1FF8 + offset];
}


WRITE8_MEMBER(sms_state::sms_sscope_w)
{
	m_mainram[0x1FF8 + offset] = data;

	int sscope = m_port_scope->read();

	if ( sscope )
	{
		// Scope is attached
		m_sscope_state = data;

		// There are occurrences when Sega Scope's state changes after VBLANK, or at
		// active screen. Most cases are solid-color frames of scene transitions, but
		// one exception is the first frame of Zaxxon 3-D's title screen. In that
		// case, this method is enough for setting the intended state for the frame.
		// No information found about a minimum time need for switch open/closed lens.
		if (m_main_scr->vpos() < (m_main_scr->height() >> 1))
		{
			m_frame_sscope_state = m_sscope_state;
		}
	}
}


READ8_MEMBER(sms_state::sms_mapper_r)
{
	return m_mainram[0x1ffc + offset];
}


WRITE8_MEMBER(sms_state::sms_mapper_w)
{
	bool bios_selected = false;
	bool cartridge_selected = false;
	bool card_selected = false;
	bool expansion_selected = false;

	m_mapper[offset] = data;
	m_mainram[0x1ffc + offset] = data;

	if (m_is_mark_iii)
	{
		if (m_cartslot && m_cartslot->m_cart)
			cartridge_selected = true;
		else if (m_cardslot && m_cardslot->m_cart)
			card_selected = true;
		else
			return; // nothing to page in
	}
	else if (m_bios_port & IO_BIOS_ROM || (m_is_gamegear && m_BIOS == NULL))
	{
		if (!(m_bios_port & IO_CARTRIDGE) || (m_is_gamegear && m_BIOS == NULL))
			cartridge_selected = true;
		else if (!(m_bios_port & IO_CARD))
			card_selected = true;
		else if (!(m_bios_port & IO_EXPANSION))
			expansion_selected = true;
		else
			return; // nothing to page in
	}
	else
	{
		if (!m_BIOS)
			return; // nothing to page in
		bios_selected = true;
	}

	switch (offset)
	{
		case 0: // Control RAM/ROM
			if (!(data & 0x08) && bios_selected && !(m_bios_port & IO_BIOS_ROM))    // BIOS ROM
			{
				if (!m_has_bios_0400 && !m_has_bios_2000)
				{
					m_bank_enabled[2] = ENABLE_BIOS;
					m_bios_page[2] = m_mapper[3];
				}
			}
			else if (cartridge_selected)    // CART ROM/RAM
			{
				m_bank_enabled[2] = ENABLE_CART;
				m_cartslot->write_mapper(space, offset, data);
			}
			else if (card_selected)    // CARD ROM/RAM
			{
				m_bank_enabled[2] = ENABLE_CARD;
				m_cardslot->write_mapper(space, offset, data);
			}
			else if (expansion_selected)    // expansion slot
			{
				m_bank_enabled[2] = ENABLE_EXPANSION;
				m_expslot->write_mapper(space, offset, data);
			}
			break;

		case 1: // Select 16k ROM bank for 0400-3fff
		case 2: // Select 16k ROM bank for 4000-7fff
		case 3: // Select 16k ROM bank for 8000-bfff
			if (bios_selected)
			{
				if (!m_has_bios_0400 && !m_has_bios_2000)
				{
					m_bank_enabled[offset - 1] = ENABLE_BIOS;
					m_bios_page[offset - 1] = data & (m_bios_page_count - 1);
				}
			}
			else if (cartridge_selected || m_is_gamegear)
			{
				m_bank_enabled[offset - 1] = ENABLE_CART;
				m_cartslot->write_mapper(space, offset, data);
			}
			else if (card_selected)
			{
				m_bank_enabled[offset - 1] = ENABLE_CARD;
				m_cardslot->write_mapper(space, offset, data);
			}
			else if (expansion_selected)
			{
				m_bank_enabled[offset - 1] = ENABLE_EXPANSION;
				m_expslot->write_mapper(space, offset, data);
			}
			break;
	}
}


READ8_MEMBER(sms_state::read_0000)
{
	if (offset < 0x400)
	{
		if (m_bank_enabled[3] == ENABLE_BIOS)
		{
			if (m_BIOS)
				return m_BIOS[(m_bios_page[3] * 0x4000) + (offset & 0x3fff)];
		}
		if (m_bank_enabled[3] == ENABLE_CART)
			return m_cartslot->read_cart(space, offset);
		if (m_cardslot && m_bank_enabled[3] == ENABLE_CARD)
			return m_cardslot->read_cart(space, offset);
		if (m_expslot && m_bank_enabled[3] == ENABLE_EXPANSION)
			return m_expslot->read(space, offset);
	}
	else
	{
		if (m_bank_enabled[0] == ENABLE_BIOS)
		{
			if (m_BIOS)
				return m_BIOS[(m_bios_page[0] * 0x4000) + (offset & 0x3fff)];
		}
		if (m_bank_enabled[0] == ENABLE_CART)
			return m_cartslot->read_cart(space, offset);
		if (m_cardslot && m_bank_enabled[0] == ENABLE_CARD)
			return m_cardslot->read_cart(space, offset);
		if (m_expslot && m_bank_enabled[0] == ENABLE_EXPANSION)
			return m_expslot->read(space, offset);
	}
	return m_region_maincpu->base()[offset];
}

READ8_MEMBER(sms_state::read_4000)
{
	if (m_bank_enabled[1] == ENABLE_BIOS)
	{
		if (m_BIOS)
			return m_BIOS[(m_bios_page[1] * 0x4000) + (offset & 0x3fff)];
	}

	if (m_bank_enabled[1] == ENABLE_CART)
		return m_cartslot->read_cart(space, offset + 0x4000);
	if (m_cardslot && m_bank_enabled[1] == ENABLE_CARD)
		return m_cardslot->read_cart(space, offset + 0x4000);
	if (m_expslot && m_bank_enabled[1] == ENABLE_EXPANSION)
		return m_expslot->read(space, offset + 0x4000);

	return m_region_maincpu->base()[offset];
}

READ8_MEMBER(sms_state::read_8000)
{
	if (m_bank_enabled[2] == ENABLE_BIOS)
	{
		if (m_BIOS)
			return m_BIOS[(m_bios_page[2] * 0x4000) + (offset & 0x3fff)];
	}

	if (m_bank_enabled[2] == ENABLE_CART)
		return m_cartslot->read_cart(space, offset + 0x8000);
	if (m_cardslot && m_bank_enabled[2] == ENABLE_CARD)
		return m_cardslot->read_cart(space, offset + 0x8000);
	if (m_expslot && m_bank_enabled[2] == ENABLE_EXPANSION)
		return m_expslot->read(space, offset + 0x8000);

	return m_region_maincpu->base()[offset];
}

WRITE8_MEMBER(sms_state::write_cart)
{
	if (m_bank_enabled[0] == ENABLE_CART)
		m_cartslot->write_cart(space, offset, data);
	if (m_cardslot && m_bank_enabled[0] == ENABLE_CARD)
		m_cardslot->write_cart(space, offset, data);
	if (m_expslot && m_bank_enabled[0] == ENABLE_EXPANSION)
		m_expslot->write(space, offset, data);
}

READ8_MEMBER(smssdisp_state::store_read_0000)
{
	if (offset < 0x400)
	{
		if (m_bank_enabled[3] == ENABLE_BIOS)
		{
			if (m_BIOS)
				return m_BIOS[(m_bios_page[3] * 0x4000) + (offset & 0x3fff)];
		}
		if (m_bank_enabled[3] == ENABLE_CART)
			return m_slots[m_current_cartridge]->read_cart(space, offset);
		if (m_bank_enabled[3] == ENABLE_CARD)
			return m_cards[m_current_cartridge]->read_cart(space, offset);
	}
	else
	{
		if (m_bank_enabled[0] == ENABLE_BIOS)
		{
			if (m_BIOS)
				return m_BIOS[(m_bios_page[0] * 0x4000) + (offset & 0x3fff)];
		}
		if (m_bank_enabled[0] == ENABLE_CART)
			return m_slots[m_current_cartridge]->read_cart(space, offset);
		if (m_bank_enabled[0] == ENABLE_CARD)
			return m_cards[m_current_cartridge]->read_cart(space, offset);
	}

	return m_region_maincpu->base()[offset];
}

READ8_MEMBER(smssdisp_state::store_read_4000)
{
	if (m_bank_enabled[1] == ENABLE_BIOS)
	{
		if (m_BIOS)
			return m_BIOS[(m_bios_page[1] * 0x4000) + (offset & 0x3fff)];
	}
	if (m_bank_enabled[1] == ENABLE_CART)
		return m_slots[m_current_cartridge]->read_cart(space, offset + 0x4000);
	if (m_bank_enabled[1] == ENABLE_CARD)
		return m_cards[m_current_cartridge]->read_cart(space, offset + 0x4000);

	return m_region_maincpu->base()[offset];
}

READ8_MEMBER(smssdisp_state::store_read_8000)
{
	if (m_bank_enabled[2] == ENABLE_BIOS)
	{
		if (m_BIOS)
			return m_BIOS[(m_bios_page[2] * 0x4000) + (offset & 0x3fff)];
	}
	if (m_bank_enabled[2] == ENABLE_CART)
		return m_slots[m_current_cartridge]->read_cart(space, offset + 0x8000);
	if (m_bank_enabled[2] == ENABLE_CARD)
		return m_cards[m_current_cartridge]->read_cart(space, offset + 0x8000);

	return m_region_maincpu->base()[offset];
}

WRITE8_MEMBER(smssdisp_state::store_write_cart)
{
	// this might only work because we are not emulating properly the cart/card selection system
	// it will have to be reviewed when proper emulation is worked on!
	if (m_bank_enabled[0] == ENABLE_CART)
		m_slots[m_current_cartridge]->write_cart(space, offset, data);
	if (m_bank_enabled[0] == ENABLE_CARD)
		m_cards[m_current_cartridge]->write_cart(space, offset, data);
}

READ8_MEMBER(smssdisp_state::store_cart_peek)
{
	if (m_bank_enabled[1] == ENABLE_CART)
		return m_slots[m_current_cartridge]->read_cart(space, 0x4000 + (offset & 0x1fff));
	if (m_bank_enabled[1] == ENABLE_CARD)
		return m_cards[m_current_cartridge]->read_cart(space, 0x4000 + (offset & 0x1fff));

	return m_region_maincpu->base()[offset];
}

WRITE8_MEMBER(sms_state::sms_bios_w)
{
	m_bios_port = data;

	logerror("bios write %02x, pc: %04x\n", data, space.device().safe_pc());

	setup_rom();
}


WRITE8_MEMBER(sms_state::gg_sio_w)
{
	logerror("*** write %02X to SIO register #%d\n", data, offset);

	m_gg_sio[offset & 0x07] = data;
	switch (offset & 7)
	{
		case 0x00: /* Parallel Data */
			break;

		case 0x01: /* Data Direction / NMI Enable */
			break;

		case 0x02: /* Serial Output */
			break;

		case 0x03: /* Serial Input */
			break;

		case 0x04: /* Serial Control / Status */
			break;
	}
}


READ8_MEMBER(sms_state::gg_sio_r)
{
	logerror("*** read SIO register #%d\n", offset);

	switch (offset & 7)
	{
		case 0x00: /* Parallel Data */
			break;

		case 0x01: /* Data Direction / NMI Enable */
			break;

		case 0x02: /* Serial Output */
			break;

		case 0x03: /* Serial Input */
			break;

		case 0x04: /* Serial Control / Status */
			break;
	}

	return m_gg_sio[offset];
}


void sms_state::setup_rom()
{
	m_bank_enabled[3] = ENABLE_NONE;
	m_bank_enabled[0] = ENABLE_NONE;
	m_bank_enabled[1] = ENABLE_NONE;
	m_bank_enabled[2] = ENABLE_NONE;

	if (m_is_mark_iii)
	{
		// Mark III uses the card slot by default, but has hardware method
		// that prioritizes the cartridge slot if it has a cartridge inserted.
		if (m_cartslot && m_cartslot->m_cart)
		{
			m_bank_enabled[3] = ENABLE_CART;
			m_bank_enabled[0] = ENABLE_CART;
			m_bank_enabled[1] = ENABLE_CART;
			m_bank_enabled[2] = ENABLE_CART;
			logerror("Switched in cartridge rom.\n");
		}
		else
		{
			m_bank_enabled[3] = ENABLE_CARD;
			m_bank_enabled[0] = ENABLE_CARD;
			m_bank_enabled[1] = ENABLE_CARD;
			m_bank_enabled[2] = ENABLE_CARD;
			logerror("Switching to card rom port.\n");
		}
		return;
	}

	/* 2. check and set up expansion port */
	if (!(m_bios_port & IO_EXPANSION) && (m_bios_port & IO_CARTRIDGE) && (m_bios_port & IO_CARD))
	{
		m_bank_enabled[3] = ENABLE_EXPANSION;
		m_bank_enabled[0] = ENABLE_EXPANSION;
		m_bank_enabled[1] = ENABLE_EXPANSION;
		m_bank_enabled[2] = ENABLE_EXPANSION;
		logerror("Switching to expansion port.\n");
	}

	/* 3. check and set up card rom */
	if (!(m_bios_port & IO_CARD) && (m_bios_port & IO_CARTRIDGE) && (m_bios_port & IO_EXPANSION))
	{
		m_bank_enabled[3] = ENABLE_CARD;
		m_bank_enabled[0] = ENABLE_CARD;
		m_bank_enabled[1] = ENABLE_CARD;
		m_bank_enabled[2] = ENABLE_CARD;
		logerror("Switching to card rom port.\n");
	}

	/* 4. check and set up cartridge rom */
	/* if ((!(bios_port & IO_CARTRIDGE) && (bios_port & IO_EXPANSION) && (bios_port & IO_CARD)) || state->m_is_gamegear) { */
	/* Out Run Europa initially writes a value to port 3E where IO_CARTRIDGE, IO_EXPANSION and IO_CARD are reset */
	if ((!(m_bios_port & IO_CARTRIDGE)) || m_is_gamegear)
	{
		m_bank_enabled[3] = ENABLE_CART;
		m_bank_enabled[0] = ENABLE_CART;
		m_bank_enabled[1] = ENABLE_CART;
		m_bank_enabled[2] = ENABLE_CART;
		logerror("Switched in cartridge rom.\n");
	}

	/* 5. check and set up bios rom */
	if (!(m_bios_port & IO_BIOS_ROM))
	{
		if (m_has_bios_0400)
		{
			// 1K bios
			m_bank_enabled[3] = ENABLE_BIOS;
			logerror("Switched in 0x0400 bios.\n");
		}
		if (m_has_bios_2000)
		{
			// 8K bios
			m_bank_enabled[3] = ENABLE_BIOS;
			m_bank_enabled[0] = ENABLE_BIOS;
			logerror("Switched in 0x2000 bios.\n");
		}
		if (m_has_bios_full)
		{
			// larger bios
			m_bank_enabled[3] = ENABLE_BIOS;
			m_bank_enabled[0] = ENABLE_BIOS;
			m_bank_enabled[1] = ENABLE_BIOS;
			m_bank_enabled[2] = ENABLE_BIOS;
			logerror("Switched in full bios.\n");
		}
	}
}


void sms_state::setup_bios()
{
	m_BIOS = memregion("user1")->base();
	m_bios_page_count = (m_BIOS ? memregion("user1")->bytes() / 0x4000 : 0);

	if (m_BIOS == NULL || m_BIOS[0] == 0x00)
	{
		m_BIOS = NULL;
		m_has_bios_0400 = 0;
		m_has_bios_2000 = 0;
		m_has_bios_full = 0;
	}

	if (m_BIOS)
	{
		m_bios_page[3] = 0;
		m_bios_page[0] = 0;
		m_bios_page[1] = (1 < m_bios_page_count) ? 1 : 0;
		m_bios_page[2] = (2 < m_bios_page_count) ? 2 : 0;
	}

	if (!m_is_mark_iii)
	{
		m_bios_port = (IO_EXPANSION | IO_CARTRIDGE | IO_CARD);
		if (!m_BIOS)
		{
			m_bios_port &= ~(IO_CARTRIDGE);
			m_bios_port |= IO_BIOS_ROM;
		}
	}
}

MACHINE_START_MEMBER(sms_state,sms)
{
	char str[7];

	m_space = &m_maincpu->space(AS_PROGRAM);

	// alibaba and blockhol are ports of games for the MSX system. The
	// MSX bios usually initializes callback "vectors" at the top of RAM.
	// The code in alibaba does not do this so the IRQ vector only contains
	// the "call $4010" without a following RET statement. That is basically
	// a bug in the program code. The only way this cartridge could have run
	// successfully on a real unit is if the RAM would be initialized with
	// a F0 pattern on power up; F0 = RET P. Do that only for consoles of
	// Japan region (including Korea), until confirmed on other consoles.
	if (m_is_region_japan)
	{
		memset((UINT8*)m_space->get_write_ptr(0xc000), 0xf0, 0x1fff);
	}

	save_item(NAME(m_paused));
	save_item(NAME(m_mapper));
	save_item(NAME(m_port_dc_reg));
	save_item(NAME(m_port_dd_reg));
	save_item(NAME(m_bank_enabled));

	if (m_has_fm)
	{
		save_item(NAME(m_fm_detect));
	}

	if (!m_is_mark_iii)
	{
		save_item(NAME(m_bios_port));
		save_item(NAME(m_bios_page));
		save_item(NAME(m_io_ctrl_reg));
		save_item(NAME(m_ctrl1_th_latch));
		save_item(NAME(m_ctrl2_th_latch));
		save_item(NAME(m_ctrl1_th_state));
		save_item(NAME(m_ctrl2_th_state));
	}

	if (m_is_gamegear)
	{
		save_item(NAME(m_gg_sio));
	}

	if (m_is_sdisp)
	{
		save_item(NAME(m_store_control));
		save_item(NAME(m_current_cartridge));

		m_slots[0] = m_cartslot;
		for (int i = 1; i < 16; i++)
		{
			sprintf(str,"slot%i",i + 1);
			m_slots[i] = machine().device<sega8_cart_slot_device>(str);
		}
		for (int i = 0; i < 16; i++)
		{
			sprintf(str,"slot%i",i + 16 + 1);
			m_cards[i] = machine().device<sega8_card_slot_device>(str);
		}
	}

	// a bunch of SG1000 carts (compatible with SG1000 Mark III) can access directly system RAM... let's install here the necessary handlers
	// TODO: are BASIC and Music actually compatible with Mark III??
	// TODO: handle device slot switching for when running on a SMS.
	if (m_cartslot->get_type() == SEGA8_BASIC_L3 || m_cartslot->get_type() == SEGA8_MUSIC_EDITOR
			|| m_cartslot->get_type() == SEGA8_DAHJEE_TYPEA || m_cartslot->get_type() == SEGA8_DAHJEE_TYPEB)
	{
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xc000, 0xffff, 0, 0, read8_delegate(FUNC(sega8_cart_slot_device::read_ram),(sega8_cart_slot_device*)m_cartslot));
		m_maincpu->space(AS_PROGRAM).install_write_handler(0xc000, 0xffff, 0, 0, write8_delegate(FUNC(sega8_cart_slot_device::write_ram),(sega8_cart_slot_device*)m_cartslot));
	}
}

MACHINE_RESET_MEMBER(sms_state,sms)
{
	if (m_has_fm)
		m_fm_detect = 0x01;

	if (!m_is_mark_iii)
	{
		m_io_ctrl_reg = 0xff;
		m_bios_port = 0;
		m_ctrl1_th_latch = 0;
		m_ctrl2_th_latch = 0;
		m_ctrl1_th_state = 1;
		m_ctrl2_th_state = 1;
		// TODO: change to work also with other device slots and handle SMS slot switching.
		m_lphaser_x_offs = (m_cartslot->m_cart) ? m_cartslot->m_cart->get_lphaser_xoffs() : 44;
	}

	if (m_is_gamegear)
	{
		if (m_cartslot->m_cart && m_cartslot->m_cart->get_sms_mode())
			m_vdp->set_sega315_5124_compatibility_mode(true);

		/* Initialize SIO stuff for GG */
		m_gg_sio[0] = 0x7f;
		m_gg_sio[1] = 0xff;
		m_gg_sio[2] = 0x00;
		m_gg_sio[3] = 0xff;
		m_gg_sio[4] = 0x00;
	}

	if (m_is_sdisp)
	{
		m_store_control = 0;
		m_current_cartridge = 0;
	}

	setup_bios();
	setup_rom();
}

READ8_MEMBER(smssdisp_state::sms_store_cart_select_r)
{
	return 0xff;
}


WRITE8_MEMBER(smssdisp_state::sms_store_cart_select_w)
{
	UINT8 slot = data >> 4;
	UINT8 slottype = data & 0x08;

	logerror("switching in part of %s slot #%d\n", slottype ? "card" : "cartridge", slot );
	/* cartridge? slot #0 */
	//if (slottype == 0)
	//  m_current_cartridge = slot;

	setup_rom();
}


READ8_MEMBER(smssdisp_state::sms_store_select1)
{
	return 0xff;
}


READ8_MEMBER(smssdisp_state::sms_store_select2)
{
	return 0xff;
}


READ8_MEMBER(smssdisp_state::sms_store_control_r)
{
	return m_store_control;
}


WRITE8_MEMBER(smssdisp_state::sms_store_control_w)
{
	logerror("0x%04X: sms_store_control write 0x%02X\n", space.device().safe_pc(), data);
	if (data & 0x02)
	{
		m_maincpu->resume(SUSPEND_REASON_HALT);
	}
	else
	{
		/* Pull reset line of CPU #0 low */
		m_maincpu->reset();
		m_maincpu->suspend(SUSPEND_REASON_HALT, 1);
	}
	m_store_control = data;
}

WRITE_LINE_MEMBER(smssdisp_state::sms_store_int_callback)
{
	if ( m_store_control & 0x01 )
	{
		m_control_cpu->set_input_line( 0, state );
	}
	else
	{
		m_maincpu->set_input_line( 0, state );
	}
}

DRIVER_INIT_MEMBER(sms_state,sg1000m3)
{
	m_is_region_japan = 1;
	m_is_mark_iii = 1;
	m_has_fm = 1;
}


DRIVER_INIT_MEMBER(sms_state,sms1)
{
	m_has_bios_full = 1;
}


DRIVER_INIT_MEMBER(sms_state,smsj)
{
	m_is_region_japan = 1;
	m_has_bios_2000 = 1;
	m_has_fm = 1;
}


DRIVER_INIT_MEMBER(sms_state,sms2kr)
{
	m_is_region_japan = 1;
	m_has_bios_full = 1;
}


DRIVER_INIT_MEMBER(smssdisp_state,smssdisp)
{
	m_is_sdisp = 1;
}


DRIVER_INIT_MEMBER(sms_state,gamegear)
{
	m_is_gamegear = 1;
	m_has_bios_0400 = 1;
}


DRIVER_INIT_MEMBER(sms_state,gamegeaj)
{
	m_is_region_japan = 1;
	m_is_gamegear = 1;
	m_has_bios_0400 = 1;
}


VIDEO_START_MEMBER(sms_state,sms1)
{
	m_left_lcd = machine().device("left_lcd");
	m_right_lcd = machine().device("right_lcd");

	m_main_scr->register_screen_bitmap(m_prevleft_bitmap);
	m_main_scr->register_screen_bitmap(m_prevright_bitmap);
	save_item(NAME(m_prevleft_bitmap));
	save_item(NAME(m_prevright_bitmap));
	save_item(NAME(m_sscope_state));
	save_item(NAME(m_frame_sscope_state));

	// Allow sscope screens to have crosshair, useful for the game missil3d
	crosshair_set_screen(machine(), 0, CROSSHAIR_SCREEN_ALL);
}


VIDEO_RESET_MEMBER(sms_state,sms1)
{
	if (m_port_scope->read())
	{
		UINT8 sscope_binocular_hack = m_port_scope_binocular->read();

		if (sscope_binocular_hack & 0x01)
			m_prevleft_bitmap.fill(rgb_t::black);
		if (sscope_binocular_hack & 0x02)
			m_prevright_bitmap.fill(rgb_t::black);
	}

	m_sscope_state = 0;
	m_frame_sscope_state = 0;
}


READ32_MEMBER(sms_state::sms_pixel_color)
{
	bitmap_rgb32 &vdp_bitmap = m_vdp->get_bitmap();
	int beam_x = m_main_scr->hpos();
	int beam_y = m_main_scr->vpos();

	return vdp_bitmap.pix32(beam_y, beam_x);
}


void sms_state::screen_vblank_sms1(screen_device &screen, bool state)
{
	// on falling edge
	if (!state)
	{
		// Most of the 3-D games usually set Sega Scope's state for next frame
		// soon after the active area of current frame was drawn, but before
		// it is displayed by the screen update function. That function needs to
		// check the state used at the time of frame drawing, to display it on
		// the correct side. So here, when the frame is about to be drawn, the
		// Sega Scope's state is stored, to be checked by that function.
		m_frame_sscope_state = m_sscope_state;
	}
}


UINT32 sms_state::screen_update_sms1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT8 sscope = 0;
	UINT8 sscope_binocular_hack;
	UINT8 occluded_view = 0;

	if (&screen != m_main_scr)
	{
		sscope = m_port_scope->read();
		if (!sscope)
		{
			// without SegaScope, both LCDs for glasses go black
			occluded_view = 1;
		}
		else if (&screen == m_left_lcd)
		{
			// with SegaScope, state 0 = left screen OFF, right screen ON
			if (!(m_frame_sscope_state & 0x01))
				occluded_view = 1;
		}
		else // it's right LCD
		{
			// with SegaScope, state 1 = left screen ON, right screen OFF
			if (m_frame_sscope_state & 0x01)
				occluded_view = 1;
		}
	}

	if (!occluded_view)
	{
		m_vdp->screen_update(screen, bitmap, cliprect);

		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// save a copy of current bitmap for the binocular hack
		if (sscope)
		{
			sscope_binocular_hack = m_port_scope_binocular->read();

			if (&screen == m_left_lcd)
			{
				if (sscope_binocular_hack & 0x01)
					copybitmap(m_prevleft_bitmap, bitmap, 0, 0, 0, 0, cliprect);
			}
			else // it's right LCD
			{
				if (sscope_binocular_hack & 0x02)
					copybitmap(m_prevright_bitmap, bitmap, 0, 0, 0, 0, cliprect);
			}
		}
	}
	else
	{
		// HACK: fake 3D->2D handling (if enabled, it repeats each frame twice on the selected lens)
		// use the copied bitmap for the binocular hack
		if (sscope)
		{
			sscope_binocular_hack = m_port_scope_binocular->read();

			if (&screen == m_left_lcd)
			{
				if (sscope_binocular_hack & 0x01)
				{
					copybitmap(bitmap, m_prevleft_bitmap, 0, 0, 0, 0, cliprect);
					return 0;
				}
			}
			else // it's right LCD
			{
				if (sscope_binocular_hack & 0x02)
				{
					copybitmap(bitmap, m_prevright_bitmap, 0, 0, 0, 0, cliprect);
					return 0;
				}
			}
		}
		bitmap.fill(rgb_t::black, cliprect);
	}

	return 0;
}

UINT32 sms_state::screen_update_sms(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_vdp->screen_update(screen, bitmap, cliprect);
	return 0;
}

VIDEO_START_MEMBER(sms_state,gamegear)
{
	m_main_scr->register_screen_bitmap(m_prev_bitmap);
	save_item(NAME(m_prev_bitmap));
}

UINT32 sms_state::screen_update_gamegear(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x, y;
	bitmap_rgb32 &vdp_bitmap = m_vdp->get_bitmap();
	static bool prev_bitmap_copied = false;

	if (!m_port_persist->read())
	{
		copybitmap(bitmap, vdp_bitmap, 0, 0, 0, 0, cliprect);
		if (prev_bitmap_copied)
		{
			m_prev_bitmap.fill(rgb_t::black);
			prev_bitmap_copied = false;
		}
	}
	else if (!prev_bitmap_copied)
	{
		copybitmap(bitmap, vdp_bitmap, 0, 0, 0, 0, cliprect);
		copybitmap(m_prev_bitmap, vdp_bitmap, 0, 0, 0, 0, cliprect);
		prev_bitmap_copied = true;
	}
	else
	{
		// HACK: fake LCD persistence effect
		// (it would be better to generalize this in the core, to be used for all LCD systems)
		for (y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			UINT32 *linedst = &bitmap.pix32(y);
			UINT32 *line0 = &vdp_bitmap.pix32(y);
			UINT32 *line1 = &m_prev_bitmap.pix32(y);
			for (x = cliprect.min_x; x <= cliprect.max_x; x++)
			{
				UINT32 color0 = line0[x];
				UINT32 color1 = line1[x];
				UINT16 r0 = (color0 >> 16) & 0x000000ff;
				UINT16 g0 = (color0 >>  8) & 0x000000ff;
				UINT16 b0 = (color0 >>  0) & 0x000000ff;
				UINT16 r1 = (color1 >> 16) & 0x000000ff;
				UINT16 g1 = (color1 >>  8) & 0x000000ff;
				UINT16 b1 = (color1 >>  0) & 0x000000ff;
				UINT8 r = (UINT8)((r0 + r1) >> 1);
				UINT8 g = (UINT8)((g0 + g1) >> 1);
				UINT8 b = (UINT8)((b0 + b1) >> 1);
				linedst[x] = (r << 16) | (g << 8) | b;
			}
		}
		copybitmap(m_prev_bitmap, vdp_bitmap, 0, 0, 0, 0, cliprect);
	}
	return 0;
}

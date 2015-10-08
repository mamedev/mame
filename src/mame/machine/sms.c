// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Charles MacDonald,Mathis Rosenhauer,Brad Oliver,Michael Luong,Fabio Priuli,Enik Land
#include "emu.h"
#include "crsshair.h"
#include "video/315_5124.h"
#include "sound/sn76496.h"
#include "sound/2413intf.h"
#include "includes/sms.h"

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#define ENABLE_NONE      0x00
#define ENABLE_EXPANSION 0x01
#define ENABLE_CARD      0x02
#define ENABLE_CART      0x04
#define ENABLE_BIOS      0x08
#define ENABLE_EXT_RAM   0x10


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


WRITE_LINE_MEMBER(sms_state::gg_ext_th_input)
{
	if (!(m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode()))
		return;

	// The EXT port act as the controller port 2 on SMS compatibility mode.
	sms_ctrl2_th_input(state);
}


void sms_state::sms_get_inputs()
{
	bool port_dd_has_th_input = true;

	UINT8 data1 = 0xff;
	UINT8 data2 = 0xff;

	m_port_dc_reg = 0xff;
	m_port_dd_reg = 0xff;

	// The bit order of the emulated controller port tries to follow its
	// physical pins numbering. For register bits whose order differs,
	// it's necessary move the equivalent controller bits to match.

	if (m_is_gamegear)
	{
		// Assume the Japanese Game Gear behaves as the
		// Japanese Master System, regarding TH input.
		if (m_is_gg_region_japan)
			port_dd_has_th_input = false;

		// For Game Gear, this function is used only if SMS mode is
		// enabled, else only register $dc receives input data, through
		// direct read of the m_port_gg_dc I/O port.

		data1 = m_port_gg_dc->read();
		m_port_dc_reg &= ~0x03f | data1;

		data2 = m_port_gg_ext->port_r();
	}
	else
	{
		// Sega Mark III does not have TH lines connected.
		// The Japanese Master System does not set port $dd with TH input.
		if (m_is_mark_iii || m_is_smsj)
			port_dd_has_th_input = false;

		data1 = m_port_ctrl1->port_r();
		m_port_dc_reg &= ~0x0f | data1; // Up, Down, Left, Right
		m_port_dc_reg &= ~0x10 | (data1 >> 1); // TL (Button 1)
		m_port_dc_reg &= ~0x20 | (data1 >> 2); // TR (Button 2)

		data2 = m_port_ctrl2->port_r();
	}

	m_port_dc_reg &= ~0xc0 | (data2 << 6); // Up, Down
	m_port_dd_reg &= ~0x03 | (data2 >> 2); // Left, Right
	m_port_dd_reg &= ~0x04 | (data2 >> 3); // TL (Button 1)
	m_port_dd_reg &= ~0x08 | (data2 >> 4); // TR (Button 2)

	if (port_dd_has_th_input)
	{
		m_port_dd_reg &= ~0x40 | data1; // TH ctrl1
		m_port_dd_reg &= ~0x80 | (data2 << 1); // TH ctrl2
	}
}


WRITE8_MEMBER(sms_state::sms_io_control_w)
{
	bool latch_hcount = false;
	UINT8 ctrl1_port_data = 0xff;
	UINT8 ctrl2_port_data = 0xff;

	if (m_is_gamegear && !(m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode()))
	{
		m_io_ctrl_reg = data;
		return;
	}

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
		else
			m_port_gg_ext->port_w(ctrl2_port_data);
	}
	// check if TH is set to input (1).
	if (data & 0x08)
	{
		if (!m_is_gamegear)
			ctrl2_port_data &= ~0x40 | m_port_ctrl2->port_r();
		else
			ctrl2_port_data &= ~0x40 | m_port_gg_ext->port_r();

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
	bool pause_pressed = false;

	if (!m_is_mark_iii)
	{
		// clear TH latch of the controller ports
		m_ctrl1_th_latch = 0;
		m_ctrl2_th_latch = 0;
	}

	if (m_is_gamegear)
	{
		if (!(m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode()))
			return;

		if (!(m_port_start->read() & 0x80))
			pause_pressed = true;
	}
	else
	{
		if (!(m_port_pause->read() & 0x80))
			pause_pressed = true;
	}

	if (pause_pressed)
	{
		if (!m_paused)
			m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);

		m_paused = 1;
	}
	else
		m_paused = 0;
}


READ8_MEMBER(sms_state::sms_input_port_dc_r)
{
	if (m_is_mark_iii)
	{
		sms_get_inputs();
		return m_port_dc_reg;
	}

	if (m_is_gamegear)
	{
		// If SMS mode is disabled, just return the data read from the
		// input port. Its mapped port bits match the bits of register $dc.
		if (!(m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode()))
			return m_port_gg_dc->read();
	}
	else
	{
		// Return if the I/O chip is disabled (1). This check isn't performed
		// for the Game Gear because has no effect on it (even in SMS mode?).
		if (m_mem_ctrl_reg & IO_CHIP)
			return 0xff;
	}

	sms_get_inputs();

	// Check if TR of controller port 1 is set to output (0)
	if (!(m_io_ctrl_reg & 0x01))
	{
		// Read TR state set through IO control port
		m_port_dc_reg &= ~0x20 | ((m_io_ctrl_reg & 0x10) << 1);
	}

	return m_port_dc_reg;
}


READ8_MEMBER(sms_state::sms_input_port_dd_r)
{
	if (m_is_mark_iii)
	{
		sms_get_inputs();
		return m_port_dd_reg;
	}

	if (m_is_gamegear)
	{
		if (!(m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode()))
			return 0xff;
	}
	else
	{
		// Return if the I/O chip is disabled (1). This check isn't performed
		// for the Game Gear because has no effect on it (even in SMS mode?).
		if (m_mem_ctrl_reg & IO_CHIP)
			return 0xff;
	}

	sms_get_inputs();

	// Check if TR of controller port 2 is set to output (0)
	if (!(m_io_ctrl_reg & 0x04))
	{
		// Read TR state set through IO control port
		m_port_dd_reg &= ~0x08 | ((m_io_ctrl_reg & 0x40) >> 3);
	}

	if (m_is_smsj || (m_is_gamegear && m_is_gg_region_japan))
	{
		// For Japanese Master System, set upper 4 bits with TH/TR
		// direction bits of IO control register, according to Enri's
		// docs ( http://www43.tok2.com/home/cmpslv/Sms/EnrSms.htm ).
		// This makes the console incapable of using the Light Phaser.
		// Assume the same for a Japanese Game Gear.
		m_port_dd_reg &= ~0x10 | ((m_io_ctrl_reg & 0x01) << 4);
		m_port_dd_reg &= ~0x20 | ((m_io_ctrl_reg & 0x04) << 3);
		m_port_dd_reg &= ~0x40 | ((m_io_ctrl_reg & 0x02) << 5);
		m_port_dd_reg &= ~0x80 | ((m_io_ctrl_reg & 0x08) << 4);
		return m_port_dd_reg;
	}

	// Reset Button
	if ( m_port_reset )
	{
		m_port_dd_reg &= ~0x10 | (m_port_reset->read() & 0x01) << 4;
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


WRITE8_MEMBER(sms_state::sms_audio_control_w)
{
	if (m_has_fm)
	{
		if (m_is_smsj)
			m_audio_control = data & 0x03;
		else
			m_audio_control = data & 0x01;
	}
}


READ8_MEMBER(sms_state::sms_audio_control_r)
{
	if (m_has_fm)
		// The register reference on SMSPower states that just the
		// first bit written is returned on reads (even for smsj?).
		return m_audio_control & 0x01;
	else
		return sms_input_port_dc_r(space, offset);
}


WRITE8_MEMBER(sms_state::sms_ym2413_register_port_w)
{
	if (m_has_fm && (m_audio_control & 0x01))
		m_ym->write(space, 0, data & 0x3f);
}


WRITE8_MEMBER(sms_state::sms_ym2413_data_port_w)
{
	if (m_has_fm && (m_audio_control & 0x01))
	{
		//logerror("data_port_w %x %x\n", offset, data);
		m_ym->write(space, 1, data);
	}
}


WRITE8_MEMBER(sms_state::sms_psg_w)
{
	// On Japanese SMS, if FM is enabled, PSG must be explicitly enabled too.
	if (m_is_smsj && (m_audio_control & 0x01) && !(m_audio_control & 0x02))
		return;

	m_psg_sms->write(space, offset, data, mem_mask);
}


WRITE8_MEMBER(sms_state::gg_psg_w)
{
	m_psg_gg->write(space, offset, data, mem_mask);
}


WRITE8_MEMBER(sms_state::gg_psg_stereo_w)
{
	if (m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode())
		return;

	m_psg_gg->stereo_w(space, offset, data, mem_mask);
}


READ8_MEMBER(sms_state::gg_input_port_00_r)
{
	if (m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode())
		return 0xff;
	else
	{
		UINT8 data = (m_is_gg_region_japan ? 0x00 : 0x40) | (m_port_start->read() & 0x80);

		//logerror("port $00 read, val: %02x, pc: %04x\n", data, activecpu_get_pc());
		return data;
	}
}


READ8_MEMBER(sms_state::sms_sscope_r)
{
	int sscope = m_port_scope->read();

	if ( sscope )
	{
		// Scope is attached
		return m_sscope_state;
	}

	return read_ram(space, 0x3ff8 + offset);
}


WRITE8_MEMBER(sms_state::sms_sscope_w)
{
	write_ram(space, 0x3ff8 + offset, data);

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


READ8_MEMBER(sms_state::read_ram)
{
	if (m_mem_device_enabled & ENABLE_EXT_RAM)
	{
		UINT8 data = 0xff;

		if (m_mem_device_enabled & ENABLE_CART)
			data &= m_cartslot->read_ram(space, offset);
		if (m_mem_device_enabled & ENABLE_CARD)
			data &= m_cardslot->read_ram(space, offset);
		if (m_mem_device_enabled & ENABLE_EXPANSION)
			data &= m_expslot->read_ram(space, offset);

		return data;
	}
	else
	{
		return m_mainram[offset & 0x1fff];
	}
}


WRITE8_MEMBER(sms_state::write_ram)
{
	if (m_mem_device_enabled & ENABLE_EXT_RAM)
	{
		if (m_mem_device_enabled & ENABLE_CART)
			m_cartslot->write_ram(space, offset, data);
		if (m_mem_device_enabled & ENABLE_CARD)
			m_cardslot->write_ram(space, offset, data);
		if (m_mem_device_enabled & ENABLE_EXPANSION)
			m_expslot->write_ram(space, offset, data);
	}
	else
	{
		m_mainram[offset & 0x1fff] = data;
	}
}


READ8_MEMBER(sms_state::sms_mapper_r)
{
	return read_ram(space, 0x3ffc + offset);
}


WRITE8_MEMBER(sms_state::sms_mapper_w)
{
	m_mapper[offset] = data;
	write_ram(space, 0x3ffc + offset, data);

	switch (offset)
	{
		case 0: // Control RAM/ROM
			if (!(data & 0x08) && (m_mem_device_enabled & ENABLE_BIOS))    // BIOS ROM
			{
				if (!m_has_bios_0400 && !m_has_bios_2000)
				{
					m_bios_page[2] = m_mapper[3];
				}
			}
			if (m_mem_device_enabled & ENABLE_CART)    // CART ROM/RAM
			{
				m_cartslot->write_mapper(space, offset, data);
			}
			if (m_mem_device_enabled & ENABLE_CARD)    // CARD ROM/RAM
			{
				m_cardslot->write_mapper(space, offset, data);
			}
			if (m_mem_device_enabled & ENABLE_EXPANSION)    // expansion slot
			{
				m_expslot->write_mapper(space, offset, data);
			}
			break;

		case 1: // Select 16k ROM bank for 0400-3fff
		case 2: // Select 16k ROM bank for 4000-7fff
		case 3: // Select 16k ROM bank for 8000-bfff
			if (m_mem_device_enabled & ENABLE_BIOS)
			{
				if (!m_has_bios_0400 && !m_has_bios_2000)
				{
					m_bios_page[offset - 1] = data & (m_bios_page_count - 1);
				}
			}
			if (m_mem_device_enabled & ENABLE_CART)
			{
				m_cartslot->write_mapper(space, offset, data);
			}
			if (m_mem_device_enabled & ENABLE_CARD)
			{
				m_cardslot->write_mapper(space, offset, data);
			}
			if (m_mem_device_enabled & ENABLE_EXPANSION)
			{
				m_expslot->write_mapper(space, offset, data);
			}
			break;
	}
}


UINT8 sms_state::read_bus(address_space &space, unsigned int page, UINT16 base_addr, UINT16 offset)
{
	if (m_is_gamegear)
	{
		// Game Gear BIOS behavior, according to Charles MacDonald: "it uses the first
		// 1K. The rest of the space is mapped to the cartridge, regardless of the slot
		// that's selected. This allows the BIOS to check for the 'TMR SEGA' string at
		// 1FF0/3FF0/7FF0, but it can't do a checksum since the first 1K of ROM is
		// unavailable. Anyway, once the BIOS decides to run the game, it disables
		// itself, and the first 1K is assigned to the cartridge ROM like normal."

		if ((m_mem_device_enabled & ENABLE_BIOS) && page == 3)
			return m_BIOS[(m_bios_page[page] * 0x4000) + (offset & 0x3fff)];
		if (m_mem_device_enabled & ENABLE_CART)
			return m_cartslot->read_cart(space, base_addr + offset);
	}
	else if (m_mem_device_enabled != ENABLE_NONE)
	{
		UINT8 data = 0xff;

		// SMS2 behavior described by Charles MacDonald's SMS notes:
		// "If the BIOS is enabled at the same time the cartridge slot is,
		// the data from both sources are logically ANDed together when read."

		if (m_mem_device_enabled & ENABLE_BIOS)
			data &= m_BIOS[(m_bios_page[page] * 0x4000) + (offset & 0x3fff)];
		if (m_mem_device_enabled & ENABLE_CART)
			data &= m_cartslot->read_cart(space, base_addr + offset);
		if (m_mem_device_enabled & ENABLE_CARD)
			data &= m_cardslot->read_cart(space, base_addr + offset);
		if (m_mem_device_enabled & ENABLE_EXPANSION)
			data &= m_expslot->read(space, base_addr + offset);

		return data;
	}
	return m_region_maincpu->base()[offset];
}


READ8_MEMBER(sms_state::read_0000)
{
	if (offset < 0x400)
	{
		return read_bus(space, 3, 0x0000, offset);
	}
	else
	{
		return read_bus(space, 0, 0x0000, offset);
	}
}

READ8_MEMBER(sms_state::read_4000)
{
	return read_bus(space, 1, 0x4000, offset);
}

READ8_MEMBER(sms_state::read_8000)
{
	return read_bus(space, 2, 0x8000, offset);
}

WRITE8_MEMBER(sms_state::write_cart)
{
	if (m_mem_device_enabled & ENABLE_CART)
		m_cartslot->write_cart(space, offset, data);
	if (m_mem_device_enabled & ENABLE_CARD)
		m_cardslot->write_cart(space, offset, data);
	if (m_mem_device_enabled & ENABLE_EXPANSION)
		m_expslot->write(space, offset, data);
}


READ8_MEMBER(smssdisp_state::store_cart_peek)
{
	if (m_mem_device_enabled != ENABLE_NONE)
	{
		UINT8 data = 0xff;

		if (m_mem_device_enabled & ENABLE_CART)
			data &= m_cartslot->read_cart(space, 0x6000 + (offset & 0x1fff));

		return data;
	}
	else
	{
		return m_region_maincpu->base()[offset];
	}
}

WRITE8_MEMBER(sms_state::sms_mem_control_w)
{
	m_mem_ctrl_reg = data;

	logerror("memory control reg write %02x, pc: %04x\n", data, space.device().safe_pc());

	setup_media_slots();
}


WRITE8_MEMBER(sms_state::gg_sio_w)
{
	if (m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode())
		return;

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
	if (m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode())
		return 0xff;

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


void sms_state::setup_enabled_slots()
{
	m_mem_device_enabled = ENABLE_NONE;

	if (m_is_mark_iii)
	{
		// Mark III uses the card slot by default, but has hardware method
		// (/CART pin) that prioritizes the cartridge slot if it has media
		// inserted. Japanese 3-D cartridges do not connect the /CART pin,
		// to not disable the card adaptor used by the 3-D glasses.
		if (m_cartslot && m_cartslot->exists())
		{
			m_mem_device_enabled |= ENABLE_CART;
			logerror("Cartridge ROM/RAM enabled.\n");
		}
		else if (m_cardslot && m_cardslot->exists())
		{
			m_mem_device_enabled |= ENABLE_CARD;
			logerror("Card ROM port enabled.\n");
		}
		return;
	}

	if (!(m_mem_ctrl_reg & IO_EXPANSION) && m_expslot && m_expslot->m_device)
	{
		m_mem_device_enabled |= ENABLE_EXPANSION;
		logerror("Expansion port enabled.\n");
	}

	if (!(m_mem_ctrl_reg & IO_CARD) && m_cardslot && m_cardslot->exists())
	{
		m_mem_device_enabled |= ENABLE_CARD;
		logerror("Card ROM port enabled.\n");
	}

	if ((m_is_gamegear || !(m_mem_ctrl_reg & IO_CARTRIDGE)) && m_cartslot && m_cartslot->exists())
	{
		m_mem_device_enabled |= ENABLE_CART;
		logerror("Cartridge ROM/RAM enabled.\n");
	}

	if (!(m_mem_ctrl_reg & IO_BIOS_ROM) && m_BIOS)
	{
		m_mem_device_enabled |= ENABLE_BIOS;
		logerror("BIOS enabled.\n");
	}
}


void sms_state::setup_media_slots()
{
	setup_enabled_slots();

	if (m_mem_device_enabled == ENABLE_NONE)
		return;

	// Setup cartridge RAM
	if (!m_is_mark_iii && (m_mem_ctrl_reg & IO_WORK_RAM)) // Work RAM disabled (1).
	{
		m_mem_device_enabled |= ENABLE_EXT_RAM;
	}
	else if (m_has_jpn_sms_cart_slot && (m_mem_device_enabled & ENABLE_CART))
	{
		// a bunch of SG1000 carts (compatible with SG1000 Mark III) use their own RAM...
		// TODO: are BASIC and Music actually compatible with Mark III??
		if (m_cartslot->get_type() == SEGA8_BASIC_L3 ||
			m_cartslot->get_type() == SEGA8_MUSIC_EDITOR ||
			m_cartslot->get_type() == SEGA8_DAHJEE_TYPEA ||
			m_cartslot->get_type() == SEGA8_DAHJEE_TYPEB)
		{
			m_mem_device_enabled |= ENABLE_EXT_RAM;
		}
	}

	// Set offset for Light Phaser
	if (!m_is_mark_iii)
	{
		m_lphaser_x_offs = -1; // same value returned for ROMs without custom offset.

		if (m_mem_device_enabled & ENABLE_CART)
			m_lphaser_x_offs = m_cartslot->m_cart->get_lphaser_xoffs();
		else if (m_mem_device_enabled & ENABLE_CARD)
			m_lphaser_x_offs = m_cardslot->m_cart->get_lphaser_xoffs();
		else if (m_mem_device_enabled & ENABLE_EXPANSION)
			m_lphaser_x_offs = m_expslot->m_device->get_lphaser_xoffs();

		if (m_lphaser_x_offs == -1)
			m_lphaser_x_offs = 36;
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
		m_mem_ctrl_reg = (IO_EXPANSION | IO_CARTRIDGE | IO_CARD);
		if (!m_BIOS)
		{
			m_mem_ctrl_reg &= ~(IO_CARTRIDGE);
			m_mem_ctrl_reg |= IO_BIOS_ROM;
		}
	}
}

MACHINE_START_MEMBER(sms_state,sms)
{
	char str[7];

	m_cartslot = machine().device<sega8_cart_slot_device>("slot");
	m_cardslot = machine().device<sega8_card_slot_device>("mycard");
	m_expslot = machine().device<sms_expansion_slot_device>("exp");
	m_space = &m_maincpu->space(AS_PROGRAM);

	if (m_mainram == NULL)
	{
		m_mainram = auto_alloc_array_clear(machine(), UINT8, 0x2000);
		save_pointer(NAME(m_mainram), 0x2000);

		// alibaba and blockhol are ports of games for the MSX system. The
		// MSX bios usually initializes callback "vectors" at the top of RAM.
		// The code in alibaba does not do this so the IRQ vector only contains
		// the "call $4010" without a following RET statement. That is basically
		// a bug in the program code. The only way this cartridge could have run
		// successfully on a real unit is if the RAM would be initialized with
		// a F0 pattern on power up; F0 = RET P.
		// This initialization breaks some Game Gear games though (e.g.
		// tempojr), suggesting that not all systems had the same initialization.
		// This also breaks some homebrew softwares (e.g. Nine Pixels).
		// For the moment we apply this to systems that have the Japanese SMS
		// cartridge slot.
		if (m_has_jpn_sms_cart_slot)
		{
			memset(m_mainram, 0xf0, 0x2000);
		}
	}

	save_item(NAME(m_paused));
	save_item(NAME(m_mapper));
	save_item(NAME(m_port_dc_reg));
	save_item(NAME(m_port_dd_reg));
	save_item(NAME(m_mem_device_enabled));

	if (m_has_fm)
	{
		save_item(NAME(m_audio_control));
	}

	if (!m_is_mark_iii)
	{
		save_item(NAME(m_mem_ctrl_reg));
		save_item(NAME(m_bios_page));
		save_item(NAME(m_io_ctrl_reg));
		save_item(NAME(m_ctrl1_th_latch));
		save_item(NAME(m_ctrl2_th_latch));
		save_item(NAME(m_ctrl1_th_state));
		save_item(NAME(m_ctrl2_th_state));
		save_item(NAME(m_lphaser_x_offs));
	}

	if (m_is_gamegear)
	{
		save_item(NAME(m_gg_sio));
	}

	if (m_is_sdisp)
	{
		machine().save().register_postload(save_prepost_delegate(FUNC(sms_state::store_post_load), this));

		save_item(NAME(m_store_control));
		save_item(NAME(m_store_cart_selection_data));

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

	if (m_cartslot)
		m_cartslot->save_ram();
}

MACHINE_RESET_MEMBER(sms_state,sms)
{
	if (m_has_fm)
		m_audio_control = 0x00;

	if (!m_is_mark_iii)
	{
		m_io_ctrl_reg = 0xff;
		m_ctrl1_th_latch = 0;
		m_ctrl2_th_latch = 0;
		m_ctrl1_th_state = 1;
		m_ctrl2_th_state = 1;
	}

	if (m_is_gamegear)
	{
		if (m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode())
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
		m_store_cart_selection_data = 0;
		store_select_cart(m_store_cart_selection_data);
	}

	setup_bios();
	setup_media_slots();
}

READ8_MEMBER(smssdisp_state::sms_store_cart_select_r)
{
	return m_store_cart_selection_data;
}


WRITE8_MEMBER(smssdisp_state::sms_store_cart_select_w)
{
	store_select_cart(data);
	m_store_cart_selection_data = data;
	setup_media_slots();
}


void sms_state::store_post_load()
{
	store_select_cart(m_store_cart_selection_data);
}


// There are two known models of the Store Display Unit:
//
// - the one with 16 cart slots and 3 card slots;
// - the one with 16 cart slots and 16 card slots.
//
// On front panel of both models there are only 16 game switches,
// that seems to change the active cart/card slot pair or, for the 4th
// game switch onward of the 16-3 model, the active cart slot only.

void sms_state::store_select_cart(UINT8 data)
{
	UINT8 slot = data >> 4;
	UINT8 slottype = data & 0x08;

	// The SMS Store Display Unit only uses the logical cartridge slot to
	// map the active cartridge or card slot, of its multiple ones.
	if (slottype == 0)
		m_cartslot = m_slots[slot];
	else
		m_cartslot = m_cards[slot];

	logerror("switching in part of %s slot #%d\n", slottype ? "card" : "cartridge", slot);
}

WRITE8_MEMBER(smssdisp_state::sms_store_control_w)
{
	int led_number = data >> 4;
	int led_column = led_number / 4;
	int led_line = led_number % 4;
	int game_number = (4 * led_column) + (3 - led_line);

	logerror("0x%04X: sms_store_control write 0x%02X\n", space.device().safe_pc(), data);
	logerror("sms_store_control: LED #%d activated for game #%d\n", led_number, game_number);

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
	m_is_mark_iii = 1;
	m_has_fm = 1;
	m_has_jpn_sms_cart_slot = 1;
}


DRIVER_INIT_MEMBER(sms_state,sms1)
{
	m_has_bios_full = 1;
}


DRIVER_INIT_MEMBER(sms_state,smsj)
{
	m_is_smsj = 1;
	m_has_bios_2000 = 1;
	m_has_fm = 1;
	m_has_jpn_sms_cart_slot = 1;
}


DRIVER_INIT_MEMBER(sms_state,sms1krfm)
{
	m_has_bios_2000 = 1;
	m_has_fm = 1;
	m_has_jpn_sms_cart_slot = 1;
}


DRIVER_INIT_MEMBER(sms_state,sms1kr)
{
	m_has_bios_2000 = 1;
	m_has_jpn_sms_cart_slot = 1;
}


DRIVER_INIT_MEMBER(sms_state,smskr)
{
	m_has_bios_full = 1;
	// Despite having a Japanese cartridge slot, this version is detected as Export region.
	m_has_jpn_sms_cart_slot = 1;
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
	m_is_gamegear = 1;
	m_has_bios_0400 = 1;
	m_is_gg_region_japan = 1;
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
	m_prev_bitmap_copied = false;
	m_main_scr->register_screen_bitmap(m_prev_bitmap);
	m_main_scr->register_screen_bitmap(m_gg_sms_mode_bitmap);
	m_line_buffer = auto_alloc_array(machine(), int, 160 * 4);

	save_item(NAME(m_prev_bitmap_copied));
	save_item(NAME(m_prev_bitmap));
	save_item(NAME(m_gg_sms_mode_bitmap));
	save_pointer(NAME(m_line_buffer), 160 * 4);
}

VIDEO_RESET_MEMBER(sms_state,gamegear)
{
	if (m_prev_bitmap_copied)
	{
		m_prev_bitmap.fill(rgb_t::black);
		m_prev_bitmap_copied = false;
	}
	if (m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode())
	{
		m_gg_sms_mode_bitmap.fill(rgb_t::black);
		memset(m_line_buffer, 0, 160 * 4 * sizeof(int));
	}
}


void sms_state::screen_gg_sms_mode_scaling(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap_rgb32 &vdp_bitmap = m_vdp->get_bitmap();
	const rectangle visarea = screen.visible_area();

	/* Plot positions relative to visarea minimum values */
	const int plot_min_x = cliprect.min_x - visarea.min_x;
	const int plot_max_x = MIN(cliprect.max_x - visarea.min_x, 159); // avoid m_line_buffer overflow.
	const int plot_min_y = cliprect.min_y - visarea.min_y;
	const int plot_max_y = cliprect.max_y - visarea.min_y;

	/* For each group of 3 SMS pixels, a group of 2 GG pixels is processed.
	   In case the cliprect coordinates may map any region of the visible area,
	   take in account the remaining position, if any, of the first group. */
	const int plot_x_first_group = plot_min_x - (plot_min_x % 2);
	const int plot_y_first_group = plot_min_y - (plot_min_y % 2);

	/* Calculation of the minimum scaled value for X */
	const int visarea_xcenter = visarea.xcenter();
	const int sms_offset_min_x = ((int) (visarea_xcenter - cliprect.min_x) / 2) * 3;
	const int sms_min_x = visarea_xcenter - sms_offset_min_x;

	/* Calculation of the minimum scaled value for Y */
	const int visarea_ycenter = visarea.ycenter();
	const int sms_offset_min_y = ((int) (visarea_ycenter - cliprect.min_y) / 2) * 3;
	const int sms_min_y = visarea_ycenter - sms_offset_min_y;

	int sms_y = sms_min_y;
	int y_min_i = plot_min_y - plot_y_first_group;

	/* Auxiliary variable for vertical scaling */
	int sms_y2 = sms_y - 1;

	for (int plot_y_group = plot_y_first_group; plot_y_group <= plot_max_y; plot_y_group += 2)
	{
		const int y_max_i = MIN(1, plot_max_y - plot_y_group);

		for (int y_i = y_min_i; y_i <= y_max_i; y_i++)
		{
			/* Include additional lines that have influence over what appears on the LCD */
			const int sms_min_y2 = sms_y + y_i - 1;
			const int sms_max_y2 = sms_y + y_i + 2;

			/* Process lines, but skip those already processed before */
			for (sms_y2 = MAX(sms_min_y2, sms_y2); sms_y2 <= sms_max_y2; sms_y2++)
			{
				int *combineline_buffer =  m_line_buffer + (sms_y2 & 0x03) * 160;

				if (sms_y2 >= vdp_bitmap.cliprect().min_y && sms_y2 <= vdp_bitmap.cliprect().max_y)
				{
					UINT32 *vdp_buffer =  &vdp_bitmap.pix32(sms_y2);

					int sms_x = sms_min_x;
					int x_min_i = plot_min_x - plot_x_first_group;

					/* Do horizontal scaling */
					for (int plot_x_group = plot_x_first_group; plot_x_group <= plot_max_x; plot_x_group += 2)
					{
						const int x_max_i = MIN(1, plot_max_x - plot_x_group);

						for (int x_i = x_min_i; x_i <= x_max_i; x_i++)
						{
							int combined = 0;

							if (sms_x + x_i >= vdp_bitmap.cliprect().min_x && sms_x + x_i + 1 <= vdp_bitmap.cliprect().max_x)
							{
								switch (x_i)
								{
								case 0:
									/* Take red and green from first pixel, and blue from second pixel */
									combined = (vdp_buffer[sms_x] & 0x00ffff00) | (vdp_buffer[sms_x + 1] & 0x000000ff);
									break;
								case 1:
									/* Take red from second pixel, and green and blue from third pixel */
									combined = (vdp_buffer[sms_x + 1] & 0x00ff0000) | (vdp_buffer[sms_x + 2] & 0x0000ffff);
									break;
								}
							}

							combineline_buffer[plot_x_group + x_i] = combined;
						}

						sms_x += 3;
						x_min_i = 0;
					}
				}
				else
				{
					memset(combineline_buffer, 0, 160 * sizeof(int));
				}
			}

			/* Do vertical scaling for a screen with 192 or 224 lines
			   Lines 0-2 and 221-223 have no effect on the output on the GG screen.
			   We will calculate the gamegear lines as follows:
			   GG_0 = 1/6 * SMS_3 + 1/3 * SMS_4 + 1/3 * SMS_5 + 1/6 * SMS_6
			   GG_1 = 1/6 * SMS_4 + 1/3 * SMS_5 + 1/3 * SMS_6 + 1/6 * SMS_7
			   GG_2 = 1/6 * SMS_6 + 1/3 * SMS_7 + 1/3 * SMS_8 + 1/6 * SMS_9
			   GG_3 = 1/6 * SMS_7 + 1/3 * SMS_8 + 1/3 * SMS_9 + 1/6 * SMS_10
			   GG_4 = 1/6 * SMS_9 + 1/3 * SMS_10 + 1/3 * SMS_11 + 1/6 * SMS_12
			   .....
			   GG_142 = 1/6 * SMS_216 + 1/3 * SMS_217 + 1/3 * SMS_218 + 1/6 * SMS_219
			   GG_143 = 1/6 * SMS_217 + 1/3 * SMS_218 + 1/3 * SMS_219 + 1/6 * SMS_220
			*/
			{
				int *line1, *line2, *line3, *line4;

				/* Setup our source lines */
				line1 = m_line_buffer + ((sms_y + y_i - 1) & 0x03) * 160;
				line2 = m_line_buffer + ((sms_y + y_i - 0) & 0x03) * 160;
				line3 = m_line_buffer + ((sms_y + y_i + 1) & 0x03) * 160;
				line4 = m_line_buffer + ((sms_y + y_i + 2) & 0x03) * 160;

				UINT32 *p_bitmap = &bitmap.pix32(visarea.min_y + plot_y_group + y_i, visarea.min_x);

				for (int plot_x = plot_min_x; plot_x <= plot_max_x; plot_x++)
				{
					rgb_t c1 = line1[plot_x];
					rgb_t c2 = line2[plot_x];
					rgb_t c3 = line3[plot_x];
					rgb_t c4 = line4[plot_x];
					p_bitmap[plot_x] =
						rgb_t(
							( c1.r() / 6 + c2.r() / 3 + c3.r() / 3 + c4.r() / 6 ),
							( c1.g() / 6 + c2.g() / 3 + c3.g() / 3 + c4.g() / 6 ),
							( c1.b() / 6 + c2.b() / 3 + c3.b() / 3 + c4.b() / 6 )
						);
				}
			}
		}

		sms_y += 3;
		y_min_i = 0;
	}
}


UINT32 sms_state::screen_update_gamegear(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap_rgb32 *source_bitmap;

	if (m_cartslot->exists() && m_cartslot->m_cart->get_sms_mode())
	{
		screen_gg_sms_mode_scaling(screen, m_gg_sms_mode_bitmap, cliprect);
		source_bitmap = &m_gg_sms_mode_bitmap;
	}
	else
	{
		source_bitmap = &m_vdp->get_bitmap();
	}

	if (!m_port_persist->read())
	{
		copybitmap(bitmap, *source_bitmap, 0, 0, 0, 0, cliprect);
		if (m_prev_bitmap_copied)
		{
			m_prev_bitmap.fill(rgb_t::black);
			m_prev_bitmap_copied = false;
		}
	}
	else if (!m_prev_bitmap_copied)
	{
		copybitmap(bitmap, *source_bitmap, 0, 0, 0, 0, cliprect);
		copybitmap(m_prev_bitmap, *source_bitmap, 0, 0, 0, 0, cliprect);
		m_prev_bitmap_copied = true;
	}
	else
	{
		// HACK: fake LCD persistence effect
		// (it would be better to generalize this in the core, to be used for all LCD systems)
		for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
		{
			UINT32 *linedst = &bitmap.pix32(y);
			UINT32 *line0 = &source_bitmap->pix32(y);
			UINT32 *line1 = &m_prev_bitmap.pix32(y);
			for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
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
		copybitmap(m_prev_bitmap, *source_bitmap, 0, 0, 0, 0, cliprect);
	}
	return 0;
}

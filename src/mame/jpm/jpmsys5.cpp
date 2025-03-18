// license:BSD-3-Clause
// copyright-holders:Philip Bennett, James Wallace, David Haywood
/***************************************************************************

    JPM System 5
    and
    JPM System 5 with Video Expansion 2 hardware

    driver by Phil Bennett

    AWP bits J.Wallace, D. Haywood

    Video System Games supported:
        * Monopoly
        * Monopoly Classic
        * Monopoly Deluxe

    Known Issues:
        * Some features used by the AWP games such as reels are not emulated.
        * Timing for reels, and other opto devices is controlled by a generated clock
        in a weird daisychain setup. We're using the later direct drive approach for now

    AWP game notes:
      The byte at 0x81 of the EVEN 68k rom appears to be some kind of
      control byte, probably region, or coin / machine type setting.
      Many sets differ only by this byte.
      Now we have BACTA protocol emulation, see if this is related.

      Many sets are probably missing sound roms, however due to the
      varying motherboard configurations (SAA vs. YM, with added UPD)
      it's hard to tell until they start doing more.

***************************************************************************/

#include "emu.h"
#include "jpmsys5.h"

#include "machine/bacta_datalogger.h"
#include "machine/clock.h"
#include "machine/input_merger.h"
#include "sound/saa1099.h"

#include "screen.h"
#include "speaker.h"

#include "jpmsys5.lh"


enum state { IDLE, START, DATA, STOP1, STOP2 };

/*************************************
 *
 *  Defines
 *
 *************************************/

enum int_levels
{
	INT_6821PIA    = 1,
	INT_TMS34061   = 2,
	INT_6840PTM    = 3,
	INT_6850ACIA   = 4,
	INT_WATCHDOG   = 5,
	INT_FLOPPYCTRL = 6,
	INT_POWERFAIL  = 7
};


/*************************************
 *
 *  Video hardware
 *
 *************************************/

void jpmsys5v_state::generate_tms34061_interrupt(int state)
{
	m_maincpu->set_input_line(INT_TMS34061, state);
}

void jpmsys5v_state::sys5_tms34061_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int func = (offset >> 19) & 3;
	int row = (offset >> 7) & 0x1ff;
	int col;

	if (func == 0 || func == 2)
		col = offset & 0xff;
	else
	{
		col = (offset << 1);

		if (~offset & 0x40000)
			row |= 0x200;
	}

	if (ACCESSING_BITS_8_15)
		m_tms34061->write(col, row, func, data >> 8);

	if (ACCESSING_BITS_0_7)
		m_tms34061->write(col | 1, row, func, data & 0xff);
}

uint16_t jpmsys5v_state::sys5_tms34061_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0;
	int func = (offset >> 19) & 3;
	int row = (offset >> 7) & 0x1ff;
	int col;

	if (func == 0 || func == 2)
		col = offset & 0xff;
	else
	{
		col = (offset << 1);

		if (~offset & 0x40000)
			row |= 0x200;
	}

	if (ACCESSING_BITS_8_15)
		data |= m_tms34061->read(col, row, func) << 8;

	if (ACCESSING_BITS_0_7)
		data |= m_tms34061->read(col | 1, row, func);

	return data;
}

void jpmsys5v_state::ramdac_w(offs_t offset, uint16_t data)
{
	if (offset == 0)
	{
		m_pal_addr = data;
		m_pal_idx = 0;
	}
	else if (offset == 1)
	{
		m_palette_val[m_pal_addr][m_pal_idx] = data;

		if (++m_pal_idx == 3)
		{
			/* Update the MAME palette */
			m_palette->set_pen_color(m_pal_addr, pal6bit(m_palette_val[m_pal_addr][0]), pal6bit(m_palette_val[m_pal_addr][1]), pal6bit(m_palette_val[m_pal_addr][2]));
			m_pal_addr++;
			m_pal_idx = 0;
		}
	}
	else
	{
		/* Colour mask? */
	}
}

uint32_t jpmsys5v_state::screen_update_jpmsys5v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_tms34061->get_display_state();

	if (m_tms34061->m_display.blanked)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	for (int y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		uint8_t const* const src = &m_tms34061->m_display.vram[(m_tms34061->m_display.dispstart & 0xffff) * 2 + 256 * y];
		uint32_t* dest = &bitmap.pix(y, cliprect.min_x);

		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			uint8_t const pen = src[(x - cliprect.min_x) >> 1];

			/* Draw two 4-bit pixels */
			*dest++ = m_palette->pen((pen >> 4) & 0xf);
			*dest++ = m_palette->pen(pen & 0xf);
		}
	}

	return 0;
}

void jpmsys5_state::sys5_draw_lamps()
{
	for (int i = 0; i < 8; i++)
	{
		m_lamps[(m_lamp_strobe << 4) | i]     = BIT(m_muxram[(m_lamp_strobe << 2) | 0], i);
		m_lamps[(m_lamp_strobe << 4) | i | 8] = BIT(m_muxram[(m_lamp_strobe << 2) | 1], i);
	}

	m_sys5leds[m_lamp_strobe] = m_muxram[(m_lamp_strobe << 2) | 2];
}

/****************************************
 *
 *  General machine functions
 *
 ****************************************/

void jpmsys5v_state::rombank_w(uint16_t data)
{
	m_rombank->set_entry(data & 0x1f);
}

uint16_t jpmsys5_state::coins_r(offs_t offset, uint16_t mem_mask)
{
	return ioport("COINS")->read() << 8;
}

uint16_t jpmsys5_state::unknown_port_r(offs_t offset, uint16_t mem_mask)
{
	return m_unknown_port.read_safe(0xffff);
}

// these are read as a dword, masked with 0x77777777 and compared to 0x76543210
uint16_t jpmsys5_state::unk_48000_r(offs_t offset, uint16_t mem_mask)
{
	logerror("%s: unk_48000_r %04x\n", machine().describe_context(), mem_mask);
	return 0x7654;
}

uint16_t jpmsys5_state::unk_48002_r(offs_t offset, uint16_t mem_mask)
{
	logerror("%s: unk_48002_r %04x\n", machine().describe_context(), mem_mask);
	return 0x3210;
}

uint16_t jpmsys5_state::unk_48006_r(offs_t offset, uint16_t mem_mask)
{
	logerror("%s: unk_48006_r %04x\n", machine().describe_context(), mem_mask);
	return 0xffff;
}

uint16_t jpmsys5_state::unk_r(offs_t offset, uint16_t mem_mask)
{
	return machine().rand();
}

uint16_t jpmsys5_state::reel_optos_r(offs_t offset, uint16_t mem_mask)
{
	logerror("%s: reel_optos_r %04x\n", machine().describe_context(), mem_mask);
	return m_optic_pattern;
}

void jpmsys5_state::reel_0123_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: reel_0123_w %04x %04x\n", machine().describe_context(), data, mem_mask);


	// only writes 0/1/2/3 to each reel?
	if (data & 0xcccc)
		popmessage("reel_0123_w upper bits set", data & 0xcccc);

	if (m_reel[0])
	{
		m_reel[0]->update(reel_interface_table[(data >> 0) & 0x03]);
		awp_draw_reel(machine(), "reel1", *m_reel[0]);
	}

	if (m_reel[1])
	{
		m_reel[1]->update(reel_interface_table[(data >> 4) & 0x03]);
		awp_draw_reel(machine(), "reel2", *m_reel[1]);
	}

	if (m_reel[2])
	{
		m_reel[2]->update(reel_interface_table[(data >> 8) & 0x03]);
		awp_draw_reel(machine(), "reel3", *m_reel[2]);
	}

	if (m_reel[3])
	{
		m_reel[3]->update(reel_interface_table[(data >> 12) & 0x03]);
		awp_draw_reel(machine(), "reel4", *m_reel[3]);
	}
}

void jpmsys5_state::reel_4567_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: reel_4567_w %04x %04x\n", machine().describe_context(), data, mem_mask);

	if (data & 0xcccc)
		popmessage("reel_4567_w upper bits set", data & 0xcccc);

	if (m_reel[4])
	{
		m_reel[4]->update(reel_interface_table[(data >> 0) & 0x03]);
		awp_draw_reel(machine(), "reel5", *m_reel[4]);
	}
	if (m_reel[5])
	{
		m_reel[5]->update(reel_interface_table[(data >> 4) & 0x03]);
		awp_draw_reel(machine(), "reel6", *m_reel[5]);
	}
#if 0
	if (m_reel[6])
	{
		m_reel[6]->update(reel_interface_table[(data >> 8) & 0x03]);
		awp_draw_reel(machine(), "reel6", *m_reel[6]);
	}
	if (m_reel[7])
	{
		m_reel[7]->update(reel_interface_table[(data >> 12) & 0x03]);
		awp_draw_reel(machine(), "reel7", *m_reel[7]);
	}
#endif
}

void jpmsys5_state::unk_48000_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: unk_48000_w %04x %04x\n", machine().describe_context(), data, mem_mask);
}

void jpmsys5_state::unk_48006_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: unk_48006_w %04x %04x\n", machine().describe_context(), data, mem_mask);
}

uint16_t jpmsys5_state::reellamps_0123_r(offs_t offset, uint16_t mem_mask)
{
	logerror("%s: reellamps_0123_r %04x\n", machine().describe_context(), mem_mask);
	return m_reellamps_0123;
}

void jpmsys5_state::reellamps_0123_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: reellamps_0123_w %04x %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_reellamps_0123);

	for (int i = 0; i < 16; i++)
		m_reellamp_out[i] = (m_reellamps_0123 >> (15 - i)) & 1;
}

uint16_t jpmsys5_state::reellamps_4567_r(offs_t offset, uint16_t mem_mask)
{
	logerror("%s: reellamps_4567_r %04x\n", machine().describe_context(), mem_mask);
	return m_reellamps_5678;
}

void jpmsys5_state::reellamps_4567_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	logerror("%s: reellamps_4567_w %04x %04x\n", machine().describe_context(), data, mem_mask);
	COMBINE_DATA(&m_reellamps_5678);

	for (int i = 0; i < 16; i++)
		m_reellamp_out[i+16] = (m_reellamps_5678 >> (15 - i)) & 1;
}


// This mux_r / mux_w area seems to be a buffer for the strobing
// are inputs actually only read into this area during strobing, just as the lamps values are only pulled
// to go to the lamps during strobing?

void jpmsys5_state::mux_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_muxram[offset]);
}

uint16_t jpmsys5_state::mux_r(offs_t offset, uint16_t mem_mask)
{
	logerror("%s: mux_r offset: %04x mask: %04x\n", machine().describe_context(), offset<<1, mem_mask);

	switch (offset)
	{
	case 0x80 / 2: return m_dsw.read_safe(0xffff);
	case 0x82 / 2: return m_dsw2.read_safe(0xffff);
	case 0x84 / 2: return m_rotary.read_safe(0xffff);
	case 0x86 / 2: return m_strobe0.read_safe(0xffff);
	case 0x88 / 2: return m_strobe1.read_safe(0xffff);
	case 0x8a / 2: return m_strobe2.read_safe(0xffff);
	case 0x8c / 2: return m_strobe3.read_safe(0xffff);
	case 0x8e / 2: return m_strobe4.read_safe(0xffff);
	default: return 0xffff;
	}
	return 0xffff;
}




void jpmsys5_state::jpm_upd7759_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
		case 0:
		{
			m_upd7759->port_w(data & 0xff);
			m_upd7759->start_w(0);
			m_upd7759->start_w(1);
			break;
		}
		case 1:
		{
			//Reset 0x04, data 0x02, clock
			m_vfd->por(data & 0x04);
			m_vfd->data(data & 0x02);
			m_vfd->sclk(data & 0x01);
			break;
		}
		case 2:
		{
			m_upd7759->reset_w(!BIT(data, 2));
			m_upd7759->set_rom_bank(BIT(data, 1));
			break;
		}
		default:
		{
			logerror("%s: upd7759: Unknown write to %x with %x\n", machine().describe_context(),  offset, data);
			break;
		}
	}
}

uint16_t jpmsys5_state::jpm_upd7759_r()
{
	return 0x14 | m_upd7759->busy_r();
}


/*************************************
 *
 *  68000 CPU memory handlers
 *
 *************************************/

void jpmsys5_state::jpm_sys5_common_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x01ffff).rom();
	map(0x040000, 0x043fff).ram().share("nvram");
	map(0x046000, 0x046001).nopw();
	map(0x046020, 0x046023).rw("acia6850_0", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x046040, 0x04604f).rw("6840ptm", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write)).umask16(0x00ff);
	map(0x046060, 0x046067).rw("6821pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write)).umask16(0x00ff);
	map(0x046080, 0x046083).rw("acia6850_1", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);

	map(0x046084, 0x046085).r(FUNC(jpmsys5_state::unknown_port_r));
//  map(0x04608c, 0x04608f).r(FUNC(jpmsys5_state::unk_r));

	map(0x04608c, 0x04608f).rw("acia6850_2", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);

	map(0x0460c0, 0x0460c1).nopw();

	map(0x048000, 0x048001).rw(FUNC(jpmsys5_state::unk_48000_r), FUNC(jpmsys5_state::unk_48000_w));
	map(0x048002, 0x048003).r(FUNC(jpmsys5_state::unk_48002_r));
	map(0x048004, 0x048005).r(FUNC(jpmsys5_state::coins_r));
	map(0x048006, 0x048007).rw(FUNC(jpmsys5_state::unk_48006_r), FUNC(jpmsys5_state::unk_48006_w));
	map(0x048008, 0x048009).nopr().w(FUNC(jpmsys5_state::reel_0123_w)); // only reads are dummy clr opcode reads?
	map(0x04800a, 0x04800b).nopr().w(FUNC(jpmsys5_state::reel_4567_w));
	map(0x04800c, 0x04800d).rw(FUNC(jpmsys5_state::reellamps_0123_r), FUNC(jpmsys5_state::reellamps_0123_w));
	map(0x04800e, 0x04800f).rw(FUNC(jpmsys5_state::reellamps_4567_r), FUNC(jpmsys5_state::reellamps_4567_w));

	// 48010 - 4801f = some I/O device?
	map(0x048012, 0x048013).r(FUNC(jpmsys5_state::reel_optos_r));

	map(0x04c000, 0x04c0ff).r(FUNC(jpmsys5_state::mux_r)).w(FUNC(jpmsys5_state::mux_w));
}

void jpmsys5_state::m68000_ym_map(address_map &map)
{
	jpm_sys5_common_map(map);
	map(0x0460a0, 0x0460a3).w("ym2413", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x04c100, 0x04c105).rw(FUNC(jpmsys5_state::jpm_upd7759_r), FUNC(jpmsys5_state::jpm_upd7759_w));
}

void jpmsys5_state::m68000_awp_map(address_map &map)
{
	m68000_ym_map(map);
}

void jpmsys5_state::m68000_awp_map_saa(address_map &map)
{
	jpm_sys5_common_map(map);
	map(0x0460a0, 0x0460a3).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff);
	map(0x04c100, 0x04c105).rw(FUNC(jpmsys5_state::jpm_upd7759_r), FUNC(jpmsys5_state::jpm_upd7759_w)); // do the SAA boards have the UPD?
}

void jpmsys5v_state::m68000_map(address_map &map)
{
	m68000_ym_map(map);

	map(0x01fffe, 0x01ffff).w(FUNC(jpmsys5v_state::rombank_w)); // extra on video system (rom board?) (although regular games do write here?)
	map(0x020000, 0x03ffff).bankr("bank1"); // extra on video system (rom board?)

	map(0x0460e0, 0x0460e5).w(FUNC(jpmsys5v_state::ramdac_w));  // extra on video system (rom board?)
	map(0x800000, 0xcfffff).rw(FUNC(jpmsys5v_state::sys5_tms34061_r), FUNC(jpmsys5v_state::sys5_tms34061_w)); // extra on video system (rom board?)
}





/*************************************
*
*  Touchscreen controller simulation
*
*************************************/

/* Serial bit transmission callback */
TIMER_CALLBACK_MEMBER(jpmsys5v_state::touch_cb)
{
	switch (m_touch_state)
	{
	case IDLE:
		break;

	case START:
		m_touch_shift_cnt = 0;
		m_acia6850[2]->write_rxd(0);
		m_touch_state = DATA;
		break;

	case DATA:
		m_acia6850[2]->write_rxd((m_touch_data[m_touch_data_count] >> (m_touch_shift_cnt)) & 1);

		if (++m_touch_shift_cnt == 8)
			m_touch_state = STOP1;

		break;

	case STOP1:
		m_acia6850[2]->write_rxd(1);
		m_touch_state = STOP2;
		break;

	case STOP2:
		m_acia6850[2]->write_rxd(1);

		if (++m_touch_data_count == 3)
		{
			m_touch_timer->reset();
			m_touch_state = IDLE;
		}
		else
		{
			m_touch_state = START;
		}
		break;
	}
}

INPUT_CHANGED_MEMBER(jpmsys5v_state::touchscreen_press)
{
	if (newval == 0)
	{
		attotime rx_period = attotime::from_hz(10000) * 16;

		/* Each touch screen packet is 3 bytes */
		m_touch_data[0] = 0x2a;
		m_touch_data[1] = 0x7 - (m_touch_axes[1]->read() >> 5) + 0x30;
		m_touch_data[2] = (m_touch_axes[0]->read() >> 5) + 0x30;

		/* Start sending the data to the 68000 serially */
		m_touch_data_count = 0;
		m_touch_state = START;
		m_touch_timer->adjust(rx_period, 0, rx_period);
	}
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( monopoly )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Change state to enter test" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Alarm" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Discontinue alarm when jam is cleared" )
	PORT_DIPSETTING(    0x00, "Continue alarm until back door open" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Payout Percentage" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x80, "45%" )
	PORT_DIPSETTING(    0x40, "40%" )
	PORT_DIPSETTING(    0xc0, "30%" )

	PORT_START("DIRECT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Back door") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Cash door") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Refill key") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_DIPNAME( 0x08, 0x08, DEF_STR ( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR ( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR ( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Reset" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR ( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10p")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20p")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("50p")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100p")
	PORT_BIT( 0xc3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TOUCH_PUSH")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(jpmsys5v_state::touchscreen_press), 0)

	PORT_START("TOUCH_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("TOUCH_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END

/*************************************
 *
 *  6821 PIA
 *
 *************************************/
void jpmsys5_state::pia_irq(int state)
{
	m_maincpu->set_input_line(INT_6821PIA, state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t jpmsys5_state::u29_porta_r()
{
	int meter_bit = 0;

	if (m_meters)
	{
		int combined_meter = m_meters->get_activity(0) | m_meters->get_activity(1) |
							m_meters->get_activity(2) | m_meters->get_activity(3) |
							m_meters->get_activity(4) | m_meters->get_activity(5) |
							m_meters->get_activity(6) | m_meters->get_activity(7);

		if (combined_meter)
			meter_bit = 0x80;
	}

	return m_direct_port->read() | meter_bit;
}

void jpmsys5_state::u29_portb_w(uint8_t data)
{
	if (m_meters)
	{
		for (int meter = 0; meter < 8; meter++)
			m_meters->update(meter, (data & (1 << meter)));
	}
}

void jpmsys5_state::u29_ca2_w(int state)
{
	//The 'CHOP' line controls power to the reel motors, without this the reels won't turn
	m_chop = state;
}

void jpmsys5_state::u29_cb2_w(int state)
{
	//On a cabinet, this overrides the volume, we don't emulate this yet
	logerror("Alarm override enabled \n");
}

/*************************************
 *
 *  6840 PTM
 *
 *************************************/

void jpmsys5_state::ptm_irq(int state)
{
	m_maincpu->set_input_line(INT_6840PTM, state ? ASSERT_LINE : CLEAR_LINE);
}

void jpmsys5_state::u26_o1_callback(int state)
{
	if (m_mpxclk != state)
	{
		if (!state) //falling edge
		{
			if (++m_lamp_strobe > 15)
				m_lamp_strobe = 0;
		}
		sys5_draw_lamps();
	}
	m_mpxclk = state;
}


/*************************************
 *
 *  6850 ACIAs
 *
 *************************************/

void jpmsys5_state::a0_tx_w(int state)
{
	m_a0_data_out = state;
}

void jpmsys5_state::a1_tx_w(int state)
{
	m_a1_data_out = state;
}

void jpmsys5_state::a2_tx_w(int state)
{
	m_a2_data_out = state;
}

/*************************************
 *
 *  Initialisation
 *
 *************************************/

void jpmsys5v_state::machine_start()
{
	jpmsys5_state::machine_start();

	m_rombank->configure_entries(0, 32, memregion("maincpu")->base() + 0x20000, 0x20000);
	m_rombank->set_entry(0);
	m_touch_timer = timer_alloc(FUNC(jpmsys5v_state::touch_cb), this);
}

void jpmsys5v_state::machine_reset()
{
	jpmsys5_state::machine_reset();

	m_touch_timer->reset();
	m_touch_state = IDLE;
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jpmsys5_state::jpmsys5_common(machine_config& config)
{
	M68000(config, m_maincpu, 8_MHz_XTAL);

	INPUT_MERGER_ANY_HIGH(config, "acia_irq").output_handler().set_inputline(m_maincpu, INT_6850ACIA);
	bacta_datalogger_device &bacta(BACTA_DATALOGGER(config, "bacta", 0));

	ACIA6850(config, m_acia6850[0], 0);
	m_acia6850[0]->txd_handler().set("bacta", FUNC(bacta_datalogger_device::write_txd));
	m_acia6850[0]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<0>));

	bacta.rxd_handler().set(m_acia6850[0], FUNC(acia6850_device::write_rxd));

	ACIA6850(config, m_acia6850[1], 0);
	m_acia6850[1]->txd_handler().set(FUNC(jpmsys5_state::a1_tx_w));
	m_acia6850[1]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<1>));

	ACIA6850(config, m_acia6850[2], 0);
	m_acia6850[2]->txd_handler().set(FUNC(jpmsys5_state::a2_tx_w));
	m_acia6850[2]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<2>));

	clock_device &bacta_clock(CLOCK(config, "bacta_clock", 19200)); // Gives 1200 baud, but real timer is programmable (location?)
	bacta_clock.signal_handler().set(m_acia6850[0], FUNC(acia6850_device::write_txc));
	bacta_clock.signal_handler().append(m_acia6850[0], FUNC(acia6850_device::write_rxc));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 10000)); // What are the correct ACIA clocks ?
	acia_clock.signal_handler().append(m_acia6850[1], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[1], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia6850[2], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[2], FUNC(acia6850_device::write_rxc));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	S16LF01(config, m_vfd);

	pia6821_device &pia(PIA6821(config, "6821pia"));
	pia.readpa_handler().set(FUNC(jpmsys5_state::u29_porta_r));
	pia.writepb_handler().set(FUNC(jpmsys5_state::u29_portb_w));
	pia.ca2_handler().set(FUNC(jpmsys5_state::u29_ca2_w));
	pia.cb2_handler().set(FUNC(jpmsys5_state::u29_cb2_w));
	pia.irqa_handler().set(FUNC(jpmsys5_state::pia_irq));
	pia.irqb_handler().set(FUNC(jpmsys5_state::pia_irq));

	/* 6840 PTM */
	ptm6840_device &ptm(PTM6840(config, "6840ptm", 1000000/4)); // with this at 1mhz the non-video games run at a ridiculous speed
	ptm.set_external_clocks(0, 0, 0);
	ptm.o1_callback().set(FUNC(jpmsys5_state::u26_o1_callback));
	ptm.irq_callback().set(FUNC(jpmsys5_state::ptm_irq));
	config.set_default_layout(layout_jpmsys5);
}

void jpmsys5_state::ymsound(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 0.30);

	ym2413_device &ym2413(YM2413(config, "ym2413", 4000000)); /* Unconfirmed */
	ym2413.add_route(ALL_OUTPUTS, "mono", 1.00);
}

void jpmsys5_state::saasound(machine_config &config)
{
	SPEAKER(config, "mono").front_center();

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 0.30);

	SAA1099(config, "saa", 4000000 /* guess */).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void jpmsys5v_state::tmsvideo(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(40'000'000) / 4, 676, 20*4, 147*4, 256, 0, 254);
	screen.set_screen_update(FUNC(jpmsys5v_state::screen_update_jpmsys5v));

	TMS34061(config, m_tms34061, 0);
	m_tms34061->set_rowshift(8);  /* VRAM address is (row << rowshift) | col */
	m_tms34061->set_vram_size(0x40000);
	m_tms34061->int_callback().set(FUNC(jpmsys5v_state::generate_tms34061_interrupt));

	PALETTE(config, "palette").set_entries(16);
}

void jpmsys5v_state::jpmsys5v(machine_config &config)
{
	jpmsys5_common(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jpmsys5v_state::m68000_map);

	ymsound(config);

	tmsvideo(config);
}

void jpmsys5_state::reels(machine_config &config)
{
	REEL(config, m_reel[0], SYS5_100STEP_REEL, 1, 3, 0x00, 2, 200);
	m_reel[0]->optic_handler().set(FUNC(jpmsys5_state::reel_optic_cb<0>));
	REEL(config, m_reel[1], SYS5_100STEP_REEL, 1, 3, 0x00, 2, 200);
	m_reel[1]->optic_handler().set(FUNC(jpmsys5_state::reel_optic_cb<1>));
	REEL(config, m_reel[2], SYS5_100STEP_REEL, 1, 3, 0x00, 2, 200);
	m_reel[2]->optic_handler().set(FUNC(jpmsys5_state::reel_optic_cb<2>));
	REEL(config, m_reel[3], SYS5_100STEP_REEL, 1, 3, 0x00, 2, 200);
	m_reel[3]->optic_handler().set(FUNC(jpmsys5_state::reel_optic_cb<3>));
	REEL(config, m_reel[4], SYS5_100STEP_REEL, 1, 3, 0x00, 2, 200);
	m_reel[4]->optic_handler().set(FUNC(jpmsys5_state::reel_optic_cb<4>));
	REEL(config, m_reel[5], SYS5_100STEP_REEL, 1, 3, 0x00, 2, 200);
	m_reel[5]->optic_handler().set(FUNC(jpmsys5_state::reel_optic_cb<5>));
}

// later (incompatible with earlier revision) motherboards used a YM2413
void jpmsys5_state::jpmsys5_ym(machine_config &config)
{
	jpmsys5_common(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jpmsys5_state::m68000_awp_map);

	METERS(config, m_meters, 0).set_number(8);

	ymsound(config);

	reels(config);
}

// the first rev PCB used an SAA1099
void jpmsys5_state::jpmsys5(machine_config &config)
{
	jpmsys5_common(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jpmsys5_state::m68000_awp_map_saa);

	METERS(config, m_meters, 0).set_number(8);

	saasound(config);

	reels(config);
}

/*************************************
 *
 *  Port definitions
 *
 *************************************/

INPUT_PORTS_START( popeye )
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Change state to enter test" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Alarm" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Discontinue alarm when jam is cleared" )
	PORT_DIPSETTING(    0x00, "Continue alarm until back door open" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Payout Percentage" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x80, "45%" )
	PORT_DIPSETTING(    0x40, "40%" )
	PORT_DIPSETTING(    0xc0, "30%" )

	PORT_START("DSW2")
	PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("ROTARY")//not everything has this hooked up, can be used as a test switch internally
	PORT_CONFNAME(0x0F, 0x0F, "Rotary Switch")
	PORT_CONFSETTING(   0x0F, "0")
	PORT_CONFSETTING(   0x0E, "1")
	PORT_CONFSETTING(   0x0D, "2")
	PORT_CONFSETTING(   0x0C, "3")
	PORT_CONFSETTING(   0x0B, "4")
	PORT_CONFSETTING(   0x0A, "5")
	PORT_CONFSETTING(   0x09, "6")
	PORT_CONFSETTING(   0x08, "7")
	PORT_CONFSETTING(   0x06, "8")
	PORT_CONFSETTING(   0x07, "9")
	PORT_CONFSETTING(   0x05, "10")
	PORT_CONFSETTING(   0x04, "11")
	PORT_CONFSETTING(   0x03, "12")
	PORT_CONFSETTING(   0x02, "13")
	PORT_CONFSETTING(   0x01, "14")
	PORT_CONFSETTING(   0x00, "15")

	PORT_START("DIRECT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Back door") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Cash door") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Refill key") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_DIPNAME( 0x08, 0x00, "Direct 0x08" ) // These are the % key, at least for popeye? But there's a pin missing if so, usually these have 4 bits
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Direct 0x10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Direct 0x20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Reset" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("COINS")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10p")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20p")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("50p")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100p")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN5 ) PORT_NAME("Token")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("00")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("01")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("02")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("03")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("04")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("05")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("06")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("07")

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("08")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("09")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("10")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("11")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("12")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("13")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("14")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("15")

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Exchange")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Collect")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Hold 3/Lo")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Hold 2/Hi")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Cancel")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("23")

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("24")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("25")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("26")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("27")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("28")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("29")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("30")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("31")

	PORT_START("STROBE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("32")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("33")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("34")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("35")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("36")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("37")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("38")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("39")

	PORT_START("STROBE5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("40")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("41")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("42")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("43")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("44")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("45")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("46")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("47")

	PORT_START("UNKNOWN_PORT")
	PORT_DIPNAME( 0x0001, 0x0000, "Unknown 0x0001" ) // if this and 0x0008 are on then j5popeye boots, what is it? something opto related?
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Unknown 0x0002" )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Unknown 0x0004" )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0000, "Unknown 0x0008" )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, "Unknown 0x0010" )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Unknown 0x0020" )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Unknown 0x0040" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Unknown 0x0080" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Unknown 0x0100" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Unknown 0x0200" )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Unknown 0x0400" )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Unknown 0x0800" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Unknown 0x1000" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Unknown 0x2000" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "Unknown 0x4000" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, "Unknown 0x8000" )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

INPUT_PORTS_END

/*************************************
 *
 *  Initialisation
 *
 *************************************/

void jpmsys5_state::machine_start()
{
	m_lamps.resolve();
	m_sys5leds.resolve();
	m_reellamp_out.resolve();

	m_lamp_strobe = 0;
}

void jpmsys5_state::machine_reset()
{
	m_acia6850[2]->write_rxd(1);
	m_acia6850[2]->write_dcd(0);
	m_vfd->reset();
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/


ROM_START( monopoly )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7398.bin", 0x000000, 0x80000, CRC(62c80f20) SHA1(322514f920d6cb48887b624786b52af34bdb8e5f) )
	ROM_LOAD16_BYTE( "7399.bin", 0x000001, 0x80000, CRC(5f410eb6) SHA1(f9949b5cba64db77187c1723a52570bdb182ce5c) )
	ROM_LOAD16_BYTE( "6668.bin", 0x100000, 0x80000, CRC(30bf082a) SHA1(29ba67a86e82f0eb4feb816a2031d62028eb11b0) )
	ROM_LOAD16_BYTE( "6669.bin", 0x100001, 0x80000, CRC(85d38c2d) SHA1(2f1a394df243e5fbbad31507b9074c997c473106) )
	ROM_LOAD16_BYTE( "6670.bin", 0x200000, 0x80000, CRC(66e2a5e1) SHA1(04d4b55d6ad121cdc3592d33e9d953affa24f01a) )
	ROM_LOAD16_BYTE( "6671.bin", 0x200001, 0x80000, CRC(b2a3cedd) SHA1(e3a5dd028b0769e08a796a96665b31491c3b18ca) )

	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "6538.bin", 0x00000, 0x40000, CRC(ccdd4ce3) SHA1(dbb24682cea8081a447ca2c53395964fc46e7f56) )
ROM_END


ROM_START( monopolyd )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7398.bin", 0x000000, 0x80000, CRC(62c80f20) SHA1(322514f920d6cb48887b624786b52af34bdb8e5f) )
	ROM_LOAD16_BYTE( "7400.bin", 0x000001, 0x80000, CRC(d6f1f98c) SHA1(f20c788a31a8fe339aed701866180a3eb16fafb9) )
	ROM_LOAD16_BYTE( "6668.bin", 0x100000, 0x80000, CRC(30bf082a) SHA1(29ba67a86e82f0eb4feb816a2031d62028eb11b0) )
	ROM_LOAD16_BYTE( "6669.bin", 0x100001, 0x80000, CRC(85d38c2d) SHA1(2f1a394df243e5fbbad31507b9074c997c473106) )
	ROM_LOAD16_BYTE( "6670.bin", 0x200000, 0x80000, CRC(66e2a5e1) SHA1(04d4b55d6ad121cdc3592d33e9d953affa24f01a) )
	ROM_LOAD16_BYTE( "6671.bin", 0x200001, 0x80000, CRC(b2a3cedd) SHA1(e3a5dd028b0769e08a796a96665b31491c3b18ca) )

	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "6538.bin", 0x00000, 0x40000, CRC(ccdd4ce3) SHA1(dbb24682cea8081a447ca2c53395964fc46e7f56) )
ROM_END

ROM_START( monopoly4 )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mono4e1",  0x000000, 0x80000, CRC(0338b165) SHA1(fdc0fcf0fddcf88d593a22885779e8224484e7e4) )
	ROM_LOAD16_BYTE( "mono4e2",  0x000001, 0x80000, CRC(c8aa21d8) SHA1(257ecf85e1d41b15bb2bbe2157e9d3f72b7e0317) )
	ROM_LOAD16_BYTE( "6668.bin", 0x100000, 0x80000, CRC(30bf082a) SHA1(29ba67a86e82f0eb4feb816a2031d62028eb11b0) )
	ROM_LOAD16_BYTE( "6669.bin", 0x100001, 0x80000, CRC(85d38c2d) SHA1(2f1a394df243e5fbbad31507b9074c997c473106) )
	ROM_LOAD16_BYTE( "6670.bin", 0x200000, 0x80000, CRC(66e2a5e1) SHA1(04d4b55d6ad121cdc3592d33e9d953affa24f01a) )
	ROM_LOAD16_BYTE( "6671.bin", 0x200001, 0x80000, CRC(b2a3cedd) SHA1(e3a5dd028b0769e08a796a96665b31491c3b18ca) )

	ROM_REGION( 0x300000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "monov4p2", 0x000001, 0x80000, CRC(3c2dd9b7) SHA1(01c87584b3599763a0c37040199014c2902dc6f3) ) // mismatched?

	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "6538.bin", 0x00000, 0x40000, CRC(ccdd4ce3) SHA1(dbb24682cea8081a447ca2c53395964fc46e7f56) )
ROM_END

ROM_START( monopoly3 )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "monov3p1", 0x000000, 0x80000, CRC(a66fc610) SHA1(fddd3b37a6aebf5c402942d26a2fa1fa130326dd) )
	ROM_LOAD16_BYTE( "monov3p2", 0x000001, 0x80000, CRC(2d629723) SHA1(c5584113e50dc5f636dbcf80e4689d2bbfe98e71) )
	ROM_LOAD16_BYTE( "6668.bin", 0x100000, 0x80000, CRC(30bf082a) SHA1(29ba67a86e82f0eb4feb816a2031d62028eb11b0) )
	ROM_LOAD16_BYTE( "6669.bin", 0x100001, 0x80000, CRC(85d38c2d) SHA1(2f1a394df243e5fbbad31507b9074c997c473106) )
	ROM_LOAD16_BYTE( "6670.bin", 0x200000, 0x80000, CRC(66e2a5e1) SHA1(04d4b55d6ad121cdc3592d33e9d953affa24f01a) )
	ROM_LOAD16_BYTE( "6671.bin", 0x200001, 0x80000, CRC(b2a3cedd) SHA1(e3a5dd028b0769e08a796a96665b31491c3b18ca) )

	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "6538.bin", 0x00000, 0x40000, CRC(ccdd4ce3) SHA1(dbb24682cea8081a447ca2c53395964fc46e7f56) )
ROM_END

//Monopoly Classic is a non-payout quiz game, where the goal is to earn points from collecting properties.

ROM_START( monoplcl )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7401.bin", 0x000000, 0x80000, CRC(eec11426) SHA1(b732a5a64d3fba676134942768b823d088792a1f) )
	ROM_LOAD16_BYTE( "7402.bin", 0x000001, 0x80000, CRC(c4c43269) SHA1(3cad3a66aae25308e8709f8eb3f29d6858b87791) )
	ROM_LOAD16_BYTE( "6668.bin", 0x100000, 0x80000, CRC(30bf082a) SHA1(29ba67a86e82f0eb4feb816a2031d62028eb11b0) )
	ROM_LOAD16_BYTE( "6669.bin", 0x100001, 0x80000, CRC(85d38c2d) SHA1(2f1a394df243e5fbbad31507b9074c997c473106) )
	ROM_LOAD16_BYTE( "6670.bin", 0x200000, 0x80000, CRC(66e2a5e1) SHA1(04d4b55d6ad121cdc3592d33e9d953affa24f01a) )
	ROM_LOAD16_BYTE( "6671.bin", 0x200001, 0x80000, CRC(b2a3cedd) SHA1(e3a5dd028b0769e08a796a96665b31491c3b18ca) )

	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "6538.bin", 0x00000, 0x40000, CRC(ccdd4ce3) SHA1(dbb24682cea8081a447ca2c53395964fc46e7f56) )
ROM_END

ROM_START( monoplcld )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7401.bin", 0x000000, 0x80000, CRC(eec11426) SHA1(b732a5a64d3fba676134942768b823d088792a1f) )
	ROM_LOAD16_BYTE( "7403.bin", 0x000001, 0x80000, CRC(95dbacb6) SHA1(bd551ccad95440a669a547092ab126178b0d0bf9) )
	ROM_LOAD16_BYTE( "6668.bin", 0x100000, 0x80000, CRC(30bf082a) SHA1(29ba67a86e82f0eb4feb816a2031d62028eb11b0) )
	ROM_LOAD16_BYTE( "6669.bin", 0x100001, 0x80000, CRC(85d38c2d) SHA1(2f1a394df243e5fbbad31507b9074c997c473106) )
	ROM_LOAD16_BYTE( "6670.bin", 0x200000, 0x80000, CRC(66e2a5e1) SHA1(04d4b55d6ad121cdc3592d33e9d953affa24f01a) )
	ROM_LOAD16_BYTE( "6671.bin", 0x200001, 0x80000, CRC(b2a3cedd) SHA1(e3a5dd028b0769e08a796a96665b31491c3b18ca) )


	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "6538.bin", 0x00000, 0x40000, CRC(ccdd4ce3) SHA1(dbb24682cea8081a447ca2c53395964fc46e7f56) )
ROM_END

ROM_START( monopldx )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8439.bin", 0x000000, 0x80000, CRC(fbd6caa4) SHA1(73e787ae41a0ce44d48a46dd623d5e1351335e3e) )
	ROM_LOAD16_BYTE( "8440.bin", 0x000001, 0x80000, CRC(4e20aebf) SHA1(79aca78f023e7f7ae7875c18c3a7696f5ab63102) )
	ROM_LOAD16_BYTE( "6879.bin", 0x100000, 0x80000, CRC(4fbd1222) SHA1(9a9c9e4768c18a6a3e717605d3c88179676b6ad1) )
	ROM_LOAD16_BYTE( "6880.bin", 0x100001, 0x80000, CRC(0370bf5f) SHA1(a0ed1dbc6aeab02e8229f23f8ba4ff880d31e7a1) )
	ROM_LOAD16_BYTE( "6881.bin", 0x200000, 0x80000, CRC(8418ee17) SHA1(5666b90db00d9e88a37655bb9a714f076e2689d6) )
	ROM_LOAD16_BYTE( "6882.bin", 0x200001, 0x80000, CRC(400f5fb4) SHA1(80b1d3902fc9f6db24f49055b07bc31c0c74a993) )

	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "modl-snd.bin", 0x000000, 0x80000, CRC(f761da41) SHA1(a07d1b4cb7ce7a24b6fb84843543b95c3aec470f) )
ROM_END


ROM_START( monopldxd )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "8439.bin", 0x000000, 0x80000, CRC(fbd6caa4) SHA1(73e787ae41a0ce44d48a46dd623d5e1351335e3e) )
	ROM_LOAD16_BYTE( "8441.bin", 0x000001, 0x80000, CRC(d0825af4) SHA1(a7291806893c42a115763e404337976b8c30e9e0) ) // 1 byte change from 8440
	ROM_LOAD16_BYTE( "6879.bin", 0x100000, 0x80000, CRC(4fbd1222) SHA1(9a9c9e4768c18a6a3e717605d3c88179676b6ad1) )
	ROM_LOAD16_BYTE( "6880.bin", 0x100001, 0x80000, CRC(0370bf5f) SHA1(a0ed1dbc6aeab02e8229f23f8ba4ff880d31e7a1) )
	ROM_LOAD16_BYTE( "6881.bin", 0x200000, 0x80000, CRC(8418ee17) SHA1(5666b90db00d9e88a37655bb9a714f076e2689d6) )
	ROM_LOAD16_BYTE( "6882.bin", 0x200001, 0x80000, CRC(400f5fb4) SHA1(80b1d3902fc9f6db24f49055b07bc31c0c74a993) )

	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "modl-snd.bin", 0x000000, 0x80000, CRC(f761da41) SHA1(a07d1b4cb7ce7a24b6fb84843543b95c3aec470f) )
ROM_END

ROM_START( monopldx1 )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mdlxv1p1", 0x000000, 0x80000, CRC(48ab1691) SHA1(6df2aad02548d5239e3974a11228bc9aad8c9170) )
	ROM_LOAD16_BYTE( "mdlxv1p2", 0x000001, 0x80000, CRC(107c3e65) SHA1(e298b3a2826f92ba6119348a36bc4735e1799797) )
	ROM_LOAD16_BYTE( "6879.bin", 0x100000, 0x80000, CRC(4fbd1222) SHA1(9a9c9e4768c18a6a3e717605d3c88179676b6ad1) )
	ROM_LOAD16_BYTE( "6880.bin", 0x100001, 0x80000, CRC(0370bf5f) SHA1(a0ed1dbc6aeab02e8229f23f8ba4ff880d31e7a1) )
	ROM_LOAD16_BYTE( "6881.bin", 0x200000, 0x80000, CRC(8418ee17) SHA1(5666b90db00d9e88a37655bb9a714f076e2689d6) )
	ROM_LOAD16_BYTE( "6882.bin", 0x200001, 0x80000, CRC(400f5fb4) SHA1(80b1d3902fc9f6db24f49055b07bc31c0c74a993) )

	ROM_REGION( 0x300000, "altrevs", 0 )
	/* p3 missing? */
	ROM_LOAD16_BYTE( "mdlxv1p4", 0x100001, 0x80000, CRC(e3fd1a27) SHA1(6bba70ff27a6d068febcbdfa1b1f8ff2ef86ef03) ) //Doesn't match set?

	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "modl-snd.bin", 0x000000, 0x80000, CRC(f761da41) SHA1(a07d1b4cb7ce7a24b6fb84843543b95c3aec470f) )
ROM_END


ROM_START( cashcade )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cashcade_2_1.bin", 0x000000, 0x010000, CRC(7672953c) SHA1(c1a2639ab6b830c971c2533d837404ae5f5aa535) )
	ROM_LOAD16_BYTE( "cashcade_2_2.bin", 0x000001, 0x010000, CRC(8ce8cd66) SHA1(4eb00af6a0260496950d04fdcc1d3d976868ce3e) )
	ROM_LOAD16_BYTE( "cashcade_2_3.bin", 0x020000, 0x010000, CRC(a4caddd1) SHA1(074e4aa870c3d28c2f120936ef6928c3b5e14301) )
	ROM_LOAD16_BYTE( "cashcade_2_4.bin", 0x020001, 0x010000, CRC(b0f595e8) SHA1(5ca12839b87d092504d8b7cc579b8f1b2406cea1) )

	// likely missing a disk? or something with the questions on
ROM_END


/* Video based titles */
GAME( 1994, monopoly,     0,          jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly (JPM) (Version 4H) (SYSTEM5 VIDEO)",  0 )
GAME( 1994, monopolyd,    monopoly,   jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly (JPM) (Version 4H, Protocol) (SYSTEM5 VIDEO)",  0 )
GAME( 1994, monopoly4,    monopoly,   jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly (JPM) (Version 4) (SYSTEM5 VIDEO)",  0 )
GAME( 1994, monopoly3,    monopoly,   jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly (JPM) (Version 3) (SYSTEM5 VIDEO)",   0 )
GAME( 1995, monoplcl,     monopoly,   jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly Classic (JPM) (Version 5) (SYSTEM5 VIDEO)", 0 )
GAME( 1995, monoplcld,    monopoly,   jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly Classic (JPM) (Version 5, Protocol) (SYSTEM5 VIDEO)", 0 )

GAME( 1995, monopldx,     0,          jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly Deluxe (JPM) (Version 6) (SYSTEM5 VIDEO)",  0 )
GAME( 1995, monopldxd,    monopldx,   jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly Deluxe (JPM) (Version 6, Protocol) (SYSTEM5 VIDEO)", 0 )
GAME( 1995, monopldx1,    monopldx,   jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly Deluxe (JPM) (Version 1) (SYSTEM5 VIDEO)", MACHINE_NOT_WORKING ) // no questions?

GAME( 199?, cashcade,     0,          jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Cashcade (JPM) (SYSTEM5 VIDEO)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND ) // shows a loading error.. is the set incomplete?

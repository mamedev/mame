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
        in a weird daisychain setup.

    AWP game notes:
      The byte at 0x81 of the EVEN 68k rom appears to be some kind of
      control byte, probably region, or coin / machine type setting.
      Many sets differ only by this byte.

      Many sets are probably missing sound roms, however due to the
      varying motherboard configurations (SAA vs. YM, with added UPD)
      it's hard to tell until they start doing more.

***************************************************************************/

#include "emu.h"
#include "includes/jpmsys5.h"

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

WRITE_LINE_MEMBER(jpmsys5v_state::generate_tms34061_interrupt)
{
	m_maincpu->set_input_line(INT_TMS34061, state);
}

WRITE16_MEMBER(jpmsys5v_state::sys5_tms34061_w)
{
	int func = (offset >> 19) & 3;
	int row = (offset >> 7) & 0x1ff;
	int col;

	if (func == 0 || func == 2)
		col = offset  & 0xff;
	else
	{
		col = (offset << 1);

		if (~offset & 0x40000)
			row |= 0x200;
	}

	if (ACCESSING_BITS_8_15)
		m_tms34061->write(space, col, row, func, data >> 8);

	if (ACCESSING_BITS_0_7)
		m_tms34061->write(space, col | 1, row, func, data & 0xff);
}

READ16_MEMBER(jpmsys5v_state::sys5_tms34061_r)
{
	uint16_t data = 0;
	int func = (offset >> 19) & 3;
	int row = (offset >> 7) & 0x1ff;
	int col;

	if (func == 0 || func == 2)
		col = offset  & 0xff;
	else
	{
		col = (offset << 1);

		if (~offset & 0x40000)
			row |= 0x200;
	}

	if (ACCESSING_BITS_8_15)
		data |= m_tms34061->read(space, col, row, func) << 8;

	if (ACCESSING_BITS_0_7)
		data |= m_tms34061->read(space, col | 1, row, func);

	return data;
}

WRITE16_MEMBER(jpmsys5v_state::ramdac_w)
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
	int x, y;

	m_tms34061->get_display_state();

	if (m_tms34061->m_display.blanked)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		uint8_t *src = &m_tms34061->m_display.vram[(m_tms34061->m_display.dispstart & 0xffff)*2 + 256 * y];
		uint32_t *dest = &bitmap.pix32(y, cliprect.min_x);

		for (x = cliprect.min_x; x <= cliprect.max_x; x +=2)
		{
			uint8_t pen = src[(x-cliprect.min_x)>>1];

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
		m_lamps   [(m_lamp_strobe << 4) | i]     = BIT(m_muxram[(m_lamp_strobe << 2) | 0], i);
		m_lamps   [(m_lamp_strobe << 4) | i | 8] = BIT(m_muxram[(m_lamp_strobe << 2) | 1], i);
		m_sys5leds[(m_lamp_strobe << 3) | i]     = BIT(m_muxram[(m_lamp_strobe << 2) | 2], i);
	}
}

/****************************************
 *
 *  General machine functions
 *
 ****************************************/

WRITE16_MEMBER(jpmsys5v_state::rombank_w)
{
	m_rombank->set_entry(data & 0x1f);
}

READ16_MEMBER(jpmsys5_state::coins_r)
{
	if (offset == 2)
		return ioport("COINS")->read() << 8;
	else
		return 0xffff;
}

WRITE16_MEMBER(jpmsys5_state::coins_w)
{
	/* TODO */
}

READ16_MEMBER(jpmsys5_state::unk_r)
{
	return 0xffff;
}

WRITE16_MEMBER(jpmsys5_state::mux_w)
{
	m_muxram[offset]=data;
}

READ16_MEMBER(jpmsys5_state::mux_r)
{
	if (offset == 0x81/2)
		return ioport("DSW")->read();

	return 0xffff;
}

WRITE16_MEMBER(jpmsys5_state::jpm_upd7759_w)
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

READ16_MEMBER(jpmsys5_state::jpm_upd7759_r)
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
	map(0x04608c, 0x04608f).rw("acia6850_2", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x0460c0, 0x0460c1).nopw();
	map(0x048000, 0x04801f).rw(FUNC(jpmsys5_state::coins_r), FUNC(jpmsys5_state::coins_w));
	map(0x04c000, 0x04c0ff).r(FUNC(jpmsys5_state::mux_r)).w(FUNC(jpmsys5_state::mux_w));
}

void jpmsys5_state::m68000_awp_map(address_map &map)
{
	jpm_sys5_common_map(map);
	map(0x0460a0, 0x0460a3).w("ym2413", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x04c100, 0x04c105).rw(FUNC(jpmsys5_state::jpm_upd7759_r), FUNC(jpmsys5_state::jpm_upd7759_w));
}

void jpmsys5_state::m68000_awp_map_saa(address_map &map)
{
	jpm_sys5_common_map(map);
	map(0x0460a0, 0x0460a3).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff);
	map(0x04c100, 0x04c105).rw(FUNC(jpmsys5_state::jpm_upd7759_r), FUNC(jpmsys5_state::jpm_upd7759_w)); // do the SAA boards have the UPD?
}

void jpmsys5v_state::m68000_map(address_map &map)
{
	jpm_sys5_common_map(map);
	map(0x01fffe, 0x01ffff).w(FUNC(jpmsys5v_state::rombank_w)); // extra on video system (rom board?) (although regular games do write here?)
	map(0x020000, 0x03ffff).bankr("bank1"); // extra on video system (rom board?)
	map(0x0460a0, 0x0460a3).w("ym2413", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x0460e0, 0x0460e5).w(FUNC(jpmsys5v_state::ramdac_w));  // extra on video system (rom board?)
	map(0x04c100, 0x04c105).rw(FUNC(jpmsys5v_state::jpm_upd7759_r), FUNC(jpmsys5v_state::jpm_upd7759_w));
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, jpmsys5v_state,touchscreen_press, nullptr)

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
WRITE_LINE_MEMBER(jpmsys5_state::pia_irq)
{
	m_maincpu->set_input_line(INT_6821PIA, state ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(jpmsys5_state::u29_porta_r)
{
	int meter_bit =0;

	if (m_meters)
	{
		int combined_meter = m_meters->GetActivity(0) | m_meters->GetActivity(1) |
							m_meters->GetActivity(2) | m_meters->GetActivity(3) |
							m_meters->GetActivity(4) | m_meters->GetActivity(5) |
							m_meters->GetActivity(6) | m_meters->GetActivity(7);

		if (combined_meter)
			meter_bit =  0x80;
	}

	return m_direct_port->read() | meter_bit;
}

WRITE8_MEMBER(jpmsys5_state::u29_portb_w)
{
	if (m_meters)
	{
		for (int meter = 0; meter < 8; meter++)
			m_meters->update(meter, (data & (1 << meter)));
	}
}

WRITE_LINE_MEMBER(jpmsys5_state::u29_ca2_w)
{
	//The 'CHOP' line controls power to the reel motors, without this the reels won't turn
	m_chop = state;
}

WRITE_LINE_MEMBER(jpmsys5_state::u29_cb2_w)
{
	//On a cabinet, this overrides the volume, we don't emulate this yet
	logerror("Alarm override enabled \n");
}

/*************************************
 *
 *  6840 PTM
 *
 *************************************/

WRITE_LINE_MEMBER(jpmsys5_state::ptm_irq)
{
	m_maincpu->set_input_line(INT_6840PTM, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(jpmsys5_state::u26_o1_callback)
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

WRITE_LINE_MEMBER(jpmsys5_state::a0_tx_w)
{
	m_a0_data_out = state;
}

WRITE_LINE_MEMBER(jpmsys5_state::a1_tx_w)
{
	m_a1_data_out = state;
}

WRITE_LINE_MEMBER(jpmsys5_state::a2_tx_w)
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
	m_touch_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(jpmsys5v_state::touch_cb),this));
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

MACHINE_CONFIG_START(jpmsys5v_state::jpmsys5v)
	M68000(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &jpmsys5v_state::m68000_map);

	INPUT_MERGER_ANY_HIGH(config, "acia_irq").output_handler().set_inputline(m_maincpu, INT_6850ACIA);

	ACIA6850(config, m_acia6850[0], 0);
	m_acia6850[0]->txd_handler().set(FUNC(jpmsys5v_state::a0_tx_w));
	m_acia6850[0]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia6850[1], 0);
	m_acia6850[1]->txd_handler().set(FUNC(jpmsys5v_state::a1_tx_w));
	m_acia6850[1]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<1>));

	ACIA6850(config, m_acia6850[2], 0);
	m_acia6850[2]->txd_handler().set(FUNC(jpmsys5v_state::a2_tx_w));
	m_acia6850[2]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<2>));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 10000)); // What are the correct ACIA clocks ?
	acia_clock.signal_handler().set(m_acia6850[0], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[0], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia6850[1], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[1], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia6850[2], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[2], FUNC(acia6850_device::write_rxc));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	S16LF01(config, m_vfd); //for debug ports

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(40'000'000) / 4, 676, 20*4, 147*4, 256, 0, 254)
	MCFG_SCREEN_UPDATE_DRIVER(jpmsys5v_state, screen_update_jpmsys5v)

	TMS34061(config, m_tms34061, 0);
	m_tms34061->set_rowshift(8);  /* VRAM address is (row << rowshift) | col */
	m_tms34061->set_vram_size(0x40000);
	m_tms34061->int_callback().set(FUNC(jpmsys5v_state::generate_tms34061_interrupt));

	MCFG_PALETTE_ADD("palette", 16)

	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("upd7759", UPD7759)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	/* Earlier revisions use an SAA1099, but no video card games seem to (?) */
	MCFG_DEVICE_ADD("ym2413", YM2413, 4000000 ) /* Unconfirmed */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	pia6821_device &pia(PIA6821(config, "6821pia", 0));
	pia.readpa_handler().set(FUNC(jpmsys5v_state::u29_porta_r));
	pia.writepb_handler().set(FUNC(jpmsys5v_state::u29_portb_w));
	pia.ca2_handler().set(FUNC(jpmsys5v_state::u29_ca2_w));
	pia.cb2_handler().set(FUNC(jpmsys5v_state::u29_cb2_w));
	pia.irqa_handler().set(FUNC(jpmsys5v_state::pia_irq));
	pia.irqb_handler().set(FUNC(jpmsys5v_state::pia_irq));

	/* 6840 PTM */
	ptm6840_device &ptm(PTM6840(config, "6840ptm", 1000000));
	ptm.set_external_clocks(0, 0, 0);
	ptm.o1_callback().set(FUNC(jpmsys5v_state::u26_o1_callback));
	ptm.irq_callback().set(FUNC(jpmsys5v_state::ptm_irq));
MACHINE_CONFIG_END

READ16_MEMBER(jpmsys5_state::mux_awp_r)
{
	static const char *const portnames[] = { "DSW", "DSW2", "ROTARY", "STROBE0", "STROBE1", "STROBE2", "STROBE3", "STROBE4" };

	if ((offset >0x7f) && (offset <0x8f))
	{
		return ioport(portnames[( (offset - 0x80) >>1)])->read();
	}
	else
	{
		return 0xffff;
	}
}

READ16_MEMBER(jpmsys5_state::coins_awp_r)
{
	switch (offset)
	{
		case 2:
		{
			return ioport("COINS")->read() << 8;
		}
		default:
		{
			logerror("coins read offset: %x",offset);
			return 0xffff;
		}
	}
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )

	PORT_START("COINS")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10p")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20p")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("50p")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100p")
	PORT_BIT( 0xc3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("STROBE0")
	PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("STROBE1")
	PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("STROBE2")
	PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("STROBE3")
	PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("STROBE4")
	PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("STROBE5")
	PORT_BIT(0xFF, IP_ACTIVE_LOW, IPT_UNKNOWN)
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
}

void jpmsys5_state::machine_reset()
{
	m_acia6850[2]->write_rxd(1);
	m_acia6850[2]->write_dcd(0);
	m_vfd->reset();
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

// later (incompatible with earlier revision) motherboards used a YM2413
void jpmsys5_state::jpmsys5_ym(machine_config &config)
{
	M68000(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &jpmsys5_state::m68000_awp_map);

	INPUT_MERGER_ANY_HIGH(config, "acia_irq").output_handler().set_inputline(m_maincpu, INT_6850ACIA);

	ACIA6850(config, m_acia6850[0], 0);
	m_acia6850[0]->txd_handler().set(FUNC(jpmsys5_state::a0_tx_w));
	m_acia6850[0]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia6850[1], 0);
	m_acia6850[1]->txd_handler().set(FUNC(jpmsys5_state::a1_tx_w));
	m_acia6850[1]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<1>));

	ACIA6850(config, m_acia6850[2], 0);
	m_acia6850[2]->txd_handler().set(FUNC(jpmsys5_state::a2_tx_w));
	m_acia6850[2]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<2>));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 10000)); // What are the correct ACIA clocks ?
	acia_clock.signal_handler().set(m_acia6850[0], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[0], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia6850[1], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[1], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia6850[2], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[2], FUNC(acia6850_device::write_rxc));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	S16LF01(config, m_vfd);

	SPEAKER(config, "mono").front_center();

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 0.30);

	/* Earlier revisions use an SAA1099 */
	ym2413_device &ym2413(YM2413(config, "ym2413", 4000000)); /* Unconfirmed */
	ym2413.add_route(ALL_OUTPUTS, "mono", 1.00);

	pia6821_device &pia(PIA6821(config, "6821pia", 0));
	pia.readpa_handler().set(FUNC(jpmsys5_state::u29_porta_r));
	pia.writepb_handler().set(FUNC(jpmsys5_state::u29_portb_w));
	pia.ca2_handler().set(FUNC(jpmsys5_state::u29_ca2_w));
	pia.cb2_handler().set(FUNC(jpmsys5_state::u29_cb2_w));
	pia.irqa_handler().set(FUNC(jpmsys5_state::pia_irq));
	pia.irqb_handler().set(FUNC(jpmsys5_state::pia_irq));

	/* 6840 PTM */
	ptm6840_device &ptm(PTM6840(config, "6840ptm", 1000000));
	ptm.set_external_clocks(0, 0, 0);
	ptm.o1_callback().set(FUNC(jpmsys5_state::u26_o1_callback));
	ptm.irq_callback().set(FUNC(jpmsys5_state::ptm_irq));
	config.set_default_layout(layout_jpmsys5);

	METERS(config, m_meters, 0).set_number(8);
}

// the first rev PCB used an SAA1099
void jpmsys5_state::jpmsys5(machine_config &config)
{
	M68000(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &jpmsys5_state::m68000_awp_map_saa);

	INPUT_MERGER_ANY_HIGH(config, "acia_irq").output_handler().set_inputline(m_maincpu, INT_6850ACIA);

	ACIA6850(config, m_acia6850[0], 0);
	m_acia6850[0]->txd_handler().set(FUNC(jpmsys5_state::a0_tx_w));
	m_acia6850[0]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia6850[1], 0);
	m_acia6850[1]->txd_handler().set(FUNC(jpmsys5_state::a1_tx_w));
	m_acia6850[1]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<1>));

	ACIA6850(config, m_acia6850[2], 0);
	m_acia6850[2]->txd_handler().set(FUNC(jpmsys5_state::a2_tx_w));
	m_acia6850[2]->irq_handler().set("acia_irq", FUNC(input_merger_device::in_w<2>));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 10000)); // What are the correct ACIA clocks ?
	acia_clock.signal_handler().set(m_acia6850[0], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[0], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia6850[1], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[1], FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia6850[2], FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia6850[2], FUNC(acia6850_device::write_rxc));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	S16LF01(config, m_vfd);

	SPEAKER(config, "mono").front_center();

	UPD7759(config, m_upd7759);
	m_upd7759->add_route(ALL_OUTPUTS, "mono", 0.30);

	SAA1099(config, "saa", 4000000 /* guess */).add_route(ALL_OUTPUTS, "mono", 1.0);

	pia6821_device &pia(PIA6821(config, "6821pia", 0));
	pia.readpa_handler().set(FUNC(jpmsys5_state::u29_porta_r));
	pia.writepb_handler().set(FUNC(jpmsys5_state::u29_portb_w));
	pia.ca2_handler().set(FUNC(jpmsys5_state::u29_ca2_w));
	pia.cb2_handler().set(FUNC(jpmsys5_state::u29_cb2_w));
	pia.irqa_handler().set(FUNC(jpmsys5_state::pia_irq));
	pia.irqb_handler().set(FUNC(jpmsys5_state::pia_irq));

	/* 6840 PTM */
	ptm6840_device &ptm(PTM6840(config, "6840ptm", 1000000));
	ptm.set_external_clocks(0, 0, 0);
	ptm.o1_callback().set(FUNC(jpmsys5_state::u26_o1_callback));
	ptm.irq_callback().set(FUNC(jpmsys5_state::ptm_irq));
	config.set_default_layout(layout_jpmsys5);

	METERS(config, m_meters, 0).set_number(8);
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



	ROM_REGION( 0x300000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7400.bin", 0x00001, 0x080000, CRC(d6f1f98c) SHA1(f20c788a31a8fe339aed701866180a3eb16fafb9) )

	// this version doesn't work too well, missing other roms, or just bad?
	ROM_LOAD16_BYTE( "monov3p1", 0x00000, 0x080000, CRC(a66fc610) SHA1(fddd3b37a6aebf5c402942d26a2fa1fa130326dd) )
	ROM_LOAD16_BYTE( "monov3p2", 0x00001, 0x080000, CRC(2d629723) SHA1(c5584113e50dc5f636dbcf80e4689d2bbfe98e71) )
	// mismastched?
	ROM_LOAD16_BYTE( "monov4p2", 0x00001, 0x080000, CRC(3c2dd9b7) SHA1(01c87584b3599763a0c37040199014c2902dc6f3) )


	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "6538.bin", 0x00000, 0x40000, CRC(ccdd4ce3) SHA1(dbb24682cea8081a447ca2c53395964fc46e7f56) )
ROM_END

ROM_START( monopolya )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "mono4e1",  0x000000, 0x80000, CRC(0338b165) SHA1(fdc0fcf0fddcf88d593a22885779e8224484e7e4) )
	ROM_LOAD16_BYTE( "mono4e2",  0x000001, 0x80000, CRC(c8aa21d8) SHA1(257ecf85e1d41b15bb2bbe2157e9d3f72b7e0317) )
	ROM_LOAD16_BYTE( "6668.bin", 0x100000, 0x80000, CRC(30bf082a) SHA1(29ba67a86e82f0eb4feb816a2031d62028eb11b0) )
	ROM_LOAD16_BYTE( "6669.bin", 0x100001, 0x80000, CRC(85d38c2d) SHA1(2f1a394df243e5fbbad31507b9074c997c473106) )
	ROM_LOAD16_BYTE( "6670.bin", 0x200000, 0x80000, CRC(66e2a5e1) SHA1(04d4b55d6ad121cdc3592d33e9d953affa24f01a) )
	ROM_LOAD16_BYTE( "6671.bin", 0x200001, 0x80000, CRC(b2a3cedd) SHA1(e3a5dd028b0769e08a796a96665b31491c3b18ca) )

	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "6538.bin", 0x00000, 0x40000, CRC(ccdd4ce3) SHA1(dbb24682cea8081a447ca2c53395964fc46e7f56) )
ROM_END


ROM_START( monoplcl )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "7401.bin", 0x000000, 0x80000, CRC(eec11426) SHA1(b732a5a64d3fba676134942768b823d088792a1f) )
	ROM_LOAD16_BYTE( "7402.bin", 0x000001, 0x80000, CRC(c4c43269) SHA1(3cad3a66aae25308e8709f8eb3f29d6858b87791) )
	ROM_LOAD16_BYTE( "6668.bin", 0x100000, 0x80000, CRC(30bf082a) SHA1(29ba67a86e82f0eb4feb816a2031d62028eb11b0) )
	ROM_LOAD16_BYTE( "6669.bin", 0x100001, 0x80000, CRC(85d38c2d) SHA1(2f1a394df243e5fbbad31507b9074c997c473106) )
	ROM_LOAD16_BYTE( "6670.bin", 0x200000, 0x80000, CRC(66e2a5e1) SHA1(04d4b55d6ad121cdc3592d33e9d953affa24f01a) )
	ROM_LOAD16_BYTE( "6671.bin", 0x200001, 0x80000, CRC(b2a3cedd) SHA1(e3a5dd028b0769e08a796a96665b31491c3b18ca) )

	ROM_REGION( 0x300000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "7403.bin", 0x000001, 0x080000, CRC(95dbacb6) SHA1(bd551ccad95440a669a547092ab126178b0d0bf9) )

	ROM_LOAD( "mdlxv1p1", 0x0000, 0x080000, CRC(48ab1691) SHA1(6df2aad02548d5239e3974a11228bc9aad8c9170) )
	ROM_LOAD( "mdlxv1p2", 0x0000, 0x080000, CRC(107c3e65) SHA1(e298b3a2826f92ba6119348a36bc4735e1799797) )
	/* p3 missing? */
	ROM_LOAD( "mdlxv1p4", 0x0000, 0x080000, CRC(e3fd1a27) SHA1(6bba70ff27a6d068febcbdfa1b1f8ff2ef86ef03) )


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

	ROM_REGION( 0x300000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "8441.bin", 0x000001, 0x080000, CRC(d0825af4) SHA1(a7291806893c42a115763e404337976b8c30e9e0) ) // 1 byte change from 8440 (doesn't boot, but might want additional hw)


	ROM_REGION( 0x80000, "upd7759", 0 )
	ROM_LOAD( "modl-snd.bin", 0x000000, 0x80000, CRC(f761da41) SHA1(a07d1b4cb7ce7a24b6fb84843543b95c3aec470f) )
ROM_END


ROM_START( cashcade )
	ROM_REGION( 0x300000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "cashcade_2_1.bin", 0x000000, 0x010000, CRC(7672953c) SHA1(c1a2639ab6b830c971c2533d837404ae5f5aa535) )
	ROM_LOAD16_BYTE( "cashcade_2_2.bin", 0x000001, 0x010000, CRC(8ce8cd66) SHA1(4eb00af6a0260496950d04fdcc1d3d976868ce3e) )
	ROM_LOAD16_BYTE( "cashcade_2_3.bin", 0x020000, 0x010000, CRC(a4caddd1) SHA1(074e4aa870c3d28c2f120936ef6928c3b5e14301) )
	ROM_LOAD16_BYTE( "cashcade_2_4.bin", 0x020001, 0x010000, CRC(b0f595e8) SHA1(5ca12839b87d092504d8b7cc579b8f1b2406cea1) )
ROM_END






/* Video based titles */
GAME( 1994, monopoly,     0,          jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly (JPM) (SYSTEM5 VIDEO, set 1)",  0 )
GAME( 1994, monopolya,    monopoly,   jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly (JPM) (SYSTEM5 VIDEO, set 2)",  0 )
GAME( 1995, monoplcl,     monopoly,   jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly Classic (JPM) (SYSTEM5 VIDEO)", 0 )
GAME( 1995, monopldx,     0,          jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Monopoly Deluxe (JPM) (SYSTEM5 VIDEO)",  0 )
GAME( 199?, cashcade,     0,          jpmsys5v, monopoly, jpmsys5v_state, empty_init, ROT0, "JPM", "Cashcade (JPM) (SYSTEM5 VIDEO)",         MACHINE_NOT_WORKING|MACHINE_NO_SOUND ) // shows a loading error.. is the set incomplete?

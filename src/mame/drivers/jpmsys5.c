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
#include "sound/saa1099.h"
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

WRITE_LINE_MEMBER(jpmsys5_state::generate_tms34061_interrupt)
{
	m_maincpu->set_input_line(INT_TMS34061, state);
}

WRITE16_MEMBER(jpmsys5_state::sys5_tms34061_w)
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

READ16_MEMBER(jpmsys5_state::sys5_tms34061_r)
{
	UINT16 data = 0;
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

WRITE16_MEMBER(jpmsys5_state::ramdac_w)
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

UINT32 jpmsys5_state::screen_update_jpmsys5v(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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
		UINT8 *src = &m_tms34061->m_display.vram[(m_tms34061->m_display.dispstart & 0xffff)*2 + 256 * y];
		UINT32 *dest = &bitmap.pix32(y, cliprect.min_x);

		for (x = cliprect.min_x; x <= cliprect.max_x; x +=2)
		{
			UINT8 pen = src[(x-cliprect.min_x)>>1];

			/* Draw two 4-bit pixels */
			*dest++ = m_palette->pen((pen >> 4) & 0xf);
			*dest++ = m_palette->pen(pen & 0xf);
		}
	}

	return 0;
}

void jpmsys5_state::sys5_draw_lamps()
{
	int i;
	for (i = 0; i <8; i++)
	{
		output_set_lamp_value( (16*m_lamp_strobe)+i,  (m_muxram[(4*m_lamp_strobe)] & (1 << i)) !=0);
		output_set_lamp_value((16*m_lamp_strobe)+i+8, (m_muxram[(4*m_lamp_strobe) +1 ] & (1 << i)) !=0);
		output_set_indexed_value("sys5led",(8*m_lamp_strobe)+i,(m_muxram[(4*m_lamp_strobe) +2 ] & (1 << i)) !=0);
	}
}

/****************************************
 *
 *  General machine functions
 *
 ****************************************/

WRITE16_MEMBER(jpmsys5_state::rombank_w)
{
	UINT8 *rom = memregion("maincpu")->base();
	data &= 0x1f;
	membank("bank1")->set_base(&rom[0x20000 + 0x20000 * data]);
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
			m_upd7759->port_w(space, 0, data & 0xff);
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
			m_upd7759->reset_w(~data & 0x04);
			m_upd7759->set_bank_base((data & 2) ? 0x20000 : 0);
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

#define JPM_SYS5_COMMON_MAP \
	ADDRESS_MAP_UNMAP_HIGH \
	AM_RANGE(0x000000, 0x01ffff) AM_ROM \
	AM_RANGE(0x040000, 0x043fff) AM_RAM AM_SHARE("nvram") \
	AM_RANGE(0x046000, 0x046001) AM_WRITENOP \
	AM_RANGE(0x046020, 0x046021) AM_DEVREADWRITE8("acia6850_0", acia6850_device, status_r, control_w, 0xff) \
	AM_RANGE(0x046022, 0x046023) AM_DEVREADWRITE8("acia6850_0", acia6850_device, data_r, data_w, 0xff) \
	AM_RANGE(0x046040, 0x04604f) AM_DEVREADWRITE8("6840ptm", ptm6840_device, read, write, 0xff) \
	AM_RANGE(0x046060, 0x046067) AM_DEVREADWRITE8("6821pia", pia6821_device, read, write,0xff) \
	AM_RANGE(0x046080, 0x046081) AM_DEVREADWRITE8("acia6850_1", acia6850_device, status_r, control_w, 0xff) \
	AM_RANGE(0x046082, 0x046083) AM_DEVREADWRITE8("acia6850_1", acia6850_device, data_r, data_w, 0xff) \
	AM_RANGE(0x04608c, 0x04608d) AM_DEVREADWRITE8("acia6850_2", acia6850_device, status_r, control_w, 0xff) \
	AM_RANGE(0x04608e, 0x04608f) AM_DEVREADWRITE8("acia6850_2", acia6850_device, data_r, data_w, 0xff) \
	AM_RANGE(0x0460c0, 0x0460c1) AM_WRITENOP \
	AM_RANGE(0x048000, 0x04801f) AM_READWRITE(coins_r, coins_w) \
	AM_RANGE(0x04c000, 0x04c0ff) AM_READ(mux_r) AM_WRITE(mux_w)

static ADDRESS_MAP_START( 68000_awp_map, AS_PROGRAM, 16, jpmsys5_state )
	JPM_SYS5_COMMON_MAP
	AM_RANGE(0x0460a0, 0x0460a3) AM_DEVWRITE8("ym2413", ym2413_device, write, 0x00ff)
	AM_RANGE(0x04c100, 0x04c105) AM_READWRITE(jpm_upd7759_r, jpm_upd7759_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( 68000_awp_map_saa, AS_PROGRAM, 16, jpmsys5_state )
	JPM_SYS5_COMMON_MAP
	AM_RANGE(0x0460a0, 0x0460a1) AM_DEVWRITE8("saa", saa1099_device, data_w, 0x00ff)
	AM_RANGE(0x0460a2, 0x0460a3) AM_DEVWRITE8("saa", saa1099_device, control_w, 0x00ff)
	AM_RANGE(0x04c100, 0x04c105) AM_READWRITE(jpm_upd7759_r, jpm_upd7759_w) // do the SAA boards have the UPD?
ADDRESS_MAP_END

static ADDRESS_MAP_START( 68000_map, AS_PROGRAM, 16, jpmsys5_state )
	JPM_SYS5_COMMON_MAP
	AM_RANGE(0x01fffe, 0x01ffff) AM_WRITE(rombank_w) // extra on video system (rom board?) (although regular games do write here?)
	AM_RANGE(0x020000, 0x03ffff) AM_ROMBANK("bank1") // extra on video system (rom board?)
	AM_RANGE(0x0460a0, 0x0460a3) AM_DEVWRITE8("ym2413", ym2413_device, write, 0x00ff)
	AM_RANGE(0x0460e0, 0x0460e5) AM_WRITE(ramdac_w)  // extra on video system (rom board?)
	AM_RANGE(0x04c100, 0x04c105) AM_READWRITE(jpm_upd7759_r, jpm_upd7759_w)
	AM_RANGE(0x800000, 0xcfffff) AM_READWRITE(sys5_tms34061_r, sys5_tms34061_w) // extra on video system (rom board?)
ADDRESS_MAP_END





	/*************************************
	*
	*  Touchscreen controller simulation
	*
	*************************************/

/* Serial bit transmission callback */
TIMER_CALLBACK_MEMBER(jpmsys5_state::touch_cb)
{
	switch (m_touch_state)
	{
		case IDLE:
		{
			break;
		}
		case START:
		{
			m_touch_shift_cnt = 0;
			m_acia6850_2->write_rxd(0);
			m_touch_state = DATA;
			break;
		}
		case DATA:
		{
			m_acia6850_2->write_rxd((m_touch_data[m_touch_data_count] >> (m_touch_shift_cnt)) & 1);

			if (++m_touch_shift_cnt == 8)
				m_touch_state = STOP1;

			break;
		}
		case STOP1:
		{
			m_acia6850_2->write_rxd(1);
			m_touch_state = STOP2;
			break;
		}
		case STOP2:
		{
			m_acia6850_2->write_rxd(1);

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
}

INPUT_CHANGED_MEMBER(jpmsys5_state::touchscreen_press)
{
	if (newval == 0)
	{
		attotime rx_period = attotime::from_hz(10000) * 16;

		/* Each touch screen packet is 3 bytes */
		m_touch_data[0] = 0x2a;
		m_touch_data[1] = 0x7 - (ioport("TOUCH_Y")->read() >> 5) + 0x30;
		m_touch_data[2] = (ioport("TOUCH_X")->read() >> 5) + 0x30;

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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, jpmsys5_state,touchscreen_press, NULL)

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
	int combined_meter = MechMtr_GetActivity(0) | MechMtr_GetActivity(1) |
							MechMtr_GetActivity(2) | MechMtr_GetActivity(3) |
							MechMtr_GetActivity(4) | MechMtr_GetActivity(5) |
							MechMtr_GetActivity(6) | MechMtr_GetActivity(7);

	int meter_bit =0;
	if(combined_meter)
	{
		meter_bit =  0x80;
	}
	else
	{
		meter_bit =  0x00;
	}

	return m_direct_port->read() | meter_bit;
}

WRITE8_MEMBER(jpmsys5_state::u29_portb_w)
{
	int meter =0;
	for (meter = 0; meter < 8; meter ++)
	{
		MechMtr_update(meter, (data & (1 << meter)));
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

WRITE8_MEMBER(jpmsys5_state::u26_o1_callback)
{
	if (m_mpxclk !=data)
	{
		if (!data) //falling edge
		{
			m_lamp_strobe++;
			if (m_lamp_strobe >15)
			{
				m_lamp_strobe =0;
			}
		}
		sys5_draw_lamps();
	}
	m_mpxclk = data;
}


/*************************************
 *
 *  6850 ACIAs
 *
 *************************************/

WRITE_LINE_MEMBER(jpmsys5_state::acia_irq)
{
	m_maincpu->set_input_line(INT_6850ACIA, state ? ASSERT_LINE : CLEAR_LINE);
}

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

WRITE_LINE_MEMBER(jpmsys5_state::write_acia_clock)
{
	m_acia6850_0->write_txc(state);
	m_acia6850_0->write_rxc(state);
	m_acia6850_1->write_txc(state);
	m_acia6850_1->write_rxc(state);
	m_acia6850_2->write_txc(state);
	m_acia6850_2->write_rxc(state);
}

/*************************************
 *
 *  Initialisation
 *
 *************************************/

MACHINE_START_MEMBER(jpmsys5_state,jpmsys5v)
{
	membank("bank1")->set_base(memregion("maincpu")->base()+0x20000);
	m_touch_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(jpmsys5_state::touch_cb),this));
}

MACHINE_RESET_MEMBER(jpmsys5_state,jpmsys5v)
{
	m_touch_timer->reset();
	m_touch_state = IDLE;
	m_acia6850_2->write_rxd(1);
	m_acia6850_2->write_dcd(0);
	m_vfd->reset();

}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( jpmsys5v, jpmsys5_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(68000_map)

	MCFG_DEVICE_ADD("acia6850_0", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(jpmsys5_state, a0_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(jpmsys5_state, acia_irq))

	MCFG_DEVICE_ADD("acia6850_1", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(jpmsys5_state, a1_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(jpmsys5_state, acia_irq))

	MCFG_DEVICE_ADD("acia6850_2", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(jpmsys5_state, a2_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(jpmsys5_state, acia_irq))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 10000) // What are the correct ACIA clocks ?
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(jpmsys5_state, write_acia_clock))

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_S16LF01_ADD("vfd",0)//for debug ports

	MCFG_MACHINE_START_OVERRIDE(jpmsys5_state,jpmsys5v)
	MCFG_MACHINE_RESET_OVERRIDE(jpmsys5_state,jpmsys5v)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_40MHz / 4, 676, 20*4, 147*4, 256, 0, 254)
	MCFG_SCREEN_UPDATE_DRIVER(jpmsys5_state, screen_update_jpmsys5v)

	MCFG_DEVICE_ADD("tms34061", TMS34061, 0)
	MCFG_TMS34061_ROWSHIFT(8)  /* VRAM address is (row << rowshift) | col */
	MCFG_TMS34061_VRAM_SIZE(0x40000) /* size of video RAM */
	MCFG_TMS34061_INTERRUPT_CB(WRITELINE(jpmsys5_state, generate_tms34061_interrupt))      /* interrupt gen callback */

	MCFG_PALETTE_ADD("palette", 16)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("upd7759", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	/* Earlier revisions use an SAA1099, but no video card games seem to (?) */
	MCFG_SOUND_ADD("ym2413", YM2413, 4000000 ) /* Unconfirmed */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_DEVICE_ADD("6821pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(jpmsys5_state, u29_porta_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(jpmsys5_state, u29_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(jpmsys5_state, u29_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(jpmsys5_state, u29_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(jpmsys5_state, pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(jpmsys5_state, pia_irq))

	/* 6840 PTM */
	MCFG_DEVICE_ADD("6840ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(1000000)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_OUT0_CB(WRITE8(jpmsys5_state, u26_o1_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE(jpmsys5_state, ptm_irq))
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
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )

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

MACHINE_START_MEMBER(jpmsys5_state,jpmsys5)
{
//  membank("bank1")->set_base(memregion("maincpu")->base()+0x20000);

	/* setup 8 mechanical meters */
	MechMtr_config(machine(),8);

}

MACHINE_RESET_MEMBER(jpmsys5_state,jpmsys5)
{
	m_acia6850_2->write_rxd(1);
	m_acia6850_2->write_dcd(0);
	m_vfd->reset();
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

// later (incompatible with earlier revision) motherboards used a YM2413
MACHINE_CONFIG_START( jpmsys5_ym, jpmsys5_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz)

	MCFG_CPU_PROGRAM_MAP(68000_awp_map)

	MCFG_DEVICE_ADD("acia6850_0", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(jpmsys5_state, a0_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(jpmsys5_state, acia_irq))

	MCFG_DEVICE_ADD("acia6850_1", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(jpmsys5_state, a1_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(jpmsys5_state, acia_irq))

	MCFG_DEVICE_ADD("acia6850_2", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(jpmsys5_state, a2_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(jpmsys5_state, acia_irq))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 10000) // What are the correct ACIA clocks ?
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(jpmsys5_state, write_acia_clock))

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_S16LF01_ADD("vfd",0)

	MCFG_MACHINE_START_OVERRIDE(jpmsys5_state,jpmsys5)
	MCFG_MACHINE_RESET_OVERRIDE(jpmsys5_state,jpmsys5)
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("upd7759", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	/* Earlier revisions use an SAA1099 */
	MCFG_SOUND_ADD("ym2413", YM2413, 4000000 ) /* Unconfirmed */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_DEVICE_ADD("6821pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(jpmsys5_state, u29_porta_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(jpmsys5_state, u29_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(jpmsys5_state, u29_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(jpmsys5_state, u29_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(jpmsys5_state, pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(jpmsys5_state, pia_irq))

	/* 6840 PTM */
	MCFG_DEVICE_ADD("6840ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(1000000)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_OUT0_CB(WRITE8(jpmsys5_state, u26_o1_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE(jpmsys5_state, ptm_irq))
	MCFG_DEFAULT_LAYOUT(layout_jpmsys5)
MACHINE_CONFIG_END

// the first rev PCB used an SAA1099
MACHINE_CONFIG_START( jpmsys5, jpmsys5_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_8MHz)
	MCFG_CPU_PROGRAM_MAP(68000_awp_map_saa)

	MCFG_DEVICE_ADD("acia6850_0", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(jpmsys5_state, a0_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(jpmsys5_state, acia_irq))

	MCFG_DEVICE_ADD("acia6850_1", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(jpmsys5_state, a1_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(jpmsys5_state, acia_irq))

	MCFG_DEVICE_ADD("acia6850_2", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE(jpmsys5_state, a2_tx_w))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(jpmsys5_state, acia_irq))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 10000) // What are the correct ACIA clocks ?
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(jpmsys5_state, write_acia_clock))

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_S16LF01_ADD("vfd",0)

	MCFG_MACHINE_START_OVERRIDE(jpmsys5_state,jpmsys5)
	MCFG_MACHINE_RESET_OVERRIDE(jpmsys5_state,jpmsys5)
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("upd7759", UPD7759, UPD7759_STANDARD_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	MCFG_SAA1099_ADD("saa", 4000000 /* guess */)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_DEVICE_ADD("6821pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(jpmsys5_state, u29_porta_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(jpmsys5_state, u29_portb_w))
	MCFG_PIA_CA2_HANDLER(WRITELINE(jpmsys5_state, u29_ca2_w))
	MCFG_PIA_CB2_HANDLER(WRITELINE(jpmsys5_state, u29_cb2_w))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(jpmsys5_state, pia_irq))
	MCFG_PIA_IRQB_HANDLER(WRITELINE(jpmsys5_state, pia_irq))

	/* 6840 PTM */
	MCFG_DEVICE_ADD("6840ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(1000000)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_OUT0_CB(WRITE8(jpmsys5_state, u26_o1_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE(jpmsys5_state, ptm_irq))
	MCFG_DEFAULT_LAYOUT(layout_jpmsys5)
MACHINE_CONFIG_END

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
GAME( 1994, monopoly    , 0         , jpmsys5v, monopoly, driver_device, 0, ROT0, "JPM", "Monopoly (JPM) (SYSTEM5 VIDEO, set 1)",         0 )
GAME( 1994, monopolya   , monopoly  , jpmsys5v, monopoly, driver_device, 0, ROT0, "JPM", "Monopoly (JPM) (SYSTEM5 VIDEO, set 2)",         0 )
GAME( 1995, monoplcl    , monopoly  , jpmsys5v, monopoly, driver_device, 0, ROT0, "JPM", "Monopoly Classic (JPM) (SYSTEM5 VIDEO)", 0 )
GAME( 1995, monopldx    , 0         , jpmsys5v, monopoly, driver_device, 0, ROT0, "JPM", "Monopoly Deluxe (JPM) (SYSTEM5 VIDEO)",  0 )
GAME( 199?, cashcade    , 0         , jpmsys5v, monopoly, driver_device, 0, ROT0, "JPM", "Cashcade (JPM) (SYSTEM5 VIDEO)", GAME_NOT_WORKING|GAME_NO_SOUND ) // shows a loading error.. is the set incomplete?

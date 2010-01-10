/***************************************************************************

    JPM System 5 with Video Expansion 2 hardware

    driver by Phil Bennett

    Games supported:
        * Monopoly
        * Monopoly Classic
        * Monopoly Deluxe

    Known Issues:
        * Artwork support is needed as the monitor bezel illuminates
        to indicate progress through the games.
        * Features used by the AWP games such as lamps, reels and
        meters are not emulated.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "sound/2413intf.h"
#include "sound/upd7759.h"
#include "video/tms34061.h"


/*************************************
 *
 *  Globals
 *
 *************************************/

static UINT8 palette[16][3];
static int pal_addr;
static int pal_idx;

static enum state { IDLE, START, DATA, STOP1, STOP2 } touch_state;
static emu_timer *touch_timer;
static int touch_data_count;
static int touch_data[3];
static int touch_shift_cnt;

static UINT8 a0_acia_dcd;
static UINT8 a0_data_out;
static UINT8 a0_data_in;

static UINT8 a1_acia_dcd;
static UINT8 a1_data_out;
static UINT8 a1_data_in;

static UINT8 a2_acia_dcd;
static UINT8 a2_data_out;
static UINT8 a2_data_in;


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
	INT_POWERFAIL  = 7,
};


/*************************************
 *
 *  Video hardware
 *
 *************************************/

static void tms_interrupt(running_machine *machine, int state)
{
	cputag_set_input_line(machine, "maincpu", INT_TMS34061, state);
}

static const struct tms34061_interface tms34061intf =
{
	"screen",		/* The screen we are acting on */
	8,				/* VRAM address is (row << rowshift) | col */
	0x40000,		/* Size of video RAM - FIXME: Should be 128kB + 32kB */
	tms_interrupt	/* Interrupt gen callback */
};

static WRITE16_HANDLER( sys5_tms34061_w )
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
		tms34061_w(space, col, row, func, data >> 8);

	if (ACCESSING_BITS_0_7)
		tms34061_w(space, col | 1, row, func, data & 0xff);
}

static READ16_HANDLER( sys5_tms34061_r )
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
		data |= tms34061_r(space, col, row, func) << 8;

	if (ACCESSING_BITS_0_7)
		data |= tms34061_r(space, col | 1, row, func);

	return data;
}

static WRITE16_HANDLER( ramdac_w )
{
	if (offset == 0)
	{
		pal_addr = data;
		pal_idx = 0;
	}
	else if (offset == 1)
	{
		palette[pal_addr][pal_idx] = data;

		if (++pal_idx == 3)
		{
			/* Update the MAME palette */
			palette_set_color_rgb(space->machine, pal_addr, pal6bit(palette[pal_addr][0]), pal6bit(palette[pal_addr][1]), pal6bit(palette[pal_addr][2]));
			pal_addr++;
			pal_idx = 0;
		}
	}
	else
	{
		/* Colour mask? */
	}
}

static VIDEO_START( jpmsys5v )
{
	tms34061_start(machine, &tms34061intf);
}

static VIDEO_UPDATE( jpmsys5v )
{
	int x, y;
	struct tms34061_display state;

	tms34061_get_display_state(&state);

	if (state.blanked)
	{
		bitmap_fill(bitmap, cliprect, get_black_pen(screen->machine));
		return 0;
	}

	for (y = cliprect->min_y; y <= cliprect->max_y; ++y)
	{
		UINT8 *src = &state.vram[(state.dispstart & 0xffff)*2 + 256 * y];
		UINT32 *dest = BITMAP_ADDR32(bitmap, y, cliprect->min_x);

		for (x = cliprect->min_x; x <= cliprect->max_x; x +=2)
		{
			UINT8 pen = src[(x-cliprect->min_x)>>1];

			/* Draw two 4-bit pixels */
			*dest++ = screen->machine->pens[(pen >> 4) & 0xf];
			*dest++ = screen->machine->pens[pen & 0xf];
		}
	}

	return 0;
}


/****************************************
 *
 *  General machine functions
 *
 ****************************************/

static WRITE16_HANDLER( rombank_w )
{
	UINT8 *rom = memory_region(space->machine, "maincpu");
	data &= 0x1f;
	memory_set_bankptr(space->machine, "bank1", &rom[0x20000 + 0x20000 * data]);
}

static READ16_HANDLER( coins_r )
{
	if (offset == 2)
		return input_port_read(space->machine, "COINS") << 8;
	else
		return 0xffff;
}

static WRITE16_HANDLER( coins_w )
{
	/* TODO */
}

static READ16_HANDLER( unk_r )
{
	return 0xffff;
}

static WRITE16_HANDLER( mux_w )
{
	/* TODO: Lamps! */
}

static READ16_HANDLER( mux_r )
{
	if (offset == 0x81/2)
		return input_port_read(space->machine, "DSW");

	return 0xffff;
}

static WRITE16_DEVICE_HANDLER( jpm_upd7759_w )
{
	if (offset == 0)
	{
		upd7759_port_w(device, 0, data & 0xff);
		upd7759_start_w(device, 0);
		upd7759_start_w(device, 1);
	}
	else if (offset == 2)
	{
		upd7759_reset_w(device, ~data & 0x4);
		upd7759_set_bank_base(device, (data & 2) ? 0x20000 : 0);
	}
	else
	{
		logerror("%s: upd7759: Unknown write to %x with %x\n", cpuexec_describe_context(device->machine),  offset, data);
	}
}

static READ16_DEVICE_HANDLER( jpm_upd7759_r )
{
	return 0x14 | upd7759_busy_r(device);
}


/*************************************
 *
 *  68000 CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( 68000_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x01ffff) AM_ROM
	AM_RANGE(0x01fffe, 0x01ffff) AM_WRITE(rombank_w)
	AM_RANGE(0x020000, 0x03ffff) AM_ROMBANK("bank1")
	AM_RANGE(0x040000, 0x043fff) AM_RAM AM_BASE_SIZE_GENERIC(nvram)
	AM_RANGE(0x046000, 0x046001) AM_WRITENOP
	AM_RANGE(0x046020, 0x046021) AM_DEVREADWRITE8("acia6850_0", acia6850_stat_r, acia6850_ctrl_w, 0xff)
	AM_RANGE(0x046022, 0x046023) AM_DEVREADWRITE8("acia6850_0", acia6850_data_r, acia6850_data_w, 0xff)
	AM_RANGE(0x046040, 0x04604f) AM_DEVREADWRITE8("6840ptm", ptm6840_read, ptm6840_write, 0xff)
	AM_RANGE(0x046060, 0x046061) AM_READ_PORT("DIRECT") AM_WRITENOP
	AM_RANGE(0x046062, 0x046063) AM_WRITENOP
	AM_RANGE(0x046064, 0x046065) AM_WRITENOP
	AM_RANGE(0x046066, 0x046067) AM_WRITENOP
	AM_RANGE(0x046080, 0x046081) AM_DEVREADWRITE8("acia6850_1", acia6850_stat_r, acia6850_ctrl_w, 0xff)
	AM_RANGE(0x046082, 0x046083) AM_DEVREADWRITE8("acia6850_1", acia6850_data_r, acia6850_data_w, 0xff)
	AM_RANGE(0x046084, 0x046085) AM_READ(unk_r) // PIA?
	AM_RANGE(0x046088, 0x046089) AM_READ(unk_r) // PIA?
	AM_RANGE(0x04608c, 0x04608d) AM_DEVREADWRITE8("acia6850_2", acia6850_stat_r, acia6850_ctrl_w, 0xff)
	AM_RANGE(0x04608e, 0x04608f) AM_DEVREADWRITE8("acia6850_2", acia6850_data_r, acia6850_data_w, 0xff)
	AM_RANGE(0x0460a0, 0x0460a3) AM_DEVWRITE8("ym2413", ym2413_w, 0x00ff)
	AM_RANGE(0x0460c0, 0x0460c1) AM_WRITENOP
	AM_RANGE(0x0460e0, 0x0460e5) AM_WRITE(ramdac_w)
	AM_RANGE(0x048000, 0x04801f) AM_READWRITE(coins_r, coins_w)
	AM_RANGE(0x04c000, 0x04c0ff) AM_READ(mux_r) AM_WRITE(mux_w)
	AM_RANGE(0x04c100, 0x04c105) AM_DEVREADWRITE("upd7759", jpm_upd7759_r, jpm_upd7759_w)
	AM_RANGE(0x800000, 0xcfffff) AM_READWRITE(sys5_tms34061_r, sys5_tms34061_w)
ADDRESS_MAP_END


 /*************************************
 *
 *  Touchscreen controller simulation
 *
 *************************************/

/* Serial bit transmission callback */
static TIMER_CALLBACK( touch_cb )
{
	switch (touch_state)
	{
		case IDLE:
		{
			break;
		}
		case START:
		{
			touch_shift_cnt = 0;
			a2_data_in = 0;
			touch_state = DATA;
			break;
		}
		case DATA:
		{
			a2_data_in = (touch_data[touch_data_count] >> (touch_shift_cnt)) & 1;

			if (++touch_shift_cnt == 8)
				touch_state = STOP1;

			break;
		}
		case STOP1:
		{
			a2_data_in = 1;
			touch_state = STOP2;
			break;
		}
		case STOP2:
		{
			a2_data_in = 1;

			if (++touch_data_count == 3)
			{
				timer_reset(touch_timer, attotime_never);
				touch_state = IDLE;
			}
			else
			{
				touch_state = START;
			}

			break;
		}
	}
}

static INPUT_CHANGED( touchscreen_press )
{
	if (newval == 0)
	{
		attotime rx_period = attotime_mul(ATTOTIME_IN_HZ(10000), 16);

		/* Each touch screen packet is 3 bytes */
		touch_data[0] = 0x2a;
		touch_data[1] = 0x7 - (input_port_read(field->port->machine, "TOUCH_Y") >> 5) + 0x30;
		touch_data[2] = (input_port_read(field->port->machine, "TOUCH_X") >> 5) + 0x30;

		/* Start sending the data to the 68000 serially */
		touch_data_count = 0;
		touch_state = START;
		timer_adjust_periodic(touch_timer, rx_period, 0, rx_period);
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
	PORT_DIPSETTING(	0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(	0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Alarm" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(	0x04, "Discontinue alarm when jam is cleared" )
	PORT_DIPSETTING(	0x00, "Continue alarm until back door open" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Payout Percentage" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(	0x00, "50%" )
	PORT_DIPSETTING(	0x80, "45%" )
	PORT_DIPSETTING(	0x40, "40%" )
	PORT_DIPSETTING(	0xc0, "30%" )

	PORT_START("DIRECT")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Back door") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Cash door") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Refill key") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_DIPNAME( 0x08, 0x08, DEF_STR ( Unknown ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR ( Unknown ) )
	PORT_DIPSETTING(	0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR ( Unknown ) )
	PORT_DIPSETTING(	0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Reset" ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(	0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR ( Unknown ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )

	PORT_START("COINS")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("10p")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20p")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("50p")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100p")
	PORT_BIT( 0xc3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TOUCH_PUSH")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED(touchscreen_press, NULL)

	PORT_START("TOUCH_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)

	PORT_START("TOUCH_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(45) PORT_KEYDELTA(15)
INPUT_PORTS_END


/*************************************
 *
 *  6840 PTM
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( ptm_irq )
{
	cputag_set_input_line(device->machine, "maincpu", INT_6840PTM, state ? ASSERT_LINE : CLEAR_LINE);
}

static const ptm6840_interface ptm_intf =
{
	1000000,
	{ 0, 0, 0 },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	DEVCB_LINE(ptm_irq)
};


/*************************************
 *
 *  6850 ACIAs
 *
 *************************************/

static WRITE_LINE_DEVICE_HANDLER( acia_irq )
{
	cputag_set_input_line(device->machine, "maincpu", INT_6850ACIA, state ? CLEAR_LINE : ASSERT_LINE);
}

/* Clocks are incorrect */

static READ_LINE_DEVICE_HANDLER( a0_rx_r )
{
	return a0_data_in;
}

static WRITE_LINE_DEVICE_HANDLER( a0_tx_w )
{
	a0_data_out = state;
}

static READ_LINE_DEVICE_HANDLER( a0_dcd_r )
{
	return a0_acia_dcd;
}

static ACIA6850_INTERFACE( acia0_if )
{
	10000,
	10000,
	DEVCB_LINE(a0_rx_r), /*&a0_data_in,*/
	DEVCB_LINE(a0_tx_w), /*&a0_data_out,*/
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(a0_dcd_r), /*&a0_acia_dcd,*/
	DEVCB_LINE(acia_irq)
};

static READ_LINE_DEVICE_HANDLER( a1_rx_r )
{
	return a1_data_in;
}

static WRITE_LINE_DEVICE_HANDLER( a1_tx_w )
{
	a1_data_out = state;
}

static READ_LINE_DEVICE_HANDLER( a1_dcd_r )
{
	return a1_acia_dcd;
}

static ACIA6850_INTERFACE( acia1_if )
{
	10000,
	10000,
	DEVCB_LINE(a1_rx_r), /*&a1_data_in,*/
	DEVCB_LINE(a1_tx_w), /*&a1_data_out,*/
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(a1_dcd_r), /*&a1_acia_dcd,*/
	DEVCB_LINE(acia_irq)
};

static READ_LINE_DEVICE_HANDLER( a2_rx_r )
{
	return a2_data_in;
}

static WRITE_LINE_DEVICE_HANDLER( a2_tx_w )
{
	a2_data_out = state;
}

static READ_LINE_DEVICE_HANDLER( a2_dcd_r )
{
	return a2_acia_dcd;
}

static ACIA6850_INTERFACE( acia2_if )
{
	10000,
	10000,
	DEVCB_LINE(a2_rx_r), /*&a2_data_in,*/
	DEVCB_LINE(a2_tx_w), /*&a2_data_out,*/
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_LINE(a2_dcd_r), /*&a2_acia_dcd,*/
	DEVCB_LINE(acia_irq)
};


/*************************************
 *
 *  Initialisation
 *
 *************************************/

static MACHINE_START( jpmsys5v )
{
	memory_set_bankptr(machine, "bank1", memory_region(machine, "maincpu"));
	touch_timer = timer_alloc(machine, touch_cb, NULL);
}

static MACHINE_RESET( jpmsys5v )
{
	timer_reset(touch_timer, attotime_never);
	touch_state = IDLE;
	a2_data_in = 1;
	a2_acia_dcd = 0;
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_DRIVER_START( jpmsys5v )
	MDRV_CPU_ADD("maincpu", M68000, 8000000)
	MDRV_CPU_PROGRAM_MAP(68000_map)

	MDRV_ACIA6850_ADD("acia6850_0", acia0_if)
	MDRV_ACIA6850_ADD("acia6850_1", acia1_if)
	MDRV_ACIA6850_ADD("acia6850_2", acia2_if)

	MDRV_NVRAM_HANDLER(generic_0fill)

	MDRV_MACHINE_START(jpmsys5v)
	MDRV_MACHINE_RESET(jpmsys5v)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_RGB32)
	MDRV_SCREEN_RAW_PARAMS(XTAL_40MHz / 4, 676, 20*4, 147*4, 256, 0, 254)

	MDRV_VIDEO_START(jpmsys5v)
	MDRV_VIDEO_UPDATE(jpmsys5v)

	MDRV_PALETTE_LENGTH(16)

	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("upd7759", UPD7759, UPD7759_STANDARD_CLOCK)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)

	/* Earlier revisions use an SAA1099 */
	MDRV_SOUND_ADD("ym2413", YM2413, 4000000 ) /* Unconfirmed */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* 6840 PTM */
	MDRV_PTM6840_ADD("6840ptm", ptm_intf)
MACHINE_DRIVER_END


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


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1994, monopoly, 0,        jpmsys5v, monopoly, 0, ROT0, "JPM", "Monopoly",         0 )
GAME( 1995, monoplcl, monopoly, jpmsys5v, monopoly, 0, ROT0, "JPM", "Monopoly Classic", 0 )
GAME( 1995, monopldx, 0,        jpmsys5v, monopoly, 0, ROT0, "JPM", "Monopoly Deluxe",  0 )

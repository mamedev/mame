// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    JPM Give us a Break hardware

    preliminary driver by Phil Bennett

    Games supported:
        * Give us a Break [8 sets]
        * Criss Cross (Sweden) [non-working - need disk image]
        * Ten Up [2 sets]

    Looking for:
        * Numbers Game
        * Pac Quiz
        * Suit Pursuit
        * Treasure Trail?

    Known issues:
        * Neither game registers coins and I can't find where the credit
        count gets updated in the code. Each game requires a unique
        security PAL - maybe this is related? I'm poking the coin values
        directly into RAM for now.
        * Verify WD FDC type

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/6840ptm.h"
#include "video/tms34061.h"
#include "sound/sn76496.h"
#include "machine/wd_fdc.h"
#include "formats/guab_dsk.h"


/*************************************
 *
 *  Defines
 *
 *************************************/


enum int_levels
{
	INT_UNKNOWN1     = 1,
	INT_UNKNOWN2     = 2,
	INT_6840PTM      = 3,
	INT_6850ACIA     = 4,
	INT_TMS34061     = 5,
	INT_FLOPPYCTRL   = 6,
	INT_WATCHDOG_INT = 7
};


struct ef9369
{
	UINT32 addr;
	UINT16 clut[16];    /* 13-bits - a marking bit and a 444 color */
};


class guab_state : public driver_device
{
public:
	guab_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms34061(*this, "tms34061"),
		m_sn(*this, "snsnd"),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:0"),
		m_palette(*this, "palette") { }

	DECLARE_WRITE_LINE_MEMBER(generate_tms34061_interrupt);
	DECLARE_WRITE16_MEMBER(guab_tms34061_w);
	DECLARE_READ16_MEMBER(guab_tms34061_r);
	DECLARE_WRITE16_MEMBER(ef9369_w);
	DECLARE_READ16_MEMBER(ef9369_r);
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_WRITE16_MEMBER(io_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_WRITE_LINE_MEMBER(ptm_irq);
	UINT32 screen_update_guab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

protected:
	virtual void machine_start();

private:
	required_device<cpu_device> m_maincpu;
	required_device<tms34061_device> m_tms34061;
	required_device<sn76489_device> m_sn;
	required_device<wd1773_t> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<palette_device> m_palette;

	struct ef9369 m_pal;
};


/*************************************
 *
 *  6840 PTM
 *
 *************************************/

WRITE_LINE_MEMBER(guab_state::ptm_irq)
{
	m_maincpu->set_input_line(INT_6840PTM, state);
}

/*************************************
 *
 *  Video hardware
 *
 *************************************/

/*****************
 * TMS34061 CRTC
 *****************/

WRITE_LINE_MEMBER(guab_state::generate_tms34061_interrupt)
{
	m_maincpu->set_input_line(INT_TMS34061, state);
}

WRITE16_MEMBER(guab_state::guab_tms34061_w)
{
	int func = (offset >> 19) & 3;
	int row = (offset >> 7) & 0xff;
	int col;

	if (func == 0 || func == 2)
		col = offset  & 0xff;
	else
		col = offset <<= 1;

	if (ACCESSING_BITS_8_15)
		m_tms34061->write(space, col, row, func, data >> 8);

	if (ACCESSING_BITS_0_7)
		m_tms34061->write(space, col | 1, row, func, data & 0xff);
}


READ16_MEMBER(guab_state::guab_tms34061_r)
{
	UINT16 data = 0;
	int func = (offset >> 19) & 3;
	int row = (offset >> 7) & 0xff;
	int col;

	if (func == 0 || func == 2)
		col = offset  & 0xff;
	else
		col = offset <<= 1;

	if (ACCESSING_BITS_8_15)
		data |= m_tms34061->read(space, col, row, func) << 8;

	if (ACCESSING_BITS_0_7)
		data |= m_tms34061->read(space, col | 1, row, func);

	return data;
}


/****************************
 *  EF9369 color palette IC
 *  (16 colors from 4096)
 ****************************/

/* Non-multiplexed mode */
WRITE16_MEMBER(guab_state::ef9369_w)
{
	struct ef9369 &pal = m_pal;
	data &= 0x00ff;

	/* Address register */
	if (offset & 1)
	{
		pal.addr = data & 0x1f;
	}
	/* Data register */
	else
	{
		UINT32 entry = pal.addr >> 1;

		if ((pal.addr & 1) == 0)
		{
			pal.clut[entry] &= ~0x00ff;
			pal.clut[entry] |= data;
		}
		else
		{
			UINT16 col;

			pal.clut[entry] &= ~0x1f00;
			pal.clut[entry] |= (data & 0x1f) << 8;

			/* Remove the marking bit */
			col = pal.clut[entry] & 0xfff;

			/* Update the MAME palette */
			m_palette->set_pen_color(entry, pal4bit(col >> 0), pal4bit(col >> 4), pal4bit(col >> 8));
		}

			/* Address register auto-increment */
		if (++pal.addr == 32)
			pal.addr = 0;
	}
}

READ16_MEMBER(guab_state::ef9369_r)
{
	struct ef9369 &pal = m_pal;
	if ((offset & 1) == 0)
	{
		UINT16 col = pal.clut[pal.addr >> 1];

		if ((pal.addr & 1) == 0)
			return col & 0xff;
		else
			return col >> 8;
	}
	else
	{
		/* Address register is write only */
		return 0xffff;
	}
}

UINT32 guab_state::screen_update_guab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y;

	m_tms34061->get_display_state();

	/* If blanked, fill with black */
	if (m_tms34061->m_display.blanked)
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	for (y = cliprect.min_y; y <= cliprect.max_y; ++y)
	{
		UINT8 *src = &m_tms34061->m_display.vram[256 * y];
		UINT16 *dest = &bitmap.pix16(y);

		for (x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			UINT8 pen = src[x >> 1];

			/* Draw two 4-bit pixels */
			*dest++ = m_palette->pen(pen >> 4);
			*dest++ = m_palette->pen(pen & 0x0f);
		}
	}

	return 0;
}


/****************************************
 *
 *  Hardware inputs (coins, buttons etc)
 *
 ****************************************/

READ16_MEMBER(guab_state::io_r)
{
	static const char *const portnames[] = { "IN0", "IN1", "IN2" };

	switch (offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		{
			return ioport(portnames[offset])->read();
		}
		case 0x30:
		{
			/* Whatever it is, bit 7 must be 0 */
			return 0x7f;
		}
		default:
		{
			osd_printf_debug("Unknown IO R:0x%x\n", 0xc0000 + (offset * 2));
			return 0;
		}
	}
}

INPUT_CHANGED_MEMBER(guab_state::coin_inserted)
{
	if (newval == 0)
	{
		UINT32 credit;
		address_space &space = m_maincpu->space(AS_PROGRAM);

		/* Get the current credit value and add the new coin value */
		credit = space.read_dword(0x8002c) + (UINT32)(FPTR)param;
		space.write_dword(0x8002c, credit);
	}
}

/****************************************
 *
 *  Hardware outputs (lamps, meters etc)
 *
 ****************************************/

WRITE16_MEMBER(guab_state::io_w)
{
	switch (offset)
	{
		case 0x10:
		{
			/* Outputs 0 - 7 */
			break;
		}
		case 0x11:
		{
			/* Outputs 8 - 15 */
			break;
		}
		case 0x12:
		{
			/* Outputs 16 - 23 */
			break;
		}
		case 0x20:
		{
			/* Outputs 24 - 31 */
			break;
		}
		case 0x21:
		{
			/* Outputs 32 - 39 */
			break;
		}
		case 0x22:
		{
			/* Outputs 40 - 47 */
			break;
		}
		case 0x30:
		{
			m_sn->write(space, 0, data & 0xff);
			break;
		}
		case 0x31:
		{
			/* Only JPM knows about the other bits... */
			m_floppy->get_device()->ss_w(BIT(data, 3));

			// one of those bits will probably control the motor, we just let it run all the time for now
			m_floppy->get_device()->mon_w(0);
			break;
		}
		case 0x32:
		{
			/* Watchdog? */
			break;
		}
		case 0x33:
		{
			/* Dunno */
			break;
		}
		default:
		{
			osd_printf_debug("Unknown IO W:0x%x with %x\n", 0xc0000 + (offset * 2), data);
		}
	}
}


/*************************************
 *
 *  68000 CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( guab_map, AS_PROGRAM, 16, guab_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x040000, 0x04ffff) AM_ROM AM_REGION("maincpu", 0x10000)
	AM_RANGE(0x0c0000, 0x0c007f) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x0c0080, 0x0c0083) AM_NOP /* ACIA 1 */
	AM_RANGE(0x0c00a0, 0x0c00a3) AM_NOP /* ACIA 2 */
	AM_RANGE(0x0c00c0, 0x0c00cf) AM_DEVREADWRITE8("6840ptm", ptm6840_device, read, write, 0xff)
	AM_RANGE(0x0c00e0, 0x0c00e7) AM_DEVREADWRITE8("fdc", wd1773_t, read, write, 0x00ff)
	AM_RANGE(0x080000, 0x080fff) AM_RAM
	AM_RANGE(0x100000, 0x100003) AM_READWRITE(ef9369_r, ef9369_w)
	AM_RANGE(0x800000, 0xb0ffff) AM_READWRITE(guab_tms34061_r, guab_tms34061_w)
	AM_RANGE(0xb10000, 0xb1ffff) AM_RAM
	AM_RANGE(0xb80000, 0xb8ffff) AM_RAM
	AM_RANGE(0xb90000, 0xb9ffff) AM_RAM
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( guab )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("50p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)50)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )   PORT_NAME("100p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)100)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Back door") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Cash door") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key switch") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_UNUSED )  PORT_NAME("50p level")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )  PORT_NAME("100p level")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Select")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("D")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("10p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)10)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("20p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)20)
INPUT_PORTS_END

static INPUT_PORTS_START( tenup )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )   PORT_NAME("50p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)50)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN4 )   PORT_NAME("100p") PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)100)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Back door") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Cash door") PORT_CODE(KEYCODE_T) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Key switch") PORT_CODE(KEYCODE_Y) PORT_TOGGLE
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_UNUSED )  PORT_NAME("10p level")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH,IPT_UNUSED )  PORT_NAME("100p level")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH,IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Pass")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Collect")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )   PORT_NAME("10p")  PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)10)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )   PORT_NAME("20p")  PORT_CHANGED_MEMBER(DEVICE_SELF, guab_state,coin_inserted, (void *)20)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void guab_state::machine_start()
{
	m_fdc->set_floppy(m_floppy->get_device());
}

FLOPPY_FORMATS_MEMBER( guab_state::floppy_formats )
	FLOPPY_GUAB_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( guab_floppies )
	SLOT_INTERFACE("dd", FLOPPY_35_DD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( guab, guab_state )
	/* TODO: Verify clock */
	MCFG_CPU_ADD("maincpu", M68000, 8000000)
	MCFG_CPU_PROGRAM_MAP(guab_map)

	/* TODO: Use real video timings */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 64*8-1, 0, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(guab_state, screen_update_guab)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)

	MCFG_DEVICE_ADD("tms34061", TMS34061, 0)
	MCFG_TMS34061_ROWSHIFT(8)  /* VRAM address is (row << rowshift) | col */
	MCFG_TMS34061_VRAM_SIZE(0x40000) /* size of video RAM */
	MCFG_TMS34061_INTERRUPT_CB(WRITELINE(guab_state, generate_tms34061_interrupt))      /* interrupt gen callback */

	MCFG_SPEAKER_STANDARD_MONO("mono")

	/* TODO: Verify clock */
	MCFG_SOUND_ADD("snsnd", SN76489, 2000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* 6840 PTM */
	MCFG_DEVICE_ADD("6840ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(1000000)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_IRQ_CB(WRITELINE(guab_state, ptm_irq))

	// floppy
	MCFG_WD1773_ADD("fdc", 8000000)
	MCFG_WD_FDC_DRQ_CALLBACK(INPUTLINE("maincpu", INT_FLOPPYCTRL))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", guab_floppies, "dd", guab_state::floppy_formats)
	MCFG_SLOT_FIXED(true)

	MCFG_SOFTWARE_LIST_ADD("floppy_list", "guab")
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( guab )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guab1a1.rom", 0x00000, 0x8000, CRC(f23a9d7d) SHA1(f933e131bdcf21cfa6001c8e20fd11d94c7a9450) )
	ROM_LOAD16_BYTE( "guab1b1.rom", 0x00001, 0x8000, CRC(af3b5492) SHA1(6fd7f29e6ed2fadccc9246f1ebd049c3f9aeff13) )
	ROM_LOAD16_BYTE( "guab2a1.rom", 0x10000, 0x8000, CRC(ae7a162c) SHA1(d69721818b8e4daba776a678b62bc7f44f371a3f) )
	ROM_LOAD16_BYTE( "guab2b1.rom", 0x10001, 0x8000, CRC(29aa26a0) SHA1(8d425ad845ccfcd8995dbf6adc1ca17989a5d3ea) )
ROM_END

ROM_START( crisscrs )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "crisscross_swe_2a1.ic49", 0x00000, 0x8000, CRC(a7ca8828) SHA1(a28482bb2bc1248a9b5c0b57904c382246a632cc) )
	ROM_LOAD16_BYTE( "crisscross_swe_2b1.ic48", 0x00001, 0x8000, CRC(7e280cae) SHA1(18c76459e39549ddba5f0cd7921013ef4f816826) )
ROM_END

ROM_START( tenup )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tu-11.bin", 0x00000, 0x8000, CRC(01843086) SHA1(106a226900e8cf929f89edf801c627f02e4afce3) )
	ROM_LOAD16_BYTE( "tu-12.bin", 0x00001, 0x8000, CRC(1c7f32b1) SHA1(2b14e2206695ae53909ae838a5c036248d9ab940) )
	ROM_LOAD16_BYTE( "tu-13.bin", 0x10000, 0x8000, CRC(d19e2bf7) SHA1(76a9cbd4f604ad39eb0e319a9a6d5a6739b0ed8c) )
	ROM_LOAD16_BYTE( "tu-14.bin", 0x10001, 0x8000, CRC(fd8a0c3c) SHA1(f87289ce6f0d2bc9b7d3a0b6deff38ba3aadf391) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, guab,     0, guab, guab,  driver_device, 0, ROT0, "JPM", "Give us a Break",      0 )
GAME( 1986, crisscrs, 0, guab, guab,  driver_device, 0, ROT0, "JPM", "Criss Cross (Sweden)", GAME_NOT_WORKING )
GAME( 1988, tenup,    0, guab, tenup, driver_device, 0, ROT0, "JPM", "Ten Up",               0 )

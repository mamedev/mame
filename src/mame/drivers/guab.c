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

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/6840ptm.h"
#include "video/tms34061.h"
#include "sound/sn76496.h"


/*************************************
 *
 *  Defines
 *
 *************************************/

#define LOG_FDC_COMMANDS    0
#define FDC_LOG(x) do { if (LOG_FDC_COMMANDS) osd_printf_debug x; } while(0)


enum int_levels
{
	INT_UNKNOWN1     = 1,
	INT_UNKNOWN2     = 2,
	INT_6840PTM      = 3,
	INT_6850ACIA     = 4,
	INT_TMS34061     = 5,
	INT_FLOPPYCTRL   = 6,
	INT_WATCHDOG_INT = 7,
};


struct ef9369
{
	UINT32 addr;
	UINT16 clut[16];    /* 13-bits - a marking bit and a 444 color */
};


struct wd1770
{
	UINT32  status;
	UINT8   cmd;
	UINT8   data;

	UINT32  side;
	INT32   track;
	INT32   sector;
	UINT32  sptr;
};


class guab_state : public driver_device
{
public:
	guab_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tms34061(*this, "tms34061"),
		m_sn(*this, "snsnd"),
		m_palette(*this, "palette") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<tms34061_device> m_tms34061;
	required_device<sn76489_device> m_sn;
	required_device<palette_device> m_palette;

	struct ef9369 m_pal;
	emu_timer *m_fdc_timer;
	struct wd1770 m_fdc;
	DECLARE_WRITE_LINE_MEMBER(generate_tms34061_interrupt);
	DECLARE_WRITE16_MEMBER(guab_tms34061_w);
	DECLARE_READ16_MEMBER(guab_tms34061_r);
	DECLARE_WRITE16_MEMBER(ef9369_w);
	DECLARE_READ16_MEMBER(ef9369_r);
	DECLARE_WRITE16_MEMBER(wd1770_w);
	DECLARE_READ16_MEMBER(wd1770_r);
	DECLARE_READ16_MEMBER(io_r);
	DECLARE_WRITE16_MEMBER(io_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_WRITE_LINE_MEMBER(ptm_irq);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_guab(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(fdc_data_callback);
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


/*************************************
 *
 *  WD1770 floppy disk controller
 *  (just enough to get by...)
 *
 *************************************/

#define USEC_DELAY          40
#define DISK_SIDES          2
#define DISK_TRACKS         80
#define DISK_SECTORS        18

#define DISK_SECTOR_SIZE    256
#define DISK_TRACK_SIZE     (DISK_SECTOR_SIZE * DISK_SECTORS)
#define DISK_SIDE_SIZE      (DISK_TRACK_SIZE * DISK_TRACKS)

enum wd1770_status
{
	BUSY             = 1 << 0,
	DATA_REQUEST     = 1 << 1,
	INDEX            = 1 << 1,
	LOST_DATA        = 1 << 2,
	TRACK_00         = 1 << 2,
	CRC_ERROR        = 1 << 3,
	RECORD_NOT_FOUND = 1 << 4,
	RECORD_TYPE      = 1 << 5,
	SPIN_UP          = 1 << 5,
	WRITE_PROTECT    = 1 << 6,
	MOTOR_ON         = 1 << 7
};


TIMER_CALLBACK_MEMBER(guab_state::fdc_data_callback)
{
	struct wd1770 &fdc = m_fdc;
	UINT8* disk = (UINT8*)memregion("user1")->base();
	int more_data = 0;

	/*
	    Disk dumps are organised as:

	    Side 0, Track 0: Sectors 0 - 17
	    Side 1, Track 0: Sectors 0 - 17
	    Side 0, Track 1: Sectors 0 - 17
	    Side 1, Track 1: Sectors 0 - 17
	    etc.
	*/

	int idx = 2 * fdc.track * (DISK_TRACK_SIZE) + (fdc.side ? DISK_TRACK_SIZE : 0)+
				fdc.sector * (DISK_SECTOR_SIZE) +
				fdc.sptr;

	/* Write or read? */
	if (fdc.cmd & 0x20)
		disk[idx] = fdc.data;
	else
		fdc.data = disk[idx];

	fdc.sptr++;

	if (fdc.sptr < DISK_SECTOR_SIZE)
	{
		more_data = 1;
	}
	else
	{
		fdc.sptr = 0;

		/* Multi-sector read */
		if (fdc.cmd & 0x10)
		{
			++fdc.sector;

			/* Keep going */
			if (fdc.sector < DISK_SECTORS)
				more_data = 1;
		}
	}

	if (more_data)
	{
		m_fdc_timer->adjust(attotime::from_usec(USEC_DELAY));
	}
	else
	{
		/* Hack!*/
		fdc.cmd = 0;
		fdc.status &= ~BUSY;
	}

	fdc.status |= DATA_REQUEST;
	m_maincpu->set_input_line(INT_FLOPPYCTRL, ASSERT_LINE);
}


WRITE16_MEMBER(guab_state::wd1770_w)
{
	struct wd1770 &fdc = m_fdc;
	data &= 0xff;

	switch (offset)
	{
		case 0:
		{
			UINT8 cmd = (data >> 4) & 0xf;

			FDC_LOG(("Command: %x:: ", data));

			switch (cmd)
			{
				/**** Type I ****/
				case 0:
				{
					FDC_LOG(("Restore\n"));
					fdc.track = 0;
					break;
				}

				case 1:
				{
					FDC_LOG(("Seek to track %d\n", fdc.data));
					fdc.track = fdc.data;
					break;
				}

				/* Step */
				case 2:
				case 3:
				{
					FDC_LOG(("Step (not implemented)\n"));
					break;
				}

				case 4: case 5:
				{
					if (++fdc.track >= DISK_TRACKS)
						fdc.track = DISK_TRACKS - 1;

					FDC_LOG(("Step-in to track %d\n", fdc.track));
					break;
				}

				case 6: case 7:
				{
					if (--fdc.track > 0)
						fdc.track = 0;

					FDC_LOG(("Step-out to track %d\n", fdc.track));
					break;
				}


				/**** Type II ****/
				case 8:
				case 9:
				{
					fdc.cmd = data;
					fdc.status |= BUSY;

					if (data & 0x10)
						FDC_LOG(("Multi "));

					FDC_LOG(("Sector read: H%d T%d S%d\n",  fdc.side,
															fdc.track,
															fdc.sector));

					/* Set the data read timer */
					m_fdc_timer->adjust(attotime::from_usec(USEC_DELAY));

					break;
				}

				case 10:
				case 11:
				{
					fdc.cmd = data;
					fdc.status |= BUSY;

					if (data & 0x10)
						FDC_LOG(("Multi "));

					FDC_LOG(("Sector write: H%d T%d S%d\n", fdc.side,
															fdc.track,
															fdc.sector));

					/* Trigger a DRQ interrupt on the CPU */
					m_maincpu->set_input_line(INT_FLOPPYCTRL, ASSERT_LINE);
					fdc.status |= DATA_REQUEST;
					break;
				}

				/**** Type III ****/
				case 12: /* Read address */
				case 14: /* Read track */
				case 15: /* Write track */
				{
					FDC_LOG(("Unsupported TYPE III operation\n"));
					break;
				}

				/**** Type IV ****/
				case 13:
				{
					/* Stop any operation in progress */
					m_fdc_timer->reset();
					fdc.status &= ~BUSY;
					FDC_LOG(("Force Interrupt\n"));
					break;
				}
			}

			break;
		}
		case 1:
		{
			FDC_LOG(("Set track: %d\n", data));
			fdc.track = data;
			break;
		}
		case 2:
		{
			FDC_LOG(("Set sector: %d\n", data));
			fdc.sector = data;
			fdc.sptr = 0;
			break;
		}
		case 3:
		{
			fdc.data = data;

			/* Clear the DRQ */
			m_maincpu->set_input_line(INT_FLOPPYCTRL, CLEAR_LINE);

			/* Queue an event to write the data if write command was specified */
			if (fdc.cmd & 0x20)
				m_fdc_timer->adjust(attotime::from_usec(USEC_DELAY));

			break;
		}
	}
}

READ16_MEMBER(guab_state::wd1770_r)
{
	struct wd1770 &fdc = m_fdc;
	UINT16 retval = 0;

	switch (offset)
	{
		case 0:
		{
			retval = fdc.status;
			break;
		}
		case 1:
		{
			retval = fdc.track;
			break;
		}
		case 2:
		{
			retval = fdc.sector;
			break;
		}
		case 3:
		{
			retval = fdc.data;

			/* Clear the DRQ */
			m_maincpu->set_input_line(INT_FLOPPYCTRL, CLEAR_LINE);
			fdc.status &= ~DATA_REQUEST;
			break;
		}
	}

	return retval;
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
	struct wd1770 &fdc = m_fdc;
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
			fdc.side = (data >> 3) & 1;
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
	AM_RANGE(0x0c00e0, 0x0c00e7) AM_READWRITE(wd1770_r, wd1770_w)
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
	m_fdc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(guab_state::fdc_data_callback),this));
}

void guab_state::machine_reset()
{
	memset(&m_fdc, 0, sizeof(m_fdc));
}

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

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "guab3.dsk", 0x00000, 0xb4000, CRC(ecb41e06) SHA1(2900a95046b38312c6035ea394b04b62c1d29f42) )
ROM_END

ROM_START( guab3a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guab1a1.rom", 0x00000, 0x8000, CRC(f23a9d7d) SHA1(f933e131bdcf21cfa6001c8e20fd11d94c7a9450) )
	ROM_LOAD16_BYTE( "guab1b1.rom", 0x00001, 0x8000, CRC(af3b5492) SHA1(6fd7f29e6ed2fadccc9246f1ebd049c3f9aeff13) )
	ROM_LOAD16_BYTE( "guab2a1.rom", 0x10000, 0x8000, CRC(ae7a162c) SHA1(d69721818b8e4daba776a678b62bc7f44f371a3f) )
	ROM_LOAD16_BYTE( "guab2b1.rom", 0x10001, 0x8000, CRC(29aa26a0) SHA1(8d425ad845ccfcd8995dbf6adc1ca17989a5d3ea) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "guab7c.dsk", 0x00000, 0xb4000, CRC(f5f10f87) SHA1(80e540339efbfe59b656d6cd4f466a17df84c123) )
ROM_END

ROM_START( guab4 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guab1a1.rom", 0x00000, 0x8000, CRC(f23a9d7d) SHA1(f933e131bdcf21cfa6001c8e20fd11d94c7a9450) )
	ROM_LOAD16_BYTE( "guab1b1.rom", 0x00001, 0x8000, CRC(af3b5492) SHA1(6fd7f29e6ed2fadccc9246f1ebd049c3f9aeff13) )
	ROM_LOAD16_BYTE( "guab2a1.rom", 0x10000, 0x8000, CRC(ae7a162c) SHA1(d69721818b8e4daba776a678b62bc7f44f371a3f) )
	ROM_LOAD16_BYTE( "guab2b1.rom", 0x10001, 0x8000, CRC(29aa26a0) SHA1(8d425ad845ccfcd8995dbf6adc1ca17989a5d3ea) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "guab8d.dsk", 0x00000, 0xb4000, CRC(b87c55ce) SHA1(20debbefae194276b0813518634cf52bed093e73) )
ROM_END

ROM_START( guab6 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guab1a1.rom", 0x00000, 0x8000, CRC(f23a9d7d) SHA1(f933e131bdcf21cfa6001c8e20fd11d94c7a9450) )
	ROM_LOAD16_BYTE( "guab1b1.rom", 0x00001, 0x8000, CRC(af3b5492) SHA1(6fd7f29e6ed2fadccc9246f1ebd049c3f9aeff13) )
	ROM_LOAD16_BYTE( "guab2a1.rom", 0x10000, 0x8000, CRC(ae7a162c) SHA1(d69721818b8e4daba776a678b62bc7f44f371a3f) )
	ROM_LOAD16_BYTE( "guab2b1.rom", 0x10001, 0x8000, CRC(29aa26a0) SHA1(8d425ad845ccfcd8995dbf6adc1ca17989a5d3ea) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "guabf6.dsk", 0x00000, 0xb4000, CRC(08804c28) SHA1(608d89d598b7acb133814540dba98cea29c6cad6) )
ROM_END

ROM_START( guab6a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guab1a1.rom", 0x00000, 0x8000, CRC(f23a9d7d) SHA1(f933e131bdcf21cfa6001c8e20fd11d94c7a9450) )
	ROM_LOAD16_BYTE( "guab1b1.rom", 0x00001, 0x8000, CRC(af3b5492) SHA1(6fd7f29e6ed2fadccc9246f1ebd049c3f9aeff13) )
	ROM_LOAD16_BYTE( "guab2a1.rom", 0x10000, 0x8000, CRC(ae7a162c) SHA1(d69721818b8e4daba776a678b62bc7f44f371a3f) )
	ROM_LOAD16_BYTE( "guab2b1.rom", 0x10001, 0x8000, CRC(29aa26a0) SHA1(8d425ad845ccfcd8995dbf6adc1ca17989a5d3ea) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "guab9f.dsk", 0x00000, 0xb4000, CRC(85329fe9) SHA1(f5ca1956c37b786fa0a4f8f0607a0a2b19d3fbb0) )
ROM_END

ROM_START( guab7 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guab1a1.rom", 0x00000, 0x8000, CRC(f23a9d7d) SHA1(f933e131bdcf21cfa6001c8e20fd11d94c7a9450) )
	ROM_LOAD16_BYTE( "guab1b1.rom", 0x00001, 0x8000, CRC(af3b5492) SHA1(6fd7f29e6ed2fadccc9246f1ebd049c3f9aeff13) )
	ROM_LOAD16_BYTE( "guab2a1.rom", 0x10000, 0x8000, CRC(ae7a162c) SHA1(d69721818b8e4daba776a678b62bc7f44f371a3f) )
	ROM_LOAD16_BYTE( "guab2b1.rom", 0x10001, 0x8000, CRC(29aa26a0) SHA1(8d425ad845ccfcd8995dbf6adc1ca17989a5d3ea) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "guab9g.dsk", 0x00000, 0xb4000, CRC(6b8c36f9) SHA1(1d9f4d943f2962603bbe39e5b58befe15954a6d6) )
ROM_END

ROM_START( guab21 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guab1a1.rom", 0x00000, 0x8000, CRC(f23a9d7d) SHA1(f933e131bdcf21cfa6001c8e20fd11d94c7a9450) )
	ROM_LOAD16_BYTE( "guab1b1.rom", 0x00001, 0x8000, CRC(af3b5492) SHA1(6fd7f29e6ed2fadccc9246f1ebd049c3f9aeff13) )
	ROM_LOAD16_BYTE( "guab2a1.rom", 0x10000, 0x8000, CRC(ae7a162c) SHA1(d69721818b8e4daba776a678b62bc7f44f371a3f) )
	ROM_LOAD16_BYTE( "guab2b1.rom", 0x10001, 0x8000, CRC(29aa26a0) SHA1(8d425ad845ccfcd8995dbf6adc1ca17989a5d3ea) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "guab21.dsk", 0x00000, 0xb4000, CRC(558ee009) SHA1(d2a02662216e10f80a215044f1929df7255b6136) )
ROM_END

ROM_START( guab43 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "guab1a1.rom", 0x00000, 0x8000, CRC(f23a9d7d) SHA1(f933e131bdcf21cfa6001c8e20fd11d94c7a9450) )
	ROM_LOAD16_BYTE( "guab1b1.rom", 0x00001, 0x8000, CRC(af3b5492) SHA1(6fd7f29e6ed2fadccc9246f1ebd049c3f9aeff13) )
	ROM_LOAD16_BYTE( "guab2a1.rom", 0x10000, 0x8000, CRC(ae7a162c) SHA1(d69721818b8e4daba776a678b62bc7f44f371a3f) )
	ROM_LOAD16_BYTE( "guab2b1.rom", 0x10001, 0x8000, CRC(29aa26a0) SHA1(8d425ad845ccfcd8995dbf6adc1ca17989a5d3ea) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "guab43.dsk", 0x00000, 0xb3f00, CRC(96167357) SHA1(9d5f66e05ed1086be9699c4149071038f6986e0c) )
ROM_END

ROM_START( crisscrs )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "crisscross_swe_2a1.ic49", 0x00000, 0x8000, CRC(a7ca8828) SHA1(a28482bb2bc1248a9b5c0b57904c382246a632cc) )
	ROM_LOAD16_BYTE( "crisscross_swe_2b1.ic48", 0x00001, 0x8000, CRC(7e280cae) SHA1(18c76459e39549ddba5f0cd7921013ef4f816826) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "crisscrs.dsk", 0x00000, 0xb4000, NO_DUMP )
ROM_END

ROM_START( tenup )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tu-11.bin", 0x00000, 0x8000, CRC(01843086) SHA1(106a226900e8cf929f89edf801c627f02e4afce3) )
	ROM_LOAD16_BYTE( "tu-12.bin", 0x00001, 0x8000, CRC(1c7f32b1) SHA1(2b14e2206695ae53909ae838a5c036248d9ab940) )
	ROM_LOAD16_BYTE( "tu-13.bin", 0x10000, 0x8000, CRC(d19e2bf7) SHA1(76a9cbd4f604ad39eb0e319a9a6d5a6739b0ed8c) )
	ROM_LOAD16_BYTE( "tu-14.bin", 0x10001, 0x8000, CRC(fd8a0c3c) SHA1(f87289ce6f0d2bc9b7d3a0b6deff38ba3aadf391) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "10up17.dsk", 0x00000, 0xb4000, CRC(7bc328df) SHA1(5f8e40d8ffc370fb19be9e386befa5fcd1f35a75) )
ROM_END

ROM_START( tenup3 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "tu-11.bin", 0x00000, 0x8000, CRC(01843086) SHA1(106a226900e8cf929f89edf801c627f02e4afce3) )
	ROM_LOAD16_BYTE( "tu-12.bin", 0x00001, 0x8000, CRC(1c7f32b1) SHA1(2b14e2206695ae53909ae838a5c036248d9ab940) )
	ROM_LOAD16_BYTE( "tu-13.bin", 0x10000, 0x8000, CRC(d19e2bf7) SHA1(76a9cbd4f604ad39eb0e319a9a6d5a6739b0ed8c) )
	ROM_LOAD16_BYTE( "tu-14.bin", 0x10001, 0x8000, CRC(fd8a0c3c) SHA1(f87289ce6f0d2bc9b7d3a0b6deff38ba3aadf391) )

	ROM_REGION( 0xb4000, "user1", 0 )
	ROM_LOAD( "10up3.dsk", 0x00000, 0xb4000, CRC(2767f017) SHA1(1c6551b089c3e3df48e0c03bd502b91fd88f0e94) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, guab,     0,     guab, guab, driver_device,  0, ROT0, "JPM", "Give us a Break (3rd edition)",      0 )
GAME( 1986, guab3a,   guab,  guab, guab, driver_device,  0, ROT0, "JPM", "Give us a Break (3rd edition alt?)", 0 )
GAME( 1986, guab4,    guab,  guab, guab, driver_device,  0, ROT0, "JPM", "Give us a Break (4th edition)",      0 )
GAME( 1986, guab6,    guab,  guab, guab, driver_device,  0, ROT0, "JPM", "Give us a Break (6th edition)",      0 )
GAME( 1986, guab6a,   guab,  guab, guab, driver_device,  0, ROT0, "JPM", "Give us a Break (6th edition alt?)", 0 )
GAME( 1986, guab7,    guab,  guab, guab, driver_device,  0, ROT0, "JPM", "Give us a Break (7th edition)",      0 )
GAME( 1986, guab21,   guab,  guab, guab, driver_device,  0, ROT0, "JPM", "Give us a Break (21st edition)",     0 )
GAME( 1986, guab43,   guab,  guab, guab, driver_device,  0, ROT0, "JPM", "Give us a Break (43rd edition)",     0 )
GAME( 1986, crisscrs, 0,     guab, guab, driver_device,  0, ROT0, "JPM", "Criss Cross (Sweden)",               GAME_NOT_WORKING )
GAME( 1988, tenup,    0,     guab, tenup, driver_device, 0, ROT0, "JPM", "Ten Up (compendium 17)",             0 )
GAME( 1988, tenup3,   tenup, guab, tenup, driver_device, 0, ROT0, "JPM", "Ten Up (compendium 3)",              0 )

/***************************************************************************

	Atari Games Cops
	(hardware developed by Nova Productions Limited)

	Preliminary driver by Mariusz Wojcieszek

***************************************************************************/


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
//#include "machine/6551acia.h"

#include "cops.lh"

#define MAIN_CLOCK XTAL_4MHz

class cops_state : public driver_device
{
public:
	cops_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_irq(0)
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

public:
	DECLARE_WRITE8_MEMBER(io1_w);
	DECLARE_READ8_MEMBER(io1_r);
	DECLARE_WRITE8_MEMBER(io2_w);
	DECLARE_READ8_MEMBER(io2_r);
	DECLARE_WRITE8_MEMBER(via1_irq);
	DECLARE_WRITE8_MEMBER(via2_irq);
	void dacia_receive(UINT8 data);
	void laserdisc_w(UINT8 data);
	DECLARE_WRITE8_MEMBER(dacia_w);
	DECLARE_READ8_MEMBER(dacia_r);
	DECLARE_WRITE8_MEMBER(cdrom_data_w);
	DECLARE_WRITE8_MEMBER(cdrom_ctrl_w);
	DECLARE_READ8_MEMBER(cdrom_data_r);

	int	m_irq;

	UINT8 m_lcd_addr_l, m_lcd_addr_h;
	UINT8 m_lcd_data_l, m_lcd_data_h;

	UINT8 m_dacia_receiver_data;
	UINT8 m_dacia_receiver_full;

	UINT8 m_cdrom_ctrl;
	UINT8 m_cdrom_data;
};

void cops_state::video_start()
{

}

UINT32 cops_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

/*************************************
 *
 * Sony CDU33A-02 CDROM
 *
 *************************************/

WRITE8_MEMBER(cops_state::cdrom_data_w)
{
	const char *regs[4] = { "CMD", "PARAM", "WRITE", "CTRL" };
	m_cdrom_data = BITSWAP8(data,0,1,2,3,4,5,6,7);
	UINT8 reg = ((m_cdrom_ctrl & 4) >> 1) | ((m_cdrom_ctrl & 8) >> 3);
	logerror("%s:cdrom_data_w(reg = %s, data = %02x)\n", machine().describe_context(), regs[reg & 0x03], m_cdrom_data);
}

WRITE8_MEMBER(cops_state::cdrom_ctrl_w)
{
	logerror("%s:cdrom_ctrl_w(%02x)\n", machine().describe_context(), data);
	m_cdrom_ctrl = data;
}

READ8_MEMBER(cops_state::cdrom_data_r)
{
	logerror("%s:cdrom_data_r\n", machine().describe_context());
	return machine().rand()&0xff;
}
/*************************************
 *
 * 6552 DACIA & LDP-1450 Laserdisc
 *
 *************************************/

void cops_state::laserdisc_w(UINT8 data)
{
	switch( data )
	{
		case 0x26: /* Video off */
			dacia_receive(0x0a);
			break;
		case 0x27: /* Video on */
			dacia_receive(0x0a);
			break;
		case 0x30: /* Digit */
		case 0x31:
		case 0x32:
		case 0x33:
		case 0x34:
		case 0x35:
		case 0x36:
		case 0x37:
		case 0x38:
		case 0x39:
			dacia_receive(0x0a);
			break;
		case 0x3f: /* Stop */
			dacia_receive(0x0a);
			break;
		case 0x40: /* Enter */
			dacia_receive(0x0a);
			break;
		case 0x43: /* Search */
			dacia_receive(0x0a);
			break;
		case 0x56: /* C. L. (Reset) */
			dacia_receive(0x0a);
			break;
		default:
			logerror("Laserdisc command %02x\n", data);
			break;
	}
}

void cops_state::dacia_receive(UINT8 data)
{
	m_dacia_receiver_data = data;
	m_dacia_receiver_full = 1;
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


READ8_MEMBER(cops_state::dacia_r)
{
	switch(offset & 0x07)
	{
		case 0: /* ISR1: Interrupt Status Register */
			return 0x40 | m_dacia_receiver_full; /* Bit 6: Transmit Data Register Empty (TDRE) */
		case 3: /* RDR1: Receive data register */
			m_dacia_receiver_full = 0;
			return m_dacia_receiver_data;
		default:
			logerror("%s:dacia_r(%02x)\n", machine().describe_context(), offset);
			return 0;
	}
}

WRITE8_MEMBER(cops_state::dacia_w)
{
	switch(offset & 0x07)
	{
		case 3: /* Transmit Data Register 1 */
			//logerror("DACIA Transmit: %02x %c\n", data, (char)data);
			laserdisc_w(data);
			break;
		default:
			logerror("%s:dacia_w(%02x,%02x)\n", machine().describe_context(), offset, data);
			break;
	}
}

/*************************************
 *
 * I/O
 *
 *************************************/

READ8_MEMBER(cops_state::io1_r)
{
	switch( offset & 0x0f )
	{
		case 0x08:	/* SW0 */
			return ioport("SW0")->read();
		case 0x09:	/* SW1 */
			return ioport("SW1")->read();
		case 0x0a:	/* SW2 */
			return ioport("SW2")->read();
		default:
			logerror("Unknown io1_r, offset = %03x\n", offset);
			return 0;
	}
	return 0;
}

WRITE8_MEMBER(cops_state::io1_w)
{
	int i;
	char output_name[16];
	UINT16 display_data;

	switch( offset & 0x0f )
	{
		case 0x00: /* WOP0 Alpha display*/
			m_lcd_addr_l = data;
			break;
		case 0x01: /* WOP1 Alpha display*/
			m_lcd_addr_h = data;
			{
				// update display
				const UINT16 addrs_table[] = { 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0002, 0x0001, 0x0080,
											   0x1000, 0x0800, 0x0400, 0x2000, 0x4000, 0x0200, 0x0100, 0x8000 };
				UINT16 addr = m_lcd_addr_l | (m_lcd_addr_h << 8);
				for (i = 0; i < 16; i++ )
				{
					if (addr == addrs_table[i])
					{
						break;
					}
				}

				if (i < 16)
				{
					sprintf(output_name, "digit%d", i);
					display_data = m_lcd_data_l | (m_lcd_data_h << 8);
					display_data = BITSWAP16(display_data, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0); // todo:
					output_set_value(output_name, display_data);
				}
			}
			break;
		case 0x02: /* WOP2 Alpha display*/
			m_lcd_data_l = data;
			break;
		case 0x03: /* WOP3 Alpha display*/
			m_lcd_data_h = data;
			break;
		case 0x04: /* WOP4 */
			break;
		case 0x05: /* WOP5 */
			break;
		case 0x06: /* WOP6 */
			break;
		case 0x07: /* WOP? */
			break;
		case 0x08: /* WOP0 */
			break;
		default:
			logerror("Unknown io1_w, offset = %03x, data = %02x\n", offset, data);
			break;
	}
}

READ8_MEMBER(cops_state::io2_r)
{
	return 0;
}

WRITE8_MEMBER(cops_state::io2_w)
{
}

/*************************************
 *
 *  VIA 1 (U18)
 *   PA0-2 Steer
 *   PA3   ?
 *   PA4-6 Fade?
 *   PA7   STK (system rom banking)
 *   PB0-7 SN76489 data bus
 *   CA1-2 n.c.
 *   CB1   /WE SN76489
 *   IRQ   IRQ
 *
 *************************************/

WRITE8_MEMBER(cops_state::via1_irq)
{
	if ( data == ASSERT_LINE )
	{
		m_irq |= 1;
	}
	else
	{
		m_irq &= ~1;
	}
	m_maincpu->set_input_line(M6502_IRQ_LINE, m_irq ? ASSERT_LINE : CLEAR_LINE);
}

static const via6522_interface via_1_interface =
{
	DEVCB_NULL, DEVCB_NULL,								/*inputs : A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,		/*inputs : CA/B1,CA/B2 */
	DEVCB_NULL, DEVCB_NULL,								/*outputs: A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,		/*outputs: CA/B1,CA/B2 */
	DEVCB_DRIVER_MEMBER(cops_state,via1_irq)			/*irq                  */
};

/*************************************
 *
 *  VIA 2 (U27)
 *   PA0-7 GUN
 *   PB0-7 GUN
 *   IRQ   IRQ
 *
 *************************************/

WRITE8_MEMBER(cops_state::via2_irq)
{
	if ( data == ASSERT_LINE )
	{
		m_irq |= 2;
	}
	else
	{
		m_irq &= ~2;
	}
	m_maincpu->set_input_line(M6502_IRQ_LINE, m_irq ? ASSERT_LINE : CLEAR_LINE);
}


static const via6522_interface via_2_interface =
{
	DEVCB_NULL, DEVCB_NULL,								/*inputs : A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,		/*inputs : CA/B1,CA/B2 */
	DEVCB_NULL, DEVCB_NULL,								/*outputs: A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,		/*outputs: CA/B1,CA/B2 */
	DEVCB_DRIVER_MEMBER(cops_state,via2_irq)			/*irq                  */
};

/*************************************
 *
 *  VIA 3
 *   PA0-7 CDROM
 *   PB0-8 CDROM
 *
 *************************************/

static const via6522_interface via_3_interface =
{
	DEVCB_DRIVER_MEMBER(cops_state, cdrom_data_r), DEVCB_NULL,								/*inputs : A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,		/*inputs : CA/B1,CA/B2 */
	DEVCB_DRIVER_MEMBER(cops_state, cdrom_data_w), DEVCB_DRIVER_MEMBER(cops_state, cdrom_ctrl_w),								/*outputs: A/B         */
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL,		/*outputs: CA/B1,CA/B2 */
	DEVCB_NULL											/*irq                  */
};

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( cops_map, AS_PROGRAM, 8, cops_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x2000, 0x9fff) AM_ROM AM_REGION("program", 0)
	AM_RANGE(0xa000, 0xafff) AM_READWRITE(io1_r, io1_w)
	AM_RANGE(0xb000, 0xb00f) AM_DEVREADWRITE("via6522_1", via6522_device, read, write)	/* VIA 1 */
	AM_RANGE(0xb800, 0xb80f) AM_DEVREADWRITE("via6522_2", via6522_device, read, write)	/* VIA 2 */
	AM_RANGE(0xc000, 0xcfff) AM_READWRITE(io2_r, io2_w)
//	AM_RANGE(0xd000, 0xd003) AM_DEVREADWRITE("acia6551_1", acia6551_device, read, write )
//	AM_RANGE(0xd004, 0xd007) AM_DEVREADWRITE("acia6551_2", acia6551_device, read, write )
	AM_RANGE(0xd000, 0xd007) AM_READWRITE(dacia_r, dacia_w)
	AM_RANGE(0xd800, 0xd80f) AM_DEVREADWRITE("via6522_3", via6522_device, read, write)	/* VIA 3 */
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("system", 0)
ADDRESS_MAP_END


static INPUT_PORTS_START( cops )
	PORT_START("SW0")
	PORT_DIPNAME( 0x01, 0x00, "SW0" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "SW1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x00, "SW2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

void cops_state::machine_start()
{
}

void cops_state::machine_reset()
{
	m_irq = 0;
	m_lcd_addr_l = m_lcd_addr_h = 0;
	m_lcd_data_l = m_lcd_data_h = 0;

	m_dacia_receiver_full = 0;
}


static PALETTE_INIT( cops )
{
}

static MACHINE_CONFIG_START( cops, cops_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6502,MAIN_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(cops_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_UPDATE_DRIVER(cops_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MCFG_PALETTE_INIT(cops)
	MCFG_PALETTE_LENGTH(8)

	/* via */
	MCFG_VIA6522_ADD("via6522_1", 0, via_1_interface)
	MCFG_VIA6522_ADD("via6522_2", 0, via_2_interface)
	MCFG_VIA6522_ADD("via6522_3", 0, via_3_interface)

	/* acia */
//	MCFG_ACIA6551_ADD("acia6551_1")
//	MCFG_ACIA6551_ADD("acia6551_2")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cops )
	ROM_REGION( 0x8000, "program", 0 )
	ROM_LOAD( "cops_prg.dat", 0x0000, 0x8000, CRC(a5c02366) SHA1(b135d72fcfe737a113c984b0b8dd78428f248414) )

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "cops_sys.dat", 0x0000, 0x8000, CRC(0060e5d0) SHA1(b8c9f6fde6a315e33fa7946e5d3bb4ea2fbe76a8) )
ROM_END

GAMEL( 1994, cops,  0,   cops,  cops,  driver_device, 0,       ROT0, "Atari Games",      "Cops", GAME_NOT_WORKING | GAME_NO_SOUND, layout_cops )

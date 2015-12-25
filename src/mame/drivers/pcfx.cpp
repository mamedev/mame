// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  pcfx.c

  Driver file to handle emulation of the NEC PC-FX.

***************************************************************************/


#include "emu.h"
#include "cpu/v810/v810.h"
#include "video/huc6261.h"
#include "video/huc6270.h"
#include "video/huc6272.h"

struct pcfx_pad_t
{
	UINT8 ctrl[2];
	UINT8 status[2];
	UINT32 latch[2];
};

class pcfx_state : public driver_device
{
public:
	enum
	{
		TIMER_PAD_FUNC
	};

	pcfx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_huc6261(*this, "huc6261") { }

	required_device<cpu_device> m_maincpu;
	required_device<huc6261_device> m_huc6261;

	virtual void machine_reset() override;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// Interrupt controller (component unknown)
	UINT16 m_irq_mask;
	UINT16 m_irq_pending;
	UINT8 m_irq_priority[8];

	pcfx_pad_t m_pad;

	DECLARE_READ16_MEMBER( irq_read );
	DECLARE_WRITE16_MEMBER( irq_write );
	DECLARE_READ16_MEMBER( pad_r );
	DECLARE_WRITE16_MEMBER( pad_w );
	DECLARE_READ8_MEMBER( extio_r );
	DECLARE_WRITE8_MEMBER( extio_w );

	inline void check_irqs();
	inline void set_irq_line(int line, int state);
	DECLARE_WRITE_LINE_MEMBER( irq8_w );
	DECLARE_WRITE_LINE_MEMBER( irq9_w );
	DECLARE_WRITE_LINE_MEMBER( irq10_w );
	DECLARE_WRITE_LINE_MEMBER( irq11_w );
	DECLARE_WRITE_LINE_MEMBER( irq12_w );
	DECLARE_WRITE_LINE_MEMBER( irq13_w );
	DECLARE_WRITE_LINE_MEMBER( irq14_w );
	DECLARE_WRITE_LINE_MEMBER( irq15_w );
	TIMER_CALLBACK_MEMBER(pad_func);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


READ8_MEMBER(pcfx_state::extio_r)
{
	address_space &io_space = m_maincpu->space(AS_IO);

	return io_space.read_byte(offset);
}

WRITE8_MEMBER(pcfx_state::extio_w)
{
	address_space &io_space = m_maincpu->space(AS_IO);

	io_space.write_byte(offset, data);
}

static ADDRESS_MAP_START( pcfx_mem, AS_PROGRAM, 32, pcfx_state )
	AM_RANGE( 0x00000000, 0x001FFFFF ) AM_RAM   /* RAM */
//  AM_RANGE( 0x80000000, 0x807FFFFF ) AM_READWRITE8(extio_r,extio_w,0xffffffff)    /* EXTIO */
	AM_RANGE( 0xE0000000, 0xE7FFFFFF ) AM_NOP   /* BackUp RAM */
	AM_RANGE( 0xE8000000, 0xE9FFFFFF ) AM_NOP   /* Extended BackUp RAM */
	AM_RANGE( 0xF8000000, 0xF8000007 ) AM_NOP   /* PIO */
	AM_RANGE( 0xFFF00000, 0xFFFFFFFF ) AM_ROMBANK("bank1")  /* ROM */
ADDRESS_MAP_END

READ16_MEMBER( pcfx_state::pad_r )
{
	UINT16 res;
	UINT8 port_type = ((offset<<1) & 0x80) >> 7;

	if(((offset<<1) & 0x40) == 0)
	{
		// status
		/*
		---- x---
		---- ---x incoming data state (0=available)
		*/
		res = m_pad.status[port_type];
		//printf("STATUS %d\n",port_type);
	}
	else
	{
		// received data
		//printf("RX %d\n",port_type);
		res = m_pad.latch[port_type] >> (((offset<<1) & 2) ? 16 : 0);

		if(((offset<<1) & 0x02) == 0)
		{
			m_pad.status[port_type] &= ~8; // clear latch on LSB read according to docs
			set_irq_line(11, 0);
		}
	}

	return res;
}

void pcfx_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_PAD_FUNC:
		pad_func(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in pcfx_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(pcfx_state::pad_func)
{
	const char *const padnames[] = { "P1", "P2" };

	m_pad.latch[param] = ioport(padnames[param])->read();
	m_pad.status[param] |= 8;
	m_pad.ctrl[param] &= ~1; // ack TX line
	// TODO: pad IRQ
	set_irq_line(11, 1);
}

WRITE16_MEMBER( pcfx_state::pad_w )
{
	UINT8 port_type = ((offset<<1) & 0x80) >> 7;

	if(((offset<<1) & 0x40) == 0)
	{
		// control
		/*
		---- -x-- receiver enable
		---- --x- enable multi-tap
		---- ---x enable send (0->1 transition)
		*/
		if(data & 1 && (!(m_pad.ctrl[port_type] & 1)))
		{
			timer_set(attotime::from_msec(1), TIMER_PAD_FUNC, port_type); // TODO: time
		}

		m_pad.ctrl[port_type] = data & 7;
		//printf("%04x CONTROL %d\n",data,port_type);
	}
	else
	{
		// transmitted data
		//printf("%04x TX %d\n",data,port_type);
	}
}


static ADDRESS_MAP_START( pcfx_io, AS_IO, 32, pcfx_state )
	AM_RANGE( 0x00000000, 0x000000FF ) AM_READWRITE16(pad_r, pad_w, 0xffffffff) /* PAD */
	AM_RANGE( 0x00000100, 0x000001FF ) AM_NOP   /* HuC6230 */
	AM_RANGE( 0x00000200, 0x000002FF ) AM_NOP   /* HuC6271 */
	AM_RANGE( 0x00000300, 0x000003FF ) AM_DEVREADWRITE16( "huc6261", huc6261_device, read, write, 0xffff )  /* HuC6261 */
	AM_RANGE( 0x00000400, 0x000004FF ) AM_DEVREADWRITE8( "huc6270_a", huc6270_device, read, write, 0xffff ) /* HuC6270-A */
	AM_RANGE( 0x00000500, 0x000005FF ) AM_DEVREADWRITE8( "huc6270_b", huc6270_device, read, write, 0xffff ) /* HuC6270-B */
	AM_RANGE( 0x00000600, 0x000006FF ) AM_DEVREADWRITE( "huc6272", huc6272_device, read, write )    /* HuC6272 */
	AM_RANGE( 0x00000C80, 0x00000C83 ) AM_NOP
	AM_RANGE( 0x00000E00, 0x00000EFF ) AM_READWRITE16( irq_read, irq_write, 0xffff )    /* Interrupt controller */
	AM_RANGE( 0x00000F00, 0x00000FFF ) AM_NOP
//  AM_RANGE( 0x00600000, 0x006FFFFF ) AM_READ(scsi_ctrl_r)
	AM_RANGE( 0x00780000, 0x007FFFFF ) AM_ROM AM_REGION("scsi_rom", 0 )
	AM_RANGE( 0x80500000, 0x805000FF ) AM_NOP   /* HuC6273 */
ADDRESS_MAP_END


static INPUT_PORTS_START( pcfx )
	/*
	xxxx ---- ---- ---- ID (0xf = 6 button pad, 0xe = tap, 0xd = ?)
	*/
	PORT_START("P1")
	PORT_BIT( 0xf0000000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ID
	PORT_DIPNAME( 0x01000000, 0x01000000, "1" )
	PORT_DIPSETTING(    0x01000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x02000000, 0x02000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x04000000, 0x04000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x08000000, 0x08000000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08000000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x010000, 0x010000, "2" )
	PORT_DIPSETTING(    0x010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x020000, 0x020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x040000, 0x040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x080000, 0x080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x100000, 0x100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x200000, 0x200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x400000, 0x400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x800000, 0x800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "3" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) ) // *
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) // *
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START1 ) PORT_PLAYER(1) PORT_NAME("RUN button")
	PORT_DIPNAME( 0x01, 0x01, "4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("P2")
	PORT_BIT( 0xf0000000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0fffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


READ16_MEMBER( pcfx_state::irq_read )
{
	UINT16 data = 0;

	switch( offset )
	{
		// Interrupts pending
		// Same bit order as mask
		case 0x00/4:
			data = m_irq_pending;
			break;

		// Interrupt mask
		case 0x40/4:
			data = m_irq_mask;
			break;

		// Interrupt priority 0
		case 0x80/4:
			data = m_irq_priority[4] | ( m_irq_priority[5] << 3 ) | ( m_irq_priority[6] << 6 ) | ( m_irq_priority[7] << 9 );
			break;

		// Interrupt priority 1
		case 0xC0/4:
			data = m_irq_priority[0] | ( m_irq_priority[1] << 3 ) | ( m_irq_priority[2] << 6 ) | ( m_irq_priority[3] << 9 );
			break;
	}

	return data;
}


WRITE16_MEMBER( pcfx_state::irq_write )
{
	switch( offset )
	{
		// Interrupts pending
		case 0x00/4:
			logerror("irq_write: Attempt to write to irq pending register\n");
			break;

		// Interrupt mask
		// --------x------- Mask interrupt level 8  (Unknown)
		// ---------x------ Mask interrupt level 9  (Timer)
		// ----------x----- Mask interrupt level 10 (Unknown)
		// -----------x---- Mask interrupt level 11 (Pad)
		// ------------x--- Mask interrupt level 12 (HuC6270-A)
		// -------------x-- Mask interrupt level 13 (HuC6272)
		// --------------x- Mask interrupt level 14 (HuC6270-B)
		// ---------------x Mask interrupt level 15 (HuC6273)
		// 0 - allow, 1 - ignore interrupt
		case 0x40/4:
			m_irq_mask = data;
			check_irqs();
			break;

		// Interrupt priority 0
		// ----xxx--------- Priority level interrupt 12
		// -------xxx------ Priority level interrupt 13
		// ----------xxx--- Priority level interrupt 14
		// -------------xxx Priority level interrupt 15
		case 0x80/4:
			m_irq_priority[7] = ( data >> 0 ) & 0x07;
			m_irq_priority[6] = ( data >> 3 ) & 0x07;
			m_irq_priority[5] = ( data >> 6 ) & 0x07;
			m_irq_priority[4] = ( data >> 9 ) & 0x07;
			check_irqs();
			break;

		// Interrupt priority 1
		// ----xxx--------- Priority level interrupt 8
		// -------xxx------ Priority level interrupt 9
		// ----------xxx--- Priority level interrupt 10
		// -------------xxx Priority level interrupt 11
		case 0xC0/4:
			m_irq_priority[3] = ( data >> 0 ) & 0x07;
			m_irq_priority[2] = ( data >> 3 ) & 0x07;
			m_irq_priority[1] = ( data >> 6 ) & 0x07;
			m_irq_priority[0] = ( data >> 9 ) & 0x07;
			check_irqs();
			break;
	}
}


inline void pcfx_state::check_irqs()
{
	UINT16 active_irqs = m_irq_pending & ~m_irq_mask;
	int highest_prio = -1;

	for (auto & elem : m_irq_priority)
	{
		if ( active_irqs & 0x80 )
		{
			if ( elem >= highest_prio )
			{
				highest_prio = elem;
			}
		}
		active_irqs <<= 1;
	}

	if ( highest_prio >= 0 )
	{
		m_maincpu->set_input_line(8 + highest_prio, ASSERT_LINE );
	}
	else
	{
		m_maincpu->set_input_line(0, CLEAR_LINE );
	}
}


inline void pcfx_state::set_irq_line(int line, int state)
{
	if ( state )
	{
//printf("Setting irq line %d\n", line);
		m_irq_pending |= ( 1 << ( 15 - line ) );
	}
	else
	{
//printf("Clearing irq line %d\n", line);
		m_irq_pending &= ~( 1 << ( 15 - line ) );
	}
	check_irqs();
}

WRITE_LINE_MEMBER( pcfx_state::irq8_w )
{
	set_irq_line(8, state);
}

WRITE_LINE_MEMBER( pcfx_state::irq9_w )
{
	set_irq_line(9, state);
}

WRITE_LINE_MEMBER( pcfx_state::irq10_w )
{
	set_irq_line(10, state);
}

WRITE_LINE_MEMBER( pcfx_state::irq11_w )
{
	set_irq_line(11, state);
}

WRITE_LINE_MEMBER( pcfx_state::irq12_w )
{
	set_irq_line(12, state);
}

WRITE_LINE_MEMBER( pcfx_state::irq13_w )
{
	set_irq_line(13, state);
}

WRITE_LINE_MEMBER( pcfx_state::irq14_w )
{
	set_irq_line(14, state);
}

WRITE_LINE_MEMBER( pcfx_state::irq15_w )
{
	set_irq_line(15, state);
}


void pcfx_state::machine_reset()
{
	membank( "bank1" )->set_base( memregion("user1")->base() );

	m_irq_mask = 0xFF;
	m_irq_pending = 0;
}


UINT32 pcfx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_huc6261->video_update( bitmap, cliprect );
	return 0;
}


static MACHINE_CONFIG_START( pcfx, pcfx_state )
	MCFG_CPU_ADD( "maincpu", V810, XTAL_21_4772MHz )
	MCFG_CPU_PROGRAM_MAP( pcfx_mem)
	MCFG_CPU_IO_MAP( pcfx_io)

	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_UPDATE_DRIVER(pcfx_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_21_4772MHz, HUC6261_WPF, 64, 64 + 1024 + 64, HUC6261_LPF, 18, 18 + 242)

	MCFG_DEVICE_ADD( "huc6270_a", HUC6270, 0 )
	MCFG_HUC6270_VRAM_SIZE(0x20000)
	MCFG_HUC6270_IRQ_CHANGED_CB(WRITELINE(pcfx_state, irq12_w))

	MCFG_DEVICE_ADD( "huc6270_b", HUC6270, 0 )
	MCFG_HUC6270_VRAM_SIZE(0x20000)
	MCFG_HUC6270_IRQ_CHANGED_CB(WRITELINE(pcfx_state, irq14_w))

	MCFG_DEVICE_ADD("huc6261", HUC6261, XTAL_21_4772MHz)
	MCFG_HUC6261_VDC1("huc6270_a")
	MCFG_HUC6261_VDC2("huc6270_b")

	MCFG_HUC6272_ADD( "huc6272", XTAL_21_4772MHz )
MACHINE_CONFIG_END


ROM_START( pcfx )
	ROM_REGION( 0x100000, "user1", 0 )
	ROM_SYSTEM_BIOS( 0, "v100", "BIOS v1.00 - 2 Sep 1994" )
	ROMX_LOAD( "pcfxbios.bin", 0x000000, 0x100000, CRC(76ffb97a) SHA1(1a77fd83e337f906aecab27a1604db064cf10074), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v101", "BIOS v1.01 - 5 Dec 1994" )
	ROMX_LOAD( "pcfxv101.bin", 0x000000, 0x100000, CRC(236102c9) SHA1(8b662f7548078be52a871565e19511ccca28c5c8), ROM_BIOS(2) )

	ROM_REGION( 0x80000, "scsi_rom", 0 )
	ROM_LOAD( "fx-scsi.rom", 0x00000, 0x80000, CRC(f3e60e5e) SHA1(65482a23ac5c10a6095aee1db5824cca54ead6e5) )
ROM_END


ROM_START( pcfxga )
	ROM_REGION( 0x100000, "user1", 0 )
	ROM_LOAD( "pcfxga.rom", 0x000000, 0x100000, CRC(41c3776b) SHA1(a9372202a5db302064c994fcda9b24d29bb1b41c) )

	ROM_REGION( 0x80000, "scsi_rom", ROMREGION_ERASEFF )
ROM_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT      INIT    COMPANY                       FULLNAME                  FLAGS */
CONS( 1994, pcfx,       0,      0,      pcfx,       pcfx, driver_device,      0,      "Nippon Electronic Company",  "PC-FX",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 199?, pcfxga,     pcfx,   0,      pcfx,       pcfx, driver_device,      0,      "Nippon Electronic Company",  "PC-FX/GA (PC ISA Card)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

/***************************************************************************

	NEC APC

	front ^
	      |
	card
	----
	69PFCU 7220               PFCU1R 2764
	69PTS  7220
	-
	69PFB2 8086/8087   DFBU2J PFBU2L 2732
	69SNB RAM

	i/o memory map:
	0x00 - 0x1f DMA
	0x20 - 0x23 i8259 master
	0x28 - 0x2b i8259 slave
	0x2b / 0x2f / 0x61 / 0x6f pit8253 (!)
	0x30 - 0x37 serial i8251, even #1 / odd #2
	0x38 - 0x3f DMA segments
	0x40 - 0x43 upd7220, even chr / odd bitmap
	0x48 - 0x4f keyboard
	0x50 - 0x53 upd765
	0x58        rtc
	0x5a - 0x5e APU
	0x60        MPU (melody)
	0x68 - 0x6f parallel port

***************************************************************************/


#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/8237dma.h"
#include "machine/upd765.h"
#include "video/upd7220.h"
#include "imagedev/flopdrv.h"
#include "formats/mfi_dsk.h"
#include "formats/d88_dsk.h"
//#include "sound/ay8910.h"

class apc_state : public driver_device
{
public:
	apc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_hgdc1(*this, "upd7220_chr"),
		  m_hgdc2(*this, "upd7220_btm"),
		  m_i8259_m(*this, "pic8259_master"),
		  m_i8259_s(*this, "pic8259_slave")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<upd7220_device> m_hgdc1;
	required_device<upd7220_device> m_hgdc2;
	required_device<pic8259_device> m_i8259_m;
	required_device<pic8259_device> m_i8259_s;
	UINT8 *m_char_rom;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);


	DECLARE_READ8_MEMBER(apc_port_28_r);
	DECLARE_WRITE8_MEMBER(apc_port_28_w);
	DECLARE_WRITE8_MEMBER(apc_port_2e_w);
	DECLARE_READ8_MEMBER(apc_port_60_r);
	DECLARE_WRITE8_MEMBER(apc_port_60_w);
	DECLARE_READ8_MEMBER(apc_gdc_r);
	DECLARE_WRITE8_MEMBER(apc_gdc_w);
	DECLARE_READ8_MEMBER(apc_dma_r);
	DECLARE_WRITE8_MEMBER(apc_dma_w);

	DECLARE_WRITE_LINE_MEMBER(apc_master_set_int_line);
	DECLARE_READ8_MEMBER(get_slave_ack);
	DECLARE_WRITE_LINE_MEMBER(pc_dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(pc_dack0_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack1_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack2_w);
	DECLARE_WRITE_LINE_MEMBER(pc_dack3_w);
	DECLARE_READ8_MEMBER(test_r);
	DECLARE_WRITE8_MEMBER(test_w);
	DECLARE_READ8_MEMBER(pc_dma_read_byte);
	DECLARE_WRITE8_MEMBER(pc_dma_write_byte);

	DECLARE_DRIVER_INIT(apc);

	int m_dma_channel;
	UINT8 m_dma_offset[2][4];
	UINT8 m_at_pages[0x10];

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual void palette_init();
};

void apc_state::video_start()
{
	m_char_rom = memregion("gfx")->base();
}

UINT32 apc_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(0, cliprect);

	/* graphics */
	m_hgdc2->screen_update(screen, bitmap, cliprect);
	m_hgdc1->screen_update(screen, bitmap, cliprect);

	return 0;
}


static UPD7220_DISPLAY_PIXELS( hgdc_display_pixels )
{
	// ...
}

static UPD7220_DRAW_TEXT_LINE( hgdc_draw_text )
{

}

READ8_MEMBER(apc_state::apc_port_28_r)
{
	UINT8 res;

	if(offset & 1)
	{
		if(offset == 3)
			res = pit8253_r(machine().device("pit8253"), space, 0);
		else
		{
			printf("Read undefined port 0x29\n");
			res = 0xff;
		}
	}
	else
	{
		res = pic8259_r(machine().device("pic8259_slave"), space, (offset & 2) >> 1);
	}

	return res;
}

WRITE8_MEMBER(apc_state::apc_port_28_w)
{
	if(offset & 1)
	{
		if(offset == 3)
			pit8253_w(machine().device("pit8253"), space, 0, data);
		else
		{
			printf("Write undefined port 0x29\n");
		}
	}
	else
	{
		pic8259_w(machine().device("pic8259_slave"), space, (offset & 2) >> 1, data);
	}
}

WRITE8_MEMBER(apc_state::apc_port_2e_w)
{
	pit8253_w(machine().device("pit8253"), space, 1, data);
}


READ8_MEMBER(apc_state::apc_port_60_r)
{
	UINT8 res;

	if(offset & 1)
	{
		if(offset == 1)
			res = pit8253_r(machine().device("pit8253"), space, 2);
		else
		{
			printf("Read undefined port %02x\n",offset+0x60);
			res = 0xff;
		}
	}
	else
	{
		printf("Read melody port %02x\n",offset+0x60);
		res = 0xff;
	}

	return res;
}

WRITE8_MEMBER(apc_state::apc_port_60_w)
{
	if(offset & 1)
	{
		if(offset == 1)
			pit8253_w(machine().device("pit8253"), space, 2, data);
		else if(offset == 7)
			pit8253_w(machine().device("pit8253"), space, 3, data);
		else
		{
			printf("Write undefined port %02x\n",offset+0x60);
		}
	}
	else
	{
		printf("Write melody port %02x\n",offset+0x60);
	}
}

READ8_MEMBER(apc_state::apc_gdc_r)
{
	UINT8 res;

	if(offset & 1)
		res = m_hgdc2->read(space, (offset & 2) >> 1); // upd7220 bitmap port
	else
		res = m_hgdc1->read(space, (offset & 2) >> 1); // upd7220 character port

	return res;
}

WRITE8_MEMBER(apc_state::apc_gdc_w)
{
	if(offset & 1)
		m_hgdc2->write(space, (offset & 2) >> 1,data); // upd7220 bitmap port
	else
		m_hgdc1->write(space, (offset & 2) >> 1,data); // upd7220 character port
}


READ8_MEMBER(apc_state::apc_dma_r)
{
	return i8237_r(machine().device("8237dma"), space, offset & 0xf);
}

WRITE8_MEMBER(apc_state::apc_dma_w)
{
	//printf("%08x %02x\n",m_maincpu->pc(),data);
	i8237_w(machine().device("8237dma"), space, offset & 0xf, data);
}


static ADDRESS_MAP_START( apc_map, AS_PROGRAM, 16, apc_state )
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
//	AM_RANGE(0xa0000, 0xaffff) space for an external ROM
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("ipl", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( apc_io, AS_IO, 16, apc_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x1f) AM_READWRITE8(apc_dma_r, apc_dma_w, 0x00ff)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, 0x00ff) // i8259
	AM_RANGE(0x28, 0x2b) AM_READWRITE8(apc_port_28_r, apc_port_28_w, 0xffff)
	AM_RANGE(0x2e, 0x2f) AM_WRITE8(apc_port_2e_w, 0x00ff)
//	0x2b RTC counter port 0
//	0x2f RTC counter mode 0 (w)
//	0x30, 0x37 serial port 0/1 (i8251) (even/odd)
//	0x38, 0x3f DMA extended address
	AM_RANGE(0x40, 0x43) AM_READWRITE8(apc_gdc_r, apc_gdc_w, 0xffff)
//  0x46 UPD7220 reset interrupt
//	0x48, 0x4f keyboard controller
	AM_RANGE(0x50, 0x53) AM_DEVICE8("upd765", upd765a_device, map, 0x00ff ) // upd765
//	0x5a  APU data (Arithmetic Processing Unit!)
//	0x5e  APU status/command
	AM_RANGE(0x60, 0x67) AM_READWRITE8(apc_port_60_r, apc_port_60_w, 0xffff)
//	0x60 Melody Processing Unit
//	0x61 RTC counter port 1
//	0x67 RTC counter mode 1 (w)
//	AM_RANGE(0x68, 0x6f) i8255 , printer port (A: status (R) B: data (W) C: command (W))
//	AM_DEVREADWRITE8("upd7220_btm", upd7220_device, read, write, 0x00ff)
ADDRESS_MAP_END

static INPUT_PORTS_START( apc )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
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

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void apc_state::machine_start()
{
}

void apc_state::machine_reset()
{
}


void apc_state::palette_init()
{
}

static UPD7220_INTERFACE( hgdc_1_intf )
{
	"screen",
	NULL,
	hgdc_draw_text,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


static UPD7220_INTERFACE( hgdc_2_intf )
{
	"screen",
	hgdc_display_pixels,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const gfx_layout charset_8x16 =
{
	8,16,
	256,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};


static GFXDECODE_START( apc )
	GFXDECODE_ENTRY( "gfx", 0x0000, charset_8x16, 0, 8 )
GFXDECODE_END


static ADDRESS_MAP_START( upd7220_1_map, AS_0, 8, apc_state)
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram_1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( upd7220_2_map, AS_0, 8, apc_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram_2")
ADDRESS_MAP_END

static const struct pit8253_config pit8253_config =
{
	{
		{
			1996800,              /* heartbeat IRQ */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			1996800,              /* Memory Refresh */
			DEVCB_NULL,
			DEVCB_NULL
		}, {
			1996800,              /* RS-232c */
			DEVCB_NULL,
			DEVCB_NULL
		}
	}
};
/*
irq assignment:

8259 master:
ir0
ir1
ir2
ir3
ir4
ir5
ir6
ir7

8259 slave:
ir0
ir1
ir2
ir3
ir4
ir5
ir6
ir7
*/


WRITE_LINE_MEMBER(apc_state::apc_master_set_int_line)
{
	//printf("%02x\n",interrupt);
	machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

READ8_MEMBER(apc_state::get_slave_ack)
{
	if (offset==7) { // IRQ = 7
		return pic8259_acknowledge( machine().device( "pic8259_slave" ));
	}
	return 0x00;
}

static const struct pic8259_interface pic8259_master_config =
{
	DEVCB_DRIVER_LINE_MEMBER(apc_state, apc_master_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_DRIVER_MEMBER(apc_state,get_slave_ack)
};

static const struct pic8259_interface pic8259_slave_config =
{
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir7_w), //TODO: check me
	DEVCB_LINE_GND,
	DEVCB_NULL
};

/****************************************
*
* I8237 DMA interface
*
****************************************/

WRITE_LINE_MEMBER(apc_state::pc_dma_hrq_changed)
{
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	i8237_hlda_w( machine().device("dma8237"), state );
}


READ8_MEMBER(apc_state::pc_dma_read_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	return space.read_byte(page_offset + offset);
}


WRITE8_MEMBER(apc_state::pc_dma_write_byte)
{
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	space.write_byte(page_offset + offset, data);
}

static void set_dma_channel(running_machine &machine, int channel, int state)
{
	apc_state *drvstate = machine.driver_data<apc_state>();
	if (!state) drvstate->m_dma_channel = channel;
}

WRITE_LINE_MEMBER(apc_state::pc_dack0_w){ /*printf("%02x 0\n",state);*/ set_dma_channel(machine(), 0, state); }
WRITE_LINE_MEMBER(apc_state::pc_dack1_w){ /*printf("%02x 1\n",state);*/ set_dma_channel(machine(), 1, state); }
WRITE_LINE_MEMBER(apc_state::pc_dack2_w){ /*printf("%02x 2\n",state);*/ set_dma_channel(machine(), 2, state); }
WRITE_LINE_MEMBER(apc_state::pc_dack3_w){ /*printf("%02x 3\n",state);*/ set_dma_channel(machine(), 3, state); }

READ8_MEMBER(apc_state::test_r)
{
//	printf("2dd DACK R\n");

	return 0xff;
}

WRITE8_MEMBER(apc_state::test_w)
{
//	printf("2dd DACK W\n");
}

static I8237_INTERFACE( dma8237_config )
{
	DEVCB_DRIVER_LINE_MEMBER(apc_state, pc_dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(apc_state, pc_dma_read_byte),
	DEVCB_DRIVER_MEMBER(apc_state, pc_dma_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_MEMBER(apc_state,test_r) },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_MEMBER(apc_state,test_w) },
	{ DEVCB_DRIVER_LINE_MEMBER(apc_state, pc_dack0_w), DEVCB_DRIVER_LINE_MEMBER(apc_state, pc_dack1_w), DEVCB_DRIVER_LINE_MEMBER(apc_state, pc_dack2_w), DEVCB_DRIVER_LINE_MEMBER(apc_state, pc_dack3_w) }
};

static const floppy_format_type apc_floppy_formats[] = {
	FLOPPY_D88_FORMAT,
	FLOPPY_MFI_FORMAT,
	NULL // TODO: IMD
};

static SLOT_INTERFACE_START( apc_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD ) // TODO: 8"
SLOT_INTERFACE_END


#define MAIN_CLOCK XTAL_5MHz

static MACHINE_CONFIG_START( apc, apc_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8086,MAIN_CLOCK)
	MCFG_CPU_PROGRAM_MAP(apc_map)
	MCFG_CPU_IO_MAP(apc_io)

	MCFG_PIT8253_ADD( "pit8253", pit8253_config )
	MCFG_PIC8259_ADD( "pic8259_master", pic8259_master_config )
	MCFG_PIC8259_ADD( "pic8259_slave", pic8259_slave_config )
	MCFG_I8237_ADD("8237dma", MAIN_CLOCK, dma8237_config)

	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", apc_floppies, "525hd", 0, apc_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", apc_floppies, "525hd", 0, apc_floppy_formats)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(apc_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_GFXDECODE(apc)

	MCFG_UPD7220_ADD("upd7220_chr", 5000000/2, hgdc_1_intf, upd7220_1_map)
	MCFG_UPD7220_ADD("upd7220_btm", 5000000/2, hgdc_2_intf, upd7220_2_map)

	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( apc )
	ROM_REGION( 0x2000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "pfbu2j.bin",   0x00000, 0x001000, CRC(86970df5) SHA1(be59c5dad3bd8afc21e9f2f1404553d4371978be) )
    ROM_LOAD16_BYTE( "pfbu2l.bin",   0x00001, 0x001000, CRC(38df2e70) SHA1(a37ccaea00c2b290610d354de08b489fa897ec48) )

//	ROM_REGION( 0x10000, "file", ROMREGION_ERASE00 )
//	ROM_LOAD( "sioapc.o", 0, 0x10000, CRC(1) SHA1(1) )

	ROM_REGION( 0x2000, "gfx", ROMREGION_ERASE00 )
    ROM_LOAD( "pfcu1r.bin",   0x000000, 0x002000, BAD_DUMP CRC(683efa94) SHA1(43157984a1746b2e448f3236f571011af9a3aa73) )
ROM_END

DRIVER_INIT_MEMBER(apc_state,apc)
{
	UINT8 *ROM = memregion("ipl")->base();

	/* patch DMA check */
	ROM[0xff334 & 0x1fff] = 0x90;
	ROM[0xff335 & 0x1fff] = 0x90;
	ROM[0xff339 & 0x1fff] = 0x90;
	ROM[0xff33a & 0x1fff] = 0x90;

}

GAME( 198?, apc,  0,   apc,  apc, apc_state,  apc,       ROT0, "NEC",      "APC", GAME_IS_SKELETON )

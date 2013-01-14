/***************************************************************************

    Casio FP-1100

    Info found at various sites:

    Casio FP1000 and FP1100 are "pre-PC" personal computers, with Cassette,
    Floppy Disk, Printer and 2 cart/expansion slots. They had 32K ROM, 64K
    main RAM, 80x25 text display, 320x200, 640x200, 640x400 graphics display.
    Floppy disk is 2x 5 1/4.

    The FP1000 had 16K videoram and monochrome only. The monitor had a switch
    to invert the display (swap foreground and background colours).

    The FP1100 had 48K videoram and 8 colours.

    Processors: Z80 @ 4MHz, uPD7801G @ 2MHz

    Came with Basic built in, and you could run CP/M 2.2 from the floppy disk.

    The keyboard is a separate unit. Sound capabilities are unknown.

    TODO:
    - irq sources and communications;
    - unimplemented instruction PER triggered

unimplemented instruction: PER
'sub' (00C0): unmapped i/o memory write to 00 = 00 & FF
'sub' (00C8): unmapped i/o memory read from 02 & FF
'sub' (03CF): unmapped i/o memory write to 00 = 00 & FF
'sub' (010E): unmapped i/o memory write to 00 = 40 & FF
'sub' (0113): unmapped i/o memory read from 02 & FF
'sub' (0116): unmapped i/o memory read from 02 & FF
'sub' (03CF): unmapped i/o memory write to 00 = 40 & FF
'sub' (06A6): unmapped program memory write to F000 = 00 & FF
'sub' (06B7): unmapped i/o memory write to 00 = 40 & FF
'sub' (FF93): unmapped program memory write to F000 = F8 & FF
'sub' (FFB1): unmapped i/o memory write to 00 = 40 & FF
'sub' (FF8F): unmapped i/o memory write to 00 = 60 & FF
'sub' (FF93): unmapped program memory write to F000 = 80 & FF
'sub' (FFB1): unmapped i/o memory write to 00 = 40 & FF
'sub' (06E4): unmapped i/o memory write to 00 = 47 & FF
'sub' (06EA): unmapped program memory write to F000 = 70 & FF

2011-09-18 Video (bitmapped) added. Notes:
- The colour planes are a guess (r,g,b might need swapping around)
- In "irq_mask_w" is a read of e800. Without this, no videoram writes occur.
- In "fp1100_vram_w" the data is inverted. Without it, a flashing cursor can be seen.
- The text mode is also present at 9000-97FF of the maincpu (not bitmapped).
- The display should keep scrolling, but it seems to crash after the first
  scroll (as can be seen in the 9000-97FF area).

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/upd7810/upd7810.h"
#include "video/mc6845.h"


class fp1100_state : public driver_device
{
public:
	fp1100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_subcpu(*this, "sub"),
	//m_cass(*this, CASSETTE_TAG),
	//m_wave(*this, WAVE_TAG),
	//m_speaker(*this, SPEAKER_TAG),
	//m_printer(*this, "centronics"),
	m_crtc(*this, "crtc"),
	//m_fdc(*this, "fdc"),
	m_p_videoram(*this, "videoram"){ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	//required_device<cassette_image_device> m_cass;
	//required_device<> m_wave;
	//required_device<> m_speaker;
	//required_device<> m_printer;
	required_device<mc6845_device> m_crtc;
	//required_device<> m_fdc;
	DECLARE_READ8_MEMBER(fp1100_mem_r);
	DECLARE_WRITE8_MEMBER(fp1100_mem_w);
	DECLARE_WRITE8_MEMBER(main_bank_w);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_WRITE8_MEMBER(main_to_sub_w);
	DECLARE_READ8_MEMBER(sub_to_main_r);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_WRITE8_MEMBER(slot_bank_w);
	DECLARE_READ8_MEMBER(slot_id_r);
	DECLARE_READ8_MEMBER(fp1100_vram_r);
	DECLARE_WRITE8_MEMBER(fp1100_vram_w);
	DECLARE_READ8_MEMBER(main_to_sub_r);
	DECLARE_WRITE8_MEMBER(sub_to_main_w);
	DECLARE_WRITE8_MEMBER(portc_w);
	UINT8 *m_wram;
	required_shared_ptr<UINT8> m_p_videoram;
	UINT8 m_mem_bank;
	UINT8 irq_mask;
	UINT8 m_main_latch;
	UINT8 m_sub_latch;
	UINT8 m_slot_num;

	struct {
		UINT8 id;
	}m_slot[8];

	struct {
		UINT8 porta;
		UINT8 portb;
		UINT8 portc;
	}m_upd7801;
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	INTERRUPT_GEN_MEMBER(fp1100_vblank_irq);
};

void fp1100_state::video_start()
{
}

static MC6845_UPDATE_ROW( fp1100_update_row )
{
	fp1100_state *state = device->machine().driver_data<fp1100_state>();
	const rgb_t *palette = palette_entry_list_raw(bitmap.palette());
	UINT8 r,g,b,col,i;
	UINT16 mem,x;
	UINT32 *p = &bitmap.pix32(y);

	for (x = 0; x < x_count; x++)
	{
		mem = (((ma+x)<<3) + ra) & 0x3fff;
		b = state->m_p_videoram[mem];
		r = state->m_p_videoram[mem+0x4000];
		g = state->m_p_videoram[mem+0x8000];

		for (i = 0; i < 8; i++)
		{
			col = BIT(r, i) + (BIT(g, i) << 1) + (BIT(b, i) << 2);
			if (x == cursor_x) col ^= 7;
			*p++ = palette[col];
		}
	}
}

READ8_MEMBER( fp1100_state::fp1100_mem_r )
{
	if(m_mem_bank == 0 && offset <= 0x8fff)
	{
		UINT8 *ipl = memregion("ipl")->base();
		return ipl[offset];
	}
	return m_wram[offset];
}

WRITE8_MEMBER( fp1100_state::fp1100_mem_w )
{
	m_wram[offset] = data;
}

WRITE8_MEMBER( fp1100_state::main_bank_w )
{
	m_mem_bank = data & 2; //(1) RAM (0) ROM
	m_slot_num = (m_slot_num & 3) | ((data & 1) << 2);
}

WRITE8_MEMBER( fp1100_state::irq_mask_w )
{
	//if((irq_mask & 0x80) != (data & 0x80))
	//  machine().device("sub")->execute().set_input_line(UPD7810_INTF2, HOLD_LINE);

	irq_mask = data;
	///printf("%02x\n",data);
	// FIXME - the below 2 lines are needed, otherwise nothing ever gets written to videoram
	address_space &mem = m_subcpu->space(AS_PROGRAM);
	data = mem.read_byte(0xe800);
}

WRITE8_MEMBER( fp1100_state::main_to_sub_w )
{
	machine().scheduler().synchronize(); // force resync
	machine().device("sub")->execute().set_input_line(UPD7810_INTF2, ASSERT_LINE);
	m_sub_latch = data;
}

READ8_MEMBER( fp1100_state::sub_to_main_r )
{
	machine().scheduler().synchronize(); // force resync
//  machine().device("maincpu")->execute().set_input_line_and_vector(0, CLEAR_LINE, 0xf0);
	return m_main_latch;
}

READ8_MEMBER( fp1100_state::unk_r )
{
	return 0;
}

WRITE8_MEMBER( fp1100_state::slot_bank_w )
{
	m_slot_num = (data & 3) | (m_slot_num & 4);
}

READ8_MEMBER( fp1100_state::slot_id_r )
{
	return m_slot[m_slot_num & 7].id;
}

static ADDRESS_MAP_START(fp1100_map, AS_PROGRAM, 8, fp1100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(fp1100_mem_r,fp1100_mem_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(fp1100_io, AS_IO, 8, fp1100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xff00, 0xff00) AM_READ(slot_id_r) AM_WRITE(slot_bank_w)
	AM_RANGE(0xff80, 0xff80) AM_READ(sub_to_main_r) AM_WRITE(irq_mask_w)
	AM_RANGE(0xffa0, 0xffa0) AM_WRITE(main_bank_w)
	AM_RANGE(0xffc0, 0xffc0) AM_READ(unk_r) AM_WRITE(main_to_sub_w)
ADDRESS_MAP_END

READ8_MEMBER( fp1100_state::fp1100_vram_r )
{
	return m_p_videoram[offset];
}

WRITE8_MEMBER( fp1100_state::fp1100_vram_w )
{
	m_p_videoram[offset] = ~data;
}

READ8_MEMBER( fp1100_state::main_to_sub_r )
{
	machine().scheduler().synchronize(); // force resync
	machine().device("sub")->execute().set_input_line(UPD7810_INTF2, CLEAR_LINE);
	return m_sub_latch;
}

WRITE8_MEMBER( fp1100_state::sub_to_main_w )
{
	machine().scheduler().synchronize(); // force resync
//  machine().device("maincpu")->execute().set_input_line_and_vector(0, ASSERT_LINE, 0xf0);
	m_main_latch = data;
}

static ADDRESS_MAP_START(fp1100_slave_map, AS_PROGRAM, 8, fp1100_state )
	AM_RANGE(0xff80, 0xffff) AM_RAM     /* upd7801 internal RAM */
	AM_RANGE(0x0000, 0x0fff) AM_ROM AM_REGION("sub_ipl",0x0000)
	AM_RANGE(0x1000, 0x1fff) AM_ROM AM_REGION("sub_ipl",0x1000)
	AM_RANGE(0x2000, 0xdfff) AM_READWRITE(fp1100_vram_r,fp1100_vram_w) AM_SHARE("videoram") //vram B/R/G
	AM_RANGE(0xe000, 0xe000) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0xe001, 0xe001) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0xe400, 0xe400) AM_READ_PORT("DSW") AM_WRITENOP // key mux write
	AM_RANGE(0xe800, 0xe800) AM_READ(main_to_sub_r) AM_WRITE(sub_to_main_w)
	AM_RANGE(0xf000, 0xff7f) AM_ROM AM_REGION("sub_ipl",0x2000)
ADDRESS_MAP_END

WRITE8_MEMBER( fp1100_state::portc_w )
{
	if((!(m_upd7801.portc & 8)) && data & 8)
		machine().device("maincpu")->execute().set_input_line_and_vector(0, HOLD_LINE,0xf8); // TODO

	m_upd7801.portc = data;
}

static ADDRESS_MAP_START(fp1100_slave_io, AS_IO, 8, fp1100_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x01, 0x01) AM_READNOP //key R
	AM_RANGE(0x02, 0x02) AM_WRITE(portc_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( fp1100 )
	/* TODO: All of those have unknown meaning */
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSW" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
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

	PORT_START("SLOTS")
	PORT_CONFNAME( 0x0003, 0x0002, "Slot #0" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0001, "ROM" )
	PORT_CONFSETTING(    0x0002, "RAM" )
	PORT_CONFSETTING(    0x0003, "FDC" )
	PORT_CONFNAME( 0x000c, 0x0008, "Slot #1" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0004, "ROM" )
	PORT_CONFSETTING(    0x0008, "RAM" )
	PORT_CONFSETTING(    0x000c, "FDC" )
	PORT_CONFNAME( 0x0030, 0x0020, "Slot #2" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0010, "ROM" )
	PORT_CONFSETTING(    0x0020, "RAM" )
	PORT_CONFSETTING(    0x0030, "FDC" )
	PORT_CONFNAME( 0x00c0, 0x0080, "Slot #3" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0040, "ROM" )
	PORT_CONFSETTING(    0x0080, "RAM" )
	PORT_CONFSETTING(    0x00c0, "FDC" )
	PORT_CONFNAME( 0x0300, 0x0200, "Slot #4" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0100, "ROM" )
	PORT_CONFSETTING(    0x0200, "RAM" )
	PORT_CONFSETTING(    0x0300, "FDC" )
	PORT_CONFNAME( 0x0c00, 0x0800, "Slot #5" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x0400, "ROM" )
	PORT_CONFSETTING(    0x0800, "RAM" )
	PORT_CONFSETTING(    0x0c00, "FDC" )
	PORT_CONFNAME( 0x3000, 0x2000, "Slot #6" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x1000, "ROM" )
	PORT_CONFSETTING(    0x2000, "RAM" )
	PORT_CONFSETTING(    0x3000, "FDC" )
	PORT_CONFNAME( 0xc000, 0x8000, "Slot #7" )
	PORT_CONFSETTING(    0x0000, "(null)" )
	PORT_CONFSETTING(    0x4000, "ROM" )
	PORT_CONFSETTING(    0x8000, "RAM" )
	PORT_CONFSETTING(    0xc000, "FDC" )
INPUT_PORTS_END


void fp1100_state::machine_start()
{
	m_wram = memregion("wram")->base();
}

void fp1100_state::machine_reset()
{
	int i;
	UINT8 slot_type;
	const UINT8 id_type[4] = { 0xff, 0x00, 0x01, 0x04};

	for(i=0;i<8;i++)
	{
		slot_type = (machine().root_device().ioport("SLOTS")->read() >> i*2) & 3;
		m_slot[i].id = id_type[slot_type];
	}
}

static const gfx_layout fp1100_chars_8x8 =
{
	8,8,
	256,
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( fp1100 )
	GFXDECODE_ENTRY( "chargen", 0x0000, fp1100_chars_8x8, 0, 1 )
GFXDECODE_END

static const UPD7810_CONFIG fp1100_slave_cpu_config = { TYPE_7801, NULL };
//static const upd1771_interface scv_upd1771c_config = { DEVCB_LINE( scv_upd1771_ack_w ) };


static MC6845_INTERFACE( mc6845_intf )
{
	"screen",   /* screen we are acting on */
	false,		/* show border area */
	8,          /* number of pixels per video memory address */
	NULL,       /* before pixel update callback */
	fp1100_update_row,      /* row update callback */
	NULL,       /* after pixel update callback */
	DEVCB_NULL, /* callback for display state changes */
	DEVCB_NULL, /* callback for cursor state changes */
	DEVCB_NULL, /* HSYNC callback */
	DEVCB_NULL, /* VSYNC callback */
	NULL        /* update address callback */
};

INTERRUPT_GEN_MEMBER(fp1100_state::fp1100_vblank_irq)
{
	if(irq_mask & 0x10)
		machine().device("maincpu")->execute().set_input_line_and_vector(0, HOLD_LINE, 0xf0);
}

static MACHINE_CONFIG_START( fp1100, fp1100_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_4MHz) //unknown clock
	MCFG_CPU_PROGRAM_MAP(fp1100_map)
	MCFG_CPU_IO_MAP(fp1100_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fp1100_state, fp1100_vblank_irq)

	MCFG_CPU_ADD( "sub", UPD7801, XTAL_4MHz ) //unknown clock
	MCFG_CPU_PROGRAM_MAP( fp1100_slave_map )
	MCFG_CPU_IO_MAP( fp1100_slave_io )
	MCFG_CPU_CONFIG( fp1100_slave_cpu_config )


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", h46505_device, screen_update)
	MCFG_PALETTE_LENGTH(8)
	MCFG_GFXDECODE(fp1100)

	/* Devices */
	MCFG_MC6845_ADD("crtc", H46505, XTAL_4MHz/2, mc6845_intf)   /* hand tuned to get ~60 fps */
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( fp1100 )
	ROM_REGION( 0x9000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "basic.rom", 0x0000, 0x9000, BAD_DUMP CRC(7c7dd17c) SHA1(985757b9c62abd17b0bd77db751d7782f2710ec3))

	ROM_REGION( 0x3000, "sub_ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "sub1.rom", 0x0000, 0x1000, CRC(8feda489) SHA1(917d5b398b9e7b9a6bfa5e2f88c5b99923c3c2a3))
	ROM_LOAD( "sub2.rom", 0x1000, 0x1000, CRC(359f007e) SHA1(0188d5a7b859075cb156ee55318611bd004128d7))
	ROM_LOAD( "sub3.rom", 0x2000, 0xf80, BAD_DUMP CRC(fb2b577a) SHA1(a9ae6b03e06ea2f5db30dfd51ebf5aede01d9672))

	ROM_REGION( 0x0800, "chargen", 0 )
	ROM_COPY( "sub_ipl", 0x2400, 0x0000, 0x0800 )

	ROM_REGION( 0x10000, "wram", ROMREGION_ERASE00 )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE     INPUT     INIT    COMPANY    FULLNAME       FLAGS */
COMP( 1983, fp1100,  0,      0,       fp1100,     fp1100, driver_device,    0,     "Casio",   "FP-1100", GAME_NOT_WORKING | GAME_NO_SOUND)

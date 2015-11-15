// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, Angelo Salese
/***************************************************************************

    QX-10

    Preliminary driver by Mariusz Wojcieszek

    Status:
    Driver boots and load CP/M from floppy image. Needs upd7220 for gfx

    Done:
    - preliminary memory map
    - floppy (upd765)
    - DMA
    - Interrupts (pic8295)

    banking:
    - 0x1c = 0
    AM_RANGE(0x0000,0x1fff) ROM
    AM_RANGE(0x2000,0xdfff) NOP
    AM_RANGE(0xe000,0xffff) RAM
    - 0x1c = 1 0x20 = 1
    AM_RANGE(0x0000,0x7fff) RAM (0x18 selects bank)
    AM_RANGE(0x8000,0x87ff) CMOS
    AM_RANGE(0x8800,0xdfff) NOP or previous bank?
    AM_RANGE(0xe000,0xffff) RAM
    - 0x1c = 1 0x20 = 0
    AM_RANGE(0x0000,0xdfff) RAM (0x18 selects bank)
    AM_RANGE(0xe000,0xffff) RAM
****************************************************************************/


#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "machine/pit8253.h"
#include "machine/pic8259.h"
#include "machine/z80dart.h"
#include "machine/mc146818.h"
#include "machine/i8255.h"
#include "machine/am9517a.h"
#include "video/upd7220.h"
#include "machine/upd765.h"
#include "machine/ram.h"
#include "machine/qx10kbd.h"

#define MAIN_CLK    15974400

#define RS232_TAG   "rs232"

/*
    Driver data
*/

class qx10_state : public driver_device
{
public:
	qx10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_pit_1(*this, "pit8253_1"),
		m_pit_2(*this, "pit8253_2"),
		m_pic_m(*this, "pic8259_master"),
		m_pic_s(*this, "pic8259_slave"),
		m_scc(*this, "upd7201"),
		m_ppi(*this, "i8255"),
		m_dma_1(*this, "8237dma_1"),
		m_dma_2(*this, "8237dma_2"),
		m_fdc(*this, "upd765"),
		m_hgdc(*this, "upd7220"),
		m_rtc(*this, "rtc"),
		m_kbd(*this, "kbd"),
		m_vram_bank(0),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette")
	{
	}

	required_device<pit8253_device> m_pit_1;
	required_device<pit8253_device> m_pit_2;
	required_device<pic8259_device> m_pic_m;
	required_device<pic8259_device> m_pic_s;
	required_device<upd7201_device> m_scc;
	required_device<i8255_device> m_ppi;
	required_device<am9517a_device> m_dma_1;
	required_device<am9517a_device> m_dma_2;
	required_device<upd765a_device> m_fdc;
	required_device<upd7220_device> m_hgdc;
	required_device<mc146818_device> m_rtc;
	required_device<rs232_port_device> m_kbd;
	UINT8 m_vram_bank;
	//required_shared_ptr<UINT8> m_video_ram;
	UINT16 *m_video_ram;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();

	void update_memory_mapping();

	DECLARE_WRITE8_MEMBER( qx10_18_w );
	DECLARE_WRITE8_MEMBER( prom_sel_w );
	DECLARE_WRITE8_MEMBER( cmos_sel_w );
	DECLARE_WRITE_LINE_MEMBER( qx10_upd765_interrupt );
	DECLARE_READ8_MEMBER( fdc_dma_r );
	DECLARE_WRITE8_MEMBER( fdc_dma_w );
	DECLARE_WRITE8_MEMBER( fdd_motor_w );
	DECLARE_READ8_MEMBER( qx10_30_r );
	DECLARE_READ8_MEMBER( gdc_dack_r );
	DECLARE_WRITE8_MEMBER( gdc_dack_w );
	DECLARE_WRITE_LINE_MEMBER( tc_w );
	DECLARE_READ8_MEMBER( mc146818_r );
	DECLARE_WRITE8_MEMBER( mc146818_w );
	DECLARE_READ8_MEMBER( get_slave_ack );
	DECLARE_READ8_MEMBER( vram_bank_r );
	DECLARE_WRITE8_MEMBER( vram_bank_w );
	DECLARE_READ16_MEMBER( vram_r );
	DECLARE_WRITE16_MEMBER( vram_w );
	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(memory_write_byte);
	DECLARE_WRITE_LINE_MEMBER(keyboard_clk);
	DECLARE_WRITE_LINE_MEMBER(keyboard_irq);

	UINT8 *m_char_rom;

	/* FDD */
	int     m_fdcint;
	int     m_fdcmotor;
	int     m_fdcready;

	/* memory */
	int     m_membank;
	int     m_memprom;
	int     m_memcmos;
	UINT8   m_cmosram[0x800];

	UINT8 m_color_mode;

	struct{
		UINT8 rx;
	}m_rs232c;

	DECLARE_PALETTE_INIT(qx10);
	DECLARE_INPUT_CHANGED_MEMBER(key_stroke);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;
	UPD7220_DISPLAY_PIXELS_MEMBER( hgdc_display_pixels );
	UPD7220_DRAW_TEXT_LINE_MEMBER( hgdc_draw_text );
};

UPD7220_DISPLAY_PIXELS_MEMBER( qx10_state::hgdc_display_pixels )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int xi,gfx[3];
	UINT8 pen;

	if(m_color_mode)
	{
		gfx[0] = m_video_ram[((address) + 0x00000) >> 1];
		gfx[1] = m_video_ram[((address) + 0x20000) >> 1];
		gfx[2] = m_video_ram[((address) + 0x40000) >> 1];
	}
	else
	{
		gfx[0] = m_video_ram[(address) >> 1];
		gfx[1] = 0;
		gfx[2] = 0;
	}

	for(xi=0;xi<16;xi++)
	{
		pen = ((gfx[0] >> xi) & 1) ? 1 : 0;
		pen|= ((gfx[1] >> xi) & 1) ? 2 : 0;
		pen|= ((gfx[2] >> xi) & 1) ? 4 : 0;

		bitmap.pix32(y, x + xi) = palette[pen];
	}
}

UPD7220_DRAW_TEXT_LINE_MEMBER( qx10_state::hgdc_draw_text )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int x;
	int xi,yi;
	int tile;
	int attr;
	UINT8 color;
	UINT8 tile_data;
	UINT8 pen;

	for( x = 0; x < pitch; x++ )
	{
		tile = m_video_ram[((addr+x)*2) >> 1] & 0xff;
		attr = m_video_ram[((addr+x)*2) >> 1] >> 8;

		color = (m_color_mode) ? 1 : (attr & 4) ? 2 : 1; /* TODO: color mode */

		for( yi = 0; yi < lr; yi++)
		{
			tile_data = (m_char_rom[tile*16+yi]);

			if(attr & 8)
				tile_data^=0xff;

			if(cursor_on && cursor_addr == addr+x) //TODO
				tile_data^=0xff;

			if(attr & 0x80 && machine().first_screen()->frame_number() & 0x10) //TODO: check for blinking interval
				tile_data=0;

			for( xi = 0; xi < 8; xi++)
			{
				int res_x,res_y;

				res_x = x * 8 + xi;
				res_y = y + yi;

				if(!machine().first_screen()->visible_area().contains(res_x, res_y))
					continue;

				if(yi >= 16)
					pen = 0;
				else
					pen = ((tile_data >> xi) & 1) ? color : 0;

				if(pen)
					bitmap.pix32(res_y, res_x) = palette[pen];
			}
		}
	}
}

UINT32 qx10_state::screen_update( screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect )
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_hgdc->screen_update(screen, bitmap, cliprect);

	return 0;
}

/*
    Memory
*/
void qx10_state::update_memory_mapping()
{
	int drambank = 0;

	if (m_membank & 1)
	{
		drambank = 0;
	}
	else if (m_membank & 2)
	{
		drambank = 1;
	}
	else if (m_membank & 4)
	{
		drambank = 2;
	}
	else if (m_membank & 8)
	{
		drambank = 3;
	}

	if (!m_memprom)
	{
		membank("bank1")->set_base(memregion("maincpu")->base());
	}
	else
	{
		membank("bank1")->set_base(m_ram->pointer() + drambank*64*1024);
	}
	if (m_memcmos)
	{
		membank("bank2")->set_base(m_cmosram);
	}
	else
	{
		membank("bank2")->set_base(m_ram->pointer() + drambank*64*1024 + 32*1024);
	}
}

READ8_MEMBER( qx10_state::fdc_dma_r )
{
	return m_fdc->dma_r();
}

WRITE8_MEMBER( qx10_state::fdc_dma_w )
{
	m_fdc->dma_w(data);
}


WRITE8_MEMBER( qx10_state::qx10_18_w )
{
	m_membank = (data >> 4) & 0x0f;
	update_memory_mapping();
}

WRITE8_MEMBER( qx10_state::prom_sel_w )
{
	m_memprom = data & 1;
	update_memory_mapping();
}

WRITE8_MEMBER( qx10_state::cmos_sel_w )
{
	m_memcmos = data & 1;
	update_memory_mapping();
}

/*
    FDD
*/

static SLOT_INTERFACE_START( qx10_floppies )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

WRITE_LINE_MEMBER( qx10_state::qx10_upd765_interrupt )
{
	m_fdcint = state;

	//logerror("Interrupt from upd765: %d\n", state);
	// signal interrupt
	m_pic_m->ir6_w(state);
}

WRITE8_MEMBER( qx10_state::fdd_motor_w )
{
	m_fdcmotor = 1;

	machine().device<floppy_connector>("upd765:0")->get_device()->mon_w(false);
	// motor off controlled by clock
}

READ8_MEMBER( qx10_state::qx10_30_r )
{
	floppy_image_device *floppy1,*floppy2;

	floppy1 = machine().device<floppy_connector>("upd765:0")->get_device();
	floppy2 = machine().device<floppy_connector>("upd765:1")->get_device();

	return m_fdcint |
			/*m_fdcmotor*/ 0 << 1 |
			((floppy1 != NULL) || (floppy2 != NULL) ? 1 : 0) << 3 |
			m_membank << 4;
}

/*
    DMA8237
*/
WRITE_LINE_MEMBER(qx10_state::dma_hrq_changed)
{
	/* Assert HLDA */
	m_dma_1->hack_w(state);
}

READ8_MEMBER( qx10_state::gdc_dack_r )
{
	logerror("GDC DACK read\n");
	return 0;
}

WRITE8_MEMBER( qx10_state::gdc_dack_w )
{
	logerror("GDC DACK write %02x\n", data);
}

WRITE_LINE_MEMBER( qx10_state::tc_w )
{
	/* floppy terminal count */
	m_fdc->tc_w(!state);
}

/*
    8237 DMA (Master)
    Channel 1: Floppy disk
    Channel 2: GDC
    Channel 3: Option slots
*/
READ8_MEMBER(qx10_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(qx10_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

/*
    8237 DMA (Slave)
    Channel 1: Option slots #1
    Channel 2: Option slots #2
    Channel 3: Option slots #3
    Channel 4: Option slots #4
*/

/*
    MC146818
*/

WRITE8_MEMBER(qx10_state::mc146818_w)
{
	m_rtc->write(space, !offset, data);
}

READ8_MEMBER(qx10_state::mc146818_r)
{
	return m_rtc->read(space, !offset);
}

WRITE_LINE_MEMBER(qx10_state::keyboard_irq)
{
	m_scc->m1_r(); // always set
	m_pic_m->ir4_w(state);
}

WRITE_LINE_MEMBER(qx10_state::keyboard_clk)
{
	// clock keyboard too
	m_scc->rxca_w(state);
	m_scc->txca_w(state);
}

/*
    Master PIC8259
    IR0     Power down detection interrupt
    IR1     Software timer #1 interrupt
    IR2     External interrupt INTF1
    IR3     External interrupt INTF2
    IR4     Keyboard/RS232 interrupt
    IR5     CRT/lightpen interrupt
    IR6     Floppy controller interrupt
    IR7     Slave cascade
*/

READ8_MEMBER( qx10_state::get_slave_ack )
{
	if (offset==7) { // IRQ = 7
		return m_pic_s->acknowledge();
	}
	return 0x00;
}


/*
    Slave PIC8259
    IR0     Printer interrupt
    IR1     External interrupt #1
    IR2     Calendar clock interrupt
    IR3     External interrupt #2
    IR4     External interrupt #3
    IR5     Software timer #2 interrupt
    IR6     External interrupt #4
    IR7     External interrupt #5

*/

READ8_MEMBER( qx10_state::vram_bank_r )
{
	return m_vram_bank;
}

WRITE8_MEMBER( qx10_state::vram_bank_w )
{
	if(m_color_mode)
	{
		m_vram_bank = data & 7;
		if(data != 1 && data != 2 && data != 4)
			printf("%02x\n",data);
	}
}

static ADDRESS_MAP_START(qx10_mem, AS_PROGRAM, 8, qx10_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x7fff ) AM_RAMBANK("bank1")
	AM_RANGE( 0x8000, 0xdfff ) AM_RAMBANK("bank2")
	AM_RANGE( 0xe000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( qx10_io , AS_IO, 8, qx10_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("pit8253_1", pit8253_device, read, write)
	AM_RANGE(0x04, 0x07) AM_DEVREADWRITE("pit8253_2", pit8253_device, read, write)
	AM_RANGE(0x08, 0x09) AM_DEVREADWRITE("pic8259_master", pic8259_device, read, write)
	AM_RANGE(0x0c, 0x0d) AM_DEVREADWRITE("pic8259_slave", pic8259_device, read, write)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE("upd7201", z80dart_device, cd_ba_r, cd_ba_w)
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE("i8255", i8255_device, read, write)
	AM_RANGE(0x18, 0x1b) AM_READ_PORT("DSW") AM_WRITE(qx10_18_w)
	AM_RANGE(0x1c, 0x1f) AM_WRITE(prom_sel_w)
	AM_RANGE(0x20, 0x23) AM_WRITE(cmos_sel_w)
	AM_RANGE(0x2c, 0x2c) AM_READ_PORT("CONFIG")
	AM_RANGE(0x2d, 0x2d) AM_READWRITE(vram_bank_r,vram_bank_w)
	AM_RANGE(0x30, 0x33) AM_READWRITE(qx10_30_r, fdd_motor_w)
	AM_RANGE(0x34, 0x35) AM_DEVICE("upd765", upd765a_device, map)
	AM_RANGE(0x38, 0x39) AM_DEVREADWRITE("upd7220", upd7220_device, read, write)
//  AM_RANGE(0x3a, 0x3a) GDC zoom
//  AM_RANGE(0x3b, 0x3b) GDC light pen req
	AM_RANGE(0x3c, 0x3d) AM_READWRITE(mc146818_r, mc146818_w)
	AM_RANGE(0x40, 0x4f) AM_DEVREADWRITE("8237dma_1", am9517a_device, read, write)
	AM_RANGE(0x50, 0x5f) AM_DEVREADWRITE("8237dma_2", am9517a_device, read, write)
//  AM_RANGE(0xfc, 0xfd) Multi-Font comms
ADDRESS_MAP_END

/* Input ports */
/* TODO: shift break */
/*INPUT_CHANGED_MEMBER(qx10_state::key_stroke)
{
    if(newval && !oldval)
    {
        m_keyb.rx = (UINT8)(FPTR)(param) & 0x7f;
        m_pic_m->ir4_w(1);
    }

    if(oldval && !newval)
        m_keyb.rx = 0;
}*/

static INPUT_PORTS_START( qx10 )
	/* TODO: All of those have unknown meaning */
	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, "DSW" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) ) //CMOS related
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

	PORT_START("CONFIG")
	PORT_CONFNAME( 0x03, 0x02, "Video Board" )
	PORT_CONFSETTING( 0x02, "Monochrome" )
	PORT_CONFSETTING( 0x01, "Color" )
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void qx10_state::machine_start()
{
}

void qx10_state::machine_reset()
{
	m_dma_1->dreq0_w(1);

	m_memprom = 0;
	m_memcmos = 0;
	m_membank = 0;
	update_memory_mapping();

	{
		int i;

		/* TODO: is there a bit that sets this up? */
		m_color_mode = ioport("CONFIG")->read() & 1;

		if(m_color_mode) //color
		{
			for ( i = 0; i < 8; i++ )
				m_palette->set_pen_color(i, pal1bit((i >> 2) & 1), pal1bit((i >> 1) & 1), pal1bit((i >> 0) & 1));
		}
		else //monochrome
		{
			for ( i = 0; i < 8; i++ )
				m_palette->set_pen_color(i, pal1bit(0), pal1bit(0), pal1bit(0));

			m_palette->set_pen_color(1, 0x00, 0x9f, 0x00);
			m_palette->set_pen_color(2, 0x00, 0xff, 0x00);
			m_vram_bank = 0;
		}
	}
}

/* F4 Character Displayer */
static const gfx_layout qx10_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	RGN_FRAC(1,1),          /* 128 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8, 8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16                    /* every char takes 16 bytes */
};

static GFXDECODE_START( qx10 )
	GFXDECODE_ENTRY( "chargen", 0x0000, qx10_charlayout, 1, 1 )
GFXDECODE_END

void qx10_state::video_start()
{
	// allocate memory
	m_video_ram = auto_alloc_array_clear(machine(), UINT16, 0x30000);

	// find memory regions
	m_char_rom = memregion("chargen")->base();
}

PALETTE_INIT_MEMBER(qx10_state, qx10)
{
	// ...
}

READ16_MEMBER( qx10_state::vram_r )
{
	int bank = 0;

	if (m_vram_bank & 1)     { bank = 0; } // B
	else if(m_vram_bank & 2) { bank = 1; } // G
	else if(m_vram_bank & 4) { bank = 2; } // R

	return m_video_ram[offset + (0x20000 * bank)];
}

WRITE16_MEMBER( qx10_state::vram_w )
{
	int bank = 0;

	if (m_vram_bank & 1)     { bank = 0; } // B
	else if(m_vram_bank & 2) { bank = 1; } // G
	else if(m_vram_bank & 4) { bank = 2; } // R

	COMBINE_DATA(&m_video_ram[offset + (0x20000 * bank)]);
}

static ADDRESS_MAP_START( upd7220_map, AS_0, 16, qx10_state )
	AM_RANGE(0x00000, 0x5ffff) AM_READWRITE(vram_r,vram_w)
ADDRESS_MAP_END

static SLOT_INTERFACE_START(keyboard)
	SLOT_INTERFACE("qx10", QX10_KEYBOARD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( qx10, qx10_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, MAIN_CLK / 4)
	MCFG_CPU_PROGRAM_MAP(qx10_mem)
	MCFG_CPU_IO_MAP(qx10_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_master", pic8259_device, inta_cb)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(qx10_state, screen_update)
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", qx10)
	MCFG_PALETTE_ADD("palette", 8)
	MCFG_PALETTE_INIT_OWNER(qx10_state, qx10)

	/* Devices */

/*
    Timer 0
    Counter CLK                         Gate                    OUT             Operation
    0       Keyboard clock (1200bps)    Memory register D0      Speaker timer   Speaker timer (100ms)
    1       Keyboard clock (1200bps)    +5V                     8259A (10E) IR5 Software timer
    2       Clock 1,9668MHz             Memory register D7      8259 (12E) IR1  Software timer
*/

	MCFG_DEVICE_ADD("pit8253_1", PIT8253, 0)
	MCFG_PIT8253_CLK0(1200)
	MCFG_PIT8253_CLK1(1200)
	MCFG_PIT8253_CLK2(MAIN_CLK / 8)

/*
    Timer 1
    Counter CLK                 Gate        OUT                 Operation
    0       Clock 1,9668MHz     +5V         Speaker frequency   1kHz
    1       Clock 1,9668MHz     +5V         Keyboard clock      1200bps (Clock / 1664)
    2       Clock 1,9668MHz     +5V         RS-232C baud rate   9600bps (Clock / 208)
*/
	MCFG_DEVICE_ADD("pit8253_2", PIT8253, 0)
	MCFG_PIT8253_CLK0(MAIN_CLK / 8)
	MCFG_PIT8253_CLK1(MAIN_CLK / 8)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(qx10_state, keyboard_clk))
	MCFG_PIT8253_CLK2(MAIN_CLK / 8)
	MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("upd7201", z80dart_device, rxtxcb_w))

	MCFG_PIC8259_ADD("pic8259_master", INPUTLINE("maincpu", 0), VCC, READ8(qx10_state, get_slave_ack))
	MCFG_PIC8259_ADD("pic8259_slave", DEVWRITELINE("pic8259_master", pic8259_device, ir7_w), GND, NULL)

	MCFG_UPD7201_ADD("upd7201", MAIN_CLK/4, 0, 0, 0, 0) // channel b clock set by pit2 channel 2
	// Channel A: Keyboard
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("kbd", rs232_port_device, write_txd))
	// Channel B: RS232
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_INT_CB(WRITELINE(qx10_state, keyboard_irq))

	MCFG_DEVICE_ADD("8237dma_1", AM9517A, MAIN_CLK/4)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(qx10_state, dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(qx10_state, tc_w))
	MCFG_I8237_IN_MEMR_CB(READ8(qx10_state, memory_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(qx10_state, memory_write_byte))
	MCFG_I8237_IN_IOR_0_CB(READ8(qx10_state, fdc_dma_r))
	MCFG_I8237_IN_IOR_1_CB(READ8(qx10_state, gdc_dack_r))
	//MCFG_I8237_IN_IOR_2_CB(DEVREAD8("upd7220", upd7220_device, dack_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(qx10_state, fdc_dma_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(qx10_state, gdc_dack_w))
	//MCFG_I8237_OUT_IOW_2_CB(DEVWRITE8("upd7220", upd7220_device, dack_w))
	MCFG_DEVICE_ADD("8237dma_2", AM9517A, MAIN_CLK/4)

	MCFG_DEVICE_ADD("i8255", I8255, 0)

	MCFG_DEVICE_ADD("upd7220", UPD7220, MAIN_CLK/6) // unk clock
	MCFG_DEVICE_ADDRESS_MAP(AS_0, upd7220_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(qx10_state, hgdc_display_pixels)
	MCFG_UPD7220_DRAW_TEXT_CALLBACK_OWNER(qx10_state, hgdc_draw_text)
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_MC146818_ADD( "rtc", XTAL_32_768kHz )
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic8259_slave", pic8259_device, ir2_w))
	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(qx10_state, qx10_upd765_interrupt))
	MCFG_UPD765_DRQ_CALLBACK(DEVWRITELINE("8237dma_1", am9517a_device, dreq0_w)) MCFG_DEVCB_INVERT
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", qx10_floppies, "525dd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", qx10_floppies, "525dd", floppy_image_device::default_floppy_formats)

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("upd7201", upd7201_device, rxb_w))

	MCFG_RS232_PORT_ADD("kbd", keyboard, "qx10")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("upd7201", z80dart_device, rxa_w))

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "qx10_flop")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( qx10 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "v006", "v0.06")
	ROMX_LOAD( "ipl006.bin", 0x0000, 0x0800, CRC(3155056a) SHA1(67cc0ae5055d472aa42eb40cddff6da69ffc6553), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v003", "v0.03")
	ROMX_LOAD( "ipl003.bin", 0x0000, 0x0800, CRC(3cbc4008) SHA1(cc8c7d1aa0cca8f9753d40698b2dc6802fd5f890), ROM_BIOS(2))

	/* This is probably the i8039 program ROM for the Q10MF Multifont card, and the actual font ROMs are missing (6 * HM43128) */
	/* The first part of this rom looks like code for an embedded controller?
	    From 0300 on, is a keyboard lookup table */
	ROM_REGION( 0x0800, "i8039", 0 )
	ROM_LOAD( "m12020a.3e", 0x0000, 0x0800, CRC(fa27f333) SHA1(73d27084ca7b002d5f370220d8da6623a6e82132))

	ROM_REGION( 0x1000, "chargen", 0 )
//  ROM_LOAD( "qge.2e",   0x0000, 0x0800, BAD_DUMP CRC(ed93cb81) SHA1(579e68bde3f4184ded7d89b72c6936824f48d10b))  //this one contains special characters only
	ROM_LOAD( "qge.2e",   0x0000, 0x1000, BAD_DUMP CRC(eb31a2d5) SHA1(6dc581bf2854a07ae93b23b6dfc9c7abd3c0569e))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY   FULLNAME       FLAGS */
COMP( 1983, qx10,  0,       0,  qx10,   qx10, driver_device,     0,       "Epson",   "QX-10",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

/***************************************************************************

        NCR Decision Mate V

        04/01/2012 Skeleton driver.

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/upd765.h"
#include "machine/8237dma.h"
#include "video/upd7220.h"
#include "formats/mfi_dsk.h"
#include "dmv.lh"

class dmv_state : public driver_device
{
public:
	dmv_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_hgdc(*this, "upd7220"),
		  m_dmac(*this, "dma8237"),
		  m_fdc(*this, "upd765"),
		  m_floppy0(*this, "upd765:0:525dd"),
		  m_floppy1(*this, "upd765:1:525dd"),
		  m_video_ram(*this, "video_ram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<upd7220_device> m_hgdc;
	required_device<i8237_device> m_dmac;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;

	virtual void video_start();
	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_WRITE8_MEMBER(leds_w);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	DECLARE_WRITE8_MEMBER(fdd_motor_w);
	DECLARE_READ8_MEMBER(sys_status_r);
	DECLARE_READ8_MEMBER(kb_ctrl_mcu_r);
	DECLARE_WRITE8_MEMBER(kb_ctrl_mcu_w);
	DECLARE_READ8_MEMBER(fdc_dma_r);
	DECLARE_WRITE8_MEMBER(fdc_dma_w);

	void fdc_irq(bool state);
	void fdc_drq(bool state);

	required_shared_ptr<UINT8> m_video_ram;
	int 		m_fdc_int_line;
};


WRITE8_MEMBER(dmv_state::leds_w)
{
	/*
        LEDs    Value       Significance
        ---------------------------------------
        None    0xFF        Check complete
        1+8     0x7E        Sumcheck error
        2+8     0xBE        GDC error
        3+8     0xDE        Disk drive error
        4+8     0xEE        16-bit processor error
        5+8     0xF6        Keyboard error
        6+8     0xFA        DMA error
        7+8     0xFC        Memory error
        All     0x00        Processor error
    */

	for(int i=0; i<8; i++)
		output_set_led_value(8-i, BIT(data, i));
}

void dmv_state::fdc_irq(bool state)
{
	m_fdc_int_line = state;
}

void dmv_state::fdc_drq(bool state)
{
	m_dmac->i8237_drq_write(3, state);
}

READ8_MEMBER(dmv_state::fdc_dma_r)
{
	return m_fdc->dma_r();
}

WRITE8_MEMBER(dmv_state::fdc_dma_w)
{
	m_fdc->dma_w(data);
}

WRITE8_MEMBER(dmv_state::fdd_motor_w)
{
	// bit 0 defines the state of the FDD motor

	m_floppy0->mon_w(!BIT(data, 0));
	m_floppy1->mon_w(!BIT(data, 0));
}

READ8_MEMBER(dmv_state::sys_status_r)
{
	/*
        Main system status
        x--- ---- FDD index
        -x--- --- IRQ 2
        --x--- -- IRQ 3
        ---x--- - IRQ 4
        ---- x--- FDC interrupt
        ---- -x-- FDD ready
        ---- --x- 16-bit CPU available (active low)
        ---- ---x FDD motor (active low)
    */
	UINT8 data = 0x00;

	// 16-bit CPU not available
	data |= 0x02;

	if (m_fdc_int_line)
		data |= 0x08;

	return data;
}

READ8_MEMBER(dmv_state::kb_ctrl_mcu_r)
{
	return upi41_master_r(machine().device("kb_ctrl_mcu"), offset);
}

WRITE8_MEMBER(dmv_state::kb_ctrl_mcu_w)
{
	upi41_master_w(machine().device("kb_ctrl_mcu"), offset, data);
}

static UPD7220_DISPLAY_PIXELS( hgdc_display_pixels )
{
	//TODO
}

static UPD7220_DRAW_TEXT_LINE( hgdc_draw_text )
{
	dmv_state *state = device->machine().driver_data<dmv_state>();
	UINT8 * chargen = state->memregion("maincpu")->base() + 0x1000;

	for( int x = 0; x < pitch; x++ )
	{
		UINT8 tile = state->m_video_ram[((addr+x)*2) & 0x1ffff] & 0xff;

		for( int yi = 0; yi < lr; yi++)
		{
			UINT8 tile_data = chargen[(tile*16+yi) & 0x7ff];

			if(cursor_on && cursor_addr == addr+x) //TODO
				tile_data^=0xff;

			for( int xi = 0; xi < 8; xi++)
			{
				int res_x,res_y;
				int pen = (tile_data >> xi) & 1 ? 1 : 0;

				if(yi >= 16) { pen = 0; }

				res_x = x * 8 + xi;
				res_y = y * lr + yi;

				if(res_x > screen_max_x || res_y > screen_max_y)
					continue;

				bitmap.pix16(res_y, res_x) = pen;
			}
		}
	}
}

static SLOT_INTERFACE_START( dmv_floppies )
	 SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
SLOT_INTERFACE_END

static ADDRESS_MAP_START(dmv_mem, AS_PROGRAM, 8, dmv_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x1fff ) AM_ROM
	AM_RANGE( 0x2000, 0xffff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( dmv_io , AS_IO, 8, dmv_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(leds_w)
	AM_RANGE(0x13, 0x13) AM_READ(sys_status_r)
	AM_RANGE(0x14, 0x14) AM_WRITE(fdd_motor_w)
	AM_RANGE(0x20, 0x2f) AM_DEVREADWRITE_LEGACY("dma8237", i8237_r, i8237_w)
	AM_RANGE(0x40, 0x41) AM_READWRITE(kb_ctrl_mcu_r, kb_ctrl_mcu_w)
	AM_RANGE(0x50, 0x51) AM_DEVICE("upd765", upd765a_device, map)
	AM_RANGE(0xa0, 0xa1) AM_DEVREADWRITE("upd7220", upd7220_device, read, write)

	//AM_RANGE(0x10, 0x11) boot ROM bankswitch (0x0000-0x1fff)
	//AM_RANGE(0x12, 0x12) pulse FDC TC line
	//AM_RANGE(0x80, 0x83) PIT8253
	//AM_RANGE(0xe0, 0xe7) RAM bankswitch
ADDRESS_MAP_END

static ADDRESS_MAP_START( dmv_keyboard_io, AS_IO, 8, dmv_state )
	//AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) keyboard rows input
	//AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) bits 0-3 kb cols out
ADDRESS_MAP_END

static ADDRESS_MAP_START( dmv_kb_ctrl_io, AS_IO, 8, dmv_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_NOP	// bit 0 data from kb, bit 1 data to kb
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_NOP
ADDRESS_MAP_END

static ADDRESS_MAP_START( upd7220_map, AS_0, 8, dmv_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1ffff)
	AM_RANGE(0x00000, 0x1ffff) AM_RAM  AM_SHARE("video_ram")
ADDRESS_MAP_END

/* Input ports */
INPUT_PORTS_START( dmv )
INPUT_PORTS_END

void dmv_state::machine_start()
{
	m_fdc->setup_intrq_cb(upd765a_device::line_cb(FUNC(dmv_state::fdc_irq), this));
	m_fdc->setup_drq_cb(upd765a_device::line_cb(FUNC(dmv_state::fdc_drq), this));
}

void dmv_state::machine_reset()
{
}

void dmv_state::video_start()
{
}

/* F4 Character Displayer */
static const gfx_layout dmv_charlayout =
{
	8, 16,					/* 8 x 16 characters */
	128,					/* 128 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	/* y offsets */
	{ STEP16(0,8) },
	8*16					/* every char takes 16 bytes */
};

static GFXDECODE_START( dmv )
	GFXDECODE_ENTRY("maincpu", 0x1000, dmv_charlayout, 0, 1)
GFXDECODE_END


static UPD7220_INTERFACE( hgdc_intf )
{
	"screen",
	hgdc_display_pixels,
	hgdc_draw_text,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//------------------------------------------------------------------------------------
//   I8237_INTERFACE
//------------------------------------------------------------------------------------

WRITE_LINE_MEMBER( dmv_state::dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	// Assert HLDA
	i8237_hlda_w(m_dmac, state);
}

static UINT8 memory_read_byte(address_space &space, offs_t address, UINT8 mem_mask) 			{ return space.read_byte(address); }
static void memory_write_byte(address_space &space, offs_t address, UINT8 data, UINT8 mem_mask) { space.write_byte(address, data); }

static I8237_INTERFACE( dmv_dma8237_config )
{
	DEVCB_DRIVER_LINE_MEMBER(dmv_state, dma_hrq_changed),
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_MEMBER(dmv_state, fdc_dma_r) },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_DRIVER_MEMBER(dmv_state, fdc_dma_w) },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL }
};


static MACHINE_CONFIG_START( dmv, dmv_state )
    /* basic machine hardware */
    MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
    MCFG_CPU_PROGRAM_MAP(dmv_mem)
    MCFG_CPU_IO_MAP(dmv_io)

	MCFG_CPU_ADD("kb_ctrl_mcu", I8741, XTAL_6MHz)
	MCFG_CPU_IO_MAP(dmv_kb_ctrl_io)

	MCFG_CPU_ADD("keyboard_mcu", I8741, XTAL_6MHz)
	MCFG_CPU_IO_MAP(dmv_keyboard_io)
	MCFG_DEVICE_DISABLE()

    /* video hardware */
    MCFG_SCREEN_ADD("screen", RASTER)
    MCFG_SCREEN_REFRESH_RATE(50)
    MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
    MCFG_SCREEN_UPDATE_DEVICE("upd7220", upd7220_device, screen_update)
    MCFG_SCREEN_SIZE(640, 480)
    MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)

	MCFG_GFXDECODE(dmv)
    MCFG_PALETTE_LENGTH(2)
    MCFG_PALETTE_INIT(black_and_white)
	MCFG_DEFAULT_LAYOUT(layout_dmv)

	// devices
	MCFG_UPD7220_ADD( "upd7220", XTAL_4MHz, hgdc_intf, upd7220_map )
	MCFG_I8237_ADD( "dma8237", XTAL_4MHz, dmv_dma8237_config )
	MCFG_UPD765A_ADD( "upd765", true, true )
	MCFG_FLOPPY_DRIVE_ADD("upd765:0", dmv_floppies, "525dd", 0, floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", dmv_floppies, "525dd", 0, floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dmv )
    ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dmv_norm.bin", 0x0000, 0x2000, CRC(bf25f3f0) SHA1(0c7dd37704db4799e340cc836f887cd543e5c964))

	ROM_REGION(0x400, "kb_ctrl_mcu", ROMREGION_ERASEFF)
	ROM_LOAD( "dmv_kb_ctrl_mcu.bin", 0x0000, 0x0400, CRC(a03af298) SHA1(144cba41294c46f5ca79b7ad8ced0e4408168775))

	 // i8741/8041 microcontroller inside the Keyboard
    ROM_REGION(0x400, "keyboard_mcu", ROMREGION_ERASEFF)
    ROM_LOAD( "dmv_kbmcu.bin", 0x0000, 0x0400, CRC(14e376de) SHA1 (ed09048ef03c602dba17ad6fcfe125c082c9bb17))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME             FLAGS */
COMP( 1984, dmv,	0,       0, 		dmv,	dmv, driver_device,	 0, 	 "NCR",   "Decision Mate V",	GAME_NOT_WORKING | GAME_NO_SOUND)


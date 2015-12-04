// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        TIM-011

        04/09/2010 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "machine/upd765.h"

#define FDC9266_TAG "u43"

class tim011_state : public driver_device
{
public:
	tim011_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_fdc(*this, FDC9266_TAG),
			m_floppy0(*this, FDC9266_TAG ":0:35dd"),
			m_floppy1(*this, FDC9266_TAG ":1:35dd"),
			m_floppy2(*this, FDC9266_TAG ":2:35dd"),
			m_floppy3(*this, FDC9266_TAG ":3:35dd") { }

	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_tim011(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(print_w);
	DECLARE_WRITE8_MEMBER(scroll_w);
	DECLARE_WRITE8_MEMBER(fdc_dma_w);
	DECLARE_READ8_MEMBER(print_r);
	DECLARE_READ8_MEMBER(scroll_r);
	UINT8 m_scroll;

	required_device<cpu_device> m_maincpu;
	required_device<upd765a_device> m_fdc;
	required_device<floppy_image_device> m_floppy0;
	required_device<floppy_image_device> m_floppy1;
	required_device<floppy_image_device> m_floppy2;
	required_device<floppy_image_device> m_floppy3;
};


static ADDRESS_MAP_START(tim011_mem, AS_PROGRAM, 8, tim011_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x01fff) AM_ROM AM_MIRROR(0x3e000)
	AM_RANGE(0x40000, 0x7ffff) AM_RAM // 256KB RAM  8 * 41256 DRAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(tim011_io, AS_IO, 8, tim011_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x007f) AM_RAM /* Z180 internal registers */
	AM_RANGE(0x0080, 0x009f) AM_DEVICE(FDC9266_TAG, upd765a_device, map)
	//AM_RANGE(0x00a0, 0x00a0) AM_MIRROR(0x001f)  AM_WRITE(fdc_dma_w)
	//AM_RANGE(0x00c0, 0x00c1) AM_MIRROR(0x000e)  AM_READWRITE(print_r,print_w)
	//AM_RANGE(0x00d0, 0x00d0) AM_MIRROR(0x000f)  AM_READWRITE(scroll_r,scroll_w)
	AM_RANGE(0x8000, 0xffff) AM_RAM // Video RAM 43256 SRAM  (32KB)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( tim011 )
INPUT_PORTS_END

void tim011_state::machine_reset()
{
	// motor is actually connected on TXS pin of CPU
	m_floppy0->mon_w(0);
	m_floppy1->mon_w(0);
	m_floppy2->mon_w(0);
	m_floppy3->mon_w(0);
}

void tim011_state::video_start()
{
}

UINT32 tim011_state::screen_update_tim011(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

WRITE8_MEMBER(tim011_state::fdc_dma_w)
{
	printf("fdc_dma_w :%02x\n",data);
}

WRITE8_MEMBER(tim011_state::print_w)
{
	//printf("print_w :%02x\n",data);
}

READ8_MEMBER(tim011_state::print_r)
{
	//printf("print_r\n");
	return 0;
}

WRITE8_MEMBER(tim011_state::scroll_w)
{
	m_scroll = data;
}

READ8_MEMBER(tim011_state::scroll_r)
{
	return m_scroll;
}

static SLOT_INTERFACE_START( tim011_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

static const floppy_format_type tim011_floppy_formats[] = {
	FLOPPY_IMD_FORMAT,
	FLOPPY_MFI_FORMAT,
	FLOPPY_MFM_FORMAT,
	nullptr
};

static MACHINE_CONFIG_START( tim011,tim011_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z180, XTAL_12_288MHz / 2) // location U17 HD64180
	MCFG_CPU_PROGRAM_MAP(tim011_mem)
	MCFG_CPU_IO_MAP(tim011_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tim011_state, irq0_line_hold)

//  MCFG_CPU_ADD("keyboard",CDP1802, XTAL_1_75MHz) // CDP1802, uknown clock

	// FDC9266 location U43 XTAL_8MHz
	MCFG_UPD765A_ADD(FDC9266_TAG, true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE("maincpu", INPUT_LINE_IRQ2))

	/* floppy drives */
	MCFG_FLOPPY_DRIVE_ADD(FDC9266_TAG ":0", tim011_floppies, "35dd", tim011_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC9266_TAG ":1", tim011_floppies, "35dd", tim011_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC9266_TAG ":2", tim011_floppies, "35dd", tim011_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC9266_TAG ":3", tim011_floppies, "35dd", tim011_floppy_formats)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(tim011_state, screen_update_tim011)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tim011 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "sys_tim011.u16", 0x0000, 0x2000, CRC(5b4f1300) SHA1(d324991c4292d7dcde8b8d183a57458be8a2be7b))
	ROM_REGION( 0x10000, "keyboard", ROMREGION_ERASEFF )
	ROM_LOAD( "keyb_tim011.bin", 0x0000, 0x1000, CRC(a99c40a6) SHA1(d6d505271d91df4e079ec3c0a4abbe75ae9d649b))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                FULLNAME       FLAGS */
COMP( 1987, tim011, 0,      0,       tim011,    tim011, driver_device,  0, "Mihajlo Pupin Institute", "TIM-011", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

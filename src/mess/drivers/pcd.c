/***************************************************************************

    Siemens PC-D

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

    Skeleton driver

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/mc2661.h"
#include "machine/omti5100.h"
#include "machine/wd_fdc.h"
#include "machine/mc146818.h"
#include "sound/speaker.h"
#include "video/scn2674.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pcd_state : public driver_device
{
public:
	pcd_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pic1(*this, "pic1"),
	m_pic2(*this, "pic2"),
	m_speaker(*this, "speaker"),
	m_sasi(*this, "sasi"),
	m_fdc(*this, "fdc"),
	m_rtc(*this, "rtc"),
	m_crtc(*this, "crtc"),
	m_vram(*this, "vram"),
	m_charram(8*1024)
	{ }

	DECLARE_READ8_MEMBER( irq_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( timer0_tick );
	DECLARE_WRITE_LINE_MEMBER( i186_timer1_w );

	DECLARE_READ8_MEMBER( charram_r );
	DECLARE_WRITE8_MEMBER( charram_w );
	DECLARE_READ16_MEMBER( nmi_io_r );
	DECLARE_WRITE16_MEMBER( nmi_io_w );
	DECLARE_READ8_MEMBER( rtc_r );
	DECLARE_WRITE8_MEMBER( rtc_w );
	DECLARE_READ8_MEMBER( stat_r );
	DECLARE_WRITE8_MEMBER( stat_w );
	DECLARE_READ8_MEMBER( led_r );
	DECLARE_WRITE8_MEMBER( led_w );
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<speaker_sound_device> m_speaker;
	required_device<omti5100_device> m_sasi;
	required_device<wd2793_t> m_fdc;
	required_device<mc146818_device> m_rtc;
	required_device<scn2674_device> m_crtc;
	required_shared_ptr<UINT16> m_vram;
	dynamic_buffer m_charram;
	UINT8 m_stat, m_led;
};


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

static const gfx_layout pcd_charlayout =
{
	8, 14,                   /* 8 x 14 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8 },
	8*16
};
void pcd_state::machine_start()
{
	machine().device<gfxdecode_device>("gfxdecode")->set_gfx(0, global_alloc(gfx_element(machine().device<palette_device>("palette"), pcd_charlayout, m_charram, 0, 2, 0)));
}

void pcd_state::machine_reset()
{
	m_stat = 0;
	m_led = 0;
}

READ8_MEMBER( pcd_state::irq_callback )
{
	return (offset ? m_pic2 : m_pic1)->acknowledge();
}

TIMER_DEVICE_CALLBACK_MEMBER( pcd_state::timer0_tick )
{
	m_maincpu->tmrin0_w(0);
	m_maincpu->tmrin0_w(1);
}

WRITE_LINE_MEMBER( pcd_state::i186_timer1_w )
{
	m_speaker->level_w(state);
}

READ8_MEMBER( pcd_state::charram_r )
{
	return m_charram[offset >> 1];
}

WRITE8_MEMBER( pcd_state::charram_w )
{
	m_charram[offset >> 1] = data;
}

READ16_MEMBER( pcd_state::nmi_io_r )
{
	if(space.debugger_access())
		return 0;
	logerror("%s: unmapped %s %04x\n", machine().describe_context(), space.name(), offset << 1);
	m_stat |= 8;
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	return 0;
}

WRITE16_MEMBER( pcd_state::nmi_io_w )
{
	if(space.debugger_access())
		return;
	logerror("%s: unmapped %s %04x\n", machine().describe_context(), space.name(), offset << 1);
	m_stat |= 8;
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

READ8_MEMBER( pcd_state::rtc_r )
{
	m_rtc->write(space, 0, offset);
	return m_rtc->read(space, 1);
}

WRITE8_MEMBER( pcd_state::rtc_w )
{
	m_rtc->write(space, 0, offset);
	m_rtc->write(space, 1, data);
}

READ8_MEMBER( pcd_state::stat_r )
{
	return m_stat;
}

WRITE8_MEMBER( pcd_state::stat_w )
{
	m_stat = data;
}

READ8_MEMBER( pcd_state::led_r )
{
	return m_led;
}

WRITE8_MEMBER( pcd_state::led_w )
{
	for(int i = 0; i < 6; i++)
		logerror("%c", (data & (1 << i)) ? '-' : '*');
	logerror("\n");
	m_led = data;
}

UINT32 pcd_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//bitmap.fill(0, cliprect);
	m_crtc->scn2574_draw(machine(), bitmap, cliprect, m_vram);
	return 0;
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( pcd_map, AS_PROGRAM, 16, pcd_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM // fixed 256k for now
	AM_RANGE(0xf0000, 0xf7fff) AM_RAM AM_SHARE("vram")
	//AM_RANGE(0xf7000, 0xfbfff) AM_READWRITE8(charram_r, charram_w, 0xffff)
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE(nmi_io_r, nmi_io_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcd_io, AS_IO, 16, pcd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf800, 0xf801) AM_DEVREADWRITE8("pic1", pic8259_device, read, write, 0xffff)
	AM_RANGE(0xf820, 0xf821) AM_DEVREADWRITE8("pic2", pic8259_device, read, write, 0xffff)
	AM_RANGE(0xf840, 0xf841) AM_READWRITE8(stat_r, stat_w, 0x00ff)
	AM_RANGE(0xf840, 0xf841) AM_READWRITE8(led_r, led_w, 0xff00)
	AM_RANGE(0xf880, 0xf8bf) AM_READWRITE8(rtc_r, rtc_w, 0xffff)
	AM_RANGE(0xf900, 0xf907) AM_DEVREADWRITE8("fdc", wd2793_t, read, write, 0xffff)
	//AM_RANGE(0xf940, 0xf943) scsi
	AM_RANGE(0xf9c0, 0xf9c3) AM_DEVREADWRITE8("usart1",mc2661_device,read,write,0xffff)  // UARTs
	AM_RANGE(0xf9d0, 0xf9d3) AM_DEVREADWRITE8("usart2",mc2661_device,read,write,0xffff)
	AM_RANGE(0xf9e0, 0xf9e3) AM_DEVREADWRITE8("usart3",mc2661_device,read,write,0xffff)
	AM_RANGE(0xf980, 0xf987) AM_DEVWRITE8("crtc", scn2674_device, mpu4_vid_scn2674_w, 0x00ff)
	AM_RANGE(0xf980, 0xf987) AM_DEVREAD8("crtc", scn2674_device, mpu4_vid_scn2674_r, 0xff00)
//	AM_RANGE(0xfa00, 0xfa7f) // pcs4-n (peripheral chip select)
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(nmi_io_r, nmi_io_w)
ADDRESS_MAP_END


//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static SLOT_INTERFACE_START( pcd_floppies )
	SLOT_INTERFACE("55f", TEAC_FD_55F)
	SLOT_INTERFACE("55g", TEAC_FD_55G)
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( pcd, pcd_state )
	MCFG_CPU_ADD("maincpu", I80186, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(pcd_map)
	MCFG_CPU_IO_MAP(pcd_io)
	MCFG_80186_TMROUT1_HANDLER(WRITELINE(pcd_state, i186_timer1_w))
	MCFG_80186_IRQ_SLAVE_ACK(READ8(pcd_state, irq_callback))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer0_tick", pcd_state, timer0_tick, attotime::from_hz(XTAL_16MHz / 24)) // adjusted to pass post

	MCFG_PIC8259_ADD("pic1", DEVWRITELINE("maincpu", i80186_cpu_device, int0_w), VCC, NULL)
	MCFG_PIC8259_ADD("pic2", DEVWRITELINE("maincpu", i80186_cpu_device, int1_w), VCC, NULL)

#if 0
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256K")
	MCFG_RAM_EXTRA_OPTIONS("512K,1M")
#endif

	// nvram
	MCFG_NVRAM_ADD_1FILL("nvram")

	// sasi controller
	MCFG_OMTI5100_ADD("sasi")

	// floppy disk controller
	MCFG_WD2793x_ADD("fdc", XTAL_16MHz/8)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("pic1", pic8259_device, ir6_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, drq1_w))

	// floppy drives
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pcd_floppies, "55g", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pcd_floppies, "55g", floppy_image_device::default_floppy_formats)

	// usart
	MCFG_DEVICE_ADD("usart1", MC2661, XTAL_4_9152MHz)
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir2_w))
	MCFG_DEVICE_ADD("usart2", MC2661, XTAL_4_9152MHz)
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir3_w))
	MCFG_DEVICE_ADD("usart3", MC2661, XTAL_4_9152MHz)
	MCFG_MC2661_TXEMT_DSCHG_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir4_w))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(640, 350)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 349)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DRIVER(pcd_state, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_SCN2674_VIDEO_ADD("crtc", 0, NULL);
	MCFG_SCN2674_GFXDECODE("gfxdecode")
	MCFG_SCN2674_PALETTE("palette")

	// rtc
	MCFG_MC146818_ADD("rtc", XTAL_32_768kHz)
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir7_w))
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pcd )
	ROM_REGION(0x4000, "bios", 0)
	ROM_LOAD16_BYTE("s26361-d359.d42", 0x0001, 0x2000, CRC(e20244dd) SHA1(0ebc5ddb93baacd9106f1917380de58aac64fe73))
	ROM_LOAD16_BYTE("s26361-d359.d43", 0x0000, 0x2000, CRC(e03db2ec) SHA1(fcae8b0c9e7543706817b0a53872826633361fda))
	ROM_FILL(0xb64, 1, 0xe2)  // post expects 0xd0 fdc command to be instant, give it a delay
	ROM_FILL(0xb65, 1, 0xfe)
	ROM_FILL(0x3ffe, 1, 0xb4)  // fix csum
	ROM_FILL(0x3fff, 1, 0x22)

	// gfx card (scn2674 with 8741), to be moved
	ROM_REGION(0x400, "graphics", 0)
	ROM_LOAD("s36361-d321-v1.bin", 0x000, 0x400, CRC(69baeb2a) SHA1(98b9cd0f38c51b4988a3aed0efcf004bedd115ff))

	// keyboard (8035), to be moved
	ROM_REGION(0x1000, "keyboard", 0)
	ROM_LOAD("pcd_keyboard.bin", 0x0000, 0x1000, CRC(d227d6cb) SHA1(3d6140764d3d043428c941826370ebf1597c63bd))
ROM_END


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1984, pcd, 0, 0, pcd, 0, driver_device, 0, "Siemens", "PC-D", GAME_NOT_WORKING )

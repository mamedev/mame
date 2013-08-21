/************************************************************************************************************

Intertec SuperBrain

2013-08-19 Skeleton

Chips: 2x Z80; FD1791; 2x 8251; 8255; BR1941; CRT8002; KR3600; DP8350
Xtals: 16.0, 10.92, 5.0688
Disk parameters: 512 bytes x 10 sectors x 35 tracks. 1 and 2-sided disks supported.
Sound: Beeper

*************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
//#include "machine/serial.h"
//#include "machine/wd_fdc.h"
//#include "machine/i8251.h"
//#include "machine/i8255.h"
//#include "sound/beep.h"

class sbrain_state : public driver_device
{
public:
	sbrain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		//, m_beep(*this, "beeper")
		//, m_brg(*this, "brg")
		//, m_u0(*this, "uart0")
		//, m_u1(*this, "uart1")
		//, m_ppi(*this, "ppi")
		//, m_fdc (*this, "fdc")
		//, m_floppy0(*this, "fdc:0")
		//, m_floppy1(*this, "fdc:1")
	{ }

public:
	const UINT8 *m_p_chargen;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_shared_ptr<const UINT8> m_p_videoram;
	DECLARE_DRIVER_INIT(sbrain);
	DECLARE_MACHINE_RESET(sbrain);
private:
	//floppy_image_device *m_floppy;
	//void fdc_intrq_w(bool state);
	//void fdc_drq_w(bool state);
	required_device<cpu_device> m_maincpu;
	//required_device<beep_device> m_beep;
	//required_device<com8116_device> m_brg;
	//required_device<i8251_device> m_uart0;
	//required_device<i8251_device> m_uart1;
	//required_device<i8255_device> m_ppi;
	//required_device<fd1791_t> m_fdc;
	//required_device<floppy_connector> m_floppy0;
	//required_device<floppy_connector> m_floppy1;
};

static ADDRESS_MAP_START( sbrain_mem, AS_PROGRAM, 8, sbrain_state )
	AM_RANGE( 0x0000, 0x3fff ) AM_READ_BANK("bankr0") AM_WRITE_BANK("bankw0")
	AM_RANGE( 0x4000, 0x7fff ) AM_RAM
	AM_RANGE( 0x8000, 0xbfff ) AM_RAMBANK("bank2")
	AM_RANGE( 0xc000, 0xf7ff ) AM_RAM
	AM_RANGE( 0xf800, 0xffff ) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sbrain_io, AS_IO, 8, sbrain_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x40, 0x40) MIRROR(6) AM_DEVREADWRITE("uart0", i8251_device, data_r, data_w)
	//AM_RANGE(0x41, 0x41) MIRROR(6) AM_DEVREADWRITE("uart0", i8251_device, status_r, control_w)
	//AM_RANGE(0x48, 0x4f) chr_int_latch
	//AM_RANGE(0x50, 0x57) key_in
	//AM_RANGE(0x58, 0x58) MIRROR(6) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	//AM_RANGE(0x59, 0x59) MIRROR(6) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	//AM_RANGE(0x60, 0x67) AM_WRITE(baud_w)
	//AM_RANGE(0x68, 0x6b) MIRROR(4) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sbrain_submem, AS_PROGRAM, 8, sbrain_state )
	AM_RANGE( 0x0000, 0x07ff ) AM_ROM
	AM_RANGE( 0x8800, 0x8fff ) AM_RAM AM_REGION("subcpu", 0x8800)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sbrain_subio, AS_IO, 8, sbrain_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x40, 0x40) MIRROR(6) AM_DEVREADWRITE("uart0", i8251_device, data_r, data_w)
	//AM_RANGE(0x41, 0x41) MIRROR(6) AM_DEVREADWRITE("uart0", i8251_device, status_r, control_w)
	//AM_RANGE(0x48, 0x4f) chr_int_latch
	//AM_RANGE(0x50, 0x57) key_in
	//AM_RANGE(0x58, 0x58) MIRROR(6) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	//AM_RANGE(0x59, 0x59) MIRROR(6) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	//AM_RANGE(0x60, 0x67) AM_WRITE(baud_w)
	//AM_RANGE(0x68, 0x6b) MIRROR(4) AM_DEVREADWRITE("ppi", i8255_device, read, write)
ADDRESS_MAP_END

//WRITE8_MEMBER( sbrain_state::baud_w )
//{
//	m_brg->str_w(data & 0x0f);
//	m_brg->stt_w(data >> 4);
//}

static INPUT_PORTS_START( sbrain )
INPUT_PORTS_END

DRIVER_INIT_MEMBER( sbrain_state, sbrain )
{
	//m_fdc->setup_intrq_cb(fd1791_t::line_cb(FUNC(sbrain_state::fdc_intrq_w), this));
	//m_fdc->setup_drq_cb(fd1791_t::line_cb(FUNC(sbrain_state::fdc_drq_w), this));

	UINT8 *main = memregion("maincpu")->base();
	UINT8 *sub = memregion("subcpu")->base();

	membank("bankr0")->configure_entry(0, &sub[0x0000]);
	membank("bankr0")->configure_entry(1, &main[0x0000]);
	membank("bankw0")->configure_entry(0, &main[0x0000]);
	membank("bank2")->configure_entry(0, &main[0x8000]);
	membank("bank2")->configure_entry(1, &sub[0x8000]);
}
#if 0
static const rs232_port_interface rs232_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

static const i8251_interface u0_intf =
{
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, rx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, tx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, dsr_r),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, dtr_w),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, rts_w),
	DEVCB_DRIVER_LINE_MEMBER(sbrain_state, sio_rxrdy_w),
	DEVCB_DRIVER_LINE_MEMBER(sbrain_state, sio_txrdy_w),
	DEVCB_NULL,
	DEVCB_NULL
};

static const i8251_interface u1_intf =
{
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, rx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", serial_port_device, tx),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, dsr_r),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, dtr_w),
	DEVCB_DEVICE_LINE_MEMBER("rs232", rs232_port_device, rts_w),
	DEVCB_DRIVER_LINE_MEMBER(sbrain_state, sio_rxrdy_w),
	DEVCB_DRIVER_LINE_MEMBER(sbrain_state, sio_txrdy_w),
	DEVCB_NULL,
	DEVCB_NULL
};
#endif
MACHINE_RESET_MEMBER( sbrain_state, sbrain )
{
	m_p_chargen = memregion("chargen")->base();
	membank("bankr0")->set_entry(0);
	membank("bankw0")->set_entry(0);
	membank("bank2")->set_entry(0);
	UINT8 *sub = memregion("subcpu")->base();
	sub[0x8800] = 0x55;
	sub[0x8801] = 0xAA;
}

UINT32 sbrain_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	UINT8 y,ra,chr,gfx;
	UINT16 sy=0,ma=0,x;

	for (y = 0; y < 24; y++)
	{
		for (ra = 0; ra < 10; ra++)
		{
			UINT16 *p = &bitmap.pix16(sy++);

			for (x = 0; x < 80; x++)
			{
				gfx = 0;
				if (ra < 9)
				{
					chr = m_p_videoram[x+ma];

					gfx = m_p_chargen[(chr<<4) | ra ];
				}
				/* Display a scanline of a character */
				*p++ = BIT(gfx, 7);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 0);
			}
		}
		ma+=160;
	}
	return 0;
}

static MACHINE_CONFIG_START( sbrain, sbrain_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(sbrain_mem)
	MCFG_CPU_IO_MAP(sbrain_io)
	MCFG_MACHINE_RESET_OVERRIDE(sbrain_state, sbrain)
	MCFG_CPU_ADD("subcpu", Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(sbrain_submem)
	MCFG_CPU_IO_MAP(sbrain_subio)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_UPDATE_DRIVER(sbrain_state, screen_update)
	MCFG_SCREEN_SIZE(640, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 239)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT_OVERRIDE(driver_device, monochrome_amber)

	/* sound hardware */
	//MCFG_SPEAKER_STANDARD_MONO("mono")
	//MCFG_SOUND_ADD("beeper", BEEP, 0)
	//MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* Devices */
	//MCFG_I8255_ADD("ppi", ppi_intf)
	//MCFG_I8251_ADD("uart0", u0_intf)
	//MCFG_I8251_ADD("uart1", u1_intf)
	//MCFG_COM8116_ADD("brg", XTAL_5_0688MHz, NULL, WRITELINE(sbrain_state, fr_w), WRITELINE(sbrain_state, ft_w))
	//MCFG_RS232_PORT_ADD("rs232", rs232_intf, default_rs232_devices, "serial_terminal")
	//MCFG_FD1791x_ADD("fdc", XTAL_8MHz / 8)
	//MCFG_FLOPPY_DRIVE_ADD("fdc:0", altos5_floppies, "525dd", floppy_image_device::default_floppy_formats)
	//MCFG_FLOPPY_DRIVE_ADD("fdc:1", altos5_floppies, "525dd", floppy_image_device::default_floppy_formats)
MACHINE_CONFIG_END

ROM_START( sbrain )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )

	ROM_REGION( 0x10000, "subcpu", ROMREGION_ERASEFF )
	ROM_LOAD( "superbrain.bin", 0x0000, 0x0800, CRC(b6a2e6a5) SHA1(a646faaecb9ac45ee1a42764628e8971524d5c13) )

	// Using the chargen from 'c10' for now.
	ROM_REGION( 0x2000, "chargen", 0 )
	ROM_LOAD( "c10_char.bin", 0x0000, 0x2000, BAD_DUMP CRC(cb530b6f) SHA1(95590bbb433db9c4317f535723b29516b9b9fcbf))
ROM_END

COMP( 1981, sbrain, 0, 0, sbrain, sbrain, sbrain_state, sbrain, "Intertec", "Superbrain", GAME_IS_SKELETON )

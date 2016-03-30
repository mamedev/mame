// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Powertran Cortex

        2012-04-20 Skeleton driver.

        ftp://ftp.whtech.com/Powertran Cortex/
        http://www.powertrancortex.com/index.html

        Uses Texas Instruments parts and similar to other TI computers.
        It was designed by TI engineers, so it may perhaps be a clone
        of another TI or the Geneve.

        Chips:
        TMS9995   - CPU
        TMS9929   - Video
        TMS9911   - DMA to floppy
        TMS9909   - Floppy Disk Controller
        AY-5-2376 - Keyboard controller

****************************************************************************/


#include "emu.h"
#include "cpu/tms9900/tms9995.h"
#include "video/tms9928a.h"

class cortex_state : public driver_device
{
public:
	cortex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "ram")
		{ }

	virtual void machine_reset() override;
	required_device<tms9995_device> m_maincpu;
	required_shared_ptr<UINT8> m_p_ram;
};

static ADDRESS_MAP_START( cortex_mem, AS_PROGRAM, 8, cortex_state )
	AM_RANGE(0x0000, 0xefff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xf100, 0xf11f) AM_RAM // memory mapping unit
	AM_RANGE(0xf120, 0xf120) AM_DEVREADWRITE("tms9928a", tms9928a_device, vram_read, vram_write)
	AM_RANGE(0xf121, 0xf121) AM_DEVREADWRITE("tms9928a", tms9928a_device, register_read, register_write)
	//AM_RANGE(0xf140, 0xf147) // fdc tms9909
ADDRESS_MAP_END

static ADDRESS_MAP_START( cortex_io, AS_IO, 8, cortex_state )
	ADDRESS_MAP_UNMAP_HIGH
	//AM_RANGE(0x0000, 0x000f) AM_READWRITE(pio_r,pio_w)
	//AM_RANGE(0x0010, 0x001f) AM_READ(keyboard_r)
	//AM_RANGE(0x0080, 0x00bf) AM_READWRITE(rs232_r,rs232_w)
	//AM_RANGE(0x0180, 0x01bf) AM_READWRITE(cass_r,cass_w)
	//AM_RANGE(0x0800, 0x080f) AM_WRITE(cent_data_w)
	//AM_RANGE(0x0810, 0x0811) AM_WRITE(cent_strobe_w)
	//AM_RANGE(0x0812, 0x0813) AM_READ(cent_stat_r)
	//AM_RANGE(0x1ee0, 0x1eef) AM_READWRITE(cpu_int_r,cpu_int_w)
	//AM_RANGE(0x1fda, 0x1fdb) AM_READWRITE(cpu_int1_r,cpu_int1_w)
	AM_RANGE(0x10000, 0x10000) AM_NOP
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( cortex )
INPUT_PORTS_END


void cortex_state::machine_reset()
{
	UINT8* ROM = memregion("maincpu")->base();
	memcpy(m_p_ram, ROM, 0x6000);
	m_maincpu->ready_line(ASSERT_LINE);
}

static MACHINE_CONFIG_START( cortex, cortex_state )
	/* basic machine hardware */
	/* TMS9995 CPU @ 12.0 MHz */
	// Standard variant, no overflow int
	// No lines connected yet
	MCFG_TMS99xx_ADD("maincpu", TMS9995, 12000000, cortex_mem, cortex_io)

	/* video hardware */
	MCFG_DEVICE_ADD( "tms9928a", TMS9929A, XTAL_10_738635MHz / 2 )
	MCFG_TMS9928A_VRAM_SIZE(0x4000)
	MCFG_TMS9928A_SCREEN_ADD_PAL( "screen" )
	MCFG_SCREEN_UPDATE_DEVICE( "tms9928a", tms9928a_device, screen_update )
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( cortex )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS(0, "basic", "Cortex Bios")
	ROMX_LOAD( "cortex_ic47.bin", 0x0000, 0x2000, CRC(bdb8c7bd) SHA1(340829dcb7a65f2e830fd5aff82a312e3ed7918f), ROM_BIOS(1))
	ROMX_LOAD( "cortex_ic46.bin", 0x2000, 0x2000, CRC(4de459ea) SHA1(00a42fe556d4ffe1f85b2ce369f544b07fbd06d9), ROM_BIOS(1))
	ROMX_LOAD( "cortex_ic45.bin", 0x4000, 0x2000, CRC(b0c9b6e8) SHA1(4e20c3f0b7546b803da4805cd3b8616f96c3d923), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "forth", "FIG-Forth")
	ROMX_LOAD( "forth_ic47.bin",  0x0000, 0x2000, CRC(999034be) SHA1(0dcc7404c38aa0ae913101eb0aa98da82104b5d4), ROM_BIOS(2))
	ROMX_LOAD( "forth_ic46.bin",  0x2000, 0x2000, CRC(8eca54cc) SHA1(0f1680e941ef60bb9bde9a4b843b78f30dff3202), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY                  FULLNAME       FLAGS */
COMP( 1982, cortex, 0,      0,       cortex,    cortex, driver_device,  0,    "Powertran Cybernetics",   "Cortex", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

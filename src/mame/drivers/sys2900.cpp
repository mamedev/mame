// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************

Systems Group (a division of Measurement Systems and Controls) System 2900 S100 computer

2009-05-12 Skeleton driver.

Photos: https://www.vintagecomputer.net/Systems-Group/2900/

The system contains 4 S100 boards, 2 8" floppies, enormous power supply,
1/4" thick mother board, 4 RS232 I/F boards, and an Australian made
"Computer Patch Board".

The S100 boards that make up the unit are:
            CPU board - Model CPC2810 Rev D
                    The board has a MK3880N-4 (Z80A CPU), CTC, 2x MK3884N-4 (Z80SIO), PIO.
                    It has a lot of jumpers and an 8 way DIP switch.
                    Crystals: 8.0000MHz, 4.91520MHz
            Memory board - Model HDM2800 Rev B
                    This contains 18 4164's and a lot of logic.
                    Again, a lot of jumpers, and several banks of links.
                    Also contains 2 large Motorola chips:
                    MC3480P (dynamic memory controller) and
                    MC3242AP (memory address multiplexer).
                    Dips: 2x 8-sw, 2x 4-sw, 1x 2-sw.
            Disk Controller board - Model FDC2800 Rev D
                    This board is damaged. The small voltage regulators
                    for +/- 12V have been fried.
                    Also, 3 sockets are empty - U1 and U38 (16 pin)
                    appear to be spares, and U10 (14 pin).
            8 port Serial I/O board - Model INO2808 Rev C
                    Room for 8 8251 USARTs.
                    I have traced out this board and managed to get it
                    working in another S100 system.

The "Computer Patch Board" seems to provide some sort of watch dog facility.

Status:
- Appears to be looping waiting for a disk

*****************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
//#include "bus/s100/s100.h"
#include "screen.h"


class sys2900_state : public driver_device
{
public:
	enum
	{
		TIMER_BOOT
	};

	sys2900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu") { }

	DECLARE_DRIVER_INIT(sys2900);
	uint32_t screen_update_sys2900(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sys2900(machine_config &config);
	void io_map(address_map &map);
	void mem_map(address_map &map);
private:
	virtual void machine_reset() override;
	virtual void video_start() override;
	required_device<cpu_device> m_maincpu;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};


ADDRESS_MAP_START(sys2900_state::mem_map)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x07ff ) AM_RAMBANK("boot")
	AM_RANGE( 0x0800, 0xefff ) AM_RAM
	AM_RANGE( 0xf000, 0xf7ff ) AM_ROM
	AM_RANGE( 0xf800, 0xffff ) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(sys2900_state::io_map)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("sio1", z80sio_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x24, 0x27) AM_DEVREADWRITE("pio",  z80pio_device, read_alt, write_alt)
	AM_RANGE(0x28, 0x2b) AM_DEVREADWRITE("sio2", z80sio_device, ba_cd_r, ba_cd_w)
	AM_RANGE(0x2c, 0x2f) AM_DEVREADWRITE("ctc",  z80ctc_device, read, write)
	AM_RANGE(0x80, 0x83) // unknown device, disk related?
	AM_RANGE(0xa0, 0xaf) // unknown device
	AM_RANGE(0xc0, 0xc3) // unknown device
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( sys2900 )
INPUT_PORTS_END


void sys2900_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_BOOT:
		/* after the first 4 bytes have been read from ROM, switch the ram back in */
		membank("boot")->set_entry(0);
		break;
	default:
		assert_always(false, "Unknown id in sys2900_state::device_timer");
	}
}

void sys2900_state::machine_reset()
{
	membank("boot")->set_entry(1);
	timer_set(attotime::from_usec(5), TIMER_BOOT);
}

DRIVER_INIT_MEMBER(sys2900_state,sys2900)
{
	uint8_t *RAM = memregion("maincpu")->base();
	membank("boot")->configure_entries(0, 2, &RAM[0x0000], 0xf000);
}

void sys2900_state::video_start()
{
}

uint32_t sys2900_state::screen_update_sys2900(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

MACHINE_CONFIG_START(sys2900_state::sys2900)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL(8'000'000) / 2)
	MCFG_CPU_PROGRAM_MAP(mem_map)
	MCFG_CPU_IO_MAP(io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 480-1)
	MCFG_SCREEN_UPDATE_DRIVER(sys2900_state, screen_update_sys2900)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_DEVICE_ADD("ctc", Z80CTC, 0)
	MCFG_DEVICE_ADD("pio", Z80PIO, 0)
	MCFG_DEVICE_ADD("sio1", Z80SIO, 0)
	MCFG_DEVICE_ADD("sio2", Z80SIO, 0)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( sys2900 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "104401cpc.bin", 0xf000, 0x0800, CRC(6c8848bc) SHA1(890e0578e5cb0e3433b4b173e5ed71d72a92af26)) // label says BE 5 1/4 107701
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    STATE          INIT     COMPANY          FULLNAME       FLAGS
COMP( 1981, sys2900, 0,      0,       sys2900,   sys2900, sys2900_state, sys2900, "Systems Group", "System 2900", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

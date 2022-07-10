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

//#include "bus/s100/s100.h"
#include "cpu/z80/z80.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"

#include "emupal.h"
#include "screen.h"


namespace {

class sys2900_state : public driver_device
{
public:
	sys2900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rom(*this, "maincpu")
		, m_ram(*this, "mainram")
	{ }

	void sys2900(machine_config &config);

private:
	uint32_t screen_update_sys2900(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void io_map(address_map &map);
	void mem_map(address_map &map);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	memory_passthrough_handler m_rom_shadow_tap;
	required_device<z80_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;
};


void sys2900_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram().share("mainram");
	map(0xf000, 0xf7ff).rom().region("maincpu", 0);
}

void sys2900_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x20, 0x23).rw("sio1", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x24, 0x27).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x28, 0x2b).rw("sio2", FUNC(z80sio_device::ba_cd_r), FUNC(z80sio_device::ba_cd_w));
	map(0x2c, 0x2f).rw("ctc", FUNC(z80ctc_device::read),     FUNC(z80ctc_device::write));
	map(0x80, 0x83); // unknown device, disk related?
	map(0xa0, 0xaf); // unknown device
	map(0xc0, 0xc3); // unknown device
}

/* Input ports */
static INPUT_PORTS_START( sys2900 )
INPUT_PORTS_END

void sys2900_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x07ff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf000, 0xf7ff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x07ff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

void sys2900_state::machine_start()
{
}

uint32_t sys2900_state::screen_update_sys2900(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void sys2900_state::sys2900(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(8'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sys2900_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sys2900_state::io_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(sys2900_state::screen_update_sys2900));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	Z80CTC(config, "ctc", 0);
	Z80PIO(config, "pio", 0);
	Z80SIO(config, "sio1", 0);
	Z80SIO(config, "sio2", 0);
}

/* ROM definition */
ROM_START( sys2900 )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD( "104401cpc.bin", 0x0000, 0x0800, CRC(6c8848bc) SHA1(890e0578e5cb0e3433b4b173e5ed71d72a92af26)) // label says BE 5 1/4 107701
ROM_END

} // anonymous namespace

/* Driver */

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY          FULLNAME       FLAGS
COMP( 1981, sys2900, 0,      0,      sys2900, sys2900, sys2900_state, empty_init, "Systems Group", "System 2900", MACHINE_IS_SKELETON | MACHINE_SUPPORTS_SAVE )

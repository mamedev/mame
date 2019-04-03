// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************

    Data RD100

    2015-10-02 Skeleton [Robbbert]

    Little is known about this system except for a few PCB pictures. No
    manuals, schematic or circuit description have been found.

    The RD100 was apparently sold in France under the "Superkit" brand. There
    appear to have been several versions. Earlier models had 7-segment LEDs
    and rudimentary keyboards. The model dumped here is apparently the K32K,
    which had a 16x2 character LCD display, a QWERTY keyboard and non-numeric
    keypad, Centronics and RS-232 ports, and an extension board for prototyping.

*********************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "emupal.h"
#include "screen.h"



class rd100_state : public driver_device
{
public:
	rd100_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void rd100(machine_config &config);

	void init_rd100();

private:
	DECLARE_MACHINE_RESET(rd100);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};


void rd100_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();
	//AM_RANGE(0x8608, 0x860f) AM_DEVREADWRITE("timer", ptm6840_device, read, write)
	map(0x8640, 0x8643).rw("pia1", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x8680, 0x8683).rw("pia2", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	//AM_RANGE(0x8700, 0x8700)  // device
	map(0x8800, 0xffff).rom().region("roms", 0x800);
}

/* Input ports */
static INPUT_PORTS_START( rd100 )
INPUT_PORTS_END

uint32_t rd100_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
//  for (int y = 0; y < 32*8; y++)
//  {
//      offs_t offset = (y / 8) * 128;

//      for (int sx = 0; sx < 64; sx++)
//      {
//          uint8_t code = m_video_ram[offset++];
//          uint8_t attr = m_video_ram[offset++];

//          offs_t char_offs = ((code & 0x7f) << 3) | (y & 0x07);
//          if (BIT(code, 7)) char_offs = ((code & 0x7f) << 3) | ((y >> 1) & 0x07);

//          uint8_t data = m_char_rom->base()[char_offs];

//          rgb_t fg = m_palette->pen_color(attr & 0x07);
//          rgb_t bg = m_palette->pen_color((attr >> 3) & 0x07);

//          for (int x = 0; x < 6; x++)
//          {
//              bitmap.pix32(y, (sx * 6) + x) = BIT(data, 7) ? fg : bg;

//              data <<= 1;
//          }
//      }
//  }

	return 0;
}

void rd100_state::init_rd100()
{
}

MACHINE_RESET_MEMBER( rd100_state, rd100 )
{
}

void rd100_state::rd100(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, XTAL(4'000'000)); // MC6809P???
	m_maincpu->set_addrmap(AS_PROGRAM, &rd100_state::mem_map);

	MCFG_MACHINE_RESET_OVERRIDE(rd100_state, rd100)

	PIA6821(config, "pia1", 0);

	PIA6821(config, "pia2", 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER, rgb_t::green()));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(250));
	screen.set_screen_update(FUNC(rd100_state::screen_update));
	screen.set_size(64*6, 32*8);
	screen.set_visarea(0, 64*6-1, 0, 32*8-1);
	PALETTE(config, "palette", palette_device::MONOCHROME);
	//GFXDECODE(config, "gfxdecode", "palette", gfx_rd100);
}

ROM_START( rd100 )
	ROM_REGION( 0x8000, "roms", 0 )
	ROM_LOAD( "pak3-01.bin",  0x0000, 0x8000, CRC(cf5bbf01) SHA1(0673f4048d700b84c30781af23fbeabe0b994306) )
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP( 1989, rd100, 0,      0,      rd100,   rd100, rd100_state, init_rd100, "Data R.D.", "RD100",  MACHINE_IS_SKELETON )

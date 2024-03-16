// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Driver for Casio Picky Talk

    Missing inputs, and I/O callbacks copied from CFX9850G need to be reviewed.

    Some points of interest can be accessed under the debugger:

    1. bpset 0x200328
    2. ip=3fe (clock screen)
    3. ip=410 (main screen)

    Hardware
    --------

    Super Picky Talk - Forest of Gurutan (JD-370):

    - PCB revision: A140947-1 Z835-1
    - LSI1 (CPU): Unknown (instruction set compatible with Hitachi HCD62121)
    - LSI3 (Static RAM): NEC D441000LGZ (1M-bit, 128K-word by 8-bit)
    - LSI5 (Mask ROM): NEC D23C8000XGX-C64 (8M-bit, 1M-word by 8-bit, pin compatible with AMD AM29F800B)

***************************************************************************/

#include "emu.h"

#include "cpu/hcd62121/hcd62121.h"

#include "emupal.h"
#include "screen.h"


namespace {

class pickytlk_state : public driver_device
{
public:
	pickytlk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_video_ram(*this, "video_ram")
		, m_display_ram(*this, "display_ram")
		, m_maincpu(*this, "maincpu")
		, m_ko(0)
		, m_port(0)
		, m_opt(0)
	{ }

	void pickytlk(machine_config &config);

private:
	void kol_w(u8 data);
	void koh_w(u8 data);
	void port_w(u8 data);
	void opt_w(u8 data);
	u8 ki_r();
	u8 in0_r();
	void pickytlk_palette(palette_device &palette) const;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void pickytlk_mem(address_map &map);

	required_shared_ptr<u8> m_video_ram;
	required_shared_ptr<u8> m_display_ram;
	required_device<hcd62121_cpu_device> m_maincpu;

	u16 m_ko;   // KO lines
	u8 m_port;  // PORT lines (serial I/O)
	u8 m_opt;   // OPT lines (contrast)
};

void pickytlk_state::pickytlk_mem(address_map &map)
{
	map(0x000000, 0x007fff).mirror(0x008000).rom();
	map(0x080000, 0x0807ff).ram().share("video_ram");
//  map(0x100000, 0x10ffff) // some memory mapped i/o???
//  map(0x110000, 0x11ffff) // some memory mapped i/o???
	map(0x200000, 0x2fffff).rom().region("mask_rom", 0);
	map(0x400000, 0x4007ff).ram().share("display_ram");
	map(0x400800, 0x41ffff).ram();
//  map(0xe10000, 0xe1ffff) // some memory mapped i/o???
}

void pickytlk_state::kol_w(u8 data)
{
	m_ko = (m_ko & 0xff00) | data;
	logerror("%s: KO is now %04x\n", machine().describe_context(), m_ko);
}

void pickytlk_state::koh_w(u8 data)
{
	m_ko = (m_ko & 0x00ff) | (u16(data) << 8);
	logerror("%s: KO is now %04x\n", machine().describe_context(), m_ko);
}

void pickytlk_state::port_w(u8 data)
{
	m_port = data;
	logerror("%s: PORT is now %02x\n", machine().describe_context(), m_port);
}

void pickytlk_state::opt_w(u8 data)
{
	m_opt = data;
	logerror("%s: OPT is now %02x\n", machine().describe_context(), m_opt);
}

u8 pickytlk_state::ki_r()
{
	// TODO
	return 0;
}

u8 pickytlk_state::in0_r()
{
	// battery level?
	// bit4 -> if reset CPU keeps restarting (several unknown instructions before jumping to 0)
	//         perhaps a battery present check?
	// bit 5 -> 0 = low battery

	// --XX ---- VDET
	// ---- -X-- data-in
	return 0x30 & ~0x00;
}

void pickytlk_state::pickytlk_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xee, 0xee, 0xcc);
	palette.set_pen_color(1, 0x11, 0x33, 0x99);
	palette.set_pen_color(2, 0x33, 0xcc, 0x77);
	palette.set_pen_color(3, 0xee, 0x77, 0x33);
}

u32 pickytlk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 offset = 0;

	for (int i = 0; i < 16; i++)
	{
		int const x = i * 8;

		for (int j = 0; j < 64; j++)
		{
			u16 *const row = &bitmap.pix(63 - j);

			u8 const data1 = m_display_ram[offset];
			u8 const data2 = m_display_ram[offset + 0x400];

			for (int b = 0; b < 8; b++)
			{
				row[x + b] = (BIT(data1, b) << 1) | BIT(data2, b);
			}

			offset++;
		}
	}

	return 0;
}


static INPUT_PORTS_START(pickytlk)
	// TODO
INPUT_PORTS_END


void pickytlk_state::pickytlk(machine_config &config)
{
	HCD62121(config, m_maincpu, 4300000); /* X1 - 4.3 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &pickytlk_state::pickytlk_mem);
	m_maincpu->kol_cb().set(FUNC(pickytlk_state::kol_w));
	m_maincpu->koh_cb().set(FUNC(pickytlk_state::koh_w));
	m_maincpu->port_cb().set(FUNC(pickytlk_state::port_w));
	m_maincpu->opt_cb().set(FUNC(pickytlk_state::opt_w));
	m_maincpu->ki_cb().set(FUNC(pickytlk_state::ki_r));
	m_maincpu->in0_cb().set(FUNC(pickytlk_state::in0_r));

	// TODO: Touchpad layout
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(128, 64);
	screen.set_visarea(0, 127, 0, 63);
	screen.set_screen_update(FUNC(pickytlk_state::screen_update));
	screen.set_palette("palette");

	// TODO: Verify amount of colors and palette. Colors can be changed by changing the contrast.
	PALETTE(config, "palette", FUNC(pickytlk_state::pickytlk_palette), 4);
}


ROM_START(pickytlk)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("cpu.lsi1", 0x0000, 0x8000, CRC(d58efff9) SHA1(a8d2c2a331d79c5299274e2f2d180deda60a5aed))

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c8000xgx-c64.lsi5", 0x00000, 0x100000, CRC(6ed6feae) SHA1(f9a63db3d048da0954cab052690deb01ec384b22))
ROM_END

} // anonymous namespace


// "CASIO スーパーピッキートーク「グルタンの森」はやわかりビデオ" has copyright dates 1997,1998,1999
COMP(1997, pickytlk, 0, 0, pickytlk, pickytlk, pickytlk_state, empty_init, "Casio", "Super Picky Talk - Forest of Gurutan", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

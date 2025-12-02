// license:BSD-3-Clause
// copyright-holders:QUFB
/***************************************************************************

    Hitachi HD621xx organizers with keypad input.
    Includes Super Denshi Techou Jr models.

    TODO:

    - Communication/infrared port;
    - Review PORT/OPT callbacks copied from CFX9850G;
    - Keyboard input for CSF-xxxx/JD-xxx models;
    - Panel active buttons display;
    - Layouts with scanned overlays;

***************************************************************************/

#include "emu.h"

#include "cpu/hcd62121/hcd62121.h"

#include "emupal.h"
#include "screen.h"

#define LOG_IO     (1U << 1)

//#define VERBOSE (LOG_IO)
#include "logmacro.h"


namespace {

class superjr_state : public driver_device
{
public:
	superjr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_display_ram(*this, "display_ram")
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_ko(0)
		, m_port(0)
		, m_opt(0)
	{ }

	void superjr(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void kol_w(u8 data);
	void koh_w(u8 data);
	void port_w(u8 data);
	void opt_w(u8 data);
	u8 ki_r();
	u8 port_r();
	u8 in0_r();
	u8 input_flag_read();

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void superjr_palette(palette_device &palette) const ATTR_COLD;
	void superjr_mem(address_map &map) ATTR_COLD;

	required_shared_ptr<u8> m_display_ram;
	required_device<hcd62121_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	u16 m_ko;   // KO lines
	u8 m_port;  // PORT lines (serial I/O)
	u8 m_opt;   // OPT lines (contrast)
};

void superjr_state::machine_start()
{
	save_item(NAME(m_ko));
	save_item(NAME(m_port));
	save_item(NAME(m_opt));
}

void superjr_state::superjr_palette(palette_device &palette) const
{
	palette.set_pen_color(0, 0xee, 0xee, 0xcc);
	palette.set_pen_color(1, 0x11, 0x11, 0x11);
}

void superjr_state::superjr_mem(address_map &map)
{

	map(0x000000, 0x017fff).rom();
//  map(0x040000, 0x04ffff) // Unknown
	map(0x080000, 0x0807ff).ram();
	map(0x100000, 0x10017f).ram().share("display_ram");
	map(0x800000, 0x83ffff).rom().region("mask_rom", 0); // Banked?
	map(0x840000, 0x847fff).ram();
}

void superjr_state::kol_w(u8 data)
{
	m_ko = (m_ko & 0xff00) | data;
	LOGMASKED(LOG_IO, "%s: KO = %04x\n", machine().describe_context(), m_ko);
}

void superjr_state::koh_w(u8 data)
{
	m_ko = (m_ko & 0x00ff) | (u16(data) << 8);
	LOGMASKED(LOG_IO, "%s: KO = %04x\n", machine().describe_context(), m_ko);
}

void superjr_state::port_w(u8 data)
{
	m_port = data;
	LOGMASKED(LOG_IO, "%s: PORT = %02x\n", machine().describe_context(), m_port);
}

void superjr_state::opt_w(u8 data)
{
	m_opt = data;
	LOGMASKED(LOG_IO, "%s: OPT = %02x\n", machine().describe_context(), m_opt);
}

u8 superjr_state::ki_r()
{
	return 0; // TODO
}

u8 superjr_state::port_r()
{
	return m_port;
}

u8 superjr_state::in0_r()
{
	// battery level?
	// bit4 -> if reset CPU keeps restarting (several unknown instructions before jumping to 0)
	//         perhaps a battery present check?
	// bit 5 -> 0 = low battery

	// --XX ---- VDET
	// ---- -X-- data-in
	return 0x30 & ~0x00;
}

u8 superjr_state::input_flag_read()
{
	return 0;
}

u32 superjr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 offset = 0;

	for (int i = 0; i < (m_screen->width() + 1) / 8; i++)
	{
		int const x = i * 8;

		for (int j = 0; j < m_screen->height(); j++)
		{
			u16 *const row = &bitmap.pix(m_screen->height() - 1 - j);

			u8 const data1 = m_display_ram[offset];

			for (int b = 0; b < 8; b++)
			{
				if (x + b < m_screen->width())
				{
					row[m_screen->width() - (x + b + 1)] = BIT(data1, b);
				}
			}

			offset++;
		}
	}

	return 0;
}

static INPUT_PORTS_START(superjr)
	// TODO
INPUT_PORTS_END

void superjr_state::superjr(machine_config &config)
{
	HCD62121(config, m_maincpu, 4300000); /* X1 - 4.3 MHz */
	m_maincpu->kol_cb().set(FUNC(superjr_state::kol_w));
	m_maincpu->koh_cb().set(FUNC(superjr_state::koh_w));
	m_maincpu->port_w_cb().set(FUNC(superjr_state::port_w));
	m_maincpu->opt_cb().set(FUNC(superjr_state::opt_w));
	m_maincpu->ki_cb().set(FUNC(superjr_state::ki_r));
	m_maincpu->port_r_cb().set(FUNC(superjr_state::port_r));
	m_maincpu->in0_cb().set(FUNC(superjr_state::in0_r));
	m_maincpu->input_flag_cb().set(FUNC(superjr_state::input_flag_read));
	m_maincpu->set_addrmap(AS_PROGRAM, &superjr_state::superjr_mem);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(95, 32);
	m_screen->set_visarea(0, 95 - 1, 0, 32 - 1);
	m_screen->set_screen_update(FUNC(superjr_state::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(superjr_state::superjr_palette), 2);
}


#define CPU_ROM_HC3000 \
	ROM_REGION(0x18000, "maincpu", 0) \
	ROM_LOAD("hc3000-03-f1.lsi", 0x0000, 0x18000, CRC(3f169797) SHA1(3cc82c3a128c477d3c67f63385d57d8925929930))

#define CPU_ROM_L180 \
	ROM_REGION(0x18000, "maincpu", 0) \
	ROM_LOAD("cpu.lsi", 0x0000, 0x18000, CRC(5291b0df) SHA1(33aeb2ef49fad72a743bd5d952138d640e51e199))

#define CPU_ROM_L196 \
	ROM_REGION(0x18000, "maincpu", 0) \
	ROM_LOAD("cpu.lsi1", 0x0000, 0x18000, CRC(d3d9b175) SHA1(f8bde2b5b47f591d6a1e57c4b90159dac6623548))


ROM_START(jd5000)
	CPU_ROM_HC3000

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("tc531001cf.rom", 0x00000, 0x20000, CRC(43b186a1) SHA1(f5a5ece8179d5e215433553cab781448692fbb60))
ROM_END

ROM_START(jd320)
	CPU_ROM_L180

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("tc531001cf.ic2", 0x00000, 0x080000, CRC(f7f1c35e) SHA1(25ffda8568699082f22a4f82859e99b0396e8e8b))
ROM_END

ROM_START(jd360)
	CPU_ROM_L180

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c4001ebgw-j06.ic2", 0x00000, 0x080000, CRC(9b9a04c3) SHA1(2a72d5d1c3f045f74af8316cde4ac93aee1124a5))
ROM_END

ROM_START(jd361)
	CPU_ROM_L196

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("mb838200bl.ic2", 0x00000, 0x100000, CRC(cdc18716) SHA1(1eb7a1f9ee1d84d39a8cde6cfe3a48e8822ade41))
ROM_END

ROM_START(jd362)
	CPU_ROM_L196

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c4001ejgw-c10.ic2", 0x00000, 0x080000, CRC(67c59d27) SHA1(93706b2bbe3fdd71505bb1b2c0ac0327ad404d7c))
ROM_END

ROM_START(jd367)
	CPU_ROM_L196

	ROM_REGION(0x100000, "mask_rom", 0)
	ROM_LOAD("d23c4001ejgw-c42.ic2", 0x00000, 0x080000, CRC(330a8f4f) SHA1(00a822c7e1ce001be45ff2d6a9b15e2bd20e05f0))
ROM_END

} // anonymous namespace


// Date 1993-07 from "Service Manual & Parts List"
COMP(1993, jd5000, 0, 0, superjr, superjr, superjr_state, empty_init, "Casio", "My Magic Diary (JD-5000)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// SUPER 電子手帳 Jr. 似顔絵テレパシー
// ROM date 9335EAI
COMP(1993?, jd320, 0, 0, superjr, superjr, superjr_state, empty_init, "Casio", "Super Denshi Techou Jr. - Caricature Telepathy", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// SUPER 電子手帳 Jr. パピーテレパシー
// Release date 1994-09 from "Casio Game Perfect Catalogue"
COMP(1994, jd360, 0, 0, superjr, superjr, superjr_state, empty_init, "Casio", "Super Denshi Techou Jr. - Puppy Telepathy", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// SUPER 電子手帳 Jr. ペットテレパシー
// Release date 1994-09 from "Casio Game Perfect Catalogue"
// Models with identical dumps: JD-361BU, JD-361PL
COMP(1994, jd361, 0, 0, superjr, superjr, superjr_state, empty_init, "Casio", "Super Denshi Techou Jr. - Pet Telepathy", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// スーパー電子手帳 ペットワールド
// ROM date 9529K7043
COMP(1995?, jd362, 0, 0, superjr, superjr, superjr_state, empty_init, "Casio", "Super Denshi Techou - Pet World", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// ROM date 9825K7018
COMP(1998?, jd367, 0, 0, superjr, superjr, superjr_state, empty_init, "Casio", "Pet Avenue - My room fantasy", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)

// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Кроха (Krokha, "Tiny"), TV game by Контур (SKB Kontur).
    It was apparently only for SKB Kontur workers, not sold in stores.

    К580ВМ80А @ 2MHz, 2KB RAM. Screen is 48x32 monochrome text.
    Joystick, built-in 1-bit speaker.

    Only known cartridge has 5 built-in games:
    - Breakout
    - Tetris
    - Snake
    - Xonix
    - Air Defence

    https://zx-pk.ru/threads/26306-igrovaya-pristavka-quot-krokha-quot.html
        discussion

    http://www.nedopc.org/forum/viewtopic.php?f=90&t=11458
        discussion + schematics

    http://alemorf.ru/comps/kroha/index.html
        photos

    TODO:
    - second joystick?
    - video timing

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class krokha_state : public driver_device
{
public:
	krokha_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_p_videoram(*this, "videoram")
		, m_p_chargen(*this, "chargen")
		, m_speaker(*this, "speaker")
	{ }

	static constexpr feature_type imperfect_features() { return feature::CONTROLS; }

	void krokha(machine_config &config);

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void krokha_mem(address_map &map) ATTR_COLD;

	void status_callback(uint8_t data);
	void speaker_w(uint8_t data);

	required_device<i8080a_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<speaker_sound_device> m_speaker;
};

//

void krokha_state::status_callback(uint8_t data)
{
	if (data & i8080a_cpu_device::STATUS_INTA)
	{
		// interrupt acknowledge
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

void krokha_state::speaker_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 1));
}

//

void krokha_state::krokha_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0xe000, 0xe7ff).ram().mirror(0x0800).share("videoram");
	map(0xf7ff, 0xf7ff).portr("P1").w(FUNC(krokha_state::speaker_w));
}

static INPUT_PORTS_START( krokha )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

uint32_t krokha_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			for (int ra = 0; ra < 8; ra++)
			{
				uint8_t gfx = m_p_chargen[m_p_videoram[x << 5 | y] << 3 | ra];
				for (int i = 0; i < 8; i++)
				{
					int dx = x << 3 | i;
					int dy = y << 3 | ra;
					if (cliprect.contains(dx, dy))
						bitmap.pix(dy, dx) = BIT(gfx, i ^ 7);
				}
			}
		}
	}

	return 0;
}


void krokha_state::krokha(machine_config &config)
{
	I8080A(config, m_maincpu, 8_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &krokha_state::krokha_mem);
	m_maincpu->out_status_func().set(FUNC(krokha_state::status_callback));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(64 * 8, 32 * 8);
	m_screen->set_visarea(16 * 8, 64 * 8 - 1, 0 * 8, 32 * 8 - 1);
	m_screen->set_screen_update(FUNC(krokha_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}

ROM_START( krokha )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("bios.bin", 0x0000, 0x2000, CRC(e37556f4) SHA1(b1da9d7338eb227b0aff5675719f7a2aab607e66))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("font.bin", 0x0000, 0x0800, CRC(2f4fcfb5) SHA1(175cafe3dc9291f505d69aced9c405c38b7f7086))
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMP  MACHINE  INPUT   CLASS         INIT        COMPANY        FULLNAME  FLAGS
CONS( 1990, krokha,  0,      0,    krokha,  krokha, krokha_state, empty_init, "SKB Kontur",  "Krokha", MACHINE_SUPPORTS_SAVE )

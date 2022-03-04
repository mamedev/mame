// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Krokha ("Tiny") TV game.  Screen is 48x32 monochrome text.

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

    To do: second joystick, keyboard (?)

****************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


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
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void krokha_mem(address_map &map);

	void status_callback(uint8_t data);
	void speaker_w(uint8_t data);

	required_device<i8080_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_p_videoram;
	required_region_ptr<u8> m_p_chargen;
	required_device<speaker_sound_device> m_speaker;

	int m_speaker_state;
};

//

void krokha_state::status_callback(uint8_t data)
{
	if (data & i8080_cpu_device::STATUS_INTA)
	{
		/* interrupt acknowledge */
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

void krokha_state::speaker_w(uint8_t data)
{
	m_speaker_state = BIT(data, 1);
	m_speaker->level_w(m_speaker_state);
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void krokha_state::machine_reset()
{
	m_speaker_state = 0;
	m_speaker->level_w(m_speaker_state);

	// setup save states
	save_item(NAME(m_speaker_state));
}

uint32_t krokha_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (uint8_t y = 0; y < 32; y++)
	{
		uint16_t ma = 0xe0 + y;
		for (uint8_t ra = 0; ra < 8; ra++)
		{
			for (uint16_t x = ma; x < ma + 64 * 32; x += 32)
			{
				uint16_t chr = m_p_videoram[x] << 3;
				uint8_t gfx = m_p_chargen[chr | ra];

				for (int i = 0; i < 8; i++)
				{
					bitmap.pix(y * 8 + ra, (x - ma) / 4 + i) = BIT(gfx, 7 - i);
				}
			}
		}
	}

	return 0;
}


void krokha_state::krokha(machine_config &config)
{
	I8080(config, m_maincpu, 2000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &krokha_state::krokha_mem);
	m_maincpu->out_status_func().set(FUNC(krokha_state::status_callback));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(64 * 8, 32 * 8);
	m_screen->set_visarea(9 * 8, (48 + 9) * 8 - 1, 0 * 8, 32 * 8 - 1);
	m_screen->set_screen_update(FUNC(krokha_state::screen_update));
	m_screen->set_palette("palette");
	m_screen->screen_vblank().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	PALETTE(config, "palette", palette_device::MONOCHROME);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 1.00);
}

ROM_START( krokha )
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD("bios.bin", 0x0000, 0x2000, CRC(e37556f4) SHA1(b1da9d7338eb227b0aff5675719f7a2aab607e66))

	ROM_REGION(0x0800, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("font.bin", 0x0000, 0x0800, CRC(2f4fcfb5) SHA1(175cafe3dc9291f505d69aced9c405c38b7f7086))
ROM_END

/* Driver */

//    YEAR  NAME     PARENT  MACHINE  INPUT   CLASS         INIT        ROT,   COMPANY        FULLNAME
GAME( 1990, krokha,  0,      krokha,  krokha, krokha_state, empty_init, ROT0,  "SKB Kontur",  "Krokha",  0 )

// license:BSD-3-Clause
// copyright-holders:

// TODO: code is scrambled/encrypted, put this where it belongs after descrambling
// (probably a GeneralPlus)

#include "emu.h"

#include "screen.h"
#include "speaker.h"


namespace {

class yuleyuan_state : public driver_device
{
public:
	yuleyuan_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_screen(*this, "screen")
	{
	}

	void yuleyuan(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<screen_device> m_screen;
};

u32 yuleyuan_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void yuleyuan_state::machine_start()
{
}


static INPUT_PORTS_START(yuleyuan)
INPUT_PORTS_END


void yuleyuan_state::yuleyuan(machine_config &config)
{
	// unknown CPU (encrypted)

	// wrong, just so it's clear this has a screen
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(160, 128); 
	m_screen->set_visarea(0, 160 - 1, 0, 128 - 1);
	m_screen->set_screen_update(FUNC(yuleyuan_state::screen_update));

	SPEAKER(config, "speaker").front_center();
}


ROM_START( yuleyuan )
	ROM_REGION( 0x1000000, "spi", 0 )
	ROM_LOAD( "25l128.bin", 0x0000000, 0x1000000, CRC(51ab49e2) SHA1(ecad532d27efea55031ffd31ac4479c9c4eceae6) )
ROM_END

} // anonymous namespace

// 星座电子宠物机
GAME( 2022, yuleyuan, 0, yuleyuan, yuleyuan, yuleyuan_state, empty_init, ROT0, "Yule Yuan", "Xingzuo Dianzi Chongwu Ji", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // dumped from yellow model

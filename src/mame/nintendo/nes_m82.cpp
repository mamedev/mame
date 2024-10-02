// license:BSD-3-Clause
// copyright-holders:kmg

/***************************************************************************

  Nintendo NES M82 Game Selectable Working Product Display

  Emulation is incomplete. It loads the builtin splash screen ROM but lacks
  12 cartridge slots, timer, game select button, etc. Listen to the nice
  little tune in the meantime. It's not the same as M8 and FamicomBox.

  TODO:
   - Carts
   - Timer
   - Game Select
   - PAL version (same internal ROM?)

***************************************************************************/

#include "emu.h"

#include "bus/nes_ctrl/ctrl.h"
#include "cpu/m6502/rp2a03.h"
#include "video/ppu2c0x.h"

#include "screen.h"
#include "speaker.h"


namespace {

class m82_state : public driver_device
{
public:
	m82_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppu(*this, "ppu")
		, m_screen(*this, "screen")
		, m_ctrl(*this, "ctrl%u", 1U)
		, m_cn12(*this, "CN12")
		, m_nt_page(*this, "nt_page%u", 0U)
	{
	}

	void nes_m82(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(m82_game_select);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<rp2a03_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;
	required_device<screen_device> m_screen;
	optional_device_array<nes_control_port_device, 5> m_ctrl;

	required_ioport m_cn12;

	required_memory_bank_array<4> m_nt_page;
	std::unique_ptr<u8 []> m_nt_ram;

	u8 m_curr_slot = 0;
	u16 m_time_limit = 0;
	emu_timer* m_play_timer = nullptr;

	TIMER_CALLBACK_MEMBER(m82_play_timer_cb);
	u8 m82_in0_r();
	u8 m82_in1_r();
	void m82_in0_w(u8 data);
	void m82_map(address_map &map) ATTR_COLD;
	void m82_ppu_map(address_map &map) ATTR_COLD;
};


INPUT_CHANGED_MEMBER(m82_state::m82_game_select)
{
	if (newval)
	{
	}
}

// TODO: Connectors 1/2 are P1 and 3/4/5 are P2? Space between them on board implies this.
u8 m82_state::m82_in0_r()
{
	u8 ret = 0x40;

	ret |= m_ctrl[0]->read_bit0();
	ret |= m_ctrl[0]->read_bit34();
	ret |= m_ctrl[1]->read_bit0();
	ret |= m_ctrl[1]->read_bit34();

	return ret;
}

u8 m82_state::m82_in1_r()
{
	u8 ret = 0x40;

	ret |= m_ctrl[2]->read_bit0();
	ret |= m_ctrl[2]->read_bit34();
	ret |= m_ctrl[3]->read_bit0();
	ret |= m_ctrl[3]->read_bit34();
	ret |= m_ctrl[4]->read_bit0();
	ret |= m_ctrl[4]->read_bit34();

	return ret;
}

void m82_state::m82_in0_w(u8 data)
{
	for (auto &ctrl : m_ctrl)
		ctrl->write(data);
}


/**************************************************************************/

TIMER_CALLBACK_MEMBER(m82_state::m82_play_timer_cb)
{
}

/***************************************************************************

   Memory map

***************************************************************************/

void m82_state::m82_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(m_ppu, FUNC(ppu2c0x_device::spriteram_dma));
	map(0x4016, 0x4016).rw(FUNC(m82_state::m82_in0_r), FUNC(m82_state::m82_in0_w)); // IN0 - input port 1
	map(0x4017, 0x4017).r(FUNC(m82_state::m82_in1_r));     // IN1 - input port 2 / PSG second control register

	map(0x8000, 0xffff).rom(); // TODO: cart PRG muxed here
}

void m82_state::m82_ppu_map(address_map &map)
{
	map(0x0000, 0x1fff).rom(); // TODO: cart CHR muxed here
	map(0x2000, 0x23ff).mirror(0x1000).bankrw(m_nt_page[0]);
	map(0x2400, 0x27ff).mirror(0x1000).bankrw(m_nt_page[1]);
	map(0x2800, 0x2bff).mirror(0x1000).bankrw(m_nt_page[2]);
	map(0x2c00, 0x2fff).mirror(0x1000).bankrw(m_nt_page[3]);
	map(0x3f00, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
}



static INPUT_PORTS_START( nes_m82 )
	PORT_START("CN13")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game Select") PORT_CODE( KEYCODE_0 ) PORT_CHANGED_MEMBER(DEVICE_SELF, m82_state, m82_game_select, 0)

	PORT_START("CN12")
	PORT_CONFNAME( 0x0f, 0x08, "Play Time Limit" )
	PORT_CONFSETTING(    0x01, "30 sec." )
	PORT_CONFSETTING(    0x02, "3 min." )
	PORT_CONFSETTING(    0x04, "6 min." )
	PORT_CONFSETTING(    0x08, "128 min." )
INPUT_PORTS_END



void m82_state::machine_start()
{
	m_nt_ram = std::make_unique<u8[]>(0x800);

	for (auto &page : m_nt_page)
		page->configure_entries(0, 2, m_nt_ram.get(), 0x400);

	// vertical mirroring. TODO: replace this with proper code
	m_nt_page[1]->set_entry(1);
	m_nt_page[3]->set_entry(1);

	m_play_timer = timer_alloc(FUNC(m82_state::m82_play_timer_cb), this);
}

void m82_state::machine_reset()
{
	switch (m_cn12->read())
	{
		case 1: m_time_limit =       30; break;
		case 2: m_time_limit =   3 * 60; break;
		case 4: m_time_limit =   6 * 60; break;
		case 8: m_time_limit = 128 * 60; break;
	}

	m_curr_slot = 0;
}


void m82_state::nes_m82(machine_config &config)
{
	// basic machine hardware
	RP2A03G(config, m_maincpu, NTSC_APU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &m82_state::m82_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(RP2A03_NTSC_XTAL / 4, 341, 0, VISIBLE_SCREEN_WIDTH, ppu2c0x_device::NTSC_SCANLINES_PER_FRAME, 0, VISIBLE_SCREEN_HEIGHT);
	m_screen->set_screen_update(m_ppu, FUNC(ppu2c0x_device::screen_update));

	PPU_2C02(config, m_ppu);
	m_ppu->set_addrmap(0, &m82_state::m82_ppu_map);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);

	NES_CONTROL_PORT(config, m_ctrl[0], famibox_control_port12_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_ctrl[1], famibox_control_port12_devices, nullptr).set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_ctrl[2], famibox_control_port12_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_ctrl[3], famibox_control_port12_devices, nullptr).set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_ctrl[4], famibox_control_port12_devices, "zapper").set_screen_tag(m_screen);
}



ROM_START( m82 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m82-prg v.1.0.ic31", 0xc000, 0x4000, CRC(7d56840a) SHA1(cbd2d14fa073273ba58367758f40d67fd8a9106d) )

	ROM_REGION( 0x2000, "ppu", 0 )
	ROM_LOAD( "m82-chr v.1.0.ic26", 0x0000, 0x2000, CRC(8e19c2b1) SHA1(1d9a34a8eadf1a6c4806d0da96f1ab690389cbd7) )
ROM_END

} // anonymous namespace


CONS( 1986, m82,  0,   0,   nes_m82, nes_m82, m82_state, empty_init, "Nintendo", "M82 Game Selectable Working Product Display",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )

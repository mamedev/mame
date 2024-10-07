// license:BSD-3-Clause
// copyright-holders:kmg

/***************************************************************************

  Nintendo NES M8 Game Selectable Working Product Display

  This was the first store display unit put out by Nintendo of America.

  Internally it contains a near-stock NES main PCB and a NES M8 board.
  The NES M8 board connects to the NES via the standard cartridge slot
  (by way of a 72-pin cart adapter as the M8 board's edge connector is
  60-pin Famicom-style), and multiplexes 16 pairs of PRG and CHR EPROMs.
  The near-stock NES is missing its CIC chip, and an additional wire
  runs from the M8 board to the host reset line of the CIC (that is the
  M8 board can directly reset the 2A03).

  The M8 board is also connected to the NES M8 LED board. This board
  houses the Game Select switch and 16 LEDs, one of which lights up near
  the title of the currently active game. The M8 board also has another
  connector that goes to a Play Time Limit nob on the back of the unit.

  Three standard NES controller connectors are hidden away in the case,
  and things are rewired so that 2 and 3 share the same lines. The M8
  unit was shipped with two joypads on 1 and 2, and a zapper on 3.

  TODO:
   - Determine how timer and game select button actually operate. Does the
     timer cycle the games like the game select button? Is the timer reset
     after game select button is pressed? What happens if the timer nob is
     adjusted while the machine is running? Timer chip is an MC1455PI.

***************************************************************************/

#include "emu.h"

#include "bus/nes_ctrl/ctrl.h"
#include "cpu/m6502/rp2a03.h"
#include "video/ppu2c0x.h"

#include "screen.h"
#include "speaker.h"


namespace {

class m8_state : public driver_device
{
public:
	m8_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppu(*this, "ppu")
		, m_screen(*this, "screen")
		, m_ctrl(*this, "ctrl%u", 1U)
		, m_dsw(*this, "DSW%u", 0U)
		, m_s1(*this, "S1")
		, m_nt_page(*this, "nt_page%u", 0U)
		, m_prg(*this, "prg%u", 0U)
		, m_chr(*this, "chr%u", 0U)
		, m_prg_bank(*this, "prg_bank")
		, m_chr_bank(*this, "chr_bank")
	{
	}

	void nes_m8(machine_config &config);
	DECLARE_INPUT_CHANGED_MEMBER(m8_dsw_changed);
	DECLARE_INPUT_CHANGED_MEMBER(m8_game_select);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<rp2a03_device> m_maincpu;
	required_device<ppu2c0x_device> m_ppu;
	required_device<screen_device> m_screen;
	optional_device_array<nes_control_port_device, 3> m_ctrl;

	required_ioport_array<4> m_dsw;
	required_ioport m_s1;

	required_memory_bank_array<4> m_nt_page;
	std::unique_ptr<u8 []> m_nt_ram;

	required_memory_region_array<16> m_prg;
	required_memory_region_array<16> m_chr;
	required_memory_bank m_prg_bank;
	required_memory_bank m_chr_bank;

	u8 m_curr_game = 0;
	u16 m_time_limit = 0;
	emu_timer* m_play_timer = nullptr;

	TIMER_CALLBACK_MEMBER(m8_play_timer_cb);
	u8 m8_in0_r();
	u8 m8_in1_r();
	void m8_in0_w(u8 data);
	void m8_set_mirroring();
	void m8_romswitch();
	void m8_reset();
	void m8_map(address_map &map) ATTR_COLD;
	void m8_ppu_map(address_map &map) ATTR_COLD;
};


INPUT_CHANGED_MEMBER(m8_state::m8_dsw_changed)
{
	m8_set_mirroring();
}

INPUT_CHANGED_MEMBER(m8_state::m8_game_select)
{
	if (newval)
		m8_reset();
}

u8 m8_state::m8_in0_r()
{
	u8 ret = 0x40;

	ret |= m_ctrl[0]->read_bit0();
	ret |= m_ctrl[0]->read_bit34();

	return ret;
}

u8 m8_state::m8_in1_r()
{
	u8 ret = 0x40;

	// both 2nd and 3rd controllers are tied to $4017
	ret |= m_ctrl[1]->read_bit0();
	ret |= m_ctrl[1]->read_bit34();
	ret |= m_ctrl[2]->read_bit0();
	ret |= m_ctrl[2]->read_bit34();

	return ret;
}

void m8_state::m8_in0_w(u8 data)
{
	for (auto &ctrl : m_ctrl)
		ctrl->write(data);
}


/**************************************************************************/

void m8_state::m8_set_mirroring()
{
	// set nametable mirroring directly from DSW
	int dip = m_curr_game / 4;
	int sw = m_curr_game & 3;
	int bit = BIT(m_dsw[dip]->read(), sw);
	for (int i = 0; i < 4; i++)
		m_nt_page[i]->set_entry(BIT(i, bit));
}

void m8_state::m8_romswitch()
{
	m_prg_bank->set_entry(m_curr_game);
	m_chr_bank->set_entry(m_curr_game);
	m8_set_mirroring();
}

void m8_state::m8_reset()
{
	m_play_timer->adjust(attotime::from_seconds(m_time_limit));
	m_curr_game = (m_curr_game + 1) & 0x0f;
	m8_romswitch();
	m_maincpu->reset();
}

TIMER_CALLBACK_MEMBER(m8_state::m8_play_timer_cb)
{
	m8_reset();
}

/***************************************************************************

   Memory map

***************************************************************************/

void m8_state::m8_map(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::read), FUNC(ppu2c0x_device::write));
	map(0x4014, 0x4014).w(m_ppu, FUNC(ppu2c0x_device::spriteram_dma));
	map(0x4016, 0x4016).rw(FUNC(m8_state::m8_in0_r), FUNC(m8_state::m8_in0_w)); // IN0 - input port 1
	map(0x4017, 0x4017).r(FUNC(m8_state::m8_in1_r));     // IN1 - input port 2 / PSG second control register

	map(0x8000, 0xffff).bankr(m_prg_bank);
}

void m8_state::m8_ppu_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr(m_chr_bank);
	map(0x2000, 0x23ff).mirror(0x1000).bankrw(m_nt_page[0]);
	map(0x2400, 0x27ff).mirror(0x1000).bankrw(m_nt_page[1]);
	map(0x2800, 0x2bff).mirror(0x1000).bankrw(m_nt_page[2]);
	map(0x2c00, 0x2fff).mirror(0x1000).bankrw(m_nt_page[3]);
	map(0x3f00, 0x3fff).rw(m_ppu, FUNC(ppu2c0x_device::palette_read), FUNC(ppu2c0x_device::palette_write));
}



static INPUT_PORTS_START( m8_base )
	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Game Select") PORT_CODE( KEYCODE_0 ) PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_game_select, 0)

	PORT_START("S1")
	PORT_CONFNAME( 0x0f, 0x08, "Play Time Limit" )
	PORT_CONFSETTING(    0x01, "20 sec." )
	PORT_CONFSETTING(    0x02, "3 min." )
	PORT_CONFSETTING(    0x04, "6 min." )
	PORT_CONFSETTING(    0x08, "25 min." )
INPUT_PORTS_END

static INPUT_PORTS_START( nes_m8 )
	PORT_INCLUDE( m8_base )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "Splash Screen Mirroring" )        PORT_DIPLOCATION("DS1:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x00, "Duck Hunt Mirroring" )            PORT_DIPLOCATION("DS1:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x00, "Hogan's Alley Mirroring" )        PORT_DIPLOCATION("DS1:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x00, "Wild Gunman Mirroring" )          PORT_DIPLOCATION("DS1:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Balloon Fight Mirroring" )        PORT_DIPLOCATION("DS2:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x02, "Ice Climber Mirroring" )          PORT_DIPLOCATION("DS2:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x00, "Kung Fu Mirroring" )              PORT_DIPLOCATION("DS2:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x08, "Pinball Mirroring" )              PORT_DIPLOCATION("DS2:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "Super Mario Bros. Mirroring" )    PORT_DIPLOCATION("DS3:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x02, "Baseball Mirroring" )             PORT_DIPLOCATION("DS3:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x04, "Golf Mirroring" )                 PORT_DIPLOCATION("DS3:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x08, "Tennis Mirroring" )               PORT_DIPLOCATION("DS3:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "Excitebike Mirroring" )           PORT_DIPLOCATION("DS4:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x00, "Mach Rider Mirroring" )           PORT_DIPLOCATION("DS4:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x04, "Wrecking Crew Mirroring" )        PORT_DIPLOCATION("DS4:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x08, "Popeye Mirroring" )               PORT_DIPLOCATION("DS4:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )
INPUT_PORTS_END

static INPUT_PORTS_START( nes_m8a )
	PORT_INCLUDE( m8_base )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "Tennis Mirroring" )               PORT_DIPLOCATION("DS1:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x02, "Baseball Mirroring" )             PORT_DIPLOCATION("DS1:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x00, "Soccer Mirroring" )               PORT_DIPLOCATION("DS1:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x08, "Golf Mirroring" )                 PORT_DIPLOCATION("DS1:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Donkey Kong Jr. Math Mirroring" ) PORT_DIPLOCATION("DS2:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x00, "Wild Gunman Mirroring" )          PORT_DIPLOCATION("DS2:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x00, "Hogan's Alley Mirroring" )        PORT_DIPLOCATION("DS2:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x00, "Duck Hunt Mirroring" )            PORT_DIPLOCATION("DS2:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Wrecking Crew Mirroring" )        PORT_DIPLOCATION("DS3:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x00, "Excitebike Mirroring" )           PORT_DIPLOCATION("DS3:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x00, "Kung Fu Mirroring" )              PORT_DIPLOCATION("DS3:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x00, "Super Mario Bros. Mirroring" )    PORT_DIPLOCATION("DS3:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Ice Climber Mirroring" )          PORT_DIPLOCATION("DS4:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x02, "Pinball Mirroring" )              PORT_DIPLOCATION("DS4:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x04, "Clu Clu Land Mirroring" )         PORT_DIPLOCATION("DS4:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x00, "Splash Screen Mirroring" )        PORT_DIPLOCATION("DS1:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )
INPUT_PORTS_END

static INPUT_PORTS_START( nes_m8b )
	PORT_INCLUDE( m8_base )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "Tennis Mirroring" )               PORT_DIPLOCATION("DS1:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x02, "Baseball Mirroring" )             PORT_DIPLOCATION("DS1:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x00, "Soccer Mirroring" )               PORT_DIPLOCATION("DS1:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x08, "10-Yard Fight Mirroring" )        PORT_DIPLOCATION("DS1:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "Golf Mirroring" )                 PORT_DIPLOCATION("DS2:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x00, "Wild Gunman Mirroring" )          PORT_DIPLOCATION("DS2:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x00, "Hogan's Alley Mirroring" )        PORT_DIPLOCATION("DS2:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x00, "Duck Hunt Mirroring" )            PORT_DIPLOCATION("DS2:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Wrecking Crew Mirroring" )        PORT_DIPLOCATION("DS3:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x00, "Excitebike Mirroring" )           PORT_DIPLOCATION("DS3:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x00, "Kung Fu Mirroring" )              PORT_DIPLOCATION("DS3:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x00, "Super Mario Bros. Mirroring" )    PORT_DIPLOCATION("DS3:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "Ice Climber Mirroring" )          PORT_DIPLOCATION("DS4:1") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x01, "V" )
	PORT_DIPNAME( 0x02, 0x02, "Pinball Mirroring" )              PORT_DIPLOCATION("DS4:2") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x02, "V" )
	PORT_DIPNAME( 0x04, 0x04, "Clu Clu Land Mirroring" )         PORT_DIPLOCATION("DS4:3") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x04, "V" )
	PORT_DIPNAME( 0x08, 0x08, "Stack Up Mirroring" )             PORT_DIPLOCATION("DS4:4") PORT_CHANGED_MEMBER(DEVICE_SELF, m8_state, m8_dsw_changed, 0)
	PORT_DIPSETTING(    0x00, "H" )
	PORT_DIPSETTING(    0x08, "V" )
INPUT_PORTS_END



void m8_state::machine_start()
{
	m_nt_ram = std::make_unique<u8[]>(0x800);

	for (auto &page : m_nt_page)
		page->configure_entries(0, 2, m_nt_ram.get(), 0x400);

	for (int i = 0; i < 16; i++)
	{
		m_prg_bank->configure_entry(i, m_prg[i]->base());
		m_chr_bank->configure_entry(i, m_chr[i]->base());
	}

	m_play_timer = timer_alloc(FUNC(m8_state::m8_play_timer_cb), this);

	save_item(NAME(m_curr_game));
	save_item(NAME(m_time_limit));
	save_pointer(NAME(m_nt_ram), 0x800);
}

void m8_state::machine_reset()
{
	switch (m_s1->read())
	{
		case 1: m_time_limit =      20; break;
		case 2: m_time_limit =  3 * 60; break;
		case 4: m_time_limit =  6 * 60; break;
		case 8: m_time_limit = 25 * 60; break;
	}

	m_curr_game = 0xf;
	m8_reset();
}


void m8_state::nes_m8(machine_config &config)
{
	// basic machine hardware
	RP2A03G(config, m_maincpu, NTSC_APU_CLOCK); // actual model is RP2A03E
	m_maincpu->set_addrmap(AS_PROGRAM, &m8_state::m8_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(RP2A03_NTSC_XTAL / 4, 341, 0, VISIBLE_SCREEN_WIDTH, ppu2c0x_device::NTSC_SCANLINES_PER_FRAME, 0, VISIBLE_SCREEN_HEIGHT);
	m_screen->set_screen_update(m_ppu, FUNC(ppu2c0x_device::screen_update));

	PPU_2C02(config, m_ppu);
	m_ppu->set_addrmap(0, &m8_state::m8_ppu_map);
	m_ppu->set_cpu_tag(m_maincpu);
	m_ppu->int_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	m_maincpu->add_route(ALL_OUTPUTS, "mono", 0.50);

	NES_CONTROL_PORT(config, m_ctrl[0], famibox_control_port12_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_ctrl[1], famibox_control_port12_devices, "joypad").set_screen_tag(m_screen);
	NES_CONTROL_PORT(config, m_ctrl[2], famibox_control_port12_devices, "zapper").set_screen_tag(m_screen);
}



ROM_START( nesm8 )
	// M8 splash screen and instructions
	ROM_REGION( 0x8000, "prg0", 0 )
	ROM_LOAD( "m8-prg-2 h.u1", 0x0000, 0x4000, CRC(15a0d398) SHA1(3601ee85d568271a1ebaae058191b092a8dcbcdc) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr0", 0 )
	ROM_LOAD( "m8-chr h.u29",  0x0000, 0x2000, CRC(7a7ad128) SHA1(a22be640cf04a6fdd9ec3b291b33d10ecbee0ef8) )

	// Duck Hunt
	ROM_REGION( 0x8000, "prg1", 0 )
	ROM_LOAD( "dh-prg h.u2",   0x0000, 0x4000, CRC(90ca616d) SHA1(b742576317cd6a04caac25252d5593844c9a0bb6) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr1", 0 )
	ROM_LOAD( "dh-chr h.u30",  0x0000, 0x2000, CRC(4e049e03) SHA1(ffad32a3bab2fb3826bc554b1b9838e837513576) )

	// Hogan's Alley
	ROM_REGION( 0x8000, "prg2", 0 )
	ROM_LOAD( "ha-prg h.u3",   0x0000, 0x4000, CRC(8963ae6e) SHA1(bca489ed0fb58e1e99f36c427bc0d7d805b6c61a) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr2", 0 )
	ROM_LOAD( "ha-chr h.u31",  0x0000, 0x2000, CRC(5df42fc4) SHA1(4fcf23151d9f11c1ef1b1007dd8058f5d5fe9ab8) )

	// Wild Gunman
	ROM_REGION( 0x8000, "prg3", 0 )
	ROM_LOAD( "wg-prg h.u4",   0x0000, 0x4000, CRC(389960db) SHA1(6b38f2c86ef27f653a2bdb9c682ac0bc981c7db6) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr3", 0 )
	ROM_LOAD( "wg-chr h.u32",  0x0000, 0x2000, CRC(a5e04856) SHA1(9194d89a34f687742216889cbb3e717a9ae81c92) )

	// Balloon Fight
	ROM_REGION( 0x8000, "prg4", 0 )
	ROM_LOAD( "bf-prg v.u5",   0x0000, 0x4000, CRC(bd2e9025) SHA1(6742fa2ea498e5b73dc2024ad3888a64e6b894e4) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr4", 0 )
	ROM_LOAD( "bf-chr v.u33",  0x0000, 0x2000, CRC(c642a1df) SHA1(e73cd3d4c0bad8e6f7a1aa6a580f3817a83756a9) )

	// Ice Climber
	ROM_REGION( 0x8000, "prg5", 0 )
	ROM_LOAD( "ic-prg v.u6",   0x0000, 0x4000, CRC(d548307f) SHA1(e323ebaa2c0be5b9a31a21743ddfe7f3d1580672) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr5", 0 )
	ROM_LOAD( "ic-chr v.u34",  0x0000, 0x2000, CRC(0d58a8b1) SHA1(7ae5f457bb5996a5373c9cf46db3d296e27ec56a) )

	// Kung Fu
	ROM_REGION( 0x8000, "prg6", 0 )
	ROM_LOAD( "sx-prg h.u7",   0x0000, 0x8000, CRC(0516375e) SHA1(55dc3550c6133f8624eb6cf3d2f145e4313c2ff6) )

	ROM_REGION( 0x2000, "chr6", 0 )
	ROM_LOAD( "sx-chr h.u35",  0x0000, 0x2000, CRC(430b49a4) SHA1(7e618dbff521c3d5ee0f3d8bb01d2e770395a6bc) )

	// Pinball
	ROM_REGION( 0x8000, "prg7", 0 )
	ROM_LOAD( "pn-prg v.u8",   0x0000, 0x4000, CRC(91d33e3c) SHA1(607c5954e5c577ac78db3234987f8fe62f86f068) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr7", 0 )
	ROM_LOAD( "pn-chr v.u36",  0x0000, 0x2000, CRC(f2a53b3d) SHA1(0d2a521fd984c76cbf9b3cbd47e68c6ce4c6eeae) )

	// Super Mario Bros.
	ROM_REGION( 0x8000, "prg8", 0 )
	ROM_LOAD( "sm-prg h.u15",  0x0000, 0x8000, CRC(5cf548d3) SHA1(fefa1097449a3a11ebf8c6199e905996c5dc8fbd) )

	ROM_REGION( 0x2000, "chr8", 0 )
	ROM_LOAD( "sm-chr h.u40",  0x0000, 0x2000, CRC(867b51ad) SHA1(394badaf0b0bdd0ea279a1bca89a9d9ddc00b1b5) )

	// Baseball
	ROM_REGION( 0x8000, "prg9", 0 )
	ROM_LOAD( "ba-prg v.u16",  0x0000, 0x4000, CRC(39d1fa03) SHA1(28d84cfefa81bbfd3d26e0f70f1b9f53383e54ad) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr9", 0 )
	ROM_LOAD( "ba-chr v.u41",  0x0000, 0x2000, CRC(cde71b82) SHA1(296ccef8a1fd9209f414ce0c788ab0dc95058242) )

	// Golf
	ROM_REGION( 0x8000, "prg10", 0 )
	ROM_LOAD( "gf-prg v.u17",  0x0000, 0x4000, CRC(f9622bfa) SHA1(b4e341a91f614bb19c67cc0205b2443591567aea) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr10", 0 )
	ROM_LOAD( "gf-chr v.u42",  0x0000, 0x2000, CRC(ff6fc790) SHA1(40177839b61f375f2ad03b203328683264845b5b) )

	// Tennis
	ROM_REGION( 0x8000, "prg11", 0 )
	ROM_LOAD( "te-prg h.u18",  0x0000, 0x4000, CRC(8b2e3e81) SHA1(e54274c0b0d651458c5459d41872b1f99904d0fb) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr11", 0 )
	ROM_LOAD( "te-chr h.u43",  0x0000, 0x2000, CRC(3a34c45b) SHA1(2cc26a01c38ead50503dccb3ee929ba7a2b6772c) )

	// Excitebike
	ROM_REGION( 0x8000, "prg12", 0 )
	ROM_LOAD( "eb-prg h.u19",  0x0000, 0x4000, CRC(3a94fa0b) SHA1(6239e91ccefdc017d233cbae388c6568a17ed04b) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr12", 0 )
	ROM_LOAD( "eb-chr h.u44",  0x0000, 0x2000, CRC(e5f72401) SHA1(a8bf028e1a62677e48e88cf421bb2a8051eb800c) )

	// Mach Rider
	ROM_REGION( 0x8000, "prg13", 0 )
	ROM_LOAD( "mr-prg h.u20",  0x0000, 0x8000, CRC(af2bbcbc) SHA1(79b0886c35137b1f31f86e935574c1816a823851) )

	ROM_REGION( 0x2000, "chr13", 0 )
	ROM_LOAD( "mr-chr h.u45",  0x0000, 0x2000, CRC(33a2b41a) SHA1(671f37bce742e63250296e62c143f8a82f860b04) )

	// Wrecking Crew
	ROM_REGION( 0x8000, "prg14", 0 )
	ROM_LOAD( "wr-prg v.u21",  0x0000, 0x8000, CRC(4328b273) SHA1(764d68f05f4a6e43fb26d7e654e237d2b0258fe4) )

	ROM_REGION( 0x2000, "chr14", 0 )
	ROM_LOAD( "wr-chr v.u46",  0x0000, 0x2000, CRC(23f0b9fd) SHA1(c7f2d4f5f555490847654b8458687f94fba3bd12) )

	// Popeye
	ROM_REGION( 0x8000, "prg15", 0 )
	ROM_LOAD( "pp-prg v.u22",  0x0000, 0x4000, CRC(0fa63a45) SHA1(50c594a6d8dcbeee2d83bca8c54c42cf57093aba) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr15", 0 )
	ROM_LOAD( "pp-chr v.u47",  0x0000, 0x2000, CRC(a5fd8d98) SHA1(09d229404babb6c89b417ac541bab80fb06d2ba9) )
ROM_END

// ROMs, labels, and locations all need confirmed
ROM_START( nesm8a )
	// Tennis
	ROM_REGION( 0x8000, "prg0", 0 )
	ROM_LOAD( "te-prg h.u1",   0x0000, 0x4000, CRC(8b2e3e81) SHA1(e54274c0b0d651458c5459d41872b1f99904d0fb) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr0", 0 )
	ROM_LOAD( "te-chr h.u29",  0x0000, 0x2000, CRC(3a34c45b) SHA1(2cc26a01c38ead50503dccb3ee929ba7a2b6772c) )

	// Baseball
	ROM_REGION( 0x8000, "prg1", 0 )
	ROM_LOAD( "ba-prg v.u2",   0x0000, 0x4000, CRC(39d1fa03) SHA1(28d84cfefa81bbfd3d26e0f70f1b9f53383e54ad) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr1", 0 )
	ROM_LOAD( "ba-chr v.u30",  0x0000, 0x2000, CRC(cde71b82) SHA1(296ccef8a1fd9209f414ce0c788ab0dc95058242) )

	// Soccer
	ROM_REGION( 0x8000, "prg2", 0 )
	ROM_LOAD( "sc-prg h.u3",   0x0000, 0x8000, CRC(32e37dcb) SHA1(9d881111634c88218a58e4404848f595824f1c0c) )

	ROM_REGION( 0x2000, "chr2", 0 )
	ROM_LOAD( "sc-chr h.u31",  0x0000, 0x2000, CRC(307b19ab) SHA1(b35ef4c2cf071db77cec1b4529b43a20cfcce172) )

	// Golf
	ROM_REGION( 0x8000, "prg3", 0 )
	ROM_LOAD( "gf-prg v.u4",   0x0000, 0x4000, CRC(f9622bfa) SHA1(b4e341a91f614bb19c67cc0205b2443591567aea) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr3", 0 )
	ROM_LOAD( "gf-chr v.u32",  0x0000, 0x2000, CRC(ff6fc790) SHA1(40177839b61f375f2ad03b203328683264845b5b) )

	// Donkey Kong Jr. Math
	ROM_REGION( 0x8000, "prg4", 0 )
	ROM_LOAD( "ca-prg h.u5",   0x0000, 0x4000, CRC(66f11648) SHA1(4e243d05d60ee578a720f6fadd6c3ebde305605b) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr4", 0 )
	ROM_LOAD( "ca-chr h.u33",  0x0000, 0x2000, CRC(73329878) SHA1(4c0c87f38af851bd605ccd980fb87ef49dfe98c8) )

	// Wild Gunman
	ROM_REGION( 0x8000, "prg5", 0 )
	ROM_LOAD( "wg-prg h.u6",   0x0000, 0x4000, CRC(389960db) SHA1(6b38f2c86ef27f653a2bdb9c682ac0bc981c7db6) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr5", 0 )
	ROM_LOAD( "wg-chr h.u34",  0x0000, 0x2000, CRC(a5e04856) SHA1(9194d89a34f687742216889cbb3e717a9ae81c92) )

	// Hogan's Alley
	ROM_REGION( 0x8000, "prg6", 0 )
	ROM_LOAD( "ha-prg h.u7",   0x0000, 0x4000, CRC(8963ae6e) SHA1(bca489ed0fb58e1e99f36c427bc0d7d805b6c61a) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr6", 0 )
	ROM_LOAD( "ha-chr h.u35",  0x0000, 0x2000, CRC(5df42fc4) SHA1(4fcf23151d9f11c1ef1b1007dd8058f5d5fe9ab8) )

	// Duck Hunt
	ROM_REGION( 0x8000, "prg7", 0 )
	ROM_LOAD( "dh-prg h.u8",   0x0000, 0x4000, CRC(90ca616d) SHA1(b742576317cd6a04caac25252d5593844c9a0bb6) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr7", 0 )
	ROM_LOAD( "dh-chr h.u36",  0x0000, 0x2000, CRC(4e049e03) SHA1(ffad32a3bab2fb3826bc554b1b9838e837513576) )

	// Wrecking Crew
	ROM_REGION( 0x8000, "prg8", 0 )
	ROM_LOAD( "wr-prg v.u15",  0x0000, 0x8000, CRC(4328b273) SHA1(764d68f05f4a6e43fb26d7e654e237d2b0258fe4) )

	ROM_REGION( 0x2000, "chr8", 0 )
	ROM_LOAD( "wr-chr v.u40",  0x0000, 0x2000, CRC(23f0b9fd) SHA1(c7f2d4f5f555490847654b8458687f94fba3bd12) )

	// Excitebike
	ROM_REGION( 0x8000, "prg9", 0 )
	ROM_LOAD( "eb-prg h.u16",  0x0000, 0x4000, CRC(3a94fa0b) SHA1(6239e91ccefdc017d233cbae388c6568a17ed04b) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr9", 0 )
	ROM_LOAD( "eb-chr h.u41",  0x0000, 0x2000, CRC(e5f72401) SHA1(a8bf028e1a62677e48e88cf421bb2a8051eb800c) )

	// Kung Fu
	ROM_REGION( 0x8000, "prg10", 0 )
	ROM_LOAD( "sx-prg h.u17",  0x0000, 0x8000, CRC(0516375e) SHA1(55dc3550c6133f8624eb6cf3d2f145e4313c2ff6) )

	ROM_REGION( 0x2000, "chr10", 0 )
	ROM_LOAD( "sx-chr h.u42",  0x0000, 0x2000, CRC(430b49a4) SHA1(7e618dbff521c3d5ee0f3d8bb01d2e770395a6bc) )

	// Super Mario Bros.
	ROM_REGION( 0x8000, "prg11", 0 )
	ROM_LOAD( "sm-prg h.u18",  0x0000, 0x8000, CRC(5cf548d3) SHA1(fefa1097449a3a11ebf8c6199e905996c5dc8fbd) )

	ROM_REGION( 0x2000, "chr11", 0 )
	ROM_LOAD( "sm-chr h.u43",  0x0000, 0x2000, CRC(867b51ad) SHA1(394badaf0b0bdd0ea279a1bca89a9d9ddc00b1b5) )

	// Ice Climber
	ROM_REGION( 0x8000, "prg12", 0 )
	ROM_LOAD( "ic-prg v.u19",  0x0000, 0x4000, CRC(d548307f) SHA1(e323ebaa2c0be5b9a31a21743ddfe7f3d1580672) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr12", 0 )
	ROM_LOAD( "ic-chr v.u44",  0x0000, 0x2000, CRC(0d58a8b1) SHA1(7ae5f457bb5996a5373c9cf46db3d296e27ec56a) )

	// Pinball
	ROM_REGION( 0x8000, "prg13", 0 )
	ROM_LOAD( "pn-prg v.u20",  0x0000, 0x4000, CRC(91d33e3c) SHA1(607c5954e5c577ac78db3234987f8fe62f86f068) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr13", 0 )
	ROM_LOAD( "pn-chr v.u45",  0x0000, 0x2000, CRC(f2a53b3d) SHA1(0d2a521fd984c76cbf9b3cbd47e68c6ce4c6eeae) )

	// Clu Clu Land
	ROM_REGION( 0x8000, "prg14", 0 )
	ROM_LOAD( "cl-prg v.u21",  0x0000, 0x4000, CRC(23a60a62) SHA1(36003de2a208bf132f1ecbf7af7efd9095ef85c8) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr14", 0 )
	ROM_LOAD( "cl-chr v.u46",  0x0000, 0x2000, CRC(a63b8b98) SHA1(8ddf97dcc8e28f2218c4a4c8b3d9d527fa0525fb) )

	// M8 splash screen and instructions
	ROM_REGION( 0x8000, "prg15", 0 )
	ROM_LOAD( "m8-prg h.u22",  0x0000, 0x4000, CRC(15a0d398) SHA1(3601ee85d568271a1ebaae058191b092a8dcbcdc) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr15", 0 )
	ROM_LOAD( "m8-chr h.u47",  0x0000, 0x2000, CRC(7a7ad128) SHA1(a22be640cf04a6fdd9ec3b291b33d10ecbee0ef8) )
ROM_END

// ROMs, labels, and locations all need confirmed
ROM_START( nesm8b )
	// Tennis
	ROM_REGION( 0x8000, "prg0", 0 )
	ROM_LOAD( "te-prg h.u1",   0x0000, 0x4000, CRC(8b2e3e81) SHA1(e54274c0b0d651458c5459d41872b1f99904d0fb) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr0", 0 )
	ROM_LOAD( "te-chr h.u29",  0x0000, 0x2000, CRC(3a34c45b) SHA1(2cc26a01c38ead50503dccb3ee929ba7a2b6772c) )

	// Baseball
	ROM_REGION( 0x8000, "prg1", 0 )
	ROM_LOAD( "ba-prg v.u2",   0x0000, 0x4000, CRC(39d1fa03) SHA1(28d84cfefa81bbfd3d26e0f70f1b9f53383e54ad) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr1", 0 )
	ROM_LOAD( "ba-chr v.u30",  0x0000, 0x2000, CRC(cde71b82) SHA1(296ccef8a1fd9209f414ce0c788ab0dc95058242) )

	// Soccer
	ROM_REGION( 0x8000, "prg2", 0 )
	ROM_LOAD( "sc-prg h.u3",   0x0000, 0x8000, CRC(32e37dcb) SHA1(9d881111634c88218a58e4404848f595824f1c0c) )

	ROM_REGION( 0x2000, "chr2", 0 )
	ROM_LOAD( "sc-chr h.u31",  0x0000, 0x2000, CRC(307b19ab) SHA1(b35ef4c2cf071db77cec1b4529b43a20cfcce172) )

	// 10-Yard Fight
	ROM_REGION( 0x8000, "prg3", 0 )
	ROM_LOAD( "ty-prg v.u4",   0x0000, 0x8000, CRC(df58fc5a) SHA1(7cc69b39ece2168574599d45ab8452edd4f5a3a1) )

	ROM_REGION( 0x2000, "chr3", 0 )
	ROM_LOAD( "ty-chr v.u32",  0x0000, 0x2000, CRC(2b8336ee) SHA1(6b40a3b7ad6214abf9c35bcbf89d9c1da06f47a0) )

	// Golf
	ROM_REGION( 0x8000, "prg4", 0 )
	ROM_LOAD( "gf-prg v.u5",   0x0000, 0x4000, CRC(f9622bfa) SHA1(b4e341a91f614bb19c67cc0205b2443591567aea) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr4", 0 )
	ROM_LOAD( "gf-chr v.u33",  0x0000, 0x2000, CRC(ff6fc790) SHA1(40177839b61f375f2ad03b203328683264845b5b) )

	// Wild Gunman
	ROM_REGION( 0x8000, "prg5", 0 )
	ROM_LOAD( "wg-prg h.u6",   0x0000, 0x4000, CRC(389960db) SHA1(6b38f2c86ef27f653a2bdb9c682ac0bc981c7db6) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr5", 0 )
	ROM_LOAD( "wg-chr h.u34",  0x0000, 0x2000, CRC(a5e04856) SHA1(9194d89a34f687742216889cbb3e717a9ae81c92) )

	// Hogan's Alley
	ROM_REGION( 0x8000, "prg6", 0 )
	ROM_LOAD( "ha-prg h.u7",   0x0000, 0x4000, CRC(8963ae6e) SHA1(bca489ed0fb58e1e99f36c427bc0d7d805b6c61a) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr6", 0 )
	ROM_LOAD( "ha-chr h.u35",  0x0000, 0x2000, CRC(5df42fc4) SHA1(4fcf23151d9f11c1ef1b1007dd8058f5d5fe9ab8) )

	// Duck Hunt
	ROM_REGION( 0x8000, "prg7", 0 )
	ROM_LOAD( "dh-prg h.u8",   0x0000, 0x4000, CRC(90ca616d) SHA1(b742576317cd6a04caac25252d5593844c9a0bb6) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr7", 0 )
	ROM_LOAD( "dh-chr h.u36",  0x0000, 0x2000, CRC(4e049e03) SHA1(ffad32a3bab2fb3826bc554b1b9838e837513576) )

	// Wrecking Crew
	ROM_REGION( 0x8000, "prg8", 0 )
	ROM_LOAD( "wr-prg v.u15",  0x0000, 0x8000, CRC(4328b273) SHA1(764d68f05f4a6e43fb26d7e654e237d2b0258fe4) )

	ROM_REGION( 0x2000, "chr8", 0 )
	ROM_LOAD( "wr-chr v.u40",  0x0000, 0x2000, CRC(23f0b9fd) SHA1(c7f2d4f5f555490847654b8458687f94fba3bd12) )

	// Excitebike
	ROM_REGION( 0x8000, "prg9", 0 )
	ROM_LOAD( "eb-prg h.u16",  0x0000, 0x4000, CRC(3a94fa0b) SHA1(6239e91ccefdc017d233cbae388c6568a17ed04b) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr9", 0 )
	ROM_LOAD( "eb-chr h.u41",  0x0000, 0x2000, CRC(e5f72401) SHA1(a8bf028e1a62677e48e88cf421bb2a8051eb800c) )

	// Kung Fu
	ROM_REGION( 0x8000, "prg10", 0 )
	ROM_LOAD( "sx-prg h.u17",  0x0000, 0x8000, CRC(0516375e) SHA1(55dc3550c6133f8624eb6cf3d2f145e4313c2ff6) )

	ROM_REGION( 0x2000, "chr10", 0 )
	ROM_LOAD( "sx-chr h.u42",  0x0000, 0x2000, CRC(430b49a4) SHA1(7e618dbff521c3d5ee0f3d8bb01d2e770395a6bc) )

	// Super Mario Bros.
	ROM_REGION( 0x8000, "prg11", 0 )
	ROM_LOAD( "sm-prg h.u18",  0x0000, 0x8000, CRC(5cf548d3) SHA1(fefa1097449a3a11ebf8c6199e905996c5dc8fbd) )

	ROM_REGION( 0x2000, "chr11", 0 )
	ROM_LOAD( "sm-chr h.u43",  0x0000, 0x2000, CRC(867b51ad) SHA1(394badaf0b0bdd0ea279a1bca89a9d9ddc00b1b5) )

	// Ice Climber
	ROM_REGION( 0x8000, "prg12", 0 )
	ROM_LOAD( "ic-prg v.u19",  0x0000, 0x4000, CRC(d548307f) SHA1(e323ebaa2c0be5b9a31a21743ddfe7f3d1580672) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr12", 0 )
	ROM_LOAD( "ic-chr v.u44",  0x0000, 0x2000, CRC(0d58a8b1) SHA1(7ae5f457bb5996a5373c9cf46db3d296e27ec56a) )

	// Pinball
	ROM_REGION( 0x8000, "prg13", 0 )
	ROM_LOAD( "pn-prg v.u20",  0x0000, 0x4000, CRC(91d33e3c) SHA1(607c5954e5c577ac78db3234987f8fe62f86f068) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr13", 0 )
	ROM_LOAD( "pn-chr v.u45",  0x0000, 0x2000, CRC(f2a53b3d) SHA1(0d2a521fd984c76cbf9b3cbd47e68c6ce4c6eeae) )

	// Clu Clu Land
	ROM_REGION( 0x8000, "prg14", 0 )
	ROM_LOAD( "cl-prg v.u21",  0x0000, 0x4000, CRC(23a60a62) SHA1(36003de2a208bf132f1ecbf7af7efd9095ef85c8) )
	ROM_RELOAD(                0x4000, 0x4000 )

	ROM_REGION( 0x2000, "chr14", 0 )
	ROM_LOAD( "cl-chr v.u46",  0x0000, 0x2000, CRC(a63b8b98) SHA1(8ddf97dcc8e28f2218c4a4c8b3d9d527fa0525fb) )

	// Stack Up
	ROM_REGION( 0x8000, "prg15", 0 )
	ROM_LOAD( "bl-prg v.u22",  0x0000, 0x8000, CRC(4ee735c1) SHA1(36b6b30f451c3b4a48464bee9a144c873ae04cc7) )

	ROM_REGION( 0x2000, "chr15", 0 )
	ROM_LOAD( "bl-chr v.u47",  0x0000, 0x2000, CRC(41f4b527) SHA1(49f8dabda7e8585e6961049c46ed913518cd959e) )
ROM_END

} // anonymous namespace


CONS( 1986, nesm8,  0,     0,  nes_m8, nes_m8,  m8_state, empty_init,  "Nintendo", "M8 Game Selectable Working Product Display (US, set 1)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1986, nesm8a, nesm8, 0,  nes_m8, nes_m8a, m8_state, empty_init,  "Nintendo", "M8 Game Selectable Working Product Display (US, set 2)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
CONS( 1986, nesm8b, nesm8, 0,  nes_m8, nes_m8b, m8_state, empty_init,  "Nintendo", "M8 Game Selectable Working Product Display (US, set 3)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )

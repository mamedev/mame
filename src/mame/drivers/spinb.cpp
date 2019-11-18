// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************************************

  PINBALL
  Spinball (previously Inder)

  Hardware is much the same as Inder, except the digital display is replaced by a DMD controlled by
  a i8031. You need to pick "Pixel Aspect (4:1)" video option in the tab menu.

  There are mistakes in the sound board schematic: IC14 pin12 goes to IC5 pin13 only. IC16 pin 22
  is the CS0 line only. So, they are not joined but are separate tracks. Also, according to PinMAME,
  the outputs of IC11 are all wrong. They should be (from top to bottom): A16, A17, A18, NC, NC,
  CS2, CS1, CS0.

  Also, very unobvious is the fact that PIA ports A and B are swapped around compared to the Inder
  soundcard.

Status
- Bushido: Displays Bushido logo. If you quickly press 5 you get a sound.
- Mach 2: Displays Mach2 logo. Makes a sound if 5 pressed
- Jolly Park: Displays the Spinball logo. After a few moments it plays music
- Verne's World: Display flashes for a second then goes blank. After a few moments music plays.

ToDo:
- Inputs and outputs (currently a copy of inder.c)
- DMD doesn't act on commands
- Electronic volume control on the music card


****************************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "machine/74157.h"
#include "machine/7474.h"
#include "machine/i8255.h"
#include "sound/msm5205.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


class spinb_state : public genpin_class
{
public:
	spinb_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_musiccpu(*this, "musiccpu")
		, m_dmdcpu(*this, "dmdcpu")
		, m_msm_a(*this, "msm_a")
		, m_msm_m(*this, "msm_m")
		, m_ic5a(*this, "ic5a")
		, m_ic5m(*this, "ic5m")
		, m_ic14a(*this, "ic14a")
		, m_ic14m(*this, "ic14m")
		, m_switches(*this, "SW.%u", 0)
	{ }

	void jolypark(machine_config &config);
	void vrnwrld(machine_config &config);
	void spinb(machine_config &config);

	void init_game0();
	void init_game1();
	void init_game2();

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_READ8_MEMBER(p3_r);
	DECLARE_WRITE8_MEMBER(p3_w);
	DECLARE_READ8_MEMBER(ppia_c_r);
	DECLARE_WRITE8_MEMBER(ppia_a_w);
	DECLARE_WRITE8_MEMBER(ppia_b_w);
	DECLARE_WRITE8_MEMBER(ppia_c_w);
	DECLARE_READ8_MEMBER(ppim_c_r);
	DECLARE_WRITE8_MEMBER(ppim_a_w);
	DECLARE_WRITE8_MEMBER(ppim_b_w);
	DECLARE_WRITE8_MEMBER(ppim_c_w);
	DECLARE_WRITE8_MEMBER(ppi60a_w);
	DECLARE_WRITE8_MEMBER(ppi60b_w);
	DECLARE_WRITE8_MEMBER(ppi64c_w);
	DECLARE_READ8_MEMBER(sw_r);
	DECLARE_WRITE8_MEMBER(dmdram_w);
	DECLARE_READ8_MEMBER(dmdram_r);
	DECLARE_READ8_MEMBER(sndcmd_r);
	DECLARE_WRITE8_MEMBER(sndbank_a_w);
	DECLARE_WRITE8_MEMBER(sndbank_m_w);
	DECLARE_WRITE8_MEMBER(sndcmd_w);
	DECLARE_WRITE8_MEMBER(lamp_w) { };
	DECLARE_WRITE8_MEMBER(lamp1_w) { };
	DECLARE_WRITE8_MEMBER(volume_w) { };
	DECLARE_WRITE8_MEMBER(disp_w);
	DECLARE_WRITE_LINE_MEMBER(ic5a_w);
	DECLARE_WRITE_LINE_MEMBER(ic5m_w);
	void spinb_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void dmd_io(address_map &map);
	void dmd_mem(address_map &map);
	void spinb_audio_map(address_map &map);
	void spinb_map(address_map &map);
	void spinb_music_map(address_map &map);
	void vrnwrld_map(address_map &map);

	bool m_pc0a;
	bool m_pc0m;
	uint8_t m_game;
	uint8_t m_portc_a;
	uint8_t m_portc_m;
	uint8_t m_row;
	uint8_t m_p3;
	uint8_t m_p32;
	uint8_t m_dmdcmd;
	uint8_t m_dmdbank;
	uint8_t m_dmdextaddr;
	uint8_t m_dmdram[0x2000];
	uint8_t m_sndcmd;
	uint8_t m_sndbank_a;
	uint8_t m_sndbank_m;
	uint32_t m_sound_addr_a;
	uint32_t m_sound_addr_m;
	uint8_t *m_p_audio;
	uint8_t *m_p_music;
	uint8_t *m_p_dmdcpu;
	void update_sound_a();
	void update_sound_m();
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_musiccpu;
	required_device<i8031_device> m_dmdcpu;
	required_device<msm5205_device> m_msm_a;
	required_device<msm5205_device> m_msm_m;
	required_device<ttl7474_device> m_ic5a;
	required_device<ttl7474_device> m_ic5m;
	required_device<hc157_device> m_ic14a;
	required_device<hc157_device> m_ic14m;
	required_ioport_array<11> m_switches;
};

void spinb_state::spinb_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x5fff).ram().share("nvram"); // 6164, battery-backed
	map(0x6000, 0x6003).mirror(0x13fc).rw("ppi60", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x6400, 0x6403).mirror(0x13fc).rw("ppi64", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x6800, 0x6803).mirror(0x13fc).rw("ppi68", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x6c00, 0x6c03).mirror(0x131c).rw("ppi6c", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x6c20, 0x6c3f).mirror(0x1300).w(FUNC(spinb_state::sndcmd_w));
	map(0x6c40, 0x6c45).mirror(0x1300).w(FUNC(spinb_state::lamp1_w));
	map(0x6c60, 0x6c60).mirror(0x1300).w(FUNC(spinb_state::disp_w));
	map(0x6ce0, 0x6ce0).nopw();
}

void spinb_state::vrnwrld_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram().share("nvram"); // 6164, battery-backed
	map(0xc000, 0xc003).mirror(0x13fc).rw("ppi60", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc400, 0xc403).mirror(0x13fc).rw("ppi64", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc800, 0xc803).mirror(0x13fc).rw("ppi68", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xcc00, 0xcc03).mirror(0x131c).rw("ppi6c", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xcc20, 0xcc3f).mirror(0x1300).w(FUNC(spinb_state::sndcmd_w));
	map(0xcc40, 0xcc45).mirror(0x1300).w(FUNC(spinb_state::lamp1_w));
	map(0xcc60, 0xcc60).mirror(0x1300).w(FUNC(spinb_state::disp_w));
	map(0xcce0, 0xcce0).nopw();
}

void spinb_state::spinb_audio_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).ram(); // 6164
	map(0x4000, 0x4003).mirror(0x1ffc).rw("ppia", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x6000, 0x6000).w(FUNC(spinb_state::sndbank_a_w));
	map(0x8000, 0x8000).r(FUNC(spinb_state::sndcmd_r));
}

void spinb_state::spinb_music_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).ram(); // 6164
	map(0x4000, 0x4003).mirror(0x1ffc).rw("ppim", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x6000, 0x6000).w(FUNC(spinb_state::sndbank_m_w));
	map(0x8000, 0x8000).r(FUNC(spinb_state::sndcmd_r));
	map(0xA000, 0xA000).w(FUNC(spinb_state::volume_w));
}

void spinb_state::dmd_mem(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void spinb_state::dmd_io(address_map &map)
{
	map(0x0000, 0x1fff).w(FUNC(spinb_state::dmdram_w));
	map(0x0000, 0xffff).r(FUNC(spinb_state::dmdram_r));
}

static INPUT_PORTS_START( spinb )
	PORT_START("SW.0")
	PORT_DIPNAME( 0x80, 0x80, "Balls")
	PORT_DIPSETTING(    0x80, "3")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPNAME( 0x30, 0x30, "Coin Slot 1")
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C )) // slot 2: 1 moneda 4 partidas
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C )) // and 4c_3c; slot 2: 1 moneda 3 partidas
	PORT_DIPNAME( 0x0c, 0x0c, "Points for free game")
	PORT_DIPSETTING(    0x0c, "2600000")
	PORT_DIPSETTING(    0x08, "3000000")
	PORT_DIPSETTING(    0x04, "3400000")
	PORT_DIPSETTING(    0x00, "3800000")

	PORT_START("SW.1")
	PORT_DIPNAME( 0x30, 0x30, "High Score") //"Handicap"
	PORT_DIPSETTING(    0x30, "4800000")
	PORT_DIPSETTING(    0x20, "5000000")
	PORT_DIPSETTING(    0x10, "5200000")
	PORT_DIPSETTING(    0x00, "5400000")
	PORT_DIPNAME( 0x08, 0x08, "Especial en Picabolas")
	PORT_DIPSETTING(    0x08, "1st Derribo")
	PORT_DIPSETTING(    0x00, "2nd Derribo")
	PORT_DIPNAME( 0x04, 0x04, "Bola Extra En Rampas")
	PORT_DIPSETTING(    0x04, "4 dianas")
	PORT_DIPSETTING(    0x00, "2 dianas")
	PORT_BIT( 0xc3, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.2")
	PORT_DIPNAME( 0x04, 0x04, "Quita bola extra en passillos 1,2,y 3")
	PORT_DIPSETTING(    0x04, DEF_STR(No))
	PORT_DIPSETTING(    0x00, DEF_STR(Yes))
	PORT_BIT( 0xfb, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SW.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) // "Monedero A"
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) // "Monedero B"
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_TILT ) // "Falta"
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 ) // "Pulsador Partidas"
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_NAME("Reset") // "Puesta a cero"
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Accounting info") // "Test economico"
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Test") // "Test tecnico"

	PORT_START("SW.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("SW.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("SW.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)

	PORT_START("SW.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)

	PORT_START("SW.8")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW.9")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW.10")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

/*
d0 = / enable RAM
d1 = display enable
d2 = RDATA    )
d3 = ROWCK    ) to DMD
d4 = COLATCH  )
d5 = part of the data input circuit
d6 = STAT0
d7 = STAT1

m_game == 0 : P1.0 low for RAM, P1.5 low for data input. They shouldn't both be low.
m_game == 1 : P1.0 low for RAM, P1.5 low for data input. They shouldn't both be low. Extra ROM selected by P3.4
m_game == 2 : P1.0 and P1.5 go to 74LS139 selector: 0 = RAM; 1 = ROM1; 2 = ROM2; 3 = data input
*/
WRITE8_MEMBER( spinb_state::p1_w )
{
	m_dmdbank = (BIT(data, 5) << 1) + BIT(data, 0);

	if (m_game < 2)
	{
		switch (m_dmdbank)
		{
			case 0:
				printf("DMD Illegal selection\n");
				break;
			case 1: // ram
				m_dmdbank = 0;
				break;
			case 2: // input
				m_dmdbank = 3;
				break;
			case 3: // nothing or (game1 external rom)
				m_dmdbank = 1;
				break;
		}
	}
}

READ8_MEMBER( spinb_state::p3_r )
{
	return m_p3 | m_p32;
}

/*
d0 = RXD - SDATA ) to DMD
d1 = TXD - DOTCK )
d2 = Interrupt Input when data is coming from maincpu
d6 = External /WR
d7 = External /RD
*/
WRITE8_MEMBER( spinb_state::p3_w )
{
	m_p3 = data & 0xfb;
	m_dmdextaddr = 0;

	if (m_game == 1)
		m_dmdextaddr = BIT(data, 3);// | (BIT(data, 5) << 1);
	else
	if (m_game == 2)
		m_dmdextaddr = BIT(data, 3) | (BIT(data, 5) << 1) | (BIT(data, 4) << 2);
}

READ8_MEMBER( spinb_state::sw_r )
{
	return m_switches[m_row]->read();
}

WRITE8_MEMBER( spinb_state::sndcmd_w )
{
	m_sndcmd = data;
}

READ8_MEMBER( spinb_state::sndcmd_r )
{
	return m_sndcmd;
}

WRITE8_MEMBER( spinb_state::dmdram_w )
{
	m_dmdram[offset & 0x1fff] = data;
}

READ8_MEMBER( spinb_state::dmdram_r )
{
	switch (m_dmdbank)
	{
		case 0:
			return m_dmdram[offset & 0x1fff];
		case 1:
			return m_p_dmdcpu[offset + (m_dmdextaddr << 16)];
		case 2:
			return m_p_dmdcpu[0x80000 + offset + (m_dmdextaddr << 16)];
	}

	m_p32 = 4;
	m_dmdcpu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);
	return m_dmdcmd;
}

WRITE8_MEMBER( spinb_state::disp_w )
{
	m_dmdcmd = data;
	m_p32 = 0;
	m_dmdcpu->set_input_line(MCS51_INT0_LINE, HOLD_LINE);
}

WRITE8_MEMBER( spinb_state::ppi60a_w )
{
	if (data)
		for (uint8_t i = 0; i < 8; i++)
			if (BIT(data, i))
				m_row = i;
}

// always 0 but we'll support it anyway
WRITE8_MEMBER( spinb_state::ppi60b_w )
{
	if (data & 7)
		for (uint8_t i = 0; i < 3; i++)
			if (BIT(data, i))
				m_row = i+8;
}

WRITE8_MEMBER( spinb_state::ppi64c_w )
{
}

WRITE8_MEMBER( spinb_state::sndbank_a_w )
{
	m_sndbank_a = data;
	m_sound_addr_a = (m_sound_addr_a & 0xffff) | ((data & 7) << 16);

	if (!BIT(data, 6))
		m_sound_addr_a |= (1<<19);
	else if (!BIT(data, 5))
		m_sound_addr_a |= (2<<19);
	else if (BIT(data, 7))
		m_sndbank_a = 0xff;

	update_sound_a();
}

WRITE8_MEMBER( spinb_state::sndbank_m_w )
{
	m_sndbank_m = data;
	m_sound_addr_m = (m_sound_addr_m & 0xffff) | ((data & 7) << 16);

	if (!BIT(data, 6))
		m_sound_addr_m |= (1<<19);
	else if (!BIT(data, 5))
		m_sound_addr_m |= (2<<19);
	else if (BIT(data, 7))
		m_sndbank_m = 0xff;

	update_sound_m();
}

void spinb_state::update_sound_a()
{
	if (m_sndbank_a != 0xff)
		m_ic14a->ba_w(m_p_audio[m_sound_addr_a]);
	else
		m_ic14a->ba_w(0);
}

void spinb_state::update_sound_m()
{
	if (m_sndbank_m != 0xff)
		m_ic14m->ba_w(m_p_music[m_sound_addr_m]);
	else
		m_ic14m->ba_w(0);
}

WRITE_LINE_MEMBER( spinb_state::ic5a_w )
{
	m_pc0a = state;
	m_ic5a->d_w(state);
	m_ic14a->select_w(state);
}

WRITE_LINE_MEMBER( spinb_state::ic5m_w )
{
	m_pc0m = state;
	m_ic5m->d_w(state);
	m_ic14m->select_w(state);
}

READ8_MEMBER( spinb_state::ppia_c_r )
{
	return (m_pc0a ? 1 : 0) | m_portc_a;
}

READ8_MEMBER( spinb_state::ppim_c_r )
{
	return (m_pc0m ? 1 : 0) | m_portc_m;
}

WRITE8_MEMBER( spinb_state::ppia_b_w )
{
	m_sound_addr_a = (m_sound_addr_a & 0xffff00) | data;
	update_sound_a();
}

WRITE8_MEMBER( spinb_state::ppim_b_w )
{
	m_sound_addr_m = (m_sound_addr_m & 0xffff00) | data;
	update_sound_m();
}

WRITE8_MEMBER( spinb_state::ppia_a_w )
{
	m_sound_addr_a = (m_sound_addr_a & 0xff00ff) | (data << 8);
	update_sound_a();
}

WRITE8_MEMBER( spinb_state::ppim_a_w )
{
	m_sound_addr_m = (m_sound_addr_m & 0xff00ff) | (data << 8);
	update_sound_m();
}

WRITE8_MEMBER( spinb_state::ppia_c_w )
{
	// pc4 - READY line back to cpu board, but not used
	m_msm_a->s1_w(BIT(data, 5));
	m_msm_a->reset_w(BIT(data, 6));
	m_ic5a->clear_w(!BIT(data, 6));
	m_portc_a = data & 0xfe;
}

WRITE8_MEMBER( spinb_state::ppim_c_w )
{
	// pc4 - READY line back to cpu board, but not used
	m_msm_m->s1_w(BIT(data, 5));
	m_msm_m->reset_w(BIT(data, 6));
	m_ic5m->clear_w(!BIT(data, 6));
	m_portc_m = data & 0xfe;
}

void spinb_state::machine_reset()
{
	m_sound_addr_a = 0;
	m_sound_addr_m = 0;
	m_sndbank_a = 0xff;
	m_sndbank_m = 0xff;
	m_row = 0;
	update_sound_a();
	update_sound_m();
}

void spinb_state::machine_start()
{
	save_item(NAME(m_dmdram)); // make it visible in the debugger
}

void spinb_state::init_game0()
{
	m_p_audio = memregion("audiorom")->base();
	m_p_music = memregion("musicrom")->base();
	m_game = 0;
}

void spinb_state::init_game1()
{
	m_p_audio = memregion("audiorom")->base();
	m_p_music = memregion("musicrom")->base();
	m_p_dmdcpu = memregion("dmdcpu")->base()+0x10000;
	m_game = 1;
}

void spinb_state::init_game2()
{
	m_p_audio = memregion("audiorom")->base();
	m_p_music = memregion("musicrom")->base();
	m_p_dmdcpu = memregion("dmdcpu")->base()+0x10000;
	m_game = 2;
}

void  spinb_state::spinb_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(1, rgb_t(0xf7, 0xaa, 0x00));
	palette.set_pen_color(2, rgb_t(0x7c, 0x55, 0x00));
}

uint32_t spinb_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint16_t sy=0,ma;
	address_space &internal = m_dmdcpu->space(AS_DATA);
	ma = internal.read_byte(0x05) << 8; // find where display memory is

	if (m_game)
	{
		ma = ((ma - 0x200) & 0x1c00) + 0x200;
		if (ma > 0x1c00) return 1; // not initialised yet

		for (uint8_t y = 0; y < 32; y++)
		{
			uint16_t *p = &bitmap.pix16(sy++);
			for (uint16_t x = 0; x < 16; x++)
			{
				uint8_t const gfx = m_dmdram[ma+0x200];
				uint8_t const gfx1 = m_dmdram[ma++];

				*p++ = BIT(gfx1, 0) ? 1 : BIT(gfx, 0) ? 2 : 0;
				*p++ = BIT(gfx1, 1) ? 1 : BIT(gfx, 1) ? 2 : 0;
				*p++ = BIT(gfx1, 2) ? 1 : BIT(gfx, 2) ? 2 : 0;
				*p++ = BIT(gfx1, 3) ? 1 : BIT(gfx, 3) ? 2 : 0;
				*p++ = BIT(gfx1, 4) ? 1 : BIT(gfx, 4) ? 2 : 0;
				*p++ = BIT(gfx1, 5) ? 1 : BIT(gfx, 5) ? 2 : 0;
				*p++ = BIT(gfx1, 6) ? 1 : BIT(gfx, 6) ? 2 : 0;
				*p++ = BIT(gfx1, 7) ? 1 : BIT(gfx, 7) ? 2 : 0;
			}
		}
	}
	else
	{
		ma &= 0x1e00;

		for (uint8_t y = 0; y < 32; y++)
		{
			uint16_t *p = &bitmap.pix16(sy++);
			for (uint16_t x = 0; x < 16; x++)
			{
				uint8_t const gfx = m_dmdram[ma++];

				*p++ = BIT(gfx, 0);
				*p++ = BIT(gfx, 1);
				*p++ = BIT(gfx, 2);
				*p++ = BIT(gfx, 3);
				*p++ = BIT(gfx, 4);
				*p++ = BIT(gfx, 5);
				*p++ = BIT(gfx, 6);
				*p++ = BIT(gfx, 7);
			}
		}
	}
	return 0;
}

void spinb_state::spinb(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(5'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &spinb_state::spinb_map);
	m_maincpu->set_periodic_int(FUNC(spinb_state::irq0_line_hold), attotime::from_hz(160)); // NE556 adjustable (if faster, then jolypark has a stack problem)

	Z80(config, m_audiocpu, XTAL(5'000'000) / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &spinb_state::spinb_audio_map);

	Z80(config, m_musiccpu, XTAL(5'000'000) / 2);
	m_musiccpu->set_addrmap(AS_PROGRAM, &spinb_state::spinb_music_map);

	I8031(config, m_dmdcpu, XTAL(16'000'000));
	m_dmdcpu->set_addrmap(AS_PROGRAM, &spinb_state::dmd_mem);
	m_dmdcpu->set_addrmap(AS_IO, &spinb_state::dmd_io);
	m_dmdcpu->port_out_cb<1>().set(FUNC(spinb_state::p1_w));
	m_dmdcpu->port_in_cb<3>().set(FUNC(spinb_state::p3_r));
	m_dmdcpu->port_out_cb<3>().set(FUNC(spinb_state::p3_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	/* Video */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(spinb_state::screen_update));
	screen.set_size(128, 32);
	screen.set_visarea(0, 127, 0, 31);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(spinb_state::spinb_palette), 3);

	/* Sound */
	genpin_audio(config);

	SPEAKER(config, "msmavol").front_center();
	MSM5205(config, m_msm_a, XTAL(384'000));
	m_msm_a->vck_callback().set("ic5a", FUNC(ttl7474_device::clock_w));
	m_msm_a->set_prescaler_selector(msm5205_device::S48_4B); /* 4KHz 4-bit */
	m_msm_a->add_route(ALL_OUTPUTS, "msmavol", 1.0);

	SPEAKER(config, "msmmvol").front_center();
	MSM5205(config, m_msm_m, XTAL(384'000));
	m_msm_m->vck_callback().set("ic5m", FUNC(ttl7474_device::clock_w));
	m_msm_m->set_prescaler_selector(msm5205_device::S48_4B); /* 4KHz 4-bit */
	m_msm_m->add_route(ALL_OUTPUTS, "msmmvol", 1.0);

	/* Devices */
	i8255_device &ppi60(I8255A(config, "ppi60"));
	//ppi60.in_pa_callback().set(FUNC(spinb_state::ppi60a_r));
	ppi60.out_pa_callback().set(FUNC(spinb_state::ppi60a_w));
	//ppi60.in_pb_callback().set(FUNC(spinb_state::ppi60b_r));
	ppi60.out_pb_callback().set(FUNC(spinb_state::ppi60b_w));
	ppi60.in_pc_callback().set(FUNC(spinb_state::sw_r));
	//ppi60.out_pc_callback().set(FUNC(spinb_state::ppi60c_w));

	i8255_device &ppi64(I8255A(config, "ppi64"));
	//ppi64.in_pa_callback().set(FUNC(spinb_state::ppi64a_r));
	//ppi64.out_pa_callback().set(FUNC(spinb_state::ppi64a_w));
	//ppi64.in_pb_callback().set(FUNC(spinb_state::ppi64b_r));
	//ppi64.out_pb_callback().set(FUNC(spinb_state::ppi64b_w));
	//ppi64.in_pc_callback().set(FUNC(spinb_state::ppi64c_r));
	ppi64.out_pc_callback().set(FUNC(spinb_state::ppi64c_w));

	I8255A(config, "ppi68");
	//ppi68.in_pa_callback().set(FUNC(spinb_state::ppi68a_r));
	//ppi68.out_pa_callback().set(FUNC(spinb_state::ppi68a_w));
	//ppi68.in_pb_callback().set(FUNC(spinb_state::ppi68b_r));
	//ppi68.out_pb_callback().set(FUNC(spinb_state::ppi68b_w));
	//ppi68.in_pc_callback().set(FUNC(spinb_state::ppi68c_r));
	//ppi68.out_pc_callback().set(FUNC(spinb_state::ppi68c_w));

	I8255A(config, "ppi6c");
	//ppi6c.in_pa_callback().set(FUNC(spinb_state::ppi6ca_r));
	//ppi6c.out_pa_callback().set(FUNC(spinb_state::ppi6ca_w));
	//ppi6c.in_pb_callback().set(FUNC(spinb_state::ppi6cb_r));
	//ppi6c.out_pb_callback().set(FUNC(spinb_state::ppi6cb_w));
	//ppi6c.in_pc_callback().set(FUNC(spinb_state::ppi6cc_r));
	//ppi6c.out_pc_callback().set(FUNC(spinb_state::ppi6cc_w));

	i8255_device &ppia(I8255A(config, "ppia"));
	ppia.out_pa_callback().set(FUNC(spinb_state::ppia_a_w));
	ppia.out_pb_callback().set(FUNC(spinb_state::ppia_b_w));
	ppia.in_pc_callback().set(FUNC(spinb_state::ppia_c_r));
	ppia.out_pc_callback().set(FUNC(spinb_state::ppia_c_w));

	i8255_device &ppim(I8255A(config, "ppim"));
	ppim.out_pa_callback().set(FUNC(spinb_state::ppim_a_w));
	ppim.out_pb_callback().set(FUNC(spinb_state::ppim_b_w));
	ppim.in_pc_callback().set(FUNC(spinb_state::ppim_c_r));
	ppim.out_pc_callback().set(FUNC(spinb_state::ppim_c_w));

	TTL7474(config, m_ic5a, 0);
	m_ic5a->comp_output_cb().set(FUNC(spinb_state::ic5a_w));

	HC157(config, m_ic14a, 0); // actually IC15 on Jolly Park
	m_ic14a->out_callback().set("msm_a", FUNC(msm5205_device::data_w));

	TTL7474(config, m_ic5m, 0);
	m_ic5m->comp_output_cb().set(FUNC(spinb_state::ic5m_w));

	HC157(config, m_ic14m, 0); // actually IC15 on Jolly Park
	m_ic14m->out_callback().set("msm_m", FUNC(msm5205_device::data_w));
}

void spinb_state::jolypark(machine_config &config)
{
	spinb(config);

	MSM6585(config.replace(), m_msm_a, XTAL(640'000));
	m_msm_a->vck_callback().set("ic5a", FUNC(ttl7474_device::clock_w));
	m_msm_a->set_prescaler_selector(msm6585_device::S40);
	m_msm_a->add_route(ALL_OUTPUTS, "msmavol", 1.0);

	MSM6585(config.replace(), m_msm_m, XTAL(640'000));
	m_msm_m->vck_callback().set("ic5m", FUNC(ttl7474_device::clock_w));
	m_msm_m->set_prescaler_selector(msm6585_device::S40);
	m_msm_m->add_route(ALL_OUTPUTS, "msmmvol", 1.0);
}

void spinb_state::vrnwrld(machine_config &config)
{
	jolypark(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &spinb_state::vrnwrld_map);
}


/*-------------------------------------------------------------------
/ Bushido (1993) - ( Last game by Inder - before becoming Spinball - but same hardware)
/-------------------------------------------------------------------*/
ROM_START(bushido)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("0-z80.bin", 0x0000, 0x2000, CRC(3ea1eb1d) SHA1(cceb6c68e481f36a5646ff4f38d3dfc4275b0c79))
	ROM_LOAD("1-z80.old", 0x2000, 0x2000, CRC(648da72b) SHA1(1005a13b4746e302d979c8b1da300e943cdcab3d))

	ROM_REGION(0x10000, "dmdcpu", 0)
	ROM_LOAD("g-disply.bin", 0x00000, 0x10000, CRC(9a1df82f) SHA1(4ad6a12ae36ec898b8ac5243da6dec3abcd9dc33))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("a-sonido.bin", 0x0000, 0x2000, CRC(cf7d5399) SHA1(c79145826cfa6be2487e3add477d9b452c553762))

	ROM_REGION(0x180000, "audiorom", 0)
	ROM_LOAD("b-sonido.bin", 0x00000, 0x80000, CRC(cb4fc885) SHA1(569f389fa8f91f886b58f44f701d2752ef01f3fa))
	ROM_LOAD("c-sonido.bin", 0x80000, 0x80000, CRC(35a43dd8) SHA1(f2b1994f67f749c65a88c95d970b655990d85b96))

	ROM_REGION(0x2000, "musiccpu", 0)
	ROM_LOAD("d-musica.bin", 0x0000, 0x2000, CRC(2cb9697c) SHA1(d5c66d616ccd5e299832704e494743429dafd569))

	ROM_REGION(0x180000, "musicrom", 0)
	ROM_LOAD("e-musica.bin", 0x00000, 0x80000, CRC(1414b921) SHA1(5df9e538ee109df28953ec8f162c60cb8c6e4d96))
	ROM_LOAD("f-musica.bin", 0x80000, 0x80000, CRC(80f3a6df) SHA1(e09ad4660e511779c6e55559fa0c2c0b0c6600c8))
ROM_END

ROM_START(bushidoa)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("0-cpu.bin", 0x0000, 0x2000, CRC(7f7e6642) SHA1(6872397eed7525f384b79cdea13531d273d8cf14))
	ROM_LOAD("1-cpu.bin", 0x2000, 0x2000, CRC(a538d37f) SHA1(d2878ad0d31b4221b823812485c7faaf666ce185))

	ROM_REGION(0x10000, "dmdcpu", 0)
	ROM_LOAD("g-disply.bin", 0x00000, 0x10000, CRC(9a1df82f) SHA1(4ad6a12ae36ec898b8ac5243da6dec3abcd9dc33))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("a-sonido.bin", 0x0000, 0x2000, CRC(cf7d5399) SHA1(c79145826cfa6be2487e3add477d9b452c553762))

	ROM_REGION(0x180000, "audiorom", 0)
	ROM_LOAD("b-sonido.bin", 0x00000, 0x80000, CRC(cb4fc885) SHA1(569f389fa8f91f886b58f44f701d2752ef01f3fa))
	ROM_LOAD("c-sonido.bin", 0x80000, 0x80000, CRC(35a43dd8) SHA1(f2b1994f67f749c65a88c95d970b655990d85b96))

	ROM_REGION(0x2000, "musiccpu", 0)
	ROM_LOAD("d-musica.bin", 0x0000, 0x2000, CRC(2cb9697c) SHA1(d5c66d616ccd5e299832704e494743429dafd569))

	ROM_REGION(0x180000, "musicrom", 0)
	ROM_LOAD("e-musica.bin", 0x00000, 0x80000, CRC(1414b921) SHA1(5df9e538ee109df28953ec8f162c60cb8c6e4d96))
	ROM_LOAD("f-musica.bin", 0x80000, 0x80000, CRC(80f3a6df) SHA1(e09ad4660e511779c6e55559fa0c2c0b0c6600c8))
ROM_END

/*-------------------------------------------------------------------
/ Mach 2 (1995)
/-------------------------------------------------------------------*/
ROM_START(mach2)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("m2cpu0.19", 0x0000, 0x2000, CRC(274c8040) SHA1(6b039b79b7e08f2bf2045bc4f1cbba790c999fed))
	ROM_LOAD("m2cpu1.19", 0x2000, 0x2000, CRC(c445df0b) SHA1(1f346c1df8df0a3c4e8cb1186280d2f34959b3f8))

	ROM_REGION(0x10000, "dmdcpu", 0)
	ROM_LOAD("m2dmdf.01", 0x00000, 0x10000, CRC(c45ccc74) SHA1(8362e799a76536a16dd2d5dde500ad3db273180f))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("m2sndd.01", 0x0000, 0x2000, CRC(e789f22d) SHA1(36aa7eac1dd37a02c982d109462dddbd85a305cc))

	ROM_REGION(0x180000, "audiorom", 0)
	ROM_LOAD("m2snde.01", 0x00000, 0x80000, CRC(f5721119) SHA1(9082198e8d875b67323266c4bf8c2c378b63dfbb))

	ROM_REGION(0x2000, "musiccpu", 0)
	ROM_LOAD("m2musa.01", 0x0000, 0x2000, CRC(2d92a882) SHA1(cead22e434445e5c25414646b1e9ae2b9457439d))

	ROM_REGION(0x180000, "musicrom", 0)
	ROM_LOAD("m2musb.01", 0x00000, 0x80000, CRC(6689cd19) SHA1(430092d51704dfda8bd8264875f1c1f4461c56e5))
	ROM_LOAD("m2musc.01", 0x80000, 0x80000, CRC(88851b82) SHA1(d0c9fa391ca213a69b7c8ae7ca52063503b5656e))
ROM_END

/*-------------------------------------------------------------------
/ Jolly Park (1996)
/-------------------------------------------------------------------*/
ROM_START(jolypark)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("jpcpu0.rom", 0x0000, 0x2000, CRC(061967af) SHA1(45048e1d9f17efa3382460fd474a5aeb4191d617))
	ROM_LOAD("jpcpu1.rom", 0x2000, 0x2000, CRC(ea99202f) SHA1(e04825e73fd25f6469b3315f063f598ea1ab44c7))

	ROM_REGION(0x30000, "dmdcpu", 0)
	ROM_LOAD("jpdmd0.rom", 0x00000, 0x10000, CRC(b57565cb) SHA1(3fef66d298893029de78fdb6ecdb562c33d76180))
	ROM_LOAD("jpdmd1.rom", 0x10000, 0x20000, CRC(40d1563f) SHA1(90dbea742202340da6fa950eedc2bceec5a2af7e)) // according to schematic this rom should be twice as big

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("jpsndc1.rom", 0x0000, 0x2000, CRC(0475318f) SHA1(7154bd5ca5b28019eb0ff598ec99bbe49260932b))

	ROM_REGION(0x180000, "audiorom", 0)
	ROM_LOAD("jpsndm4.rom", 0x00000, 0x80000, CRC(735f3db7) SHA1(81dc893f5194d6ac1af54b262555a40c5c3e0292))
	ROM_LOAD("jpsndm5.rom", 0x80000, 0x80000, CRC(769374bd) SHA1(8121369714c55cc06c493b15e5c2ca79b13aff52))

	ROM_REGION(0x2000, "musiccpu", 0)
	ROM_LOAD("jpsndc0.rom", 0x0000, 0x2000, CRC(a97259dc) SHA1(58dea3f36b760112cfc32d306077da8cf6cdec5a))

	ROM_REGION(0x180000, "musicrom", 0)
	ROM_LOAD("jpsndm1.rom", 0x000000, 0x80000, CRC(fc91d2f1) SHA1(c838a0b31bbec9dbc96b46d692c8d6f1286fe46a))
	ROM_LOAD("jpsndm2.rom", 0x080000, 0x80000, CRC(fb2d1882) SHA1(fb0ef9def54d9163a46354a0df0757fac6cbd57c))
	ROM_LOAD("jpsndm3.rom", 0x100000, 0x80000, CRC(77e515ba) SHA1(17b635d107c437bfc809f8cc1a6cd063cef12691))
ROM_END

/*-------------------------------------------------------------------
/ Verne's World (1996)
/-------------------------------------------------------------------*/
ROM_START(vrnwrld)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("vwcpu0.rom", 0x0000, 0x4000, CRC(89c980e7) SHA1(09eeed0232255875cf119d59143d839ff40e30dd))
	ROM_LOAD("vwcpu1.rom", 0x4000, 0x4000, CRC(a4db4e64) SHA1(fc55781295fc723741de24ad60311b7e33551830))

	ROM_REGION(0x110000, "dmdcpu", 0)
	ROM_LOAD("vwdmd0.rom", 0x00000, 0x10000, CRC(40600060) SHA1(7ad619bcb5e5e50325360f4e946b5bfa072caead))
	ROM_LOAD("vwdmd1.rom", 0x10000, 0x80000, CRC(de4a1060) SHA1(6b848dfd8aafdbcf7e1593f98bd1c3d69306aa11))
	ROM_LOAD("vwdmd2.rom", 0x90000, 0x80000, CRC(29fc8da7) SHA1(2704f14a3338a63abda3bcbc56e9f984a679eb38))

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("vws2ic9.rom", 0x0000, 0x2000, CRC(ab8cb4c5) SHA1(92a702c11e2cef703992244529ba86079d5ab9b0))

	ROM_REGION(0x180000, "audiorom", 0)
	ROM_LOAD("vws3ic15.rom", 0x00000, 0x80000, CRC(d62c9443) SHA1(7c6b8662d88ba6592da8b83af11087647105e8dd))

	ROM_REGION(0x2000, "musiccpu", 0)
	ROM_LOAD("vws4ic30.rom", 0x0000, 0x2000, CRC(ecd18a19) SHA1(558e687e0429d31fafe8db05954d9a8ad90d6aeb))

	ROM_REGION(0x180000, "musicrom", 0)
	ROM_LOAD("vws5ic25.rom", 0x000000, 0x80000, CRC(56d349f0) SHA1(e71d2d03c3e978c552e272de8850cc265255fbd1))
	ROM_LOAD("vws6ic26.rom", 0x080000, 0x80000, CRC(bee399c1) SHA1(b2c6e4830641ed32b9643dc8c1fa08a2da5a7e9b))
	ROM_LOAD("vws7ic27.rom", 0x100000, 0x80000, CRC(7335b29c) SHA1(4de6de09f069feecbad2e5ef50032e8d381ff9b1))
ROM_END

GAME(1993, bushido,   0,       spinb,    spinb, spinb_state, init_game0, ROT0, "Inder/Spinball", "Bushido (set 1)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993, bushidoa,  bushido, spinb,    spinb, spinb_state, init_game0, ROT0, "Inder/Spinball", "Bushido (set 2)", MACHINE_IS_SKELETON_MECHANICAL)
GAME(1995, mach2,     0,       spinb,    spinb, spinb_state, init_game0, ROT0, "Spinball",       "Mach 2",          MACHINE_IS_SKELETON_MECHANICAL)
GAME(1996, jolypark,  0,       jolypark, spinb, spinb_state, init_game1, ROT0, "Spinball",       "Jolly Park",      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1996, vrnwrld,   0,       vrnwrld,  spinb, spinb_state, init_game2, ROT0, "Spinball",       "Verne's World",   MACHINE_IS_SKELETON_MECHANICAL)

// license:BSD-3-Clause
// copyright-holders:Robbbert
/******************************************************************************************

PINBALL
Atari Generation/System 1

Schematics and PinMAME used as references.

    Dipswitch vs address:
    SW1 Toggle 1 = 200B
    SW1 Toggle 2 = 200A
    SW1 Toggle 3 = 2009
    SW1 Toggle 4 = 2008
    SW1 Toggle 5 = 200F
    SW1 Toggle 6 = 200E
    SW1 Toggle 7 = 200D
    SW1 Toggle 8 = 200C
    SW2 Toggle 1 = 2003
    SW2 Toggle 2 = 2002
    SW2 Toggle 3 = 2001
    SW2 Toggle 4 = 2000
    SW2 Toggle 5 = 2007
    SW2 Toggle 6 = 2006
    SW2 Toggle 7 = 2005
    SW2 Toggle 8 = 2004

No NVRAM; coin counters are mechanical.

Outhole for each game:
- spcrider: M  (solenoid 5)
- midearth: M  (solenoid 1)
- atarians: G  (solenoid unknown)
- time2000: J  (solenoid 12)
- aavenger: X  (solenoid 4)

Status:
- All games are playable
- When switched on, display is all 8, and it goes blank after a while. This is normal,
  it's documented in the manual.

ToDo:
- Mechanical sounds
- Dips vary per game


****************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "speaker.h"

#include "atari_s1.lh"


namespace {

#define MASTER_CLK XTAL(4'000'000) / 4
#define DMA_CLK MASTER_CLK / 2
#define AUDIO_CLK DMA_CLK / 4
#define DMA_INT DMA_CLK / 128
#define NMI_INT DMA_INT / 16
//#define BIT6_CLK NMI_INT / 4

class atari_s1_state : public genpin_class
{
public:
	atari_s1_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_p_prom(*this, "proms")
		, m_maincpu(*this, "maincpu")
		, m_p_ram(*this, "ram")
		, m_dac(*this, "dac")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_player_lamps(*this, "text%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void midearth(machine_config &config);
	void atari_s1(machine_config &config);
	void atarians(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 m1080_r();
	void m1080_w(u8 data);
	u8 m1084_r();
	void m1084_w(u8 data);
	u8 m1088_r();
	void m1088_w(u8 data);
	u8 m108c_r();
	void m108c_w(u8 data);
	u8 switch_r(offs_t offset);
	void disp_w(offs_t, u8);
	void meter_w(u8 data);
	void audioen_w(u8 data);
	void audiores_w(u8 data);
	void midearth_w(offs_t offset, u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_s);
	void common_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void atarians_map(address_map &map) ATTR_COLD;
	void midearth_map(address_map &map) ATTR_COLD;

	bool m_audio_en = false;
	u8 m_timer_s[3]{};
	u8 m_vol = 0U;
	u8 m_1080 = 0U;
	u8 m_1084 = 0U;
	u8 m_1088 = 0U;
	u8 m_108c = 0U;
	u8 m_bit6 = 0U;
	u8 m_t_c = 0U;
	required_region_ptr<u8> m_p_prom;
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<u8> m_p_ram;
	required_device<dac_4bit_r2r_device> m_dac;
	required_ioport_array<10> m_io_keyboard;
	output_finder<78> m_digits;
	output_finder<8> m_player_lamps;
	output_finder<160> m_io_outputs;   // 32 solenoids + 128 lamps
};

void atari_s1_state::common_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff);
	map(0x1080, 0x1083).rw(FUNC(atari_s1_state::m1080_r), FUNC(atari_s1_state::m1080_w));
	map(0x1084, 0x1087).rw(FUNC(atari_s1_state::m1084_r), FUNC(atari_s1_state::m1084_w));
	map(0x1088, 0x108b).rw(FUNC(atari_s1_state::m1088_r), FUNC(atari_s1_state::m1088_w));
	map(0x108c, 0x108f).rw(FUNC(atari_s1_state::m108c_r), FUNC(atari_s1_state::m108c_w));
	map(0x2000, 0x204f).mirror(0x0F80).r(FUNC(atari_s1_state::switch_r)).nopw(); // aavenger ROL 200B causes a spurious write
	map(0x3000, 0x3fff).w(FUNC(atari_s1_state::audioen_w)); // audio enable
	map(0x4000, 0x4fff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x5080, 0x508f).w(FUNC(atari_s1_state::meter_w)); // time2000 only
	map(0x6000, 0x6fff).w(FUNC(atari_s1_state::audiores_w)); // audio reset
	map(0x7000, 0x7fff).rom().region("maincpu",0);
}

void atari_s1_state::mem_map(address_map &map)
{
	common_map(map);
	map(0x0000, 0x00ff).ram().share("ram").w(FUNC(atari_s1_state::disp_w));
}

void atari_s1_state::atarians_map(address_map &map)
{
	common_map(map);
	map(0x0000, 0x01ff).ram().share("ram").w(FUNC(atari_s1_state::disp_w));
}

void atari_s1_state::midearth_map(address_map &map)
{
	atarians_map(map);
	map(0x1000, 0x11ff).w(FUNC(atari_s1_state::midearth_w));
}

static INPUT_PORTS_START( atari_s1 )
	PORT_START("X0") // 2000-2007
	PORT_DIPNAME( 0xc3, 0x00, DEF_STR( Coinage ) ) // left chute; right chute
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )   // 1C_1C ; 1C_1C
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )   // 1C_2C ; 1C_2C
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )   // 1C_3C ; 1C_3C
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )   // 1C_4C ; 1C_4C
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )   // 2C_1C ; 2C_1C
	PORT_DIPSETTING(    0x82, DEF_STR( 2C_3C ) )   // 2C_3C ; 2C_3C
	PORT_DIPSETTING(    0x81, DEF_STR( 2C_5C ) )   // 2C_5C ; 2C_5C
	PORT_DIPNAME( 0x04, 0x04, "Match" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Balls" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPNAME( 0x30, 0x20, "Special" )
	PORT_DIPSETTING(    0x00, "Extra Ball" )
	PORT_DIPSETTING(    0x20, "Free Game" )
	PORT_DIPSETTING(    0x10, "20000 points" ) // same as 0x30

	PORT_START("X1") // 2008-200F
	PORT_DIPNAME( 0x01, 0x00, "Replay score" )
	PORT_DIPSETTING(    0x01, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x02, 0x02, "Last Ball double bonus" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Spelling Award" )
	PORT_DIPSETTING(    0x00, "Extra Ball" )
	PORT_DIPSETTING(    0x04, "20,000 points" )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Test")
	PORT_DIPNAME( 0x30, 0x10, "Exceed replay score" )
	PORT_DIPSETTING(    0x00, "Nothing" )
	PORT_DIPSETTING(    0x20, "Extra Ball" )
	PORT_DIPSETTING(    0x10, "Replay" ) // same as 0x30
	PORT_DIPNAME( 0xc0, 0x00, "Max Credits" )
	PORT_DIPSETTING(    0x00, "8" )
	PORT_DIPSETTING(    0x80, "12" )
	PORT_DIPSETTING(    0x40, "15" )
	PORT_DIPSETTING(    0xc0, "20" )

	PORT_START("X2") // 2010-2017
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X3") // 2018-201F
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X4") // 2020-2027
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Cabinet Tilt") // 17
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_8) PORT_NAME("Pendulum Tilt") //18
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Flipper") //19 flipper in midearth, aavenger, time2000. Normal input for other games.
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Flipper") //20 same note as above.
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP24")

	PORT_START("X5") // 2028-202F
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X6") // 2030-2037
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP35") // outhole atarians
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP38") // outhole time2000
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP40")

	PORT_START("X7") // 2038-203F
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP41") // outhole middle-earth, space-riders
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP48")

	PORT_START("X8") // 2040-2047
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP49")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP52") // outhole airborne-avenger
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP55")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP56")

	PORT_START("X9") // 2048-204F
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP57")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP58")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP59")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP60")
	PORT_DIPNAME( 0x10, 0x00, "High score (bit 0)" )
	PORT_DIPSETTING(    0x10, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x20, 0x20, "High score (bit 1)" )
	PORT_DIPSETTING(    0x20, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x40, 0x40, "High score (bit 2)" )
	PORT_DIPSETTING(    0x40, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x80, 0x00, "High score (bit 3)" )
	PORT_DIPSETTING(    0x80, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
INPUT_PORTS_END

u8 atari_s1_state::m1080_r()
{
	return m_1080 & 0xf0;
}

void atari_s1_state::m1080_w(u8 data)
{
	m_1080 = data;
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

u8 atari_s1_state::m1084_r()
{
	return m_1084 & 0xf0;
}

void atari_s1_state::m1084_w(u8 data)
{
	m_1084 = data;
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8U+i] = BIT(data, i);

	data &= 15;

	if (data != m_vol)
	{
		m_vol = data;
		float vol = m_vol/16.666+0.1;
		m_dac->set_output_gain(0, vol);
	}
}

u8 atari_s1_state::m1088_r()
{
	return m_1088 & 0xf0;
}

void atari_s1_state::m1088_w(u8 data)
{
	m_1088 = data;
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[16U+i] = BIT(data, i);
}

u8 atari_s1_state::m108c_r()
{
	return m_108c;
}

void atari_s1_state::m108c_w(u8 data)
{
	m_108c = data;
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[24U+i] = BIT(data, i);
}

void atari_s1_state::meter_w(u8 data)
{
// time2000 has optional coin counters etc
}

// midearth has a ram mirror that goes on top of the output ports
void atari_s1_state::midearth_w(offs_t offset, u8 data)
{
	m_p_ram[offset] = data;

	if (offset < 16)
		for (u8 i = 0; i < 8; i++)
			m_io_outputs[32+offset*8+i] = BIT(data, i);
	else
	if ((offset >= 0x80) && (offset <= 0x8f))
	{
		u8 t = BIT(offset, 2, 2);
		for (u8 i = 0; i < 8; i++)
			m_io_outputs[t*8+i] = BIT(data, i);

		switch (t)
		{
			case 0:
				m1080_w(data);
				break;
			case 1:
				m1084_w(data);
				break;
			case 2:
				m1088_w(data);
				break;
			case 3:
				m108c_w(data);
				break;
		}
	}
}

u8 atari_s1_state::switch_r(offs_t offset)
{
	return (BIT(m_io_keyboard[offset>>3]->read(), offset&7 ) << 7) | (BIT(m_bit6, 1) << 6) | 0x3f; // switch bit | BIT6_CLK
}

TIMER_DEVICE_CALLBACK_MEMBER( atari_s1_state::nmi )
{
	m_bit6++;
	if (m_t_c > 0x40)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	else
		m_t_c++;
}

void atari_s1_state::disp_w(offs_t offset, u8 data)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	m_p_ram[offset] = data;

	// Display
	if (offset < 32)
	{
		if ((offset < 16) && ((offset & 0x03) == 0x03))
		{
			// Player number
			m_player_lamps[offset >> 2] = !BIT(patterns[data & 0x0f], 6); // uses 'g' segment
		}
		else
		{
			// Digits
			m_digits[offset*2]   = patterns[data >> 4];
			m_digits[offset*2+1] = patterns[data & 15];
		}
	}
	else
	// Lamps
	if (offset < 48)
	{
		offset &= 15;
		for (u8 i = 0; i < 8; i++)
			m_io_outputs[32U+offset*8+i] = BIT(data, i);
	}
}

// Sound
// Presettable 74LS161 binary divider controlled by 1088:d0-3
// Then a pair of 7493 to generate 5 address lines, enabled by audiores
// The address lines are merged with 1080:d0-3 to form a lookup on the prom
// Output of prom goes to a 4-bit DAC
// Volume is controlled by 1084:d0-3
// Variables:
// m_timer_s[1] count in 74LS161
// m_timer_s[2] count in 7493s
TIMER_DEVICE_CALLBACK_MEMBER( atari_s1_state::timer_s )
{
	m_timer_s[1]++;

	if (m_timer_s[1] > 15)
	{
		m_timer_s[1] = m_1088 & 15; // set to preset value
		if (m_audio_en)
		{
			m_timer_s[2]++;
			offs_t offs = (m_timer_s[2] & 31) | ((m_1080 & 15) << 5);
			m_dac->write(m_p_prom[offs]);
		}
		else
			m_timer_s[2] = 0;
	}
}

void atari_s1_state::audioen_w(u8 data)
{
	m_audio_en = true;
}

void atari_s1_state::audiores_w(u8 data)
{
	m_audio_en = false;
}


void atari_s1_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_player_lamps.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_audio_en));
	save_item(NAME(m_timer_s));
	save_item(NAME(m_vol));
	save_item(NAME(m_1080));
	save_item(NAME(m_1084));
	save_item(NAME(m_1088));
	save_item(NAME(m_108c));
	save_item(NAME(m_bit6));
	save_item(NAME(m_t_c));
}

void atari_s1_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_vol = 0;
	m_dac->set_output_gain(0, 0);
	m_t_c = 0;
	m_audio_en = false;
	m_bit6 = 0;
}

void atari_s1_state::atari_s1(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, MASTER_CLK);
	m_maincpu->set_addrmap(AS_PROGRAM, &atari_s1_state::mem_map);

	WATCHDOG_TIMER(config, "watchdog");

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "speaker").front_center();

	DAC_4BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.3); // unknown DAC

	/* Video */
	config.set_default_layout(layout_atari_s1);

	TIMER(config, "nmi").configure_periodic(FUNC(atari_s1_state::nmi), attotime::from_hz(NMI_INT));
	TIMER(config, "timer_s").configure_periodic(FUNC(atari_s1_state::timer_s), attotime::from_hz(AUDIO_CLK));
}

void atari_s1_state::atarians(machine_config &config)
{
	atari_s1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &atari_s1_state::atarians_map);
}

void atari_s1_state::midearth(machine_config &config)
{
	atari_s1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &atari_s1_state::midearth_map);
}

/*-------------------------------------------------------------------
/ The Atarians (11/1976)
/-------------------------------------------------------------------*/
ROM_START(atarians)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("atarian.e00", 0x0000, 0x0800, CRC(6066bd63) SHA1(e993497d0ca9f056e18838494089def8bdc265c9))
	ROM_LOAD("atarian.e0",  0x0800, 0x0800, CRC(45cb0427) SHA1(e286930ca36bdd0f79acefd142d2a5431fa8005b))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("07028-01.bin", 0x0000, 0x0200, CRC(e8034b5b) SHA1(6959912c530efcc4a0c690800867fb0d1f33627f))
ROM_END

/*-------------------------------------------------------------------
/ Time 2000 (06/1977)
/-------------------------------------------------------------------*/
ROM_START(time2000)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("time.e00", 0x0000, 0x0800, CRC(e380f35c) SHA1(f2b4c508c8b7a2ce9924da97c05fb31d5115f36f))
	ROM_LOAD("time.e0",  0x0800, 0x0800, CRC(1e79c133) SHA1(54ce5d59a00334fcec8b12c077d70e3629549af0))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("07028-01.bin", 0x0000, 0x0200, CRC(e8034b5b) SHA1(6959912c530efcc4a0c690800867fb0d1f33627f))
ROM_END

/*-------------------------------------------------------------------
/ Airborne Avenger (09/1977)
/-------------------------------------------------------------------*/
ROM_START(aavenger)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("airborne.e00", 0x0000, 0x0800, CRC(05ac26b8) SHA1(114d587923ade9370d606e428af02a407d272c85))
	ROM_LOAD("airborne.e0",  0x0800, 0x0800, CRC(44e67c54) SHA1(7f94189c12e322c41908d651cf6a3b6061426959))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("20252-01.bin", 0x0000, 0x0200, CRC(3d44551d) SHA1(926100f8169ab20230ad2168f94e6ad65fb1a7dc))
ROM_END

/*-------------------------------------------------------------------
/ Middle Earth (02/1978)
/-------------------------------------------------------------------*/
ROM_START(midearth)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("609.bin", 0x0000, 0x0800, CRC(589df745) SHA1(4bd3e4f177e8d86bab41f3a14c169b936eeb480a))
	ROM_LOAD("608.bin", 0x0800, 0x0800, CRC(28b92faf) SHA1(8585770f4059049f1dcbc0c6ef5718b6ff1a5431))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("20252-01.bin", 0x0000, 0x0200, CRC(3d44551d) SHA1(926100f8169ab20230ad2168f94e6ad65fb1a7dc))
ROM_END

ROM_START(mideartha)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("20855-01.bin", 0x0000, 0x0800, CRC(4a9d47ca) SHA1(57c4458822109c3ba2fa53ac1c1cd6e169e51b24))
	ROM_LOAD("20856-01.bin", 0x0800, 0x0800, CRC(8f119e37) SHA1(5a4d63605865f3ceca4c09dbdcd888498c615b89))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("20252-01.bin", 0x0000, 0x0200, CRC(3d44551d) SHA1(926100f8169ab20230ad2168f94e6ad65fb1a7dc))
ROM_END

/*-------------------------------------------------------------------
/ Space Riders (09/1978)
/-------------------------------------------------------------------*/
ROM_START(spcrider)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("spacer.bin", 0x0000, 0x0800, CRC(3cf1cd73) SHA1(c46044fb815b439f12fb3e21c470c8b93ebdfd55))
	ROM_LOAD("spacel.bin", 0x0800, 0x0800, CRC(66ffb04e) SHA1(42d8b7fb7206b30478f631d0e947c0908dcf5419))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("20967-01.j3", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b)) // PinMAME note: nuatari lists 20967-01 (and claims that all the SR boards (5) he has feature that one), manual schematics and parts list 20252-01 though
ROM_END

} // Anonymous namespace


GAME( 1976, atarians,  0,        atarians, atari_s1, atari_s1_state, empty_init, ROT0, "Atari", "The Atarians",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1977, time2000,  0,        atari_s1, atari_s1, atari_s1_state, empty_init, ROT0, "Atari", "Time 2000",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1977, aavenger,  0,        atari_s1, atari_s1, atari_s1_state, empty_init, ROT0, "Atari", "Airborne Avenger",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978, midearth,  0,        midearth, atari_s1, atari_s1_state, empty_init, ROT0, "Atari", "Middle Earth",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978, mideartha, midearth, midearth, atari_s1, atari_s1_state, empty_init, ROT0, "Atari", "Middle Earth (alternate)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1978, spcrider,  0,        atari_s1, atari_s1, atari_s1_state, empty_init, ROT0, "Atari", "Space Riders",             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

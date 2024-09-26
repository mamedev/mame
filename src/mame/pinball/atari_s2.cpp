// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************

PINBALL
Atari Generation/System 2 and 3

System 2 : Manuals and PinMAME used as references (couldn't find full schematics).
System 3 : PinMAME used as reference (couldn't find anything else).

The only difference seems to be an extra bank of inputs (or something) at 2008-200B.

Status:
- All are playable.

ToDo:
- noise generator sounds like a loud barrrr instead of noise, fortunately it isn't used.
- roadrunr: test button not working, sets off an alarm instead. Slam Tilt?


*****************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "cpu/m6800/m6800.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/dac.h"
#include "speaker.h"

#include "atari_s2.lh"


namespace {

class atari_s2_state : public genpin_class
{
public:
	atari_s2_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_p_prom(*this, "proms")
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_dac1(*this, "dac1")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void atari_s2(machine_config &config);
	void atari_s3(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void sound0_w(u8 data);
	void sound1_w(u8 data);
	void lamp_w(offs_t, u8);
	void sol0_w(u8 data);
	void sol1_w(u8 data);
	void intack_w(u8 data);
	void display_w(offs_t offset, u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_s);

	void atari_s2_map(address_map &map) ATTR_COLD;
	void atari_s3_map(address_map &map) ATTR_COLD;

	bool m_timer_sb = false;
	u8 m_timer_s[5]{};
	u8 m_sound0 = 0U;
	u8 m_sound1 = 0U;
	u8 m_vol = 0U;
	u8 m_t_c = 0U;
	u8 m_segment[7]{};
	required_region_ptr<u8> m_p_prom;
	required_device<cpu_device> m_maincpu;
	required_device<dac_4bit_binary_weighted_device> m_dac;
	required_device<dac_3bit_binary_weighted_device> m_dac1;
	output_finder<68> m_digits;
	output_finder<80> m_io_outputs;   // 16 solenoids + 64 lamps
};

void atari_s2_state::atari_s2_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fff);
	map(0x0000, 0x00ff).mirror(0x0700).ram();
	map(0x0800, 0x08ff).mirror(0x0700).ram().share("nvram"); // battery backed
	map(0x1000, 0x1000).mirror(0x07F8).portr("X0");
	map(0x1001, 0x1001).mirror(0x07F8).portr("X1");
	map(0x1002, 0x1002).mirror(0x07F8).portr("X2");
	map(0x1003, 0x1003).mirror(0x07F8).portr("X3");
	map(0x1004, 0x1004).mirror(0x07F8).portr("X4");
	map(0x1005, 0x1005).mirror(0x07F8).portr("X5");
	map(0x1006, 0x1006).mirror(0x07F8).portr("X6");
	map(0x1007, 0x1007).mirror(0x07F8).portr("X7");
	map(0x1800, 0x1800).mirror(0x071F).w(FUNC(atari_s2_state::sound0_w));
	map(0x1820, 0x1820).mirror(0x071F).w(FUNC(atari_s2_state::sound1_w));
	map(0x1840, 0x1847).mirror(0x0718).w(FUNC(atari_s2_state::display_w));
	map(0x1860, 0x1867).mirror(0x0718).w(FUNC(atari_s2_state::lamp_w));
	map(0x1880, 0x1880).mirror(0x071F).w(FUNC(atari_s2_state::sol0_w));
	map(0x18a0, 0x18a7).mirror(0x0718).w(FUNC(atari_s2_state::sol1_w));
	map(0x18c0, 0x18c0).mirror(0x071F).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x18e0, 0x18e0).mirror(0x071F).w(FUNC(atari_s2_state::intack_w));
	map(0x2000, 0x2000).mirror(0x07FC).portr("DSW0");
	map(0x2001, 0x2001).mirror(0x07FC).portr("DSW1");
	map(0x2002, 0x2002).mirror(0x07FC).portr("DSW2");
	map(0x2003, 0x2003).mirror(0x07FC).portr("DSW3");
	map(0x2800, 0x3fff).rom().region("maincpu", 0);
}

void atari_s2_state::atari_s3_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x3fff);
	atari_s2_map(map);
	map(0x2008, 0x2008).mirror(0x07F4).portr("DSW4");
	map(0x2009, 0x2009).mirror(0x07F4).portr("DSW5");
	map(0x200a, 0x200a).mirror(0x07F4).portr("DSW6");
	map(0x200b, 0x200b).mirror(0x07F4).portr("DSW7");
}

static INPUT_PORTS_START( atari_s2 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x05, "Max Credits" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x01, "10" )
	PORT_DIPSETTING(    0x02, "15" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x04, "25" )
	PORT_DIPSETTING(    0x05, "30" )
	PORT_DIPSETTING(    0x06, "35" )
	PORT_DIPSETTING(    0x07, "40" )
	PORT_DIPNAME( 0x08, 0x00, "Balls" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x50, 0x00, "Special" )
	PORT_DIPSETTING(    0x10, "Extra Ball" )
	PORT_DIPSETTING(    0x00, "Free Game" )
	PORT_DIPSETTING(    0x40, "50000 points" )
	PORT_DIPSETTING(    0x50, "60000 points" )
	PORT_DIPNAME( 0x20, 0x00, "Free Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Match" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x10, 0x00, "Upper Lanes" )
	PORT_DIPSETTING(    0x10, "Start only" )
	PORT_DIPSETTING(    0x00, "Start and Advance" )
	PORT_DIPNAME( 0x60, 0x00, "Extra Ball reward" )
	PORT_DIPSETTING(    0x00, "Extra Ball" )
	PORT_DIPSETTING(    0x20, "20000 points" )
	PORT_DIPSETTING(    0x60, "30000 points" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x1f, 0x02, "Coinage L Chute" )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0x11, "1 coin/10 credits" )
	PORT_DIPSETTING(    0x12, "1 coin/11 credits" )
	PORT_DIPSETTING(    0x13, "1 coin/12 credits" )
	PORT_DIPSETTING(    0x14, "1 coin/13 credits" )
	PORT_DIPSETTING(    0x15, "1 coin/14 credits" )
	PORT_DIPSETTING(    0x16, "1 coin/15 credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x09, "2 coins/9 credits" )
	PORT_DIPSETTING(    0x0b, "2 coins/11 credits" )
	PORT_DIPSETTING(    0x0d, "2 coins/13 credits" )
	PORT_DIPSETTING(    0x0f, "2 coins/15 credits" )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_BIT( 0x40, 0x00, IPT_UNUSED ) // Hercules: High Score Million Limit (manual says it MUST BE ON)
	PORT_DIPNAME( 0x80, 0x00, "High Score Display" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x1f, 0x02, "Coinage R Chute" )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_8C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_9C ) )
	PORT_DIPSETTING(    0x11, "1 coin/10 credits" )
	PORT_DIPSETTING(    0x12, "1 coin/11 credits" )
	PORT_DIPSETTING(    0x13, "1 coin/12 credits" )
	PORT_DIPSETTING(    0x14, "1 coin/13 credits" )
	PORT_DIPSETTING(    0x15, "1 coin/14 credits" )
	PORT_DIPSETTING(    0x16, "1 coin/15 credits" )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x09, "2 coins/9 credits" )
	PORT_DIPSETTING(    0x0b, "2 coins/11 credits" )
	PORT_DIPSETTING(    0x0d, "2 coins/13 credits" )
	PORT_DIPSETTING(    0x0f, "2 coins/15 credits" )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPNAME( 0x20, 0x00, "Ladder Memory" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Replays for High Score" )
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x80, "3" )

	PORT_START("DSW4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW5")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW7")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X0") // 1000
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Test")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1") // 1001
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP09")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP10")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP11")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP12")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP13")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2") // 1002
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP18")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP19")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP20")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP21")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3") // 1003
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP24")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP25")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP26")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("INP27")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP28")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP29")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4") // 1004
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP32")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP33")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP34")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP35")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP36")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP37")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5") // 1005
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP40")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP41")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP42")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP43")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X6") // 1006
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt") // Superman: Slam Tilt wired in parallel with this.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP52")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP53")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X7") // 1007
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP56")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP57")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP58")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP59")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP60")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP61")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

// remove unassigned inputs that cause machine to reboot
static INPUT_PORTS_START( hercules )
	PORT_INCLUDE(atari_s2)
	PORT_MODIFY("X1")
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("X3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("X5")
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("X7")
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END
/* solenoids hercules
   4,5 = bumpers
   8,9 = slings
    15 = outhole
   6,7 = coin counters
    14 = total plays counter

   solenoids superman
   4,5,8,11 = bumpers
   9,10     = slings
        15  = outhole
       6,7  = coin counters
        12  = drop target
        13  = drop hole kicker
        14  = total plays counter
*/

void atari_s2_state::sol0_w(u8 data)
{
	switch (data)
	{
		case 15:
			m_samples->start(0, 5);
			break;
		case 4:
		case 5:
			m_samples->start(1, 0);
			break;
		case 8:
		case 9:
			//m_samples->start(1, 7);
			break;
		//default:
			//if (data) printf("%X ",data);
	}
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

void atari_s2_state::sol1_w(u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8+i] = BIT(data, i);
}

void atari_s2_state::lamp_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[16+offset*8+i] = BIT(data, i);
}

void atari_s2_state::display_w(offs_t offset, u8 data)
{
	static constexpr u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	if (offset < 7)
	{
		m_segment[offset] = patterns[data&15];
	}
	else
	{
		data &= 7;
		for (u8 i = 0; i < 7; i++)
			m_digits[i * 10 + data] = m_segment[i];
	}
}

void atari_s2_state::intack_w(u8 data)
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

// Sound
// 4 frequencies (500k,250k,125k,62.5k) come from main clock circuits
// We choose one of these with SEL A,B
// Then presettable 74LS161 binary divider controlled by m_sound0:d4-7
// Then a 74LS393 to generate 5 address lines
// The address lines are merged with m_sound0:d0-3 to form a lookup on the prom
// Output of prom goes to a 4-bit DAC
// Volume is controlled by m_sound1:d0-3
// Noise is a pair of 74LS164 shift registers, connected to form a pseudo-random pattern
// Variables:
// m_timer_s[0] inc each timer cycle, bit 0 = 500k, bit 1 = 250k, bit 2 = 125k, bit 3 = 62.5k
// m_timer_s[1] count in 74LS161
// m_timer_s[2] count in 74LS393
// m_timer_s[3] shift register of 74LS164 P4
// m_timer_s[4] shift register of 74LS164 N4
// m_timer_sb   wanted output of m_timer_s[0]
TIMER_DEVICE_CALLBACK_MEMBER( atari_s2_state::timer_s )
{
	m_timer_s[0]++;
	bool cs = BIT(m_timer_s[0], (m_sound0 & 0x30) >> 4); // select which frequency to work with by using SEL A,B
	if (cs != m_timer_sb)
	{
		m_timer_sb = cs;
		m_timer_s[1]++;
		if (m_timer_s[1] > 15)
		{
			// wave
			m_timer_s[1] = m_sound1; // set to preset value
			m_timer_s[2]++;
			offs_t offs = (m_timer_s[2] & 31) | ((m_sound0 & 15) << 5);
			if (BIT(m_sound0, 6))
				m_dac->write(m_p_prom[offs]);
			// noise
			if (BIT(m_sound0, 7))
			{
				bool ab0 = BIT(m_timer_s[3], 0) ^ BIT(m_timer_s[4], 6);
				bool ab1 = !BIT(m_timer_s[3], 1);
				m_timer_s[3] = (m_timer_s[3] << 1) | ab0;
				m_timer_s[4] = (m_timer_s[4] << 1) | ab1;
				m_dac1->write(m_timer_s[4] & 7);
			}
			else
			{
				m_timer_s[3] = 0;
				m_timer_s[4] = 0;
			}
		}
	}
}

// d0-3 = sound data to prom
// d4-5 = select initial clock frequency
// d6 h = enable wave
// d7 h = enable noise
void atari_s2_state::sound0_w(u8 data)
{
	m_sound0 = data;
	offs_t offs = (m_timer_s[2] & 31) | ((m_sound0 & 15) << 5);
	if (BIT(m_sound0, 6))
		m_dac->write(m_p_prom[offs]);
}

// d0-3 = volume
// d4-7 = preset on 74LS161
void atari_s2_state::sound1_w(u8 data)
{
	m_sound1 = data >> 4;

	data &= 15;

	if (data != m_vol)
	{
		// 4066  + r65-r68 (68k,33k,18k,8.2k)
		m_vol = data;
		float vol = m_vol/16.666+0.1;
		m_dac->set_output_gain(0, vol);
		m_dac1->set_output_gain(0, vol);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER( atari_s2_state::irq )
{
	if (m_t_c > 0x40)
		m_maincpu->set_input_line(M6800_IRQ_LINE, HOLD_LINE);
	else
		m_t_c++;
}

void atari_s2_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_timer_sb));
	save_item(NAME(m_timer_s));
	save_item(NAME(m_sound0));
	save_item(NAME(m_sound1));
	save_item(NAME(m_vol));
	save_item(NAME(m_t_c));
	save_item(NAME(m_segment));
}

void atari_s2_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_vol = 0;
	m_dac->set_output_gain(0,0);
	m_dac1->set_output_gain(0,0);
	m_sound0 = 0;
	m_sound1 = 0;
}


void atari_s2_state::atari_s2(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, XTAL(4'000'000) / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &atari_s2_state::atari_s2_map);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	WATCHDOG_TIMER(config, "watchdog");

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "speaker").front_center();

	DAC_4BIT_BINARY_WEIGHTED(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.15); // r23-r26 (68k,33k,18k,8.2k)
	DAC_3BIT_BINARY_WEIGHTED(config, m_dac1, 0).add_route(ALL_OUTPUTS, "speaker", 0.15); // r18-r20 (100k,47k,100k)

	/* Video */
	config.set_default_layout(layout_atari_s2);

	TIMER(config, "irq").configure_periodic(FUNC(atari_s2_state::irq), attotime::from_hz(XTAL(4'000'000) / 8192));
	TIMER(config, "timer_s").configure_periodic(FUNC(atari_s2_state::timer_s), attotime::from_hz(150000));
}

void atari_s2_state::atari_s3(machine_config &config)
{
	atari_s2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &atari_s2_state::atari_s3_map);
}


/*-------------------------------------------------------------------
/ Superman (03/1979)
/-------------------------------------------------------------------*/
ROM_START(supermap)
	ROM_REGION(0x1800, "maincpu", 0)
	ROM_LOAD("supmn_k.rom", 0x0000, 0x0800, CRC(a28091c2) SHA1(9f5e47db408da96a31cb2f3be0fa9fb1e79f8d85))
	ROM_LOAD("atari_m.rom", 0x0800, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_LOAD("atari_j.rom", 0x1000, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("20967-01.j3", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Hercules (05/1979)
/-------------------------------------------------------------------*/
ROM_START(hercules)
	ROM_REGION(0x1800, "maincpu", 0)
	ROM_LOAD("herc_k.rom",  0x0000, 0x0800, CRC(65e099b1) SHA1(83a06bc82e0f8f4c0655886c6a9962bb28d00c5e))
	ROM_LOAD("atari_m.rom", 0x0800, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_LOAD("atari_j.rom", 0x1000, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("20967-01.j3", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Road Runner (??/1979)
/-------------------------------------------------------------------*/
ROM_START(roadrunr)
	ROM_REGION(0x1800, "maincpu", 0)
	ROM_LOAD("0000.716",    0x0000, 0x0800, CRC(62f5f394) SHA1(ff91066d43d788119e3337788abd86e5c0bf2d92))
	ROM_LOAD("3000.716",    0x0800, 0x0800, CRC(2fc01359) SHA1(d3df20c764bb68a5316367bb18d34a03293e7fa6))
	ROM_LOAD("3800.716",    0x1000, 0x0800, CRC(77262408) SHA1(3045a732c39c96002f495f64ed752279f7d43ee7))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("20967-01.j3", 0x0000, 0x0200, BAD_DUMP CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b)) // PinMAME note: unknown so far if using the 20967-01 is correct for Road Runner, but sounds good
ROM_END

} // Anonymous namespace


GAME( 1979, supermap, 0, atari_s2, atari_s2, atari_s2_state, empty_init, ROT0, "Atari", "Superman (Pinball)", MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1979, hercules, 0, atari_s2, hercules, atari_s2_state, empty_init, ROT0, "Atari", "Hercules",           MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1979, roadrunr, 0, atari_s3, atari_s2, atari_s2_state, empty_init, ROT0, "Atari", "Road Runner",        MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE )

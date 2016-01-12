// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************

    PINBALL
    Atari Generation/System 2 and 3

    System 2 : Manuals and PinMAME used as references (couldn't find full schematics).
    System 3 : PinMAME used as reference (couldn't find anything else).

    The only difference seems to be an extra bank of inputs (or something) at 2008-200B.

ToDo:
- 4x4 not emulated yet, appears to be a different cpu and hardware.
- sounds to be verified against a real machine
- noise generator sounds like a loud barrrr instead of noise, fortunately it
  doesn't seem to be used.
- inputs, outputs, dips vary per machine
- High score isn't saved or remembered


*****************************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "sound/dac.h"
#include "atari_s2.lh"


class atari_s2_state : public genpin_class
{
public:
	atari_s2_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dac(*this, "dac")
		, m_dac1(*this, "dac1")
	{ }

	DECLARE_WRITE8_MEMBER(sound0_w);
	DECLARE_WRITE8_MEMBER(sound1_w);
	DECLARE_WRITE8_MEMBER(lamp_w) { };
	DECLARE_WRITE8_MEMBER(sol0_w);
	DECLARE_WRITE8_MEMBER(sol1_w) { };
	DECLARE_WRITE8_MEMBER(intack_w);
	DECLARE_WRITE8_MEMBER(display_w);
	TIMER_DEVICE_CALLBACK_MEMBER(irq);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_s);
private:
	bool m_timer_sb;
	UINT8 m_timer_s[5];
	UINT8 m_sound0;
	UINT8 m_sound1;
	UINT8 m_vol;
	UINT8 m_t_c;
	UINT8 m_segment[7];
	UINT8 *m_p_prom;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_device<dac_device> m_dac1;
};


static ADDRESS_MAP_START( atari_s2_map, AS_PROGRAM, 8, atari_s2_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("nvram") // battery backed
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.0")
	AM_RANGE(0x1001, 0x1001) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.1")
	AM_RANGE(0x1002, 0x1002) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.2")
	AM_RANGE(0x1003, 0x1003) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.3")
	AM_RANGE(0x1004, 0x1004) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.4")
	AM_RANGE(0x1005, 0x1005) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.5")
	AM_RANGE(0x1006, 0x1006) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.6")
	AM_RANGE(0x1007, 0x1007) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.7")
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x071F) AM_WRITE(sound0_w)
	AM_RANGE(0x1820, 0x1820) AM_MIRROR(0x071F) AM_WRITE(sound1_w)
	AM_RANGE(0x1840, 0x1847) AM_MIRROR(0x0718) AM_WRITE(display_w)
	AM_RANGE(0x1860, 0x1867) AM_MIRROR(0x0718) AM_WRITE(lamp_w)
	AM_RANGE(0x1880, 0x1880) AM_MIRROR(0x071F) AM_WRITE(sol0_w)
	AM_RANGE(0x18a0, 0x18a7) AM_MIRROR(0x0718) AM_WRITE(sol1_w)
	AM_RANGE(0x18c0, 0x18c0) AM_MIRROR(0x071F) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x18e0, 0x18e0) AM_MIRROR(0x071F) AM_WRITE(intack_w)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x07FC) AM_READ_PORT("DSW0")
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x07FC) AM_READ_PORT("DSW1")
	AM_RANGE(0x2002, 0x2002) AM_MIRROR(0x07FC) AM_READ_PORT("DSW2")
	AM_RANGE(0x2003, 0x2003) AM_MIRROR(0x07FC) AM_READ_PORT("DSW3")
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( atari_s3_map, AS_PROGRAM, 8, atari_s2_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("nvram") // battery backed
	AM_RANGE(0x1000, 0x1000) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.0")
	AM_RANGE(0x1001, 0x1001) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.1")
	AM_RANGE(0x1002, 0x1002) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.2")
	AM_RANGE(0x1003, 0x1003) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.3")
	AM_RANGE(0x1004, 0x1004) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.4")
	AM_RANGE(0x1005, 0x1005) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.5")
	AM_RANGE(0x1006, 0x1006) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.6")
	AM_RANGE(0x1007, 0x1007) AM_MIRROR(0x07F8) AM_READ_PORT("SWITCH.7")
	AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x071F) AM_WRITE(sound0_w)
	AM_RANGE(0x1820, 0x1820) AM_MIRROR(0x071F) AM_WRITE(sound1_w)
	AM_RANGE(0x1840, 0x1847) AM_MIRROR(0x0718) AM_WRITE(display_w)
	AM_RANGE(0x1860, 0x1867) AM_MIRROR(0x0718) AM_WRITE(lamp_w)
	AM_RANGE(0x1880, 0x1880) AM_MIRROR(0x071F) AM_WRITE(sol0_w)
	AM_RANGE(0x18a0, 0x18a7) AM_MIRROR(0x0718) AM_WRITE(sol1_w)
	AM_RANGE(0x18c0, 0x18c0) AM_MIRROR(0x071F) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x18e0, 0x18e0) AM_MIRROR(0x071F) AM_WRITE(intack_w)
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x07F4) AM_READ_PORT("DSW0")
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x07F4) AM_READ_PORT("DSW1")
	AM_RANGE(0x2002, 0x2002) AM_MIRROR(0x07F4) AM_READ_PORT("DSW2")
	AM_RANGE(0x2003, 0x2003) AM_MIRROR(0x07F4) AM_READ_PORT("DSW3")
	AM_RANGE(0x2008, 0x2008) AM_MIRROR(0x07F4) AM_READ_PORT("DSW4")
	AM_RANGE(0x2009, 0x2009) AM_MIRROR(0x07F4) AM_READ_PORT("DSW5")
	AM_RANGE(0x200a, 0x200a) AM_MIRROR(0x07F4) AM_READ_PORT("DSW6")
	AM_RANGE(0x200b, 0x200b) AM_MIRROR(0x07F4) AM_READ_PORT("DSW7")
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END

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
	PORT_BIT( 0x40, 0x00, IPT_UNUSED )
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

	PORT_START("SWITCH.0") // 1000
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Test") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SWITCH.1") // 1001
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SWITCH.2") // 1002
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SWITCH.3") // 1003
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SWITCH.4") // 1004
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SWITCH.5") // 1005
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SWITCH.6") // 1006
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SWITCH.7") // 1007
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
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

WRITE8_MEMBER( atari_s2_state::sol0_w )
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
}

WRITE8_MEMBER( atari_s2_state::display_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	if (offset<7)
		m_segment[offset] = patterns[data&15];
	else
	{
		data &= 7;
		for (UINT8 i = 0; i < 7; i++)
			output().set_digit_value(i * 10 + data, m_segment[i]);
	}
}

WRITE8_MEMBER( atari_s2_state::intack_w )
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
			if BIT(m_sound0, 6)
				m_dac->write_unsigned8(m_p_prom[offs]<< 4);
			// noise
			if BIT(m_sound0, 7)
			{
				bool ab0 = BIT(m_timer_s[3], 0) ^ BIT(m_timer_s[4], 6);
				bool ab1 = !BIT(m_timer_s[3], 1);
				m_timer_s[3] = (m_timer_s[3] << 1) | ab0;
				m_timer_s[4] = (m_timer_s[4] << 1) | ab1;
				m_dac1->write_unsigned8((m_timer_s[4] & 7)<< 5);
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
WRITE8_MEMBER( atari_s2_state::sound0_w )
{
	m_sound0 = data;
	offs_t offs = (m_timer_s[2] & 31) | ((m_sound0 & 15) << 5);
	if BIT(m_sound0, 6)
		m_dac->write_unsigned8(m_p_prom[offs]<< 4);
}

// d0-3 = volume
// d4-7 = preset on 74LS161
WRITE8_MEMBER( atari_s2_state::sound1_w )
{
	m_sound1 = data >> 4;

	data &= 15;

	if (data != m_vol)
	{
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

void atari_s2_state::machine_reset()
{
	m_p_prom = memregion("proms")->base();
	m_vol = 0;
	m_sound0 = 0;
	m_sound1 = 0;
}


static MACHINE_CONFIG_START( atari_s2, atari_s2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_4MHz / 4)
	MCFG_CPU_PROGRAM_MAP(atari_s2_map)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("dac", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
	MCFG_SOUND_ADD("dac1", DAC, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_atari_s2)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", atari_s2_state, irq, attotime::from_hz(XTAL_4MHz / 8192))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer_s", atari_s2_state, timer_s, attotime::from_hz(150000))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( atari_s3, atari_s2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(atari_s3_map)
MACHINE_CONFIG_END


/*-------------------------------------------------------------------
/ Superman (03/1979)
/-------------------------------------------------------------------*/
ROM_START(supermap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("supmn_k.rom", 0x2800, 0x0800, CRC(a28091c2) SHA1(9f5e47db408da96a31cb2f3be0fa9fb1e79f8d85))
	ROM_LOAD("atari_m.rom", 0x3000, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_LOAD("atari_j.rom", 0x3800, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Hercules (05/1979)
/-------------------------------------------------------------------*/
ROM_START(hercules)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("herc_k.rom",  0x2800, 0x0800, CRC(65e099b1) SHA1(83a06bc82e0f8f4c0655886c6a9962bb28d00c5e))
	ROM_LOAD("atari_m.rom", 0x3000, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_LOAD("atari_j.rom", 0x3800, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Road Runner (??/1979)
/-------------------------------------------------------------------*/
ROM_START(roadrunr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("0000.716", 0x2800, 0x0800, CRC(62f5f394) SHA1(ff91066d43d788119e3337788abd86e5c0bf2d92))
	ROM_LOAD("3000.716", 0x3000, 0x0800, CRC(2fc01359) SHA1(d3df20c764bb68a5316367bb18d34a03293e7fa6))
	ROM_LOAD("3800.716", 0x3800, 0x0800, CRC(77262408) SHA1(3045a732c39c96002f495f64ed752279f7d43ee7))

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ 4x4 (10/1982)
/-------------------------------------------------------------------*/
ROM_START(fourx4)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("8000ce65.bin", 0x8000, 0x2000, CRC(27341155) SHA1(c0da1fbf64f93ab163b2ea6bfbfc7b778cea819f)) \
	ROM_LOAD("a0004c37.bin", 0xa000, 0x2000, CRC(6f93102f) SHA1(d6520987ed5805b0e6b5da5653fc7cb063e86dda)) \
	ROM_LOAD("c000a70c.bin", 0xc000, 0x2000, CRC(c31ca8d3) SHA1(53f20eff0084771dc61d19db7ddae52e4423e75e)) \
	ROM_RELOAD(0xe000, 0x2000)

	ROM_REGION(0x0200, "proms", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

GAME( 1979, supermap,  0,  atari_s2,  atari_s2, driver_device, 0,  ROT0, "Atari", "Superman (Pinball)", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND)
GAME( 1979, hercules,  0,  atari_s2,  atari_s2, driver_device, 0,  ROT0, "Atari", "Hercules", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND)
GAME( 1979, roadrunr,  0,  atari_s3,  atari_s2, driver_device, 0,  ROT0, "Atari", "Road Runner", MACHINE_MECHANICAL | MACHINE_IMPERFECT_SOUND)
GAME( 1982, fourx4,    0,  atari_s3,  atari_s2, driver_device, 0,  ROT0, "Atari", "4x4", MACHINE_IS_SKELETON_MECHANICAL)

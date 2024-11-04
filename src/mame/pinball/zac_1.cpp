// license:BSD-3-Clause
// copyright-holders:Robbbert
/*************************************************************************************

PINBALL
Zaccaria Generation 1

Setup is via a menu - there are no dipswitches.

If you see 6 and 9 flashing at start- this indicates the battery is flat,
 and a full setup is required before it can be used.

At start, the highscore will be set to a random value. Beating this score will
 award a bonus. Tilting will cause the high score to advance by 100,000.

If at any time you 'clock' the machine (ie exceed 999,990), the last digit will
 flash, indicating you have a million. All games have 6-digit displays - don't be
 fooled by the numerous videos of recreations on YouTube.

All games:
- To start, press 1
- Press X to end ball
- Ball counter runs backwards, so 1 = last ball.
- Earth Wind Fire, Locomotion:
    When on the last ball, scoring points racks up a bonus counter. At the end, you get
     an extra timed part, with infinite balls until the time runs out. Then press X.

Games:
- Combat
- Winter Sports
- House of Diamonds
- Future World
- Shooting the Rapids
- Hot Wheels
- Fire Mountain
- Star God
- Space Shuttle
- Earth Wind Fire
- Locomotion
**** Other Manufacturer ****
- Horror (conversion of Space Shuttle)

Sound - there's 5 sound boards
 1. 4 audio oscillators which can be switched on and off independently. It's shown
    on the schematic of all machines, but only works for (hod, wsports, and futurwld).
 2. 8035 CPU and a 1408 DAC. (Earth Wind Fire, Space Shuttle)
    This board is quite similar to that in quasar. It's possible that the input lines
    need swapping around as in quasar. Earth Wind Fire seems fine as is, but Space Shuttle
    may need a swap.
 3. NE555 and SN76477 with switchable sounds (achieved with 21 switching diodes
    and 8 data bits). (Fire Mountain, HotWheels, Strapids, Stargod)
 4. Stargoda uses something else entirely. No schematics.
 5. Locomotion uses the 8035 CPU, and a 76477 and NE555 on a unique board.

Each game has its own map of inputs and outputs, although fortunately some
 of them happen to be fairly common. For example the outhole is always on the
 same output line, while the knocker is the same except for 'strapids'.


Status:
- Games are working, incomplete sound.

ToDo:
- Monotone oscillator frequencies are guesses. We assume that only one tone at a time is used.
- hotwheel,strapids,stargod,stargodb,firemntn: NE555 to add
- stargod: Sound is totally bad (stargodb is ok)
- stargoda: Different unknown sound card
- No machines sound like the real ones
- Inbuilt Printer

**************************************************************************************/


#include "emu.h"
#include "cpu/s2650/s2650.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/clock.h"
#include "machine/gen_latch.h"
#include "genpin.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/mm5837.h"
#include "sound/sn76477.h"
#include "sound/spkrdev.h"
#include "speaker.h"
#include "zac_1.lh"

namespace {

class zac_1_state : public genpin_class
{
public:
	zac_1_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_speaker(*this, "speaker")
		, m_p_ram(*this, "ram")
		, m_monotone(*this, "monotone")
		, m_astable(*this, "astable")
		, m_dac(*this, "dac")
		, m_soundlatch(*this, "soundlatch")
		, m_sn(*this, "sn")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
		{ }

	void locomotp(machine_config &config);
	void config_base(machine_config &config);
	void zac1(machine_config &config);
	void zac2(machine_config &config);
	void zac3(machine_config &config);
	void zac4(machine_config &config);

private:
	u8 ctrl_r();
	void ctrl_w(u8 data);
	int audio_t1_r();
	int serial_r();
	void serial_w(int state);
	void noise_w(int state);
	void clock_w(int state);
	u8 reset_int_r();
	void reset_int_w(u8 data);
	TIMER_DEVICE_CALLBACK_MEMBER(zac_1_inttimer);
	TIMER_DEVICE_CALLBACK_MEMBER(zac_1_outtimer);
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	void audio_command_w(u8 data);
	u8 audio_command_r();

	void locomotp_data(address_map &map) ATTR_COLD;
	void locomotp_io(address_map &map) ATTR_COLD;
	void locomotp_map(address_map &map) ATTR_COLD;
	void zac_1_data(address_map &map) ATTR_COLD;
	void zac_1_io(address_map &map) ATTR_COLD;
	void zac_1_map(address_map &map) ATTR_COLD;
	void audio_data(address_map &map) ATTR_COLD;
	void audio_io(address_map &map) ATTR_COLD;

	u8 m_t_c = 0U;
	u8 m_out_offs = 0U;
	u8 m_input_line = 0U;
	u8 m_sn_store = 0xffU;
	u8 m_40193 = 0U;
	bool m_clock_state = false;
	bool m_noise_state = false;

	required_device<s2650_device> m_maincpu;
	optional_device<i8035_device> m_audiocpu;
	optional_device<speaker_sound_device> m_speaker;
	required_shared_ptr<u8> m_p_ram;
	optional_device<clock_device> m_monotone;
	optional_device<clock_device> m_astable;
	optional_device<dac_byte_interface> m_dac;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<sn76477_device> m_sn;
	output_finder<78> m_digits;
	output_finder<192> m_io_outputs;
};


void zac_1_state::zac_1_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x13ff).rom();
	map(0x1400, 0x17ff).w(FUNC(zac_1_state::reset_int_w));
	map(0x1800, 0x18ff).mirror(0x300).ram().share("ram");
	map(0x1c00, 0x1fff).rom();
}

void zac_1_state::zac_1_io(address_map &map)
{
	map.unmap_value_high();
}

void zac_1_state::zac_1_data(address_map &map)
{
	map.unmap_value_high();
	map(S2650_CTRL_PORT, S2650_CTRL_PORT).rw(FUNC(zac_1_state::ctrl_r), FUNC(zac_1_state::ctrl_w));
}

void zac_1_state::audio_data(address_map &map)
{
	map(0x0000, 0x07ff).rom();  // 2716
}

void zac_1_state::audio_io(address_map &map)
{
	map(0x00, 0x7f).ram();  // 6810
	map(0x80, 0xff).r(FUNC(zac_1_state::audio_command_r)); // 40097
}

static INPUT_PORTS_START( common )
	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("CPU Test") PORT_CODE(KEYCODE_3_PAD) // doesn't seem to do anything

	PORT_START("DSW2")
	PORT_DIPNAME( 0x10, 0x10, "Sound Test" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Test") PORT_CODE(KEYCODE_0_PAD)  // this performs tests
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Tilt") PORT_CODE(KEYCODE_9)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Slam") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Printer")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RAM Reset") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x3e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD )  // Another tilt
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Burn Test") PORT_CODE(KEYCODE_2_PAD)
INPUT_PORTS_END

static INPUT_PORTS_START( firemntn )
	PORT_INCLUDE( common )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Flap") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Flap") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bottom Inside RH Canal") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bottom Outside RH Canal") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bottom Outside LH Canal") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bottom Inside LH Canal") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Top Hole") PORT_CODE(KEYCODE_G)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Bumper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Top Canal") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Top Centre Canal") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Top Canal") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Top Fixed Target") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Top Fixed Target") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Bumper") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Top Hole") PORT_CODE(KEYCODE_O)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bottom Bumper") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Top Contact") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Top Contact") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Spinning Target") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Lateral Outside Contacts") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Spinning Target") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bank Contacts") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Moving Target") PORT_CODE(KEYCODE_W)

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Bank Target 1") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Bank Target 2") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Bank Target 3") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LH Bank Target 4") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Bank Target 1") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Bank Target 2") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Bank Target 3") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("RH Bank Target 4") PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END

static INPUT_PORTS_START( zac_1 )
	PORT_INCLUDE( common )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Outhole") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP17") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP18") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP19") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP20") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP21") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP22") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP23") PORT_CODE(KEYCODE_G)

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP24") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP25") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP26") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP27") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP28") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP29") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP30") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP31") PORT_CODE(KEYCODE_O)

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP32") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP33") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP34") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP35") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP36") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP37") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP38") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP39") PORT_CODE(KEYCODE_W)

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP40") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP41") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP42") PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP43") PORT_CODE(KEYCODE_STOP)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP44") PORT_CODE(KEYCODE_SLASH)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP45") PORT_CODE(KEYCODE_COLON)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP46") PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP47") PORT_CODE(KEYCODE_ENTER)
INPUT_PORTS_END

void zac_1_state::audio_command_w(u8 data)
{
	static const double res[8] { RES_INF, 4700, 2000, 820, 680, 560, 470, 330 }; // locomotp slf resistors

	if (m_soundlatch)
		m_soundlatch->write(data);

	if (m_sn)
	{
		//if (m_p_ram[0x86]==1 || m_p_ram[0xb2]==1) // check for game over or tilt
		//{
			//m_sn->enable_w(1);
			//return;
		//}

		if (data == m_sn_store)
			return;
		m_sn_store = data;

		// Locomotion
		if (m_astable)
		{
			// check for the steam whistle
			if (data == 3)
				m_astable->set_unscaled_clock(1277);
			else
			{
				m_astable->set_unscaled_clock(0);
				m_clock_state = 0;
			}

			// 4028 (U12) choose an action upon the 40193 (increment, decrement, reset)
			switch (data)
			{
				case 7: // schematic says 0 in error
					m_40193 = 0;
					break;
				case 1:
					if (m_40193 < 7)
						m_40193++;
					break;
				case 2:
					if (m_40193 > 1)
						m_40193--;
					break;
				default: // other numbers are for electronic sounds
					break;
			}

			if (m_40193)
			{
				// select locomotive speed (jiggle B and V to adjust throttle)
				m_sn->envelope_1_w(0);
				m_sn->envelope_2_w(1);
				m_sn->mixer_b_w(0);
				m_sn->mixer_c_w(1);
				m_sn->slf_res_w(RES_K(res[m_40193 & 7]));
				m_sn->enable_w(0);
			}
			else
			{
				// constant hiss of steam
				m_sn->envelope_1_w(1);
				m_sn->envelope_2_w(0);
				m_sn->mixer_b_w(1);
				m_sn->mixer_c_w(0);
				m_sn->slf_res_w(RES_INF);  // pin is disconnected
				m_sn->enable_w(1);
			}
		}
		else
		{
			// Hotwheels, Stargod, Strapids, Firemntn
			if (!BIT(data, 0))
				m_sn->enable_w(1);
			m_sn->vco_w(1);
			m_sn->mixer_b_w(0);
			m_sn->decay_res_w(RES_K(1500));
			m_sn->vco_res_w(RES_K(100));
			m_sn->slf_res_w(RES_M(1));
			m_sn->one_shot_res_w(RES_K(1500));
			m_sn->set_pitch_voltage(2.5); // wrong, this is varied by a waveform from the 555.
			// These extra resistors are in parallel with the above ones, the resultant value is used.
			switch (data >> 1)
			{
				case 0:
					m_sn->decay_res_w(RES_K(468));  // D22
					m_sn->one_shot_res_w(9933);  // D7
					break;
				case 1:
					m_sn->decay_res_w(RES_K(4.7)); // D17
					m_sn->vco_res_w(31972);  // D11
					m_sn->one_shot_res_w(RES_K(136));  // D4
					break;
				case 2:
					m_sn->decay_res_w(RES_K(468));  // D23
					m_sn->vco_res_w(24812);   // D13
					m_sn->one_shot_res_w(9933);  // D6
					break;
				case 3:
					m_sn->decay_res_w(RES_K(468)); // D21
					m_sn->vco_res_w(31972);  // D12
					m_sn->one_shot_res_w(9933);  // D8
					break;
				case 4:
					m_sn->vco_w(0);   // D24 (and also enable NE555)
					m_sn->decay_res_w(RES_K(27));  // D18
					m_sn->vco_res_w(4489);  // D16
					break;
				case 5:
					m_sn->vco_w(0);   // D25 (and also enable NE555)
					m_sn->decay_res_w(RES_K(27));  // D20
					m_sn->vco_res_w(4489);  // D15
					m_sn->one_shot_res_w(9933);  // D5
					break;
				case 6:
					m_sn->mixer_b_w(1);
					m_sn->one_shot_res_w(RES_K(358));  // D3
					break;
				case 7:
					m_sn->decay_res_w(RES_K(192));  // D19
					m_sn->vco_res_w(9090);   // D14
					m_sn->slf_res_w(31945);  // D10
					m_sn->one_shot_res_w(93750);  // D9
					break;
			}
			if (BIT(data, 0))
				m_sn->enable_w(0);
		}
	}
}

// Locomotion: The noise and tone represent a steam whistle
void zac_1_state::noise_w(int state)
{
	m_noise_state = state;
	m_speaker->level_w((m_clock_state && m_noise_state) ? 1 : 0);
}

void zac_1_state::clock_w(int state)
{
	m_clock_state = state;
	m_speaker->level_w((m_clock_state && m_noise_state) ? 1 : 0);
}

u8 zac_1_state::audio_command_r()
{
	return m_soundlatch->read() | (ioport("DSW2")->read() & 0x30);
}

int zac_1_state::audio_t1_r()
{
	return (m_soundlatch->read() == 0);
}

u8 zac_1_state::ctrl_r()
{
// reads inputs
	if (m_input_line == 0xfe)
		return ioport("X0")->read();
	else
	if (m_input_line == 0xfd)
		return ioport("X1")->read();
	else
	if (m_input_line == 0xfb)
		return ioport("X2")->read();
	else
	if (m_input_line == 0xf7)
		return ioport("X3")->read();
	else
	if (m_input_line == 0xef)
		return ioport("X4")->read();
	else
	if (m_input_line == 0xdf)
		return ioport("X5")->read();
	else
		return 0xff;
}

void zac_1_state::ctrl_w(u8 data)
{
	m_input_line = data;
}

void zac_1_state::reset_int_w(u8 data)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

int zac_1_state::serial_r()
{
// from printer
	return 0;
}

void zac_1_state::serial_w(int state)
{
// to printer
}

void zac_1_state::machine_reset()
{
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_t_c = 0;
	m_out_offs = 0;
	m_input_line = 0;
	m_sn_store = 0xff;
	m_clock_state = 0;
	m_noise_state = 0;
	m_40193 = 0;

	// init system if invalid
	if (m_p_ram[0xf7] == 5 || m_p_ram[0xf8] == 0x0a)
	{}
	else
	{
		m_p_ram[0xc0] = 3; // 3 balls
		for (u8 i=0xc1; i < 0xd6; i++)
			m_p_ram[i] = 1; // enable match & coin slots
		m_p_ram[0xf7] = 5;
		m_p_ram[0xf8] = 0x0a;
	}
}

void zac_1_state::machine_start()
{
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_t_c));
	save_item(NAME(m_out_offs));
	save_item(NAME(m_input_line));
	save_item(NAME(m_sn_store));
	save_item(NAME(m_clock_state));
	save_item(NAME(m_noise_state));
	save_item(NAME(m_40193));
}

TIMER_DEVICE_CALLBACK_MEMBER(zac_1_state::zac_1_inttimer)
{
	if (m_t_c > 0x40)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
	else
		m_t_c++;
}

/* scores = 1800-182D; solenoids = 1840-1853;
   lamps = 1880-18BF; bookkeeping=18C0-18FF. 4-tone osc=1854-1857.
   182E-183F is a storage area for inputs. */
TIMER_DEVICE_CALLBACK_MEMBER(zac_1_state::zac_1_outtimer)
{
	static const u8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
	m_out_offs++;
	if (m_out_offs > 0xbf)
		m_out_offs = 0;
	u8 data = m_p_ram[m_out_offs] & 15;
	m_io_outputs[m_out_offs] = data;

	// display
	if (m_out_offs < 0x2e)
	{
		u8 display = (m_out_offs >> 3) & 7;
		u8 digit = m_out_offs & 7;
		m_digits[display * 10 + digit] = patterns[data];
		if (m_out_offs == 0x16)
			audio_command_w(data);
	}
	else
	// solenoids
	if (m_out_offs >= 0x40 && m_out_offs <= 0x53)
	{
		if (m_out_offs == 0x4a) // outhole
		{
			if (BIT(data, 0))
				m_samples->start(0, 5);
		}
		else
		if (m_out_offs == 0x4b) // knocker (strapids uses 0x43)
		{
			if (BIT(data, 0))
				m_samples->start(0, 6);
		}
	}
	else
	// basic audio (always fitted but not always used)
	if (m_out_offs >= 0x54 && m_out_offs <= 0x57 && m_monotone)
	{
		switch (m_out_offs - 0x54)
		{
			case 0:
				if (data)
				{
					m_monotone->set_unscaled_clock(988);
					m_out_offs = 0x7f;
				}
				break;
			case 1:
				if (data)
				{
					m_monotone->set_unscaled_clock(750);
					m_out_offs = 0x7f;
				}
				break;
			case 2:
				if (data)
				{
					m_monotone->set_unscaled_clock(600);
					m_out_offs = 0x7f;
				}
				break;
			case 3:
				if (data)
				{
					m_monotone->set_unscaled_clock(400);
					m_out_offs = 0x7f;
				}
		}
		if (m_out_offs == 0x57)
			m_monotone->set_unscaled_clock(0);
	}
}

void zac_1_state::config_base(machine_config &config)
{
	/* basic machine hardware */
	S2650(config, m_maincpu, 6000000/2); // no xtal, just 2 chips forming a random oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &zac_1_state::zac_1_map);
	m_maincpu->set_addrmap(AS_IO, &zac_1_state::zac_1_io);
	m_maincpu->set_addrmap(AS_DATA, &zac_1_state::zac_1_data);
	m_maincpu->sense_handler().set(FUNC(zac_1_state::serial_r));
	m_maincpu->flag_handler().set(FUNC(zac_1_state::serial_w));
	m_maincpu->intack_handler().set([this]() { return (ioport("TEST")->read() ) ? 0x10 : 0x18; });

	NVRAM(config, "ram", nvram_device::DEFAULT_ALL_0);

	TIMER(config, "zac_1_inttimer").configure_periodic(FUNC(zac_1_state::zac_1_inttimer), attotime::from_hz(200));
	TIMER(config, "zac_1_outtimer").configure_periodic(FUNC(zac_1_state::zac_1_outtimer), attotime::from_hz(187500));

	/* Video */
	config.set_default_layout(layout_zac_1);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
}

void zac_1_state::zac1(machine_config &config)
{
	config_base(config);
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);
	CLOCK(config, m_monotone, 0); // basic audio
	m_monotone->signal_handler().set(m_speaker, FUNC(speaker_sound_device::level_w));
}

void zac_1_state::zac2(machine_config &config)
{
	config_base(config);
	// sound card with 8035 and 1408
	I8035(config, m_audiocpu, XTAL(6'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &zac_1_state::audio_data);
	m_audiocpu->set_addrmap(AS_IO, &zac_1_state::audio_io);
	m_audiocpu->t1_in_cb().set(FUNC(zac_1_state::audio_t1_r));
	m_audiocpu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	MC1408(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.275);
	GENERIC_LATCH_8(config, m_soundlatch);
}

void zac_1_state::zac3(machine_config &config)
{
	config_base(config);
	// sound card with 76477
	SN76477(config, m_sn);
	m_sn->set_noise_params(RES_K(220), RES_K(4.7), CAP_P(2200));
	m_sn->set_decay_res(RES_K(1500));
	m_sn->set_attack_params(CAP_N(2200), RES_K(4.7));
	m_sn->set_amp_res(RES_K(47));
	m_sn->set_feedback_res(RES_K(220));
	m_sn->set_vco_params(5.0, CAP_U(0.33), RES_K(100));
	m_sn->set_pitch_voltage(5.0);
	m_sn->set_slf_params(CAP_U(2.2), RES_M(1));
	m_sn->set_oneshot_params(CAP_U(2.2), RES_K(1500));
	m_sn->set_vco_mode(1);
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->set_envelope_params(1, 0);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.30);
	// add NE555 here - it provides external vco control,
	// with 2 fixed frequencies and an auto-sliding frequency
}

void zac_1_state::zac4(machine_config &config)
{
	config_base(config);
	// stargoda sound card - could be a motorola cpu
}

/*************************** LOCOMOTION ********************************/

void zac_1_state::locomotp_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x17ff).rom();
	map(0x1800, 0x18ff).mirror(0x300).ram().share("ram");
	map(0x1c00, 0x1fff).rom();
}

void zac_1_state::locomotp_io(address_map &map)
{
	map.unmap_value_high();
}

void zac_1_state::locomotp_data(address_map &map)
{
	map(S2650_CTRL_PORT, S2650_CTRL_PORT).rw(FUNC(zac_1_state::ctrl_r), FUNC(zac_1_state::ctrl_w));
	map(S2650_DATA_PORT, S2650_DATA_PORT).r(FUNC(zac_1_state::reset_int_r));
}

u8 zac_1_state::reset_int_r()
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	return 0;
}

void zac_1_state::locomotp(machine_config &config)
{
	zac2(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &zac_1_state::locomotp_map);
	m_maincpu->set_addrmap(AS_IO, &zac_1_state::locomotp_io);
	m_maincpu->set_addrmap(AS_DATA, &zac_1_state::locomotp_data);

	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	// SN76477 sound effects
	SN76477(config, m_sn);
	m_sn->set_noise_params(RES_K(39), RES_K(100), CAP_P(470));
	m_sn->set_decay_res(RES_M(1));
	m_sn->set_attack_params(CAP_U(22), RES_K(4.7));
	m_sn->set_amp_res(RES_K(47));
	m_sn->set_feedback_res(RES_K(68));
	m_sn->set_slf_params(CAP_N(220), RES_K(560));
	m_sn->set_oneshot_params(CAP_U(1), RES_K(330));
	m_sn->set_mixer_params(0, 0, 0);
	m_sn->add_route(ALL_OUTPUTS, "mono", 0.125);

	// MM5837 noise device
	mm5837_device &noise(MM5837(config, "noise"));
	noise.set_vdd(-12);   // relative to vss, should it be relative to ground?
	noise.output_callback().set(FUNC(zac_1_state::noise_w));

	// 555 timer, 1277 Hz, merges with noise to make a steam whistle
	CLOCK(config, m_astable, 0);
	m_astable->signal_handler().set(FUNC(zac_1_state::clock_w));
}

/* SYSTEM-1 ALTERNATE ROMS =======================================================================

This is a list of known alternate roms.

Hot Wheels (alternatives for rom3)
ROM_LOAD( "hw04-3-425i.bin", 0x0800, 0x0400, CRC(ef0dcd76) SHA1(2250ecb883534df394466bdae96cef1ab7adf190) )
ROM_LOAD( "hw04-3-429a.bin", 0x0800, 0x0400, CRC(997daff6) SHA1(c0889f1c48e72cdf4b10548442002b31499d4123) )
ROM_LOAD( "hw04-3-5245.bin", 0x0800, 0x0400, CRC(2067112a) SHA1(8a6c21c6d0fff97b3577f0334d0f5e45a1f076c8) )

Locomotion (tested, not working)
ROM_LOAD( "loc-2.fil",  0x1c00, 0x0800, CRC(2ca902ac) SHA1(39d2728194933527f2aa4b2f5c2b2695b31bbedf) )
ROM_LOAD( "loc-4.fil",  0x0c00, 0x0800, CRC(c370a033) SHA1(e21c008662d7253d0eabf68832f93eb458999748) )
ROM_LOAD( "loc-5.fil",  0x1000, 0x0800, CRC(ba1f3e71) SHA1(f691a9b50295a1ec60c85c820c90d4af629ebc9c) )

Space Shuttle
ROM_LOAD( "campioneflash1-2-3.bin", 0x0000, 0x0800, CRC(61894206) SHA1(f78724b416c27c26990ad28c1c4f5376353be55b) )
ROM_LOAD( "flash17.bin",  0x0000, 0x0800, CRC(5049326d) SHA1(3b2f4ea054962bf4ba41d46663b7d3d9a77590ef) )
ROM_LOAD( "spaceshuttle3.bin", 0x0800, 0x0400, CRC(c6a95dfc) SHA1(135e65264455da41c35a68378227b1b84517f98c) )

Star God (tested, not working)
ROM_LOAD( "stargod5.lgc", 0x1000, 0x0400, CRC(03cd4e24) SHA1(b73d383dc71e44277de9116a702b899a54ce32b9) )
ROM_LOAD( "stargod.snd",  0x7800, 0x0800, CRC(5079e493) SHA1(51d366cdd09ad00b8b016b0ea1c85ac95ef94d71) )

*/

/*--------------------------------
/ Earth Wind Fire (04/81)
/-------------------------------*/
ROM_START(ewf)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "zac_boot.u1", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "ewf_2.u2",    0x1c00, 0x0400, CRC(aa67e0b4) SHA1(4491eff7081fd5e397974fac1156992ce2012d0b))
	ROM_LOAD ( "ewf_3.u3",    0x0800, 0x0400, CRC(b21bf015) SHA1(ecddfe1d6797c39e094a7f86efabf0abea0fa4af))
	ROM_LOAD ( "ewf_4.u4",    0x0c00, 0x0400, CRC(d110da3f) SHA1(88e27347d209fab5be924f95b0a001476ea92c1f))
	ROM_LOAD ( "ewf_5.u5",    0x1000, 0x0400, CRC(f695dab6) SHA1(48ca60718cea40baa5052f690c8d69eb7ab32b0e))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("ewf.snd",       0x0000, 0x0800, CRC(5079e493) SHA1(51d366cdd09ad00b8b016b0ea1c85ac95ef94d71))
ROM_END

/*--------------------------------
/ Fire Mountain (01/80)
/-------------------------------*/
ROM_START(firemntn)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "zac_boot.u1", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "firemt_2.u2", 0x1c00, 0x0400, CRC(d146253f) SHA1(69910ddd1b7f1a0a0db689e750a0288d10e92951))
	ROM_LOAD ( "firemt_3.u3", 0x0800, 0x0400, CRC(d9faae07) SHA1(9883be01e2d359a111528029407141c9792c3583))
	ROM_LOAD ( "firemt_4.u4", 0x0c00, 0x0400, CRC(b5cac3da) SHA1(94f1153571a099574d041a5168854056a692a03d))
	ROM_LOAD ( "firemt_5.u5", 0x1000, 0x0400, CRC(13f11d84) SHA1(031f43467a4a01810297e3bfe0762ed2eed4e251))
ROM_END

/*--------------------------------
/ Future World (10/78)
/-------------------------------*/
ROM_START(futurwld)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "futwld_1.u1", 0x0000, 0x0400, CRC(d83b8793) SHA1(3bb04d8395191ecf324b6da0bcddcf7bd8d41867))
	ROM_LOAD ( "futwld_2.u2", 0x0400, 0x0400, CRC(bdcb7e1d) SHA1(e6c0c7e8188df87937f0b22dbb0639872e03e948))
	ROM_LOAD ( "futwld_3.u3", 0x0800, 0x0400, CRC(48e3d293) SHA1(0029f30c4a94067e7782e22499b11db86f051934))
	ROM_LOAD ( "futwld_4.u4", 0x0c00, 0x0400, CRC(b1de2120) SHA1(970e1c4eadb7ace1398684accac289a434d13d84))
	ROM_LOAD ( "futwld_5.u5", 0x1000, 0x0400, CRC(6b7965f2) SHA1(31314bc63f01717004c5c2448b5db7d292145b60))
ROM_END

/*--------------------------------
/ Horror
/-------------------------------*/
// Conversion kit for Space Shuttle. One ROM is new.

/*--------------------------------
/ Hot Wheels (09/79)
/-------------------------------*/
ROM_START(hotwheel)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "zac_boot.u1", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "htwhls_2.u2", 0x1c00, 0x0400, CRC(7ff870ae) SHA1(274ee7c2cb92b6710c546058e7277f06720b5e37))
	ROM_LOAD ( "htwhls_3.u3", 0x0800, 0x0400, CRC(7c1fba91) SHA1(d514e9b3128dfe7999e414fd9044dc20c0d76c66))
	ROM_LOAD ( "htwhls_4.u4", 0x0c00, 0x0400, CRC(974804ba) SHA1(f35c1b52327b2d3170a9a28dbee4d1437f1f594a))
	ROM_LOAD ( "htwhls_5.u5", 0x1000, 0x0400, CRC(e28f3c60) SHA1(eb780be60b41017d105288cef71906d15474b8fa))
ROM_END

/*--------------------------------
// House of Diamonds (07/78)
/-------------------------------*/
ROM_START(hod)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "hod_1.u1", 0x0000, 0x0400, CRC(b666af0e) SHA1(e6a96ed30733e7b011ba35d1a628cefd073f29a1))
	ROM_LOAD ( "hod_2.u2", 0x0400, 0x0400, CRC(956aac25) SHA1(2a59c3589d14e36ab2c61c6fbc9e8212410a385b))
	ROM_LOAD ( "hod_3.u3", 0x0800, 0x0400, CRC(88b05360) SHA1(44992a01eaa8f58296d6fb003da8dad528f2b937))
	ROM_LOAD ( "hod_4.u4", 0x0c00, 0x0400, CRC(25b6be1f) SHA1(351138404865d69ccb3ad450deda0776e987fdd2))
	ROM_LOAD ( "hod_5.u5", 0x1000, 0x0400, CRC(81b73c40) SHA1(21b80cff132becdb028e6ee895231da635189ef4))
ROM_END

/*--------------------------------
/ Locomotion (09/81)
/-------------------------------*/
ROM_START(locomotp)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "loc-1.u1", 0x0000, 0x0800, CRC(8d0252a2) SHA1(964dca642fb26eef2c132eca354a0ffce32e25df))
	ROM_LOAD ( "loc-2.u2", 0x1c00, 0x0400, CRC(9dbd8601) SHA1(10bc37d2691c7237a14e0718febed2aa7822db23))
	ROM_LOAD ( "loc-3.u3", 0x0800, 0x0400, CRC(8cadea7b) SHA1(e712add828dd22a2b495f0479f949748db21fbf7))
	ROM_CONTINUE(0x1400, 0x0400)
	ROM_LOAD ( "loc-4.u4", 0x0c00, 0x0400, CRC(177c89b6) SHA1(23de8208dbbf141952a974514fc752ed2eb6b202))
	ROM_LOAD ( "loc-5.u5", 0x1000, 0x0400, CRC(cad4122a) SHA1(df29914adeb9675abbd9f43dbef23adf2fe96c81))

	ROM_REGION(0x1000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("loc-snd.fil", 0x0000, 0x0800, CRC(51ea9d2a) SHA1(9a68687af2c1cad2a261f61a67a625d906c502e1))
ROM_END

/*--------------------------------
/ Shooting the Rapids (04/79)
/-------------------------------*/
ROM_START(strapids)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "rapids_1.u1", 0x0000, 0x0400, CRC(2a30cef3) SHA1(1af0ad08316fca565a6de1d308ed0495907656e7))
	ROM_LOAD ( "rapids_2.u2", 0x0400, 0x0400, CRC(04adaa14) SHA1(7819de53cee669b7e42624cd577ed1e3b771d2a9))
	ROM_LOAD ( "rapids_3.u3", 0x0800, 0x0400, CRC(397992fb) SHA1(46e4f293fc8d8094eb16030261342504694fbf8f))
	ROM_LOAD ( "rapids_4.u4", 0x0c00, 0x0400, CRC(3319fa21) SHA1(b384a7347e0d6ca3bec53f356312b66d66b5b03f))
	ROM_LOAD ( "rapids_5.u5", 0x1000, 0x0400, CRC(0dd67110) SHA1(0c32e400ef07d7243148ae280e145a3e050313e8))
ROM_END

/*--------------------------------
/ Space Shuttle (09/80)
/-------------------------------*/
ROM_START(sshtlzac)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "zac_boot.u1", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "spcshtl2.u2", 0x1c00, 0x0400, CRC(0e06771b) SHA1(f30f3727f24219e5047c871fe81c2e172a17cd38))
	ROM_LOAD ( "spcshtl3.u3", 0x0800, 0x0400, CRC(a302e5a9) SHA1(1585f4000d105a7a2be5638ade9ab8668e6c8a5e))
	ROM_LOAD ( "spcshtl4.u4", 0x0c00, 0x0400, CRC(a02ee0b5) SHA1(50532bdc347ecfdbd4cc43403ff2cb1dcb1fe1ac))
	ROM_LOAD ( "spcshtl5.u5", 0x1000, 0x0400, CRC(d1dabd9b) SHA1(0d28336764f43fa4d1b23d849b6ec0f60b2b4ecf))

	ROM_REGION(0x1000, "audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("spcshtl.snd", 0x0000, 0x0800, CRC(9a61781c) SHA1(0293640653d8cc9532debd31bbb70f025b4e6d03))
ROM_END

/*--------------------------------
/ Star God (05/80)
/-------------------------------*/
ROM_START(stargod)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "zac_boot.u1", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "stargod2.u2", 0x1c00, 0x0400, CRC(7a784b03) SHA1(bc3490b69913f52e3e9db5c3de5617ab89efe073))
	ROM_LOAD ( "stargod3.u3", 0x0800, 0x0400, CRC(95492ac0) SHA1(992ad53efc5b53020e3939dfca5431fd50b6571c))
	ROM_LOAD ( "stargod4.u4", 0x0c00, 0x0400, CRC(09e5682a) SHA1(c9fcad4f55ee005e204a49fa65e7d77ecfde9680))
	ROM_LOAD ( "stargod5.u5", 0x1000, 0x0400, CRC(43ba2462) SHA1(6749bdceca4a1dc2bc90d7ee3b671f52219e1af4))
ROM_END

ROM_START(stargoda)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "zac_boot.u1", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "stargod2.u2", 0x1c00, 0x0400, CRC(7a784b03) SHA1(bc3490b69913f52e3e9db5c3de5617ab89efe073))
	ROM_LOAD ( "stargod3.u3", 0x0800, 0x0400, CRC(95492ac0) SHA1(992ad53efc5b53020e3939dfca5431fd50b6571c))
	ROM_LOAD ( "stargod4.u4", 0x0c00, 0x0400, CRC(09e5682a) SHA1(c9fcad4f55ee005e204a49fa65e7d77ecfde9680))
	ROM_LOAD ( "stargod5.u5", 0x1000, 0x0400, CRC(43ba2462) SHA1(6749bdceca4a1dc2bc90d7ee3b671f52219e1af4))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("stargod.snd", 0x7800, 0x0800, CRC(c9103a68) SHA1(cc77af54fdb192f0b334d9d1028210618c3f1d95))
	ROM_RELOAD( 0xf800, 0x0800)
ROM_END

ROM_START(stargodb) // alternate version of the stargod set, with variable replay score
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "zac_boot.u1", 0x0000, 0x0800, CRC(62a3da59) SHA1(db571139aff61757f6c0fda6fa0d1fea8257cb15))
	ROM_LOAD ( "stargod.u2",  0x1c00, 0x0400, CRC(a92ae202) SHA1(b5ed61f0c0c769f0bd4f5a69677d0eb5122bdb56))
	ROM_LOAD ( "stargod.u3",  0x0800, 0x0400, CRC(e677cf0d) SHA1(6aff0275148ccc56f2ac1bf5e5bd5baed64bfa7c))
	ROM_LOAD ( "stargod.u4",  0x0c00, 0x0400, CRC(fdfbb31f) SHA1(b64a529a097a7e2589ff124998160d375153d16c))
	ROM_LOAD ( "stargod.u5",  0x1000, 0x0400, CRC(536484f8) SHA1(7c40bf7e8b5b21cce44d96633581730ea9eeb176))
ROM_END

/*--------------------------------
/ Winter Sports (01/78)
/-------------------------------*/
ROM_START(wsports)
	ROM_REGION(0x2000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD ( "ws1.u1", 0x0000, 0x0400, CRC(58feb058) SHA1(50216bba5be28284e63d826543297d1b6b609325))
	ROM_LOAD ( "ws2.u2", 0x0400, 0x0400, CRC(ece702cb) SHA1(84cf0976b33bd7cf25976de9c66cc85808f1cd50))
	ROM_LOAD ( "ws3.u3", 0x0800, 0x0400, CRC(ff7f6824) SHA1(0eef4aca51c0e823f7634d7fc22c96c590239269))
	ROM_LOAD ( "ws4.u4", 0x0c00, 0x0400, CRC(74460cf2) SHA1(4afa612af1eff8eae686ceba7c117bc7962272c7))
	ROM_LOAD ( "ws5.u5", 0x1000, 0x0400, CRC(5ef51ced) SHA1(390579d0482ceabf87924f7718ef33e336726d92))
ROM_END

} // anonymous namespace

// Basic audio
GAME(1978, wsports,   0,       zac1,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Winter Sports",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )
GAME(1978, hod,       0,       zac1,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "House of Diamonds",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )
GAME(1978, futurwld,  0,       zac1,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Future World",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )

// 1B1125 audio (SN76477, NE555)
GAME(1979, strapids,  0,       zac3,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Shooting the Rapids",              MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )
GAME(1979, hotwheel,  0,       zac3,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Hot Wheels",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )
GAME(1980, firemntn,  0,       zac3,  firemntn, zac_1_state, empty_init, ROT0, "Zaccaria", "Fire Mountain",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )
GAME(1980, stargod,   0,       zac3,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Star God",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )
GAME(1980, stargodb,  stargod, zac3,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Star God (variable replay score)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )

// 1B1346 audio (i8035, MC1408)
GAME(1980, sshtlzac,  0,       zac2,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Space Shuttle (Zaccaria)",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )
GAME(1981, ewf,       0,       zac2,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Earth Wind Fire",                  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )

// 1B1146 audio (i8035, MC1408, SN76477, NE555, MM5837)
GAME(1981, locomotp,  0,       locomotp, zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Locomotion",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )

// unknown audio
GAME(1980, stargoda,  stargod, zac4,     zac_1, zac_1_state, empty_init, ROT0, "Zaccaria", "Star God (alternate sound)",       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE  )

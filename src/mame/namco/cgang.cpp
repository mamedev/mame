// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

『コズモギャングス』 (COSMOGANGS) by Namco, 1990. USA distribution was handled by
Data East, they titled it "Cosmo Gang".

It is an electromechanical arcade lightgun game. There is no screen, feedback
is with motorized elements, lamps and 7segs, and of course sounds and music.

To shoot the targets in MAME, either enable -mouse and click on one of the
pink aliens(left mouse button doubles as gun trigger by default).
Or, configure the gun aim inputs and share them with the trigger. For example
use Z,X,C,V,B for the gun aims, and "Z or X or C or V or B" for the trigger.

TODO:
- dump/add Japanese version
- shot target is sometimes ignored, maybe a BTANB? since it's easy to dismiss
  on the real thing as a missed aim. It turns on the lightgun lamp but then
  doesn't read the lightsensor.

--------------------------------------------------------------------------------

Hardware notes:

Main CPU side:
- Hitachi HD6809P @ 4MHz
- Hitachi HA1835P watchdog timer
- 32KB ROM(27C256), 8KB RAM(HM6264AP-10)
- 4*M5L8255AP-5 PPI, 2*M5L8253P-5 PIT
- 5*MB8713 motor drivers

Audio CPU side:
- Hitachi HD68B09EP @ 2MHz (8MHz XTAL)
- 32KB ROM(27C256), 16KB RAM(2*HM6264AP-10, some pins N/C)
- M5L8255AP-5 PPI
- Namco CUS121 sound interface, same chip used in Namco System 1
- Yamaha YM2151 @ 3.57MHz, 2*NEC D7759C @ 640kHz
- 2*128KB ADPCM ROM (27C010, one for each D7759C; labels usually peeled off
  or handwritten, perhaps due to field upgrade)

Cabinet:
- 5 lanes with movable aliens, lightsensor under mouth
- 5 'energy crates', aliens will try to steal them
- 2 lightguns
- UFO with leds above cabinet
- sliding door (only for Japan version?)
- 7segs for scorekeeping
- 2 speakers (but final mix is mono!)
- optional ticket/prize dispenser

Overall, the hardware has similarities with Wacky Gator, see wacky_gator.cpp.

*******************************************************************************/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/ripple_counter.h"
#include "machine/ticket.h"
#include "machine/watchdog.h"
#include "sound/upd7759.h"
#include "sound/ymopm.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "cgang.lh"


namespace {

// length of the sliding door in msec (time it takes to open or close)
static constexpr int DOOR_MOTOR_LIMIT = 2000;

// length of each cosmogang lane, in motor steps
// At game start, one cosmo going forward takes around 7 seconds, and if the player does nothing
// at the 1st round, game is lost with around 10 seconds remaining (compared to video recording).
static constexpr int CG_MOTOR_LIMIT = 1000;


class cgang_state : public driver_device
{
public:
	cgang_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_watchdog(*this, "watchdog"),
		m_latch(*this, "latch%u", 0),
		m_pit(*this, "pit%u", 0),
		m_ppi(*this, "ppi%u", 0),
		m_ticket(*this, "ticket"),
		m_adpcm(*this, "adpcm%u", 0),
		m_ymsnd(*this, "ymsnd"),
		m_spot(*this, "spot"),
		m_digits(*this, "digits"),
		m_fake(*this, "FAKE%u", 1),
		m_conf(*this, "CONF1"),
		m_dipsw(*this, "SW%u", 1),
		m_gun_lamps(*this, "gun_lamp%u", 0U),
		m_spot_lamps(*this, "spot_lamp%u", 0U),
		m_misc_lamps(*this, "misc_lamp%u", 0U),
		m_ufo_lamps(*this, "ufo_lamp%u", 0U),
		m_ufo_sol(*this, "ufo_sol"),
		m_en_sol(*this, "en_sol%u", 0U),
		m_cg_sol(*this, "cg_sol%u", 0U),
		m_door_count(*this, "door_count"),
		m_en_count(*this, "en_count%u", 0U),
		m_cg_count(*this, "cg_count%u", 0U)
	{ }

	void cgang(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device_array<generic_latch_8_device, 2> m_latch;
	required_device_array<pit8253_device, 2> m_pit;
	required_device_array<i8255_device, 5> m_ppi;
	required_device<ticket_dispenser_device> m_ticket;
	required_device_array<upd7759_device, 2> m_adpcm;
	required_device<ym2151_device> m_ymsnd;
	required_device<pwm_display_device> m_spot;
	required_device<pwm_display_device> m_digits;
	required_ioport_array<2> m_fake;
	required_ioport m_conf;
	required_ioport_array<4> m_dipsw;
	output_finder<2> m_gun_lamps;
	output_finder<8> m_spot_lamps;
	output_finder<2+3+1+2+3> m_misc_lamps;
	output_finder<8> m_ufo_lamps;
	output_finder<> m_ufo_sol;
	output_finder<5> m_en_sol;
	output_finder<5> m_cg_sol;
	output_finder<> m_door_count;
	output_finder<5> m_en_count;
	output_finder<5> m_cg_count;

	int m_watchdog_clk = 0;
	int m_main_irq = 0;
	int m_main_firq = 0;
	u8 m_door_motor_on = 0;
	int m_door_motor_pos = 0;
	u8 m_cg_motor_on = 0;
	u8 m_cg_motor_dir = 0;

	int m_cg_motor_clk[5] = { };
	int m_cg_motor_pos[5] = { };
	int m_en_pos[5] = { };

	emu_timer *m_door_timer;
	emu_timer *m_sol_filter[5];

	TIMER_CALLBACK_MEMBER(output_sol) { m_en_sol[param >> 1] = param & 1; }

	// address maps
	void main_map(address_map &map);
	void sound_map(address_map &map);

	// I/O handlers
	void main_irq_w(int state);
	void main_firq_w(int state);
	void main_irq_clear_w(u8 data);
	void main_firq_clear_w(u8 data);
	template<int N> void motor_clock_w(int state);
	void cg_motor_tick(int i);
	TIMER_CALLBACK_MEMBER(door_motor_tick);
	void refresh_motor_output();

	u8 ppi1_b_r();
	u8 ppi1_c_r();
	u8 ppi2_a_r();
	u8 ppi2_b_r();
	void ppi2_c_w(u8 data);
	void ppi3_a_w(u8 data);
	void ppi3_b_w(u8 data);
	void ppi3_c_w(u8 data);
	void set_en_sol(int i, int state);
	void ppi4_a_w(u8 data);
	void ppi4_b_w(u8 data);
	void ppi4_c_w(u8 data);

	template<int N> void adpcm_w(u8 data);
	void spot_w(u8 data);
	void output_spot_w(offs_t offset, u8 data) { m_spot_lamps[offset >> 6] = data; }

	void ppi5_a_w(u8 data);
	void ppi5_b_w(u8 data);
	u8 ppi5_c_r();
};

void cgang_state::machine_start()
{
	for (int i = 0; i < 5; i++)
		m_sol_filter[i] = timer_alloc(FUNC(cgang_state::output_sol), this);

	m_door_timer = timer_alloc(FUNC(cgang_state::door_motor_tick), this);
	m_door_timer->adjust(attotime::from_msec(1), 0, attotime::from_msec(1));

	// resolve outputs
	m_gun_lamps.resolve();
	m_spot_lamps.resolve();
	m_misc_lamps.resolve();
	m_ufo_lamps.resolve();
	m_ufo_sol.resolve();
	m_en_sol.resolve();
	m_cg_sol.resolve();
	m_door_count.resolve();
	m_en_count.resolve();
	m_cg_count.resolve();

	// register for savestates
	save_item(NAME(m_watchdog_clk));
	save_item(NAME(m_main_irq));
	save_item(NAME(m_main_firq));
	save_item(NAME(m_door_motor_on));
	save_item(NAME(m_door_motor_pos));
	save_item(NAME(m_cg_motor_on));
	save_item(NAME(m_cg_motor_dir));
	save_item(NAME(m_cg_motor_clk));
	save_item(NAME(m_cg_motor_pos));
	save_item(NAME(m_en_pos));
}

void cgang_state::machine_reset()
{
	m_main_irq = 0;
	m_main_firq = 0;

	// initial motor positions
	for (int i = 0; i < 5; i++)
	{
		m_cg_motor_clk[i] = 0;
		m_cg_motor_pos[i] = 0;
		m_en_pos[i] = CG_MOTOR_LIMIT;
		m_en_sol[i] = 0;
	}

	m_door_motor_pos = 0;
	m_door_motor_on = 0;
	m_cg_motor_on = 0;

	refresh_motor_output();
}



/*******************************************************************************
    I/O
*******************************************************************************/

// maincpu (misc)

void cgang_state::main_irq_w(int state)
{
	// irq on rising edge
	if (state && !m_main_irq)
		m_maincpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);

	m_main_irq = state;
}

void cgang_state::main_firq_w(int state)
{
	// firq on rising edge
	if (state && !m_main_firq)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);

	m_main_firq = state;
}

void cgang_state::main_irq_clear_w(u8 data)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
}

void cgang_state::main_firq_clear_w(u8 data)
{
	m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);
}

template<int N>
void cgang_state::motor_clock_w(int state)
{
	// clock stepper motors
	if (state && !m_cg_motor_clk[N] && BIT(m_cg_motor_on, N))
		cg_motor_tick(N);

	m_cg_motor_clk[N] = state;
}

void cgang_state::cg_motor_tick(int i)
{
	// note: the cosmogangs are stuck on the drive belts(5),
	// and the energy crates can lock themselves into position

	if (BIT(m_cg_motor_dir, i))
	{
		if (m_cg_motor_pos[i] > 0)
		{
			// pull energy crate
			if (m_en_sol[i] && m_cg_motor_pos[i] == m_en_pos[i])
				m_en_pos[i]--;

			m_cg_motor_pos[i]--;
		}
	}
	else
	{
		if (m_cg_motor_pos[i] < CG_MOTOR_LIMIT)
		{
			if (m_cg_motor_pos[i] < m_en_pos[i])
				m_cg_motor_pos[i]++;

			// push energy crate
			else if (m_en_sol[i])
			{
				m_cg_motor_pos[i]++;
				m_en_pos[i]++;
			}
		}
	}

	refresh_motor_output();
}

TIMER_CALLBACK_MEMBER(cgang_state::door_motor_tick)
{
	if (m_door_motor_on & 2 && m_door_motor_pos < DOOR_MOTOR_LIMIT)
		m_door_motor_pos++;
	else if (m_door_motor_on & 1 && m_door_motor_pos > 0)
		m_door_motor_pos--;

	refresh_motor_output();
}

void cgang_state::refresh_motor_output()
{
	// output motor positions in range 0-255
	for (int i = 0; i < 5; i++)
	{
		m_cg_count[i] = int((m_cg_motor_pos[i] / float(CG_MOTOR_LIMIT)) * 255.0 + 0.5);
		m_en_count[i] = int((m_en_pos[i] / float(CG_MOTOR_LIMIT)) * 255.0 + 0.5);
	}

	m_door_count = (m_conf->read() & 1) ? int((m_door_motor_pos / float(DOOR_MOTOR_LIMIT)) * 255.0 + 0.5) : 0;
}


// maincpu (PPI1-PPI4)

u8 cgang_state::ppi1_b_r()
{
	u8 data = 0xff;

	// PB0-PB4: cabinet front limit (CR1INI-CR5INI)
	for (int i = 0; i < 5; i++)
		if (m_en_pos[i] == CG_MOTOR_LIMIT)
			data ^= 1 << i;

	// PB5-PB7: cabinet back limit (HST1-HST3)
	for (int i = 0; i < 3; i++)
		if (m_cg_motor_pos[i] == 0)
			data ^= 1 << (i + 5);

	return data;
}

u8 cgang_state::ppi1_c_r()
{
	u8 data = 0x7f;

	// PC0-PC1: cabinet back limit (HST4-HST5)
	for (int i = 0; i < 2; i++)
		if (m_cg_motor_pos[i + 3] == 0)
			data ^= 1 << i;

	// PC2-PC6: cosmogang-energy crate collision (CR1-CR5)
	for (int i = 0; i < 5; i++)
		if (m_cg_motor_pos[i] == m_en_pos[i])
			data ^= 1 << (i + 2);

	// PC7: audiocpu mailbox status
	data |= m_latch[1]->pending_r() ? 0 : 0x80;

	return data;
}

u8 cgang_state::ppi2_a_r()
{
	u8 data = 0x60;

	// PA0-PA4: cosmogang hit lightsensors (G1HIT-G5HIT)
	for (int i = 0; i < 2; i++)
	{
		u8 mask = m_gun_lamps[i] ? 0x1f : 0;
		data |= m_fake[i]->read() & mask;
	}

	// lightsensors masked with SW3
	data |= ~m_dipsw[2]->read() & 0x1f;

	// PA5: door down limit switch
	// PA6: door up limit switch
	if (m_door_motor_pos == 0)
		data ^= 0x20;
	else if (m_door_motor_pos == DOOR_MOTOR_LIMIT)
		data ^= 0x40;

	// force door limit switches low to disable
	if (~m_conf->read() & 1)
		data &= ~0x60;

	return data;
}

u8 cgang_state::ppi2_b_r()
{
	u8 data = 0x1f;

	// PB0-PB4: cabinet center (G1POG-G5POG)
	for (int i = 0; i < 5; i++)
		if (m_cg_motor_pos[i] < (CG_MOTOR_LIMIT / 8))
			data ^= 1 << i;

	return data;
}

void cgang_state::ppi2_c_w(u8 data)
{
	// PC0: coincounter
	// PC1: coinlock solenoid
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_lockout_w(0, BIT(~data, 1));

	// PC2: start button lamp (easy)
	// PC3: start button lamp (hard)
	m_misc_lamps[0] = BIT(data, 2);
	m_misc_lamps[1] = BIT(data, 3);
}

void cgang_state::ppi3_a_w(u8 data)
{
	// PA0-PA4: cosmogang motor direction
	m_cg_motor_dir = data & 0x1f;

	// PA5,PA6: ufo left/right lamps
	for (int i = 0; i < 2; i++)
		m_misc_lamps[i + 6] = BIT(data, i + 5);
}

void cgang_state::ppi3_b_w(u8 data)
{
	// PB0-PB4: cosmogang motor power
	m_cg_motor_on = data & 0x1f;

	// PB5: game over(winning) lamp
	// PB6: 1P lamp
	// PB7: 2P lamp
	for (int i = 0; i < 3; i++)
		m_misc_lamps[i + 8] = BIT(data, i + 5);
}

void cgang_state::ppi3_c_w(u8 data)
{
	// PC0-PC3: 7448
	// PC4-PC7: 7445
	static const u8 ls48_map[0x10] =
		{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

	m_digits->matrix((1 << (data >> 4 & 0xf)) & 0x3ff, ls48_map[data & 0xf]);
}

void cgang_state::set_en_sol(int i, int state)
{
	// put a filter on energy crate solenoids, since game keeps strobing them
	if (state != (m_sol_filter[i]->param() & 1))
		m_sol_filter[i]->adjust(attotime::from_msec(1), i << 1 | state);
}

void cgang_state::ppi4_a_w(u8 data)
{
	// PA2-PA4: round leds
	for (int i = 0; i < 3; i++)
		m_misc_lamps[i + 2] = BIT(data, i + 2);

	// PA5-PA7: energy crate solenoids (1-3)
	for (int i = 0; i < 3; i++)
		set_en_sol(i, BIT(data, i + 5));
}

void cgang_state::ppi4_b_w(u8 data)
{
	// PB0,PB1: energy crate solenoids (4-5)
	for (int i = 0; i < 2; i++)
		set_en_sol(i + 3, BIT(data, i));

	// PB2-PB6: cosmogang solenoids
	for (int i = 0; i < 5; i++)
		m_cg_sol[i] = BIT(data, i + 2);

	// PB7: target lamp
	m_misc_lamps[5] = BIT(data, 7);
}

void cgang_state::ppi4_c_w(u8 data)
{
	// PC0,PC1: gun xenon lamps
	for (int i = 0; i < 2; i++)
		m_gun_lamps[i] = BIT(~data, i);

	// PC4: ticket motor
	m_ticket->motor_w(BIT(data, 4));

	// PC5: door motor reverse
	// PC6: door motor
	m_door_motor_on = data >> 5 & 3;

	// PC7: watchdog P-RUN
	int wd = BIT(data, 7);
	if (wd && !m_watchdog_clk)
		m_watchdog->reset_w();

	m_watchdog_clk = wd;
}


// audiocpu

template<int N>
void cgang_state::adpcm_w(u8 data)
{
	m_adpcm[N]->port_w(data);

	// also strobes start
	m_adpcm[N]->start_w(0);
	m_adpcm[N]->start_w(1);
}

void cgang_state::spot_w(u8 data)
{
	// d0-d2: ufo boss spotlights
	// d3-d7: cosmo spotlights
	// it strobes them for dimming
	m_spot->matrix(1, data);
}

void cgang_state::ppi5_a_w(u8 data)
{
	// PA0,PA1: ADPCM reset
	m_adpcm[0]->reset_w(BIT(data, 0));
	m_adpcm[1]->reset_w(BIT(data, 1));

	// PA2: music volume
	// PA3: voice volume
	// PA4: mute
	bool mute = BIT(~data, 4);
	m_ymsnd->set_output_gain(ALL_OUTPUTS, mute ? 0.0 : (BIT(data, 2) ? 0.25 : 1.0));
	m_adpcm[0]->set_output_gain(ALL_OUTPUTS, mute ? 0.0 : (BIT(data, 3) ? 0.25 : 1.0));
	m_adpcm[1]->set_output_gain(ALL_OUTPUTS, mute ? 0.0 : (BIT(data, 3) ? 0.25 : 1.0));

	// PA7: ufo boss mouth solenoid
	m_ufo_sol = BIT(data, 7);
}

void cgang_state::ppi5_b_w(u8 data)
{
	// PB0-PB7: ufo lamps
	for (int i = 0; i < 8; i++)
		m_ufo_lamps[i] = BIT(data, i);
}

u8 cgang_state::ppi5_c_r()
{
	u8 data = 0;

	// PC0,PC1: ADPCM busy
	data |= m_adpcm[0]->busy_r() ? 1 : 0;
	data |= m_adpcm[1]->busy_r() ? 2 : 0;

	// PC2: maincpu mailbox status
	data |= m_latch[0]->pending_r() ? 0 : 4;

	// PC5-PC7: SW1
	data |= m_dipsw[0]->read() << 5;

	return data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void cgang_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2003).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x2004, 0x2007).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x2008, 0x200b).rw(m_ppi[2], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x200c, 0x200f).rw(m_ppi[3], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x4000, 0x4000).mirror(0x0003).w(m_latch[0], FUNC(generic_latch_8_device::write));
	map(0x4004, 0x4004).mirror(0x0003).r(m_latch[1], FUNC(generic_latch_8_device::read));
	map(0x4008, 0x4008).mirror(0x0003).w(FUNC(cgang_state::main_irq_clear_w));
	map(0x400c, 0x400c).mirror(0x0003).w(FUNC(cgang_state::main_firq_clear_w));
	map(0x4010, 0x4013).rw(m_pit[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x4014, 0x4017).rw(m_pit[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x4018, 0x4018).mirror(0x0003).portr("SW2");
	map(0x8000, 0xffff).rom();
}

void cgang_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1001).rw(m_ymsnd, FUNC(ym2151_device::status_r), FUNC(ym2151_device::write));
	map(0x2000, 0x2003).mirror(0x0ffc).rw(m_ppi[4], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x3000, 0x3000).mirror(0x0fff).r(m_latch[0], FUNC(generic_latch_8_device::read));
	map(0x4000, 0x4000).mirror(0x0fff).w(m_latch[1], FUNC(generic_latch_8_device::write));
	map(0x5000, 0x5000).mirror(0x0fff).w(FUNC(cgang_state::adpcm_w<0>));
	map(0x6000, 0x6000).mirror(0x0fff).w(FUNC(cgang_state::adpcm_w<1>));
	map(0x7000, 0x7000).mirror(0x0fff).w(FUNC(cgang_state::spot_w)).nopr();
	map(0x8000, 0xffff).rom();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cgang )
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("Start (Hard)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start (Easy)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Dispenser")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("FAKE1") // fake inputs, indicating gun is aimed at target
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Aim Target 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Aim Target 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Aim Target 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Aim Target 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 Aim Target 5")

	PORT_START("FAKE2") // "
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Aim Target 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Aim Target 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Aim Target 3")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Aim Target 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 Aim Target 5")

	PORT_START("CONF1") // motorized sliding door in front of ufo entrance
	PORT_CONFNAME( 0x01, 0x01, "Sliding Door System")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x01, DEF_STR( On ) )

	PORT_START("SW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3" )

	PORT_START("SW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, "Attract Play" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, "Points per Ticket" ) PORT_DIPLOCATION("SW2:4,5") PORT_CONDITION("SW2", 0x20, EQUALS, 0x20)
	PORT_DIPSETTING(    0x18, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPSETTING(    0x08, "15" )
	PORT_DIPSETTING(    0x00, "20" )
	PORT_DIPNAME( 0x18, 0x18, "Points per Prize" ) PORT_DIPLOCATION("SW2:4,5") PORT_CONDITION("SW2", 0x20, EQUALS, 0x00)
	PORT_DIPSETTING(    0x18, "20" )
	PORT_DIPSETTING(    0x10, "40" )
	PORT_DIPSETTING(    0x08, "60" )
	PORT_DIPSETTING(    0x00, "80" )
	PORT_DIPNAME( 0x20, 0x20, "Dispenser Mode" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "Ticket" )
	PORT_DIPSETTING(    0x00, "Prize" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )

	PORT_START("SW3") // disables one of the lanes in case of hardware malfunction
	PORT_DIPNAME( 0x01, 0x01, "Drive System 1" ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Drive System 2" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Drive System 3" ) PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Drive System 4" ) PORT_DIPLOCATION("SW3:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Drive System 5" ) PORT_DIPLOCATION("SW3:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6" )

	PORT_START("SW4")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW4:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Dispenser Enabled" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cgang_state::cgang(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cgang_state::main_map);

	MC6809E(config, m_audiocpu, 8_MHz_XTAL/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &cgang_state::sound_map);
	m_audiocpu->set_periodic_int(FUNC(cgang_state::nmi_line_pulse), attotime::from_hz(8_MHz_XTAL/4 / 0x1000));

	PIT8253(config, m_pit[0], 0);
	m_pit[0]->set_clk<0>(4_MHz_XTAL/4);
	m_pit[0]->set_clk<1>(4_MHz_XTAL/4);
	m_pit[0]->set_clk<2>(4_MHz_XTAL/4);
	m_pit[0]->out_handler<0>().set(FUNC(cgang_state::motor_clock_w<0>));
	m_pit[0]->out_handler<1>().set(FUNC(cgang_state::motor_clock_w<1>));
	m_pit[0]->out_handler<2>().set(FUNC(cgang_state::motor_clock_w<2>));

	PIT8253(config, m_pit[1], 0);
	m_pit[1]->set_clk<0>(4_MHz_XTAL/4);
	m_pit[1]->set_clk<1>(4_MHz_XTAL/4);
	m_pit[1]->set_clk<2>(4_MHz_XTAL/4);
	m_pit[1]->out_handler<0>().set(FUNC(cgang_state::motor_clock_w<3>));
	m_pit[1]->out_handler<1>().set(FUNC(cgang_state::motor_clock_w<4>));
	m_pit[1]->out_handler<2>().set("int_clk", FUNC(ripple_counter_device::clock_w));

	ripple_counter_device &int_clk(RIPPLE_COUNTER(config, "int_clk")); // 4040
	int_clk.set_stages(12);
	int_clk.count_out_cb().set_inputline(m_maincpu, INPUT_LINE_NMI).bit(0);
	int_clk.count_out_cb().append(FUNC(cgang_state::main_irq_w)).bit(3);
	int_clk.count_out_cb().append(FUNC(cgang_state::main_firq_w)).bit(4);

	GENERIC_LATCH_8(config, m_latch[0]);
	GENERIC_LATCH_8(config, m_latch[1]);

	I8255(config, m_ppi[0]); // 0x9b: all = input
	m_ppi[0]->in_pa_callback().set_ioport("IN1");
	m_ppi[0]->in_pb_callback().set(FUNC(cgang_state::ppi1_b_r));
	m_ppi[0]->in_pc_callback().set(FUNC(cgang_state::ppi1_c_r));

	I8255(config, m_ppi[1]); // 0x9a: A & B = input, Clow = output, Chigh = input
	m_ppi[1]->in_pa_callback().set(FUNC(cgang_state::ppi2_a_r));
	m_ppi[1]->in_pb_callback().set(FUNC(cgang_state::ppi2_b_r));
	m_ppi[1]->in_pc_callback().set_ioport("SW4").lshift(4);
	m_ppi[1]->out_pc_callback().set(FUNC(cgang_state::ppi2_c_w));

	I8255(config, m_ppi[2]); // 0x80: all = output
	m_ppi[2]->out_pa_callback().set(FUNC(cgang_state::ppi3_a_w));
	m_ppi[2]->out_pb_callback().set(FUNC(cgang_state::ppi3_b_w));
	m_ppi[2]->out_pc_callback().set(FUNC(cgang_state::ppi3_c_w));

	I8255(config, m_ppi[3]); // 0x80: all = output
	m_ppi[3]->out_pa_callback().set(FUNC(cgang_state::ppi4_a_w));
	m_ppi[3]->out_pb_callback().set(FUNC(cgang_state::ppi4_b_w));
	m_ppi[3]->out_pc_callback().set(FUNC(cgang_state::ppi4_c_w));

	I8255(config, m_ppi[4]); // 0x89: A & B = output, C = input
	m_ppi[4]->out_pa_callback().set(FUNC(cgang_state::ppi5_a_w));
	m_ppi[4]->out_pb_callback().set(FUNC(cgang_state::ppi5_b_w));
	m_ppi[4]->in_pc_callback().set(FUNC(cgang_state::ppi5_c_r));

	for (int i = 0; i < 5; i++)
	{
		m_ppi[i]->tri_pa_callback().set_constant(0);
		m_ppi[i]->tri_pb_callback().set_constant(0);
		m_ppi[i]->tri_pc_callback().set_constant(0);
	}

	WATCHDOG_TIMER(config, m_watchdog); // HA1835P
	m_watchdog->set_time(attotime::from_msec(100)); // approximation

	TICKET_DISPENSER(config, m_ticket, attotime::from_msec(3000), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH);

	// video hardware
	PWM_DISPLAY(config, m_digits).set_size(10, 7);
	m_digits->set_segmask(0x3ff, 0x7f);

	PWM_DISPLAY(config, m_spot).set_size(1, 8);
	m_spot->output_x().set(FUNC(cgang_state::output_spot_w));
	m_spot->set_bri_levels(0.0576, 0.144, 0.36, 0.9); // dimmed lights

	config.set_default_layout(layout_cgang);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YM2151(config, m_ymsnd, 3.579545_MHz_XTAL);
	m_ymsnd->add_route(ALL_OUTPUTS, "mono", 0.50);

	UPD7759(config, m_adpcm[0], 640_kHz_XTAL);
	m_adpcm[0]->add_route(ALL_OUTPUTS, "mono", 0.75);
	UPD7759(config, m_adpcm[1], 640_kHz_XTAL);
	m_adpcm[1]->add_route(ALL_OUTPUTS, "mono", 0.75);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cgang )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cg1_mp0d.4j", 0x8000, 0x8000, CRC(2114cb55) SHA1(4e330cb3d8d96ec06faa25cbaeed97b1c2eff8db) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD("cg1_sp0b.4b", 0x8000, 0x8000, CRC(62974140) SHA1(5eee3f6345521e3fb76acb3acaa5c9df75db91db) )

	ROM_REGION( 0x20000, "adpcm0", 0 )
	ROM_LOAD("cg2_9c_e02a.9c", 0x00000, 0x20000, CRC(f9a3f8a0) SHA1(5ad8b408d36397227019afd15c3516f85488c6df) ) // handwritten label seen on one PCB

	ROM_REGION( 0x20000, "adpcm1", 0 )
	ROM_LOAD("cg2_9e_586e.9e", 0x00000, 0x20000, CRC(40e7f60b) SHA1(af641b0562db1ae033cee67df583d178fd8c93f3) ) // "
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME   PARENT  MACHINE  INPUT  CLASS        INIT        MNTR  COMPANY, FULLNAME, FLAGS
GAME( 1990, cgang, 0,      cgang,   cgang, cgang_state, empty_init, ROT0, "Namco (Data East license)", "Cosmo Gang (US)", MACHINE_SUPPORTS_SAVE | MACHINE_MECHANICAL | MACHINE_IMPERFECT_CONTROLS )

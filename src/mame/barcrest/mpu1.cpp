// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

	Barcrest MPU1 (1978)

	Barcrest's first fruit machine platform to use a conventional CPU, 
	possibly the first of it's kind. Has electromechanical reels 
	instead of the usual stepper reels. The hardware features a MC6800 
	CPU clocked at around 1 MHz, 128 bytes of RAM and two PIAs for I/O. 
	The PIAs can interface with up to 13 lamps, 4 reels, 8 triacs and 12 inputs 
    (4 of these being special coin inputs). A 1-bit DAC is used for audio. 
	Games are stored on cartridges that can hold up to 4K of data. 
	Most cartridges use a common 2K mask ROM, with the remaining 2K 
	available for the game program.
	
	TODO:
    - Layouts (there currently aren't any...)

**********************************************************************/

#include "emu.h"

#include "fruitsamples.h"

#include "cpu/m6800/m6800.h"

#include "machine/6821pia.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "speaker.h"

namespace {

#include "mpu1.lh"
#include "m_bappl2.lh"
#include "m_bapple.lh"
#include "m_gndgit.lh"
#include "m_lndg.lh"
#include "m_mtchit.lh"
#include "m_mtchup.lh"

class mpu1_state : public driver_device
{
public:
	mpu1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia1(*this, "pia1"),
		m_pia2(*this, "pia2"),
		m_lamps(*this, "lamp%u", 0U),
		m_reel_out(*this, "reel%u", 1U),
		m_sreel_out(*this, "sreel%u", 1U),
		m_dac(*this, "dac"),
		m_samples(*this, "samples")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(coin_input);

	void mpu1(machine_config &config);
	void mpu1_lg(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	template <unsigned Lamp> DECLARE_WRITE_LINE_MEMBER(pia_lamp_w) { m_lamps[Lamp] = state; }

	void pia1_portb_w(uint8_t data);
	void pia1_portb_lg_w(uint8_t data);
	uint8_t pia2_porta_r();
	void pia2_porta_w(uint8_t data);
	void pia2_portb_w(uint8_t data);
	void payout_cash_w(bool state);
	void payout_token_w(bool state);
	void meter_w(int meter, bool state);
	void coin_lockout_w(bool state);
	void reel_w(int reel, bool state);

	void mpu1_map(address_map &map);

	uint8_t m_reel_select;
	bool m_pia2a_select;
	bool m_prev_payout[2];
	uint8_t m_reel_state[4];
	uint16_t m_reel_pos[4];
	emu_timer *m_reel_timer[4];
	attotime m_reel_speed;
	uint8_t m_pia2a_bit7_value;
	emu_timer *m_change_pia2a_bit7_timer;

	TIMER_DEVICE_CALLBACK_MEMBER(nmi);
	template <unsigned Reel> TIMER_CALLBACK_MEMBER(reel_move);
	TIMER_CALLBACK_MEMBER(change_pia2a_bit7);

	enum
	{
		REEL_STOPPED = 0,
		REEL_SPINNING,
		REEL_STOPPING
	};

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	output_finder<13> m_lamps;
	output_finder<4> m_reel_out;
	output_finder<4> m_sreel_out;
	required_device<dac_1bit_device> m_dac;
	required_device<fruit_samples_device> m_samples;
};

void mpu1_state::mpu1_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x007f).ram();
	map(0x0600, 0x0fff).rom();
	map(0x1000, 0x17ff).rom().region("maskrom", 0).mirror(0x2800);
	map(0x2000, 0x2003).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2004, 0x2007).rw(m_pia2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}

void mpu1_state::pia1_portb_w(uint8_t data)
{
	if(BIT(data, 7) == 0)
	{
		reel_w(0, BIT(data, 0));
		reel_w(1, BIT(data, 1));
		reel_w(2, BIT(data, 2));
		reel_w(3, BIT(data, 3));
		coin_lockout_w(BIT(data, 4));
		m_lamps[11] = BIT(data, 5);
		meter_w(0, BIT(data, 6));
	}
	else
	{
		m_lamps[12] = BIT(data, 0);
		payout_cash_w(BIT(data, 1));
		payout_token_w(BIT(data, 2));
		meter_w(1, BIT(data, 3));
		meter_w(2, BIT(data, 4));
		meter_w(3, BIT(data, 5));
		meter_w(4, BIT(data, 6));
	}
}

void mpu1_state::pia1_portb_lg_w(uint8_t data)
{
	if(data == 0)
	{
		for(int i = 0; i < 5; i++) meter_w(i, 0);
		payout_cash_w(0);
		payout_token_w(0);
	}

	if(BIT(data, 7) == 0)
	{
		reel_w(0, BIT(data, 0));
		reel_w(1, BIT(data, 1));
		reel_w(2, BIT(data, 2));
		reel_w(3, BIT(data, 3));
		coin_lockout_w(BIT(data, 4));
		// Manual says bit 5 might be "Reel Motor", reels work fine without this
		m_lamps[11] = BIT(data, 6);
	}
	else
	{
		meter_w(0, BIT(data, 0));
		payout_cash_w(BIT(data, 1));
		payout_token_w(BIT(data, 2));
		meter_w(1, BIT(data, 3));
		meter_w(2, BIT(data, 4));
		meter_w(3, BIT(data, 5));
		meter_w(4, BIT(data, 6));
	}
}

uint8_t mpu1_state::pia2_porta_r()
{
	if(m_pia2a_select == 0)
	{
		uint16_t pos = m_reel_pos[m_reel_select];
		if(pos % 20 == 0)
			return (pos / 20) + 1;
		else 
			return 0;
	}
	else
	{
		// Games won't boot until bit 7 here reads 1, but will only accept coins if it's 0 (door interlock switch?)
		return m_pia2a_bit7_value;
	}
}

void mpu1_state::pia2_porta_w(uint8_t data)
{
	m_pia2a_select = BIT(data, 7);
	if(m_pia2a_select == 0) m_reel_select = (data & 0x60) >> 5;
}

void mpu1_state::pia2_portb_w(uint8_t data)
{
	for(int i = 0; i < 8; i++) m_lamps[i] = BIT(data, i);
}

void mpu1_state::payout_cash_w(bool state)
{
	if(!m_prev_payout[0] && state) m_samples->play(fruit_samples_device::SAMPLE_PAYOUT);
	m_prev_payout[0] = state;
}

void mpu1_state::payout_token_w(bool state)
{
	if(!m_prev_payout[1] && state) m_samples->play(fruit_samples_device::SAMPLE_PAYOUT);
	m_prev_payout[1] = state;
}

void mpu1_state::meter_w(int meter, bool state)
{
	machine().bookkeeping().coin_counter_w(meter, state);
	// MPU1 doesn't have audible meters
}

void mpu1_state::coin_lockout_w(bool state)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
	machine().bookkeeping().coin_lockout_w(1, !state);
	machine().bookkeeping().coin_lockout_w(2, !state);
	machine().bookkeeping().coin_lockout_w(3, !state);
}

TIMER_DEVICE_CALLBACK_MEMBER( mpu1_state::nmi )
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( mpu1_state::coin_input )
{
	switch(param)
	{
		case 0: m_pia1->ca1_w(newval); break;
		case 1: m_pia1->cb1_w(newval); break;
		case 2: m_pia2->ca1_w(newval); break;
		case 3: m_pia2->cb1_w(newval); break;
	}
}

TIMER_CALLBACK_MEMBER( mpu1_state::change_pia2a_bit7 )
{
	m_pia2a_bit7_value = 0;
}

/* MPU1 does not have stepper reels, it instead uses an electromechanical reel system.
   Each reel has a single output - setting the output high causes the reel to start moving, 
   and once it's set back low, the reel will stop at whichever symbol it's heading towards.
   Position tracking is done via contacts on index plates below each symbol, which the CPU
   will read once all reels are stopped.

   I've modeled the reels as having 400 virtual "steps". Every reel has 20 symbols, which
   gives 20 "steps" between each symbol. */
void mpu1_state::reel_w(int reel, bool state)
{
	if(m_reel_state[reel] == REEL_STOPPED)
	{
		if(state)
		{
			m_reel_state[reel] = REEL_SPINNING;
			m_reel_timer[reel]->adjust(m_reel_speed);
			m_samples->play(fruit_samples_device::SAMPLE_EM_REEL_1_START + reel);
		}
	}
	else if(m_reel_state[reel] == REEL_SPINNING)
	{
		if(!state)
		{
			if(m_reel_pos[reel] % 20 == 0) // If reel is already on a symbol, then stop it immediately
			{
				m_reel_timer[reel]->adjust(attotime::never);
				m_reel_state[reel] = REEL_STOPPED;
				m_samples->play(fruit_samples_device::SAMPLE_EM_REEL_1_STOP + reel);
			}
			else m_reel_state[reel] = REEL_STOPPING;
		}
	}
}

template <unsigned Reel> 
TIMER_CALLBACK_MEMBER( mpu1_state::reel_move )
{
	if(m_reel_pos[Reel] == 0)
		m_reel_pos[Reel] = 400 - 1;
	else
		m_reel_pos[Reel]--;

	if(m_reel_state[Reel] == REEL_STOPPING && m_reel_pos[Reel] % 20 == 0) // Stop once a symbol is reached
	{
		m_reel_timer[Reel]->adjust(attotime::never);
		m_reel_state[Reel] = REEL_STOPPED;
		m_samples->play(fruit_samples_device::SAMPLE_EM_REEL_1_STOP + Reel);
	}
	else m_reel_timer[Reel]->adjust(m_reel_speed);

	m_reel_out[Reel] = m_reel_pos[Reel];
	m_sreel_out[Reel] = (m_reel_pos[Reel] * 0x10000) / 400;
}

static INPUT_PORTS_START( mpu1_inputs )
	PORT_START("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_TOGGLE

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("5p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 0) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 1) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("10p Token") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 2) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 3) PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( m_gndgit )
	PORT_INCLUDE( mpu1_inputs )

	PORT_MODIFY("IN")
	PORT_DIPNAME( 0x01, 0x01, "Gamble feature limit" )
	PORT_DIPSETTING(    0x00, "£1" )
	PORT_DIPSETTING(    0x01, "50p" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
INPUT_PORTS_END

static INPUT_PORTS_START( m_mtchit )
	PORT_INCLUDE( mpu1_inputs )

	PORT_MODIFY("IN")
	PORT_DIPNAME( 0x01, 0x01, "Gamble feature limit" )
	PORT_DIPSETTING(    0x00, "£1" )
	PORT_DIPSETTING(    0x01, "50p" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Match 1/Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Match 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Match 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
INPUT_PORTS_END

static INPUT_PORTS_START( m_mtchup )
	PORT_INCLUDE( mpu1_inputs )

	PORT_MODIFY("IN")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Match 1/Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Match 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Match 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_MODIFY("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 1) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p Token") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 2) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("50p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 3) PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( m_lndg )
	PORT_INCLUDE( mpu1_inputs )

	PORT_MODIFY("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Respin")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
INPUT_PORTS_END

void mpu1_state::machine_start()
{
	m_lamps.resolve();
	m_reel_out.resolve();
	m_sreel_out.resolve();

	save_item(NAME(m_reel_state));
	save_item(NAME(m_reel_pos));
	save_item(NAME(m_reel_speed));

	m_reel_timer[0] = timer_alloc(FUNC(mpu1_state::reel_move<0>), this);
	m_reel_timer[1] = timer_alloc(FUNC(mpu1_state::reel_move<1>), this);
	m_reel_timer[2] = timer_alloc(FUNC(mpu1_state::reel_move<2>), this);
	m_reel_timer[3] = timer_alloc(FUNC(mpu1_state::reel_move<3>), this);
	m_change_pia2a_bit7_timer = timer_alloc(FUNC(mpu1_state::change_pia2a_bit7), this);

	for(int i = 0; i < 4; i++) 
	{
		m_reel_state[i] = REEL_STOPPED;
		m_reel_pos[i] = 0;
	}
}

void mpu1_state::machine_reset()
{
	// Return PIA2 port A bit 7 as set for 0.1s from boot, then clear it
	m_pia2a_bit7_value = 0x80;
	m_change_pia2a_bit7_timer->adjust(attotime::from_msec(100));
}

void mpu1_state::mpu1(machine_config &config)
{
	M6800(config, m_maincpu, 1'000'000); /* On MPU1, the clock comes from a multivibrator circuit varying somewhere around 1 MHz from
	                                        board to board. This results in for example slightly different sound pitch across machines.
	                                        I've set a stable 1 MHz clock here, which is also the case on MPU2. */
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu1_state::mpu1_map);

	TIMER(config, "nmi").configure_periodic(FUNC(mpu1_state::nmi), attotime::from_hz(100)); // From AC zero crossing detector

	PIA6821(config, m_pia1, 0);
	m_pia1->readpa_handler().set_ioport("IN");
	m_pia1->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia1->ca1_w(0);
	m_pia1->ca2_handler().set(m_dac, FUNC(dac_1bit_device::write));
	m_pia1->writepb_handler().set(FUNC(mpu1_state::pia1_portb_w));
	m_pia1->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia1->cb1_w(0);
	m_pia1->cb2_handler().set(FUNC(mpu1_state::pia_lamp_w<8>));

	PIA6821(config, m_pia2, 0);
	m_pia2->readpa_handler().set(FUNC(mpu1_state::pia2_porta_r));
	m_pia2->set_port_a_input_overrides_output_mask(0x80);
	m_pia2->writepa_handler().set(FUNC(mpu1_state::pia2_porta_w));
	m_pia2->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia2->ca1_w(0);
	m_pia2->ca2_handler().set(FUNC(mpu1_state::pia_lamp_w<9>));
	m_pia2->writepb_handler().set(FUNC(mpu1_state::pia2_portb_w));
	m_pia2->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia2->cb1_w(0);
	m_pia2->cb2_handler().set(FUNC(mpu1_state::pia_lamp_w<10>));

	m_reel_speed = attotime::from_usec(2000); // Seems close enough to footage of a real machine

	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.25);

	FRUIT_SAMPLES(config, m_samples);
}

void mpu1_state::mpu1_lg(machine_config &config)
{
	mpu1(config);

	m_pia1->writepb_handler().set(FUNC(mpu1_state::pia1_portb_lg_w));

	m_reel_speed = attotime::from_usec(2600); // Slower reels
}

// Common mask ROM on most cartridges, also used by MPU2
#define MPU1_MASKROM \
	ROM_REGION( 0x800, "maskrom", 0 ) \
	ROM_LOAD( "scm39199.bin", 0x0, 0x800, CRC(198d77ee) SHA1(ef466e539efd6e31c82ef01b09d63b7580f068fe) )
	// Barcrest later switched to SL32000 (a Fairchild part)

ROM_START( m_gndgit )
	MPU1_MASKROM

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "gndgit.p1", 0x600, 0x400, CRC(292b402c) SHA1(8d5fa5aa0b45a2605351119289fba92daa95a552) )
	ROM_LOAD( "gndgit.p2", 0xa00, 0x400, CRC(a9f10d07) SHA1(93874b28915972ae321b2a1471878d99839dd974) )
ROM_END

ROM_START( m_mtchit )
	MPU1_MASKROM

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mtchit.p1", 0x800, 0x200, CRC(a446cee0) SHA1(e4d700361184a7e9a29d9bf517b4b7531a9d5aba) )
	ROM_LOAD( "mtchit.p2", 0xa00, 0x200, CRC(e009faf1) SHA1(d26cd05bda88da4ff56cba00670fa499d0067e81) )
ROM_END

ROM_START( m_mtchup )
	MPU1_MASKROM

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mup2.p1", 0x800, 0x800, CRC(7af45c0a) SHA1(ee11a7173829bf6426c5f278cbe887051d7b2fb8) )
ROM_END

ROM_START( m_bapple )
	MPU1_MASKROM

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bapple.p1", 0x800, 0x200, CRC(3265ffbf) SHA1(c92d85e094d8f2eb2e2de96f67296f8e14a5d0ff) )
	ROM_LOAD( "bapple.p2", 0xa00, 0x200, CRC(e75d2760) SHA1(abe3e5e3e1bef8e410dc9bac968688ccd4abfcee) )
ROM_END

ROM_START( m_lndg )
	MPU1_MASKROM

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ln.p1", 0x800, 0x200, CRC(9649e598) SHA1(3f19fa8354d6eacaaeaa0f939e471844adbf65a6) )
	ROM_LOAD( "ln.p2", 0xa00, 0x200, CRC(70c157f0) SHA1(a47372c72375a94fff3e6f155fc05b89dabe3b1c) )
ROM_END

ROM_START( m_bappl2 )
	// Doesn't use the standard mask ROM
	ROM_REGION( 0x800, "maskrom", 0 )
	ROM_LOAD( "bappl2.p1", 0x0000, 0x800, CRC(933a7ab4) SHA1(8a33e6bef587d3f6f6228fe15147c9dcef984972) )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_FILL(              0x600, 0xa00, 0x00)
ROM_END

} // anonymous namespace

#define GAME_FLAGS MACHINE_NOT_WORKING|MACHINE_MECHANICAL|MACHINE_REQUIRES_ARTWORK|MACHINE_SUPPORTS_SAVE

GAMEL(1978,  m_gndgit,  0,        mpu1, m_gndgit,  mpu1_state, empty_init, ROT0, "Barcrest", "Golden Nudge It (Barcrest) (MPU1) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m_gndgit )
GAMEL(1979,  m_mtchit,  0,        mpu1, m_mtchit,  mpu1_state, empty_init, ROT0, "Barcrest", "Match It (Barcrest) (MPU1) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m_mtchit )
GAMEL(1981,  m_mtchup,  0,        mpu1, m_mtchup,  mpu1_state, empty_init, ROT0, "Barcrest", "Match Up (Barcrest) (MPU1) (10p Stake, £2 Jackpot)", GAME_FLAGS, layout_m_mtchup )
GAMEL(1980?, m_lndg,    0,        mpu1_lg, m_lndg,  mpu1_state, empty_init, ROT0, "Leisure Games", "Lucky Nudge (Leisure Games) (MPU1) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m_lndg )
GAMEL(1980?, m_bapple,  0,        mpu1_lg, m_lndg,  mpu1_state, empty_init, ROT0, "Leisure Games", "Big Apple (Leisure Games) (MPU1) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m_bapple )
GAMEL(1981?, m_bappl2,  0,        mpu1_lg, m_lndg,  mpu1_state, empty_init, ROT0, "Leisure Games", "Big Apple (Leisure Games) (MPU1) (5p Stake, £2 Jackpot)", GAME_FLAGS, layout_m_bappl2 ) // Remade version with different sounds etc.

// license:BSD-3-Clause
// copyright-holders:SomeRandomGuyIdk
/**********************************************************************

    Barcrest MPU1 & MPU2 (1978)

    Barcrest's first fruit machine platform to use a conventional CPU,
    possibly the first of it's kind. Has electromechanical reels
    instead of the usual stepper reels. The hardware features a MC6800
    CPU clocked at around 1 MHz, 128 bytes of RAM and two PIAs for I/O.
    The PIAs can interface with up to 13 lamps, 4 reels, 8 triacs and 12 inputs
    (4 of these being special coin inputs). A 1-bit DAC is used for audio.
    Games are stored on cartridges that can hold up to 4K of data.
    Most cartridges use a common 2K mask ROM, with the remaining 2K
    available for the game program.

    MPU2 seems to have been made alongside MPU1 as a more capable version
    of the platform. Barcrest initially used it for export games
    which required more I/O, possibly UK club machines as well though
    I haven't seen any of those. Regular AWP games adopted MPU2 in 1980,
    when Barcrest started using stepper reels. Compared to MPU1, MPU2
    gained two more PIAs giving 10 extra inputs, 14 lamp outputs and
    a 7-segment display interface supporting up to 8 displays. Stepper games
    used the new lamp outputs for driving reels (which resulted in the drive
    transistors wearing out) and display outputs for driving lamps.
    At some point Barcrest started using a lamp relay to switch
    between two sets of usable lamps. Cartridges were increased
    to 6K ROM capacity, with an optional 256 nibble NVRAM.
    Barcrest also made MPU2 upgrade boards that plugged into a regular
    MPU1 board and made it fully compatible with MPU2 games.

    TODO:
    - Layouts (there currently aren't any for MPU1...)

**********************************************************************/

#include "emu.h"

#include "awpvid.h"
#include "fruitsamples.h"

#include "cpu/m6800/m6800.h"

#include "machine/6821pia.h"
#include "machine/em_reel.h"
#include "machine/nvram.h"
#include "machine/steppers.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "speaker.h"

namespace {

#include "mpu1.lh"
#include "mpu2.lh"
#include "m_bappl2.lh"
#include "m_bapple.lh"
#include "m_gndgit.lh"
#include "m_lndg.lh"
#include "m_mtchit.lh"
#include "m_mtchup.lh"
#include "m2comet.lh"
#include "m2frpoly.lh"
#include "m2hilite.lh"
#include "m2luckyl.lh"
#include "m2luckys.lh"
#include "m2rockon.lh"
#include "m2splite.lh"
#include "m2sstar.lh"
#include "m2starl.lh"
#include "m2svlite.lh"
#include "m2triple.lh"

class mpu12_base_state : public driver_device
{
public:
	DECLARE_INPUT_CHANGED_MEMBER(coin_input);

protected:
	enum class lamp_flags : uint8_t
	{
		IGNORE_RELAY = 0,
		RELAY_OFF = 0x1,
		RELAY_ON = 0x2,
		RELAY_BOTH = 0x3
	};

	mpu12_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nmi_timer(*this, "nmi"),
		m_pia1(*this, "pia1"),
		m_pia2(*this, "pia2"),
		m_lamps(*this, "lamp%u", 0U),
		m_dac(*this, "dac"),
		m_samples(*this, "samples")
	{ }

	void mpu12_base(machine_config &config);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(nmi);
	TIMER_CALLBACK_MEMBER(change_pia2a_bit7);

	void pia_lamp_update(int data) { update_pia_lamps(); }
	virtual void update_pia_lamps() {}
	void set_lamp(bool state, int lamp1, int lamp2 = 0, lamp_flags flags = lamp_flags::IGNORE_RELAY);
	void update_lamp_relay(uint8_t state);
	void payout_cash_w(bool state);
	void payout_token_w(bool state);
	void meter_w(int meter, bool state);
	void meter_tick_w(int meter, bool state);
	void coin_lockout_w(bool state);

	void mpu1_map(address_map &map) ATTR_COLD;

	uint8_t m_lamp_relay;
	lamp_flags m_lamp_flags_pia2b[8];
	lamp_flags m_lamp_flags_pia3b[7];
	lamp_flags m_lamp_flags_pia4b[8];
	lamp_flags m_lamp_flags_pia1cb2;
	lamp_flags m_lamp_flags_pia2ca2;
	lamp_flags m_lamp_flags_pia2cb2;
	lamp_flags m_lamp_flags_pia3ca2;
	lamp_flags m_lamp_flags_pia3cb2;
	lamp_flags m_lamp_flags_pia4ca2;
	lamp_flags m_lamp_flags_pia4cb2;
	bool m_pia2a_select;
	bool m_payout_state[2];
	uint8_t m_pia2a_bit7_value;
	emu_timer *m_change_pia2a_bit7_timer;

	required_device<cpu_device> m_maincpu;
	required_device<timer_device> m_nmi_timer;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	output_finder<56> m_lamps;
	required_device<dac_1bit_device> m_dac;
	required_device<fruit_samples_device> m_samples;
};

class mpu1_state : public mpu12_base_state
{
public:
	mpu1_state(const machine_config &mconfig, device_type type, const char *tag) :
		mpu12_base_state(mconfig, type, tag),
		m_reels(*this, "emreel%u", 1U)
	{ }

	void mpu1(machine_config &config);
	void mpu1_lg(machine_config &config);

protected:
	enum { STEPS_PER_SYMBOL = 20 };

	void add_em_reels(machine_config &config, int symbols, attotime period);
	template <unsigned Reel> void reel_sample_cb(int state);

	void pia1_portb_w(uint8_t data);
	void pia1_portb_lg_w(uint8_t data);
	uint8_t pia2_porta_r();
	void pia2_porta_w(uint8_t data);
	uint8_t reel_pos_r(uint8_t reel);
	virtual void update_pia_lamps() override;

	uint8_t m_reel_select;

	required_device_array<em_reel_device, 4> m_reels;
};

class mpu2_em_state : public mpu1_state
{
public:
	mpu2_em_state(const machine_config &mconfig, device_type type, const char *tag) :
		mpu1_state(mconfig, type, tag),
		m_pia3(*this, "pia3"),
		m_pia4(*this, "pia4"),
		m_disp_persist_timers(*this, "disp_timer%u", 0U),
		m_digits(*this, "digit%u", 0U),
		m_nvram(*this, "nvram", 0x100, ENDIANNESS_BIG)
	{ }

	void mpu2_em(machine_config &config);
	void mpu2_em_sstar(machine_config &config);
	void mpu2_em_starl(machine_config &config);
	void mpu2_em_lg(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;

	template <unsigned Digit> TIMER_DEVICE_CALLBACK_MEMBER(clear_digit) { m_digits[Digit] = 0; };

	void pia1_portb_w(uint8_t data);
	void pia1_portb_sstar_w(uint8_t data);
	void pia1_portb_starl_w(uint8_t data);
	void pia1_portb_lg_w(uint8_t data);
	void pia3_porta_w(uint8_t data);
	void pia3_portb_disp_w(uint8_t data);
	uint8_t nvram_r(offs_t offset) { return m_nvram[offset] & 0xf; }
	void nvram_w(offs_t offset, uint8_t data) { m_nvram[offset] = data & 0xf; }
	virtual void update_pia_lamps() override;

	void mpu2_em_map(address_map &map) ATTR_COLD;

	uint8_t m_disp_digit;

	required_device<pia6821_device> m_pia3;
	required_device<pia6821_device> m_pia4;
	required_device_array<timer_device, 8> m_disp_persist_timers;
	output_finder<8> m_digits;

	memory_share_creator<uint8_t> m_nvram;
};

class mpu2_stepper_state : public mpu12_base_state
{
public:
	mpu2_stepper_state(const machine_config &mconfig, device_type type, const char *tag) :
		mpu12_base_state(mconfig, type, tag),
		m_pia3(*this, "pia3"),
		m_pia4(*this, "pia4"),
		m_reels(*this, "reel%u", 0U)
	{ }

	void mpu2_stepper(machine_config &config);
	void mpu2_stepper_rockon(machine_config &config);
	void mpu2_stepper_hilite(machine_config &config);
	void mpu2_stepper_splite(machine_config &config);
	void mpu2_stepper_frpoly(machine_config &config);
	void mpu2_stepper_triple(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void device_post_load() override;

	template <unsigned N> void opto_cb(int state) { m_optos[N] = state; }

	void pia1_portb_rockon_w(uint8_t data);
	void pia1_portb_hilite_w(uint8_t data);
	void pia1_portb_splite_w(uint8_t data);
	uint8_t pia2_porta_r();
	void pia2_porta_w(uint8_t data) { m_pia2a_select = BIT(data, 7); }
	void pia4_portb_w(uint8_t data);
	void pia3_ca2_w(int state) { m_reel1 = (m_reel1 & ~0x1) | (state ? 0x1 : 0); reel_w(0, m_reel1); }
	void pia3_cb2_w(int state) { m_reel1 = (m_reel1 & ~0x2) | (state ? 0x2 : 0); reel_w(0, m_reel1); }
	void pia4_ca2_w(int state) { m_reel1 = (m_reel1 & ~0x4) | (state ? 0x4 : 0); reel_w(0, m_reel1); }
	void pia4_cb2_w(int state) { m_reel1 = (m_reel1 & ~0x8) | (state ? 0x8 : 0); reel_w(0, m_reel1); }
	void reel_w(int reel, uint8_t data);
	virtual void update_pia_lamps() override;

	void mpu2_stepper_map(address_map &map) ATTR_COLD;

	uint8_t m_optos[3];
	uint8_t m_reel1;

	required_device<pia6821_device> m_pia3;
	required_device<pia6821_device> m_pia4;
	required_device_array<stepper_device, 3> m_reels;
};

void mpu12_base_state::mpu1_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x007f).ram();
	map(0x0800, 0x1fff).rom().mirror(0x6000);
	map(0x2000, 0x2003).rw(m_pia1, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x2004, 0x2007).rw(m_pia2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}

void mpu2_em_state::mpu2_em_map(address_map &map)
{
	mpu1_map(map);

	map(0x2400, 0x24ff).ram().rw(FUNC(mpu2_em_state::nvram_r), FUNC(mpu2_em_state::nvram_w)).share("nvram");
	map(0x4000, 0x4003).rw(m_pia3, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4008, 0x400b).rw(m_pia4, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}

void mpu2_stepper_state::mpu2_stepper_map(address_map &map)
{
	mpu1_map(map);

	map(0x4000, 0x4003).rw(m_pia3, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x4008, 0x400b).rw(m_pia4, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
}

void mpu1_state::pia1_portb_w(uint8_t data)
{
	if(BIT(data, 7) == 0)
	{
		m_reels[0]->set_state(BIT(data, 0));
		m_reels[1]->set_state(BIT(data, 1));
		m_reels[2]->set_state(BIT(data, 2));
		m_reels[3]->set_state(BIT(data, 3));
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
		m_reels[0]->set_state(BIT(data, 0));
		m_reels[1]->set_state(BIT(data, 1));
		m_reels[2]->set_state(BIT(data, 2));
		m_reels[3]->set_state(BIT(data, 3));
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

void mpu2_em_state::pia1_portb_w(uint8_t data)
{
	if(BIT(data, 7) == 0)
	{
		m_reels[0]->set_state(BIT(data, 0));
		m_reels[1]->set_state(BIT(data, 1));
		m_reels[2]->set_state(BIT(data, 2));
		m_reels[3]->set_state(BIT(data, 3));
		coin_lockout_w(BIT(data, 4));
		meter_w(0, BIT(data, 5));
		meter_w(1, BIT(data, 6));
	}
	else
	{
		payout_cash_w(BIT(data, 1));
		payout_token_w(BIT(data, 2));
		meter_w(2, BIT(data, 3));
		meter_w(3, BIT(data, 4));
		meter_w(4, BIT(data, 5));
		meter_w(5, BIT(data, 6));
	}
}

void mpu2_em_state::pia1_portb_sstar_w(uint8_t data)
{
	if(BIT(data, 7) == 0)
	{
		m_reels[0]->set_state(BIT(data, 0));
		m_reels[1]->set_state(BIT(data, 1));
		m_reels[2]->set_state(BIT(data, 2));
		m_reels[3]->set_state(BIT(data, 3));
		coin_lockout_w(BIT(data, 4));
		meter_w(1, BIT(data, 5));
		m_lamps[25] = BIT(data, 6);
	}
	else
	{
		meter_w(0, BIT(data, 3)); // Payout meter
		meter_w(2, BIT(data, 4));
	}
}

void mpu2_em_state::pia1_portb_starl_w(uint8_t data)
{
	if(BIT(data, 7) == 0)
	{
		m_reels[0]->set_state(BIT(data, 0));
		m_reels[1]->set_state(BIT(data, 1));
		m_reels[2]->set_state(BIT(data, 2));
		m_reels[3]->set_state(BIT(data, 3));
		coin_lockout_w(BIT(data, 4));
		meter_w(1, BIT(data, 5));
		m_lamps[25] = BIT(data, 6);
	}
	else
	{
		update_lamp_relay(BIT(data, 0));
		meter_w(0, BIT(data, 3)); // Payout meter
		meter_w(2, BIT(data, 4));
		meter_w(3, BIT(data, 6));
	}
}

void mpu2_em_state::pia1_portb_lg_w(uint8_t data)
{
	if(data == 0)
	{
		for(int i = 0; i < 5; i++) meter_w(i, 0);
		payout_cash_w(0);
		payout_token_w(0);
	}

	if(BIT(data, 7) == 0)
	{
		if(BIT(data, 0))
		{
			for(int i = 0; i < 4; i++)
				m_reels[i]->set_direction(em_reel_device::dir::FORWARD);
		}
		m_reels[1]->set_state(BIT(data, 1));
		m_reels[2]->set_state(BIT(data, 2));
		m_reels[3]->set_state(BIT(data, 3));
		coin_lockout_w(BIT(data, 4));
		if(BIT(data, 5))
		{
			for(int i = 0; i < 4; i++)
				m_reels[i]->set_direction(em_reel_device::dir::REVERSE);
		}
		m_lamps[25] = BIT(data, 6);
	}
	else
	{
		meter_w(0, BIT(data, 0));
		/* m2comet actually differentiates between cash and token wins, but the highest cash win the
		   code allows is 50p. The only cabinet I've seen a photo of says "All wins paid in cash", so
		   presumably machines were modified to fire the cash payout from both the cash and token triacs. */
		payout_cash_w(BIT(data, 1) | BIT(data, 2));
		meter_w(1, BIT(data, 3));
		meter_w(2, BIT(data, 4));
		meter_w(3, BIT(data, 5));
		meter_w(4, BIT(data, 6));
	}
}

void mpu2_stepper_state::pia1_portb_rockon_w(uint8_t data)
{
	if(BIT(data, 7) == 0)
	{
		m_lamps[11] = BIT(data, 0);
		coin_lockout_w(BIT(data, 4));
		meter_tick_w(0, BIT(data, 5));
		meter_tick_w(1, BIT(data, 6));
	}
	else
	{
		payout_cash_w(BIT(data, 1));
		payout_token_w(BIT(data, 2));
		meter_tick_w(2, BIT(data, 3));
		meter_tick_w(3, BIT(data, 4));
		meter_tick_w(4, BIT(data, 5));
		meter_tick_w(5, BIT(data, 6));
	}
}

void mpu2_stepper_state::pia1_portb_hilite_w(uint8_t data)
{
	if(BIT(data, 7) == 0)
	{
		m_lamps[11] = BIT(data, 0);
		m_lamps[12] = BIT(data, 1);
		update_lamp_relay(BIT(data, 2));
		payout_cash_w(BIT(data, 3));
		coin_lockout_w(BIT(data, 4));
		meter_tick_w(0, BIT(data, 5));
		meter_tick_w(1, BIT(data, 6));
	}
	else
	{
		m_lamps[13] = BIT(data, 0);
		m_lamps[14] = BIT(data, 1);
		payout_token_w(BIT(data, 2));
		meter_tick_w(2, BIT(data, 3));
		meter_tick_w(3, BIT(data, 4));
		meter_tick_w(4, BIT(data, 5));
		meter_tick_w(5, BIT(data, 6));
	}
}

void mpu2_stepper_state::pia1_portb_splite_w(uint8_t data)
{
	if(BIT(data, 7) == 0)
	{
		m_lamps[11] = BIT(data, 0);
		m_lamps[12] = BIT(data, 1);
		update_lamp_relay(BIT(data, 2));
		coin_lockout_w(BIT(data, 4));
		meter_tick_w(0, BIT(data, 5));
		meter_tick_w(1, BIT(data, 6));
	}
	else
	{
		m_lamps[13] = BIT(data, 0);
		payout_cash_w(BIT(data, 1));
		payout_token_w(BIT(data, 2));
		meter_tick_w(2, BIT(data, 3));
		meter_tick_w(3, BIT(data, 4));
		meter_tick_w(4, BIT(data, 5));
		meter_tick_w(5, BIT(data, 6));
	}
}

uint8_t mpu1_state::pia2_porta_r()
{
	if(m_pia2a_select == 0)
	{
		return reel_pos_r(m_reel_select);
	}
	else
	{
		// Games won't boot until bit 7 here reads 1, but will only accept coins if it's 0
		return m_pia2a_bit7_value;
	}
}

uint8_t mpu2_stepper_state::pia2_porta_r()
{
	if(m_pia2a_select == 0)
		return (m_optos[0] ? 0x1 : 0) | (m_optos[1] ? 0x2 : 0) | (m_optos[2] ? 0x4 : 0);
	else
		return m_pia2a_bit7_value;
}

void mpu1_state::pia2_porta_w(uint8_t data)
{
	m_pia2a_select = BIT(data, 7);
	if(m_pia2a_select == 0) m_reel_select = (data & 0x60) >> 5;
}

void mpu2_em_state::pia3_porta_w(uint8_t data)
{
	m_disp_digit = data & 0x7;
	m_lamps[15] = BIT(data, 3);
	m_lamps[16] = BIT(data, 4);
}

void mpu2_em_state::pia3_portb_disp_w(uint8_t data)
{
	if(data)
	{
		uint8_t digit = m_disp_digit;
		m_digits[digit] = bitswap<8>(data, 7, 0, 1, 2, 3, 4, 5, 6) & 0x7f;
		/* Displays are switched on one at a time (persistence of vision),
		   timer for each display to crudely simulate the effect. This needs
		   to be simulated, since in order to clear digits, games just stop
		   updating them. */
		m_disp_persist_timers[digit]->adjust(attotime::from_msec(50)); // 3 frames (screenless drivers run at 60 Hz)
	}
}

void mpu2_stepper_state::pia4_portb_w(uint8_t data)
{
	reel_w(1, data & 0xf);
	reel_w(2, (data & 0xf0) >> 4);
}

void mpu12_base_state::payout_cash_w(bool state)
{
	if(!m_payout_state[0] && state)
		m_samples->play(fruit_samples_device::SAMPLE_PAYOUT);
	m_payout_state[0] = state;
}

void mpu12_base_state::payout_token_w(bool state)
{
	if(!m_payout_state[1] && state)
		m_samples->play(fruit_samples_device::SAMPLE_PAYOUT);
	m_payout_state[1] = state;
}

void mpu12_base_state::meter_w(int meter, bool state)
{
	machine().bookkeeping().coin_counter_w(meter, state);
	// Early machines don't have audible meters
}

void mpu12_base_state::meter_tick_w(int meter, bool state)
{
	machine().bookkeeping().coin_counter_w(meter, state);
	if(state) m_samples->play(fruit_samples_device::SAMPLE_METER);
}

void mpu12_base_state::coin_lockout_w(bool state)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
	machine().bookkeeping().coin_lockout_w(1, !state);
	machine().bookkeeping().coin_lockout_w(2, !state);
	machine().bookkeeping().coin_lockout_w(3, !state);
}

void mpu12_base_state::set_lamp(bool state, int lamp1, int lamp2, lamp_flags flags)
{
	/* A lamp output can be powered in one or both of the relay states,
	   or not connected to the relay at all */
	if(flags == lamp_flags::IGNORE_RELAY)
	{
		m_lamps[lamp1] = state;
		return;
	}

	if(!m_lamp_relay)
	{
		if(flags == lamp_flags::RELAY_OFF || flags == lamp_flags::RELAY_BOTH)
			m_lamps[lamp1] = state;
		else
			m_lamps[lamp1] = 0;
		m_lamps[lamp2] = 0;
	}
	else
	{
		if(flags == lamp_flags::RELAY_ON || flags == lamp_flags::RELAY_BOTH)
			m_lamps[lamp2] = state;
		else
			m_lamps[lamp2] = 0;
		m_lamps[lamp1] = 0;
	}
}

void mpu12_base_state::update_lamp_relay(uint8_t state)
{
	if(m_lamp_relay != state)
	{
		m_lamp_relay = state;
		update_pia_lamps();
	}
}

void mpu1_state::update_pia_lamps()
{
	for(int i = 0; i < 8; i++)
		set_lamp(BIT(m_pia2->b_output(), i), i);

	set_lamp(m_pia1->cb2_output(), 8);
	set_lamp(m_pia2->ca2_output(), 9);
	set_lamp(m_pia2->cb2_output(), 10);
}

void mpu2_em_state::update_pia_lamps()
{
	for(int i = 0; i < 8; i++)
		set_lamp(BIT(m_pia2->b_output(), i), i, 33 + i, m_lamp_flags_pia2b[i]);
	for(int i = 0; i < 8; i++)
		set_lamp(BIT(m_pia4->b_output(), i), 17 + i, 48 + i, m_lamp_flags_pia4b[i]);
	for(int i = 0; i < 7; i++) // PIA3 B used by m2comet only, doesn't have relay
		set_lamp(BIT(m_pia3->b_output(), i), 26 + i);

	set_lamp(m_pia1->cb2_output(), 8, 41, m_lamp_flags_pia1cb2);
	set_lamp(m_pia2->ca2_output(), 9, 42, m_lamp_flags_pia2ca2);
	set_lamp(m_pia2->cb2_output(), 10, 43, m_lamp_flags_pia2cb2);
	set_lamp(m_pia3->ca2_output(), 11, 44, m_lamp_flags_pia3ca2);
	set_lamp(m_pia3->cb2_output(), 12, 45, m_lamp_flags_pia3cb2);
	set_lamp(m_pia4->ca2_output(), 13, 46, m_lamp_flags_pia4ca2);
	set_lamp(m_pia4->cb2_output(), 14, 47, m_lamp_flags_pia4cb2);
}

void mpu2_stepper_state::update_pia_lamps()
{
	for(int i = 0; i < 8; i++)
		set_lamp(BIT(m_pia2->b_output(), i), i, 22 + i, m_lamp_flags_pia2b[i]);
	for(int i = 0; i < 7; i++)
		set_lamp(BIT(m_pia3->b_output(), i), 15 + i, 33 + i, m_lamp_flags_pia3b[i]);

	set_lamp(m_pia1->cb2_output(), 8, 30, m_lamp_flags_pia1cb2);
	set_lamp(m_pia2->ca2_output(), 9, 31, m_lamp_flags_pia2ca2);
	set_lamp(m_pia2->cb2_output(), 10, 32, m_lamp_flags_pia2cb2);
}

uint8_t mpu1_state::reel_pos_r(uint8_t reel)
{
	uint16_t const pos = m_reels[reel]->get_pos();

	if(pos % STEPS_PER_SYMBOL == 0)
		return (pos / STEPS_PER_SYMBOL) + 1;
	else
		return 0;
}

void mpu2_stepper_state::reel_w(int reel, uint8_t data)
{
	m_reels[reel]->update(data);
	constexpr char reelnames[3][6] = { "reel1", "reel2", "reel3" };
	awp_draw_reel(machine(), reelnames[reel], *m_reels[reel]);
}

TIMER_DEVICE_CALLBACK_MEMBER( mpu12_base_state::nmi )
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

INPUT_CHANGED_MEMBER( mpu12_base_state::coin_input )
{
	switch(param)
	{
		case 0: m_pia1->ca1_w(newval); break;
		case 1: m_pia1->cb1_w(newval); break;
		case 2: m_pia2->ca1_w(newval); break;
		case 3: m_pia2->cb1_w(newval); break;
	}
}

TIMER_CALLBACK_MEMBER( mpu12_base_state::change_pia2a_bit7 )
{
	m_pia2a_bit7_value = 0;
}

template <unsigned Reel>
void mpu1_state::reel_sample_cb(int state)
{
	if(state == 0)
		m_samples->play(fruit_samples_device::SAMPLE_EM_REEL_1_STOP + Reel);
	else if(state == 1)
		m_samples->play(fruit_samples_device::SAMPLE_EM_REEL_1_START + Reel);
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
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mpu2_inputs )
	PORT_INCLUDE( mpu1_inputs )

	PORT_START("IN_PIA4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN_PIA3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( coin_5p_10p_10pt_50p )
	PORT_MODIFY("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("5p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 0) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 1) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("10p Token") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 2) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 3) PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( coin_10p_10pt_50p )
	PORT_MODIFY("COIN")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 1) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p Token") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 2) PORT_IMPULSE(2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("50p") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 3) PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( coin_dutch )
	PORT_MODIFY("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Fl 0,25") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 0) PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Fl 1") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 1) PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("Fl 2,50") PORT_CHANGED_MEMBER(DEVICE_SELF, mpu1_state, coin_input, 2) PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( m_gndgit )
	PORT_INCLUDE( mpu1_inputs )
	PORT_INCLUDE( coin_5p_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_CONFNAME( 0x01, 0x00, "Token tube" ) // Gamble feature limited to £1 if empty
	PORT_CONFSETTING(    0x00, "Full" )
	PORT_CONFSETTING(    0x01, "Empty" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
INPUT_PORTS_END

static INPUT_PORTS_START( m_mtchit )
	PORT_INCLUDE( mpu1_inputs )
	PORT_INCLUDE( coin_5p_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_CONFNAME( 0x01, 0x00, "Token tube" ) // Gamble feature limited to 50p if empty
	PORT_CONFSETTING(    0x00, "Full" )
	PORT_CONFSETTING(    0x01, "Empty" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Match 1/Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Match 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Match 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
INPUT_PORTS_END

static INPUT_PORTS_START( m_mtchup )
	PORT_INCLUDE( mpu1_inputs )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Match 1/Gamble")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Match 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Match 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
INPUT_PORTS_END

static INPUT_PORTS_START( m_lndg )
	PORT_INCLUDE( mpu1_inputs )
	PORT_INCLUDE( coin_5p_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Respin")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")
INPUT_PORTS_END

static INPUT_PORTS_START( m2comet )
	PORT_INCLUDE( mpu2_inputs )
	PORT_INCLUDE( coin_5p_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Collect")

	PORT_MODIFY("IN_PIA4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Gamble")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Nudge Down")
	PORT_DIPNAME( 0xf0, 0xf0, "Jackpot amount (melons)" ) // Hold & nudge chance decreases with the higher options
	PORT_DIPSETTING(    0xf0, u8"£1" )
	PORT_DIPSETTING(    0xe0, u8"£2" )
	PORT_DIPSETTING(    0xd0, u8"£3" )
	PORT_DIPSETTING(    0xc0, u8"£4" )
	PORT_DIPSETTING(    0xb0, u8"£5" )
INPUT_PORTS_END

static INPUT_PORTS_START( m2rockon )
	PORT_INCLUDE( mpu2_inputs )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_CONFNAME( 0x01, 0x00, "Token tube" ) // Gamble limited to £1 if empty
	PORT_CONFSETTING(    0x00, "Full" )
	PORT_CONFSETTING(    0x01, "Empty" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_MODIFY("IN_PIA4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_INTERLOCK ) PORT_NAME("Back Door") PORT_TOGGLE
	/* Clears credits when activated if door is open. Don't know what this looks like on
	   actual machines if even there, it's not shown on any wiring diagrams. */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Clear Credits")
INPUT_PORTS_END

static INPUT_PORTS_START( m2hilite )
	PORT_INCLUDE( mpu2_inputs )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_CONFNAME( 0x01, 0x00, "Token tube" ) // Gamble limited to £1 if empty
	PORT_CONFSETTING(    0x00, "Full" )
	PORT_CONFSETTING(    0x01, "Empty" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold/Nudge Down 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Hold/Nudge Down 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Hold/Nudge Down 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_MODIFY("IN_PIA4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_INTERLOCK ) PORT_NAME("Back Door") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Nudge Up 3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up 2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up 1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Clear Credits")
INPUT_PORTS_END

static INPUT_PORTS_START( m2svlite )
	PORT_INCLUDE( m2hilite )

	PORT_MODIFY("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // No token tube, always full

	PORT_MODIFY("COIN")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( m2splite )
	PORT_INCLUDE( mpu2_inputs )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_CONFNAME( 0x01, 0x00, "Token tube" ) // Gamble limited to £1 if empty
	PORT_CONFSETTING(    0x00, "Full" )
	PORT_CONFSETTING(    0x01, "Empty" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold/Nudge Down 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Hold/Nudge Down 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Hold/Nudge Down 3")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_MODIFY("IN_PIA4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Auto Nudge")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_INTERLOCK ) PORT_NAME("Back Door") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Nudge Up 3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up 2")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up 1")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Clear Credits")
INPUT_PORTS_END

static INPUT_PORTS_START( m2luckys )
	PORT_INCLUDE( m2splite )

	PORT_MODIFY("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // Token tube switch always full
INPUT_PORTS_END

static INPUT_PORTS_START( m2frpoly )
	PORT_INCLUDE( mpu2_inputs )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold/Nudge Down 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Hold/Nudge Down 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Hold/Nudge Down 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_MODIFY("IN_PIA4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Auto Nudge")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up")
INPUT_PORTS_END

static INPUT_PORTS_START( m2triple )
	PORT_INCLUDE( mpu2_inputs )
	PORT_INCLUDE( coin_10p_10pt_50p )

	PORT_MODIFY("IN")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold/Nudge Down 1")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Hold/Nudge Down 2")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Hold/Nudge Down 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect")

	PORT_MODIFY("IN_PIA4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Auto Nudge")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up")
INPUT_PORTS_END

static INPUT_PORTS_START( m2sstar )
	PORT_INCLUDE( mpu2_inputs )
	PORT_INCLUDE( coin_dutch )

	PORT_MODIFY("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gamble")

	PORT_MODIFY("IN_PIA4")
	PORT_DIPNAME( 0x0f, 0x0f, "Percentage (initial card symbols)" )
	PORT_DIPSETTING(    0x0e, DEF_STR(Very_Low) )
	PORT_DIPSETTING(    0x0d, DEF_STR(Low) )
	PORT_DIPSETTING(    0x0f, DEF_STR(Medium) )
	PORT_DIPSETTING(    0x0b, DEF_STR(High) )
	PORT_DIPSETTING(    0x07, DEF_STR(Very_High) )
	PORT_DIPNAME( 0x10, 0x10, "Attract mode" )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x10, DEF_STR(On) )
	PORT_DIPNAME( 0x20, 0x20, "Accept multiple coins" )
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x20, DEF_STR(Yes) )
INPUT_PORTS_END

static INPUT_PORTS_START( m2starl )
	PORT_INCLUDE( mpu2_inputs )
	PORT_INCLUDE( coin_dutch )

	PORT_MODIFY("IN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Collect")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )

	PORT_MODIFY("IN_PIA4")
	PORT_DIPNAME( 0x0f, 0x0f, "Percentage (initial card symbols)" )
	PORT_DIPSETTING(    0x00, DEF_STR(Very_Low) )
	PORT_DIPSETTING(    0x01, DEF_STR(Low) )
	PORT_DIPSETTING(    0x0f, DEF_STR(Medium) )
	PORT_DIPSETTING(    0x03, DEF_STR(High) )
	PORT_DIPSETTING(    0x07, DEF_STR(Very_High) )
	PORT_DIPNAME( 0x10, 0x10, "Attract mode" )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x10, DEF_STR(On) )
	PORT_DIPNAME( 0x20, 0x20, "Accept multiple coins" )
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x20, DEF_STR(Yes) )
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	/* Weird testing/demo switch. Machine plays like normal when on, but once the switch is set to off,
	   the current state is saved and meters and payouts are disabled. Turning the switch back on loads
	   the previously saved state and the machine plays normally again. Restarting the machine while the
	   switch is off makes it forget it's credits and game state. */
	PORT_DIPNAME( 0x80, 0x80, "Save/load" )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x80, DEF_STR(On) )

	PORT_MODIFY("IN_PIA3")
	PORT_DIPNAME( 0xc0, 0xc0, "Credit limit" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x40, "200" )
	PORT_DIPSETTING(    0xc0, DEF_STR(None) )
INPUT_PORTS_END

static INPUT_PORTS_START( m2jmt9 )
	PORT_INCLUDE( mpu2_inputs )
	PORT_INCLUDE( coin_5p_10p_10pt_50p )
INPUT_PORTS_END

void mpu12_base_state::machine_start()
{
	m_lamps.resolve();

	m_change_pia2a_bit7_timer = timer_alloc(FUNC(mpu1_state::change_pia2a_bit7), this);

	save_item(NAME(m_lamp_relay));

	m_lamp_relay = 0;
}

void mpu2_em_state::machine_start()
{
	mpu12_base_state::machine_start();

	m_digits.resolve();
}

void mpu2_stepper_state::machine_start()
{
	mpu12_base_state::machine_start();

	save_item(NAME(m_reel1));
	save_item(NAME(m_optos));

	m_reel1 = 0;
}

void mpu12_base_state::machine_reset()
{
	// Return PIA2 port A bit 7 as set for 0.1s from boot, then clear it
	m_pia2a_bit7_value = 0x80;
	m_change_pia2a_bit7_timer->adjust(attotime::from_msec(100));
}

void mpu2_stepper_state::device_post_load()
{
	constexpr char reelnames[3][6] = { "reel1", "reel2", "reel3" };
	for(int i = 0; i < 3; i++) awp_draw_reel(machine(), reelnames[i], *m_reels[i]);
}

void mpu12_base_state::mpu12_base(machine_config &config)
{
	M6800(config, m_maincpu, 1'000'000); /* On MPU1, the clock comes from a multivibrator circuit varying somewhere around 1 MHz from
	                                        board to board. This results in for example slightly different sound pitch across machines.
	                                        I've set a stable 1 MHz clock here, which is also the case on MPU2. */
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu12_base_state::mpu1_map);

	TIMER(config, m_nmi_timer).configure_periodic(FUNC(mpu12_base_state::nmi), attotime::from_hz(100)); // From AC zero crossing detector
	m_nmi_timer->set_start_delay(attotime::from_msec(5)); // Don't go to NMI at reset time

	PIA6821(config, m_pia1);
	m_pia1->readpa_handler().set_ioport("IN");
	m_pia1->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia1->ca1_w(0);
	m_pia1->ca2_handler().set(m_dac, FUNC(dac_1bit_device::write));
	m_pia1->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia1->cb1_w(0);
	m_pia1->cb2_handler().set(FUNC(mpu12_base_state::pia_lamp_update));

	PIA6821(config, m_pia2);
	m_pia2->set_port_a_input_overrides_output_mask(0x80);
	m_pia2->irqa_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia2->ca1_w(0);
	m_pia2->ca2_handler().set(FUNC(mpu12_base_state::pia_lamp_update));
	m_pia2->writepb_handler().set(FUNC(mpu12_base_state::pia_lamp_update));
	m_pia2->irqb_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);
	m_pia2->cb1_w(0);
	m_pia2->cb2_handler().set(FUNC(mpu12_base_state::pia_lamp_update));

	SPEAKER(config, "mono").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.25);

	FRUIT_SAMPLES(config, m_samples);

	std::fill(std::begin(m_lamp_flags_pia2b), std::end(m_lamp_flags_pia2b), lamp_flags::RELAY_BOTH);
	std::fill(std::begin(m_lamp_flags_pia3b), std::end(m_lamp_flags_pia3b), lamp_flags::RELAY_BOTH);
	std::fill(std::begin(m_lamp_flags_pia4b), std::end(m_lamp_flags_pia4b), lamp_flags::RELAY_BOTH);
	m_lamp_flags_pia1cb2 = lamp_flags::RELAY_BOTH;
	m_lamp_flags_pia2ca2 = lamp_flags::RELAY_BOTH;
	m_lamp_flags_pia2cb2 = lamp_flags::RELAY_BOTH;
	m_lamp_flags_pia3ca2 = lamp_flags::RELAY_BOTH;
	m_lamp_flags_pia3cb2 = lamp_flags::RELAY_BOTH;
	m_lamp_flags_pia4ca2 = lamp_flags::RELAY_BOTH;
	m_lamp_flags_pia4cb2 = lamp_flags::RELAY_BOTH;

	m_lamp_flags_pia2b[1] = lamp_flags::IGNORE_RELAY; // Hold 1
	m_lamp_flags_pia2b[2] = lamp_flags::IGNORE_RELAY; // Hold 2
	m_lamp_flags_pia2b[3] = lamp_flags::IGNORE_RELAY; // Hold 3
	m_lamp_flags_pia2b[4] = lamp_flags::IGNORE_RELAY; // Start
}

void mpu1_state::add_em_reels(machine_config &config, int symbols, attotime period)
{
	for(int i = 0; i < 4; i++)
	{
		std::set<uint16_t> detents;
		for(int i = 0; i < symbols; i++)
			detents.insert(i * STEPS_PER_SYMBOL);

		EM_REEL(config, m_reels[i], symbols * STEPS_PER_SYMBOL, detents, period);
	}

	m_reels[0]->state_changed_callback().set(FUNC(mpu1_state::reel_sample_cb<0>));
	m_reels[1]->state_changed_callback().set(FUNC(mpu1_state::reel_sample_cb<1>));
	m_reels[2]->state_changed_callback().set(FUNC(mpu1_state::reel_sample_cb<2>));
	m_reels[3]->state_changed_callback().set(FUNC(mpu1_state::reel_sample_cb<3>));

	m_pia2->readpa_handler().set(FUNC(mpu1_state::pia2_porta_r));
	m_pia2->writepa_handler().set(FUNC(mpu1_state::pia2_porta_w));
}

void mpu1_state::mpu1(machine_config &config)
{
	mpu12_base(config);

	m_pia1->writepb_handler().set(FUNC(mpu1_state::pia1_portb_w));

	add_em_reels(config, 20, attotime::from_double(0.8));
}

void mpu1_state::mpu1_lg(machine_config &config)
{
	mpu1(config);

	m_pia1->writepb_handler().set(FUNC(mpu1_state::pia1_portb_lg_w));

	for(int i = 0; i < 4; i++)
		m_reels[i]->set_rotation_period(attotime::from_double(1.04));
}

void mpu2_em_state::mpu2_em(machine_config &config)
{
	mpu12_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu2_em_state::mpu2_em_map);

	m_pia1->writepb_handler().set(FUNC(mpu2_em_state::pia1_portb_w));

	PIA6821(config, m_pia3);
	m_pia3->readpa_handler().set_ioport("IN_PIA3");
	m_pia3->writepa_handler().set(FUNC(mpu2_em_state::pia3_porta_w));
	m_pia3->ca2_handler().set(FUNC(mpu2_em_state::pia_lamp_update));
	m_pia3->writepb_handler().set(FUNC(mpu2_em_state::pia3_portb_disp_w));
	m_pia3->cb2_handler().set(FUNC(mpu2_em_state::pia_lamp_update));

	PIA6821(config, m_pia4);
	m_pia4->readpa_handler().set_ioport("IN_PIA4");
	m_pia4->writepa_handler().set_nop();
	m_pia4->ca2_handler().set(FUNC(mpu2_em_state::pia_lamp_update));
	m_pia4->writepb_handler().set(FUNC(mpu2_em_state::pia_lamp_update));
	m_pia4->cb2_handler().set(FUNC(mpu2_em_state::pia_lamp_update));

	TIMER(config, m_disp_persist_timers[0]).configure_generic(FUNC(mpu2_em_state::clear_digit<0>));
	TIMER(config, m_disp_persist_timers[1]).configure_generic(FUNC(mpu2_em_state::clear_digit<1>));
	TIMER(config, m_disp_persist_timers[2]).configure_generic(FUNC(mpu2_em_state::clear_digit<2>));
	TIMER(config, m_disp_persist_timers[3]).configure_generic(FUNC(mpu2_em_state::clear_digit<3>));
	TIMER(config, m_disp_persist_timers[4]).configure_generic(FUNC(mpu2_em_state::clear_digit<4>));
	TIMER(config, m_disp_persist_timers[5]).configure_generic(FUNC(mpu2_em_state::clear_digit<5>));
	TIMER(config, m_disp_persist_timers[6]).configure_generic(FUNC(mpu2_em_state::clear_digit<6>));
	TIMER(config, m_disp_persist_timers[7]).configure_generic(FUNC(mpu2_em_state::clear_digit<7>));

	add_em_reels(config, 20, attotime::from_double(0.8));
}

void mpu2_em_state::mpu2_em_sstar(machine_config &config)
{
	mpu2_em(config);

	m_pia1->writepb_handler().set(FUNC(mpu2_em_state::pia1_portb_sstar_w));
}

void mpu2_em_state::mpu2_em_starl(machine_config &config)
{
	mpu2_em(config);

	m_pia1->writepb_handler().set(FUNC(mpu2_em_state::pia1_portb_starl_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	m_lamp_flags_pia2b[0] = lamp_flags::IGNORE_RELAY; // Hold 1
	m_lamp_flags_pia2b[6] = lamp_flags::IGNORE_RELAY; // SL 1
	m_lamp_flags_pia2b[7] = lamp_flags::IGNORE_RELAY; // SL 2
	m_lamp_flags_pia1cb2 = lamp_flags::RELAY_OFF; // Card melon
	m_lamp_flags_pia2ca2 = lamp_flags::RELAY_OFF; // Card SL
	m_lamp_flags_pia2cb2 = lamp_flags::RELAY_OFF; // Card plum 2
	m_lamp_flags_pia3ca2 = lamp_flags::RELAY_OFF; // Card grapes 2
	m_lamp_flags_pia3cb2 = lamp_flags::RELAY_OFF; // Card apple 2
}

void mpu2_em_state::mpu2_em_lg(machine_config &config)
{
	mpu2_em(config);

	m_pia1->writepb_handler().set(FUNC(mpu2_em_state::pia1_portb_lg_w));
	m_pia3->writepb_handler().set(FUNC(mpu2_em_state::pia_lamp_update));

	for(int i = 0; i < 4; i++)
		m_reels[i]->set_rotation_period(attotime::from_double(1.04));
}

void mpu2_stepper_state::mpu2_stepper(machine_config &config)
{
	mpu12_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &mpu2_stepper_state::mpu2_stepper_map);

	m_pia2->readpa_handler().set(FUNC(mpu2_stepper_state::pia2_porta_r));
	m_pia2->writepa_handler().set(FUNC(mpu2_stepper_state::pia2_porta_w));

	PIA6821(config, m_pia3);
	m_pia3->ca2_handler().set(FUNC(mpu2_stepper_state::pia3_ca2_w));
	m_pia3->writepb_handler().set(FUNC(mpu2_stepper_state::pia_lamp_update));
	m_pia3->cb2_handler().set(FUNC(mpu2_stepper_state::pia3_cb2_w));

	PIA6821(config, m_pia4);
	m_pia4->readpa_handler().set_ioport("IN_PIA4");
	m_pia4->ca2_handler().set(FUNC(mpu2_stepper_state::pia4_ca2_w));
	m_pia4->writepb_handler().set(FUNC(mpu2_stepper_state::pia4_portb_w));
	m_pia4->cb2_handler().set(FUNC(mpu2_stepper_state::pia4_cb2_w));

	REEL(config, m_reels[0], BARCREST_48STEP_REEL, 3, 5, 0, 0xc, 48*2);
	m_reels[0]->optic_handler().set(FUNC(mpu2_stepper_state::opto_cb<0>));
	REEL(config, m_reels[1], BARCREST_48STEP_REEL, 3, 5, 0, 0xc, 48*2);
	m_reels[1]->optic_handler().set(FUNC(mpu2_stepper_state::opto_cb<1>));
	REEL(config, m_reels[2], BARCREST_48STEP_REEL, 3, 5, 0, 0xc, 48*2);
	m_reels[2]->optic_handler().set(FUNC(mpu2_stepper_state::opto_cb<2>));
}

void mpu2_stepper_state::mpu2_stepper_rockon(machine_config &config)
{
	mpu2_stepper(config);

	m_pia1->writepb_handler().set(FUNC(mpu2_stepper_state::pia1_portb_rockon_w));
}

void mpu2_stepper_state::mpu2_stepper_hilite(machine_config &config)
{
	mpu2_stepper(config);

	m_pia1->writepb_handler().set(FUNC(mpu2_stepper_state::pia1_portb_hilite_w));

	m_lamp_flags_pia2b[0] = lamp_flags::IGNORE_RELAY; // Count 4
	m_lamp_flags_pia1cb2 = lamp_flags::IGNORE_RELAY; // Cash gamble lose
	m_lamp_flags_pia2ca2 = lamp_flags::RELAY_ON; // Nudge gamble collect 10p
	m_lamp_flags_pia2cb2 = lamp_flags::IGNORE_RELAY; // Gamble & collect buttons
	m_lamp_flags_pia3b[6] = lamp_flags::RELAY_OFF; // Cash gamble £2
}

void mpu2_stepper_state::mpu2_stepper_splite(machine_config &config)
{
	mpu2_stepper(config);

	m_pia1->writepb_handler().set(FUNC(mpu2_stepper_state::pia1_portb_splite_w));

	m_lamp_flags_pia2b[0] = lamp_flags::IGNORE_RELAY; // Count 4
	m_lamp_flags_pia2b[5] = lamp_flags::RELAY_OFF; // Count 2
	m_lamp_flags_pia1cb2 = lamp_flags::RELAY_OFF; // Cash gamble lose
	m_lamp_flags_pia2ca2 = lamp_flags::IGNORE_RELAY; // Nudge gamble collect 10p
	m_lamp_flags_pia2cb2 = lamp_flags::IGNORE_RELAY; // Gamble & collect buttons
	m_lamp_flags_pia3b[6] = lamp_flags::RELAY_OFF; // Cash gamble £2
}

void mpu2_stepper_state::mpu2_stepper_frpoly(machine_config &config)
{
	mpu2_stepper(config);

	m_pia1->writepb_handler().set(FUNC(mpu2_stepper_state::pia1_portb_hilite_w));

	m_lamp_flags_pia2cb2 = lamp_flags::IGNORE_RELAY; // Collect
}

void mpu2_stepper_state::mpu2_stepper_triple(machine_config &config)
{
	mpu2_stepper(config);

	m_pia1->writepb_handler().set(FUNC(mpu2_stepper_state::pia1_portb_splite_w));

	m_lamp_flags_pia1cb2 = lamp_flags::IGNORE_RELAY; // Gamble
	m_lamp_flags_pia2cb2 = lamp_flags::IGNORE_RELAY; // Collect
}

// Common mask ROM on most cartridges
#define MPU12_MASKROM \
	ROM_LOAD( "scm39199.bin", 0x1000, 0x800, CRC(198d77ee) SHA1(ef466e539efd6e31c82ef01b09d63b7580f068fe) ) \
	ROM_RELOAD( 0x1800, 0x800 )
	// Barcrest later switched to SL32000 (a Fairchild part)

ROM_START( m_gndgit )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "gndgit.p1", 0x800, 0x200, CRC(4720ba32) SHA1(548e77c801448fff0e8d9026f74a5f7f11ce2ad0) )
	ROM_LOAD( "gndgit.p2", 0xa00, 0x200, CRC(29d44f00) SHA1(611a7151ef0aad17c16b52d0a04436b2e8028f34) )
ROM_END

ROM_START( m_mtchit )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "mtchit.p1", 0x800, 0x200, CRC(a446cee0) SHA1(e4d700361184a7e9a29d9bf517b4b7531a9d5aba) )
	ROM_LOAD( "mtchit.p2", 0xa00, 0x200, CRC(e009faf1) SHA1(d26cd05bda88da4ff56cba00670fa499d0067e81) )
ROM_END

ROM_START( m_mtchup )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "mup2.p1", 0x800, 0x800, CRC(7af45c0a) SHA1(ee11a7173829bf6426c5f278cbe887051d7b2fb8) )
ROM_END

ROM_START( m_lndg )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "ln.p1", 0x800, 0x200, CRC(9649e598) SHA1(3f19fa8354d6eacaaeaa0f939e471844adbf65a6) )
	ROM_LOAD( "ln.p2", 0xa00, 0x200, CRC(70c157f0) SHA1(a47372c72375a94fff3e6f155fc05b89dabe3b1c) )
ROM_END

ROM_START( m_bapple )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "bapple.p1", 0x800, 0x200, CRC(3265ffbf) SHA1(c92d85e094d8f2eb2e2de96f67296f8e14a5d0ff) )
	ROM_LOAD( "bapple.p2", 0xa00, 0x200, CRC(e75d2760) SHA1(abe3e5e3e1bef8e410dc9bac968688ccd4abfcee) )
ROM_END

ROM_START( m_bappl2 )
	ROM_REGION( 0x10000, "maincpu", 0 )

	// Doesn't use the standard mask ROM
	ROM_LOAD( "bappl2.p1", 0x1000, 0x800, CRC(933a7ab4) SHA1(8a33e6bef587d3f6f6228fe15147c9dcef984972) )
	ROM_RELOAD( 0x1800, 0x800 )
ROM_END

ROM_START( m2comet )
	ROM_REGION( 0x10000, "maincpu", 0 )

	// Doesn't use the standard mask ROM
	ROM_LOAD( "cc.p1", 0x0800, 0x800, CRC(c76d8e54) SHA1(8e8e1b3cd9427b5739e3e5f63a1741c26530b2e9) )
	ROM_LOAD( "cc.p2", 0x1000, 0x800, CRC(9b901d52) SHA1(b444f7bb8aa203e7355ab5d36b8e319bc7d66089) )
	ROM_RELOAD( 0x1800, 0x800 )
ROM_END

ROM_START( m2rockon )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "rockon2.p1", 0x0800, 0x800, CRC(47907c53) SHA1(749adc992c2fc9685c229baf294bd2be79e5d269) )
ROM_END

ROM_START( m2hilite )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "hl2.p1", 0x0800, 0x800, CRC(be46ab30) SHA1(54482157e82acc811fc7c1c95d5feacc472b2e10) )
	ROM_LOAD( "hl2.p2", 0x1800, 0x800, CRC(48546c53) SHA1(f50f9b4fa4091510692f08a0d85c80c9803f2657) )
ROM_END

ROM_START( m2luckyl )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "lln_v1.p1", 0x0800, 0x800, CRC(59abe126) SHA1(5ace8b15ab1e9610975ce3f67a8f29a525d07c0f) )
	ROM_LOAD( "lln_v1.p2", 0x1800, 0x800, CRC(ca311afb) SHA1(562160c924722957372681d8bb73f68b5529a08c) )
ROM_END

ROM_START( m2svlite )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "sl.p1", 0x0800, 0x800, CRC(afe04b5a) SHA1(3b3385a9b039992279fda5b87926b5089a448581) )
	ROM_LOAD( "sl.p2", 0x1800, 0x800, CRC(b3b1c3a8) SHA1(b4c1540f39cf27ed1312b00b0c7c1f3028c5ed2c) )
ROM_END

ROM_START( m2splite )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "spl4.p1", 0x0800, 0x800, CRC(12ed200e) SHA1(4a864ab870b9e82009f85060c3e3922159e0a6f0) )
	ROM_LOAD( "spl4.p2", 0x1800, 0x800, CRC(a1aaad88) SHA1(5140af86b305c2e9b4cb3eb8b275b49c1e9c6e5a) )
ROM_END

ROM_START( m2splitea )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "spl7.p1", 0x0800, 0x800, CRC(1ebfabe1) SHA1(15fc6ef1fe57eac9d74021b5eba63f3956b6a4b8) )
	ROM_LOAD( "spl7.p2", 0x1800, 0x800, CRC(ca2191ff) SHA1(54dd6cf2c1c47795a29caf638a3f2c2bf81a011e) )
ROM_END

ROM_START( m2spliteb )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "spl9.p1", 0x0800, 0x800, CRC(6cf53786) SHA1(d433c93a8fdf6686c61e746cd1d9a73f04dc47f1) )
	ROM_LOAD( "spl9.p2", 0x1800, 0x800, CRC(4dcff290) SHA1(73e0d954efa393781ebbce304eca3cba98f8aba7) )
ROM_END

ROM_START( m2luckys )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "lucky_strike_v1.p1", 0x0800, 0x800, CRC(6a729aa9) SHA1(a69b09878696faeb251bbae885841601bd6a0ea1) )
	ROM_LOAD( "lucky_strike_v1.p2", 0x1800, 0x800, CRC(6bb734c8) SHA1(d4cfb32aff7f1902f05afb005c118e9b60262fe2) )
ROM_END

/* m2frpoly and m2triple could have been converted from either a Hi Lights or Spot Light cabinet,
   as detailed in ads. Presumably there were different ROM sets depending on the cabinet, the
   H and S letters on the labels seem to suggest that much. */
ROM_START( m2frpoly )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "fr_h_10p_3_1_411a.p1", 0x0800, 0x800, CRC(7a5806d9) SHA1(149b44598df64cbd599deeb9f0b5aa78fdb9c34b) )
	ROM_LOAD( "fr_h_10p_3_2_411a.p2", 0x1800, 0x800, CRC(8dc72794) SHA1(058e315b4e09844d317ca7937b088d5cb5cbe6de) )
ROM_END

ROM_START( m2triple )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "tc_s1.p1", 0x0800, 0x800, CRC(faafdc77) SHA1(df8e14b478b1dc431441ca4b61ad1e6073d539ab) )
	ROM_LOAD( "tc_s2.p2", 0x1800, 0x800, CRC(2c558f07) SHA1(aaf5dc76f9ce6e476ff64141b62f456ac2fc052c) )
ROM_END

ROM_START( m2sstar )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "s-star.p1", 0x0800, 0x400, CRC(3b655614) SHA1(9ea7f0e9a6174941300adfc96af16612695f7fbe) )
	ROM_LOAD( "s-star.p2", 0x0c00, 0x400, CRC(22053f4d) SHA1(e56062afe097c62633cd8e13068abb33354e015b) )
ROM_END

ROM_START( m2starl )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "dsl07.p1", 0x0800, 0x400, CRC(35265650) SHA1(0f84f6208c197453f1691504d494463855ec8833) )
	ROM_LOAD( "dsl07.p2", 0x1800, 0x800, CRC(76cbce48) SHA1(03a5959dad8892118d91d3c89385f8ccd402eb24) )
ROM_END

ROM_START( m2jmt9 )
	ROM_REGION( 0x10000, "maincpu", 0 )

	MPU12_MASKROM
	ROM_LOAD( "jmt9.p1", 0x0800, 0x200, CRC(4f547cf6) SHA1(91783769363fa60cabb408895661c1a6d4896477) )
	ROM_LOAD( "jmt9.p2", 0x0a00, 0x200, CRC(bfaed4e7) SHA1(0c1685e40a5cbfbbfbc791789455390f85e8019a) )
ROM_END

} // anonymous namespace

#define GAME_FLAGS MACHINE_NOT_WORKING|MACHINE_MECHANICAL|MACHINE_REQUIRES_ARTWORK|MACHINE_SUPPORTS_SAVE

// AWP
GAMEL(1978,  m_gndgit,  0,        mpu1,                 m_gndgit,  mpu1_state, empty_init, ROT0, "Barcrest", u8"Golden Nudge It (Barcrest) (MPU1) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m_gndgit )
GAMEL(1979,  m_mtchit,  0,        mpu1,                 m_mtchit,  mpu1_state, empty_init, ROT0, "Barcrest", u8"Match It (Barcrest) (MPU1) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m_mtchit )
GAMEL(1981,  m_mtchup,  0,        mpu1,                 m_mtchup,  mpu1_state, empty_init, ROT0, "Barcrest", u8"Match Up (Barcrest) (MPU1) (10p Stake, £2 Jackpot)", GAME_FLAGS, layout_m_mtchup )
GAMEL(1980?, m_lndg,    0,        mpu1_lg,              m_lndg,    mpu1_state, empty_init, ROT0, "Leisure Games", u8"Lucky Nudge (Leisure Games) (MPU1) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m_lndg )
GAMEL(1980?, m_bapple,  0,        mpu1_lg,              m_lndg,    mpu1_state, empty_init, ROT0, "Leisure Games", u8"Big Apple (Leisure Games) (MPU1) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m_bapple )
GAMEL(1981?, m_bappl2,  0,        mpu1_lg,              m_lndg,    mpu1_state, empty_init, ROT0, "Leisure Games", u8"Big Apple (Leisure Games) (MPU1) (5p Stake, £2 Jackpot)", GAME_FLAGS, layout_m_bappl2 ) // Remade version with different sounds etc.
GAMEL(1981?, m2comet,   0,        mpu2_em_lg,           m2comet,   mpu2_em_state, empty_init, ROT0, "Leisure Games", u8"Comet Catcher (Leisure Games) (MPU2) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m2comet )
GAMEL(1981,  m2rockon,  0,        mpu2_stepper_rockon,  m2rockon,  mpu2_stepper_state, empty_init, ROT0, "Barcrest", u8"Rock On (Barcrest) (MPU2) (10p Stake, £2 Jackpot)", GAME_FLAGS, layout_m2rockon )
GAMEL(1981,  m2hilite,  0,        mpu2_stepper_hilite,  m2hilite,  mpu2_stepper_state, empty_init, ROT0, "Barcrest", u8"Hi Lights (Barcrest) (MPU2) (10p Stake, £2 Jackpot)", GAME_FLAGS, layout_m2hilite )
GAMEL(198?,  m2luckyl,  m2hilite, mpu2_stepper_hilite,  m2hilite,  mpu2_stepper_state, empty_init, ROT0, "Leisure Games", u8"Lucky Line (Leisure Games) (MPU2) (5p Stake, £2 Jackpot)", GAME_FLAGS, layout_m2luckyl ) // 5p stake rebuild of Hi Lights
GAMEL(198?,  m2svlite,  m2hilite, mpu2_stepper_hilite,  m2svlite,  mpu2_stepper_state, empty_init, ROT0, "<unknown>", u8"Silver Lights (MPU2) (5p Stake, £1 Jackpot)", GAME_FLAGS, layout_m2svlite ) // £1/5p rebuild of Hi Lights
GAMEL(1981,  m2splite,  0,        mpu2_stepper_splite,  m2splite,  mpu2_stepper_state, empty_init, ROT0, "Barcrest", u8"Spot Light (Barcrest) (MPU2) (revision 4, 10p Stake, £2 Jackpot)", GAME_FLAGS, layout_m2splite )
GAMEL(1981,  m2splitea, m2splite, mpu2_stepper_splite,  m2splite,  mpu2_stepper_state, empty_init, ROT0, "Barcrest", u8"Spot Light (Barcrest) (MPU2) (revision 7, 10p Stake, £2 Jackpot)", GAME_FLAGS, layout_m2splite )
GAMEL(1981,  m2spliteb, m2splite, mpu2_stepper_splite,  m2splite,  mpu2_stepper_state, empty_init, ROT0, "Barcrest", u8"Spot Light (Barcrest) (MPU2) (revision 9, 10p Stake, £2 Jackpot)", GAME_FLAGS, layout_m2splite )
GAMEL(198?,  m2luckys,  m2splite, mpu2_stepper_splite,  m2luckys,  mpu2_stepper_state, empty_init, ROT0, "Associated Leisure", u8"Lucky Strike (Associated Leisure) (MPU2) (5p Stake, £2 Jackpot)", GAME_FLAGS, layout_m2luckys ) // 5p stake rebuild of Spot Light
GAMEL(1984,  m2frpoly,  0,        mpu2_stepper_frpoly,  m2frpoly,  mpu2_stepper_state, empty_init, ROT0, "VFS", u8"Fruitopoly (VFS) (MPU2) (Hi Lights conversion, 10p Stake, £3 Jackpot)", GAME_FLAGS, layout_m2frpoly )
GAMEL(1984,  m2triple,  0,        mpu2_stepper_triple,  m2triple,  mpu2_stepper_state, empty_init, ROT0, "VFS", u8"Triple Chance (VFS) (MPU2) (Spot Light conversion, 2p Stake, £1.50 Jackpot)", GAME_FLAGS, layout_m2triple )
// Dutch
GAMEL(1978,  m2sstar,   0,        mpu2_em_sstar,        m2sstar,   mpu2_em_state, empty_init, ROT0, "Barcrest", "Super Star (Dutch) (Barcrest) (MPU2)", GAME_FLAGS, layout_m2sstar )
GAMEL(1980?, m2starl,   0,        mpu2_em_starl,        m2starl,   mpu2_em_state, empty_init, ROT0, "Barcrest", "Star Light (Dutch) (Barcrest) (MPU2) (revision 07)", GAME_FLAGS, layout_m2starl )
// Misc
GAMEL(197?,  m2jmt9,    0,        mpu2_em,              m2jmt9,    mpu2_em_state, empty_init, ROT0, "Barcrest", "JMT9 Test Program (Barcrest) (MPU1/2)", GAME_FLAGS, layout_mpu2 )

// license:BSD-3-Clause
// copyright-holders:David Haywood, SomeRandomGuyIdk
/**********************************************************************
	
	JPM Stepper Reel Unit
	
	JPM's first CPU-based fruit machine platform, from late 1978/1979.
	Notably the first system to use stepper reels instead of EM reels.
	Uses a 1.5MHz TMS9980A CPU together with some TTL for I/O providing
	56 outputs (16 used by reels) & 24 inputs (8 used by optos), 
	a ROM card holding game ROMs, and a selection of expansion boards. 
	Sound is output by a simple 6-tone NE556-based circuit.
	
	TODO:
	- Layouts
	- Netlist audio works but isn't quite right. The tone pot needs to
	  be set to 17% for it to not cut out, and even then popping can be heard.
	  Needs a look from someone with more analog knowledge than me.
	- Add remaining games
	
	Expansion boards:
	Bus Extension
	Optional board with 128 nibbles NVRAM and 24 DIP switches, only supported by early JPM games
	
	Input Extension
	Same as Bus Extension except with 8 extra inputs replacing the NVRAM (moved to ROM card), used by club games
	
	Logic Extension
	56 extra outputs addressed from CRU memory, accessed via 9 existing outputs
	
	Maxi Logic Extension
	64 extra outputs addressed from CRU memory, accessed via 9 existing outputs
	
	Mini Logic Extension
	16 extra outputs addressed directly from CRU memory
	
	Output Extension
	16 extra outputs addressed from main memory
	
	ROM cards:
	Most SRU games used a 3K ROM card for storage. A few later games had a 4K card,
	and a 6K card with 512 nibbles of NVRAM was used for club games.
	
**********************************************************************/

#include "emu.h"

#include "audio/fruitsamples.h"
#include "audio/nl_jpmsru.h"
#include "video/awpvid.h"

#include "cpu/tms9900/tms9980a.h"

#include "machine/netlist.h"
#include "machine/nvram.h"
#include "machine/steppers.h"
#include "machine/timer.h"

#include "speaker.h"

#include "netlist/nl_setup.h"

#include "jpmsru.lh"
#include "j_ewn.lh"
#include "j_ndu.lh"
#include "j_dud.lh"
#include "j_lan.lh"

#define MAIN_CLOCK 6_MHz_XTAL

class jpmsru_state : public driver_device
{
public:
	jpmsru_state(const machine_config &mconfig, device_type type, const char *tag) : 
			driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_inputs(*this, "IN%u", 0U),
			m_reel(*this, "reel%u", 0U),
			m_lamp(*this, "lamp%u", 0U),
			m_digits(*this, "digit%u", 0U),
			m_audio_in(*this, "nl_audio:in%u", 0U),
			m_samples(*this, "samples"),
			m_nvram(*this, "nvram", 0x80, ENDIANNESS_BIG),
			m_dips(*this, "DIP%u", 0U)
	{ }

	void jpmsru_3k(machine_config &config);
	void jpmsru_3k_busext(machine_config &config);
	void jpmsru_4k(machine_config &config);
	void ewn(machine_config &config);
	void ewn2(machine_config &config);
	void ndu(machine_config &config);
	void dud(machine_config &config);
	void lan(machine_config &config);

	void init_jpmsru();
	
	template <unsigned N> DECLARE_READ_LINE_MEMBER(opto_r) { return m_opto[N]; }
protected:
	virtual void machine_start() override;

private:
	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(opto_cb) { m_opto[N] = state; }

	uint8_t inputs_r(offs_t offset);
	void reel_w(offs_t offset, uint8_t data);
	void update_int();
	void audio_w(offs_t offset, uint8_t data);
	void int1_en_w(offs_t offset, uint8_t data);
	void int2_en_w(offs_t offset, uint8_t data);
	uint8_t busext_data_r(offs_t offset);
	void busext_data_w(offs_t offset, uint8_t data);
	void busext_bdir_w(offs_t offset, uint8_t data);
	void busext_mode_w(offs_t offset, uint8_t data);
	void busext_addr_w(offs_t offset, uint8_t data);
	uint8_t busext_dips_r(offs_t offset);
	void out_lamp_w(offs_t offset, uint8_t data);
	void out_lamp_ext_w(offs_t offset, uint8_t data);
	void out_disp_w(offs_t offset, uint8_t data);
	void out_payout_cash_w(offs_t offset, uint8_t data);
	void out_payout_token_w(offs_t offset, uint8_t data);
	template<unsigned Meter> void out_meter_w(offs_t offset, uint8_t data);
	void out_coin_lockout_w(offs_t offset, uint8_t data);

	void jpmsru_3k_map(address_map &map);
	void jpmsru_4k_map(address_map &map);
	void jpmsru_io(address_map &map);
	void jpmsru_busext_io(address_map &map);
	void outputs_ewn(address_map &map);
	void outputs_ewn2(address_map &map);
	void outputs_ndu(address_map &map);
	void outputs_dud(address_map &map);
	void outputs_lan(address_map &map);
	
	bool m_int1;
	bool m_int2;
	bool m_int1_en;
	bool m_int2_en;
	int m_reelbits[4];
	bool m_opto[4];
	int m_disp_digit;
	bool m_disp_d1;
	bool m_disp_d2;
	bool m_busext_bdir;
	uint8_t m_busext_mode;
	uint8_t m_busext_addr;
	
	TIMER_DEVICE_CALLBACK_MEMBER(int1);
	TIMER_DEVICE_CALLBACK_MEMBER(int2);

	// devices
	required_device<cpu_device> m_maincpu;
	required_ioport_array<3> m_inputs;
	required_device_array<stepper_device, 4> m_reel;
	output_finder<56> m_lamp;
	output_finder<2> m_digits;
	required_device_array<netlist_mame_logic_input_device, 6> m_audio_in;
	required_device<fruit_samples_device> m_samples;
	
	memory_share_creator<uint8_t> m_nvram;
	optional_ioport_array<3> m_dips;
};

void jpmsru_state::jpmsru_3k_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x0e00, 0x0eff).ram();
	/* Some sort of peculiar data logging system used by later JPM games. 
       It consists of 32 bytes of memory where games write various statistics 
       (total plays, win amount, win symbol, gamble win/lose etc.) either as numeric values 
       or ASCII text. Most likely for JPM internal use only. */
	map(0x1400, 0x141f).ram();
}

void jpmsru_state::jpmsru_4k_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0eff).ram();
	map(0x0f00, 0x0fff).rom();
}

void jpmsru_state::jpmsru_io(address_map &map)
{
	map(0x00, 0x2f).r(FUNC(jpmsru_state::inputs_r));
	map(0x00, 0x1f).w(FUNC(jpmsru_state::reel_w));
	map(0x70, 0x7b).w(FUNC(jpmsru_state::audio_w));
	map(0x7c, 0x7d).w(FUNC(jpmsru_state::int2_en_w));
	map(0x7e, 0x7f).w(FUNC(jpmsru_state::int1_en_w));
	// Outputs, all lamps by default
	map(0x20, 0x6f).w(FUNC(jpmsru_state::out_lamp_w));
}

void jpmsru_state::jpmsru_busext_io(address_map &map)
{
	jpmsru_io(map);
	
	map(0x80, 0x87).rw(FUNC(jpmsru_state::busext_data_r), FUNC(jpmsru_state::busext_data_w));
	map(0x88, 0x89).w(FUNC(jpmsru_state::busext_bdir_w));
	map(0x8a, 0x8d).w(FUNC(jpmsru_state::busext_mode_w));
	map(0x90, 0x9d).w(FUNC(jpmsru_state::busext_addr_w));
	map(0x90, 0xbf).r(FUNC(jpmsru_state::busext_dips_r));
}

// CRU maps for each game
void jpmsru_state::outputs_ewn(address_map &map)
{
	jpmsru_busext_io(map);
	
	map(0x3a, 0x3b).w(FUNC(jpmsru_state::out_meter_w<0>));
	map(0x3c, 0x3d).w(FUNC(jpmsru_state::out_meter_w<1>));
	map(0x3e, 0x3f).w(FUNC(jpmsru_state::out_meter_w<2>));
	map(0x40, 0x41).w(FUNC(jpmsru_state::out_meter_w<3>));
	map(0x42, 0x4d).w(FUNC(jpmsru_state::out_disp_w));
	map(0x4e, 0x4f).w(FUNC(jpmsru_state::out_meter_w<4>));
	map(0x50, 0x51).w(FUNC(jpmsru_state::out_meter_w<5>));
	map(0x6a, 0x6b).w(FUNC(jpmsru_state::out_payout_cash_w));
	map(0x6c, 0x6d).w(FUNC(jpmsru_state::out_payout_token_w));
	map(0x6e, 0x6f).w(FUNC(jpmsru_state::out_coin_lockout_w));
}

void jpmsru_state::outputs_ewn2(address_map &map)
{
	jpmsru_io(map);
	
	map(0x3a, 0x3b).w(FUNC(jpmsru_state::out_meter_w<0>));
	map(0x3c, 0x3d).w(FUNC(jpmsru_state::out_meter_w<1>));
	map(0x3e, 0x3f).w(FUNC(jpmsru_state::out_meter_w<2>));
	map(0x40, 0x41).w(FUNC(jpmsru_state::out_meter_w<3>));
	map(0x42, 0x4d).w(FUNC(jpmsru_state::out_disp_w));
	map(0x4e, 0x4f).w(FUNC(jpmsru_state::out_meter_w<4>));
	map(0x50, 0x51).w(FUNC(jpmsru_state::out_meter_w<5>));
	map(0x6a, 0x6b).w(FUNC(jpmsru_state::out_payout_cash_w));
	map(0x6c, 0x6d).w(FUNC(jpmsru_state::out_payout_token_w));
	map(0x6e, 0x6f).w(FUNC(jpmsru_state::out_coin_lockout_w));
}

void jpmsru_state::outputs_ndu(address_map &map)
{
	jpmsru_busext_io(map);
	
	map(0x3a, 0x3b).w(FUNC(jpmsru_state::out_meter_w<0>));
	map(0x3c, 0x3d).w(FUNC(jpmsru_state::out_meter_w<1>));
	map(0x3e, 0x3f).w(FUNC(jpmsru_state::out_meter_w<2>));
	map(0x40, 0x41).w(FUNC(jpmsru_state::out_meter_w<3>));
	map(0x42, 0x4d).w(FUNC(jpmsru_state::out_disp_w));
	map(0x4e, 0x4f).w(FUNC(jpmsru_state::out_meter_w<4>));
	map(0x50, 0x51).w(FUNC(jpmsru_state::out_meter_w<5>));
	map(0x64, 0x65).w(FUNC(jpmsru_state::out_meter_w<6>));
	map(0x6a, 0x6b).w(FUNC(jpmsru_state::out_payout_cash_w));
	map(0x6c, 0x6d).w(FUNC(jpmsru_state::out_payout_token_w));
	map(0x6e, 0x6f).w(FUNC(jpmsru_state::out_coin_lockout_w));
}

void jpmsru_state::outputs_dud(address_map &map)
{
	jpmsru_io(map);
	
	map(0x3a, 0x3b).w(FUNC(jpmsru_state::out_meter_w<0>));
	map(0x3c, 0x3d).w(FUNC(jpmsru_state::out_meter_w<1>));
	map(0x3e, 0x3f).w(FUNC(jpmsru_state::out_meter_w<2>));
	map(0x40, 0x41).w(FUNC(jpmsru_state::out_meter_w<3>));
	map(0x42, 0x4d).w(FUNC(jpmsru_state::out_disp_w));
	map(0x4e, 0x4f).w(FUNC(jpmsru_state::out_meter_w<4>));
	map(0x50, 0x51).w(FUNC(jpmsru_state::out_meter_w<5>));
	map(0x64, 0x65).w(FUNC(jpmsru_state::out_meter_w<6>));
	map(0x6a, 0x6b).w(FUNC(jpmsru_state::out_payout_cash_w));
	map(0x6c, 0x6d).w(FUNC(jpmsru_state::out_payout_token_w));
	map(0x6e, 0x6f).w(FUNC(jpmsru_state::out_coin_lockout_w));
}

void jpmsru_state::outputs_lan(address_map &map)
{
	jpmsru_io(map);
	map(0x38, 0x39).w(FUNC(jpmsru_state::out_meter_w<0>));
	map(0x3a, 0x3b).w(FUNC(jpmsru_state::out_meter_w<1>));
	map(0x3c, 0x3d).w(FUNC(jpmsru_state::out_meter_w<2>));
	map(0x3e, 0x3f).w(FUNC(jpmsru_state::out_meter_w<3>));
	map(0x40, 0x41).w(FUNC(jpmsru_state::out_meter_w<4>));
	map(0x42, 0x4d).w(FUNC(jpmsru_state::out_disp_w));
	map(0x4e, 0x4f).w(FUNC(jpmsru_state::out_meter_w<5>));
	map(0x50, 0x51).w(FUNC(jpmsru_state::out_meter_w<6>));
	map(0x6a, 0x6b).w(FUNC(jpmsru_state::out_payout_cash_w));
	map(0x6c, 0x6d).w(FUNC(jpmsru_state::out_payout_token_w));
	map(0x6e, 0x6f).w(FUNC(jpmsru_state::out_coin_lockout_w));
	// Mini Logic Extension outputs, used for extra lamps
	map(0x80, 0x9f).w(FUNC(jpmsru_state::out_lamp_ext_w));
}

uint8_t jpmsru_state::inputs_r(offs_t offset)
{
	return BIT(m_inputs[(offset & 0x18) >> 3]->read(), offset & 0x7);
}

void jpmsru_state::reel_w(offs_t offset, uint8_t data)
{
	const int reel = (offset & 0xc) >> 2;
	const int bit = offset & 0x3;
	m_reelbits[reel] = (m_reelbits[reel] & ~(1 << bit)) | (data ? (1 << bit) : 0);
	
	if(bit == 3) 
	{
		m_reel[reel]->update(m_reelbits[reel]);
		const char reelnames[4][6] = { "reel1", "reel2", "reel3", "reel4" };
		awp_draw_reel(machine(), reelnames[reel], *m_reel[reel]);
	}
}

void jpmsru_state::out_lamp_w(offs_t offset, uint8_t data)
{
	m_lamp[offset] = data;
}

void jpmsru_state::out_lamp_ext_w(offs_t offset, uint8_t data)
{
	// Extra lamps beyond the stock 40 outputs
	m_lamp[offset + 40] = data;
}

void jpmsru_state::out_disp_w(offs_t offset, uint8_t data)
{
	switch(offset) 
	{
		case 0: m_disp_digit = (m_disp_digit & ~0x01) | (data ? 0x00 : 0x01); break;
		case 1: m_disp_digit = (m_disp_digit & ~0x02) | (data ? 0x00 : 0x02); break;
		case 2: m_disp_digit = (m_disp_digit & ~0x04) | (data ? 0x00 : 0x04); break;
		case 3: m_disp_digit = (m_disp_digit & ~0x08) | (data ? 0x00 : 0x08); break;
		case 4: m_disp_d1 = data; break;
		case 5: m_disp_d2 = data; break;
	}
	
	static constexpr uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 };
	if(m_disp_d1) m_digits[1] = patterns[m_disp_digit];
	if(m_disp_d2) m_digits[0] = patterns[m_disp_digit];
}

template<unsigned Meter>
void jpmsru_state::out_meter_w(offs_t offset, uint8_t data)
{
	machine().bookkeeping().coin_counter_w(Meter, data);
	// SRU doesn't have audible meters
}

void jpmsru_state::out_payout_cash_w(offs_t offset, uint8_t data)
{
	if(data) m_samples->play(fruit_samples_device::SAMPLE_PAYOUT);
}

void jpmsru_state::out_payout_token_w(offs_t offset, uint8_t data)
{
	if(data) m_samples->play(fruit_samples_device::SAMPLE_PAYOUT);
}

void jpmsru_state::out_coin_lockout_w(offs_t offset, uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, !data);
	machine().bookkeeping().coin_lockout_w(1, !data);
	machine().bookkeeping().coin_lockout_w(2, !data);
	machine().bookkeeping().coin_lockout_w(3, !data);
}

void jpmsru_state::audio_w(offs_t offset, uint8_t data)
{
	m_audio_in[offset]->write(data);
}

void jpmsru_state::update_int()
{
	// 74148 priority encoder
	if(m_int1)
		m_maincpu->set_input_line(INT_9980A_LEVEL1, ASSERT_LINE);
	else if(m_int2)
		m_maincpu->set_input_line(INT_9980A_LEVEL2, ASSERT_LINE);
	else
		m_maincpu->set_input_line(INT_9980A_CLEAR, CLEAR_LINE);
}

void jpmsru_state::int1_en_w(offs_t offset, uint8_t data)
{
	if(m_int1_en && !data) m_int1 = 0; // Acknowledge on high->low transition
	m_int1_en = data;
	
	update_int();
}

void jpmsru_state::int2_en_w(offs_t offset, uint8_t data)
{
	if(m_int2_en && !data) m_int2 = 0; // Acknowledge on high->low transition
	m_int2_en = data;
	
	update_int();
}

uint8_t jpmsru_state::busext_data_r(offs_t offset)
{
	if(m_busext_mode == 3 && m_busext_bdir == 1)
	{
		return (m_nvram[m_busext_addr] >> offset) & 1;
	}
	return 0;
}

void jpmsru_state::busext_data_w(offs_t offset, uint8_t data)
{
	if(m_busext_mode == 2 && m_busext_bdir == 0)
	{
		m_nvram[m_busext_addr] = (m_nvram[m_busext_addr] & ~(1 << offset)) | (data ? (1 << offset) : 0);
	}
}

void jpmsru_state::busext_bdir_w(offs_t offset, uint8_t data)
{
	m_busext_bdir = data;
	return;
}

void jpmsru_state::busext_mode_w(offs_t offset, uint8_t data)
{
	m_busext_mode = (m_busext_mode & ~(2 >> offset)) | (data ? (2 >> offset) : 0);
	return;
}

void jpmsru_state::busext_addr_w(offs_t offset, uint8_t data)
{
	m_busext_addr = (m_busext_addr & ~(1 << offset)) | (data ? (1 << offset) : 0);
	return;
}

uint8_t jpmsru_state::busext_dips_r(offs_t offset)
{
	return BIT(m_dips[(offset & 0x18) >> 3]->read(), offset & 0x7);
}

TIMER_DEVICE_CALLBACK_MEMBER(jpmsru_state::int1)
{
	if(m_int1_en) 
	{
		m_int1 = 1;
		update_int();
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(jpmsru_state::int2)
{
	if(m_int2_en) 
	{
		m_int2 = 1;
		update_int();
	}
}

static INPUT_PORTS_START( jpmsru_inputs )
	PORT_START("IN0")
	// Optos
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jpmsru_state, opto_r<0>)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jpmsru_state, opto_r<1>)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jpmsru_state, opto_r<2>)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(jpmsru_state, opto_r<3>)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	// TTL inputs
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )
	
	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Self Test")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("5p")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("10p Token")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p")
	
	PORT_START("POT")
	PORT_ADJUSTER( 50, "Tone Pot" )  NETLIST_ANALOG_PORT_CHANGED("nl_audio", "pot")
INPUT_PORTS_END

static INPUT_PORTS_START( j_ewn )
	PORT_INCLUDE( jpmsru_inputs )
	
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Down")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")
	
	PORT_START("DIP0")
	PORT_DIPNAME( 0x0f, 0x00, "Nudge chance" )
	PORT_DIPSETTING (   0x0f, "1%" )
	PORT_DIPSETTING (   0x0e, "2%" )
	PORT_DIPSETTING (   0x0d, "3%" )
	PORT_DIPSETTING (   0x0c, "4%" )
	PORT_DIPSETTING (   0x0b, "5%" )
	PORT_DIPSETTING (   0x0a, "6%" )
	PORT_DIPSETTING (   0x09, "7%" )
	PORT_DIPSETTING (   0x00, "8%" )
	PORT_DIPSETTING (   0x01, "9%" )
	PORT_DIPSETTING (   0x02, "10%" )
	PORT_DIPSETTING (   0x03, "11%" )
	PORT_DIPSETTING (   0x04, "12%" )
	PORT_DIPSETTING (   0x05, "13%" )
	PORT_DIPSETTING (   0x06, "14%" )
	PORT_DIPSETTING (   0x07, "15%" )
	PORT_DIPNAME( 0xf0, 0x00, "Win hold chance" )
	PORT_DIPSETTING (   0xf0, "17%" )
	PORT_DIPSETTING (   0xe0, "19%" )
	PORT_DIPSETTING (   0xd0, "21%" )
	PORT_DIPSETTING (   0xc0, "23%" )
	PORT_DIPSETTING (   0xb0, "25%" )
	PORT_DIPSETTING (   0xa0, "27%" )
	PORT_DIPSETTING (   0x90, "29%" )
	PORT_DIPSETTING (   0x00, "31%" )
	PORT_DIPSETTING (   0x10, "33%" )
	PORT_DIPSETTING (   0x20, "35%" )
	PORT_DIPSETTING (   0x30, "37%" )
	PORT_DIPSETTING (   0x40, "39%" )
	PORT_DIPSETTING (   0x50, "41%" )
	PORT_DIPSETTING (   0x60, "43%" )
	PORT_DIPSETTING (   0x70, "45%" )
	
	PORT_START("DIP1")
	PORT_DIPNAME( 0x0f, 0x00, "Hold chance" )
	PORT_DIPSETTING (   0x0f, "17%" )
	PORT_DIPSETTING (   0x0e, "19%" )
	PORT_DIPSETTING (   0x0d, "21%" )
	PORT_DIPSETTING (   0x0c, "23%" )
	PORT_DIPSETTING (   0x0b, "25%" )
	PORT_DIPSETTING (   0x0a, "27%" )
	PORT_DIPSETTING (   0x09, "29%" )
	PORT_DIPSETTING (   0x00, "31%" )
	PORT_DIPSETTING (   0x01, "33%" )
	PORT_DIPSETTING (   0x02, "35%" )
	PORT_DIPSETTING (   0x03, "37%" )
	PORT_DIPSETTING (   0x04, "39%" )
	PORT_DIPSETTING (   0x05, "41%" )
	PORT_DIPSETTING (   0x06, "43%" )
	PORT_DIPSETTING (   0x07, "45%" )
	PORT_DIPNAME( 0x10, 0x00, "Store credits" )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x10, DEF_STR(On) )
	PORT_DIPNAME( 0x20, 0x00, "Use default hold/nudge chance" ) // 31% and 8%
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, DEF_STR(On) )
	PORT_DIPUNUSED( 0x40, 0x00 )
	PORT_DIPUNUSED( 0x80, 0x00 )
	
	PORT_START("DIP2")
	PORT_DIPUNUSED( 0x01, 0x00 )
	PORT_DIPUNUSED( 0x02, 0x00 )
	PORT_DIPUNUSED( 0x04, 0x00 )
	PORT_DIPUNUSED( 0x08, 0x00 )
	PORT_DIPUNUSED( 0x10, 0x00 )
	PORT_DIPUNUSED( 0x20, 0x00 )
	PORT_DIPUNUSED( 0x40, 0x00 )
	PORT_DIPUNUSED( 0x80, 0x00 )
INPUT_PORTS_END

static INPUT_PORTS_START( j_ewn2 )
	PORT_INCLUDE( jpmsru_inputs )
	
	PORT_MODIFY("IN0")
	PORT_CONFNAME( 0x80, 0x80, "5p/10p jumper" )
	PORT_CONFSETTING(	 0x00, "5p" )
	PORT_CONFSETTING(	 0x80, "10p" )
	
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Down")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")
INPUT_PORTS_END

static INPUT_PORTS_START( j_ndu )
	PORT_INCLUDE( jpmsru_inputs )
	
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Down")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")
	
	PORT_START("DIP0")
	PORT_DIPNAME( 0x0f, 0x00, "Nudge chance" )
	PORT_DIPSETTING (   0x0f, "1%" )
	PORT_DIPSETTING (   0x0e, "2%" )
	PORT_DIPSETTING (   0x0d, "3%" )
	PORT_DIPSETTING (   0x0c, "4%" )
	PORT_DIPSETTING (   0x0b, "5%" )
	PORT_DIPSETTING (   0x0a, "6%" )
	PORT_DIPSETTING (   0x09, "7%" )
	PORT_DIPSETTING (   0x00, "8%" )
	PORT_DIPSETTING (   0x01, "9%" )
	PORT_DIPSETTING (   0x02, "10%" )
	PORT_DIPSETTING (   0x03, "11%" )
	PORT_DIPSETTING (   0x04, "12%" )
	PORT_DIPSETTING (   0x05, "13%" )
	PORT_DIPSETTING (   0x06, "14%" )
	PORT_DIPSETTING (   0x07, "15%" )
	PORT_DIPNAME( 0xf0, 0x00, "Win hold chance" )
	PORT_DIPSETTING (   0xf0, "17%" )
	PORT_DIPSETTING (   0xe0, "19%" )
	PORT_DIPSETTING (   0xd0, "21%" )
	PORT_DIPSETTING (   0xc0, "23%" )
	PORT_DIPSETTING (   0xb0, "25%" )
	PORT_DIPSETTING (   0xa0, "27%" )
	PORT_DIPSETTING (   0x90, "29%" )
	PORT_DIPSETTING (   0x00, "31%" )
	PORT_DIPSETTING (   0x10, "33%" )
	PORT_DIPSETTING (   0x20, "35%" )
	PORT_DIPSETTING (   0x30, "37%" )
	PORT_DIPSETTING (   0x40, "39%" )
	PORT_DIPSETTING (   0x50, "41%" )
	PORT_DIPSETTING (   0x60, "43%" )
	PORT_DIPSETTING (   0x70, "45%" )
	
	PORT_START("DIP1")
	PORT_DIPNAME( 0x0f, 0x00, "Hold chance" )
	PORT_DIPSETTING (   0x0f, "17%" )
	PORT_DIPSETTING (   0x0e, "19%" )
	PORT_DIPSETTING (   0x0d, "21%" )
	PORT_DIPSETTING (   0x0c, "23%" )
	PORT_DIPSETTING (   0x0b, "25%" )
	PORT_DIPSETTING (   0x0a, "27%" )
	PORT_DIPSETTING (   0x09, "29%" )
	PORT_DIPSETTING (   0x00, "31%" )
	PORT_DIPSETTING (   0x01, "33%" )
	PORT_DIPSETTING (   0x02, "35%" )
	PORT_DIPSETTING (   0x03, "37%" )
	PORT_DIPSETTING (   0x04, "39%" )
	PORT_DIPSETTING (   0x05, "41%" )
	PORT_DIPSETTING (   0x06, "43%" )
	PORT_DIPSETTING (   0x07, "45%" )
	PORT_DIPUNUSED( 0x10, 0x00 )
	PORT_DIPNAME( 0x20, 0x00, "Use default hold/nudge chance" ) // 31% and 8%
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, DEF_STR(On) )
	PORT_DIPUNUSED( 0x40, 0x00 )
	PORT_DIPUNUSED( 0x80, 0x00 )
	
	PORT_START("DIP2")
	PORT_DIPUNUSED( 0x01, 0x00 )
	PORT_DIPUNUSED( 0x02, 0x00 )
	PORT_DIPUNUSED( 0x04, 0x00 )
	PORT_DIPUNUSED( 0x08, 0x00 )
	PORT_DIPUNUSED( 0x10, 0x00 )
	PORT_DIPUNUSED( 0x20, 0x00 )
	PORT_DIPUNUSED( 0x40, 0x00 )
	PORT_DIPUNUSED( 0x80, 0x00 )
INPUT_PORTS_END

static INPUT_PORTS_START( j_dud )
	PORT_INCLUDE( jpmsru_inputs )
	
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Down")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")
INPUT_PORTS_END

static INPUT_PORTS_START( j_dud2 )
	PORT_INCLUDE( j_dud )
	
	PORT_MODIFY("IN0")
	PORT_CONFNAME( 0x80, 0x80, "5p/10p jumper" )
	PORT_CONFSETTING(	 0x00, "5p" )
	PORT_CONFSETTING(	 0x80, "10p" )
INPUT_PORTS_END

static INPUT_PORTS_START( j_lan )
	PORT_INCLUDE( jpmsru_inputs )
	
	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Hold 3")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Reverse")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Feature Stop")
INPUT_PORTS_END

static INPUT_PORTS_START( j_lan2 )
	PORT_INCLUDE( j_lan )
	
	PORT_MODIFY("IN0")
	PORT_CONFNAME( 0x80, 0x80, "5p/10p jumper" )
	PORT_CONFSETTING(	 0x00, "5p" )
	PORT_CONFSETTING(	 0x80, "10p" )
INPUT_PORTS_END

void jpmsru_state::machine_start()
{
	m_lamp.resolve();
	m_digits.resolve();
	
	save_item(NAME(m_reelbits[0]));
	save_item(NAME(m_reelbits[1]));
	save_item(NAME(m_reelbits[2]));
	save_item(NAME(m_reelbits[3]));
	save_item(NAME(m_int1));
	save_item(NAME(m_int2));
	save_item(NAME(m_int1_en));
	save_item(NAME(m_int2_en));
	save_item(NAME(m_disp_digit));
}

void jpmsru_state::init_jpmsru()
{
	m_int1 = 0;
	m_int2 = 0;
	m_reelbits[0] = 0;
	m_reelbits[1] = 0;
	m_reelbits[2] = 0;
	m_reelbits[3] = 0;
	m_disp_digit = 0;
	m_busext_mode = 0;
	m_busext_addr = 0;
}

// Base SRU with 3K ROM card
void jpmsru_state::jpmsru_3k(machine_config &config)
{
	TMS9980A(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &jpmsru_state::jpmsru_3k_map);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::jpmsru_io);
	
	TIMER(config, "int1").configure_periodic(FUNC(jpmsru_state::int1), attotime::from_hz(MAIN_CLOCK / 32768)); // 183.1 Hz reel timing
	TIMER(config, "int2").configure_periodic(FUNC(jpmsru_state::int2), attotime::from_hz(100)); // 100 Hz from AC zero crossing detector
	
	REEL(config, m_reel[0], SRU_200STEP_REEL, 8, 10, 0, 7, 200*2);
	m_reel[0]->optic_handler().set(FUNC(jpmsru_state::opto_cb<0>));
	REEL(config, m_reel[1], SRU_200STEP_REEL, 8, 10, 0, 7, 200*2);
	m_reel[1]->optic_handler().set(FUNC(jpmsru_state::opto_cb<1>));
	REEL(config, m_reel[2], SRU_200STEP_REEL, 8, 10, 0, 7, 200*2);
	m_reel[2]->optic_handler().set(FUNC(jpmsru_state::opto_cb<2>));
	REEL(config, m_reel[3], SRU_200STEP_REEL, 8, 10, 0, 7, 200*2);
	m_reel[3]->optic_handler().set(FUNC(jpmsru_state::opto_cb<3>));
	
	config.set_default_layout(layout_jpmsru);
	
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "nl_audio", 48000)
		.set_source(NETLIST_NAME(jpmsru))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, m_audio_in[0], "IN1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[1], "IN2.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[2], "IN3.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[3], "IN4.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[4], "IN5.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[5], "IN6.IN", 0);
	NETLIST_ANALOG_INPUT(config, "nl_audio:pot", "R8.DIAL");

	NETLIST_STREAM_OUTPUT(config, "nl_audio:cout0", 0, "OUT").set_mult_offset(1.0, 0.0);

	FRUIT_SAMPLES(config, m_samples);
}

// SRU with 3K ROM card and Bus Extension board
void jpmsru_state::jpmsru_3k_busext(machine_config &config)
{
	jpmsru_3k(config);
	
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::jpmsru_busext_io);
	
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

// SRU with 4K ROM card
void jpmsru_state::jpmsru_4k(machine_config &config)
{
	jpmsru_3k(config);
}

// Game configs
void jpmsru_state::ewn(machine_config &config)
{
	jpmsru_3k_busext(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_ewn);
}

void jpmsru_state::ewn2(machine_config &config)
{
	jpmsru_3k(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_ewn2);
}

void jpmsru_state::ndu(machine_config &config)
{
	jpmsru_3k_busext(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_ndu);
}

void jpmsru_state::dud(machine_config &config)
{
	jpmsru_3k(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_dud);
}

void jpmsru_state::lan(machine_config &config)
{
	jpmsru_3k(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_lan);
}

ROM_START( j_ewn )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewn20.1", 0x0000, 0x400, CRC(e90f686b) SHA1(aec88647c6289b01149b2816845a568481b1d37f) )
	ROM_LOAD( "ewn20.2", 0x0400, 0x400, CRC(c02a2427) SHA1(57144443a03db56a803b19e14e868b1ccd222f37) )
	ROM_LOAD( "ewn20.3", 0x0800, 0x400, CRC(a64e4df7) SHA1(1512c3c85e100dadd5ff67fed731feb69cc8575e) )
ROM_END

ROM_START( j_ewna )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewn26.a1", 0x0000, 0x400, CRC(a92760b7) SHA1(dfef0dab7799a4b4975755c1584efca81a3798c4) )
	ROM_LOAD( "ewn26.2", 0x0400, 0x400, CRC(bd24e59e) SHA1(038ed23283a7b61e873f543de32b685630fcdb97) )
	ROM_LOAD( "ewn26.3", 0x0800, 0x400, CRC(a3280b35) SHA1(2771c81735c69ae3efb02715ac97901dae434e72) )
ROM_END

ROM_START( j_ewnb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewn.1", 0x0000, 0x400, CRC(84ce735e) SHA1(98bae928246050ae88518ca511447fbef5c810f5) )
	ROM_LOAD( "ewn.2", 0x0400, 0x400, CRC(4c121f5e) SHA1(1221ff91ff9e352efeabb26a60eab93aae5bca5e) )
	ROM_LOAD( "ewn.3", 0x0800, 0x400, CRC(bef3a938) SHA1(6a6844203c6361b65f5b07853d9dbe18a29ebc44) )
ROM_END

ROM_START( j_ndu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ndu17.1", 0x0000, 0x400, CRC(174a8519) SHA1(3d9cc2a531ff91b3313aa893a9f774eea7847b8b) )
	ROM_LOAD( "ndu17.2", 0x0400, 0x400, CRC(634644b8) SHA1(35f2c71f81ddab18b85aa0a240dca55b0531f8d0) )
	ROM_LOAD( "ndu17.3", 0x0800, 0x400, CRC(60ef9c60) SHA1(e3614407a74c9e462cdfb3275b1c99b706cd824c) )
ROM_END

ROM_START( j_ndua ) // 24% hold chance instead of 26%
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ndu17.1", 0x0000, 0x400, CRC(fae0f783) SHA1(1a09f8f425ec6b6d02aa6f32f7c7e22798a80a3a) )
	ROM_LOAD( "ndu17.2", 0x0400, 0x400, CRC(634644b8) SHA1(35f2c71f81ddab18b85aa0a240dca55b0531f8d0) )
	ROM_LOAD( "ndu17.3", 0x0800, 0x400, CRC(60ef9c60) SHA1(e3614407a74c9e462cdfb3275b1c99b706cd824c) )
ROM_END

ROM_START( j_dud )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dud10.1", 0x0000, 0x400, CRC(e6cc42bc) SHA1(5f24f9fdb577a4ea4ef8d35352dd63021ebf26cd) )
	ROM_LOAD( "dud10.2", 0x0400, 0x400, CRC(69243c04) SHA1(958791fbd515ab6e2b38391527b611977303ad10) )
	ROM_LOAD( "dud10.3", 0x0800, 0x400, CRC(9f67e2f7) SHA1(f850655ba5d3651ff91f624431deb0e008fab57e) )
ROM_END

ROM_START( j_duda )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dud.1", 0x0000, 0x400, CRC(66445282) SHA1(8614b5330d72ed28141974e60a2238e003f4bce1) )
	ROM_LOAD( "dud.2", 0x0400, 0x400, CRC(2945e808) SHA1(e306b5f9cc9f4999b9b4b8536101f2b69728f6ca) )
	ROM_LOAD( "dud.3", 0x0800, 0x400, CRC(f4359851) SHA1(43c17c147a96aba901435154de657594fbec6008) )
ROM_END

ROM_START( j_dt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dt13.1", 0x0000, 0x400, CRC(ce0b9d56) SHA1(1a21ddc4af260f8f4799c0d0e6ab17fc6385fbd5) )
	ROM_LOAD( "dt13.2", 0x0400, 0x400, CRC(cd9ac1cd) SHA1(e5983145e52843be701752c7fb52a99d799a489d) )
	ROM_LOAD( "dt13.3", 0x0800, 0x400, CRC(6de3a213) SHA1(b264fc45adbe3b2b833890f4a6238bd2f41d053f) )
ROM_END

// LIGHT A NUDGE - REVISION 17
ROM_START( j_lan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lan17.f1", 0x0000, 0x400, CRC(82d974b4) SHA1(2e10b40cf45c0aa7669e8bb046382982de1e77e3) )
	ROM_LOAD( "lan17.2", 0x0400, 0x400, CRC(e58416ed) SHA1(7d1cdd7007297a467487c6f58abefb4a70197838) )
	ROM_LOAD( "lan17.3", 0x0800, 0x400, CRC(fdc857b4) SHA1(3b89f0cd5e8ed73c2e3cded1edc64d23d0fefa54) )
ROM_END

ROM_START( j_lana )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lan.1", 0x0000, 0x400, CRC(7d959d11) SHA1(6f2834a9279cee1abb48837ef862fb42b28b1cb8) )
	ROM_LOAD( "lan.2", 0x0400, 0x400, CRC(18089aed) SHA1(1fc5250b56ba0f5211a591fbc0470fa60535cdd8) )
	ROM_LOAD( "lan.3", 0x0800, 0x400, CRC(d3e76076) SHA1(1f9f96351e8bc08722dc047c8b80c4697c589939) )
ROM_END


ROM_START( j_ews )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ews13c1.bin", 0x0000, 0x000400, CRC(2eec7c4d) SHA1(a1740d27e60192659392ba7602b9b62947c4f6db) )
	ROM_LOAD( "ews13b2.bin", 0x0400, 0x000400, CRC(b84b7858) SHA1(90fd64881d52e1f4362ccbcb9434dbf7b25b97f9) )
	ROM_LOAD( "ews13.3",     0x0800, 0x000400, CRC(4d8e197a) SHA1(1569327f0e4b5d7632658b69abf59076effb2600) )
ROM_END

ROM_START( j_ews8a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ews8a.1", 0x0000, 0x000400, CRC(52e9709a) SHA1(0b437834f48ca7718e0b30303916eed00c7fb4c9) )
	ROM_LOAD( "ews8a.2", 0x0400, 0x000400, CRC(ee4a4809) SHA1(292a12a5ddc5a22c8568016b34dfec7959f49027) )
	ROM_LOAD( "ews8a.3", 0x0800, 0x000400, CRC(3700a7a3) SHA1(cf24a54e6aa3a3a86ff75f6e8bcb692d0cfd0e80) )
ROM_END

ROM_START( j_luckac )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "la1.bin", 0x0000, 0x000400, CRC(21076280) SHA1(d5cf25d289f03c743f4428273ac002df3164c344) )
	ROM_LOAD( "la2.bin", 0x0400, 0x000400, CRC(cae10bc1) SHA1(a740946437a3b277b714f13d001783987f57bc77) )
	ROM_LOAD( "la3.bin", 0x0800, 0x000400, CRC(cb9362ac) SHA1(a16d43ba01b24e1b515881957c1559d33a03bcc4) )
ROM_END

ROM_START( j_plus2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "plus2_1.bin", 0x0000, 0x000400, CRC(f635174d) SHA1(9478aabc0eaa25d4ae44d2385e738584f03f6647) )
	ROM_LOAD( "plus2_2.bin", 0x0400, 0x000400, CRC(0999d32f) SHA1(e08c852f8f3aff8ab7b73e9c0b0502ab91f9e844) )
	ROM_LOAD( "plus2_3.bin", 0x0800, 0x000400, CRC(d3dfd6ab) SHA1(4cf0f8977fb2c023bf2ccc8d9d74352ce32206bf) )
	ROM_LOAD( "plus2_4.bin", 0x0c00, 0x000400, CRC(8b6922b4) SHA1(7b7fc7b0708bf96846860254fea957bcbc952923) )
ROM_END

ROM_START( j_super2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super2_1.bin", 0x0000, 0x000400, CRC(a1df2719) SHA1(eed80329c14ef6c272a8c622e8a4bc7d14ac87e6) )
	ROM_LOAD( "super2_2.bin", 0x0400, 0x000400, CRC(0fd5ddd0) SHA1(e8d31b009b29486d36d11052af857c609a7f1f84) )
	ROM_LOAD( "super2_3.bin", 0x0800, 0x000400, CRC(ddd998d3) SHA1(5964da70ae4c2f174dc3d1494fc67579c221a7b7) )
ROM_END

ROM_START( j_luck2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lt_9.1", 0x0000, 0x000400, CRC(97236ce3) SHA1(f71861576f33daec3e1d371c670b535e6fd32b5e) )
	ROM_LOAD( "lt_9.2", 0x0400, 0x000400, CRC(6e1cd083) SHA1(17edaa9880ae2a6d6d99e771e41b985527d5ed3b) )
	ROM_LOAD( "lt_9.3", 0x0800, 0x000400, CRC(d6881e6f) SHA1(42a83f01d67a8f530ca2a10ffeff30237bdfba94) )
ROM_END

ROM_START( j_unk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sruunk1.p1", 0x0000, 0x000400, CRC(be7d3b79) SHA1(3304dcc69e93eca2e6e89df0b18afc6874ebacf0) )
	ROM_LOAD( "sruunk1.p2", 0x0400, 0x000400, CRC(bf19cd60) SHA1(77b0b439628589cb0db1b74a760b652519c20991) )
	ROM_LOAD( "sruunk1.p3", 0x0800, 0x000400, CRC(25138e03) SHA1(644fc6144ea74f08dc892f106ad494ba364afe86) )
ROM_END

#define GAME_FLAGS MACHINE_NOT_WORKING|MACHINE_MECHANICAL|MACHINE_REQUIRES_ARTWORK|MACHINE_IMPERFECT_SOUND|MACHINE_SUPPORTS_SAVE

GAMEL( 1979?, j_ewn,     0,        ewn,  j_ewn,  jpmsru_state, init_jpmsru, ROT0, "JPM", "Each Way Nudger (JPM) (SRU) (revision 20, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_ewn )
GAMEL( 1981?, j_ewna,    j_ewn,    ewn2, j_ewn2, jpmsru_state, init_jpmsru, ROT0, "JPM", "Each Way Nudger (JPM) (SRU) (revision 26A, £2 Jackpot)", GAME_FLAGS, layout_j_ewn )
GAMEL( 1981?, j_ewnb,    j_ewn,    ewn2, j_ewn2, jpmsru_state, init_jpmsru, ROT0, "JPM", "Each Way Nudger (JPM) (SRU) (£2 Jackpot)", GAME_FLAGS, layout_j_ewn )
GAMEL( 1979?, j_ndu,     0,        ndu,  j_ndu,  jpmsru_state, init_jpmsru, ROT0, "JPM", "Nudge Double Up (JPM) (SRU) (revision 17, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_ndu )
GAMEL( 1979?, j_ndua,    j_ndu,    ndu,  j_ndu,  jpmsru_state, init_jpmsru, ROT0, "JPM", "Nudge Double Up (JPM) (SRU) (revision 17, 5p Stake, £1 Jackpot, lower %)", GAME_FLAGS, layout_j_ndu )
GAMEL( 1980?, j_dud,     0,        dud,  j_dud,  jpmsru_state, init_jpmsru, ROT0, "JPM", "Nudge Double Up Deluxe (JPM) (SRU) (revision 10, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_dud )
GAMEL( 1981?, j_duda,    j_dud,    dud,  j_dud2, jpmsru_state, init_jpmsru, ROT0, "JPM", "Nudge Double Up Deluxe (JPM) (SRU) (£2 Jackpot)", GAME_FLAGS, layout_j_dud )
GAMEL( 1981?, j_dt,      j_dud,    dud,  j_dud2, jpmsru_state, init_jpmsru, ROT0, "JPM", "Double Top (JPM) (SRU) (revision 13, £2 Jackpot)", GAME_FLAGS, layout_j_dud )
GAMEL( 1980?, j_lan,     0,        lan,  j_lan,  jpmsru_state, init_jpmsru, ROT0, "JPM", "Lite A Nudge (JPM) (SRU) (revision 17F, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_lan )
GAMEL( 1981?, j_lana,    j_lan,    lan,  j_lan2, jpmsru_state, init_jpmsru, ROT0, "JPM", "Lite A Nudge (JPM) (SRU) (£2 Jackpot)", GAME_FLAGS, layout_j_lan )
// Unsorted
GAMEL(198?,  j_ews,      0,        jpmsru_3k,        jpmsru_inputs, jpmsru_state, init_jpmsru, ROT0, "JPM","Each Way Shuffle (Barcrest?, set 1, revision 16)", MACHINE_IS_SKELETON_MECHANICAL, layout_jpmsru )
GAMEL(198?,  j_ews8a,    j_ews,    jpmsru_3k,        jpmsru_inputs, jpmsru_state, init_jpmsru, ROT0, "JPM","Each Way Shuffle (Barcrest?, set 2, revision 8a)", MACHINE_IS_SKELETON_MECHANICAL, layout_jpmsru )
GAMEL(198?,  j_luckac,   0,        jpmsru_3k,        jpmsru_inputs, jpmsru_state, init_jpmsru, ROT0, "JPM","Lucky Aces (Unk)",                                MACHINE_IS_SKELETON_MECHANICAL, layout_jpmsru )
GAMEL(198?,  j_super2,   0,        jpmsru_3k,        jpmsru_inputs, jpmsru_state, init_jpmsru, ROT0, "JPM","Super 2 (JPM)",                                         MACHINE_IS_SKELETON_MECHANICAL, layout_jpmsru )
GAMEL(198?,  j_luck2,    0,        jpmsru_4k,        jpmsru_inputs, jpmsru_state, init_jpmsru, ROT0, "JPM","Lucky 2's",                                     MACHINE_IS_SKELETON_MECHANICAL, layout_jpmsru )
GAMEL(198?,  j_unk,      0,        jpmsru_4k,        jpmsru_inputs, jpmsru_state, init_jpmsru, ROT0, "JPM?","unknown SRU Game (JPM?)",                              MACHINE_IS_SKELETON_MECHANICAL, layout_jpmsru )
GAMEL(198?,  j_plus2,    0,        jpmsru_4k,         jpmsru_inputs, jpmsru_state, init_jpmsru, ROT0, "JPM","Plus 2 (JPM)",                                          MACHINE_IS_SKELETON_MECHANICAL, layout_jpmsru )
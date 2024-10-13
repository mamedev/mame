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

#include "nl_jpmsru.h"

#include "awpvid.h"
#include "fruitsamples.h"

#include "cpu/tms9900/tms9980a.h"

#include "machine/netlist.h"
#include "machine/nvram.h"
#include "machine/steppers.h"
#include "machine/timer.h"
#include "sound/dac.h"

#include "speaker.h"

#include "netlist/nl_setup.h"

namespace {

#include "jpmsru.lh"
#include "j_cnudgr.lh"
#include "j_dud.lh"
#include "j_ewn.lh"
#include "j_ews.lh"
#include "j_ewsdlx.lh"
#include "j_la.lh"
#include "j_lal.lh"
#include "j_lan.lh"
#include "j_lc.lh"
#include "j_lt.lh"
#include "j_ndu.lh"
#include "j_plus2.lh"
#include "j_ssh.lh"
#include "j_sup2p.lh"
#include "j_super2.lh"
#include "j_supsh.lh"

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
			m_nvram(*this, "nvram", 0x100, ENDIANNESS_BIG),
			m_dips(*this, "DIP%u", 0U)
	{ }

	void jpmsru_3k(machine_config &config);
	void jpmsru_3k_busext(machine_config &config);
	void jpmsru_4k(machine_config &config);
	void jpmsru_6k(machine_config &config);
	void ewn(machine_config &config);
	void ewn2(machine_config &config);
	void ndu(machine_config &config);
	void dud(machine_config &config);
	void lan(machine_config &config);
	void super2(machine_config &config);
	void ews(machine_config &config);
	void lt(machine_config &config);
	void sup2p(machine_config &config);
	void lal(machine_config &config);

	template <unsigned N> int opto_r() { return m_opto[N]; }
protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void device_post_load() override;

	template <unsigned N> void opto_cb(int state) { m_opto[N] = state; }

	TIMER_DEVICE_CALLBACK_MEMBER(int1);
	TIMER_DEVICE_CALLBACK_MEMBER(int2);

	uint8_t inputs_r(offs_t offset);
	uint8_t inputs_ext_r(offs_t offset);
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
	void out_disp_6digit_w(offs_t offset, uint8_t data);
	void out_payout_cash_w(offs_t offset, uint8_t data);
	void out_payout_token_w(offs_t offset, uint8_t data);
	void out_payout_2x50p_a_w(offs_t offset, uint8_t data);
	void out_payout_2x50p_b_w(offs_t offset, uint8_t data);
	template<unsigned Meter> void out_meter_w(offs_t offset, uint8_t data);
	void out_coin_lockout_w(offs_t offset, uint8_t data);
	void out_10p_lockout_w(offs_t offset, uint8_t data);
	void out_50p_lockout_w(offs_t offset, uint8_t data);
	void out_logicext_w(offs_t offset, uint8_t data);

	void jpmsru_3k_map(address_map &map) ATTR_COLD;
	void jpmsru_4k_map(address_map &map) ATTR_COLD;
	void jpmsru_6k_map(address_map &map) ATTR_COLD;
	void jpmsru_io(address_map &map) ATTR_COLD;
	void jpmsru_busext_io(address_map &map) ATTR_COLD;
	void outputs_ewn(address_map &map) ATTR_COLD;
	void outputs_ewn2(address_map &map) ATTR_COLD;
	void outputs_ndu(address_map &map) ATTR_COLD;
	void outputs_dud(address_map &map) ATTR_COLD;
	void outputs_lan(address_map &map) ATTR_COLD;
	void outputs_super2(address_map &map) ATTR_COLD;
	void outputs_ews(address_map &map) ATTR_COLD;
	void outputs_sup2p(address_map &map) ATTR_COLD;
	void outputs_lal(address_map &map) ATTR_COLD;

	uint8_t m_int1;
	uint8_t m_int2;
	uint8_t m_int1_en;
	uint8_t m_int2_en;
	uint8_t m_reelbits[4];
	bool m_opto[4];
	uint8_t m_disp_digit;
	bool m_disp_select[6];
	uint8_t m_logicext_addr;
	bool m_logicext_data;
	bool m_busext_bdir;
	uint8_t m_busext_mode;
	uint8_t m_busext_addr;

	required_device<cpu_device> m_maincpu;
	required_ioport_array<4> m_inputs;
	required_device_array<stepper_device, 4> m_reel;
	output_finder<104> m_lamp;
	output_finder<6> m_digits;
	required_device_array<netlist_mame_logic_input_device, 6> m_audio_in;
	required_device<fruit_samples_device> m_samples;

	memory_share_creator<uint8_t> m_nvram;
	optional_ioport_array<3> m_dips;
};

class jpmsru_dac_state : public jpmsru_state
{
public:
	jpmsru_dac_state(const machine_config &mconfig, device_type type, const char *tag) :
			jpmsru_state(mconfig, type, tag),
			m_dac(*this, "dac")
	{ }

	void lc(machine_config &config);

private:
	void outputs_lc(address_map &map) ATTR_COLD;

	required_device<dac_1bit_device> m_dac;
};

void jpmsru_state::jpmsru_3k_map(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0cff).mirror(0x300).ram();
	map(0x1000, 0x13ff).noprw(); // Unused ROM space
	/* NVRAM space, not there on most games, depending on the game the ROM card
	   has it unpopulated or straight up doesn't have it. Later AWP games write
	   various stats to the nonexistent NVRAM and the code has support for some
	   sort of datalogger ROM at 0x1C00, which if detected gets jumped to and
	   ends up sending the stats somewhere. There aren't any known dumps of
	   those ROMs, if they were ever in the wild in the first place. */
	map(0x1400, 0x14ff).ram();
	map(0x1800, 0x1fff).noprw(); // Unused ROM space
}

void jpmsru_state::jpmsru_4k_map(address_map &map)
{
	jpmsru_3k_map(map);

	map(0x1000, 0x13ff).rom();
}

void jpmsru_state::jpmsru_6k_map(address_map &map)
{
	jpmsru_3k_map(map);

	map(0x1000, 0x13ff).rom();
	map(0x1400, 0x14ff).ram().share("nvram");
	map(0x1800, 0x1fff).rom();
}

void jpmsru_state::jpmsru_io(address_map &map)
{
	map.global_mask(0xff);
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

void jpmsru_state::outputs_super2(address_map &map)
{
	outputs_lan(map);

	map(0x5a, 0x5b).w(FUNC(jpmsru_state::out_50p_lockout_w));
}

void jpmsru_state::outputs_ews(address_map &map)
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
	map(0x54, 0x55).w(FUNC(jpmsru_state::out_50p_lockout_w));
	map(0x6a, 0x6b).w(FUNC(jpmsru_state::out_payout_cash_w));
	map(0x6c, 0x6d).w(FUNC(jpmsru_state::out_payout_token_w));
	map(0x6e, 0x6f).w(FUNC(jpmsru_state::out_coin_lockout_w));
	// Mini Logic Extension outputs, used for extra lamps
	map(0x80, 0x9f).w(FUNC(jpmsru_state::out_lamp_ext_w));
}

void jpmsru_state::outputs_sup2p(address_map &map)
{
	jpmsru_io(map);

	map(0x3e, 0x3f).w(FUNC(jpmsru_state::out_meter_w<0>));
	map(0x42, 0x4d).w(FUNC(jpmsru_state::out_disp_w));
	map(0x4e, 0x4f).w(FUNC(jpmsru_state::out_meter_w<1>));
	map(0x50, 0x51).w(FUNC(jpmsru_state::out_meter_w<2>));
	map(0x54, 0x55).w(FUNC(jpmsru_state::out_50p_lockout_w));
	map(0x6a, 0x6b).w(FUNC(jpmsru_state::out_payout_cash_w));
	map(0x6e, 0x6f).w(FUNC(jpmsru_state::out_coin_lockout_w));
}

void jpmsru_dac_state::outputs_lc(address_map &map)
{
	jpmsru_io(map);

	map(0x60, 0x6f).r(FUNC(jpmsru_dac_state::inputs_ext_r)); // Input Extension

	/* There are 2 outputs for each payout, labeled payout 1 and payout 2.
	   Both are always written to at the same time. I'm using the first output for payouts. */
	map(0x28, 0x29).nopw(); // 10p payout 2
	map(0x2a, 0x2b).nopw(); // 2x50p A payout 2
	map(0x30, 0x31).w(FUNC(jpmsru_dac_state::out_meter_w<0>));
	map(0x32, 0x33).w(FUNC(jpmsru_dac_state::out_meter_w<1>));
	map(0x34, 0x35).w(FUNC(jpmsru_dac_state::out_meter_w<2>));
	map(0x36, 0x37).w(FUNC(jpmsru_dac_state::out_meter_w<3>));
	map(0x38, 0x39).w(FUNC(jpmsru_dac_state::out_meter_w<4>));
	map(0x3a, 0x3b).w(FUNC(jpmsru_dac_state::out_payout_cash_w)); // 10p payout
	map(0x3c, 0x3d).w(FUNC(jpmsru_dac_state::out_payout_2x50p_a_w));
	map(0x3e, 0x3f).w(FUNC(jpmsru_dac_state::out_payout_2x50p_b_w));
	map(0x40, 0x4b).w(FUNC(jpmsru_dac_state::out_disp_w));
	map(0x4e, 0x4f).w(m_dac, FUNC(dac_bit_interface::data_w)).umask16(0xff00);
	map(0x50, 0x61).w(FUNC(jpmsru_dac_state::out_logicext_w));
	map(0x62, 0x63).nopw(); // 2x50p B payout 2
	map(0x6c, 0x6d).w(FUNC(jpmsru_dac_state::out_10p_lockout_w));
	map(0x6e, 0x6f).w(FUNC(jpmsru_dac_state::out_50p_lockout_w));
}

void jpmsru_state::outputs_lal(address_map &map)
{
	jpmsru_busext_io(map);

	map(0x3a, 0x3b).w(FUNC(jpmsru_state::out_meter_w<1>));
	map(0x3c, 0x3d).w(FUNC(jpmsru_state::out_meter_w<2>));
	map(0x3e, 0x3f).w(FUNC(jpmsru_state::out_meter_w<0>)); // Payout meter
	map(0x42, 0x55).w(FUNC(jpmsru_state::out_disp_6digit_w));
	map(0x5c, 0x6b).w(FUNC(jpmsru_state::out_logicext_w)); // Last bit (reset) is cut off, not used or earlier extender board with no reset line?
	map(0x6c, 0x6d).w(FUNC(jpmsru_state::out_meter_w<3>));
	map(0x6e, 0x6f).w(FUNC(jpmsru_state::out_coin_lockout_w));
}

uint8_t jpmsru_state::inputs_r(offs_t offset)
{
	return BIT(m_inputs[(offset & 0x18) >> 3]->read(), offset & 0x7);
}

uint8_t jpmsru_state::inputs_ext_r(offs_t offset)
{
	return BIT(m_inputs[3]->read(), offset);
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
		case 0:
		case 1:
		case 2:
		case 3: m_disp_digit = (m_disp_digit & ~(1 << offset)) | (data ? 0 : (1 << offset)); break;
		case 4:
		case 5: m_disp_select[offset - 4] = data; break;
	}

	static constexpr uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 };
	for(int i = 0; i < 2; i++) if(m_disp_select[i]) m_digits[i] = patterns[m_disp_digit];
}

void jpmsru_state::out_disp_6digit_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
		case 1:
		case 2:
		case 3: m_disp_digit = (m_disp_digit & ~(1 << offset)) | (data ? 0 : (1 << offset)); break;
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9: m_disp_select[offset - 4] = data; break;
	}

	static constexpr uint8_t patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0,0,0,0,0,0 };
	for(int i = 0; i < 6; i++) if(m_disp_select[i]) m_digits[i] = patterns[m_disp_digit];
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

void jpmsru_state::out_payout_2x50p_a_w(offs_t offset, uint8_t data) // First 2x50p tube on Lucky Casino
{
	if(data) m_samples->play(fruit_samples_device::SAMPLE_PAYOUT);
}

void jpmsru_state::out_payout_2x50p_b_w(offs_t offset, uint8_t data) // Second 2x50p tube on Lucky Casino
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

void jpmsru_state::out_10p_lockout_w(offs_t offset, uint8_t data) // 10p lockout on Lucky Casino
{
	machine().bookkeeping().coin_lockout_w(0, !data);
}

void jpmsru_state::out_50p_lockout_w(offs_t offset, uint8_t data)
{
	// 50p is always coin 4
	machine().bookkeeping().coin_lockout_w(3, !data);
}

void jpmsru_state::out_logicext_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0: out_lamp_ext_w(m_logicext_addr, !m_logicext_data); break; /* All dumped games use solely lamps as outputs,
		                                                                     so keep things simple for now */
		case 1: m_logicext_data = data; break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7: m_logicext_addr = (m_logicext_addr & ~(1 << (offset - 2))) | (data ? 0 : (1 << (offset - 2))); break;
		case 8: m_logicext_addr = 0; break;
	}
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
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("5p") PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("10p Token") PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(2)

	PORT_START("IN3") // Input Extension
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("POT")
	PORT_ADJUSTER( 50, "Tone Pot" )  NETLIST_ANALOG_PORT_CHANGED("nl_audio", "pot")
INPUT_PORTS_END

static INPUT_PORTS_START( j_ewn )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Down")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")

	PORT_START("DIP0")
	PORT_DIPNAME( 0x0f, 0x00, "Nudge chance" )
	PORT_DIPSETTING(    0x0f, "1%" )
	PORT_DIPSETTING(    0x0e, "2%" )
	PORT_DIPSETTING(    0x0d, "3%" )
	PORT_DIPSETTING(    0x0c, "4%" )
	PORT_DIPSETTING(    0x0b, "5%" )
	PORT_DIPSETTING(    0x0a, "6%" )
	PORT_DIPSETTING(    0x09, "7%" )
	PORT_DIPSETTING(    0x00, "8%" )
	PORT_DIPSETTING(    0x01, "9%" )
	PORT_DIPSETTING(    0x02, "10%" )
	PORT_DIPSETTING(    0x03, "11%" )
	PORT_DIPSETTING(    0x04, "12%" )
	PORT_DIPSETTING(    0x05, "13%" )
	PORT_DIPSETTING(    0x06, "14%" )
	PORT_DIPSETTING(    0x07, "15%" )
	PORT_DIPNAME( 0xf0, 0x00, "Win hold chance" )
	PORT_DIPSETTING(    0xf0, "17%" )
	PORT_DIPSETTING(    0xe0, "19%" )
	PORT_DIPSETTING(    0xd0, "21%" )
	PORT_DIPSETTING(    0xc0, "23%" )
	PORT_DIPSETTING(    0xb0, "25%" )
	PORT_DIPSETTING(    0xa0, "27%" )
	PORT_DIPSETTING(    0x90, "29%" )
	PORT_DIPSETTING(    0x00, "31%" )
	PORT_DIPSETTING(    0x10, "33%" )
	PORT_DIPSETTING(    0x20, "35%" )
	PORT_DIPSETTING(    0x30, "37%" )
	PORT_DIPSETTING(    0x40, "39%" )
	PORT_DIPSETTING(    0x50, "41%" )
	PORT_DIPSETTING(    0x60, "43%" )
	PORT_DIPSETTING(    0x70, "45%" )

	PORT_START("DIP1")
	PORT_DIPNAME( 0x0f, 0x00, "Hold chance" )
	PORT_DIPSETTING(    0x0f, "17%" )
	PORT_DIPSETTING(    0x0e, "19%" )
	PORT_DIPSETTING(    0x0d, "21%" )
	PORT_DIPSETTING(    0x0c, "23%" )
	PORT_DIPSETTING(    0x0b, "25%" )
	PORT_DIPSETTING(    0x0a, "27%" )
	PORT_DIPSETTING(    0x09, "29%" )
	PORT_DIPSETTING(    0x00, "31%" )
	PORT_DIPSETTING(    0x01, "33%" )
	PORT_DIPSETTING(    0x02, "35%" )
	PORT_DIPSETTING(    0x03, "37%" )
	PORT_DIPSETTING(    0x04, "39%" )
	PORT_DIPSETTING(    0x05, "41%" )
	PORT_DIPSETTING(    0x06, "43%" )
	PORT_DIPSETTING(    0x07, "45%" )
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

static INPUT_PORTS_START( j_ewn1 )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Down")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")
INPUT_PORTS_END

static INPUT_PORTS_START( j_ewn2 )
	PORT_INCLUDE( j_ewn1 )

	PORT_MODIFY("IN0")
	PORT_CONFNAME( 0x80, 0x80, "5p/10p jumper" )
	PORT_CONFSETTING(    0x00, "5p" )
	PORT_CONFSETTING(    0x80, "10p" )
INPUT_PORTS_END

static INPUT_PORTS_START( j_ndu )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Down")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")
INPUT_PORTS_END

static INPUT_PORTS_START( j_ndu17 )
	PORT_INCLUDE( j_ndu )

	PORT_START("DIP0")
	PORT_DIPNAME( 0x0f, 0x00, "Nudge chance" )
	PORT_DIPSETTING(    0x0f, "1%" )
	PORT_DIPSETTING(    0x0e, "2%" )
	PORT_DIPSETTING(    0x0d, "3%" )
	PORT_DIPSETTING(    0x0c, "4%" )
	PORT_DIPSETTING(    0x0b, "5%" )
	PORT_DIPSETTING(    0x0a, "6%" )
	PORT_DIPSETTING(    0x09, "7%" )
	PORT_DIPSETTING(    0x00, "8%" )
	PORT_DIPSETTING(    0x01, "9%" )
	PORT_DIPSETTING(    0x02, "10%" )
	PORT_DIPSETTING(    0x03, "11%" )
	PORT_DIPSETTING(    0x04, "12%" )
	PORT_DIPSETTING(    0x05, "13%" )
	PORT_DIPSETTING(    0x06, "14%" )
	PORT_DIPSETTING(    0x07, "15%" )
	PORT_DIPNAME( 0xf0, 0x00, "Win hold chance" )
	PORT_DIPSETTING(    0xf0, "12%" )
	PORT_DIPSETTING(    0xe0, "14%" )
	PORT_DIPSETTING(    0xd0, "16%" )
	PORT_DIPSETTING(    0xc0, "18%" )
	PORT_DIPSETTING(    0xb0, "20%" )
	PORT_DIPSETTING(    0xa0, "22%" )
	PORT_DIPSETTING(    0x90, "24%" )
	PORT_DIPSETTING(    0x00, "26%" )
	PORT_DIPSETTING(    0x10, "28%" )
	PORT_DIPSETTING(    0x20, "30%" )
	PORT_DIPSETTING(    0x30, "32%" )
	PORT_DIPSETTING(    0x40, "34%" )
	PORT_DIPSETTING(    0x50, "36%" )
	PORT_DIPSETTING(    0x60, "38%" )
	PORT_DIPSETTING(    0x70, "40%" )

	PORT_START("DIP1")
	PORT_DIPNAME( 0x0f, 0x00, "Hold chance" )
	PORT_DIPSETTING(    0x0f, "12%" )
	PORT_DIPSETTING(    0x0e, "14%" )
	PORT_DIPSETTING(    0x0d, "16%" )
	PORT_DIPSETTING(    0x0c, "18%" )
	PORT_DIPSETTING(    0x0b, "20%" )
	PORT_DIPSETTING(    0x0a, "22%" )
	PORT_DIPSETTING(    0x09, "24%" )
	PORT_DIPSETTING(    0x00, "26%" )
	PORT_DIPSETTING(    0x01, "28%" )
	PORT_DIPSETTING(    0x02, "30%" )
	PORT_DIPSETTING(    0x03, "32%" )
	PORT_DIPSETTING(    0x04, "34%" )
	PORT_DIPSETTING(    0x05, "36%" )
	PORT_DIPSETTING(    0x06, "38%" )
	PORT_DIPSETTING(    0x07, "40%" )
	PORT_DIPUNUSED( 0x10, 0x00 )
	PORT_DIPNAME( 0x20, 0x00, "Use default hold/nudge chance" ) // 26% and 8%
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

static INPUT_PORTS_START( j_ndu17_alt )
	PORT_INCLUDE( j_ndu17 )

	PORT_MODIFY("DIP0")
	PORT_DIPNAME( 0xf0, 0x00, "Win hold chance" )
	PORT_DIPSETTING(    0xf0, "10%" )
	PORT_DIPSETTING(    0xe0, "12%" )
	PORT_DIPSETTING(    0xd0, "14%" )
	PORT_DIPSETTING(    0xc0, "16%" )
	PORT_DIPSETTING(    0xb0, "18%" )
	PORT_DIPSETTING(    0xa0, "20%" )
	PORT_DIPSETTING(    0x90, "22%" )
	PORT_DIPSETTING(    0x00, "24%" )
	PORT_DIPSETTING(    0x10, "26%" )
	PORT_DIPSETTING(    0x20, "28%" )
	PORT_DIPSETTING(    0x30, "30%" )
	PORT_DIPSETTING(    0x40, "32%" )
	PORT_DIPSETTING(    0x50, "34%" )
	PORT_DIPSETTING(    0x60, "36%" )
	PORT_DIPSETTING(    0x70, "38%" )

	PORT_MODIFY("DIP1")
	PORT_DIPNAME( 0x0f, 0x00, "Hold chance" )
	PORT_DIPSETTING(    0x0f, "10%" )
	PORT_DIPSETTING(    0x0e, "12%" )
	PORT_DIPSETTING(    0x0d, "14%" )
	PORT_DIPSETTING(    0x0c, "16%" )
	PORT_DIPSETTING(    0x0b, "18%" )
	PORT_DIPSETTING(    0x0a, "20%" )
	PORT_DIPSETTING(    0x09, "22%" )
	PORT_DIPSETTING(    0x00, "24%" )
	PORT_DIPSETTING(    0x01, "26%" )
	PORT_DIPSETTING(    0x02, "28%" )
	PORT_DIPSETTING(    0x03, "30%" )
	PORT_DIPSETTING(    0x04, "32%" )
	PORT_DIPSETTING(    0x05, "34%" )
	PORT_DIPSETTING(    0x06, "36%" )
	PORT_DIPSETTING(    0x07, "38%" )
	PORT_DIPUNUSED( 0x10, 0x00 )
	PORT_DIPNAME( 0x20, 0x00, "Use default hold/nudge chance" ) // 24% and 8%
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x20, DEF_STR(On) )
	PORT_DIPUNUSED( 0x40, 0x00 )
	PORT_DIPUNUSED( 0x80, 0x00 )
INPUT_PORTS_END

static INPUT_PORTS_START( j_dud )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Down")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")
INPUT_PORTS_END

static INPUT_PORTS_START( j_dud2 )
	PORT_INCLUDE( j_dud )

	PORT_MODIFY("IN0")
	PORT_CONFNAME( 0x80, 0x80, "5p/10p jumper" )
	PORT_CONFSETTING(    0x00, "5p" )
	PORT_CONFSETTING(    0x80, "10p" )
INPUT_PORTS_END

static INPUT_PORTS_START( j_lan )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Feature Stop")
INPUT_PORTS_END

static INPUT_PORTS_START( j_lan2 )
	PORT_INCLUDE( j_lan )

	PORT_MODIFY("IN0")
	PORT_CONFNAME( 0x80, 0x80, "5p/10p jumper" )
	PORT_CONFSETTING(    0x00, "5p" )
	PORT_CONFSETTING(    0x80, "10p" )
INPUT_PORTS_END

static INPUT_PORTS_START( j_super2 )
	PORT_INCLUDE( j_lan )

	PORT_MODIFY("IN2")
	PORT_CONFNAME( 0x02, 0x02, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x02, "Full" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("20p") PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("2p") PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( j_ews )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN0")
	PORT_CONFNAME( 0x80, 0x80, "5p/10p jumper" )
	PORT_CONFSETTING(    0x00, "5p" )
	PORT_CONFSETTING(    0x80, "10p" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_DIPNAME( 0x08, 0x00, "Percentage Stabiliser" )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x08, DEF_STR(On) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Shuffle")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Feature Stop")
	PORT_CONFNAME( 0x02, 0x02, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x02, "Full" )
INPUT_PORTS_END

static INPUT_PORTS_START( j_ewsdlx )
	PORT_INCLUDE( j_ews )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // No % stabiliser
INPUT_PORTS_END

static INPUT_PORTS_START( j_ssh )
	PORT_INCLUDE( j_ews )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // No stake jumper

	PORT_MODIFY("IN1")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // No % stabiliser

	PORT_MODIFY("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("20p") PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("2p") PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( j_supsh )
	PORT_INCLUDE( j_ews )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) /* No stake jumper, 50p coin pulses meter 1 160 times if this is on and prizes
	                                                are doubled as well, which doesn't make sense with 10p coins */

	PORT_MODIFY("IN2")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // No coin tube switch
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p Token") PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( j_supsha )
	PORT_INCLUDE( j_supsh )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("5p") PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("20p") PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( j_lt )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Shuffle")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Feature Stop")
	PORT_CONFNAME( 0x02, 0x02, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x02, "Full" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p Token") PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( j_plus2 )
	PORT_INCLUDE( j_lt )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("2p") PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( j_sup2p )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 ) PORT_NAME("Nudge 1")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 ) PORT_NAME("Nudge 2")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 ) PORT_NAME("Nudge 3")
	PORT_DIPNAME( 0x08, 0x00, "Percentage Stabiliser" )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x08, DEF_STR(On) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Shuffle")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Feature Stop")
	PORT_CONFNAME( 0x02, 0x02, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x02, "Full" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("20p") PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( j_la )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN0")
	PORT_CONFNAME( 0x80, 0x80, "1p/2p jumper" )
	PORT_CONFSETTING(    0x00, "1p" )
	PORT_CONFSETTING(    0x80, "2p" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Nudge Up")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Feature Stop")
	PORT_CONFNAME( 0x02, 0x02, "Coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x02, "Full" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("5p") PORT_IMPULSE(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("2p") PORT_IMPULSE(2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("1p") PORT_IMPULSE(2)
INPUT_PORTS_END

static INPUT_PORTS_START( j_cnudgr )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL ) PORT_NAME("Cancel/Gamble")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Feature Stop")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Nudge Up")
INPUT_PORTS_END

static INPUT_PORTS_START( j_lc )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN0")
	PORT_CONFNAME( 0x80, 0x80, "Game type" )
	PORT_CONFSETTING(    0x00, "Club Casino (£50, 5p stake)" )
	PORT_CONFSETTING(    0x80, "Lucky Casino (£100, 10p stake)" )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Self Test")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_POKER_CANCEL )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPNAME( 0x80, 0x00, "Win Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x80, DEF_STR(On) )

	PORT_MODIFY("IN2")
	PORT_DIPNAME( 0x01, 0x00, "Test Hold" ) // Always gives holds if door is open
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x01, DEF_STR(On) )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_GAMBLE_TAKE ) PORT_NAME("Take Win")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_GAMBLE_HALF ) PORT_NAME("Take Half")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gamble 8/1")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gamble 4/1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gamble Evens")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Gamble Odds")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_MODIFY("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("10p") PORT_IMPULSE(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("50p") PORT_IMPULSE(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Shuffle")
	PORT_CONFNAME( 0x08, 0x08, "10p coin tube" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x08, "Full" )
	PORT_CONFNAME( 0x10, 0x10, "50p coin tube A" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x10, "Full" )
	PORT_CONFNAME( 0x20, 0x20, "50p coin tube B" )
	PORT_CONFSETTING(    0x00, "Empty" )
	PORT_CONFSETTING(    0x20, "Full" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_INTERLOCK ) PORT_NAME("Back Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
INPUT_PORTS_END

static INPUT_PORTS_START( j_lal )
	PORT_INCLUDE( jpmsru_inputs )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Dump Credits")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Cancel/Collect")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Start/Gamble")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Take Bonus Points")

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_GAMBLE_PAYOUT )
	PORT_DIPNAME( 0x02, 0x00, "Test Hold" )
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )
	PORT_DIPSETTING(    0x02, DEF_STR(On) )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) // No refill key
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Fl 0,25") PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_NAME("Fl 1") PORT_IMPULSE(1)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Fl 0,50") PORT_IMPULSE(1)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_NAME("Fl 2,50") PORT_IMPULSE(1)

	PORT_START("DIP0")
	PORT_DIPNAME( 0x0f, 0x00, "Credit limit" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x02, "70" )
	PORT_DIPSETTING(    0x04, "100" )
	PORT_DIPSETTING(    0x08, "200" )
	PORT_DIPNAME( 0x10, 0x10, "Accept multiple coins" )
	PORT_DIPSETTING(    0x00, DEF_STR(No) )
	PORT_DIPSETTING(    0x10, DEF_STR(Yes) )
	PORT_DIPUNUSED( 0x20, 0x00 )
	PORT_DIPUNUSED( 0x40, 0x00 )
	PORT_DIPUNUSED( 0x80, 0x00 )

	PORT_START("DIP1")
	PORT_DIPUNUSED( 0x01, 0x00 )
	PORT_DIPUNUSED( 0x02, 0x00 )
	PORT_DIPUNUSED( 0x04, 0x00 )
	PORT_DIPUNUSED( 0x08, 0x00 )
	PORT_DIPNAME( 0xf0, 0x00, "Special symbol block chance" )
	// Chance for every spin of the machine blocking double bar from appearing on reel 2 and single/triple bar on reel 3
	PORT_DIPSETTING(    0xf0, "36%" )
	PORT_DIPSETTING(    0xe0, "38%" )
	PORT_DIPSETTING(    0xd0, "40%" )
	PORT_DIPSETTING(    0xc0, "42%" )
	PORT_DIPSETTING(    0xb0, "44%" )
	PORT_DIPSETTING(    0xa0, "46%" )
	PORT_DIPSETTING(    0x90, "48%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x10, "52%" )
	PORT_DIPSETTING(    0x20, "54%" )
	PORT_DIPSETTING(    0x30, "56%" )
	PORT_DIPSETTING(    0x40, "58%" )
	PORT_DIPSETTING(    0x50, "60%" )
	PORT_DIPSETTING(    0x60, "62%" )
	PORT_DIPSETTING(    0x70, "64%" )

	PORT_START("DIP2")
	PORT_DIPNAME( 0x0f, 0x00, "Gamble win chance" )
	PORT_DIPSETTING(    0x0f, "36%" )
	PORT_DIPSETTING(    0x0e, "38%" )
	PORT_DIPSETTING(    0x0d, "40%" )
	PORT_DIPSETTING(    0x0c, "42%" )
	PORT_DIPSETTING(    0x0b, "44%" )
	PORT_DIPSETTING(    0x0a, "46%" )
	PORT_DIPSETTING(    0x09, "48%" )
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x01, "52%" )
	PORT_DIPSETTING(    0x02, "54%" )
	PORT_DIPSETTING(    0x03, "56%" )
	PORT_DIPSETTING(    0x04, "58%" )
	PORT_DIPSETTING(    0x05, "60%" )
	PORT_DIPSETTING(    0x06, "62%" )
	PORT_DIPSETTING(    0x07, "64%" )
	PORT_DIPUNUSED( 0x10, 0x00 )
	PORT_DIPUNUSED( 0x20, 0x00 )
	PORT_DIPUNUSED( 0x40, 0x00 )
	PORT_DIPUNUSED( 0x80, 0x00 )
INPUT_PORTS_END

void jpmsru_state::machine_start()
{
	m_lamp.resolve();
	m_digits.resolve();

	save_item(NAME(m_reelbits));
	save_item(NAME(m_int1));
	save_item(NAME(m_int2));
	save_item(NAME(m_int1_en));
	save_item(NAME(m_int2_en));
	save_item(NAME(m_disp_digit));

	m_int1 = 0;
	m_int2 = 0;
	m_reelbits[0] = 0;
	m_reelbits[1] = 0;
	m_reelbits[2] = 0;
	m_reelbits[3] = 0;
	m_disp_digit = 0;
	m_busext_mode = 0;
	m_busext_addr = 0;
	m_logicext_addr = 0;
}

void jpmsru_state::device_post_load()
{
	const char reelnames[4][6] = { "reel1", "reel2", "reel3", "reel4" };
	for(int i = 0; i < 3; i++) awp_draw_reel(machine(), reelnames[i], *m_reel[i]);
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
		.add_route(ALL_OUTPUTS, "mono", 0.1);

	NETLIST_LOGIC_INPUT(config, m_audio_in[0], "IN1.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[1], "IN2.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[2], "IN3.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[3], "IN4.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[4], "IN5.IN", 0);
	NETLIST_LOGIC_INPUT(config, m_audio_in[5], "IN6.IN", 0);
	NETLIST_ANALOG_INPUT(config, "nl_audio:pot", "R8.DIAL");

	NETLIST_STREAM_OUTPUT(config, "nl_audio:cout0", 0, "OUT").set_mult_offset(1.0, -2.0);

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

	m_maincpu->set_addrmap(AS_PROGRAM, &jpmsru_state::jpmsru_4k_map);
}

// SRU with 6K ROM card
void jpmsru_state::jpmsru_6k(machine_config &config)
{
	jpmsru_3k(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &jpmsru_state::jpmsru_6k_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
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

void jpmsru_state::super2(machine_config &config)
{
	jpmsru_3k(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_super2);
}

void jpmsru_state::ews(machine_config &config)
{
	jpmsru_3k(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_ews);
}

void jpmsru_state::lt(machine_config &config)
{
	jpmsru_4k(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_ews);
}

void jpmsru_state::sup2p(machine_config &config)
{
	jpmsru_3k(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_sup2p);
}

void jpmsru_dac_state::lc(machine_config &config)
{
	jpmsru_6k(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_dac_state::outputs_lc);

	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void jpmsru_state::lal(machine_config &config)
{
	jpmsru_3k_busext(config);
	m_maincpu->set_addrmap(AS_IO, &jpmsru_state::outputs_lal);
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

ROM_START( j_ewnc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewn.1", 0x0000, 0x400, CRC(be7d3b79) SHA1(3304dcc69e93eca2e6e89df0b18afc6874ebacf0) )
	ROM_LOAD( "ewn.2", 0x0400, 0x400, CRC(bf19cd60) SHA1(77b0b439628589cb0db1b74a760b652519c20991) )
	ROM_LOAD( "ewn.3", 0x0800, 0x400, CRC(25138e03) SHA1(644fc6144ea74f08dc892f106ad494ba364afe86) )
ROM_END

ROM_START( j_ewnd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewn23.c1", 0x0000, 0x400, CRC(7c0f6d57) SHA1(0657f2a4bb70c7758138bae621e8c2a3f4c2c181) )
	ROM_LOAD( "ewn23.2", 0x0400, 0x400, CRC(3950ebb4) SHA1(395a1823ea92f0f84c65413739b75d399b8b386a) )
	ROM_LOAD( "ewn23.3", 0x0800, 0x400, CRC(410ff8b2) SHA1(36aa2207a6339e665ecb1834812d6fb3ec5f5fc8) )
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

ROM_START( j_ndub )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ndu.1", 0x0000, 0x400, CRC(20497d42) SHA1(3ae6bcc1cf62c2f28862d482b5425b4aea00aed5) )
	ROM_LOAD( "ndu.2", 0x0400, 0x400, CRC(9a7a3de6) SHA1(232f8223d0b1ee2a6703590193e3d74ac9debca1) )
	ROM_LOAD( "ndu.3", 0x0800, 0x400, CRC(012255c6) SHA1(c1303f855167bbe3d313ec440c142fd1a0253dcd) )
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

ROM_START( j_dudb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dud12.1", 0x0000, 0x400, CRC(2720ca33) SHA1(2b50cc8c01f10b4a536361eb5da0f72602808784) )
	ROM_LOAD( "dud12.2", 0x0400, 0x400, CRC(c87e98d9) SHA1(840f0f01aacb3df1cce4d75635de476bd615680b) )
	ROM_LOAD( "dud12.3", 0x0800, 0x400, CRC(1ffb8513) SHA1(7d10f9303ab91b413dfc316321e12ca930880b23) )
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

ROM_START( j_lanb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lan.1", 0x0000, 0x400, CRC(3c3050a9) SHA1(27bdec979410533d1b7dbd447542123eccd21607) )
	ROM_LOAD( "lan.2", 0x0400, 0x400, CRC(c4018d6f) SHA1(c0b5a172324ab78b48329aa78bc66223cb133aa0) )
	ROM_LOAD( "lan.3", 0x0800, 0x400, CRC(8f41078e) SHA1(f9028264688b31d7ef5f9697fc93bcf4d2131f84) )
ROM_END

ROM_START( j_super2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "super2.1", 0x0000, 0x400, CRC(a1df2719) SHA1(eed80329c14ef6c272a8c622e8a4bc7d14ac87e6) )
	ROM_LOAD( "super2.2", 0x0400, 0x400, CRC(0fd5ddd0) SHA1(e8d31b009b29486d36d11052af857c609a7f1f84) )
	ROM_LOAD( "super2.3", 0x0800, 0x400, CRC(ddd998d3) SHA1(5964da70ae4c2f174dc3d1494fc67579c221a7b7) )
ROM_END

ROM_START( j_ews )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ews8.a1", 0x0000, 0x400, CRC(52e9709a) SHA1(0b437834f48ca7718e0b30303916eed00c7fb4c9) )
	ROM_LOAD( "ews8.2", 0x0400, 0x400, CRC(ee4a4809) SHA1(292a12a5ddc5a22c8568016b34dfec7959f49027) )
	ROM_LOAD( "ews8.3", 0x0800, 0x400, CRC(3700a7a3) SHA1(cf24a54e6aa3a3a86ff75f6e8bcb692d0cfd0e80) )
ROM_END

ROM_START( j_ewsa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ews13.a1", 0x0000, 0x400, CRC(d02d2983) SHA1(9e9e106083dfea44228ae56e73a3fe7184fdb473) )
	ROM_LOAD( "ews13.2", 0x0400, 0x400, CRC(6b93d262) SHA1(39fce614845ba1d59e27e678318b1a9331797a9b) )
	ROM_LOAD( "ews13.3", 0x0800, 0x400, CRC(4d8e197a) SHA1(1569327f0e4b5d7632658b69abf59076effb2600) )
ROM_END

ROM_START( j_ewsb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ews13.c1", 0x0000, 0x400, CRC(2eec7c4d) SHA1(a1740d27e60192659392ba7602b9b62947c4f6db) )
	ROM_LOAD( "ews13.b2", 0x0400, 0x400, CRC(b84b7858) SHA1(90fd64881d52e1f4362ccbcb9434dbf7b25b97f9) )
	ROM_LOAD( "ews13.3", 0x0800, 0x400, CRC(4d8e197a) SHA1(1569327f0e4b5d7632658b69abf59076effb2600) )
ROM_END

ROM_START( j_ewsdlx )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ewsdlx.1", 0x0000, 0x400, CRC(b628ef7d) SHA1(cd10a0bbaefd93cfbd14f1398de2d3a528760806) )
	ROM_LOAD( "ewsdlx.2", 0x0400, 0x400, CRC(fd36fd6a) SHA1(670418bc6a36712030a77a99c62c44e70451f30d) )
	ROM_LOAD( "ewsdlx.3", 0x0800, 0x400, CRC(e4eed790) SHA1(64049e0a2f4ec6b72aede34cbcb0ce287844d247) )
ROM_END

ROM_START( j_ssh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ssh.1", 0x0000, 0x400, CRC(7bbf8c00) SHA1(edb59505abeae28d3ecd9c7f8f1f49ba1eaf72ab) )
	ROM_LOAD( "ssh.2", 0x0400, 0x400, CRC(97100cbd) SHA1(154e1348e48a4c9170c7b85e51ec8d381ea420f7) )
	ROM_LOAD( "ssh.3", 0x0800, 0x400, CRC(a01848d7) SHA1(bd6b093655855aa4870bb319191ae690abebbece) )
ROM_END

ROM_START( j_supsh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supsh.1", 0x0000, 0x400, CRC(e2f28e4f) SHA1(1ace28cd0da0a31264f5c7f7e885f2df1dff22d2) )
	ROM_LOAD( "supsh.2", 0x0400, 0x400, CRC(028ea40f) SHA1(541e450153f0076c63cd0f1f3074d0a44717ee11) )
	ROM_LOAD( "supsh.3", 0x0800, 0x400, CRC(9e871667) SHA1(b2119b119173e7a9f23a124cecc1efca7131a543) )
ROM_END

ROM_START( j_supsha )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "supsh.1", 0x0000, 0x400, CRC(1ed7a9a3) SHA1(1f11566a0e28f5f792cabd983b90e1e4ce5cd7a7) )
	ROM_LOAD( "supsh.2", 0x0400, 0x400, CRC(6358f793) SHA1(eebafbac489bec2864f114ebd1d79d76453a1807) )
	ROM_LOAD( "supsh.3", 0x0800, 0x400, CRC(b1e5467b) SHA1(6756e35ca0285313f5dab266967b12ec3e55af2c) )
ROM_END

// LUCKY TWOS REV
ROM_START( j_lt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lt9.1", 0x0000, 0x400, CRC(206a7c9a) SHA1(44723d7f8428ad3f7d802b9465634d81ba8753da) )
	ROM_LOAD( "lt9.2", 0x0400, 0x400, CRC(6e1cd083) SHA1(17edaa9880ae2a6d6d99e771e41b985527d5ed3b) )
	ROM_LOAD( "lt9.3", 0x0800, 0x400, CRC(d6881e6f) SHA1(42a83f01d67a8f530ca2a10ffeff30237bdfba94) )
	ROM_LOAD( "lt9.4", 0x1000, 0x400, CRC(97236ce3) SHA1(f71861576f33daec3e1d371c670b535e6fd32b5e) )
ROM_END

// LUCKY TWOS REV
ROM_START( j_ts )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ts.1", 0x0000, 0x400, CRC(414c9541) SHA1(2351170c21c92c46f000d8bcf5a46651084e807c) )
	ROM_LOAD( "ts.2", 0x0400, 0x400, CRC(fe245c53) SHA1(750a7b6f04ae194e3c59412d6aefc9975a7906ee) )
	ROM_LOAD( "ts.3", 0x0800, 0x400, CRC(83cef11a) SHA1(7cc9b1ccedd2fe23fbfd7ff7ec2e3097397ca736) )
	ROM_LOAD( "ts.4", 0x1000, 0x400, CRC(da323129) SHA1(76f68919e59093f6925842ba87062b102bccd5b9) )
ROM_END

ROM_START( j_plus2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "plus2.1", 0x0000, 0x400, CRC(f635174d) SHA1(9478aabc0eaa25d4ae44d2385e738584f03f6647) )
	ROM_LOAD( "plus2.2", 0x0400, 0x400, CRC(0999d32f) SHA1(e08c852f8f3aff8ab7b73e9c0b0502ab91f9e844) )
	ROM_LOAD( "plus2.3", 0x0800, 0x400, CRC(d3dfd6ab) SHA1(4cf0f8977fb2c023bf2ccc8d9d74352ce32206bf) )
	ROM_LOAD( "plus2.4", 0x1000, 0x400, CRC(8b6922b4) SHA1(7b7fc7b0708bf96846860254fea957bcbc952923) )
ROM_END

ROM_START( j_goldn2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "goldn2.1", 0x0000, 0x400, CRC(6179002b) SHA1(a0b3311e2d2db6e88954cec681dff4ed50230826) )
	ROM_LOAD( "goldn2.2", 0x0400, 0x400, CRC(0999d32f) SHA1(e08c852f8f3aff8ab7b73e9c0b0502ab91f9e844) )
	ROM_LOAD( "goldn2.3", 0x0800, 0x400, CRC(d3dfd6ab) SHA1(4cf0f8977fb2c023bf2ccc8d9d74352ce32206bf) )
	ROM_LOAD( "goldn2.4", 0x1000, 0x400, CRC(8b6922b4) SHA1(7b7fc7b0708bf96846860254fea957bcbc952923) )
ROM_END

ROM_START( j_sup2p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sup2p.1", 0x0000, 0x400, CRC(5a7349e6) SHA1(23227892ed10b4c43e987c8448053e5fa1242d39) )
	ROM_LOAD( "sup2p.2", 0x0400, 0x400, CRC(e0eac683) SHA1(101653ab839f961b7dfd7e530418c00d4edb7ca1) )
	ROM_LOAD( "sup2p.3", 0x0800, 0x400, CRC(14794c2b) SHA1(2f471530d524db6bab68818138760c0c22a032d5) )
ROM_END

ROM_START( j_la )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "la.1", 0x0000, 0x400, CRC(21076280) SHA1(d5cf25d289f03c743f4428273ac002df3164c344) )
	ROM_LOAD( "la.2", 0x0400, 0x400, CRC(cae10bc1) SHA1(a740946437a3b277b714f13d001783987f57bc77) )
	ROM_LOAD( "la.3", 0x0800, 0x400, CRC(cb9362ac) SHA1(a16d43ba01b24e1b515881957c1559d33a03bcc4) )
ROM_END

// CASH NUDGER RV
ROM_START( j_cnudgr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cnudgr.1", 0x0000, 0x400, CRC(3936c450) SHA1(bc7cd4698c5eb8fbbf990f8e55ddfdde0f48a387) )
	ROM_LOAD( "cnudgr.2", 0x0400, 0x400, CRC(0b52b71e) SHA1(ce981baaeb254ec1f28d4dd030ac96f96405392f) )
	ROM_LOAD( "cnudgr.3", 0x0800, 0x400, CRC(9e956248) SHA1(0ade65e9e48a240a5aaf0738ee64b374bee15c92) )
ROM_END

ROM_START( j_lc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc8.a1", 0x0000, 0x400, CRC(48cf1e66) SHA1(22df81e7c82a009fbac0a3974f670d716820e6db) )
	ROM_LOAD( "lc8.2", 0x0400, 0x400, CRC(bfc3065d) SHA1(8a80bd1bbd222c84c678c1117c1c0b1f72ed1ffe) )
	ROM_LOAD( "lc8.3", 0x0800, 0x400, CRC(615e2942) SHA1(e5bf8247fe0362c85800110bdf1334492bb14d47) )
	ROM_LOAD( "lc8.4", 0x1000, 0x400, CRC(950b393a) SHA1(74d6867d170fb74e183595b94c5aae5501e04b27) )
	ROM_LOAD( "lc8.5", 0x1800, 0x400, CRC(76a720bc) SHA1(c7e6a8e2e2d1b5e6fcfd560ce7a93c4b5775cef8) )
ROM_END

ROM_START( j_lca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc8.1", 0x0000, 0x400, CRC(ca6f21b8) SHA1(a538b6d02c44f13568a021ccc34d8deeae02e742) )
	ROM_LOAD( "lc8.2", 0x0400, 0x400, CRC(bfc3065d) SHA1(8a80bd1bbd222c84c678c1117c1c0b1f72ed1ffe) )
	ROM_LOAD( "lc8.3", 0x0800, 0x400, CRC(615e2942) SHA1(e5bf8247fe0362c85800110bdf1334492bb14d47) )
	ROM_LOAD( "lc8.4", 0x1000, 0x400, CRC(950b393a) SHA1(74d6867d170fb74e183595b94c5aae5501e04b27) )
	ROM_LOAD( "lc8.5", 0x1800, 0x400, CRC(76a720bc) SHA1(c7e6a8e2e2d1b5e6fcfd560ce7a93c4b5775cef8) )
ROM_END

ROM_START( j_lal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lal52.1", 0x0000, 0x400, CRC(b436dca1) SHA1(9c74eca6a20409b2f5804356aba9ca75f6b2ca90) )
	ROM_LOAD( "lal52.2", 0x0400, 0x400, CRC(757ac03b) SHA1(e7c081f133e59e51298831e93ebd539f6d8e193b) )
	ROM_LOAD( "lal52.3", 0x0800, 0x400, CRC(332e606b) SHA1(846983383a9f7ea781177712a3e4532902ae86f7) )
ROM_END

} // anonymous namespace

#define GAME_FLAGS MACHINE_NOT_WORKING|MACHINE_MECHANICAL|MACHINE_REQUIRES_ARTWORK|MACHINE_SUPPORTS_SAVE

// AWP
GAMEL( 1979?, j_ewn,     0,        ewn,       j_ewn,         jpmsru_state, empty_init, ROT0, "JPM", u8"Each Way Nudger (JPM) (SRU) (revision 20, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_ewn )
GAMEL( 1981?, j_ewna,    j_ewn,    ewn2,      j_ewn2,        jpmsru_state, empty_init, ROT0, "JPM", u8"Each Way Nudger (JPM) (SRU) (revision 26A, 5p/10p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_ewn )
GAMEL( 1981?, j_ewnb,    j_ewn,    ewn2,      j_ewn2,        jpmsru_state, empty_init, ROT0, "JPM", u8"Each Way Nudger (JPM) (SRU) (5p/10p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_ewn )
GAMEL( 1979?, j_ewnc,    j_ewn,    ewn,       j_ewn,         jpmsru_state, empty_init, ROT0, "JPM", u8"Each Way Nudger (JPM) (SRU) (earlier, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_ewn )
GAMEL( 1979?, j_ewnd,    j_ewn,    ewn2,      j_ewn1,        jpmsru_state, empty_init, ROT0, "JPM", u8"Each Way Nudger (JPM) (SRU) (revision 23C, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_ewn )
GAMEL( 1979?, j_ndu,     0,        ndu,       j_ndu17,       jpmsru_state, empty_init, ROT0, "JPM", u8"Nudge Double Up (JPM) (SRU) (revision 17, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_ndu )
GAMEL( 1979?, j_ndua,    j_ndu,    ndu,       j_ndu17_alt,   jpmsru_state, empty_init, ROT0, "JPM", u8"Nudge Double Up (JPM) (SRU) (revision 17, 5p Stake, £1 Jackpot, lower %)", GAME_FLAGS, layout_j_ndu )
GAMEL( 1979?, j_ndub,    j_ndu,    ndu,       j_ndu,         jpmsru_state, empty_init, ROT0, "JPM", u8"Nudge Double Up (JPM) (SRU) (earlier?, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_ndu )
GAMEL( 1980?, j_dud,     0,        dud,       j_dud,         jpmsru_state, empty_init, ROT0, "JPM", u8"Nudge Double Up Deluxe (JPM) (SRU) (revision 10, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_dud )
GAMEL( 1981?, j_duda,    j_dud,    dud,       j_dud2,        jpmsru_state, empty_init, ROT0, "JPM", u8"Nudge Double Up Deluxe (JPM) (SRU) (5p/10p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_dud )
GAMEL( 1981?, j_dudb,    j_dud,    dud,       j_dud2,        jpmsru_state, empty_init, ROT0, "JPM", u8"Nudge Double Up Deluxe (JPM) (SRU) (revision 12, £2 Jackpot)", GAME_FLAGS, layout_j_dud )
GAMEL( 1981?, j_dt,      j_dud,    dud,       j_dud2,        jpmsru_state, empty_init, ROT0, "JPM", u8"Double Top (JPM) (SRU) (revision 13, 5p/10p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_dud )
GAMEL( 1980?, j_lan,     0,        lan,       j_lan,         jpmsru_state, empty_init, ROT0, "JPM", u8"Lite A Nudge (JPM) (SRU) (revision 17F, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_lan )
GAMEL( 1981?, j_lana,    j_lan,    lan,       j_lan2,        jpmsru_state, empty_init, ROT0, "JPM", u8"Lite A Nudge (JPM) (SRU) (5p/10p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_lan )
GAMEL( 1980?, j_lanb,    j_lan,    lan,       j_lan,         jpmsru_state, empty_init, ROT0, "JPM", u8"Lite A Nudge (JPM) (SRU) (earlier, 5p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_lan )
GAMEL( 198?,  j_super2,  j_lan,    super2,    j_super2,      jpmsru_state, empty_init, ROT0, "<unknown>", u8"Super 2 (SRU) (2p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_super2 ) // £1/2p rebuild of Lite A Nudge
GAMEL( 1981,  j_ews,     0,        ews,       j_ews,         jpmsru_state, empty_init, ROT0, "JPM", u8"Each Way Shuffle (JPM) (SRU) (revision 8A, 5p/10p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_ews )
GAMEL( 1981,  j_ewsa,    j_ews,    ews,       j_ews,         jpmsru_state, empty_init, ROT0, "JPM", u8"Each Way Shuffle (JPM) (SRU) (revision 13A, 5p/10p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_ews )
GAMEL( 1981,  j_ewsb,    j_ews,    ews,       j_ews,         jpmsru_state, empty_init, ROT0, "JPM", u8"Each Way Shuffle (JPM) (SRU) (revision 13C, 5p/10p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_ews )
GAMEL( 1984?, j_ewsdlx,  j_ews,    ews,       j_ewsdlx,      jpmsru_state, empty_init, ROT0, "CTL", u8"Each Way Shuffle Deluxe (CTL) (SRU) (5p/10p Stake, £3 Jackpot)", GAME_FLAGS, layout_j_ewsdlx ) // £3 rebuild of Each Way Shuffle
GAMEL( 1984?, j_ssh,     j_ews,    ews,       j_ssh,         jpmsru_state, empty_init, ROT0, "CTL", u8"Silver Shuffle (CTL) (SRU) (2p Stake, £1.50 Jackpot)", GAME_FLAGS, layout_j_ssh ) // £1.50/2p rebuild of Each Way Shuffle
GAMEL( 1984?, j_supsh,   j_ews,    ews,       j_supsh,       jpmsru_state, empty_init, ROT0, "Louth Coin", u8"Super Shuffle (Louth Coin) (SRU) (10p Stake, £3 Jackpot)", GAME_FLAGS, layout_j_supsh ) // Rebuild of Each Way Shuffle, adds an extra symbol
GAMEL( 1984?, j_supsha,  j_ews,    ews,       j_supsha,      jpmsru_state, empty_init, ROT0, "Louth Coin", u8"Super Shuffle (Louth Coin) (SRU) (5p Stake, £1.50 Jackpot)", GAME_FLAGS, layout_j_supsh )
GAMEL( 1981,  j_lt,      0,        lt,        j_lt,          jpmsru_state, empty_init, ROT0, "JPM", u8"Lucky 2's (JPM) (SRU) (revision 9, 10p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_lt )
GAMEL( 1982,  j_ts,      j_lt,     lt,        j_lt,          jpmsru_state, empty_init, ROT0, "JPM", u8"Two Step (JPM) (SRU) (5p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_lt )
GAMEL( 198?,  j_plus2,   j_lt,     lt,        j_plus2,       jpmsru_state, empty_init, ROT0, "CTL", u8"Plus 2 (CTL) (SRU) (2p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_plus2 ) // £1/2p rebuild of Lucky 2's
GAMEL( 1984?, j_goldn2,  j_lt,     lt,        j_plus2,       jpmsru_state, empty_init, ROT0, "CTL", u8"Golden 2's (CTL) (SRU) (2p Stake, £1.50 Jackpot)", GAME_FLAGS, layout_j_plus2 ) // £1.50 JP version of above
GAMEL( 198?,  j_sup2p,   0,        sup2p,     j_sup2p,       jpmsru_state, empty_init, ROT0, "Mdm", u8"Super 2p Shuffle (Mdm) (SRU) (2p Stake, £1 Jackpot)", GAME_FLAGS, layout_j_sup2p )
GAMEL( 1984?, j_la,      0,        lan,       j_la,          jpmsru_state, empty_init, ROT0, "<unknown>", u8"Lucky Aces (SRU) (1p/2p Stake, £1.50 Jackpot)", GAME_FLAGS, layout_j_la )
GAMEL( 198?,  j_cnudgr,  0,        lan,       j_cnudgr,      jpmsru_state, empty_init, ROT0, "<unknown>", u8"Cash Nudger? (SRU) (5p Stake, £2 Jackpot)", GAME_FLAGS, layout_j_cnudgr )
// Club
GAMEL( 1981,  j_lc,      0,        lc,        j_lc,          jpmsru_dac_state, empty_init, ROT0, "JPM", "Lucky Casino (JPM) (SRU) (revision 8A)", GAME_FLAGS, layout_j_lc )
GAMEL( 1981,  j_lca,     j_lc,     lc,        j_lc,          jpmsru_dac_state, empty_init, ROT0, "JPM", "Lucky Casino (JPM) (SRU) (revision 8, lower %)", GAME_FLAGS, layout_j_lc ) // Smaller hold chance, probably revision 8B/8C
// Dutch
GAMEL( 1979?, j_lal,     0,        lal,       j_lal,         jpmsru_state, empty_init, ROT0, "JPM", "Lite a Line (Dutch) (JPM) (SRU) (revision 52)", GAME_FLAGS, layout_j_lal )

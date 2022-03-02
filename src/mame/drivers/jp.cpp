// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/******************************************************************************************************

PINBALL
Juegos Populares

All manuals are in Spanish.
Schematic has a number of errors and omissions, so referring to PinMAME.

Games:
- Petaco (newer hardware)
- Faeton
- Halley Comet
- Aqualand
- America 1492
- Olympus
- Lortium
- Pimbal (Pinball 3000)
- Petaco (older hardware) is in peyper.cpp

Status:
- All games are Playable
- Pimbal is multiball, so press XY to end a ball.
- petacon, petacona: There is no ball digit, it's a set of 5 lamps like EM machines.
    The ball digit segments E,F,G,H indicate which player's turn, by flashing.

ToDo:
- Mechanical sounds

*******************************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/z80/z80.h"
#include "machine/74157.h"
#include "machine/74259.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "speaker.h"

#include "jp.lh"


namespace {

class jp_state : public genpin_class
{
public:
	jp_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_latch(*this, "latch%u", 0U)
		, m_io_keyboard(*this, "X%d", 0U)
		, m_msm(*this, "msm")
		, m_adpcm_select(*this, "adpcm_select")
		, m_adpcm_bank(*this, "adpcm_bank")
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void init_6d() { m_game = 1; }
	void jp(machine_config &config);
	void jps(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	u8 porta_r();
	u8 portb_r();
	void out1_w(offs_t offset, u8 data);
	void out2_w(offs_t offset, u8 data);
	DECLARE_WRITE_LINE_MEMBER(disp_data_w);
	DECLARE_WRITE_LINE_MEMBER(disp_clock_w);
	DECLARE_WRITE_LINE_MEMBER(disp_strobe_w);
	DECLARE_WRITE_LINE_MEMBER(row_w);
	void sample_bank_w(u8 data);
	void adpcm_reset_w(u8 data);
	DECLARE_WRITE_LINE_MEMBER(vck_w);
	IRQ_CALLBACK_MEMBER(sound_int_cb);

	void main_map(address_map &map);
	void audio_map(address_map &map);

	void update_display();

	u32 m_disp_data = 0U;
	bool m_adpcm_ff = 0;
	bool m_game = 0;

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device_array<ls259_device, 10> m_latch;
	required_ioport_array<8> m_io_keyboard;
	optional_device<msm5205_device> m_msm;
	optional_device<ls157_device> m_adpcm_select;
	optional_memory_bank m_adpcm_bank;
	output_finder<100> m_digits;
	output_finder<80> m_io_outputs;   // 16 solenoids + 64 lamps
};


void jp_state::main_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).mirror(0x1800).ram().share("nvram"); // ram-"5128" battery backed
	map(0x6000, 0x6000).mirror(0x1ffc).w("ay", FUNC(ay8910_device::address_w));
	map(0x6001, 0x6001).mirror(0x1ffc).r("ay", FUNC(ay8910_device::data_r));
	map(0x6002, 0x6002).mirror(0x1ffc).w("ay", FUNC(ay8910_device::data_w));
	map(0xa000, 0xa007).mirror(0x1ff8).w(FUNC(jp_state::out1_w));
	map(0xc000, 0xc007).mirror(0x1ff8).w(FUNC(jp_state::out2_w));
}

void jp_state::audio_map(address_map &map)
{
	map(0x0000, 0x3fff).rom(); // includes ADPCM data from 0x0400 to 0x3fff
	map(0x4000, 0x47ff).ram();
	map(0x5000, 0x5000).w(FUNC(jp_state::sample_bank_w));
	map(0x6000, 0x6000).w(m_adpcm_select, FUNC(ls157_device::ba_w));
	map(0x7000, 0x7000).w(FUNC(jp_state::adpcm_reset_w));
	map(0x8000, 0xffff).bankr("adpcm_bank");
}

static INPUT_PORTS_START( jp )
	PORT_START("X0")
	PORT_DIPNAME( 0x01, 0x01, "SW 1")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "SW 2")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "SW 3")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x08, "SW 5")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x10, "SW 6")
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "SW 7")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "SW 8")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x80, "SW 9")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))

	PORT_START("X1")
	PORT_DIPNAME( 0x01, 0x01, "SW 10")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "SW 13")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "Balls") // SW12 confirmed
	PORT_DIPSETTING(    0x04, "3")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPNAME( 0x08, 0x08, "SW 14")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x10, 0x10, "SW 4") // confirmed
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "SW 15")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "SW 11") // confirmed
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x80, "SW 16")
	PORT_DIPSETTING(    0x80, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))

	PORT_START("X2")
	PORT_DIPNAME( 0x01, 0x01, "SW A")
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x02, 0x02, "SW B")
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x04, 0x04, "SW C")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x08, 0x08, "SW D")
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP30")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP31")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP32")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP33")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP34")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP35")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP36")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP37")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP40")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP41")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP42")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP43")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP44")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP45")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP46")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP47")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP50")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP51")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP52")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP53")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP54")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP55")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP56")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP57")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP67")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP70")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP71")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP72")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP73")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP74")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP75")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP76")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP77")
INPUT_PORTS_END

void jp_state::out1_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 8; i++)
	{
		m_latch[i]->write_bit(offset, BIT(data, i));
		m_io_outputs[i*8U+offset] = BIT(data, i);
	}
}

void jp_state::out2_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 2; i++)
	{
		m_latch[8 + i]->write_bit(offset, BIT(data, i));
		m_io_outputs[64U+i*8U+offset] = BIT(data, i);
	}
}

WRITE_LINE_MEMBER(jp_state::row_w)
{
}

WRITE_LINE_MEMBER(jp_state::disp_data_w)
{
}

WRITE_LINE_MEMBER(jp_state::disp_clock_w)
{
	if (state)
	{
		bool data_bit = !m_latch[0]->q1_r();
		m_disp_data = (m_disp_data << 1) | data_bit;
	}
	if (!m_latch[2]->q1_r())
		update_display();
}

WRITE_LINE_MEMBER(jp_state::disp_strobe_w)
{
	if (state)
		update_display();
}

void jp_state::update_display()
{
	if (m_game)  // 6-digit display
	{
		u8 segment = BIT(m_disp_data, 24, 8);
		if (segment == 0)
		{ // ball number
			u8 segment = BIT(m_disp_data, 6, 8);
			m_digits[94] = bitswap<8>(segment, 0, 1, 2, 3, 4, 5, 6, 7) ^ 0xff;
		}
		else
		{
			for (u8 i = 0; i < 24; i++)
				if (BIT(m_disp_data, i))
					m_digits[i] = m_digits[i] & ~segment;
				else
					m_digits[i] = m_digits[i] | segment;
		}
	}
	else
	{       // 7-segment display
		u8 segment, t = BIT(m_disp_data, 24, 4);
		if (t == 8)
		{ // ball number
			segment = BIT(m_disp_data, 6, 8);
			m_digits[94] = bitswap<8>(segment, 0, 1, 2, 3, 4, 5, 6, 7) ^ 0xff;
		}
		else
		{ // main displays
			if (t == 7)
				segment = 128;
			else
				segment = 1 << (6-t);

			for (u8 i = 0; i < 32; i++)
				if (BIT(m_disp_data, i))
					m_digits[i] = m_digits[i] & ~segment;
				else
					m_digits[i] = m_digits[i] | segment;
		}
	}
}

u8 jp_state::porta_r()
{
	u8 result = 0xff;
	if (m_latch[3]->q1_r() == 0)
		result &= m_io_keyboard[1]->read();
	if (m_latch[4]->q1_r() == 0)
		result &= m_io_keyboard[7]->read();
	if (m_latch[7]->q1_r() == 0)
		result &= m_io_keyboard[2]->read();
	return result;
}

u8 jp_state::portb_r()
{
	u8 result = 0xff;
	if (m_latch[3]->q1_r() == 0)
		result &= m_io_keyboard[0]->read();
	if (m_latch[4]->q1_r() == 0)
		result &= m_io_keyboard[3]->read();
	if (m_latch[5]->q1_r() == 0)
		result &= m_io_keyboard[4]->read();
	if (m_latch[6]->q1_r() == 0)
		result &= m_io_keyboard[5]->read();
	if (m_latch[7]->q1_r() == 0)
		result &= m_io_keyboard[6]->read();
	return result;
}

void jp_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	if (m_adpcm_bank.found())
		m_adpcm_bank->configure_entries(0, 16, memregion("sound1")->base(), 0x8000);

	m_disp_data = 0;
	m_adpcm_ff = false;

	save_item(NAME(m_disp_data));
	save_item(NAME(m_adpcm_ff));
}

void jp_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
	for (u8 i = 24; i < 32; i++)
		m_digits[i] = 0;

	//m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	m_digits[96] = 0x3f;
	m_digits[97] = 0x3f;
	m_digits[98] = 0x3f;
	m_digits[99] = 0x3f;
}

void jp_state::jp(machine_config &config)

{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &jp_state::main_map);
	m_maincpu->set_periodic_int(FUNC(jp_state::irq0_line_hold), attotime::from_hz(8_MHz_XTAL / 8192)); // 4020 divider

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	LS259(config, m_latch[0]);
	m_latch[0]->q_out_cb<1>().set(FUNC(jp_state::disp_data_w)).invert();

	LS259(config, m_latch[1]);
	m_latch[1]->q_out_cb<1>().set(FUNC(jp_state::disp_clock_w)).invert();

	LS259(config, m_latch[2]);
	m_latch[2]->q_out_cb<1>().set(FUNC(jp_state::disp_strobe_w)).invert();

	LS259(config, m_latch[3]);
	m_latch[3]->q_out_cb<1>().set(FUNC(jp_state::row_w));

	LS259(config, m_latch[4]);
	m_latch[4]->q_out_cb<1>().set(FUNC(jp_state::row_w));

	LS259(config, m_latch[5]);
	m_latch[5]->q_out_cb<1>().set(FUNC(jp_state::row_w));

	LS259(config, m_latch[6]);
	m_latch[6]->q_out_cb<1>().set(FUNC(jp_state::row_w));

	LS259(config, m_latch[7]);
	m_latch[7]->q_out_cb<1>().set(FUNC(jp_state::row_w));

	LS259(config, m_latch[8]);

	LS259(config, m_latch[9]);

	/* Video */
	config.set_default_layout(layout_jp);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "ayvol").front_center();
	ay8910_device &ay(AY8910(config, "ay", 8_MHz_XTAL / 4));
	ay.port_a_read_callback().set(FUNC(jp_state::porta_r));
	ay.port_b_read_callback().set(FUNC(jp_state::portb_r));
	ay.add_route(ALL_OUTPUTS, "ayvol", 0.9);
}

void jp_state::sample_bank_w(u8 data)
{
	m_adpcm_bank->set_entry(data & 15);
}

void jp_state::adpcm_reset_w(u8 data)
{
	m_msm->reset_w(BIT(data, 0));
}

WRITE_LINE_MEMBER(jp_state::vck_w)
{
	if (state)
	{
		m_adpcm_ff = !m_adpcm_ff;
		m_adpcm_select->select_w(m_adpcm_ff);
		if (m_adpcm_ff)
			m_audiocpu->set_input_line(0, ASSERT_LINE);
	}
}

IRQ_CALLBACK_MEMBER(jp_state::sound_int_cb)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}

void jp_state::jps(machine_config &config)
{
	jp(config);
	Z80(config, m_audiocpu, 8_MHz_XTAL / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &jp_state::audio_map);
	m_audiocpu->set_irq_acknowledge_callback(FUNC(jp_state::sound_int_cb));

	LS157(config, m_adpcm_select, 0); // not labeled in manual; might even be a CD4019
	m_adpcm_select->out_callback().set("msm", FUNC(msm5205_device::data_w));

	SPEAKER(config, "msmvol").front_center();
	MSM5205(config, m_msm, 384'000); // not labeled in manual; clock unknown
	m_msm->vck_callback().set(FUNC(jp_state::vck_w));
	m_msm->set_prescaler_selector(msm5205_device::S48_4B); // unknown
	m_msm->add_route(ALL_OUTPUTS, "msmvol", 1.0);

	m_latch[9]->q_out_cb<5>().set_inputline("audiocpu", INPUT_LINE_NMI); // only external input for sound board
}

/*-------------------------------------------------------------------
/ America 1492 #1107
/-------------------------------------------------------------------*/
ROM_START(america)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("cpvi1492.dat", 0x0000, 0x2000, CRC(e1d3bd57) SHA1(049c17cd717404e58339100ab8efd4d6bf8ee791))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("sbvi1492.dat", 0x0000, 0x4000, CRC(38934e06) SHA1(eef850a5096a7436b728921aed22fe5f3d85b4ee))

	ROM_REGION(0x80000, "sound1", 0)
	ROM_LOAD("b1vi1492.dat", 0x0000, 0x8000, CRC(e93083ed) SHA1(6a44675d8cc8b8af40091646f589b833245bf092))
	ROM_LOAD("b2vi1492.dat", 0x8000, 0x8000, CRC(88be85a0) SHA1(ebf9d88847d6fd787892f0a34258f38e48445014))
	ROM_LOAD("b3vi1492.dat", 0x10000, 0x8000, CRC(1304c87b) SHA1(f84eb3116dd9841892f46106f9443c09cc094675))
	ROM_LOAD("b4vi1492.dat", 0x18000, 0x8000, CRC(831e4033) SHA1(f51f3f5a226692caed59e4aac0843cdb40f0667d))
	ROM_LOAD("b5vi1492.dat", 0x20000, 0x8000, CRC(46ee29a5) SHA1(08d756f5a0430aca723f842951dd8520024859b0))
	ROM_LOAD("b6vi1492.dat", 0x28000, 0x8000, CRC(5180d751) SHA1(6c2e8edf606d24d86f4ab6da4adaf1d1095e9b19))
	ROM_LOAD("b7vi1492.dat", 0x30000, 0x8000, CRC(ba98138f) SHA1(2c8ef3b17972b7022afdf89c6280d02038b65501))
ROM_END

/*-------------------------------------------------------------------
/ Aqualand #1105
/-------------------------------------------------------------------*/
ROM_START(aqualand)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("jpaqcpu", 0x0000, 0x2000, CRC(53230fab) SHA1(0b049f3be412be598982537e7fa7abf9b2766a16))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("jpaqsds", 0x0000, 0x4000, CRC(ff1e0cd2) SHA1(ef58d2b59929c7250dd30c413a3ba31ebfd7e09d))

	ROM_REGION(0x80000, "sound1", 0)
	ROM_LOAD("jpaq-1sd", 0x0000, 0x8000, CRC(7cdf2f7a) SHA1(e00482a6accd11e96fd0d444b3167b7d36332f7b))
	ROM_LOAD("jpaq-2sd", 0x8000, 0x8000, CRC(db05c774) SHA1(2d40410b70de6ab0de57e94c6d8ada6e8a4a2050))
	ROM_LOAD("jpaq-3sd", 0x10000, 0x8000, CRC(df38304e) SHA1(ec6f0c99764e3c3fe7e1de09b2d9b59d85d168d5))
	ROM_LOAD("jpaq-4sd", 0x18000, 0x8000, CRC(8065c03e) SHA1(0731cb76d3be117a82c4ad5b7e23b53e05b3a95a))
	ROM_LOAD("jpaq-5sd", 0x20000, 0x8000, CRC(a387a1a6) SHA1(20abee033a33e388a5f2ed3896a650766b62cfa2))
	ROM_LOAD("jpaq-6sd", 0x28000, 0x8000, CRC(55076afb) SHA1(68b86e6855b2a80e37d2fb172bb0c4fa107d4aba))
	ROM_LOAD("jpaq-7sd", 0x30000, 0x8000, CRC(67675b5b) SHA1(52b7cb310ddeff0bde7f0dfd37f61ab09964a75d))
	ROM_LOAD("jpaq-8sd", 0x38000, 0x8000, CRC(c9d2d30e) SHA1(ee504b0e2aa69f541c3f4d245cc6525a7c920fa7))
	ROM_LOAD("jpaq-9sd", 0x40000, 0x8000, CRC(3bc45f9f) SHA1(6d838b1ba94087f9a29af016b68125400dcf1fe5))
	ROM_LOAD("jpaq10sd", 0x48000, 0x8000, CRC(239cb7f3) SHA1(1abc59bc73cf84ee3b73d500bf57a2a202291fcb))
	ROM_LOAD("jpaq11sd", 0x50000, 0x8000, CRC(e5b9e70f) SHA1(7db0a13166120fe20bb76072475b092e942629cf))
	ROM_LOAD("jpaq12sd", 0x58000, 0x8000, CRC(9aa37260) SHA1(6eec14f0d7152bf0cfadabe5b3017b9b6b7aa2d3))
	ROM_LOAD("jpaq13sd", 0x60000, 0x8000, CRC(5599792e) SHA1(9d844d9f155f299bbe2d512f8ed84410e7a9cfb3))
	ROM_LOAD("jpaq14sd", 0x68000, 0x8000, CRC(0bdcbbbd) SHA1(555d8ed846079894cfc60041fb724deeaddc4e89))
ROM_END

/*-------------------------------------------------------------------
/ Faeton #1103
/-------------------------------------------------------------------*/
ROM_START(faeton)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("faeton.cpu", 0x0000, 0x2000, CRC(ef7e6915) SHA1(5d3d86549606b3d9134bb3f6d3026d6f3e07d4cd))
ROM_END

ROM_START(faeton6d)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("faeton6d.bin", 0x0000, 0x2000, CRC(e7c45ba4) SHA1(c0f82271ef44841c599f0e7b9a1ed9cd16f0580a))
ROM_END

/*-------------------------------------------------------------------
/ Halley Comet #1104
/-------------------------------------------------------------------*/
ROM_START(halley)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("halley.cpu", 0x0000, 0x2000, CRC(b158a0d7) SHA1(ad071ac3d06a99a8fbd4df461071fe03dc1e1a26))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("hc_sh", 0x0000, 0x4000, CRC(8af15ded) SHA1(2abc199b612df6180dc116f56ec0027dacf30e77))

	ROM_REGION(0x80000, "sound1", 0)
	ROM_LOAD("hc_s1",   0x0000,  0x8000, CRC(3146b12f) SHA1(9d3974c267e1b2f8d0a8edc78f4013823e4d5e9b))
	ROM_LOAD("hc_s2",   0x8000,  0x8000, CRC(8b525f15) SHA1(3ba78a730b11d32fb6ebbcfc52672b9bb5ca5250))
	ROM_LOAD("hc_s3",   0x10000, 0x8000, CRC(59a7c53d) SHA1(b1d27f06ff8bd44aa5a4c8fd3b405b67684ae644))
	ROM_LOAD("hc_s4",   0x18000, 0x8000, CRC(14149419) SHA1(e39ba211e8784c8f46d89b7ce8a046443ab87f3a))
	ROM_LOAD("hc_s5",   0x20000, 0x8000, CRC(9ab8f478) SHA1(116efd8c5524ab8a6a26d4b8187f6559f1940340))
	ROM_LOAD("hc_s6",   0x28000, 0x8000, CRC(0fd00c1e) SHA1(1b143bf87541be68a37e133ff5dab5d5cff006b5))
	ROM_LOAD("hc_s7",   0x30000, 0x8000, CRC(731b9b5d) SHA1(153cd93d99e386f1b52be5360d4789b53b112e34))
	ROM_LOAD("hc_da0",  0x38000, 0x8000, CRC(5172993b) SHA1(4ae8adc59c95efefc48fcf7524b3da6e7d65e9c7))
	ROM_LOAD("hc_da1",  0x40000, 0x8000, CRC(e9ddc966) SHA1(9fa2bdbafed8b1c1e190f1f99af54ea1d9c81d26))
	ROM_LOAD("hc_da2",  0x48000, 0x8000, CRC(2e1a89a6) SHA1(adf34ce979b254b19abaf824ff656f647df601db))
	ROM_LOAD("hc_da3",  0x50000, 0x8000, CRC(00bbabb0) SHA1(2d584c53e32fce1a105bb86aaa91c427bf741f2d))
	ROM_LOAD("hc_da4",  0x58000, 0x8000, CRC(402358e8) SHA1(8513b0c0bf40363af323577175dfe569bd6b8686))
	ROM_LOAD("hc_da5",  0x60000, 0x8000, CRC(a6bd8ccd) SHA1(128acc73ba2009ffa29f65fd570917ad0dec4142))
	ROM_LOAD("hc_da6",  0x68000, 0x8000, CRC(9eba3c37) SHA1(a435cdbeb43f5216f58d6e90522e5a25b3bccaef))
	ROM_LOAD("hc_da7",  0x70000, 0x8000, CRC(28249d52) SHA1(43cebbe555cae3a49e91deb3cfe715f743507e4a))
	ROM_LOAD("hc_da8",  0x78000, 0x8000, CRC(3f2e81ee) SHA1(648e2b97fa2d6c4dcd16fef5d8c4b9baeee2f290))
ROM_END

ROM_START(halleya)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("hc_pgm", 0x0000, 0x2000, CRC(dc5eaa8f) SHA1(2f3af60ba5439f67e9c69de543167ac31abc09f1))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("hc_sh", 0x0000, 0x4000, CRC(8af15ded) SHA1(2abc199b612df6180dc116f56ec0027dacf30e77))

	ROM_REGION(0x80000, "sound1", 0)
	ROM_LOAD("hc_s1",   0x0000,  0x8000, CRC(3146b12f) SHA1(9d3974c267e1b2f8d0a8edc78f4013823e4d5e9b))
	ROM_LOAD("hc_s2",   0x8000,  0x8000, CRC(8b525f15) SHA1(3ba78a730b11d32fb6ebbcfc52672b9bb5ca5250))
	ROM_LOAD("hc_s3",   0x10000, 0x8000, CRC(59a7c53d) SHA1(b1d27f06ff8bd44aa5a4c8fd3b405b67684ae644))
	ROM_LOAD("hc_s4",   0x18000, 0x8000, CRC(14149419) SHA1(e39ba211e8784c8f46d89b7ce8a046443ab87f3a))
	ROM_LOAD("hc_s5",   0x20000, 0x8000, CRC(9ab8f478) SHA1(116efd8c5524ab8a6a26d4b8187f6559f1940340))
	ROM_LOAD("hc_s6",   0x28000, 0x8000, CRC(0fd00c1e) SHA1(1b143bf87541be68a37e133ff5dab5d5cff006b5))
	ROM_LOAD("hc_s7",   0x30000, 0x8000, CRC(731b9b5d) SHA1(153cd93d99e386f1b52be5360d4789b53b112e34))
	ROM_LOAD("hc_da0",  0x38000, 0x8000, CRC(5172993b) SHA1(4ae8adc59c95efefc48fcf7524b3da6e7d65e9c7))
	ROM_LOAD("hc_da1",  0x40000, 0x8000, CRC(e9ddc966) SHA1(9fa2bdbafed8b1c1e190f1f99af54ea1d9c81d26))
	ROM_LOAD("hc_da2",  0x48000, 0x8000, CRC(2e1a89a6) SHA1(adf34ce979b254b19abaf824ff656f647df601db))
	ROM_LOAD("hc_da3",  0x50000, 0x8000, CRC(00bbabb0) SHA1(2d584c53e32fce1a105bb86aaa91c427bf741f2d))
	ROM_LOAD("hc_da4",  0x58000, 0x8000, CRC(402358e8) SHA1(8513b0c0bf40363af323577175dfe569bd6b8686))
	ROM_LOAD("hc_da5",  0x60000, 0x8000, CRC(a6bd8ccd) SHA1(128acc73ba2009ffa29f65fd570917ad0dec4142))
	ROM_LOAD("hc_da6",  0x68000, 0x8000, CRC(9eba3c37) SHA1(a435cdbeb43f5216f58d6e90522e5a25b3bccaef))
	ROM_LOAD("hc_da7",  0x70000, 0x8000, CRC(28249d52) SHA1(43cebbe555cae3a49e91deb3cfe715f743507e4a))
	ROM_LOAD("hc_da8",  0x78000, 0x8000, CRC(3f2e81ee) SHA1(648e2b97fa2d6c4dcd16fef5d8c4b9baeee2f290))
ROM_END

/*-------------------------------------------------------------------
/ Lortium #1110
/-------------------------------------------------------------------*/
ROM_START(lortium)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("cpulort1.dat", 0x0000, 0x2000, CRC(4943e31f) SHA1(2cbc0a1feb711b5540e9288b9b59527cc85361fc))
	ROM_LOAD("cpulort2.dat", 0x2000, 0x2000, CRC(71eebb26) SHA1(9d49c1012555bda24ac7287499bcb93828cbb57f))
ROM_END

/*-------------------------------------------------------------------
/ Pimbal
/-------------------------------------------------------------------*/
ROM_START(pimbal)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("p3000.r1", 0x0000, 0x2000, CRC(57fb5958) SHA1(536d6564c184f214edf821b83a27aa7f75c7ad00))
	ROM_LOAD("p3000.r2", 0x2000, 0x2000, CRC(b8aae5ad) SHA1(8639b132aa69281f4460f80e84e0d30a5dc298d0))
ROM_END

/*-------------------------------------------------------------------
/ Olympus #1108
/-------------------------------------------------------------------*/
ROM_START(olympus)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("olympus.dat", 0x0000, 0x2000, CRC(08b021e8) SHA1(9662d37ccef94b6e6bc3c8c81dea0c0a34c8052d))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("cs.128", 0x0000, 0x4000, CRC(39b9107a) SHA1(8a11fa0c1558d0b1d309446b8a6f97e761b6559d))

	ROM_REGION(0x80000, "sound1", 0)
	ROM_LOAD("c1.256", 0x0000, 0x8000, CRC(93ceefbf) SHA1(be50b3d4485d4e8291047a52ca60656b55729555))
	ROM_LOAD("c2.256", 0x8000, 0x8000, CRC(8d404cf7) SHA1(e521ff1cf999496bada5348b7f845c468f053f0f))
	ROM_LOAD("c3.256", 0x10000, 0x8000, CRC(266eb5dd) SHA1(0eb7c098ddb7f257daf625e5209a54c306d365bf))
	ROM_LOAD("c4.256", 0x18000, 0x8000, CRC(082a052d) SHA1(f316fbe6ff63433861a8856e297c953ce29a8901))
	ROM_LOAD("c5.256", 0x20000, 0x8000, CRC(402a3fb2) SHA1(1c078ca519271bf2bcbe0bc10e33078861085fcf))
	ROM_LOAD("c6.256", 0x28000, 0x8000, CRC(d113add1) SHA1(c0258226994af162ef766d5e8d27f809dac4ef7f))
	ROM_LOAD("c7.256", 0x30000, 0x8000, CRC(13f5fcad) SHA1(e7a8b76527067f16aa62d0f22eccde5b55eba972))
ROM_END

/*-------------------------------------------------------------------
/ Petaco (using the new hardware, probably #1102)
/-------------------------------------------------------------------*/
ROM_START(petacon)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("petaco-n.dat", 0x0000, 0x2000, CRC(9e4d6944) SHA1(54b39e28152d481bd485433b4a7bf46174a78dbb))
ROM_END

ROM_START(petacona)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("petacona.bin", 0x0000, 0x2000, CRC(81502083) SHA1(c67a095fb5e868467577e7a86de5d51c59b3a68e))
ROM_END

/*-------------------------------------------------------------------
/ Petaco 2 #1106?
/-------------------------------------------------------------------*/
ROM_START(petaco2)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("petaco2.dat", 0x0000, 0x2000, CRC(9a3d6409) SHA1(bca061e254c3214b940080c92d2cf88904f1b81c))

	ROM_REGION(0x4000, "audiocpu", 0)
	ROM_LOAD("jpsonid0.dat", 0x0000, 0x4000, CRC(1bdbdd60) SHA1(903012e58cdb4041e5546a377f5c9df83dc93737))

	ROM_REGION(0x80000, "sound1", 0)
	ROM_LOAD("jpsonid1.dat", 0x0000, 0x8000, CRC(e39da92a) SHA1(79eb60710bdf6b826349e02ae909426cb81e131e))
	ROM_LOAD("jpsonid2.dat", 0x8000, 0x8000, CRC(88456f1e) SHA1(168fe88ae9da5114d0ef6427df0503ca2eea9089))
	ROM_LOAD("jpsonid3.dat", 0x10000, 0x8000, CRC(c7597d29) SHA1(45abe1b28ad14610ac8e2bc3a70af46bbe6277f4))
	ROM_LOAD("jpsonid4.dat", 0x18000, 0x8000, CRC(0d29a028) SHA1(636cc9a1f6128c820b18db4bf764e0be10a46119))
	ROM_LOAD("jpsonid5.dat", 0x20000, 0x8000, CRC(76790393) SHA1(23df394ecd11205d83073dca160f8f9a98aaa169))
	ROM_LOAD("jpsonid6.dat", 0x28000, 0x8000, CRC(53c3f0b4) SHA1(dcf4c63636e2b7ff5cd2db99d949db9e33b78fc7))
	ROM_LOAD("jpsonid7.dat", 0x30000, 0x8000, CRC(ff430b1b) SHA1(423592a40eba174108dfc6817e549c643bb3c80f))
ROM_END

} // Anonymous namespace

// 6-digit display
GAME(1985,  petacon,  0,      jp,     jp, jp_state, init_6d,    ROT0, "Juegos Populares", "Petaco (new hardware)",                MACHINE_IS_SKELETON_MECHANICAL )
GAME(1985,  petacona, 0,      jp,     jp, jp_state, init_6d,    ROT0, "Juegos Populares", "Petaco (new hardware, alternate set)", MACHINE_IS_SKELETON_MECHANICAL )
GAME(1985,  faeton6d, faeton, jp,     jp, jp_state, init_6d,    ROT0, "Juegos Populares", "Faeton (6 digits)",                    MACHINE_IS_SKELETON_MECHANICAL )

// 7-digit display
GAME(1985,  petaco2,  0,      jps,    jp, jp_state, empty_init, ROT0, "Juegos Populares", "Petaco 2",                             MACHINE_IS_SKELETON_MECHANICAL )
GAME(1985,  faeton,   0,      jp,     jp, jp_state, empty_init, ROT0, "Juegos Populares", "Faeton (7 digits)",                    MACHINE_IS_SKELETON_MECHANICAL )
GAME(1986,  halley,   0,      jps,    jp, jp_state, empty_init, ROT0, "Juegos Populares", "Halley Comet",                         MACHINE_IS_SKELETON_MECHANICAL )
GAME(1986,  halleya,  halley, jps,    jp, jp_state, empty_init, ROT0, "Juegos Populares", "Halley Comet (alternate version)",     MACHINE_IS_SKELETON_MECHANICAL )
GAME(1986,  aqualand, 0,      jps,    jp, jp_state, empty_init, ROT0, "Juegos Populares", "Aqualand",                             MACHINE_IS_SKELETON_MECHANICAL )
GAME(1986,  america,  0,      jps,    jp, jp_state, empty_init, ROT0, "Juegos Populares", "America 1492",                         MACHINE_IS_SKELETON_MECHANICAL )
GAME(1986,  olympus,  0,      jps,    jp, jp_state, empty_init, ROT0, "Juegos Populares", "Olympus (Juegos Populares)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME(1987,  lortium,  0,      jp,     jp, jp_state, empty_init, ROT0, "Juegos Populares", "Lortium",                              MACHINE_IS_SKELETON_MECHANICAL )
GAME(19??,  pimbal,   0,      jp,     jp, jp_state, empty_init, ROT0, "Juegos Populares", "Pimbal (Pinball 3000)",                MACHINE_IS_SKELETON_MECHANICAL )

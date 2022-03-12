// license:BSD-3-Clause
// copyright-holders:Robbbert
/*******************************************************************************

PINBALL
Jeutel (France) System 84

Games:
- Apocalypse Now
- Evolution
- Excalibur (not the Gottlieb or Vifico game)
- Le King
- Olympic Games
- Papillon (not the Video Dens game)
- Valkyrie (also shown as Walkyrie in the manual)

Status:
- All machines are playable.
- Multiball key code is XZ - need to jiggle these keys at start of game.

ToDo:
- Speech not working because the TMS5110 INT pin is not emulated
- Le King: Bad sound rom
- Match doesn't display or work

********************************************************************************/

#include "emu.h"
#include "machine/genpin.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/tms5110.h"
#include "speaker.h"

#include "jeutel.lh"


namespace {

class jeutel_state : public genpin_class
{
public:
	jeutel_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_cpu2(*this, "cpu2")
		, m_audiocpu(*this, "audiocpu")
		, m_speech(*this, "speech")
		, m_tms(*this, "tms")
		, m_ppi(*this, "ppi%d", 0U)
		, m_io_dips(*this, "DSW%d", 0U)
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void jeutel(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void machine_start() override;
	u8 portb_r();
	void porta_w(u8 data);
	void ppi0a_w(u8 data) { m_row = data; }
	u8 ppi0b_r() { u8 data = 0xff; for (u8 i = 0; i < 5; i++) if (!BIT(m_row, i)) data &= m_io_keyboard[i]->read(); return data; }
	void ppi0c_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[i] = BIT(data, i); }
	void ppi1a_w(u8 data);
	u16 seg8to14(u16 data);
	void ppi1b_w(u8 data);
	void ppi1c_w(u8 data) { m_lamp_data = data; }
	void ppi2a_w(u8 data);
	u8 ppi2b_r() { u8 data = 0xff; for (u8 i = 0; i < 4; i++) if (!BIT(m_diprow, i)) data &= m_io_dips[i]->read(); return data; }
	void ppi2c_w(u8 data) { m_sndcmd = data; }
	void tmaddr_w(u8 data);
	void tminc_w(u8);
	u8 tmdata_r();
	DECLARE_WRITE_LINE_MEMBER(clock_w);
	void cpu2_map(address_map &map);
	void audio_main_map(address_map &map);
	void audio_io_map(address_map &map);
	void main_map(address_map &map);

	u8 m_sndcmd = 0U;
	u8 m_digit = 0U;
	u8 m_row = 0U;
	u8 m_diprow = 0U;
	u16 m_tmbyte = 0U;
	u8 m_tmbit = 0U;
	u8 m_tmtemp = 0U;
	u8 m_t_c = 0U;
	u8 m_lamp_data = 0U;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu2;
	required_device<cpu_device> m_audiocpu;
	required_region_ptr<u8> m_speech;
	required_device<tms5110_device> m_tms;
	required_device_array<i8255_device, 3> m_ppi;
	required_ioport_array<4> m_io_dips;
	required_ioport_array<5> m_io_keyboard;
	output_finder<60> m_digits;
	output_finder<136> m_io_outputs;
};


void jeutel_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom().region("roms", 0);
	map(0xc000, 0xc3ff).ram().share("nvram");
	map(0xc400, 0xc7ff).ram();
	map(0xe000, 0xe003).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void jeutel_state::cpu2_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom().region("roms", 0x2000);
	map(0x2000, 0x2003).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x3000, 0x3003).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x4000, 0x4000).nopw(); // writes 12 here many times
	map(0x8000, 0x83ff).ram();
	map(0xc000, 0xc3ff).ram().share("nvram");
}

void jeutel_state::audio_main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).rom().region("roms", 0x3000);
	map(0x4000, 0x43ff).ram();
	map(0x6000, 0x7fff).w("dac", FUNC(dac_byte_interface::data_w));
	map(0x8000, 0x8000).w(FUNC(jeutel_state::tmaddr_w));
}

void jeutel_state::audio_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x01, 0x01).w("aysnd", FUNC(ay8910_device::data_w));
	map(0x04, 0x04).r("aysnd", FUNC(ay8910_device::data_r));
}

static INPUT_PORTS_START( jeutel )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S25")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x02, "S29")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x04, "Tilt Penalty")
	PORT_DIPSETTING(    0x00, "Game")
	PORT_DIPSETTING(    0x04, "Ball")
	PORT_DIPNAME( 0x08, 0x08, "S21")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x10, "Balls")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x10, "3")
	PORT_DIPNAME( 0x20, 0x20, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPNAME( 0x40, 0x40, "S1")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "S5")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S26")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x02, "S30")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x04, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x08, "S22")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x10, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPNAME( 0x40, 0x40, "S2")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "S6")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "S27")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x02, "S31")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x04, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x08, "S23")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x10, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPNAME( 0x40, 0x40, "S3")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x80, "S7")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "S28")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x02, "S32")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x04, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x08, "S24")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x10, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "S16")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPNAME( 0x40, 0x40, "S4")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPNAME( 0x80, 0x00, "S8")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x80, DEF_STR( Off ))

	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP00")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP01")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP02")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP03")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP04")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP05")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP06")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP07")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP10")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP11")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP12")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP13")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP14")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP15")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP16")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP17")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP20")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP21")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP22")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP23")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP24")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP25")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP26")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP27")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP30")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP37")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Test")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

u8 jeutel_state::tmdata_r()
{
	return BIT(m_speech[m_tmbyte], m_tmbit);
}

void jeutel_state::tminc_w(u8 data)
{
	m_tmbit = (m_tmbit+1) & 7;
	if (!m_tmbit)
		m_tmbyte = (m_tmbyte+1) & 0x1fff;
}

void jeutel_state::tmaddr_w(u8 data)
{
	m_tmtemp = data;
}

u8 jeutel_state::portb_r()
{
	return m_sndcmd;
}

void jeutel_state::porta_w(u8 data)
{
	if (!BIT(data, 3))
		m_tmbyte = (m_tmbyte & 0xff) | (m_tmtemp << 8);
	if (!BIT(data, 4))
	{
		m_tmbyte = (m_tmbyte & 0xff00) | m_tmtemp;
		m_tmbit = 0U;
	}

	if ((data & 0xf0) == 0xf0)
	{
		m_tms->ctl_w(tms5110_device::CMD_RESET);
		m_tms->pdc_w(1);
		m_tms->pdc_w(0);
	}
	else if ((data & 0xf0) == 0xd0)
	{
		m_tms->ctl_w(tms5110_device::CMD_SPEAK);
		m_tms->pdc_w(1);
		m_tms->pdc_w(0);
	}
}

void jeutel_state::ppi1a_w(u8 data)
{
	static const u8 patterns[16] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x81,0x79,0x38,0x3e,0x1e,0 }; // 6331 (letters are T,E,L,U,J,blank)
	bool blank = !BIT(data, 7);
	u16 segment = seg8to14(patterns[data & 15]);

	if (BIT(data, 6))
	{
		m_digits[40+m_digit] = segment;
	}
	else if (BIT(data, 4))
	{
		m_digits[m_digit] = (blank) ? 0 : segment;
	}
	else if (BIT(data, 5))
	{
		m_digits[20+m_digit] = (blank) ? 0 : segment;
	}
}

u16 jeutel_state::seg8to14(u16 data)
{
	// convert custom 8seg digit to MAME 14seg digit
	return bitswap<10>(data,7,7,6,6,5,4,3,2,1,0);
}

void jeutel_state::ppi1b_w(u8 data)
{
	m_digit = data & 0x0f;
	if (m_digit > 7)
		m_digit+=2;

	data >>= 4;
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[8+data*8+i] = BIT(m_lamp_data, i);

	if (data == 9)
	{
		if (BIT(m_lamp_data, 0))
			m_samples->start(5, 5); // outhole
		if (BIT(m_lamp_data, 2))
			m_samples->start(0, 6); // gong
	}
}

void jeutel_state::ppi2a_w(u8 data)
{
	m_diprow = data;
	if (!BIT(data, 6))
		m_audiocpu->reset();
	if (m_t_c > 0x10)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, BIT(data, 7) ? ASSERT_LINE : CLEAR_LINE);
}

void jeutel_state::machine_start()
{
	genpin_class::machine_start();
	m_io_outputs.resolve();
	m_digits.resolve();

	save_item(NAME(m_t_c));
	save_item(NAME(m_row));
	save_item(NAME(m_diprow));
	save_item(NAME(m_lamp_data));
	save_item(NAME(m_sndcmd));
	save_item(NAME(m_digit));
	save_item(NAME(m_tmbyte));
	save_item(NAME(m_tmbit));
	save_item(NAME(m_tmtemp));
}

void jeutel_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_sndcmd = 0U;
	m_digit = 0U;
	m_t_c = 0U;
	m_tmbyte = 0U;
	m_tmbit = 0U;
}

WRITE_LINE_MEMBER( jeutel_state::clock_w )
{
	m_cpu2->set_input_line(0, (state) ? ASSERT_LINE : CLEAR_LINE);
	if (m_cpu2->state_int(Z80_HALT))
		m_cpu2->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	if (m_t_c < 0x20)
		m_t_c++;
}

void jeutel_state::jeutel(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &jeutel_state::main_map);

	Z80(config, m_cpu2, 8_MHz_XTAL / 2);
	m_cpu2->set_addrmap(AS_PROGRAM, &jeutel_state::cpu2_map);

	Z80(config, m_audiocpu, 8_MHz_XTAL / 2);
	m_audiocpu->set_addrmap(AS_PROGRAM, &jeutel_state::audio_main_map);
	m_audiocpu->set_addrmap(AS_IO, &jeutel_state::audio_io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Video */
	config.set_default_layout(layout_jeutel);

	/* Sound */
	genpin_audio(config);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 8_MHz_XTAL / 4));
	aysnd.port_a_write_callback().set(FUNC(jeutel_state::porta_w));
	aysnd.port_b_read_callback().set(FUNC(jeutel_state::portb_r));
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.8);

	TMS5110A(config, m_tms, 640000);
	m_tms->m0().set(FUNC(jeutel_state::tminc_w));
	m_tms->data().set(FUNC(jeutel_state::tmdata_r));
	//m_tms->irq().set_inputline(m_audiocpu, INPUT_LINE_IRQ0);  // emulation doesn't support INT pin
	m_tms->add_route(ALL_OUTPUTS, "mono", 1.0);

	MC1408(config, "dac", 0).add_route(ALL_OUTPUTS, "mono", 0.5);  // NE5018

	/* Devices */
	I8255A(config, m_ppi[0]);   // IC31
	//m_ppi[0]->in_pa_callback().set(FUNC(jeutel_state::ppi0a_r));
	m_ppi[0]->out_pa_callback().set(FUNC(jeutel_state::ppi0a_w));
	m_ppi[0]->in_pb_callback().set(FUNC(jeutel_state::ppi0b_r));
	//m_ppi[0]->out_pb_callback().set(FUNC(jeutel_state::ppi0b_w));
	//m_ppi[0]->in_pc_callback().set(FUNC(jeutel_state::ppi0c_r));
	m_ppi[0]->out_pc_callback().set(FUNC(jeutel_state::ppi0c_w));

	I8255A(config, m_ppi[1]);   // IC32
	//m_ppi[1]->in_pa_callback().set(FUNC(jeutel_state::ppi1a_r));
	m_ppi[1]->out_pa_callback().set(FUNC(jeutel_state::ppi1a_w));
	//m_ppi[1]->in_pb_callback().set(FUNC(jeutel_state::ppi1b_r));
	m_ppi[1]->out_pb_callback().set(FUNC(jeutel_state::ppi1b_w));
	//m_ppi[1]->in_pc_callback().set(FUNC(jeutel_state::ppi1c_r));
	m_ppi[1]->out_pc_callback().set(FUNC(jeutel_state::ppi1c_w));

	I8255A(config, m_ppi[2]);   // IC33
	//m_ppi[2]->in_pa_callback().set(FUNC(jeutel_state::ppi2a_r));
	m_ppi[2]->out_pa_callback().set(FUNC(jeutel_state::ppi2a_w));
	m_ppi[2]->in_pb_callback().set(FUNC(jeutel_state::ppi2b_r));
	//m_ppi[2]->out_pb_callback().set(FUNC(jeutel_state::ppi2b_w));
	//m_ppi[2]->in_pc_callback().set(FUNC(jeutel_state::ppi2c_r));
	m_ppi[2]->out_pc_callback().set(FUNC(jeutel_state::ppi2c_w));

	CLOCK(config, "rclock", 2000).signal_handler().set(FUNC(jeutel_state::clock_w));
}

} // Anonymous namespace


/*--------------------------------
/ Le King
/-------------------------------*/
ROM_START(leking)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("game-m.bin",  0x0000, 0x2000, CRC(4b66517a) SHA1(1939ea78932d469a16441507bb90b032c5f77b1e))
	ROM_LOAD("game-v.bin",  0x2000, 0x1000, CRC(cbbc8b55) SHA1(4fe150fa3b565e5618896c0af9d51713b381ed88))
	ROM_LOAD("sound-v.bin", 0x3000, 0x1000, CRC(36130e7b) SHA1(d9b66d43b55272579b3972005355b8a18ce6b4a9))

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("sound-p.bin", 0x0000, 0x2000, BAD_DUMP CRC(97eedd6c) SHA1(3bb8e5d32417c49ef97cbe407f2c5eeb214bf72d))
ROM_END

/*--------------------------------
/ Olympic Games
/-------------------------------*/
ROM_START(olympic)
	ROM_REGION(0x4000, "roms", 0)
	ROM_LOAD("game-jo1.bin", 0x0000, 0x2000, CRC(c9f040cf) SHA1(c689f3a82d904d3f9fc8688d4c06082c51645b2f))
	ROM_LOAD("game-v.bin",   0x2000, 0x1000, CRC(cd284a20) SHA1(94568e1247994c802266f9fbe4a6f6ed2b55a978))
	ROM_LOAD("sound-j0.bin", 0x3000, 0x1000, CRC(5c70ce72) SHA1(b0b6cc7b6ec3ed9944d738b61a0d144b77b07000))

	ROM_REGION(0x2000, "speech", 0)
	ROM_LOAD("sound-p.bin",  0x0000, 0x2000, CRC(97eedd6c) SHA1(3bb8e5d32417c49ef97cbe407f2c5eeb214bf72d))
ROM_END


GAME( 1983, leking,  0, jeutel, jeutel, jeutel_state, empty_init, ROT0, "Jeutel", "Le King",       MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1984, olympic, 0, jeutel, jeutel, jeutel_state, empty_init, ROT0, "Jeutel", "Olympic Games", MACHINE_IS_SKELETON_MECHANICAL )

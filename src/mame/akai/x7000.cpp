// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************
	Akai S612, S700, X7000 samplers

	These early Akai samplers share a 6-voice sound hardware design based around
	two 8253 (or 8254) timers each clocking an 8237 DMA controller into a 12-bit DAC.
	Two more 8253s also clock a total of six MF6CN-50 lowpass filter ICs (one per voice).
	The S700 and X7000 expand the sampling capacity via bank switching.

	All three models support Quick Disk loading and saving.
	The S612 uses a separate disk drive unit based around a Z80 SIO and discrete logic.
	The S700 and X7000 instead have a built-in drive controlled by a 8251 and MB87013.

	The S612 also supports Commodore Datasette tape units, but the physical connector was
	removed early in production. The Aug. 1985 service bulletin cites "the unfavorable
	popularity of Commodore type cassette data recorder" [sic].	Both firmware versions
	support it if present, though.

	TODO:
	- all actual sound output (and input)
	- layouts
	- S612 analog dials/sliders
	- disk support

***************************************************************************/
#include "emu.h"

#include "bus/midi/midi.h"
#include "bus/pet/cass.h"
#include "cpu/upd7810/upd7810.h"
#include "cpu/z80/z80.h"
#include "machine/6850acia.h"
#include "machine/74259.h"
#include "machine/adc0808.h"
#include "machine/am9517a.h"
#include "machine/bankdev.h"
#include "machine/clock.h"
#include "machine/gen_latch.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "machine/input_merger.h"
#include "machine/mb87013.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {
	
class s612_state : public driver_device
{
public:
	s612_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irq(*this, "irq")
		, m_acia(*this, "acia")
		, m_ram(*this, "ram")
		, m_cpu_bank(*this, "cpu_bank")
		, m_voice_bank(*this, "voice_bank%u", 0)
		, m_sample_timer(*this, "sample_timer%u", 0)
		, m_filter_timer(*this, "filter_timer%u", 0)
		, m_dma(*this, "dma%u", 0)
		, m_cass(*this, "cass")
		, m_led_digit(*this, "led_digit")
		, m_led(*this, "led%u", 0U)
	{}

	void base_config(machine_config &config, bool use_8254 = true);
	void s612(machine_config &config);

	ioport_value cass_sense_r() { return m_cass->sense_r(); }
	ioport_value cass_data_r() { return m_cass->read(); }

protected:
	enum // IRQ sources
	{
		IRQ_ACIA,
		IRQ_SUB_CPU
	};

	virtual void machine_start() override ATTR_COLD;

	virtual void common_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	TIMER_CALLBACK_MEMBER(refresh_timer);

	void led_digit_w(u8 data) { m_led_digit = data; }
	template<int Num> void led_w(int state) { m_led[Num] = state; }

	void dma_w(offs_t offset, u8 data);
	u8 dma_r(offs_t offset);

	template<int Num> void voice_bank_sel_w(u8 data);
	void cpu_bank_sel_w(u8 data);
	void cpu_bank_msb_w(offs_t offset, u8 data);
	void cpu_bank_lsb_w(offs_t offset, u8 data);
	u8 cpu_bank_msb_r(offs_t offset);
	u8 cpu_bank_lsb_r(offs_t offset);

	required_device<z80_device> m_maincpu;
	required_device<input_merger_any_high_device> m_irq;
	required_device<acia6850_device> m_acia;

	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_cpu_bank;
	required_device_array<address_map_bank_device, 6> m_voice_bank;

	required_device_array<pit8253_device, 2> m_sample_timer;
	required_device_array<pit8253_device, 2> m_filter_timer;
	required_device_array<am9517a_device, 2> m_dma;

	optional_device<pet_datassette_port_device> m_cass;

	output_finder<> m_led_digit;
	output_finder<10> m_led;

	emu_timer *m_refresh_timer;

	u8 m_refresh_counter;
};

class s700_state : public s612_state
{
public:
	s700_state(const machine_config &mconfig, device_type type, const char *tag)
		: s612_state(mconfig, type, tag)
		, m_lcdc(*this, "lcdc")
		, m_dial(*this, "DIAL")
		, m_sl(*this, "SL%u", 0)
	{}

	void s700(machine_config &config);

	ioport_value dial_r();

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void common_map(address_map &map) override ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	HD44780_PIXEL_UPDATE(lcd_update);

	void sl_w(u8 data) { m_sl_select = data; }
	u8 rl_r();

	required_device<hd44780_device> m_lcdc;

	required_ioport m_dial;
	required_ioport_array<4> m_sl;

	u8 m_sl_select;
};

class x7000_state : public s700_state
{
public:
	x7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: s700_state(mconfig, type, tag)
		, m_subcpu(*this, "subcpu")
		, m_key_latch(*this, "key_latch")
		, m_keys(*this, "KEY%u", 0)
		, m_pitch(*this, "PITCH")
	{}

	void x7000(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	virtual void common_map(address_map &map) override ATTR_COLD;

	void keys_w(u8 data) { m_key_sel = data; }
	u8 keys_r();
	u8 bend_up_r();
	u8 bend_down_r();

	void sub_pc_w(u8 data);

	u8 sub_to_main_r();

	required_device<upd78c10_device> m_subcpu;
	required_device<generic_latch_8_device> m_key_latch;

	required_ioport_array<8> m_keys;
	required_ioport m_pitch;

	u8 m_key_sel;
	u8 m_sub_pc;
};

/**************************************************************************/
void s612_state::machine_start()
{
	m_led_digit.resolve();
	m_led.resolve();

	m_cpu_bank->space().install_ram(0, m_ram->mask(), m_ram->pointer());
	for (int i = 0; i < 6; i++)
		m_voice_bank[i]->space().install_ram(0, m_ram->mask(), m_ram->pointer());

	m_refresh_timer = timer_alloc(FUNC(s612_state::refresh_timer), this);
	m_refresh_timer->adjust(attotime::zero, 0, attotime::from_ticks(4096, 8_MHz_XTAL));

	m_refresh_counter = 0;

	save_item(NAME(m_refresh_counter));
}

/**************************************************************************/
void s700_state::machine_start()
{
	s612_state::machine_start();

	m_sl_select = 0xf;

	save_item(NAME(m_sl_select));
}

/**************************************************************************/
void x7000_state::machine_start()
{
	s700_state::machine_start();

	m_key_sel = 0xff;
	m_sub_pc = 0xff;

	save_item(NAME(m_key_sel));
	save_item(NAME(m_sub_pc));
}

/**************************************************************************/
void s612_state::common_map(address_map &map)
{
	// most of the system ignores /MREQ and /IORQ and can be accessed via either space
	map(0x0000, 0x1fff).rom().region("maincpu", 0);
	map(0x2000, 0x21ff).mirror(0x1000).rw(FUNC(s612_state::dma_r), FUNC(s612_state::dma_w));
	map(0x2200, 0x22ff).mirror(0x1000).rw(m_filter_timer[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x2300, 0x23ff).mirror(0x1000).rw(m_filter_timer[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x2400, 0x24ff).mirror(0x1000).rw(m_sample_timer[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x2500, 0x25ff).mirror(0x1000).rw(m_sample_timer[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x2600, 0x26ff).mirror(0x1000).rw("adc", FUNC(adc0809_device::data_r), FUNC(adc0809_device::address_offset_start_w));
	map(0x2700, 0x2703).mirror(0x10f8).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	// 28xx-2Dxx: envelope control
	map(0x2e00, 0x2eff).mirror(0x1000).w("latch0", FUNC(hc259_device::write_a0));
	map(0x2f00, 0x2fff).mirror(0x1000).w("latch1", FUNC(hc259_device::write_a0));
	// 6000-7fff: disk interface
}

/**************************************************************************/
void s700_state::common_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4a00, 0x4aff).rw("qdc", FUNC(mb87013_device::read), FUNC(mb87013_device::write));
	map(0x4b00, 0x4bff).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0x4c00, 0x4cff).portr("ENCDR");
	// 4Dxx: envelope/mute control (A4..6 = address, A0 = strobe, D0..7 = data)
	map(0x5000, 0x51ff).rw(FUNC(s700_state::dma_r), FUNC(s700_state::dma_w));
	map(0x5200, 0x52ff).rw(m_filter_timer[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x5300, 0x53ff).rw(m_filter_timer[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x5400, 0x54ff).rw(m_sample_timer[0], FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x5500, 0x55ff).rw(m_sample_timer[1], FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x5600, 0x5600).mirror(0x00fc).w(FUNC(s700_state::voice_bank_sel_w<0>));
	map(0x5601, 0x5601).mirror(0x00fc).w(FUNC(s700_state::voice_bank_sel_w<1>));
	map(0x5602, 0x5602).mirror(0x00fc).w(FUNC(s700_state::voice_bank_sel_w<2>));
	map(0x5603, 0x5603).mirror(0x00fc).w(FUNC(s700_state::voice_bank_sel_w<3>));
	map(0x5701, 0x5701).mirror(0x00fc).w(FUNC(s700_state::voice_bank_sel_w<4>));
	map(0x5702, 0x5702).mirror(0x00fc).w(FUNC(s700_state::voice_bank_sel_w<5>));
	map(0x5703, 0x5703).mirror(0x00fc).w(FUNC(s700_state::cpu_bank_sel_w));
	map(0x6000, 0x7fff).ram().share("mainram");
}

/**************************************************************************/
void x7000_state::common_map(address_map &map)
{
	s700_state::common_map(map);
	map(0x4f00, 0x4fff).r(FUNC(x7000_state::sub_to_main_r));
}

/**************************************************************************/
void s612_state::mem_map(address_map &map)
{
	common_map(map);
	map(0x8000, 0xffff).rw(FUNC(s612_state::cpu_bank_msb_r), FUNC(s612_state::cpu_bank_msb_w));
}

/**************************************************************************/
void s612_state::io_map(address_map &map)
{
	common_map(map);
	map(0x2704, 0x2707).mirror(0x10f8).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x8000, 0xffff).rw(FUNC(s612_state::cpu_bank_lsb_r), FUNC(s612_state::cpu_bank_lsb_w));
}

/**************************************************************************/
void s700_state::io_map(address_map &map)
{
	common_map(map);
	map(0x4900, 0x4901).mirror(0x00fc).w(m_lcdc, FUNC(hd44780_device::write));
	map(0x4902, 0x4903).mirror(0x00fc).r(m_lcdc, FUNC(hd44780_device::read));
	map(0x4e00, 0x4eff).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x8000, 0xffff).rw(FUNC(s700_state::cpu_bank_lsb_r), FUNC(s700_state::cpu_bank_lsb_w));
}

/**************************************************************************/
void s612_state::base_config(machine_config &config, bool use_8254)
{
	const auto clk = 8_MHz_XTAL; // on S700 and X7000 this is actually 16MHz, but the derived frequencies are the same

	Z80(config, m_maincpu, clk/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &s612_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, m_irq);
	m_irq->output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	// MIDI
	ACIA6850(config, m_acia);
	m_acia->irq_handler().set(m_irq, FUNC(input_merger_any_high_device::in_w<IRQ_ACIA>));

	auto& midi_clock(CLOCK(config, "midi_clock", clk/16));
	midi_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	midi_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_acia->txd_handler().set(mdout, FUNC(midi_port_device::write_txd));

	auto &mdthru(MIDI_PORT(config, "mdthru"));
	midiout_slot(mdthru);
	mdin.rxd_handler().append(mdthru, FUNC(midi_port_device::write_txd));

	// sample memory (size depends on model)
	RAM(config, m_ram);
	ADDRESS_MAP_BANK(config, m_cpu_bank).set_options(ENDIANNESS_NATIVE, 16, 20, 0x10000);
	for (int i = 0; i < 6; i++)
		ADDRESS_MAP_BANK(config, m_voice_bank[i]).set_options(ENDIANNESS_NATIVE, 16, 20, 0x20000);

	// voice 0-3 DMA
	AM9517A(config, m_dma[0], clk/2);

	// voice 4-5 DMA
	AM9517A(config, m_dma[1], clk/2);

	// voice 0-2 filter clocks
	PIT8253(config, m_filter_timer[0]);
	m_filter_timer[0]->set_clk<0>(clk/2);
	m_filter_timer[0]->set_clk<1>(clk/2);
	m_filter_timer[0]->set_clk<2>(clk/2);

	// voice 3-5 filter clocks
	PIT8253(config, m_filter_timer[1]);
	m_filter_timer[1]->set_clk<0>(clk/2);
	m_filter_timer[1]->set_clk<1>(clk/2);
	m_filter_timer[1]->set_clk<2>(clk/2);

	if (use_8254)
	{
		// voice 0-2 sample clocks
		PIT8254(config, m_sample_timer[0]);
		// voice 3-5 sample clocks
		PIT8254(config, m_sample_timer[1]);
	}
	else
	{
		PIT8253(config, m_sample_timer[0]);
		PIT8253(config, m_sample_timer[1]);
	}
	m_sample_timer[0]->set_clk<0>(clk);
	m_sample_timer[0]->set_clk<1>(clk);
	m_sample_timer[0]->set_clk<2>(clk);

	m_sample_timer[1]->set_clk<0>(clk);
	m_sample_timer[1]->set_clk<1>(clk);
	m_sample_timer[1]->set_clk<2>(clk);
}

/**************************************************************************/
void s612_state::s612(machine_config &config)
{
	base_config(config, false);
	m_maincpu->set_addrmap(AS_IO, &s612_state::io_map);

	// sample memory: 12-bit x 32k x 1 bank
	m_ram->set_default_size("64K");

	PET_DATASSETTE_PORT(config, m_cass, cbm_datassette_devices, "c1530");

	auto &ppi(I8255(config, "ppi"));
	ppi.out_pa_callback().set(FUNC(s612_state::led_digit_w));
	ppi.in_pb_callback().set_ioport("PB");
	ppi.in_pc_callback().set_ioport("PC");

	auto &adc(ADC0809(config, "adc", 8_MHz_XTAL / 16)); // TODO
	adc.in_callback<0>().set_constant(0x80); // tune
	adc.in_callback<1>().set_constant(0x00); // start/splice point
	adc.in_callback<2>().set_constant(0xff); // end point
	adc.in_callback<3>().set_constant(0); // LFO speed
	adc.in_callback<4>().set_constant(0); // LFO depth
	adc.in_callback<5>().set_constant(0); // LFO delay
	adc.in_callback<6>().set_constant(0xff); // filter
	adc.in_callback<7>().set_constant(0xff); // decay

	auto &latch0(HC259(config, "latch0"));
	latch0.q_out_cb<0>().set(FUNC(s612_state::led_w<0>)); // new
	latch0.q_out_cb<1>().set(FUNC(s612_state::led_w<1>)); // overdub
	latch0.q_out_cb<2>().set(FUNC(s612_state::led_w<2>)); // mono/poly
	latch0.q_out_cb<3>().set(FUNC(s612_state::led_w<3>)); // load
	latch0.q_out_cb<4>().set(FUNC(s612_state::led_w<4>)); // save
	latch0.q_out_cb<5>().set(FUNC(s612_state::led_w<5>)); // transpose
	latch0.q_out_cb<6>().set(m_cass, FUNC(pet_datassette_port_device::motor_w)).invert();
	latch0.q_out_cb<7>().set(m_cass, FUNC(pet_datassette_port_device::write));

	auto &latch1(HC259(config, "latch1"));
	latch1.q_out_cb<0>().set(FUNC(s612_state::led_w<6>)); // one shot
	latch1.q_out_cb<1>().set(FUNC(s612_state::led_w<7>)); // looping
	latch1.q_out_cb<2>().set(FUNC(s612_state::led_w<8>)); // alternating
	latch1.q_out_cb<3>().set(FUNC(s612_state::led_w<9>)); // manual splice
	// TODO: bit 6 = mute, bit 7 = feedback
}

/**************************************************************************/
void s700_state::s700(machine_config &config)
{
	base_config(config);
	m_maincpu->set_addrmap(AS_IO, &s700_state::io_map);

	// sample memory: 12-bit x 64k x 3 banks, expandable to 8 banks
	m_ram->set_default_size("1M").set_extra_options("384K");

	auto &kdc(I8279(config, "kdc", 16_MHz_XTAL / 8));
	kdc.out_sl_callback().set(FUNC(s700_state::sl_w));
	kdc.in_rl_callback().set(FUNC(s700_state::rl_r));

	auto &i8251(I8251(config, "i8251", 16_MHz_XTAL / 4));
	i8251.write_cts(0);

	auto &qdc(MB87013(config, "qdc", 6.5_MHz_XTAL));
	qdc.sio_rd_callback().set(i8251, FUNC(i8251_device::read));
	qdc.sio_wr_callback().set(i8251, FUNC(i8251_device::write));
	qdc.txc_callback().set(i8251, FUNC(i8251_device::write_txc));
	qdc.rxc_callback().set(i8251, FUNC(i8251_device::write_rxc));
	qdc.rxd_callback().set(i8251, FUNC(i8251_device::write_rxd));
	qdc.dsr_callback().set(i8251, FUNC(i8251_device::write_dsr));
	qdc.op4_callback().set(qdc, FUNC(mb87013_device::rts_w));
	i8251.dtr_handler().set(qdc, FUNC(mb87013_device::dtr_w));
	i8251.txd_handler().set(qdc, FUNC(mb87013_device::txd_w));

	// LCD
	HD44780(config, m_lcdc, 270'000); // TODO: type and clock both guessed
	m_lcdc->set_lcd_size(2, 8);
	m_lcdc->set_pixel_update_cb(FUNC(s700_state::lcd_update));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6 * 16, 8);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);
}

/**************************************************************************/
void x7000_state::x7000(machine_config &config)
{
	s700(config);

	GENERIC_LATCH_8(config, m_key_latch);

	UPD78C11(config, m_subcpu, 12_MHz_XTAL);
	m_subcpu->pa_in_cb().set(FUNC(x7000_state::keys_r));
	m_subcpu->pb_in_cb().set(FUNC(x7000_state::keys_r));
	m_subcpu->pc_in_cb().set_constant(0xff); // PC7 high = single chip mode
	m_subcpu->pc_out_cb().set(FUNC(x7000_state::sub_pc_w));
	m_subcpu->pd_out_cb().set(m_key_latch, FUNC(generic_latch_8_device::write));
	m_subcpu->pf_out_cb().set(FUNC(x7000_state::keys_w));
	m_subcpu->an0_func().set(FUNC(x7000_state::bend_up_r));
	m_subcpu->an1_func().set(FUNC(x7000_state::bend_down_r));
	m_subcpu->an2_func().set_ioport("MOD");
}

/**************************************************************************/
TIMER_CALLBACK_MEMBER(s612_state::refresh_timer)
{
	m_refresh_counter++;
}

/**************************************************************************/
void s612_state::dma_w(offs_t offset, u8 data)
{
	// A0 and A4 are swapped when the CPU is accessing the DMA controllers
	// (to make 16-bit accesses easier)
	m_dma[BIT(offset, 8)]->write(bitswap<5>(offset, 0, 3, 2, 1, 4), data);
}

/**************************************************************************/
u8 s612_state::dma_r(offs_t offset)
{
	return m_dma[BIT(offset, 8)]->read(bitswap<5>(offset, 0, 3, 2, 1, 4));
}

/**************************************************************************/
template<int Num>
void s612_state::voice_bank_sel_w(u8 data)
{
	// TODO: bit 3 = generate NMI on DMA EOP
	m_voice_bank[Num]->set_bank(data & 0x7);
}

/**************************************************************************/
void s612_state::cpu_bank_sel_w(u8 data)
{
	// bits 0..2 = physical bank, bit 3 = A15
	m_cpu_bank->set_bank(bitswap<4>(data, 2, 1, 0, 3));
}

/**************************************************************************/
void s612_state::cpu_bank_msb_w(offs_t offset, u8 data)
{
	m_cpu_bank->write16(offset, data << 8, 0xff00);
}

/**************************************************************************/
void s612_state::cpu_bank_lsb_w(offs_t offset, u8 data)
{
	m_cpu_bank->write16(offset, data & 0xf0, 0x00ff);
}

/**************************************************************************/
u8 s612_state::cpu_bank_msb_r(offs_t offset)
{
	return m_cpu_bank->read16(offset, 0xff00) >> 8;
}

/**************************************************************************/
u8 s612_state::cpu_bank_lsb_r(offs_t offset)
{
	// bits 0-3 of DRAM, bits 0-3 of refresh counter
	return (m_cpu_bank->read16(offset, 0x00ff) & 0xf0)
		| (m_refresh_counter & 0x0f);
}

/**************************************************************************/
HD44780_PIXEL_UPDATE(s700_state::lcd_update)
{
	if (x < 6 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(y, line * 48 + pos * 6 + x) = state;
}

/**************************************************************************/
u8 s700_state::rl_r()
{
	u8 data = 0xff;
	for (int i = 0; i < 4; i++)
		if (!BIT(m_sl_select, i))
			data &= m_sl[i]->read();

	return data;
}

/**************************************************************************/
ioport_value s700_state::dial_r()
{
	const u8 val = m_dial->read();
	return (val >> 7) ^ (val >> 6);
}

/**************************************************************************/
u8 x7000_state::keys_r()
{
	u8 data = 0xff;
	for (int i = 0; i < 8; i++)
		if (!BIT(m_key_sel, i))
			data &= m_keys[i]->read();

	return data;
}

/**************************************************************************/
u8 x7000_state::bend_up_r()
{
	const u16 pos = m_pitch->read();
	if (pos >= 0x100)
		return pos & 0xff;

	return 0;
}

/**************************************************************************/
u8 x7000_state::bend_down_r()
{
	const u16 pos = m_pitch->read();
	if (pos < 0x100)
		return pos ^ 0xff;

	return 0;
}

/**************************************************************************/
void x7000_state::sub_pc_w(u8 data)
{
	if (BIT(m_sub_pc, 2) && BIT(~data, 2))
	{
		m_irq->in_w<IRQ_SUB_CPU>(ASSERT_LINE);
		m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
	m_sub_pc = data;
}

/**************************************************************************/
u8 x7000_state::sub_to_main_r()
{
	if (!machine().side_effects_disabled())
	{
		m_irq->in_w<IRQ_SUB_CPU>(CLEAR_LINE);
		m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	return ~m_key_latch->read();
}

static INPUT_PORTS_START( s612 )
	// TODO: analog controls

	PORT_START("PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("New")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Overdub")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Save")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Verify")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Load")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Mono/Poly")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Channel Down")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Channel Up")

	PORT_START("PC")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("One Shot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Looping")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Alternating")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rec. Trigger")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Manual Splice")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Key Transpose")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s612_state::cass_sense_r))
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s612_state::cass_data_r))
INPUT_PORTS_END

static INPUT_PORTS_START( s700 )
	PORT_START("SL0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Save")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Verify")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Load")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Sample")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("MIDI")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SL1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Loop")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("End")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Start")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Key Range")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Transpose")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SL2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Audio Trigger")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Tune")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Out")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Scan")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LFO")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SL3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Program")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("New")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Overdub")
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ENCDR")
	PORT_BIT( 0x03, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s700_state::dial_r))
	PORT_BIT( 0x0c, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Play Back")
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER  ) PORT_NAME("Program Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER  ) PORT_NAME("Rec/PB Trigger")
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Control") PORT_SENSITIVITY(50) PORT_KEYDELTA(75) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT)
INPUT_PORTS_END

static INPUT_PORTS_START( x7000 )
	PORT_START("SL0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Scan Mode")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Scan Direction")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Start")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Loop")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("End")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Auto Loop")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LFO Speed")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LFO Depth")

	PORT_START("SL1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("LFO Delay")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Filter")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Level")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Release")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Program Tune")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Resample")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("New")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Overdub")

	PORT_START("SL2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Sample")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Key Range")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Audio Trigger")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Load")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Verify")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Save")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SL3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Program")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Constant Pitch")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Transpose")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("MIDI")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Bend Width / Key Transpose")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Master Tune")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("ENCDR")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s700_state::dial_r))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("Sustain")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("Local") PORT_TOGGLE
	// default to on
	PORT_DIPSETTING( 0x08, DEF_STR(On) )
	PORT_DIPSETTING( 0x00, DEF_STR(Off) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // no playback button
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("Program Up")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER  ) PORT_NAME("Rec/PB Trigger")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Control") PORT_SENSITIVITY(50) PORT_KEYDELTA(75) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT)

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E2")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#2")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G2")

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#3")

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E3")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G3")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A3")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#3")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B3")

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E4")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F4")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#4")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G4")

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#4")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A4")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#4")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#5")

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E5")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F5")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#5")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A5")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#5")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B5")

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C#6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D#6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F#6")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G6")

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G#6")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A6")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A#6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B6")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C7")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
		
	PORT_START("PITCH")
	PORT_BIT( 0x1ff, 0x100, IPT_PADDLE ) PORT_NAME("Pitch Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)

	PORT_START("MOD")
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL_V ) PORT_NAME("Modulation Wheel") PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_INVERT PORT_PLAYER(2) PORT_CODE_DEC(JOYCODE_Y_DOWN_SWITCH) PORT_CODE_INC(JOYCODE_Y_UP_SWITCH)
INPUT_PORTS_END


ROM_START( s612 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v11", "Version 1.1")
	ROMX_LOAD( "612aki11.bin", 0x0000, 0x2000, CRC(e425e68a) SHA1(4779e7808990d2429966e122057130cb6fa4cfb9), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v10", "Version 1.0")
	ROMX_LOAD( "akai s612 v1.0.bin", 0x0000, 0x2000, CRC(7431a468) SHA1(308d6bbe4e367809bd5b77f5a608fddc14549369), ROM_BIOS(1) )
ROM_END

ROM_START( x7000 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v16", "Version 1.6")
	ROMX_LOAD( "x7000v16.bin", 0x0000, 0x4000, CRC(cb53186a) SHA1(1e78a1238cdf352c6114345a7fc5888b6c147446), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v12", "Version 1.2")
	ROMX_LOAD( "akai x7000 1.2.bin", 0x0000, 0x4000, CRC(509b3d2a) SHA1(5adf64e51ed6e6e17ad7f8adecd370783c973ae6), ROM_BIOS(1) )

	ROM_REGION(0x1000, "subcpu", 0)
	ROM_LOAD( "upd78c11g-044-36.ic58", 0x0000, 0x1000, CRC(59fee9fa) SHA1(830eea667e46437f9a65280bf59234107abe49c8) )
ROM_END

ROM_START( s700 )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v21", "Version 2.1")
	ROMX_LOAD( "akai_s700_2.1.bin", 0x0000, 0x4000, CRC(4da08430) SHA1(cb63f9f13c9c15f796a316c6f0ae82be447d3a0e), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "v16", "Version 1.6")
	ROMX_LOAD( "akai-s700_v1_6_128.bin", 0x0000, 0x4000, CRC(19489855) SHA1(aceccf00aac86000dab7dd6566ebbff4fc488786), ROM_BIOS(1) )
ROM_END

}

SYST( 1985, s612,  0,     0, s612,  s612,  s612_state,  empty_init, "Akai", "S612 MIDI Digital Sampler", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
SYST( 1986, x7000, 0,     0, x7000, x7000, x7000_state, empty_init, "Akai", "X7000 Sampling Keyboard",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
SYST( 1986, s700,  x7000, 0, s700,  s700,  s700_state,  empty_init, "Akai", "S700 MIDI Digital Sampler", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

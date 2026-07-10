// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************
	Akai S900, S950 samplers

	The successors to the S700/X7000 have similar sound hardware, but with 8 voices
	and increased sample memory limits. The Quick Disk drives were also replaced with
	standard 3.5" floppies.

	TODO:
	- output & input filters (9x MF6-50)
	- layouts
	- software list
	- formatting SCSI drives on S950 doesn't seem to work
	- RS232 port for S950 (shares 6850 w/ MIDI)
***************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "bus/nscsi/devices.h"
#include "bus/rs232/rs232.h"
#include "cpu/nec/nec.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/6850acia.h"
#include "machine/am9517a.h"
#include "machine/bankdev.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/mb87030.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "sound/adc.h"
#include "sound/dac.h"
#include "sound/mixer.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "formats/s900_dsk.h"
#include "formats/hxchfe_dsk.h"

namespace {
	
class s900_state : public driver_device
{
public:
	s900_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nmi(*this, "nmi")
		, m_pic(*this, "pic")
		, m_acia(*this, "acia")
		, m_dma(*this, "dma")
		, m_lcdc(*this, "lcdc")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_sample_timer(*this, "sample_timer%u", 0)
		, m_filter_timer(*this, "filter_timer%u", 0)
		, m_voice_dma(*this, "voice_dma%u", 0)
		, m_dac(*this, "dac%u", 0)
		, m_adc(*this, "adc")
		, m_rom_view(*this, "rom_view")
		, m_fadetbl(*this, "fadetbl")
		, m_keys(*this, "KEY%u", 0)
	{}

	void s900(machine_config &config) ATTR_COLD;

	ioport_value fdc_irq_r() { return m_fdc->get_irq(); }

	DECLARE_INPUT_CHANGED_MEMBER(dial_w);

protected:
	enum // NMI sources
	{
		NMI_MIDI,
		NMI_RS232
	};

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void base_config(machine_config &config) ATTR_COLD;

	virtual void common_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	void disk_motor_w(int state);
	virtual void media_ctrl_w(u8 data);

	HD44780_PIXEL_UPDATE(lcd_update);

	u8 key_rows_r();
	u8 key_cols_r();

	void dial_reset_w(u8) { m_dial_count = 0x8; }
	u8 dial_count_r() { return m_dial_count; }

	template <int Voice> void voice_dreq_set_w(int state);
	template <int Voice> void voice_dreq_clear_w(int state);
	template <int Num> void voice_eop_w(int state);

	void adc_dreq_set_w(int state);
	void adc_dreq_clear_w(int state);

	u16 adc_r();

	u16 voice_dma_mem_r(offs_t offset);
	void voice_dma_mem_w(offs_t offset, u16 data);
	template <int Voice> void voice_dma_io_w(offs_t offset, u16 data);

	void envelope_w(offs_t offset, u8 data);

	required_device<v30_device> m_maincpu;
	required_device<input_merger_any_high_device> m_nmi;
	required_device<pic8259_device> m_pic;
	required_device<acia6850_device> m_acia;
	required_device<upd71071_device> m_dma;
	required_device<hd44780_device> m_lcdc;

	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_floppy;

	required_device_array<pit8254_device, 3> m_sample_timer;
	required_device_array<pit8253_device, 3> m_filter_timer;
	required_device_array<upd71071_device, 2> m_voice_dma;

	required_device_array<dac_12bit_r2r_twos_complement_device, 8> m_dac;
	required_device<adc12_device> m_adc;

	memory_view m_rom_view;
	optional_memory_region m_fadetbl;

	required_ioport_array<5> m_keys;

	u8 m_dial_count;

	u8 m_current_voice;
	u8 m_voice_eop[2];
};

class s950_state : public s900_state
{
public:
	s950_state(const machine_config &mconfig, device_type type, const char *tag)
		: s900_state(mconfig, type, tag)
		, m_ram(*this, "ram")
		, m_cpu_bank(*this, "cpu_bank%u", 0)
		, m_voice_bank(*this, "voice_bank")
		, m_spc(*this, "spc")
		, m_scsi_regs(*this, "scsi_regs")
	{}

	void s950(machine_config &config) ATTR_COLD;
	void s950scsi(machine_config &config) ATTR_COLD;

	ioport_value floppy_density_r();
	ioport_value hd_irq_r() { return m_hd_irq; }

protected:
	enum // peripheral DMA sources
	{
		DMA_ADC,
		DMA_DAT,
		DMA_FDC,
		DMA_HD // ACSI or SCSI depending on option board
	};

	virtual void machine_start() override ATTR_COLD;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void io_map_scsi(address_map &map) ATTR_COLD;
	void scsi_reg_map(address_map &map) ATTR_COLD;

	virtual void media_ctrl_w(u8 data) override;

	void bank_w(offs_t offset, u8 data);

	void adc_dreq_set_w(int state);

	void update_dreq();
	template <int Num> void dma_dreq_w(int state);
	void dma_dack_w(int state);
	void dma_eop_w(int state);

	u16 dma_io_r(offs_t offset);
	void dma_io_w(offs_t offset, u16 data);

	required_device<ram_device> m_ram;
	required_device_array<address_map_bank_device, 4> m_cpu_bank;
	required_device<address_map_bank_device> m_voice_bank;

	optional_device<mb89352_device> m_spc;
	optional_device<address_map_bank_device> m_scsi_regs;

	u8 m_dma_target;
	u8 m_dreq[4];
	u8 m_hd_irq;
};


/**************************************************************************/
void s900_state::machine_start()
{
	m_current_voice = 0;
	m_voice_eop[0] = m_voice_eop[1] = 0;

	save_item(NAME(m_current_voice));
	save_item(NAME(m_voice_eop));
}

/**************************************************************************/
void s950_state::machine_start()
{
	s900_state::machine_start();

	for (int i = 0; i < 4; i++)
		m_cpu_bank[i]->space().install_ram(0, m_ram->mask(), m_ram->pointer());
	m_voice_bank->space().install_ram(0, m_ram->mask(), m_ram->pointer());

	m_dma_target = 0;
	m_dreq[0] = m_dreq[1] = m_dreq[2] = m_dreq[3] = 0;
	m_hd_irq = 0;

	save_item(NAME(m_dma_target));
	save_item(NAME(m_dreq));
	save_item(NAME(m_hd_irq));
}

/**************************************************************************/
void s900_state::machine_reset()
{
	media_ctrl_w(0);
}

/**************************************************************************/
void s900_state::mem_map(address_map &map)
{
	map(0x00000, 0xfffff).ram();
	map(0x80000, 0xfffff).view(m_rom_view);
	m_rom_view[0](0x80000, 0x87fff).mirror(0x78000).rom().region("maincpu", 0);
}

/**************************************************************************/
void s950_state::mem_map(address_map &map)
{
	map(0x00000, 0x0ffff).mirror(0x40000).rw(m_cpu_bank[0], FUNC(address_map_bank_device::read16), FUNC(address_map_bank_device::write16));
	map(0x10000, 0x1ffff).mirror(0x40000).rw(m_cpu_bank[1], FUNC(address_map_bank_device::read16), FUNC(address_map_bank_device::write16));
	map(0x20000, 0x2ffff).mirror(0x40000).rw(m_cpu_bank[2], FUNC(address_map_bank_device::read16), FUNC(address_map_bank_device::write16));
	map(0x30000, 0x3ffff).mirror(0x40000).rw(m_cpu_bank[3], FUNC(address_map_bank_device::read16), FUNC(address_map_bank_device::write16));
	map(0x80000, 0x8ffff).mirror(0x70000).rom().region("maincpu", 0);
}

/**************************************************************************/
void s900_state::common_map(address_map &map)
{
	map(0x0010, 0x0017).mirror(0xff00).umask16(0x00ff).rw(m_sample_timer[0], FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0018, 0x001f).mirror(0xff00).umask16(0x00ff).rw(m_sample_timer[1], FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0020, 0x0027).mirror(0xff00).umask16(0x00ff).rw(m_sample_timer[2], FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0028, 0x002f).mirror(0xff00).umask16(0x00ff).rw(m_filter_timer[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0030, 0x0037).mirror(0xff00).umask16(0x00ff).rw(m_filter_timer[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0038, 0x003f).mirror(0xff00).umask16(0x00ff).rw(m_filter_timer[2], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0048, 0x004b).mirror(0xff00).umask16(0x00ff).w(m_lcdc, FUNC(hd44780_device::write));
	map(0x004c, 0x004f).mirror(0xff00).umask16(0x00ff).r(m_lcdc, FUNC(hd44780_device::read));
	map(0x0060, 0x0067).mirror(0xff00).umask16(0x00ff).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0068, 0x0069).mirror(0xff04).umask16(0x00ff).r(FUNC(s900_state::dial_count_r));
	map(0x006a, 0x006b).mirror(0xff04).umask16(0x00ff).portr("PORT6A");
	map(0x0068, 0x006f).mirror(0xff00).w(FUNC(s900_state::dial_reset_w));
	map(0x0078, 0x007f).select(0xff00).umask16(0x00ff).w(FUNC(s900_state::envelope_w));
	map(0x0080, 0x009f).mirror(0xfe00).rw(m_dma, FUNC(upd71071_device::read), FUNC(upd71071_device::write));
	map(0x00a0, 0x00bf).mirror(0xfe00).rw(m_voice_dma[0], FUNC(upd71071_device::read), FUNC(upd71071_device::write));
	map(0x00c0, 0x00df).mirror(0xfe00).rw(m_voice_dma[1], FUNC(upd71071_device::read), FUNC(upd71071_device::write));
	map(0x00e0, 0x00ff).mirror(0xfe00).umask16(0x00ff).rw(m_pic, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0180, 0x019f).mirror(0xfe00).r(FUNC(s900_state::adc_r));
}

/**************************************************************************/
void s900_state::io_map(address_map &map)
{
	common_map(map);
	map(0x0000, 0x0007).mirror(0xff00).umask16(0x00ff).m(m_fdc, FUNC(upd7265_device::map));
	map(0x0008, 0x0008).mirror(0xff06).umask16(0x00ff).w(FUNC(s900_state::media_ctrl_w));
//	map(0x0040, 0x0047).select(0xff00).umask16(0x00ff) - TODO drum trigger unit
	map(0x0050, 0x0057).mirror(0xff00).umask16(0x00ff).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x0058, 0x005b).mirror(0xff04).umask16(0x00ff).rw("i8251", FUNC(i8251_device::read), FUNC(i8251_device::write));
}

/**************************************************************************/
void s950_state::io_map(address_map &map)
{
	common_map(map);
	map(0x0000, 0x0007).mirror(0xff00).umask16(0x00ff).m(m_fdc, FUNC(upd72066_device::map));
	map(0x0008, 0x0008).mirror(0xff06).umask16(0x00ff).w(FUNC(s950_state::media_ctrl_w));
	map(0x0050, 0x0051).mirror(0xff00).umask16(0x00ff).w(m_acia, FUNC(acia6850_device::control_w));
	map(0x0052, 0x0053).mirror(0xff00).umask16(0x00ff).r(m_acia, FUNC(acia6850_device::status_r));
	map(0x0054, 0x0055).mirror(0xff00).umask16(0x00ff).w(m_acia, FUNC(acia6850_device::data_w));
	map(0x0056, 0x0057).mirror(0xff00).umask16(0x00ff).r(m_acia, FUNC(acia6850_device::data_r));
//	map(0x0058, 0x005f).mirror(0xff00).umask16(0x00ff) - hard disk interface
//	map(0x01a0, 0x01bf).mirror(0xfe00) - digital audio interface
	map(0x01c0, 0x01df).mirror(0xfe00).umask16(0x00ff).w(FUNC(s950_state::bank_w));
}

/**************************************************************************/
void s950_state::io_map_scsi(address_map &map)
{
	io_map(map);
	map(0x0058, 0x0059).mirror(0xff00).umask16(0x00ff).lw8(NAME([this](u8 data) { m_scsi_regs->set_bank(data); }));
	map(0x005c, 0x005d).mirror(0xff00).umask16(0x00ff).rw(m_scsi_regs, FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
}

/**************************************************************************/
void s950_state::scsi_reg_map(address_map &map)
{
	map(0x0, 0xf).m(m_spc, FUNC(mb89352_device::map));
}

/**************************************************************************/
static void s900_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

/**************************************************************************/
static void s950_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

/**************************************************************************/
static void floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_S900_FORMAT);
	fr.add(FLOPPY_HFE_FORMAT);
}

/**************************************************************************/
void s900_state::base_config(machine_config &config)
{
	const auto clk = 16_MHz_XTAL;

	V30(config, m_maincpu, clk/2);

	INPUT_MERGER_ANY_HIGH(config, m_nmi);
	m_nmi->output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PIC8259(config, m_pic);
	m_pic->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_maincpu->set_irq_acknowledge_callback(m_pic, FUNC(pic8259_device::inta_cb));

	auto &ppi(I8255(config, "ppi"));
	ppi.in_pa_callback().set(FUNC(s900_state::key_rows_r));
	ppi.in_pb_callback().set(FUNC(s900_state::key_cols_r));

	// MIDI
	ACIA6850(config, m_acia);
	m_acia->irq_handler().set(m_nmi, FUNC(input_merger_any_high_device::in_w<NMI_MIDI>));

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_acia->txd_handler().set(mdout, FUNC(midi_port_device::write_txd));

	auto &mdthru(MIDI_PORT(config, "mdthru"));
	midiout_slot(mdthru);
	mdin.rxd_handler().append(mdthru, FUNC(midi_port_device::write_txd));

	// LCD
	HD44780(config, m_lcdc, 270'000); // TODO: type and clock both guessed
	m_lcdc->set_lcd_size(2, 40);
	m_lcdc->set_pixel_update_cb(FUNC(s900_state::lcd_update));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6 * 40, 16);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	UPD71071(config, m_dma, clk/2);
	m_dma->out_hreq_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);
	m_dma->out_hreq_callback().append(m_dma, FUNC(upd71071_device::hack_w));

	// sound
	SPEAKER(config, "speaker", 8);
	SPEAKER(config, "monitor");

	ADC12(config, m_adc);

	auto &input_mixer(MIXER(config, "input"));
	input_mixer.add_route(0, m_adc, 1.0);

	for (int i = 0; i < 8; i++)
	{
		DAC_12BIT_R2R_TWOS_COMPLEMENT(config, m_dac[i]).add_route(0, "speaker", 1.0 / 8, i);
		// TODO: filters
	}

	auto &mic(MICROPHONE(config, "mic"));
	mic.add_route(0, "input", 1.0);
//	mic.add_route(0, "monitor", 1.0 / 6);

	auto& linein(CASSETTE(config, "linein"));
	linein.set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	linein.add_route(0, "input", 1.0);
	linein.add_route(0, "monitor", 1.0 / 6);

	// voice 0-3 DMA
	UPD71071(config, m_voice_dma[0], clk/2);
	m_voice_dma[0]->dreq_active_low();
	m_voice_dma[0]->out_hreq_callback().set(m_dma, FUNC(upd71071_device::dreq2_w));
	m_dma->out_dack_callback<2>().set(m_voice_dma[0], FUNC(upd71071_device::hack_w));
	m_voice_dma[0]->out_eop_callback().set(FUNC(s900_state::voice_eop_w<0>));
	m_voice_dma[0]->out_io16w_callback<0>().set(FUNC(s900_state::voice_dma_io_w<0>));
	m_voice_dma[0]->out_io16w_callback<1>().set(FUNC(s900_state::voice_dma_io_w<1>));
	m_voice_dma[0]->out_io16w_callback<2>().set(FUNC(s900_state::voice_dma_io_w<2>));
	m_voice_dma[0]->out_io16w_callback<3>().set(FUNC(s900_state::voice_dma_io_w<3>));
	m_voice_dma[0]->out_dack_callback<0>().set(FUNC(s900_state::voice_dreq_clear_w<0>));
	m_voice_dma[0]->out_dack_callback<1>().set(FUNC(s900_state::voice_dreq_clear_w<1>));
	m_voice_dma[0]->out_dack_callback<2>().set(FUNC(s900_state::voice_dreq_clear_w<2>));
	m_voice_dma[0]->out_dack_callback<3>().set(FUNC(s900_state::voice_dreq_clear_w<3>));

	// voice 4-7 DMA
	UPD71071(config, m_voice_dma[1], clk/2);
	m_voice_dma[1]->dreq_active_low();
	m_voice_dma[1]->out_hreq_callback().set(m_dma, FUNC(upd71071_device::dreq3_w));
	m_dma->out_dack_callback<3>().set(m_voice_dma[1], FUNC(upd71071_device::hack_w));
	m_voice_dma[1]->out_eop_callback().set(FUNC(s900_state::voice_eop_w<1>));
	m_voice_dma[1]->out_io16w_callback<0>().set(FUNC(s900_state::voice_dma_io_w<4>));
	m_voice_dma[1]->out_io16w_callback<1>().set(FUNC(s900_state::voice_dma_io_w<5>));
	m_voice_dma[1]->out_io16w_callback<2>().set(FUNC(s900_state::voice_dma_io_w<6>));
	m_voice_dma[1]->out_io16w_callback<3>().set(FUNC(s900_state::voice_dma_io_w<7>));
	m_voice_dma[1]->out_dack_callback<0>().set(FUNC(s900_state::voice_dreq_clear_w<4>));
	m_voice_dma[1]->out_dack_callback<1>().set(FUNC(s900_state::voice_dreq_clear_w<5>));
	m_voice_dma[1]->out_dack_callback<2>().set(FUNC(s900_state::voice_dreq_clear_w<6>));
	m_voice_dma[1]->out_dack_callback<3>().set(FUNC(s900_state::voice_dreq_clear_w<7>));

	// voice 0-2 sample clocks
	PIT8254(config, m_sample_timer[0]);
	m_sample_timer[0]->set_clk<0>(clk/2);
	m_sample_timer[0]->set_clk<1>(clk/2);
	m_sample_timer[0]->set_clk<2>(clk/2);
	m_sample_timer[0]->out_handler<0>().set(FUNC(s900_state::voice_dreq_set_w<0>));
	m_sample_timer[0]->out_handler<1>().set(FUNC(s900_state::voice_dreq_set_w<1>));
	m_sample_timer[0]->out_handler<2>().set(FUNC(s900_state::voice_dreq_set_w<2>));

	// voice 3-5 sample clocks
	PIT8254(config, m_sample_timer[1]);
	m_sample_timer[1]->set_clk<0>(clk/2);
	m_sample_timer[1]->set_clk<1>(clk/2);
	m_sample_timer[1]->set_clk<2>(clk/2);
	m_sample_timer[1]->out_handler<0>().set(FUNC(s900_state::voice_dreq_set_w<3>));
	m_sample_timer[1]->out_handler<1>().set(FUNC(s900_state::voice_dreq_set_w<4>));
	m_sample_timer[1]->out_handler<2>().set(FUNC(s900_state::voice_dreq_set_w<5>));

	// voice 6-7 sample clocks + RS232 clock (S900) / MIDI clock (S950)
	PIT8254(config, m_sample_timer[2]);
	m_sample_timer[2]->set_clk<0>(clk/2);
	m_sample_timer[2]->set_clk<1>(clk/2);
	m_sample_timer[2]->set_clk<2>(clk/2);
	m_sample_timer[2]->out_handler<0>().set(FUNC(s900_state::voice_dreq_set_w<6>));
	m_sample_timer[2]->out_handler<1>().set(FUNC(s900_state::voice_dreq_set_w<7>));

	// voice 0-2 filter clocks
	PIT8253(config, m_filter_timer[0]);
	m_filter_timer[0]->set_clk<0>(clk/4);
	m_filter_timer[0]->set_clk<1>(clk/4);
	m_filter_timer[0]->set_clk<2>(clk/4);

	// voice 3-5 filter clocks
	PIT8253(config, m_filter_timer[1]);
	m_filter_timer[1]->set_clk<0>(clk/4);
	m_filter_timer[1]->set_clk<1>(clk/4);
	m_filter_timer[1]->set_clk<2>(clk/4);

	// voice 6-7 filter clocks
	PIT8253(config, m_filter_timer[2]);
	m_filter_timer[2]->set_clk<0>(clk/4);
	m_filter_timer[2]->set_clk<1>(clk/4);
	m_filter_timer[2]->set_clk<2>(clk/32);
}

/**************************************************************************/
void s900_state::s900(machine_config &config)
{
	base_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &s900_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &s900_state::io_map);

	m_dma->in_io16r_callback<0>().set(FUNC(s900_state::adc_r));
	m_dma->in_mem16r_callback().set(FUNC(s900_state::voice_dma_mem_r));
	m_dma->out_mem16w_callback().set(FUNC(s900_state::voice_dma_mem_w));
	m_dma->out_dack_callback<0>().set(FUNC(s900_state::adc_dreq_clear_w));

	m_voice_dma[0]->in_mem16r_callback().set(FUNC(s900_state::voice_dma_mem_r));
	m_voice_dma[0]->out_mem16w_callback().set(FUNC(s900_state::voice_dma_mem_w));
	m_voice_dma[1]->in_mem16r_callback().set(FUNC(s900_state::voice_dma_mem_r));
	m_voice_dma[1]->out_mem16w_callback().set(FUNC(s900_state::voice_dma_mem_w));

	m_sample_timer[0]->out_handler<0>().append(FUNC(s900_state::adc_dreq_set_w));

	auto& midi_clock(CLOCK(config, "midi_clock", 16_MHz_XTAL / 32));
	midi_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	midi_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));

	auto &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "null_modem"));

	auto &i8251(I8251(config, "i8251", 16_MHz_XTAL / 2));
	i8251.rxrdy_handler().set(m_nmi, FUNC(input_merger_any_high_device::in_w<NMI_RS232>));
	i8251.rts_handler().set(rs232, FUNC(rs232_port_device::write_rts));
	i8251.txd_handler().set(rs232, FUNC(rs232_port_device::write_txd));
	rs232.cts_handler().set(i8251, FUNC(i8251_device::write_cts));
	rs232.rxd_handler().set(i8251, FUNC(i8251_device::write_rxd));

	m_sample_timer[2]->out_handler<2>().set(i8251, FUNC(i8251_device::write_txc));
	m_sample_timer[2]->out_handler<2>().append(i8251, FUNC(i8251_device::write_rxc));

	UPD7265(config, m_fdc, 16_MHz_XTAL / 4);
	FLOPPY_CONNECTOR(config, m_floppy, s900_floppies, "35dd", floppy_formats).enable_sound(true);
}

/**************************************************************************/
void s950_state::s950(machine_config &config)
{
	base_config(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &s950_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &s950_state::io_map);

	// stock hardware has 512kw RAM w/ two expansion slots
	// original expansion boards are 512kw each, OS supports any multiple of 512kw up to 7680kw total
	RAM(config, m_ram).set_default_size("3M").set_extra_options("1M,2M,4M,5M,6M,7M,8M,9M,10M,11M,12M,13M,14M,15M");
	ADDRESS_MAP_BANK(config, m_voice_bank).set_options(ENDIANNESS_LITTLE, 16, 24);
	for (int i = 0; i < 4; i++)
		ADDRESS_MAP_BANK(config, m_cpu_bank[i]).set_options(ENDIANNESS_LITTLE, 16, 24, 0x10000);

	m_dma->in_memr_callback().set(m_voice_bank, FUNC(address_map_bank_device::read8));
	m_dma->out_memw_callback().set(m_voice_bank, FUNC(address_map_bank_device::write8));
	m_dma->in_mem16r_callback().set(m_voice_bank, FUNC(address_map_bank_device::read16));
	m_dma->out_mem16w_callback().set(m_voice_bank, FUNC(address_map_bank_device::write16));
	m_dma->in_ior_callback<0>().set(FUNC(s950_state::dma_io_r));
	m_dma->in_io16r_callback<0>().set(FUNC(s950_state::dma_io_r));
	m_dma->out_iow_callback<0>().set(FUNC(s950_state::dma_io_w));
	m_dma->out_io16w_callback<0>().set(FUNC(s950_state::dma_io_w));
	m_dma->out_dack_callback<0>().set(FUNC(s950_state::dma_dack_w));
	m_dma->out_eop_callback().set(FUNC(s950_state::dma_eop_w)).invert();

	m_voice_dma[0]->in_mem16r_callback().set(m_voice_bank, FUNC(address_map_bank_device::read16));
	m_voice_dma[0]->out_mem16w_callback().set(m_voice_bank, FUNC(address_map_bank_device::write16));
	m_voice_dma[1]->in_mem16r_callback().set(m_voice_bank, FUNC(address_map_bank_device::read16));
	m_voice_dma[1]->out_mem16w_callback().set(m_voice_bank, FUNC(address_map_bank_device::write16));

	m_sample_timer[0]->out_handler<0>().append(FUNC(s950_state::adc_dreq_set_w));

	m_sample_timer[2]->out_handler<2>().set(m_acia, FUNC(acia6850_device::write_txc));
	m_sample_timer[2]->out_handler<2>().append(m_acia, FUNC(acia6850_device::write_rxc));

	UPD72066(config, m_fdc, 16_MHz_XTAL / 4);
	m_fdc->set_select_lines_connected(false);
	m_fdc->drq_wr_callback().set(FUNC(s950_state::dma_dreq_w<DMA_FDC>));
	FLOPPY_CONNECTOR(config, m_floppy, s950_floppies, "35hd", floppy_formats).enable_sound(true);
}

/**************************************************************************/
void s950_state::s950scsi(machine_config &config)
{
	s950(config);
	m_maincpu->set_addrmap(AS_IO, &s950_state::io_map_scsi);

	auto &scsi(NSCSI_BUS(config, "scsi"));
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);

	MB89352(config, m_spc, 8_MHz_XTAL); // actual type guessed
	scsi.set_external_device(7, m_spc);
	m_spc->out_irq_callback().set([this](int state){ m_hd_irq = state; });
	m_spc->out_dreq_callback().set(FUNC(s950_state::dma_dreq_w<DMA_HD>));

	ADDRESS_MAP_BANK(config, m_scsi_regs);
	m_scsi_regs->set_addrmap(0, &s950_state::scsi_reg_map);
	m_scsi_regs->set_addr_width(4);
	m_scsi_regs->set_data_width(8);
}

/**************************************************************************/
void s900_state::disk_motor_w(int state)
{
	auto fd = m_floppy->get_device();
	if (fd)
	{
		fd->mon_w(state);
		fd->inuse_w(state);
	}
}

/**************************************************************************/
ioport_value s950_state::floppy_density_r()
{
	auto fd = m_floppy->get_device();
	if (fd)
	{
		return fd->floppy_is_hd();
	}

	return 0;
}

/**************************************************************************/
void s900_state::media_ctrl_w(u8 data)
{
	disk_motor_w(BIT(~data, 0));
	m_fdc->tc_w(BIT(data, 2));
	m_fdc->reset_w(BIT(~data, 3));

	if (BIT(data, 3))
		m_rom_view.disable();
	else
		m_rom_view.select(0);
}

/**************************************************************************/
void s950_state::media_ctrl_w(u8 data)
{
	disk_motor_w(BIT(~data, 0));
	if (BIT(data, 0))
		m_fdc->set_floppy(m_floppy->get_device());
	else
		m_fdc->set_floppy(nullptr);

	m_fdc->set_rate(BIT(data, 1) ? 250000 : 500000);
	m_fdc->reset_w(BIT(~data, 3));

	m_dma_target = BIT(data, 4, 2);
}

/**************************************************************************/
HD44780_PIXEL_UPDATE(s900_state::lcd_update)
{
	if (x < 6 && y < 8 && line < 2 && pos < 40)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

/**************************************************************************/
u8 s900_state::key_rows_r()
{
	u8 data = 0xff;
	for (int i = 0; i < 5; i++)
		if (m_keys[i]->read() != 0xff)
			data &= ~(1 << i);

	return data;
}

/**************************************************************************/
u8 s900_state::key_cols_r()
{
	u8 data = 0xff;
	for (int i = 0; i < 5; i++)
		data &= m_keys[i]->read();

	return data;
}

/**************************************************************************/
INPUT_CHANGED_MEMBER(s900_state::dial_w)
{
	if (!BIT(oldval, 7) && BIT(newval, 7))
	{
		const u8 dir = BIT(newval, 7) ^ BIT(newval, 6);
		if (dir && m_dial_count < 15)
			m_dial_count++;
		else if (!dir && m_dial_count > 0)
			m_dial_count--;
	}
}

/**************************************************************************/
template <int Voice>
void s900_state::voice_dreq_set_w(int state)
{
	// DREQ generated on both clock edges
	m_voice_dma[Voice / 4]->dreq_w<Voice % 4>(0);
}

/**************************************************************************/
template <int Voice>
void s900_state::voice_dreq_clear_w(int state)
{
	if (!state)
	{
		m_current_voice = Voice;
		m_voice_dma[Voice / 4]->dreq_w<Voice % 4>(1);
	}
}

/**************************************************************************/
template <int Num>
void s900_state::voice_eop_w(int state)
{
	m_voice_eop[Num] = state;
	if ((m_current_voice / 4) == Num)
		m_pic->set_irq_line(m_current_voice, !state);
}

/**************************************************************************/
void s900_state::adc_dreq_set_w(int state)
{
	m_dma->dreq0_w(1);
}

/**************************************************************************/
void s950_state::adc_dreq_set_w(int state)
{
	dma_dreq_w<DMA_ADC>(1);
}

/**************************************************************************/
void s900_state::adc_dreq_clear_w(int state)
{
	if (state)
		m_dma->dreq0_w(0);
}

/**************************************************************************/
u16 s900_state::adc_r()
{
	return m_adc->read() << 4;
}

/**************************************************************************/
u16 s900_state::voice_dma_mem_r(offs_t offset)
{
	/*
	The S900's velocity crossfade works by encoding the fade position in the upper 4 address bits,
	reading the 'soft' and 'loud' samples from separate DRAM banks simultaneously, and then
	mixing them via 3 identical lookup table ROMs and binary adders.
	For non-crossfaded samples, the fade position is just used to select one of the DRAM banks.
	*/
	const offs_t base = BIT(offset, 0, 18) << 1;
	const u8 fade = BIT(offset, 19, 4);

	const u16 soft = m_maincpu->space(AS_PROGRAM).read_word(base);
	const u16 loud = m_maincpu->space(AS_PROGRAM).read_word(base | (1 << 19));

	const offs_t key1 = BIT(soft, 4, 4) | (BIT(loud, 4, 4) << 4) | (fade << 8);
	const offs_t key2 = BIT(soft, 8, 4) | (BIT(loud, 8, 4) << 4) | (fade << 8);
	const offs_t key3 = BIT(soft, 12, 4) | (BIT(loud, 12, 4) << 4) | (fade << 8);

	return (m_fadetbl->as_u8(key1) & 0xf0) + (m_fadetbl->as_u8(key2) << 4) + (m_fadetbl->as_u8(key3) << 8);
}

/**************************************************************************/
void s900_state::voice_dma_mem_w(offs_t offset, u16 data)
{
	m_maincpu->space(AS_PROGRAM).write_word(offset << 1, data);
}

/**************************************************************************/
template <int Voice>
void s900_state::voice_dma_io_w(offs_t offset, u16 data)
{
	m_dac[Voice]->write(data >> 4);
}

/**************************************************************************/
void s900_state::envelope_w(offs_t offset, u8 data)
{
	m_dac[BIT(offset, 7, 3)]->set_output_gain(0, data / 255.0);
}

/**************************************************************************/
void s950_state::bank_w(offs_t offset, u8 data)
{
	m_cpu_bank[offset & 3]->set_bank(data);
}

/**************************************************************************/
void s950_state::update_dreq()
{
	m_dma->dreq0_w(m_dreq[m_dma_target & 3]);
}

/**************************************************************************/
template <int Num>
void s950_state::dma_dreq_w(int state)
{
	m_dreq[Num] = state;
	update_dreq();
}

/**************************************************************************/
void s950_state::dma_dack_w(int state)
{
	switch (m_dma_target & 3)
	{
	case DMA_ADC:
		if (state)
			dma_dreq_w<DMA_ADC>(0);
		break;
	}
}

/**************************************************************************/
void s950_state::dma_eop_w(int state)
{
	if ((m_dma_target & 3) == DMA_FDC)
		m_fdc->tc_line_w(state);
}

/**************************************************************************/
u16 s950_state::dma_io_r(offs_t offset)
{
	switch (m_dma_target & 3)
	{
	case DMA_ADC:
		return adc_r();

	case DMA_FDC:
		return m_fdc->dma_r();

	case DMA_HD:
		if (m_spc)
			return m_spc->dma_r();
		break;
	}

	return 0;
}

/**************************************************************************/
void s950_state::dma_io_w(offs_t offset, u16 data)
{
	switch (m_dma_target & 3)
	{
	case DMA_FDC:
		m_fdc->dma_w(data);
		break;

	case DMA_HD:
		if (m_spc)
			m_spc->dma_w(data);
		break;
	}
}


static INPUT_PORTS_START( s900 )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Master Tune")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Disk")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Utility")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("MIDI")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Edit Prog")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Edit Sample")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Rec")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Play")

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Cursor Right")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Cursor Left")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 9")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 8")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 7")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Cursor Down")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Cursor Up")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 6")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 5")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 4")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Letter")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Space")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 2")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 1")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Play Back")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Enter")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Off / -")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("On / +")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Keypad 0")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIAL")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Control") PORT_SENSITIVITY(50) PORT_KEYDELTA(75) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s900_state::dial_w), 0)

	PORT_START("PORT6A")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s900_state::fdc_irq_r))
	PORT_BIT( 0x06, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("Rec/PB Trigger")
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( s950 )
	PORT_INCLUDE(s900)

	PORT_MODIFY("PORT6A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s950_state::floppy_density_r))
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s950_state::hd_irq_r))
INPUT_PORTS_END


ROM_START( s900 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v12c", "Version 1.2c")
	ROMX_LOAD("s900 v1.2c lsb.ic12", 0x0000, 0x4000, CRC(d0925ed4) SHA1(6c07a9baf4dfe59de5362e1f44e9a28e08c1b765), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("s900 v1.2c msb.ic11", 0x0001, 0x4000, CRC(24ccb3e0) SHA1(bcdbfb590f894849716e40722f82b4302de3366b), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "v12a", "Version 1.2a")
	ROMX_LOAD("akai s900 v1.2a l.ic12", 0x0000, 0x4000, CRC(1077a91a) SHA1(0cb91474300f3dc090375e33ca73dd4d7a438f88), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("akai s900 v1.2a m.ic11", 0x0001, 0x4000, CRC(558e402d) SHA1(6dce63a2216c4d4b015e0abf9b9ea492e776b201), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_REGION(0x1000, "fadetbl", 0)
	// 3 copies at IC73, IC74, IC75 used to calculate velocity crossfade (2764, A12 tied high)
	// lower half appears to be the equivalent for unsigned samples
	ROM_LOAD("akai s900 r v1.0.ic73", 0x0000, 0x1000, CRC(6efe6486) SHA1(f9026d6399fd68a6098311f39d3f02cb82609726))
	ROM_CONTINUE(0x0000, 0x1000)
ROM_END

ROM_START( s950 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v12a", "Version 1.2a")
	ROMX_LOAD("s950_1_2a_lsb.ic12", 0x0000, 0x8000, CRC(21af5435) SHA1(df0f6bcadadb28af1ca72562b3cea91a67d2b221), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("s950_1_2a_msb.ic11", 0x0001, 0x8000, CRC(e14ceba2) SHA1(3b3d2ac74f418223c7a9b0b1437d1bb71f2189c6), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "v10a", "Version 1.0a")
	ROMX_LOAD("s950 l 1.0a.ic12", 0x0000, 0x8000, CRC(9745b78a) SHA1(b43aeaf2a08728a1a8c832a205de10c9372e8cc1), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("s950 m 1.0a.ic11", 0x0001, 0x8000, CRC(81dc95cb) SHA1(2ef684e1211cfb0f9c8721f618c7944da2758dea), ROM_BIOS(1) | ROM_SKIP(1))
ROM_END

ROM_START( s950scsi )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD16_BYTE("s950_1_2b_lsb.ic12", 0x0000, 0x8000, CRC(01e0c3ad) SHA1(a374289a6110d97ac0871bdfbe8defe0036dc928))
	ROM_LOAD16_BYTE("s950_1_2b_msb.ic11", 0x0001, 0x8000, CRC(5b5d25cf) SHA1(05d9c4c21ed60c289ea6ec23308c6454b43b200e))
ROM_END

} // anonymous namespace

SYST( 1986, s900,      0,        0, s900,     s900,  s900_state,  empty_init, "Akai", "S900 MIDI Digital Sampler", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
SYST( 1988, s950,      0,        0, s950,     s950,  s950_state,  empty_init, "Akai", "S950 MIDI Digital Sampler", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
SYST( 1988, s950scsi,  s950,     0, s950scsi, s950,  s950_state,  empty_init, "Akai", "S950 MIDI Digital Sampler (with SCSI)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

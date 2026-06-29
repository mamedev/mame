// license:BSD-3-Clause
// copyright-holders:Devin Acker

/***************************************************************************
	Akai S1000, S1100 samplers

	These are the 16-bit successors to the 12-bit S900/S950 family, and are the first S-series
	samplers to use dedicated sound hardware instead of off-the-shelf DMA controllers.

	TODO:
	- layouts
	- some sound hardware details (see devices/sound/l6009.cpp)
	- software list for floppies
	- make sure SCSI hard disks work (currently only tested with CDs)
	- S1000KB support (same ROMs as S1000 plus extra key scan MCU, probably uPD7811 like X7000)
	- S1100EX support? (expander version of S1100, connects over both MIDI and SCSI)
	- DSP56001 emulation is needed for S1100 onboard effects
***************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "bus/nscsi/devices.h"
#include "cpu/dsp56000/dsp56000.h"
#include "cpu/nec/v5x.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/mb87030.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "sound/adc.h"
#include "sound/l6009.h"
#include "sound/mixer.h"
#include "video/hd61830.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/s900_dsk.h"
#include "formats/hxchfe_dsk.h"

namespace {
	
class s1000_state : public driver_device
{
public:
	s1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_l6009(*this, "l6009")
		, m_lcdc(*this, "lcdc")
		, m_ppi(*this, "ppi")
		, m_hc259(*this, "hc259")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "floppy")
		, m_spc(*this, "spc")
		, m_ram(*this, "ram")
		, m_adc(*this, "adc%u", 0)
		, m_keys(*this, "KEY%u", 0)
		, m_led(*this, "led%u", 0U)
	{}

	void s1000pb(machine_config &config) ATTR_COLD;
	void s1000(machine_config &config) ATTR_COLD;

	ioport_value keys_r();

	DECLARE_INPUT_CHANGED_MEMBER(dial_w);
	template <int Num> ioport_value dial_r() { return m_dial_pos[Num]; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	// s1000/s1000kb relies on this behavior to detect if the internal keyboard is present
	u16 unmap_r(offs_t offset) { return offset << 1; }

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void lcd_map(address_map &map) ATTR_COLD;
	void sound_rom_map(address_map &map) ATTR_COLD;

	void disk_motor_w(int state);
	void disk_rate_w(int state);

	u8 dma_mem8_r(offs_t offset);
	void dma_mem8_w(offs_t offset, u8 data);
	u16 dma_mem16_r(offs_t offset);
	void dma_mem16_w(offs_t offset, u16 data);

	u8 dma2_io_r(offs_t offset);
	void dma2_io_w(offs_t offset, u8 data);

	void adc_clock_w(int state);
	template <int Num> void adc_dack_w(int state);

	void update_dreq();
	void fdc_sel_w(int state);
	template <int Num> void dma_dreq_w(int state);

	void leds_w(u8 data);

	required_device<v50_device> m_maincpu;
	required_device<l6009_device> m_l6009;
	required_device<hd61830_device> m_lcdc;
	required_device<i8255_device> m_ppi;
	required_device<hc259_device> m_hc259;

	required_device<upd765_family_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<mb89352_device> m_spc;

	required_device<ram_device> m_ram;

	optional_device_array<adc16_device, 2> m_adc;

	required_ioport_array<8> m_keys;
	output_finder<8> m_led;

	u8 m_dial_pos[2];

	u8 m_fdc_sel;
	u8 m_dmarq2[2];
};

class s1100_state : public s1000_state
{
public:
	s1100_state(const machine_config &mconfig, device_type type, const char *tag)
		: s1000_state(mconfig, type, tag)
		, m_dsp(*this, "dsp")
	{}

	void s1100(machine_config &config) ATTR_COLD;

protected:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<dsp56001_device> m_dsp;
};


/**************************************************************************/
void s1000_state::machine_start()
{
	m_l6009->space(l6009_device::AS_RAM).install_ram(0, m_ram->mask() >> 1, m_ram->pointer());

	m_dial_pos[0] = m_dial_pos[1] = 0;

	m_fdc_sel = 0;
	m_dmarq2[0] = m_dmarq2[1] = 0;

	save_item(NAME(m_dial_pos));
	save_item(NAME(m_fdc_sel));
	save_item(NAME(m_dmarq2));
}

/**************************************************************************/
void s1000_state::machine_reset()
{
}

/**************************************************************************/
void s1000_state::mem_map(address_map &map)
{
	map(0x00000, 0xfffff).r(FUNC(s1000_state::unmap_r));
	map(0x00000, 0x3ffff).ram();
	map(0xe0000, 0xfffff).rom().region("maincpu", 0);
}

/**************************************************************************/
void s1100_state::mem_map(address_map &map)
{
	map(0x00000, 0xfffff).r(FUNC(s1100_state::unmap_r));
	map(0x00000, 0x7ffff).ram();
	map(0xc0000, 0xfffff).rom().region("maincpu", 0);
}

/**************************************************************************/
void s1000_state::io_map(address_map &map)
{
	map(0x0000, 0xffff).r(FUNC(s1100_state::unmap_r));
	map(0x0000, 0x001f).mirror(0x0fe0).umask16(0x00ff).m(m_spc, FUNC(mb89352_device::map));
	map(0x1000, 0x1003).mirror(0x0ffc).umask16(0x00ff).m(m_fdc, FUNC(upd72066_device::map));
	map(0x2000, 0x2fff).umask16(0x00ff).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x3000, 0x3fff).umask16(0x00ff).portr("P3000").w(m_hc259, FUNC(hc259_device::write_a0));
	map(0x4000, 0x4fff).rw(m_l6009, FUNC(l6009_device::itp_r), FUNC(l6009_device::itp_w));
	map(0x5000, 0x5fff).rw(m_l6009, FUNC(l6009_device::flr_r), FUNC(l6009_device::flr_w));
	map(0x6000, 0x6000).mirror(0x0ff8).umask16(0x00ff).w(m_lcdc, FUNC(hd61830_device::data_w));
	map(0x6002, 0x6002).mirror(0x0ff8).umask16(0x00ff).r(m_lcdc, FUNC(hd61830_device::data_r));
	map(0x6004, 0x6004).mirror(0x0ff8).umask16(0x00ff).w(m_lcdc, FUNC(hd61830_device::control_w));
	map(0x6006, 0x6006).mirror(0x0ff8).umask16(0x00ff).r(m_lcdc, FUNC(hd61830_device::status_r));
}

/**************************************************************************/
void s1100_state::io_map(address_map &map)
{
	s1000_state::io_map(map);
	map(0x1000, 0x1003).mirror(0x0ffc).umask16(0x00ff).m(m_fdc, FUNC(upd72069_device::map));
	// 7800: DSP host interface
	// TODO: this is only to get s1100 to boot
	map(0x7814, 0x7815).umask16(0x00ff).lr8(NAME([]() { return 0x02; }));
}

/**************************************************************************/
void s1000_state::lcd_map(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x000, 0x7ff).ram();
}

/**************************************************************************/
void s1000_state::sound_rom_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

/**************************************************************************/
static void s1000_floppies(device_slot_interface &device)
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
void s1000_state::s1000pb(machine_config &config)
{
	V50(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_tclk(16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &s1000_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &s1000_state::io_map);
	m_maincpu->out_hreq_cb().set(m_maincpu, FUNC(v50_device::hack_w));
	m_maincpu->in_memr_cb().set(FUNC(s1000_state::dma_mem8_r));
	m_maincpu->in_mem16r_cb().set(FUNC(s1000_state::dma_mem16_r));
	m_maincpu->out_memw_cb().set(FUNC(s1000_state::dma_mem8_w));
	m_maincpu->out_mem16w_cb().set(FUNC(s1000_state::dma_mem16_w));
	m_maincpu->in_ior_cb<2>().set(FUNC(s1000_state::dma2_io_r));
	m_maincpu->out_iow_cb<2>().set(FUNC(s1000_state::dma2_io_w));
	m_maincpu->out_eop_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ2);
	m_maincpu->out_eop_cb().append(m_fdc, FUNC(upd765_family_device::tc_line_w));

	RAM(config, m_ram).set_default_size("32M").set_extra_options("2M,4M,6M,8M,16M,24M");

	I8255(config, m_ppi);
	m_ppi->in_pa_callback().set_ioport("PA");
	m_ppi->out_pc_callback().set(FUNC(s1000_state::leds_w));

	HC259(config, m_hc259);
	// TODO: bit 0 = ADC clock (22/44 kHz)
	// TODO: bit 1 = mute
	m_hc259->q_out_cb<2>().set(FUNC(s1000_state::fdc_sel_w));
	m_hc259->q_out_cb<3>().set(FUNC(s1000_state::disk_motor_w));
	// TODO: bit 4 = input monitor
	// bit 5 = floppy density select (output to pin 2 of drive connector)
	m_hc259->q_out_cb<6>().set(FUNC(s1000_state::disk_rate_w));
	m_hc259->q_out_cb<7>().set(m_fdc, FUNC(upd765_family_device::reset_w));

	// MIDI
	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_maincpu, FUNC(v50_device::rxd_w));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_maincpu->txd_handler_cb().set(mdout, FUNC(midi_port_device::write_txd));

	auto &mdthru(MIDI_PORT(config, "mdthru"));
	midiout_slot(mdthru);
	mdin.rxd_handler().append(mdthru, FUNC(midi_port_device::write_txd));

	// LCD
	HD61830(config, m_lcdc, 16_MHz_XTAL / 8); // LC7981
	m_lcdc->set_addrmap(0, &s1000_state::lcd_map);
	m_lcdc->set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(80);
	screen.set_screen_update(m_lcdc, FUNC(hd61830_device::screen_update));
	screen.set_size(240, 64);
	screen.set_visarea(0, 240-1, 0, 64-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	// sound
	SPEAKER(config, "mono", 8);
	SPEAKER(config, "stereo", 2).front();
	SPEAKER(config, "fx").front_center();

	L6009(config, m_l6009, 33.8688_MHz_XTAL);
	m_l6009->set_addrmap(l6009_device::AS_ROM, &s1000_state::sound_rom_map);
	for (int i = 0; i < 8; i++)
		m_l6009->add_route(l6009_device::OUT_MONO + i, "mono", 1.0, i);
	m_l6009->add_route(l6009_device::OUT_STEREO_L, "stereo", 1.0, 0);
	m_l6009->add_route(l6009_device::OUT_STEREO_R, "stereo", 1.0, 1);
	m_l6009->add_route(l6009_device::OUT_EFFECT, "fx", 1.0);

	auto &scsi(NSCSI_BUS(config, "scsi"));
	// default SCSI IDs are 5 for the drive, 6 for the sampler
	// let's default to CD-ROM on 5 for ease of use w/ software lists
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, "cdrom_2x");
	NSCSI_CONNECTOR(config, "scsi:7", default_scsi_devices, nullptr);

	MB89352(config, m_spc, 16_MHz_XTAL / 2);
	scsi.set_external_device(6, m_spc);
	m_spc->out_irq_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ3);
	m_spc->out_dreq_callback().set(FUNC(s1000_state::dma_dreq_w<0>));

	UPD72066(config, m_fdc, 16_MHz_XTAL / 4);
	m_fdc->set_select_lines_connected(false);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ4);
	m_fdc->drq_wr_callback().set(FUNC(s1000_state::dma_dreq_w<1>));

	FLOPPY_CONNECTOR(config, m_floppy, s1000_floppies, "35hd", floppy_formats).enable_sound(true);

	// TODO: S900/S950, S1000 floppy softlists
	SOFTWARE_LIST(config, "cd_list").set_compatible("s3000_cdrom");
}

/**************************************************************************/
void s1000_state::s1000(machine_config &config)
{
	s1000pb(config);

	ADC16(config, m_adc[0]);
	ADC16(config, m_adc[1]);

	auto &input_mixer(MIXER(config, "input"));
	input_mixer.add_route(0, m_adc[0], 1.0);
	input_mixer.add_route(1, m_adc[1], 1.0);

	auto &mic(MICROPHONE(config, "mic", 2));
	mic.add_route(0, input_mixer, 1.0, 0);
	mic.add_route(1, input_mixer, 1.0, 1);

	auto& linein(CASSETTE(config, "linein"));
	linein.set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	linein.add_route(0, input_mixer, 1.0, 0);
	linein.add_route(0, input_mixer, 1.0, 1);

	// 44.1khz sample clock generates ADC DMA requests but also enables dial controls instead of cursor keys
	m_l6009->sample_clock_cb().set(FUNC(s1000_state::adc_clock_w));

	m_maincpu->in_io16r_cb<0>().set(m_adc[0], FUNC(adc16_device::read));
	m_maincpu->in_io16r_cb<1>().set(m_adc[1], FUNC(adc16_device::read));
	m_maincpu->out_dack_cb<0>().set(FUNC(s1000_state::adc_dack_w<0>));
	m_maincpu->out_dack_cb<1>().set(FUNC(s1000_state::adc_dack_w<1>));
}

/**************************************************************************/
void s1100_state::s1100(machine_config &config)
{
	s1000(config);
	m_maincpu->set_unscaled_clock(20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &s1100_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &s1100_state::io_map);

	DSP56001(config, m_dsp, 20_MHz_XTAL).set_disable();

	m_hc259->q_out_cb<5>().set(FUNC(s1100_state::disk_rate_w));
	m_hc259->q_out_cb<6>().set_nop(); // TODO: DSP output routing

	UPD72069(config.replace(), m_fdc, 32_MHz_XTAL);
	m_fdc->set_select_lines_connected(false);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ4);
	m_fdc->drq_wr_callback().set(FUNC(s1100_state::dma_dreq_w<1>));
}

/**************************************************************************/
void s1000_state::disk_motor_w(int state)
{
	auto fd = m_floppy->get_device();
	if (fd)
	{
		fd->mon_w(!state);
		fd->inuse_w(!state);
	}

	if (state)
		m_fdc->set_floppy(fd);
	else
		m_fdc->set_floppy(nullptr);
}

/**************************************************************************/
void s1000_state::disk_rate_w(int state)
{
	m_fdc->set_rate(state ? 250000 : 500000);
}

/**************************************************************************/
u8 s1000_state::dma_mem8_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

/**************************************************************************/
void s1000_state::dma_mem8_w(offs_t offset, u8 data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

/**************************************************************************/
u16 s1000_state::dma_mem16_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_word(offset << 1);
}

/**************************************************************************/
void s1000_state::dma_mem16_w(offs_t offset, u16 data)
{
	m_maincpu->space(AS_PROGRAM).write_word(offset << 1, data);
}

/**************************************************************************/
u8 s1000_state::dma2_io_r(offs_t offset)
{
	if (m_fdc_sel)
		return m_fdc->dma_r();

	return m_spc->dma_r();
}

/**************************************************************************/
void s1000_state::dma2_io_w(offs_t offset, u8 data)
{
	if (m_fdc_sel)
		m_fdc->dma_w(data);
	else
		m_spc->dma_w(data);
}

/**************************************************************************/
void s1000_state::adc_clock_w(int state)
{
	if (!state)
		m_maincpu->dreq_w<0>(1);
	else
		m_maincpu->dreq_w<1>(1);
}

/**************************************************************************/
template <int Num>
void s1000_state::adc_dack_w(int state)
{
	if (!state)
		m_maincpu->dreq_w<Num>(0);
}

/**************************************************************************/
void s1000_state::update_dreq()
{
	m_maincpu->dreq_w<2>(m_dmarq2[m_fdc_sel & 1]);
}

/**************************************************************************/
void s1000_state::fdc_sel_w(int state)
{
	m_fdc_sel = state;
	update_dreq();
}

/**************************************************************************/
template <int Num>
void s1000_state::dma_dreq_w(int state)
{
	m_dmarq2[Num] = state;
	update_dreq();
}

/**************************************************************************/
ioport_value s1000_state::keys_r()
{
	u8 data = 0x0f;
	const u8 sel = m_ppi->pb_r();

	for (int i = 0; i < 8; i++)
		if (!BIT(sel, i))
			data &= m_keys[i]->read();

	return data;
}

/**************************************************************************/
INPUT_CHANGED_MEMBER(s1000_state::dial_w)
{
	// 2-bit gray code, low bits connected to INTP5 and 6
	const u8 pos = (newval >> 7) ^ (newval >> 6);
	if (BIT(pos ^ m_dial_pos[param], 0))
		m_maincpu->set_input_line(INPUT_LINE_IRQ5 + param, BIT(pos, 0));

	m_dial_pos[param] = pos;
}

/**************************************************************************/
void s1000_state::leds_w(u8 data)
{
	for (int i = 0; i < 8; i++)
		m_led[i] = BIT(~data, i);
}


static INPUT_PORTS_START( s1000pb )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Select Prog / I") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F1 / A")          PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("3 / Y")           PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME(u8"- / \u25b6")    PORT_CODE(KEYCODE_MINUS_PAD)

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Select Sample / J") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F2 / B")            PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Name")              PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Enter / Play")      PORT_CODE(KEYCODE_ENTER_PAD)

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Edit Prog / K") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F3 / C")        PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("7 / Q")         PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("4 / T")         PORT_CODE(KEYCODE_4_PAD)

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("MIDI / L") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F4 / D")   PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("8 / R")    PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("5 / U")    PORT_CODE(KEYCODE_5_PAD)

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Disk / M") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F5 / E")   PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("9 / S")    PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("6 / V")    PORT_CODE(KEYCODE_6_PAD)

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Master Tune / N") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F6 / F")          PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Mark / #")        PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Jump / .")        PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Drum / O")     PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F7 / G")       PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("2 / X")        PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME(u8"+ / \u25c0") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Option / P") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("F8 / H")     PORT_CODE(KEYCODE_8)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("1 / W")      PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("0 / Z")      PORT_CODE(KEYCODE_0_PAD)

	PORT_START("PA")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s1000_state::keys_r))
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_OTHER )  PORT_NAME(u8"Cursor \u25c0") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_OTHER )  PORT_NAME(u8"Cursor \u25b6") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_OTHER )  PORT_NAME(u8"Data \u25c0")   PORT_CODE(KEYCODE_DOWN)
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_OTHER )  PORT_NAME(u8"Data \u25b6")   PORT_CODE(KEYCODE_UP)

	PORT_START("P3000")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( s1000 )
	PORT_INCLUDE(s1000pb)

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("Edit Sample / J") PORT_CODE(KEYCODE_W)

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_NAME("Cursor") PORT_SENSITIVITY(50) PORT_KEYDELTA(75) PORT_CODE_DEC(KEYCODE_LEFT) PORT_CODE_INC(KEYCODE_RIGHT) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s1000_state::dial_w), 0)

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL_V ) PORT_NAME("Data") PORT_SENSITIVITY(50) PORT_KEYDELTA(75) PORT_CODE_DEC(KEYCODE_DOWN) PORT_CODE_INC(KEYCODE_UP) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(s1000_state::dial_w), 1)

	PORT_MODIFY("PA")
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s1000_state::dial_r<0>));
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(s1000_state::dial_r<1>));
	
	PORT_MODIFY("P3000")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER  ) PORT_NAME("Foot Switch")
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_OTHER  ) // high when foot switch plugged in
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


ROM_START( s1000 )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v440", "Version 4.40")
	ROMX_LOAD("s1000 44 lsb.ic9",  0x00000, 0x10000, CRC(82689608) SHA1(e0dd9663ffcc0272e66f2dfef15861965d16ec52), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("s1000 44 msb.ic10", 0x00001, 0x10000, CRC(b30a0b80) SHA1(82c380df4cc62ef820b2d7222509868251c753b5), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "v430", "Version 4.30")
	ROMX_LOAD("s100043 ls.ic9",  0x00000, 0x10000, CRC(61995ec5) SHA1(52135c8df887c166c81f2c801c5e962596465dde), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("s100043 ms.ic10", 0x00001, 0x10000, CRC(70ada357) SHA1(94dadcfac611d02a13ba6eb201eb487291185e3b), ROM_BIOS(1) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(2, "v221", "Version 2.21")
	ROMX_LOAD("s1000 221 lsb oct 91.ic9",  0x00000, 0x10000, CRC(d7302aa9) SHA1(2fe65e504da1c4dd20d93de6fe52bd0b2e62e094), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("s1000 221 msb oct 91.ic10", 0x00001, 0x10000, CRC(bc1097b1) SHA1(f9e58002a6e25b9014104e23a31b85c9d9d298eb), ROM_BIOS(2) | ROM_SKIP(1))

	ROM_REGION16_LE(0x20000, "l6009", 0)
	ROM_LOAD16_BYTE("s1000.ic11", 0x00000, 0x10000, CRC(9b80e447) SHA1(e3fa4a47dd75b95840d27f292a5c6114029743bb))
	ROM_LOAD16_BYTE("s1000.ic12", 0x00001, 0x10000, CRC(2c076351) SHA1(5194b48aca572d712f94268413f56f9500e30283))
ROM_END

#define rom_s1000pb rom_s1000

ROM_START( s1100 )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "v430", "Version 4.30")
	ROMX_LOAD("akai_s1100_osv4_30_lsb.ic10", 0x00000, 0x20000, CRC(8fa1e673) SHA1(8acc4d4df13cfc1135eef990c6a37e533315247c), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("akai_s1100_osv4_30_msb.ic11", 0x00001, 0x20000, CRC(82d90460) SHA1(8a0b2944600d9e919b49838219eeedee10a73a87), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "v202", "Version 2.02")
	ROMX_LOAD("v2_02_lsb.ic10", 0x00000, 0x20000, CRC(a97d1871) SHA1(771c7cc64bfcfe32e4524345e1f5c1b323f9fc66), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("v2_02_msb.ic11", 0x00001, 0x20000, CRC(677ba79e) SHA1(6be139ffd88c0007ca27d78c54e00e467c2f508d), ROM_BIOS(1) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(2, "v136", "Version 1.36")
	ROMX_LOAD("exv1_36_lsb.ic10", 0x00000, 0x20000, CRC(00c47d62) SHA1(fa2d5d9fc31ecb185d69a3ae83f10232e22be3ff), ROM_BIOS(2) | ROM_SKIP(1))
	ROMX_LOAD("exv1_36_msb.ic11", 0x00001, 0x20000, CRC(0a819d79) SHA1(418117aef29d107ad42ddc1c57d8796d8e8d43fe), ROM_BIOS(2) | ROM_SKIP(1))

	ROM_REGION16_LE(0x20000, "l6009", 0)
	ROM_LOAD16_BYTE("s1100.ic12", 0x00000, 0x10000, CRC(9b80e447) SHA1(e3fa4a47dd75b95840d27f292a5c6114029743bb))
	ROM_LOAD16_BYTE("s1100.ic13", 0x00001, 0x10000, CRC(2c076351) SHA1(5194b48aca572d712f94268413f56f9500e30283))

	ROM_REGION(0x600, "plds", 0)
	ROM_LOAD("peel18cv8.ic54", 0x000, 0x155, NO_DUMP) // on CPU board
	ROM_LOAD("peel18cv8.ic22", 0x200, 0x155, NO_DUMP) // on DSP board
	ROM_LOAD("peel18cv8.ic23", 0x400, 0x155, NO_DUMP) // on DSP board
ROM_END

} // anonymous namespace

SYST( 1988, s1000,   0,     0, s1000,   s1000,   s1000_state, empty_init, "Akai", "S1000 MIDI Stereo Digital Sampler",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
SYST( 1988, s1000pb, s1000, 0, s1000pb, s1000pb, s1000_state, empty_init, "Akai", "S1000PB MIDI Stereo Digital Sampler", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
SYST( 1990, s1100,   0,     0, s1100,   s1000,   s1100_state, empty_init, "Akai", "S1100 MIDI Stereo Digital Sampler",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

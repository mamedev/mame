// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg DSS-1 synthesizer.

****************************************************************************/

#include "emu.h"
#include "bus/midi/midi.h"
#include "bus/nscsi/devices.h"
#include "cpu/i8085/i8085.h"
#include "cpu/m6800/m6801.h"
#include "cpu/nec/v5x.h"
#include "imagedev/floppy.h"
#include "machine/gen_latch.h"
#include "machine/i8155.h"
#include "machine/i8255.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


namespace {

class korg_dss1_state : public driver_device
{
public:
	korg_dss1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cpu1(*this, "cpu1")
		, m_cpu2(*this, "cpu2")
		, m_io1(*this, "io1")
		, m_io2(*this, "io2")
		, m_latch(*this, "latch%u", 1U)
		, m_pit(*this, "pit%u", 1U)
		, m_fdc(*this, "fdc")
		, m_lcdc(*this, "lcdc")
		, m_rombank(*this, "rombank")
	{
	}

	void dss1(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u8 klm782_ga1_r(offs_t offset);
	void klm782_ga1_w(offs_t offset, u8 data);
	u8 klm782_ga2_1_r(offs_t offset);
	void klm782_ga2_1_w(offs_t offset, u8 data);
	u8 klm782_ga2_2_r(offs_t offset);
	void klm782_ga2_2_w(offs_t offset, u8 data);
	u8 klm782_ga3_1_r(offs_t offset);
	void klm782_ga3_1_w(offs_t offset, u8 data);
	u8 klm782_ga3_2_r(offs_t offset);
	void klm782_ga3_2_w(offs_t offset, u8 data);
	void dmaram_lsb_w(u8 data);
	void dmaram_msb_w(u8 data);
	u8 dmaram_lsb_r();
	u8 dmaram_msb_r();
	void mode_select_w(u8 data);

	void klm780(machine_config &config);
	void klm781(machine_config &config);
	void klm782(machine_config &config);

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void bank_switch_w(u8 data);
	void panel_led_w(u8 data);
	void fdc_tc_w(int state);
	void sed9420c_trgin_w(int state);
	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);

	void vcf_vca_ef_w(u8 data);
	u8 cpu2_p5_r();
	void ad_select_w(u8 data);
	void da_lsb_w(u8 data);
	void da_msb_w(u8 data);
	u8 kbd_r();
	void kbd_w(u8 data);

	void cpu1_map(address_map &map) ATTR_COLD;
	void cpu2_map(address_map &map) ATTR_COLD;

	void palette_init_dss1(palette_device &palette);

protected:
	optional_device<cpu_device> m_cpu1;
	required_device<hd6303x_cpu_device> m_cpu2;
	required_device<i8155_device> m_io1;
	required_device<i8255_device> m_io2;
	required_device_array<generic_latch_8_device, 2> m_latch;
	required_device_array<pit8253_device, 6> m_pit;
	required_device<upd765a_device> m_fdc;
	required_device<hd44780_device> m_lcdc;
	optional_memory_bank m_rombank;
};

class korg_dssmsrk_state : public korg_dss1_state
{
public:
	korg_dssmsrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: korg_dss1_state(mconfig, type, tag)
		, m_msrkcpu(*this, "msrkcpu")
		, m_scsic(*this, "scsi:7:scsic")
	{
	}

	void dssmsrk(machine_config &config);

private:
	void msrk_map(address_map &map) ATTR_COLD;
	void msrk_io_map(address_map &map) ATTR_COLD;

	u8 fdc_r(offs_t offset);
	void fdc_w(offs_t offset, u8 data);

	required_device<v40_device> m_msrkcpu;
	required_device<ncr53c80_device> m_scsic;
};

void korg_dss1_state::machine_start()
{
	if (m_rombank.found())
		m_rombank->configure_entries(0, 2, memregion("klm780")->base(), 0x8000);
}

void korg_dss1_state::machine_reset()
{
	if (m_rombank.found())
		m_rombank->set_entry(0);
}

HD44780_PIXEL_UPDATE(korg_dss1_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 20)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}


void korg_dss1_state::bank_switch_w(u8 data)
{
	m_rombank->set_entry(data & 0x01);
}

void korg_dss1_state::panel_led_w(u8 data)
{
	// TODO
	// TODO
}

void korg_dss1_state::fdc_tc_w(int state)
{
	if (m_cpu1.found())
		m_fdc->tc_w(state);
	else
	{
		// TODO: MSRK rejumpers this to control SED9420C's MIN/STD input instead
	}
}

void korg_dss1_state::sed9420c_trgin_w(int state)
{
	// TODO
}

u8 korg_dss1_state::fdc_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_cpu1->adjust_icount(-1);

	if (BIT(offset, 0))
		return m_fdc->fifo_r();
	else
		return m_fdc->msr_r();
}

u8 korg_dssmsrk_state::fdc_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_msrkcpu->adjust_icount(-1);

	if (BIT(offset, 0))
		return m_fdc->fifo_r();
	else
		return m_fdc->msr_r();
}

void korg_dss1_state::fdc_w(offs_t offset, u8 data)
{
	if (!machine().side_effects_disabled())
		m_cpu1->adjust_icount(-1);

	if (BIT(offset, 0))
		m_fdc->fifo_w(data);
}

void korg_dssmsrk_state::fdc_w(offs_t offset, u8 data)
{
	if (!machine().side_effects_disabled())
		m_msrkcpu->adjust_icount(-1);

	if (BIT(offset, 0))
		m_fdc->fifo_w(data);
}

void korg_dss1_state::vcf_vca_ef_w(u8 data)
{
	// TODO
}

u8 korg_dss1_state::cpu2_p5_r()
{
	// TODO: other bits
	return m_latch[0]->pending_r() | (m_latch[1]->pending_r() << 1);
}

void korg_dss1_state::ad_select_w(u8 data)
{
	// TODO
}

void korg_dss1_state::da_lsb_w(u8 data)
{
	// TODO
}

void korg_dss1_state::da_msb_w(u8 data)
{
	// TODO
}

u8 korg_dss1_state::kbd_r()
{
	// TODO
	return 0;
}

void korg_dss1_state::kbd_w(u8 data)
{
	// TODO
}

u8 korg_dss1_state::klm782_ga1_r(offs_t offset)
{
	// TODO
	return 0;
}

void korg_dss1_state::klm782_ga1_w(offs_t offset, u8 data)
{
	// TODO
}

u8 korg_dss1_state::klm782_ga2_1_r(offs_t offset)
{
	// TODO
	return 0;
}

void korg_dss1_state::klm782_ga2_1_w(offs_t offset, u8 data)
{
	// TODO
}

u8 korg_dss1_state::klm782_ga2_2_r(offs_t offset)
{
	// TODO
	return 0;
}

void korg_dss1_state::klm782_ga2_2_w(offs_t offset, u8 data)
{
	// TODO
}

u8 korg_dss1_state::klm782_ga3_1_r(offs_t offset)
{
	// TODO
	return 0;
}

void korg_dss1_state::klm782_ga3_1_w(offs_t offset, u8 data)
{
	// TODO
}

u8 korg_dss1_state::klm782_ga3_2_r(offs_t offset)
{
	// TODO
	return 0;
}

void korg_dss1_state::klm782_ga3_2_w(offs_t offset, u8 data)
{
	// TODO
}

void korg_dss1_state::dmaram_lsb_w(u8 data)
{
	// TODO
}

void korg_dss1_state::dmaram_msb_w(u8 data)
{
	// TODO
}

u8 korg_dss1_state::dmaram_lsb_r()
{
	// TODO
	return 0;
}

u8 korg_dss1_state::dmaram_msb_r()
{
	// TODO
	return 0;
}

void korg_dss1_state::mode_select_w(u8 data)
{
	// TODO
}

void korg_dss1_state::cpu1_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("rombank");
	map(0x8000, 0x9fff).rom().region("klm780", 0x10000);
	map(0xa000, 0xbfff).ram();
	map(0xc000, 0xc0ff).mirror(0x200).rw(m_io1, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0xc100, 0xc107).mirror(0x2f8).rw(m_io1, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0xc400, 0xc403).mirror(0x3fc).rw(m_io2, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xc800, 0xc800).mirror(0x3ff).w(FUNC(korg_dss1_state::bank_switch_w));
	map(0xd000, 0xd003).mirror(0x300).rw(m_pit[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xd004, 0xd007).mirror(0x300).rw(m_pit[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xd008, 0xd00b).mirror(0x300).rw(m_pit[2], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xd00c, 0xd00f).mirror(0x300).rw(m_pit[3], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xd010, 0xd013).mirror(0x300).rw(m_pit[4], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xd014, 0xd017).mirror(0x300).rw(m_pit[5], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xd018, 0xd018).mirror(0x303).w(FUNC(korg_dssmsrk_state::dmaram_lsb_w));
	map(0xd01c, 0xd01c).mirror(0x303).w(FUNC(korg_dssmsrk_state::dmaram_msb_w));
	map(0xd020, 0xd027).mirror(0x318).rw(FUNC(korg_dssmsrk_state::klm782_ga3_1_r), FUNC(korg_dssmsrk_state::klm782_ga3_1_w));
	map(0xd040, 0xd047).mirror(0x318).rw(FUNC(korg_dssmsrk_state::klm782_ga3_2_r), FUNC(korg_dssmsrk_state::klm782_ga3_2_w));
	map(0xd060, 0xd061).mirror(0x31e).rw(FUNC(korg_dssmsrk_state::klm782_ga1_r), FUNC(korg_dssmsrk_state::klm782_ga1_w));
	map(0xd080, 0xd08f).mirror(0x310).rw(FUNC(korg_dssmsrk_state::klm782_ga2_1_r), FUNC(korg_dssmsrk_state::klm782_ga2_1_w));
	map(0xd0a0, 0xd0af).mirror(0x310).rw(FUNC(korg_dssmsrk_state::klm782_ga2_2_r), FUNC(korg_dssmsrk_state::klm782_ga2_2_w));
	map(0xd0c0, 0xd0c0).mirror(0x31f).r(FUNC(korg_dssmsrk_state::dmaram_lsb_r));
	map(0xd0e0, 0xd0e0).mirror(0x31f).r(FUNC(korg_dssmsrk_state::dmaram_msb_r));
	map(0xd400, 0xd400).mirror(0x3fe).w(m_latch[0], FUNC(generic_latch_8_device::write));
	map(0xd800, 0xd800).mirror(0x3fe).r(m_latch[1], FUNC(generic_latch_8_device::read));
	map(0xdc00, 0xdc01).mirror(0x3fe).rw(FUNC(korg_dss1_state::fdc_r), FUNC(korg_dss1_state::fdc_w));
	map(0xe000, 0xffff).ram();
}

void korg_dssmsrk_state::msrk_map(address_map &map)
{
	map.global_mask(0x3ffff);
	map(0x00000, 0x07fff).rom().region("klm780", 0);
	map(0x08000, 0x0bfff).ram();
	map(0x0c000, 0x0c0ff).mirror(0x200).rw(m_io1, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x0c100, 0x0c107).mirror(0x2f8).rw(m_io1, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
	map(0x0c400, 0x0c403).mirror(0x3fc).rw(m_io2, FUNC(i8255_device::read), FUNC(i8255_device::write));
	//map(0x0cc00, 0x0cc1f).mirror(0x3e0).w(FUNC(korg_dssmsrk_state::msrk_ga_w));
	map(0x0d000, 0x0d003).mirror(0x300).rw(m_pit[0], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0d004, 0x0d007).mirror(0x300).rw(m_pit[1], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0d008, 0x0d00b).mirror(0x300).rw(m_pit[2], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0d00c, 0x0d00f).mirror(0x300).rw(m_pit[3], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0d010, 0x0d013).mirror(0x300).rw(m_pit[4], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0d014, 0x0d017).mirror(0x300).rw(m_pit[5], FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x0d018, 0x0d018).mirror(0x303).w(FUNC(korg_dssmsrk_state::dmaram_lsb_w));
	map(0x0d01c, 0x0d01c).mirror(0x303).w(FUNC(korg_dssmsrk_state::dmaram_msb_w));
	map(0x0d020, 0x0d027).mirror(0x318).rw(FUNC(korg_dssmsrk_state::klm782_ga3_1_r), FUNC(korg_dssmsrk_state::klm782_ga3_1_w));
	map(0x0d040, 0x0d047).mirror(0x318).rw(FUNC(korg_dssmsrk_state::klm782_ga3_2_r), FUNC(korg_dssmsrk_state::klm782_ga3_2_w));
	map(0x0d060, 0x0d061).mirror(0x31e).rw(FUNC(korg_dssmsrk_state::klm782_ga1_r), FUNC(korg_dssmsrk_state::klm782_ga1_w));
	map(0x0d080, 0x0d08f).mirror(0x310).rw(FUNC(korg_dssmsrk_state::klm782_ga2_1_r), FUNC(korg_dssmsrk_state::klm782_ga2_1_w));
	map(0x0d0a0, 0x0d0af).mirror(0x310).rw(FUNC(korg_dssmsrk_state::klm782_ga2_2_r), FUNC(korg_dssmsrk_state::klm782_ga2_2_w));
	map(0x0d0c0, 0x0d0c0).mirror(0x31f).r(FUNC(korg_dssmsrk_state::dmaram_lsb_r));
	map(0x0d0e0, 0x0d0e0).mirror(0x31f).r(FUNC(korg_dssmsrk_state::dmaram_msb_r));
	map(0x0d400, 0x0d400).mirror(0x3fe).w(m_latch[0], FUNC(generic_latch_8_device::write));
	map(0x0d800, 0x0d800).mirror(0x3fe).r(m_latch[1], FUNC(generic_latch_8_device::read));
	map(0x0e000, 0x0ffff).ram();
	map(0x10000, 0x17fff).mirror(0x8000).rom().region("klm780", 0x8000);
	map(0x30000, 0x3ffff).rom().region("msrk", 0);
}

void korg_dssmsrk_state::msrk_io_map(address_map &map)
{
	map(0xc800, 0xc807).mirror(0x3f8).rw(m_scsic, FUNC(ncr53c80_device::read), FUNC(ncr53c80_device::write));
	map(0xdc00, 0xdc01).mirror(0x3fe).rw(FUNC(korg_dssmsrk_state::fdc_r), FUNC(korg_dssmsrk_state::fdc_w));
}

void korg_dss1_state::cpu2_map(address_map &map)
{
	map(0x0100, 0x0100).mirror(0x3cff).w(FUNC(korg_dss1_state::da_lsb_w));
	map(0x0200, 0x0200).mirror(0x3cff).w(FUNC(korg_dss1_state::da_msb_w));
	map(0x0300, 0x0300).mirror(0x3cff).r(m_latch[0], FUNC(generic_latch_8_device::read));
	map(0x0300, 0x0300).mirror(0x3cff).w(m_latch[1], FUNC(generic_latch_8_device::write));
	map(0x4000, 0x4000).mirror(0x3fff).rw(FUNC(korg_dss1_state::kbd_r), FUNC(korg_dss1_state::kbd_w));
	map(0x8000, 0x9fff).mirror(0x2000).ram();
	map(0xc000, 0xffff).rom().region("klm781", 0);
}


static INPUT_PORTS_START(dss1)
INPUT_PORTS_END

static void dss1_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void korg_dss1_state::palette_init_dss1(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

void korg_dss1_state::klm780(machine_config &config)
{
	I8085A(config, m_cpu1, 10_MHz_XTAL); // uPD8085AC-2
	m_cpu1->set_addrmap(AS_PROGRAM, &korg_dss1_state::cpu1_map);

	I8155(config, m_io1, 10_MHz_XTAL / 2); // uPD8155HC-2
	m_io1->out_to_callback().set_inputline(m_cpu1, I8085_RST75_LINE);
	m_io1->in_pa_callback().set(m_lcdc, FUNC(hd44780_device::db_r));
	m_io1->out_pa_callback().set(m_lcdc, FUNC(hd44780_device::db_w));
	m_io1->out_pb_callback().set(FUNC(korg_dss1_state::panel_led_w));
	m_io1->out_pc_callback().set(m_lcdc, FUNC(hd44780_device::e_w)).bit(0);
	m_io1->out_pc_callback().append(m_lcdc, FUNC(hd44780_device::rw_w)).bit(1);
	m_io1->out_pc_callback().append(m_lcdc, FUNC(hd44780_device::rs_w)).bit(2);
	m_io1->out_pc_callback().append(m_fdc, FUNC(upd765a_device::reset_w)).bit(3).invert();
	m_io1->out_pc_callback().append(FUNC(korg_dss1_state::fdc_tc_w)).bit(4);
	m_io1->out_pc_callback().append(FUNC(korg_dss1_state::sed9420c_trgin_w)).bit(5);

	I8255(config, m_io2); // uPD8255AC-2
	m_io2->out_pc_callback().set(FUNC(korg_dss1_state::mode_select_w));

	UPD765A(config, m_fdc, 16_MHz_XTAL / 4, true, true); // uPD765AC; clocked through SED9420C
	m_fdc->intrq_wr_callback().set_inputline(m_cpu1, I8085_INTR_LINE);
	FLOPPY_CONNECTOR(config, "fdc:0", dss1_floppies, "35dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", dss1_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats).enable_sound(true);

	GENERIC_LATCH_8(config, m_latch[0]);
	m_latch[0]->data_pending_callback().set_inputline(m_cpu1, I8085_RST65_LINE).invert();

	GENERIC_LATCH_8(config, m_latch[1]);
	m_latch[1]->data_pending_callback().set_inputline(m_cpu1, I8085_RST55_LINE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*20, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(korg_dss1_state::palette_init_dss1), 2);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(2, 20);
	m_lcdc->set_pixel_update_cb(FUNC(korg_dss1_state::lcd_pixel_update));
}

void korg_dss1_state::klm781(machine_config &config)
{
	HD6303X(config, m_cpu2, 8_MHz_XTAL); // HD63B03X
	m_cpu2->set_addrmap(AS_PROGRAM, &korg_dss1_state::cpu2_map);
	m_cpu2->out_p2_cb().set(FUNC(korg_dss1_state::vcf_vca_ef_w));
	m_cpu2->in_p5_cb().set(FUNC(korg_dss1_state::cpu2_p5_r));
	m_cpu2->out_p6_cb().set(FUNC(korg_dss1_state::ad_select_w));
	m_cpu2->out_ser_tx_cb().set("midi_out", FUNC(midi_port_device::write_txd));

	MIDI_PORT(config, "midi_in", midiin_slot, "midiin");
	MIDI_PORT(config, "midi_out", midiout_slot, "midiout");
}

void korg_dss1_state::klm782(machine_config &config)
{
	PIT8253(config, m_pit[0]);
	PIT8253(config, m_pit[1]);
	PIT8253(config, m_pit[2]);
	PIT8253(config, m_pit[3]);
	PIT8253(config, m_pit[4]);
	PIT8253(config, m_pit[5]);
}

void korg_dss1_state::dss1(machine_config &config)
{
	klm780(config);
	klm781(config);
	klm782(config);
}

void korg_dssmsrk_state::dssmsrk(machine_config &config)
{
	V40(config, m_msrkcpu, 16_MHz_XTAL); // uPD70208
	m_msrkcpu->set_addrmap(AS_PROGRAM, &korg_dssmsrk_state::msrk_map);
	m_msrkcpu->set_addrmap(AS_IO, &korg_dssmsrk_state::msrk_io_map);
	m_msrkcpu->out_hreq_cb().set_inputline(m_msrkcpu, INPUT_LINE_HALT);
	m_msrkcpu->out_hreq_cb().append(m_msrkcpu, FUNC(v40_device::hack_w));
	m_msrkcpu->in_ior_cb<0>().set(m_fdc, FUNC(upd765a_device::dma_r));
	m_msrkcpu->out_iow_cb<0>().set(m_fdc, FUNC(upd765a_device::dma_w));
	m_msrkcpu->in_ior_cb<1>().set(m_scsic, FUNC(ncr53c80_device::dma_r));
	m_msrkcpu->out_iow_cb<1>().set(m_scsic, FUNC(ncr53c80_device::dma_w));
	m_msrkcpu->out_eop_cb().set(m_fdc, FUNC(upd765a_device::tc_line_w));
	//m_msrkcpu->out_eop_cb().append(m_scsic, FUNC(ncr53c80_device::eop_w));
	m_msrkcpu->tout1_cb().set(m_msrkcpu, FUNC(v40_device::tclk_w));

	klm780(config);
	config.device_remove("cpu1");
	m_io1->set_clock(16_MHz_XTAL / 4); // CLKOUT divider not verified
	m_io1->out_to_callback().set_inputline(m_msrkcpu, INPUT_LINE_IRQ1);
	m_fdc->intrq_wr_callback().set_inputline(m_msrkcpu, INPUT_LINE_IRQ6);
	m_fdc->drq_wr_callback().set(m_msrkcpu, FUNC(v40_device::dreq_w<0>)); // FIXME: delayed by 74HCT164
	m_latch[0]->data_pending_callback().set_inputline(m_msrkcpu, INPUT_LINE_IRQ2).invert();
	m_latch[1]->data_pending_callback().set_inputline(m_msrkcpu, INPUT_LINE_IRQ3);

	klm781(config);
	klm782(config);

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("scsic", NCR53C80).machine_config([this] (device_t *device) {
		auto &scsic = downcast<ncr53c80_device &>(*device); // 48-pin DIP
		scsic.irq_handler().set_inputline(m_msrkcpu, INPUT_LINE_IRQ4);
		scsic.drq_handler().set(m_msrkcpu, FUNC(v40_device::dreq_w<1>));
	});
}


ROM_START(dss1)
	ROM_REGION(0x12000, "klm780", 0)
	ROM_SYSTEM_BIOS(0, "v36", "Version 36")
	ROM_SYSTEM_BIOS(1, "v34", "Version 34")
	ROM_SYSTEM_BIOS(2, "v31", "Version 31")
	ROMX_LOAD("860336.ic19", 0x00000, 0x8000, CRC(44515595) SHA1(f4873c219cda4158506acbfb06abb8a72224049d), ROM_BIOS(0)) // 27256 (bank 0)
	ROMX_LOAD("860334.ic19", 0x00000, 0x8000, CRC(076c5956) SHA1(752a26761c63e46f1a6efa1b19a60bafa7a5bc42), ROM_BIOS(1))
	ROMX_LOAD("860331.ic19", 0x00000, 0x8000, CRC(fcefcc79) SHA1(2332ffcbbe61d460d1929e65cbaa01e0766ba51f), ROM_BIOS(2))
	ROMX_LOAD("860436.ic18", 0x08000, 0x8000, CRC(c8830cf9) SHA1(2bcdcfd9afc9e6b2078afdf9c027622b705780a9), ROM_BIOS(0)) // 27256 (bank 1)
	ROMX_LOAD("860434.ic18", 0x08000, 0x8000, CRC(5b253cac) SHA1(20507004ead025b03b2f7a31af87c9dae968e8eb), ROM_BIOS(1))
	ROMX_LOAD("860431.ic18", 0x08000, 0x8000, CRC(ed0e4238) SHA1(b7e56a3f414dd13bb6b9ef40262c6b93e52f5254), ROM_BIOS(2))
	ROMX_LOAD("860536.ic12", 0x10000, 0x2000, CRC(547388a1) SHA1(4c3dfaebe48f441955f2a6bd42dd4d30eb63b913), ROM_BIOS(0)) // 2764 (not banked)
	ROMX_LOAD("860534.ic12", 0x10000, 0x2000, CRC(9b5792b7) SHA1(e031d7f3a5c4f67399a75050bd02343ed6f8926d), ROM_BIOS(1))
	ROMX_LOAD("860531.ic12", 0x10000, 0x2000, CRC(a8173858) SHA1(e1d898594ad9e65a95478cf1ec14b9b3cafd0159), ROM_BIOS(2))

	ROM_REGION(0x4000, "klm781", 0)
	ROMX_LOAD("860236.ic15", 0x0000, 0x4000, CRC(b4ea379a) SHA1(66b3586b6fb7fa5edf70a933e49f626452ffe006), ROM_BIOS(0))
	ROMX_LOAD("860234.ic15", 0x0000, 0x4000, CRC(5766cdb5) SHA1(ded6c5758fbf90fd65b80d5b08e72349d38555d2), ROM_BIOS(1))
	ROMX_LOAD("860231.ic15", 0x0000, 0x4000, CRC(2559b3aa) SHA1(8b7998ffc24405d0ca08a93c23c18f97aff74d16), ROM_BIOS(2))
ROM_END

ROM_START(dssmsrk)
	ROM_REGION(0x10000, "klm780", 0)
	ROM_LOAD("113000.ic18", 0x00000, 0x8000, CRC(6fa13d52) SHA1(8de0e4e8ac4afe0d0fb116b6c8b64739ba81a722))
	ROM_LOAD("113001.ic19", 0x08000, 0x8000, CRC(ec7e2473) SHA1(dacea4a545ce3ed1fc11d9025e86dfd1d32a222c))
	// EPROM at IC12 is replaced with D4364C-15 SRAM

	ROM_REGION(0x4000, "klm781", 0)
	ROM_LOAD("860236.ic15", 0x0000, 0x4000, CRC(b4ea379a) SHA1(66b3586b6fb7fa5edf70a933e49f626452ffe006))

	ROM_REGION(0x10000, "msrk", 0)
	ROM_LOAD("113003.u30", 0x00000, 0x10000, CRC(81b17db3) SHA1(af8a3167e06641d41b9b9e6e024335c2eb827274))
ROM_END

} // anonymous namespace


SYST(1986, dss1,    0,    0, dss1,    dss1, korg_dss1_state,    empty_init, "Korg",               "DSS-1 Digital Sampling Synthesizer",                        MACHINE_IS_SKELETON)
SYST(1987, dssmsrk, dss1, 0, dssmsrk, dss1, korg_dssmsrk_state, empty_init, "Korg / Sound Logic", "DSS-1 Digital Sampling Synthesizer (Memory/SCSI Retrofit)", MACHINE_IS_SKELETON)

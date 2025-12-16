// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    s3000.cpp - Akai S2000, S2800, S3000, CD3000i, S3200, S3000XL,
                S3200XL, and CD3000XL rackmount 16-bit samplers.
    Driver by R. Belmont

    These rackmount samplers are all built on variants of what became the
    MPC3000 hardware.  The major difference is the lack of the 7810/7811
    sub-CPU to manage the panel, since without drum pads there's no need
    for ultra-low-latency response from the panel.

    The S2000 is the lowest end model, with a slightly slower CPU, no SCSI, a smaller
    LCD, and fewer controls.  The S2800 has the same LCD and front panel as the S3000
    but again is bare-bones on features.

    "XL" models have more ROM and RAM space for a more advanced OS.  Note that the
    S2000 is based on the XL hardware, not the base S3000/CD3000i hardware.

    Hardware:
        CPU: NEC V53 (32 MHz on 3x00, 31.94 MHz on 2000)
        Floppy: uPD72069
        SCSI: MB89352
        LCD: LC7981 (3x00 and 2800) or HD44780 (2000)
        Sound DSP: L7A1045-L6028
        Filter (XL models and S3200 only): L7A0906-L6029 DFL
        Effects DSP (XL models only): L7A1414-L6038 DFX

    Working:
        - Keyboard matrix on all models
        - LED outputs should be correct for all models
        - MIDI In on all models
        - SCSI CD-ROM and hard disk on S2000, CD3000i, and CD3000XL
        - Loading and playing sounds from CD-ROM and hard disk on S2000
          and CD3000XL
        - S2000 can save its OS to a hard disk and boot from it instead
          of floppy
        - Floppy can be formatted successfully now, and S2000 boots the OS from
          floppy correctly

    TODOs:
        - Non-XL models appear not to read DiskChg via the uPD TS line like the
          XLs do.  Need to find where it does that.
        - S3000 and CD3000i don't find any sample RAM.  The XL models and S2000
          both use the MPC3000 approach of actually testing the memory and find
          all 32 megs fine.  This appears to be related to the other bits in the
          ID register.
        - Many Akai factory CD sounds have clipping, need to check if that's
          an L6028 volume issue or bad loop points (or both).
        - Split the data entry knob into a device so we can share it across the
          various Akai drivers.

***************************************************************************/

#include "emu.h"

#include "bus/midi/midi.h"
#include "bus/nscsi/devices.h"
#include "cpu/nec/v5x.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/mb87030.h"
#include "machine/output_latch.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "sound/l7a1045_l6028_dsp_a.h"
#include "video/hd44780.h"
#include "video/hd61830.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/dfi_dsk.h"
#include "formats/dsk_dsk.h"
#include "formats/hxchfe_dsk.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/imd_dsk.h"
#include "formats/ipf_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/td0_dsk.h"
#include "formats/pc_dsk.h"

#include "s2000.lh"

namespace {

static constexpr uint8_t BIT6 = 0x40;
static constexpr uint8_t BIT7 = 0x80;

class s3000_state : public driver_device
{
public:
	s3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_lcdc(*this, "lcdc")
		, m_s2klcd(*this, "s2klcd")
		, m_dsp(*this, "dsp")
		, m_mdout(*this, "mdout")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_floppy_led(*this, "drive_led")
		, m_klcs(*this, "klcs")
		, m_ledlatch(*this, "ledlatch")
		, m_keys(*this, "C%u", 0)
		, m_dataentry(*this, "DATAENTRY")
		, m_key_scan_row(0)
		, m_last_dial(0)
		, m_count_dial(0)
		, m_quadrature_phase(0)
	{ }

	void s2000(machine_config &config);
	void s3000(machine_config &config);
	void s3000xl(machine_config &config);
	void cd3000(machine_config &config);
	void cd3000xl(machine_config &config);

private:
	required_device<v53a_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<hd61830_device> m_lcdc;
	optional_device<hd44780_device> m_s2klcd;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<midi_port_device> m_mdout;
	required_device<upd72069_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	output_finder<> m_floppy_led;
	required_device<i8255_device> m_klcs;
	required_device<output_latch_device> m_ledlatch;
	required_ioport_array<8> m_keys;
	required_ioport m_dataentry;

	static void floppies(device_slot_interface &device);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void s2000_map(address_map &map) ATTR_COLD;
	void s3000_map(address_map &map) ATTR_COLD;
	void s3000xl_map(address_map &map) ATTR_COLD;
	void s2000_io_map(address_map &map) ATTR_COLD;
	void s3000_io_map(address_map &map) ATTR_COLD;
	void s3000xl_io_map(address_map &map) ATTR_COLD;
	void cd3000_io_map(address_map &map) ATTR_COLD;
	void dsp_map(address_map &map) ATTR_COLD;

	void floppy_led_cb(floppy_image_device *, int state);

	uint8_t dma_memr_cb(offs_t offset);
	void dma_memw_cb(offs_t offset, uint8_t data);
	uint16_t dma_mem16r_cb(offs_t offset);
	void dma_mem16w_cb(offs_t offset, uint16_t data);
	void s3000_palette(palette_device &palette) const;

	uint8_t id_r();
	uint8_t cd_id_r();

	uint8_t klcs_porta_r();
	void klcs_portb_w(uint8_t data);

	uint8_t ts_r();

	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	TIMER_DEVICE_CALLBACK_MEMBER(dial_timer_tick);

	uint8_t m_key_scan_row;
	int m_last_dial, m_count_dial, m_quadrature_phase;
};

void s3000_state::machine_start()
{
	save_item(NAME(m_key_scan_row));
	save_item(NAME(m_last_dial));
	save_item(NAME(m_count_dial));
	save_item(NAME(m_quadrature_phase));

	m_floppy_led.resolve();
	m_floppy->get_device()->setup_led_cb(floppy_image_device::led_cb(&s3000_state::floppy_led_cb, this));
}

void s3000_state::machine_reset()
{
	m_maincpu->cts_w(CLEAR_LINE);   // grounded on schematic
	m_fdc->ready_w(CLEAR_LINE);     // also grounded on schematic
}

void s3000_state::s2000_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram(); // 2x HM514260 256kx16 = 512K
	map(0x0f8000, 0x0fffff).rom().region("maincpu", 0);
	map(0x180000, 0x1fffff).ram();
}

void s3000_state::s3000_map(address_map &map)
{
	map(0x000000, 0x0bffff).ram();  // 2x HM514260 256kx16 = 512K
	map(0x0c0000, 0x0fffff).rom().region("maincpu", 0);
}

void s3000_state::s3000xl_map(address_map &map)
{
	map(0x000000, 0x07ffff).ram();
	map(0x080000, 0x0fffff).rom().region("maincpu", 0);
	map(0x180000, 0x1fffff).ram();
}

// 0 = SPCS (SCSI)
// 2 = FDCS (FDC)
// 4 = KLCS (keyboard/LEDs/controls)
// 6 = LCDCS (LCD controller)
// 8 = NSCS (DSP)
// a = FXCS (effects DSP - XL models only)
// c = WADCS (A/D converter - XL models only)
// e = FILCS (filter control - XL models only)
void s3000_state::s2000_io_map(address_map &map)
{
	map(0x0000, 0x001f).m("scsi:7:spc", FUNC(mb89352_device::map)).umask16(0x00ff);
	map(0x0020, 0x0023).m(m_fdc, FUNC(upd72069_device::map)).umask16(0x00ff);
	map(0x0040, 0x0047).rw(m_klcs, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0050, 0x0057).rw(m_klcs, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0060, 0x0060).rw(m_s2klcd, FUNC(hd44780_device::data_r), FUNC(hd44780_device::data_w)).umask16(0x00ff);
	map(0x0062, 0x0062).rw(m_s2klcd, FUNC(hd44780_device::control_r), FUNC(hd44780_device::control_w)).umask16(0x00ff);
	map(0x0080, 0x008f).m(m_dsp, FUNC(l7a1045_sound_device::map));
	map(0x00a0, 0x00a5).nopw(); // quiet writes to the effects DSP
}

void s3000_state::s3000_io_map(address_map &map)
{
	map(0x0000, 0x001f).m("scsi:7:spc", FUNC(mb89352_device::map)).umask16(0x00ff);
	map(0x0020, 0x0023).m(m_fdc, FUNC(upd72069_device::map)).umask16(0x00ff);
	map(0x0048, 0x0048).rw(m_lcdc, FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w)).umask16(0x00ff);
	map(0x004a, 0x004a).rw(m_lcdc, FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w)).umask16(0x00ff);
	map(0x0050, 0x0057).rw(m_klcs, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0060, 0x0061).r(FUNC(s3000_state::id_r)).umask16(0x00ff);
	map(0x0080, 0x008f).m(m_dsp, FUNC(l7a1045_sound_device::map));
	map(0x00a0, 0x00a5).nopw(); // quiet writes to the effects DSP
}

void s3000_state::cd3000_io_map(address_map &map)
{
	s3000_io_map(map);
	map(0x0060, 0x0061).r(FUNC(s3000_state::cd_id_r)).umask16(0x00ff);
}

void s3000_state::s3000xl_io_map(address_map &map)
{
	map(0x0000, 0x001f).m("scsi:7:spc", FUNC(mb89352_device::map)).umask16(0x00ff);
	map(0x0020, 0x0023).m(m_fdc, FUNC(upd72069_device::map)).umask16(0x00ff);
	map(0x0040, 0x0047).rw(m_klcs, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0050, 0x0057).rw(m_klcs, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x0060, 0x0060).rw(m_lcdc, FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w)).umask16(0x00ff);
	map(0x0062, 0x0062).rw(m_lcdc, FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w)).umask16(0x00ff);
	map(0x0080, 0x008f).m(m_dsp, FUNC(l7a1045_sound_device::map));
	map(0x00a0, 0x00a5).nopw(); // quiet writes to the effects DSP
}

// stock wave memory is 4x HM514400 1Mx4 = 2 MiB
void s3000_state::dsp_map(address_map &map)
{
	map(0x0000'0000, 0x01ff'ffff).ram();
}

void s3000_state::floppy_led_cb(floppy_image_device *, int state)
{
	m_floppy_led = state;
}

// 00 = CD3000i (invalid with S3000/3200 ROM)
// 40 = S3200?
// 80 = S3000?
// C0 = S3000XL?
// other bits involved in SIMM detection
uint8_t s3000_state::id_r()
{
	return 0x80;
}

uint8_t s3000_state::cd_id_r()
{
	return 0x00;
}

uint8_t s3000_state::klcs_porta_r()
{
	uint8_t rv = m_keys[m_key_scan_row]->read() & ~0xc0;

	if (m_count_dial)
	{
		const bool negative = (m_count_dial < 0);

		switch (m_quadrature_phase >> 1)
		{
			case 0:
				rv |= negative ? BIT7 : BIT6;
				break;

			case 1:
				rv |= BIT6 | BIT7;
				break;

			case 2:
				rv |= negative ? BIT6 : BIT7;
				break;

			case 3:
				break;
		}
		m_quadrature_phase++;
		m_quadrature_phase &= 7;

		// generate a complete 4-part pulse train for each single change in the position
		if (m_quadrature_phase == 0)
		{
			if (m_count_dial < 0)
			{
				m_count_dial++;
			}
			else
			{
				m_count_dial--;
			}
		}
	}

	return rv;
}

void s3000_state::klcs_portb_w(uint8_t data)
{
	m_key_scan_row = data ^ 0xff;
	if (m_key_scan_row != 0)
	{
		m_key_scan_row = count_leading_zeros_32(m_key_scan_row) - 24;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(s3000_state::dial_timer_tick)
{
	const int new_dial = m_dataentry->read();

	if (new_dial != m_last_dial)
	{
		int diff = new_dial - m_last_dial;
		if (diff > 0x80)
		{
			diff = 0x100 - diff;
		}
		if (diff < -0x80)
		{
			diff = -0x100 - diff;
		}

		m_count_dial += diff;
		m_last_dial = new_dial;
	}
}

uint8_t s3000_state::ts_r()
{
	const auto imagedev = m_floppy->get_device();
	return imagedev->dskchg_r();
}

HD44780_PIXEL_UPDATE(s3000_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
	{
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
	}
}

uint16_t s3000_state::dma_mem16r_cb(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_word(offset << 1);
}

void s3000_state::dma_mem16w_cb(offs_t offset, uint16_t data)
{
	m_maincpu->space(AS_PROGRAM).write_word(offset << 1, data);
}

uint8_t s3000_state::dma_memr_cb(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void s3000_state::dma_memw_cb(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

void s3000_state::s3000_palette(palette_device &palette) const
{
//  palette.set_pen_color(0, rgb_t(138, 146, 148));
//  palette.set_pen_color(1, rgb_t(92, 83, 88));
	palette.set_pen_color(0, rgb_t(88, 247, 0)); // bright green
	palette.set_pen_color(1, rgb_t(3, 179, 6));  // dark green
}

void s3000_state::floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

static void add_formats(format_registration &fr)
{
	fr.add(FLOPPY_DFI_FORMAT);
	fr.add(FLOPPY_MFM_FORMAT);
	fr.add(FLOPPY_TD0_FORMAT);
	fr.add(FLOPPY_IMD_FORMAT);
	fr.add(FLOPPY_DSK_FORMAT);
	fr.add(FLOPPY_PC_FORMAT);
	fr.add(FLOPPY_IPF_FORMAT);
	fr.add(FLOPPY_HFE_FORMAT);
}

void s3000_state::s3000(machine_config &config)
{
	V53A(config, m_maincpu, 32_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &s3000_state::s3000_map);
	m_maincpu->set_addrmap(AS_IO, &s3000_state::s3000_io_map);
	m_maincpu->out_hreq_cb().set(m_maincpu, FUNC(v53a_device::hack_w));
	m_maincpu->in_memr_cb().set(FUNC(s3000_state::dma_memr_cb));
	m_maincpu->out_memw_cb().set(FUNC(s3000_state::dma_memw_cb));
	m_maincpu->in_mem16r_cb().set(FUNC(s3000_state::dma_mem16r_cb));
	m_maincpu->out_mem16w_cb().set(FUNC(s3000_state::dma_mem16w_cb));
	m_maincpu->out_eop_cb().set("tc", FUNC(input_merger_device::in_w<0>)).invert();
	m_maincpu->out_dack_cb<1>().set("tc", FUNC(input_merger_device::in_w<1>));
	m_maincpu->sint_handler_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_maincpu->txrdy_handler_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ4);

	m_maincpu->in_ior_cb<0>().set("scsi:7:spc", FUNC(mb89352_device::dma_r));
	m_maincpu->out_iow_cb<0>().set("scsi:7:spc", FUNC(mb89352_device::dma_w));
	m_maincpu->in_ior_cb<1>().set(m_fdc, FUNC(upd72069_device::dma_r));
	m_maincpu->out_iow_cb<1>().set(m_fdc, FUNC(upd72069_device::dma_w));
	m_maincpu->in_io16r_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_r16_cb));
	m_maincpu->out_io16w_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_w16_cb));

	m_maincpu->set_tclk(4'000'000);
	m_maincpu->v53_tout_handler<0>().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_maincpu->v53_tout_handler<1>().set_inputline(m_maincpu, INPUT_LINE_IRQ6);
	// timer 2 is the clock for the LCD controller

	constexpr XTAL V53_CLKOUT = 32_MHz_XTAL / 2;
//  constexpr XTAL V53_PCLKOUT = 32_MHz_XTAL / 4;

	INPUT_MERGER_ALL_HIGH(config, "tc").output_handler().set(m_fdc, FUNC(upd72069_device::tc_line_w));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(80);
	m_screen->set_screen_update("lcdc", FUNC(hd61830_device::screen_update));
	m_screen->set_size(240, 64);   //6x20, 8x8
	m_screen->set_visarea(0, 240-1, 0, 64-1);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(s3000_state::s3000_palette), 2);

	UPD72069(config, m_fdc, V53_CLKOUT);
	m_fdc->set_ready_line_connected(false); // uPD READY pin is grounded on schematic
	m_fdc->set_ts_line_connected(false);    // actually connected to DSKCHG (!)
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ3);
	m_fdc->intrq_wr_callback().append(m_maincpu, FUNC(v53a_device::dsr_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v53a_device::dreq_w<1>));
	m_fdc->ts_rd_callback().set(FUNC(s3000_state::ts_r));

	FLOPPY_CONNECTOR(config, m_floppy, s3000_state::floppies, "35hd", add_formats).enable_sound(true);

	HD61830(config, m_lcdc, 4.9152_MHz_XTAL / 2 / 2); // LC7981

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_maincpu, FUNC(v53a_device::rxd_w));

	midiout_slot(MIDI_PORT(config, "mdout"));
	m_maincpu->txd_handler_cb().set(m_mdout, FUNC(midi_port_device::write_txd));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("spc", MB89352).machine_config(
		[this](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(32_MHz_XTAL / 4); // PCLKOUT
			spc.out_irq_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ2);
			spc.out_dreq_callback().set(m_maincpu, FUNC(v53a_device::dreq_w<0>));
		});

	//        S3000/CD3000i     S2000      S3000XL/CD3000XL
	// led0 - Select Program    Load       Single
	// led1 - Edit Sample       Effects    Multi
	// led2 - Edit Program      Save       Sample
	// led3 - MIDI              Sample     Effects
	// led4 - Disk              Global     Edit
	// led5 - Tune Level        Multi      Global
	// led6 - Utility           Edit       Save
	// led7 - Help              Single     Load
	OUTPUT_LATCH(config, m_ledlatch);
	m_ledlatch->bit_handler<0>().set_output("led0").invert();
	m_ledlatch->bit_handler<1>().set_output("led1").invert();
	m_ledlatch->bit_handler<2>().set_output("led2").invert();
	m_ledlatch->bit_handler<3>().set_output("led3").invert();
	m_ledlatch->bit_handler<4>().set_output("led4").invert();
	m_ledlatch->bit_handler<5>().set_output("led5").invert();
	m_ledlatch->bit_handler<6>().set_output("led6").invert();
	m_ledlatch->bit_handler<7>().set_output("led7").invert();

	I8255(config, m_klcs); // MB89255B
	m_klcs->in_pa_callback().set(FUNC(s3000_state::klcs_porta_r));
	m_klcs->out_pb_callback().set(FUNC(s3000_state::klcs_portb_w));
	m_klcs->out_pc_callback().set(m_ledlatch, FUNC(output_latch_device::write));

	SPEAKER(config, "speaker", 2).front();

	L7A1045(config, m_dsp, 33.8688_MHz_XTAL);
	m_dsp->set_addrmap(AS_DATA, &s3000_state::dsp_map);
	m_dsp->drq_handler_cb().set(m_maincpu, FUNC(v53a_device::dreq_w<3>));
	m_dsp->add_route(l7a1045_sound_device::L6028_LEFT, "speaker", 1.0, 0);
	m_dsp->add_route(l7a1045_sound_device::L6028_RIGHT, "speaker", 1.0, 1);

	TIMER(config, "dialtimer").configure_periodic(FUNC(s3000_state::dial_timer_tick), attotime::from_hz(60.0));
}

void s3000_state::s2000(machine_config &config)
{
	s3000(config);
	m_maincpu->set_clock(31.9488_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &s3000_state::s2000_map);
	m_maincpu->set_addrmap(AS_IO, &s3000_state::s2000_io_map);

	config.device_remove("lcdc");

	HD44780(config, m_s2klcd, 4.9152_MHz_XTAL / 4); // HD44780
	m_s2klcd->set_lcd_size(2, 16);
	m_s2klcd->set_pixel_update_cb(FUNC(s3000_state::lcd_pixel_update));

	m_screen->set_screen_update(m_s2klcd, FUNC(hd44780_device::screen_update));
	m_screen->set_size(6 * 16, 8 * 2);
	m_screen->set_visarea_full();

	NSCSI_CONNECTOR(config.replace(), "scsi:5", default_scsi_devices, "cdrom");

	config.set_default_layout(layout_s2000);
}

void s3000_state::cd3000(machine_config &config)
{
	s3000(config);
	m_maincpu->set_addrmap(AS_IO, &s3000_state::cd3000_io_map);

	NSCSI_CONNECTOR(config.replace(), "scsi:4", default_scsi_devices, "cdrom");
}

void s3000_state::s3000xl(machine_config &config)
{
	s3000(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &s3000_state::s3000xl_map);
	m_maincpu->set_addrmap(AS_IO, &s3000_state::s3000xl_io_map);

	m_klcs->in_pa_callback().set(FUNC(s3000_state::klcs_porta_r));
}

void s3000_state::cd3000xl(machine_config &config)
{
	s3000xl(config);

	NSCSI_CONNECTOR(config.replace(), "scsi:4", default_scsi_devices, "cdrom");
}

// KC = port A, KR = port B, KD = port C
//   0           1     2       3          4
// 0 HELP        F8    1/W     0/Z
// 1 UTILITIES   F7    2/X     +/<        RIGHT
// 2 TUNE/LEVEL  F6    MARK/#  JUMP/.     UP
// 3 DISK/E      F5    9/S     6/R
// 4 MIDI/D      F4    8/V     5/U
// 5 EDIT P/C    F3    7/Q     4/T
// 6 EDIT S/B    F2    NAME    ENT/PLAY   DOWN
// 7 SELECT P/A  F1    3/Y     -/>        LEFT
static INPUT_PORTS_START(s3000)
	PORT_START("C0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 / W") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 / Z") PORT_CODE(KEYCODE_0)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Utilities") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 / X") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+ / <") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tune / Level") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mark / #") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Jump / .") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Disk / E") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 / S") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 / R") PORT_CODE(KEYCODE_6)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI / D") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 / V") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 / U") PORT_CODE(KEYCODE_5)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit Prog / K") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 / Q") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 / T") PORT_CODE(KEYCODE_4)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit Sample / J") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2 / B") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Name") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter/Play") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Select Prog / I") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1 / I") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 / Y") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- / >") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DATAENTRY")
	PORT_BIT( 0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)
INPUT_PORTS_END

static INPUT_PORTS_START(s3000xl)
	PORT_START("C0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load / H") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F8 / P") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 / W") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 / Z") PORT_CODE(KEYCODE_0)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Save / G") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7 / O") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 / X") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+ / >") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Global / F") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6 / N") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mark / #") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Jump / .") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit / E") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5 / M") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 / S") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 / V") PORT_CODE(KEYCODE_6)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Effects / D") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4 / L") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 / R") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 / U") PORT_CODE(KEYCODE_5)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sample / C") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3 / K") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 / Q") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 / T") PORT_CODE(KEYCODE_4)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Multi / B") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2 / J") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Name") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter/Play") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Single / A") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1 / I") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 / Y") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- / <") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DATAENTRY")
	PORT_BIT( 0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)
INPUT_PORTS_END

static INPUT_PORTS_START(cd3000xl)
	PORT_START("C0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load / H") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F8 / P") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 / W") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0 / Z") PORT_CODE(KEYCODE_0)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Save / G") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F7 / O") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 / X") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("+ / >") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Global / F") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6 / N") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mark / #") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Jump / .") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit / E") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5 / M") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 / S") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 / V") PORT_CODE(KEYCODE_6)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Effects / D") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4 / L") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 / R") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 / U") PORT_CODE(KEYCODE_5)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sample / C") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3 / K") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 / Q") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 / T") PORT_CODE(KEYCODE_4)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Multi / B") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2 / J") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Name") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter/Play") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Single / A") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1 / I") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 / Y") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("- / <") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left Arrow")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DATAENTRY")
	PORT_BIT( 0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)
INPUT_PORTS_END

static INPUT_PORTS_START(s2000)
	PORT_START("C0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sample") PORT_CODE(KEYCODE_E)
	PORT_BIT(0xfd, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Play") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Save") PORT_CODE(KEYCODE_D)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Group Up") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Group Down") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Multi") PORT_CODE(KEYCODE_W)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Effects") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Global") PORT_CODE(KEYCODE_S)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Load") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Page Up") PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Single") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("C7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Page Down") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit") PORT_CODE(KEYCODE_A)
	PORT_BIT(0xfc, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("DATAENTRY")
	PORT_BIT( 0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CODE_DEC(KEYCODE_OPENBRACE) PORT_CODE_INC(KEYCODE_CLOSEBRACE)
INPUT_PORTS_END

ROM_START( s2000 )
	ROM_REGION(0x8000, "maincpu", 0) // V53 code
	ROM_SYSTEM_BIOS(0, "default", "ver 1.5")
	ROMX_LOAD( "akai s2000 1.5.bin", 0x000000, 0x008000, CRC(c65de1f8) SHA1(a0f823be95d10c019e997ecfa3eeaa9c88420263), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "v10", "ver 1.0")
	ROMX_LOAD( "akai s2000 version 1.0.bin", 0x000000, 0x008000, CRC(316bba63) SHA1(c794311ca0c249beda4b36833719d92cc7e45ac5), ROM_BIOS(1) )
ROM_END

ROM_START( s3000 )
	ROM_REGION(0x40000, "maincpu", 0) // V53 code
	ROM_SYSTEM_BIOS(0, "default", "ver 2.0")
	ROMX_LOAD("akai s3x00 v2.0 low.bin", 0x000000, 0x020000, CRC(12a657c9) SHA1(ba77af24b647a2edcd1ff67c5b16e3103efb8039), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("akai s3x00 v2.0 high.bin", 0x000001, 0x020000, CRC(1763a18d) SHA1(95042987ef6f6e63dae1b4771f4841d605af5695), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "ver15", "ver 1.50")
	ROMX_LOAD( "akai s3000 1.50 lsb.bin", 0x000000, 0x020000, CRC(071b614c) SHA1(f349ddebf8d7e0b2bb2a63302f668aa0283c2805), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "akai s3000 1.50 msb.bin", 0x000001, 0x020000, CRC(a951216b) SHA1(98e1887a07221cc556aa315db4cba1a176a323be), ROM_SKIP(1) | ROM_BIOS(1) )
ROM_END

ROM_START( cd3000i )
	ROM_REGION(0x40000, "maincpu", 0) // V53 code
	ROM_SYSTEM_BIOS(0, "default", "ver 2.00")
	ROMX_LOAD( "cd3000_v2_0_lo.bin", 0x000000, 0x020000, CRC(6e59a979) SHA1(153e8186aa2b56a73abc112b0a6f13bffceb7c6b), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD( "cd3000_v2_0_hi.bin", 0x000001, 0x020000, CRC(da737625) SHA1(eb1c73400f476086c489700d27f6f8ab8f514a0d), ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "ver16", "ver 1.60")
	ROMX_LOAD("akai_cd3000i_ls_v1.60.bin", 0x000000, 0x020000, CRC(378af711) SHA1(53fd853e2d5dcd757952ba9a68cdaedae65e0fbb), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("akai_cd3000i_ms_v1.60.bin", 0x000001, 0x020000, CRC(5a4f5cf4) SHA1(f3400f2938d803c76690bbf00c9e4d50099acb7e), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START( s3000xl )
	ROM_REGION(0x80000, "maincpu", 0) // V53 code

	ROM_SYSTEM_BIOS(0, "default", "ver 2.00")
	ROMX_LOAD( "s3000xl_v2_0.bin", 0x000000, 0x080000, CRC(26a00ddd) SHA1(ace009953aeccff4dfbee0ccfa091a0fea39a386), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "ver15", "ver 1.50")
	ROMX_LOAD( "akai s3000xl v1-50.bin", 0x000000, 0x080000, CRC(01a195ce) SHA1(24160094fa4acb6d6c4d097b7f63b309ed0945cc), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "ver106", "ver 1.06")
	ROMX_LOAD( "akai s3000xl v1-06.bin", 0x000000, 0x080000, CRC(f251faf8) SHA1(b8e3a26ac42fa4a5c7bd1a3a87a866934b0d7a6b), ROM_BIOS(2) )
ROM_END

ROM_START( cd3000xl )
	ROM_REGION(0x80000, "maincpu", 0) // V53 code
	ROM_LOAD( "akai - cd3000xl - osv2.00.bin", 0x000000, 0x080000, CRC(6d4cc4ff) SHA1(89ee642e8e18e0ce218bb4766b620690323c26cc) )
ROM_END

} // anonymous namespace

SYST( 1993, s2000,    0, 0, s2000,    s2000,    s3000_state, empty_init, "Akai", "S2000", MACHINE_NOT_WORKING )
SYST( 1993, s3000,    0, 0, s3000,    s3000,    s3000_state, empty_init, "Akai", "S3000", MACHINE_NOT_WORKING )
SYST( 1993, cd3000i,  0, 0, cd3000,   s3000,    s3000_state, empty_init, "Akai", "CD3000i", MACHINE_NOT_WORKING)
SYST( 1994, s3000xl,  0, 0, s3000xl,  s3000xl,  s3000_state, empty_init, "Akai", "S3000XL", MACHINE_NOT_WORKING)
SYST( 1994, cd3000xl, 0, 0, cd3000xl, cd3000xl, s3000_state, empty_init, "Akai", "CD3000XL", MACHINE_NOT_WORKING)

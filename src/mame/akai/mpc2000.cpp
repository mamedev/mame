// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    mpc2000.cpp - Akai MPC2000/MPC2000XL music workstation
    Driver by R. Belmont

    This is a cost-reduced MPC3000, with fewer MIDI ports, IDE alongside
    SCSI, and a few other omissions.  The actual sound generation is identical
    to the 3000 however.

    Hardware:
        CPU: NEC V53 (32 MHz)
        Floppy: uPD72068
        Bus master IDE
        SCSI: MB89352 (optional on 2000, standard on 2000XL)
        LCD: LC7981
        UART: MB89371A
        Panel controller CPU: NEC uPD78C10AGQ @ 12 MHz
        Sound DSP: L7A1045-L6028

    TODO:
        - FDC motor is never turned off, at least via auxcmd.  Is there some other uPD76x
          mechanism that might do this?  Floppy sounds are disabled for now because of this.
        - SCSI and ATA don't work (SCSI has the same issues as MPC3000, likely the same cause)
        - Boot ROM is an Intel E28F800B5 "boot block flash", and can be live updated.

***************************************************************************/

#include "emu.h"

#include "bus/ata/ataintf.h"
#include "bus/nscsi/devices.h"
#include "bus/midi/midi.h"
#include "cpu/nec/v5x.h"
#include "cpu/upd7810/upd7810.h"
#include "formats/dfi_dsk.h"
#include "formats/hxchfe_dsk.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/imd_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/td0_dsk.h"
#include "formats/dsk_dsk.h"
#include "formats/pc_dsk.h"
#include "formats/ipf_dsk.h"
#include "imagedev/floppy.h"
#include "machine/74259.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/mb87030.h"
#include "machine/mb89371.h"
#include "machine/pit8253.h"
#include "machine/upd765.h"
#include "sound/l7a1045_l6028_dsp_a.h"
#include "video/hd61830.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

static constexpr uint8_t BIT4 = (1 << 4);
static constexpr uint8_t BIT5 = (1 << 5);

class mpc2000_state : public driver_device
{
public:
	mpc2000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_dsp(*this, "dsp")
		, m_screen(*this, "screen")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_ata(*this, "ata")
		, m_sio(*this, "sio")
		, m_keys(*this, "Y%u", 0)
		, m_dataentry(*this, "DATAENTRY")
		, m_key_scan_row(0)
		, m_drum_scan_row(0)
		, m_variation_slider(0)
		, m_last_dial(0)
		, m_count_dial(0)
		, m_quadrature_phase(0)
		, m_fdc_ata(0)
		, m_fdc_irq(0)
		, m_fdc_drq(0)
		, m_ata_irq(0)
		, m_ata_drq(0)
		, m_wadcsn(0)
		, m_lcdx{ 0, 0 }
		, m_lcdy{ 0, 0 }
		, m_lcdcmd{ 0, 0 }
	{
		std::fill_n(&m_vram[0], std::size(m_vram), 0);
	}

	void mpc2000(machine_config &config);

	void init_mpc2000();

	DECLARE_INPUT_CHANGED_MEMBER(variation_changed);

private:
	required_device<v53a_device> m_maincpu;
	required_device<upd7810_device> m_subcpu;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<screen_device> m_screen;
	required_device<upd72069_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<ata_interface_device> m_ata;
	required_device<mb89371_device> m_sio;
	required_ioport_array<8> m_keys;
	required_ioport m_dataentry;

	static void floppies(device_slot_interface &device);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mpc2000_map(address_map &map) ATTR_COLD;
	void mpc2000_io_map(address_map &map) ATTR_COLD;
	void mpc2000_sub_map(address_map &map) ATTR_COLD;
	void dsp_map(address_map &map) ATTR_COLD;

	uint8_t dma_memr_cb(offs_t offset);
	void dma_memw_cb(offs_t offset, uint8_t data);
	uint16_t dma_mem16r_cb(offs_t offset);
	void dma_mem16w_cb(offs_t offset, uint16_t data);
	void mpc2000_palette(palette_device &palette) const;
	uint8_t lcd_csr();
	template <int cs> void lcd_csw(offs_t offset, uint8_t data);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t wadcsn_r();
	void wadcn_w(uint8_t data);

	uint8_t subcpu_pa_r();
	uint8_t subcpu_pb_r();
	uint8_t subcpu_pc_r();
	void subcpu_pb_w(uint8_t data);
	void subcpu_pc_w(uint8_t data);
	uint8_t an_pads_r();
	uint8_t an4_r();

	void fdc_scsi_w(int state);
	void fdc_irq_w(int state);
	void fdc_drq_w(int state);
	void ata_irq_w(int state);
	void ata_drq_w(int state);
	void recalc_fdc_ata();

	uint8_t ts_r();

	TIMER_DEVICE_CALLBACK_MEMBER(dial_timer_tick);

	uint8_t m_key_scan_row, m_drum_scan_row, m_variation_slider;
	int m_last_dial, m_count_dial, m_quadrature_phase;
	int m_fdc_ata, m_fdc_irq, m_fdc_drq, m_ata_irq, m_ata_drq;

	uint8_t m_wadcsn;
	uint16_t m_lcdx[2];
	uint8_t m_lcdy[2], m_lcdcmd[2];
	uint16_t m_vram[256*64];
};

void mpc2000_state::machine_start()
{
	save_item(NAME(m_key_scan_row));
	save_item(NAME(m_drum_scan_row));
}

void mpc2000_state::machine_reset()
{
	// all CTS lines are connected to GND
	m_sio->write_cts<0>(CLEAR_LINE);
	m_sio->write_cts<1>(CLEAR_LINE);
	m_fdc->ready_w(CLEAR_LINE);
}

void mpc2000_state::mpc2000_map(address_map &map)
{
	map(0x00'0000, 0x07'ffff).ram(); // RAM is 2x HM5118160 (1M x 16 bit) for a total of 4MiB
	map(0x08'0000, 0x0f'ffff).rom().region("maincpu", 0).mirror(0x70'0000);
	map(0x10'0000, 0x17'ffff).ram();
	map(0x20'0000, 0x27'ffff).ram();
	map(0x30'0000, 0x37'ffff).ram();
	map(0x40'0000, 0x47'ffff).ram();
	map(0x50'0000, 0x57'ffff).ram();
	map(0x60'0000, 0x67'ffff).ram();
	map(0x70'0000, 0x77'ffff).ram();
}

/*
00 = SPCSN
20 = FDCSN
40 = N/A
60 = LCD1N
80 = NSCSN
A0 = FXCSN
C0 = WADCSN (D2 to start sampling, 1A to stop)
E0 = IDECS1N
100 = LCD2N
120 = N/A
140 = LOLEDN
160 = HILEDN
180 = SIO1N
1A0 = SIO2N
1C0 = LTCCSN
1E0 = IDECS0N
*/
void mpc2000_state::mpc2000_io_map(address_map &map)
{
	map(0x0000, 0x001f).m("scsi:7:spc", FUNC(mb89352_device::map)).umask16(0x00ff);
	map(0x0020, 0x0023).m(m_fdc, FUNC(upd72069_device::map)).umask16(0x00ff);
	map(0x0060, 0x0063).rw(FUNC(mpc2000_state::lcd_csr), FUNC(mpc2000_state::lcd_csw<0>)).umask16(0x00ff);
	map(0x0080, 0x008f).m(m_dsp, FUNC(l7a1045_sound_device::map));
	map(0x00a0, 0x00a3).nopw(); // silence effects writes
	map(0x00c0, 0x00c0).rw(FUNC(mpc2000_state::wadcsn_r), FUNC(mpc2000_state::wadcn_w)).umask16(0x00ff);
	map(0x00e0, 0x00ff).rw(m_ata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));

	map(0x0100, 0x0103).rw(FUNC(mpc2000_state::lcd_csr), FUNC(mpc2000_state::lcd_csw<1>)).umask16(0x00ff);
	map(0x0140, 0x015f).w("loledlatch", FUNC(hc259_device::write_a3)).umask16(0x00ff);
	map(0x0160, 0x017f).w("hiledlatch", FUNC(hc259_device::write_a3)).umask16(0x00ff);
	map(0x0180, 0x0187).m(m_sio, FUNC(mb89371_device::map<0>)).umask16(0x00ff);
	map(0x01a0, 0x01a7).m(m_sio, FUNC(mb89371_device::map<1>)).umask16(0x00ff);
	map(0x01e0, 0x01ff).rw(m_ata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
}

uint16_t mpc2000_state::dma_mem16r_cb(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_word(offset << 1);
}

void mpc2000_state::dma_mem16w_cb(offs_t offset, uint16_t data)
{
	m_maincpu->space(AS_PROGRAM).write_word(offset << 1, data);
}

uint8_t mpc2000_state::dma_memr_cb(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void mpc2000_state::dma_memw_cb(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

void mpc2000_state::mpc2000_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("subcpu", 0);
}

void mpc2000_state::dsp_map(address_map &map)
{
	map(0x0000'0000, 0x01ff'ffff).ram();
}

void mpc2000_state::mpc2000_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(230, 240, 250));
	palette.set_pen_color(1, rgb_t(64, 140, 250));
}

u32 mpc2000_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < 60; y++)
	{
		uint16_t *scanline = &bitmap.pix(y);
		const int yoffs = 256*y;

		for (int x = 0; x < 248; x++)
		{
			*scanline++ = m_vram[yoffs + x];
		}
	}

	return 0;
}

uint8_t mpc2000_state::lcd_csr()
{
	return 0;
}

template <int cs> void mpc2000_state::lcd_csw(offs_t offset, uint8_t data)
{
	if (!offset)
	{
		m_lcdcmd[cs] = data;
	}
	else
	{
		switch (m_lcdcmd[cs])
		{
			case 0x20:
				if ((m_lcdx[0] < 256) && (m_lcdy[cs] < 60))
				{
					for (int i = 0; i < 8; i++)
					{
						m_vram[(m_lcdy[cs] * 256) + m_lcdx[cs] + i] = BIT(data, 7-i);
					}
					m_lcdy[cs]++;
				}
				break;

			case 0x21:
				m_lcdx[cs] = (data << 3) + (cs * 160);
				break;

			case 0x22:
				m_lcdy[cs] = 0;
				break;
		}
	}
}

template void mpc2000_state::lcd_csw<0>(offs_t offset, uint8_t data);
template void mpc2000_state::lcd_csw<1>(offs_t offset, uint8_t data);

// WADCSN, handled by Xilinx FPGA at IC220
// bit 7 = 1 to enable the right channel input
// bit 6 = 1 to enable the left channel input
// bit 5 is always clear
// bit 4 is always set
// bit 3 = 1 to disable analog input
// bit 2 = 1 to enable S/PDIF digital input
// bit 1 is always set
// bit 0 is always clear
uint8_t mpc2000_state::wadcsn_r()
{
	return m_wadcsn;
}

void mpc2000_state::wadcn_w(uint8_t data)
{
	m_wadcsn = data;
}

INPUT_CHANGED_MEMBER(mpc2000_state::variation_changed)
{
	if (!oldval && newval)
	{
		m_variation_slider = newval;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(mpc2000_state::dial_timer_tick)
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

uint8_t mpc2000_state::subcpu_pa_r()
{
	return m_keys[7 - m_key_scan_row]->read();
}

uint8_t mpc2000_state::subcpu_pb_r()
{
	return 0;
}

uint8_t mpc2000_state::subcpu_pc_r()
{
	uint8_t rv = 0;

	if (m_count_dial)
	{
		const bool negative = (m_count_dial < 0);

		switch (m_quadrature_phase >> 1)
		{
		case 0:
			rv = negative ? BIT5 : BIT4;
			break;

		case 1:
			rv = BIT4 | BIT5;
			break;

		case 2:
			rv = negative ? BIT4 : BIT5;
			break;

		case 3:
			rv = 0;
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

// drum pad row select, active low
void mpc2000_state::subcpu_pb_w(uint8_t data)
{
	// convert to 1/2/4/8
	m_drum_scan_row = (data & 0xf) ^ 0xf;
	if (m_drum_scan_row != 0)
	{
		// get a row number 0-3
		m_drum_scan_row = count_leading_zeros_32(m_drum_scan_row) - 28;
	}
}

// main buttons row select (PC1-PC3)
void mpc2000_state::subcpu_pc_w(uint8_t data)
{
	m_key_scan_row = ((data ^ 0xff) >> 1) & 7;
}

uint8_t mpc2000_state::an_pads_r()
{
	return 0xff;
}

uint8_t mpc2000_state::an4_r()
{
	return m_variation_slider;
}

void mpc2000_state::fdc_scsi_w(int state)
{
	m_fdc_ata = state;
}

void mpc2000_state::fdc_irq_w(int state)
{
	m_fdc_irq = state;
	recalc_fdc_ata();
}

void mpc2000_state::fdc_drq_w(int state)
{
	m_fdc_drq = state;
	recalc_fdc_ata();
}

void mpc2000_state::ata_irq_w(int state)
{
	m_ata_irq = state;
	recalc_fdc_ata();
}

void mpc2000_state::ata_drq_w(int state)
{
	m_ata_drq = state;
	recalc_fdc_ata();
}

void mpc2000_state::recalc_fdc_ata()
{
	if (m_fdc_ata)
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ3, m_fdc_irq);
		m_maincpu->dreq_w<1>(m_fdc_drq);
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_IRQ3, m_ata_irq);
		m_maincpu->dreq_w<1>(m_ata_drq);
	}
}

uint8_t mpc2000_state::ts_r()
{
	const auto imagedev = m_floppy->get_device();
	return imagedev->dskchg_r();
}

void mpc2000_state::floppies(device_slot_interface &device)
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

void mpc2000_state::mpc2000(machine_config &config)
{
	V53A(config, m_maincpu, 32_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpc2000_state::mpc2000_map);
	m_maincpu->set_addrmap(AS_IO, &mpc2000_state::mpc2000_io_map);
	m_maincpu->out_hreq_cb().set(m_maincpu, FUNC(v53a_device::hack_w));
	m_maincpu->in_memr_cb().set(FUNC(mpc2000_state::dma_memr_cb));
	m_maincpu->out_memw_cb().set(FUNC(mpc2000_state::dma_memw_cb));
	m_maincpu->in_mem16r_cb().set(FUNC(mpc2000_state::dma_mem16r_cb));
	m_maincpu->out_mem16w_cb().set(FUNC(mpc2000_state::dma_mem16w_cb));
	m_maincpu->out_eop_cb().set("tc", FUNC(input_merger_device::in_w<0>)).invert();
	m_maincpu->in_ior_cb<0>().set("scsi:7:spc", FUNC(mb89352_device::dma_r));
	m_maincpu->out_iow_cb<0>().set("scsi:7:spc", FUNC(mb89352_device::dma_w));
	m_maincpu->in_ior_cb<1>().set(m_fdc, FUNC(upd72069_device::dma_r));
	m_maincpu->out_iow_cb<1>().set(m_fdc, FUNC(upd72069_device::dma_w));
	m_maincpu->in_io16r_cb<1>().set(m_ata, FUNC(ata_interface_device::read_dma));
	m_maincpu->out_io16w_cb<1>().set(m_ata, FUNC(ata_interface_device::write_dma));
	m_maincpu->in_io16r_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_r16_cb));
	m_maincpu->out_io16w_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_w16_cb));
	m_maincpu->set_tclk(4'000'000); // FIXME: DAWCK generated by DSP (also tied to V53 DSR input)
	m_maincpu->tout_handler<0>().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_maincpu->tout_handler<1>().set_inputline(m_maincpu, INPUT_LINE_IRQ5);
	m_maincpu->rxrdy_handler_cb().set("intp4", FUNC(input_merger_device::in_w<0>));

	constexpr XTAL V53_CLKOUT = 32_MHz_XTAL / 2;
	constexpr XTAL V53_PCLKOUT = 32_MHz_XTAL / 4;

	// HC02 gates
	INPUT_MERGER_ALL_HIGH(config, "tc").output_handler().set(m_fdc, FUNC(upd72069_device::tc_line_w));

	hc259_device &loledlatch(HC259(config, "loledlatch"));
	loledlatch.q_out_cb<0>().set_output("led0").invert(); // After
	loledlatch.q_out_cb<1>().set_output("led1").invert(); // Record
	loledlatch.q_out_cb<2>().set_output("led2").invert(); // Undo Sq
	loledlatch.q_out_cb<3>().set_output("led3").invert(); // Play
	loledlatch.q_out_cb<4>().set_output("led4").invert(); // Over Dub
	loledlatch.q_out_cb<5>().set_output("led5").invert(); // not used
	loledlatch.q_out_cb<6>().set_output("led6").invert(); // not used
	loledlatch.q_out_cb<7>().set_output("led7").invert(); // not used

	hc259_device &hiledlatch(HC259(config, "hiledlatch"));
	hiledlatch.q_out_cb<0>().set_output("led8").invert();  // Full Level
	hiledlatch.q_out_cb<1>().set_output("led9").invert();  // Bank D
	hiledlatch.q_out_cb<2>().set_output("led10").invert(); // Bank B
	hiledlatch.q_out_cb<3>().set_output("led11").invert(); // Track Mute
	hiledlatch.q_out_cb<4>().set_output("led12").invert(); // Next Seq
	hiledlatch.q_out_cb<5>().set_output("led13").invert(); // Bank A
	hiledlatch.q_out_cb<6>().set_output("led14").invert(); // Bank C
	hiledlatch.q_out_cb<7>().set_output("led15").invert(); // 16 Levels

	UPD78C10(config, m_subcpu, 12_MHz_XTAL);
	m_subcpu->set_addrmap(AS_PROGRAM, &mpc2000_state::mpc2000_sub_map);
	m_subcpu->txd_func().set(m_maincpu, FUNC(v53a_device::rxd_w));
	m_subcpu->pa_in_cb().set(FUNC(mpc2000_state::subcpu_pa_r));
	m_subcpu->pb_in_cb().set(FUNC(mpc2000_state::subcpu_pb_r));
	m_subcpu->pc_in_cb().set(FUNC(mpc2000_state::subcpu_pc_r));
	m_subcpu->pb_out_cb().set(FUNC(mpc2000_state::subcpu_pb_w));
	m_subcpu->pc_out_cb().set(FUNC(mpc2000_state::subcpu_pc_w));
	m_subcpu->an0_func().set(FUNC(mpc2000_state::an_pads_r));
	m_subcpu->an1_func().set(FUNC(mpc2000_state::an_pads_r));
	m_subcpu->an2_func().set(FUNC(mpc2000_state::an_pads_r));
	m_subcpu->an3_func().set(FUNC(mpc2000_state::an_pads_r));
	m_subcpu->an4_func().set(FUNC(mpc2000_state::an4_r));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(80);
	m_screen->set_screen_update(FUNC(mpc2000_state::screen_update));
	m_screen->set_size(248, 60);
	m_screen->set_visarea(0, 248-1, 0, 60-1);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(mpc2000_state::mpc2000_palette), 2);

	UPD72069(config, m_fdc, V53_CLKOUT); // actually UPD72068, which is software-identical
	m_fdc->set_ready_line_connected(false); // uPD READY pin is grounded on schematic
	m_fdc->set_ts_line_connected(false);    // actually connected to DSKCHG (!)
	m_fdc->intrq_wr_callback().set(FUNC(mpc2000_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(mpc2000_state::fdc_drq_w));
	m_fdc->ts_rd_callback().set(FUNC(mpc2000_state::ts_r));

	FLOPPY_CONNECTOR(config, m_floppy, mpc2000_state::floppies, "35hd", add_formats).enable_sound(false);

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
	m_ata->irq_handler().set(FUNC(mpc2000_state::ata_irq_w));
	m_ata->dmarq_handler().set(FUNC(mpc2000_state::ata_drq_w));

	INPUT_MERGER_ANY_HIGH(config, "intp4").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ4);

	// IC100: MIDI A & B
	MB89371(config, m_sio, V53_PCLKOUT);
	m_sio->rxrdy_handler<0>().set("intp4", FUNC(input_merger_device::in_w<1>));
	m_sio->rxrdy_handler<1>().set("intp4", FUNC(input_merger_device::in_w<2>));
	m_sio->txrdy_handler<0>().set_inputline(m_maincpu, INPUT_LINE_IRQ7);
	m_sio->txrdy_handler<1>().set_inputline(m_maincpu, INPUT_LINE_IRQ6);
	m_sio->txd_handler<0>().set("mdout1", FUNC(midi_port_device::write_txd));
	m_sio->txd_handler<1>().set("mdout2", FUNC(midi_port_device::write_txd));
	m_sio->dtr_handler<0>().set(FUNC(mpc2000_state::fdc_scsi_w));

	MIDI_PORT(config, "mdin1", midiin_slot, "midiin").rxd_handler().set(m_sio, FUNC(mb89371_device::write_rxd<0>));
	MIDI_PORT(config, "mdin2", midiin_slot, "midiin").rxd_handler().set(m_sio, FUNC(mb89371_device::write_rxd<1>));
	MIDI_PORT(config, "mdout1", midiout_slot, "midiout");
	MIDI_PORT(config, "mdout2", midiout_slot, "midiout");

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

	TIMER(config, "dialtimer").configure_periodic(FUNC(mpc2000_state::dial_timer_tick), attotime::from_hz(60.0));

	SPEAKER(config, "speaker", 2).front();
	SPEAKER(config, "outputs", 8).unknown();

	L7A1045(config, m_dsp, 33.8688_MHz_XTAL); // clock verified by schematic
	m_dsp->set_addrmap(AS_DATA, &mpc2000_state::dsp_map);
	m_dsp->drq_handler_cb().set(m_maincpu, FUNC(v53a_device::dreq_w<3>));
	m_dsp->add_route(l7a1045_sound_device::L6028_LEFT, "speaker", 1.0, 0);
	m_dsp->add_route(l7a1045_sound_device::L6028_RIGHT, "speaker", 1.0, 1);

	m_dsp->add_route(l7a1045_sound_device::L6028_OUT0, "outputs", 1.0, 0);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT1, "outputs", 1.0, 1);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT2, "outputs", 1.0, 2);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT3, "outputs", 1.0, 3);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT4, "outputs", 1.0, 4);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT5, "outputs", 1.0, 5);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT6, "outputs", 1.0, 6);
	m_dsp->add_route(l7a1045_sound_device::L6028_OUT7, "outputs", 1.0, 7);

	// back compatible with MPC3000 and MPC60 disks
	SOFTWARE_LIST(config, "flop_mpc3000").set_original("mpc3000_flop");
}

static INPUT_PORTS_START( mpc2000 )
	PORT_START("Y0")
	PORT_BIT(0x81, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9 / MIDI/Sync") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8 / Other") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Step <") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Play") PORT_CODE(KEYCODE_V)

	PORT_START("Y1")
	PORT_BIT(0x81, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 / Misc") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Step >") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("<< Bar") PORT_CODE(KEYCODE_F)

	PORT_START("Y2")
	PORT_BIT(0x81, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3 / Load") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1 / Song") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("After") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Go To") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down Arrow")

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6 / Program") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5 / Trim") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tap Tempo") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Record") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right Arrow")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bar >>") PORT_CODE(KEYCODE_G)

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Main Screen") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4 / Sample") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Undo") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Over Dub") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left Arrow")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Play Start") PORT_CODE(KEYCODE_B)

	PORT_START("Y5")
	PORT_BIT(0x81, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Window") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 / Mixer") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Erase") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up Arrow")

	PORT_START("Y6")
	PORT_BIT(0xe1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Next Seq") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Track Mute") PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bank A") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bank B") PORT_CODE(KEYCODE_M)

	PORT_START("Y7")
	PORT_BIT(0xe1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("16 Levels") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Full Level") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bank D") PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Bank C") PORT_CODE(KEYCODE_COMMA)

	PORT_START("VARIATION")
	PORT_ADJUSTER(100, "NOTE VARIATION") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(mpc2000_state::variation_changed), 1)

	PORT_START("DATAENTRY")
	PORT_BIT( 0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_CODE_DEC(KEYCODE_F14) PORT_CODE_INC(KEYCODE_F15)
INPUT_PORTS_END

ROM_START( mpc2000xl )
	ROM_REGION(0x80000, "maincpu", 0)   // V53 code
	ROM_SYSTEM_BIOS(0, "default", "ver 1.20 (February 7, 2005)")
	ROMX_LOAD( "mpc2000xl_120.bin", 0x000000, 0x080000, CRC(c082f188) SHA1(382be688972fe3d85caeca99abff4b6c391347fb), ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "ver114", "ver 1.14 (May 16, 2001)")
	ROMX_LOAD( "mpc2000xl_114.bin", 0x000000, 0x080000, CRC(7b4fe8af) SHA1(f272ef2b1948730446fe260e4c0c830884832d3a), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "ver112", "ver 1.12 (December 15, 2000)")
	ROMX_LOAD( "mpc2000xl_112.bin", 0x000000, 0x080000, CRC(552f660a) SHA1(91d1a1bb5aaa0b98538b4b47567c59770ba9a356), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS(3, "ver111", "ver 1.11 (March 23, 2000)")
	ROMX_LOAD( "mpc2000xl_111.bin", 0x000000, 0x080000, CRC(0c232258) SHA1(3dd6481c9ced76baa8990580a23aec7c3aa9a1d1), ROM_BIOS(3) )

	ROM_SYSTEM_BIOS(4, "ver110", "ver 1.10 (March 2, 2000)")
	ROMX_LOAD( "mpc2000xl_110.bin", 0x000000, 0x080000, CRC(12700bee) SHA1(dee8833a55b065cd81f34a2bcc29df0dcf26c9f1), ROM_BIOS(4) )

	ROM_SYSTEM_BIOS(5, "ver107", "ver 1.07 (November 8, 1999)")
	ROMX_LOAD( "mpc2000xl_107.bin", 0x000000, 0x080000, CRC(3bc97b99) SHA1(2653b08245cc0d223bff24bc6da8c4817e39095c), ROM_BIOS(5) )

	ROM_REGION(0x8000, "subcpu", 0)    // uPD78C10 panel controller code
	ROM_LOAD( "akai mpc2000xl op v1_0.bin", 0x000000, 0x008000, CRC(24382ade) SHA1(b02ca9b8a4ae41f4414e12d5188f638dbb99f36c) )
ROM_END

void mpc2000_state::init_mpc2000()
{
}

CONS( 1994, mpc2000xl, 0, 0, mpc2000, mpc2000, mpc2000_state, init_mpc2000, "Akai / Roger Linn", "MPC 2000XL", MACHINE_NOT_WORKING )

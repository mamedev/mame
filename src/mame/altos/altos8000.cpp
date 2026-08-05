// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Preliminary driver for Altos ACS8000 Microcomputer Family & clones.

    According to "Manual Geral Poly 201 DP", the system also includes a video
    board that uses a I8275 CRTC and 4802 RAM.

*******************************************************************************/

#include "emu.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/output_latch.h"
#include "machine/wd_fdc.h"
#include "machine/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"
#include "screen.h"
#include "speaker.h"

namespace {

class altos8000_state : public driver_device
{
public:
	altos8000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "floppy%u", 1U)
		, m_ipl_view(*this, "ipl")
	{
	}

	void poly201(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 dma_mem_r(offs_t offset);
	void dma_mem_w(offs_t offset, u8 data);
	u8 dma_io_r(offs_t offset);
	void dma_io_w(offs_t offset, u8 data);

	void floppy_select_w(u8 data);
	u8 floppy_sense_r();
	void floppy_misc_w(u8 data);
	void ipl_disable_w(u8 data);

	void poly201_mem_map(address_map &map) ATTR_COLD;
	void poly201_io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<wd_fdc_device_base> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	memory_view m_ipl_view;

	floppy_image_device *m_selected_floppy;
};


void altos8000_state::machine_start()
{
	m_selected_floppy = nullptr;
}

void altos8000_state::machine_reset()
{
	m_ipl_view.select(0);
}

u8 altos8000_state::dma_mem_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void altos8000_state::dma_mem_w(offs_t offset, u8 data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

u8 altos8000_state::dma_io_r(offs_t offset)
{
	return m_maincpu->space(AS_IO).read_byte(offset);
}

void altos8000_state::dma_io_w(offs_t offset, u8 data)
{
	m_maincpu->space(AS_IO).write_byte(offset, data);
}

void altos8000_state::floppy_select_w(u8 data)
{
	m_fdc->dden_w(!BIT(data, 0));

	m_selected_floppy = nullptr;
	for (int i = 0; i < 4; i++)
	{
		if (BIT(data, 2 + i))
		{
			m_selected_floppy = m_floppy[i]->get_device();
			break;
		}
	}
	m_fdc->set_floppy(m_selected_floppy);
}

u8 altos8000_state::floppy_sense_r()
{
	u8 ret = 0;

	if (!m_selected_floppy || m_selected_floppy->dskchg_r())
		ret |= 0x04;
	if (m_selected_floppy && !m_selected_floppy->twosid_r())
		ret |= 0x80;

	return ret;
}

void altos8000_state::floppy_misc_w(u8 data)
{
	m_fdc->mr_w(BIT(data, 1));
	// Not used in Poly 201
	if (0 && m_selected_floppy)
		m_selected_floppy->ss_w(!BIT(data, 5));
}

void altos8000_state::ipl_disable_w(u8 data)
{
	m_ipl_view.disable();
}

void altos8000_state::poly201_mem_map(address_map &map)
{
	map(0x0000, 0xffff).ram();
	map(0x0000, 0xffff).view(m_ipl_view);
	m_ipl_view[0](0x0000, 0x03ff).mirror(0xdc00).rom().region("ipl", 0);
}

void altos8000_state::poly201_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).mirror(3).rw("dma", FUNC(z80dma_device::read), FUNC(z80dma_device::write));
	map(0x04, 0x07).rw(m_fdc, FUNC(fd1797_device::read), FUNC(fd1797_device::write));
	map(0x08, 0x0b).rw("pio1", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x0c, 0x0f).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw("pio2", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x14, 0x14).mirror(3).w(FUNC(altos8000_state::ipl_disable_w));
	map(0x1c, 0x1f).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0x5e, 0x5e).nopw();
	map(0xcf, 0xcf).nopw();
}


static INPUT_PORTS_START(poly201)
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "dma" },
	{ "pio1" },
	{ "pio2" },
	{ "ctc" },
	{ "dart" },
	{ nullptr }
};

static void altos8000_floppies(device_slot_interface &device)
{
	device.option_add("8dd", FLOPPY_8_DSDD);
}

void altos8000_state::poly201(machine_config &config)
{
	Z80(config, m_maincpu, 8_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &altos8000_state::poly201_mem_map);
	m_maincpu->set_addrmap(AS_IO, &altos8000_state::poly201_io_map);
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->busack_cb().set("dma", FUNC(z80dma_device::bai_w));

	z80dma_device &dma(Z80DMA(config, "dma", 8_MHz_XTAL / 2));
	dma.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	dma.out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSREQ);
	dma.in_mreq_callback().set(FUNC(altos8000_state::dma_mem_r));
	dma.out_mreq_callback().set(FUNC(altos8000_state::dma_mem_w));
	dma.in_iorq_callback().set(FUNC(altos8000_state::dma_io_r));
	dma.out_iorq_callback().set(FUNC(altos8000_state::dma_io_w));

	z80pio_device &pio1(Z80PIO(config, "pio1", 8_MHz_XTAL / 2));
	pio1.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio1.out_pa_callback().set(FUNC(altos8000_state::floppy_select_w));
	pio1.in_pb_callback().set(FUNC(altos8000_state::floppy_sense_r));
	pio1.out_pb_callback().set(FUNC(altos8000_state::floppy_misc_w));

	z80pio_device &pio2(Z80PIO(config, "pio2", 8_MHz_XTAL / 2));
	pio2.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	pio2.out_pa_callback().set("user", FUNC(centronics_device::write_strobe)).bit(0);
	//pio2.out_pa_callback().append("user", FUNC(centronics_device::write_prime)).bit(1);
	//pio2.out_pa_callback().append("user", FUNC(centronics_device::write_cntl)).bit(2);
	pio2.out_pb_callback().set("userout", FUNC(output_latch_device::write));

	z80dart_device &dart(Z80DART(config, "dart", 8_MHz_XTAL / 2));
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	dart.out_txda_callback().set("cons", FUNC(rs232_port_device::write_txd));
	dart.out_rtsa_callback().set("dart", FUNC(z80dart_device::ctsa_w));
	dart.out_dtra_callback().set("cons", FUNC(rs232_port_device::write_dtr));
	dart.out_txdb_callback().set("printer", FUNC(rs232_port_device::write_txd));
	dart.out_rtsb_callback().set("dart", FUNC(z80dart_device::ctsb_w));
	dart.out_dtrb_callback().set("printer", FUNC(rs232_port_device::write_dtr));

	z80ctc_device &ctc(Z80CTC(config, "ctc", 8_MHz_XTAL / 2));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	ctc.set_clk<0>(8_MHz_XTAL / 4);
	ctc.set_clk<1>(8_MHz_XTAL / 4);
	ctc.set_clk<2>(8_MHz_XTAL / 4);
	ctc.zc_callback<0>().set("dart", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<0>().append("dart", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<2>().set("dart", FUNC(z80dart_device::rxtxcb_w));

	FD1797(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set("pio1", FUNC(z80pio_device::pa6_w));
	// Poly 201 apparently uses NMI to time data reads/writes, not the Z80 DMA
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_fdc->hld_wr_callback().set("pio1", FUNC(z80pio_device::pa1_w));

	for (auto &flop : m_floppy)
		FLOPPY_CONNECTOR(config, flop, altos8000_floppies, "8dd", floppy_image_device::default_mfm_floppy_formats);

	rs232_port_device &console(RS232_PORT(config, "cons", default_rs232_devices, "terminal"));
	console.rxd_handler().set("dart", FUNC(z80dart_device::rxa_w));
	console.dsr_handler().set("dart", FUNC(z80dart_device::dcda_w));

	rs232_port_device &printer(RS232_PORT(config, "printer", default_rs232_devices, nullptr));
	printer.rxd_handler().set("dart", FUNC(z80dart_device::rxb_w));
	printer.dsr_handler().set("dart", FUNC(z80dart_device::dcdb_w));

	output_latch_device &userout(OUTPUT_LATCH(config, "userout"));

	centronics_device &userport(CENTRONICS(config, "user", centronics_devices, nullptr));
	userport.set_output_latch(userout);
	userport.ack_handler().set("pio2", FUNC(z80pio_device::strobe_b));
	userport.fault_handler().set("pio2", FUNC(z80pio_device::pa3_w));
	userport.perror_handler().set("pio2", FUNC(z80pio_device::pa4_w));
	userport.busy_handler().set("pio2", FUNC(z80pio_device::pa5_w));
	userport.select_handler().set("pio2", FUNC(z80pio_device::pa6_w));
}

ROM_START(poly201)
	ROM_REGION(0x400, "ipl", 0)
	// "Monitor Vers 1.53" (2708 EPROM)
	ROM_LOAD("poly201m.bin", 0x000, 0x400, CRC(dbcd4d77) SHA1(78d37ea444b56a8a6bcbc7055bfa11ba3ca507ba))
ROM_END

} // anonymous namespace

SYST(1981, poly201, 0, 0, poly201, poly201, altos8000_state, empty_init, u8"Polymax Sistemas e Periféricos", "Poly 201 DP", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND)

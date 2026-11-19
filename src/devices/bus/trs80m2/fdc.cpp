// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Tandy Model II floppy disk controller/printer interface emulation

    8709063 (8000042): FD1791, Z80 PIO and Centronics port.

    Port    Read                    Write
    E0-E3   PIO                     PIO
    E4-E7   FD1791 (data inverted)  FD1791 (data inverted)
    EF                              drive select

**********************************************************************/

#include "emu.h"
#include "fdc.h"


DEFINE_DEVICE_TYPE(TRS80M2_FDC, trs80m2_fdc_device, "trs80m2_fdc", "Tandy Model II floppy disk controller")


namespace {

void trs80m2_floppies(device_slot_interface &device)
{
	device.option_add("8ssdd", FLOPPY_8_SSDD); // Shugart SA-800
	device.option_add("8dsdd", FLOPPY_8_DSDD); // Shugart SA-850
}

} // anonymous namespace


trs80m2_fdc_device::trs80m2_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TRS80M2_FDC, tag, owner, clock),
	device_trs80m2_card_interface(mconfig, *this),
	m_fdc(*this, "u6"),
	m_pio(*this, "u22"),
	m_centronics(*this, "j2"),
	m_cent_data_out(*this, "cent_data_out"),
	m_floppies(*this, "u6:%u", 0U),
	m_floppy(nullptr),
	m_drvslt(0xff),
	m_centronics_busy(0),
	m_centronics_fault(0),
	m_centronics_perror(0)
{
}

void trs80m2_fdc_device::device_add_mconfig(machine_config &config)
{
	FD1791(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(m_pio, FUNC(z80pio_device::pa0_w));
	m_fdc->drq_wr_callback().set([this] (int state) { m_slot->xferrq_w(state); });
	FLOPPY_CONNECTOR(config, m_floppies[0], trs80m2_floppies, "8dsdd", floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[1], trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[2], trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppies[3], trs80m2_floppies, nullptr, floppy_image_device::default_mfm_floppy_formats);

	Z80PIO(config, m_pio, DERIVED_CLOCK(1, 1));
	m_pio->out_int_callback().set([this] (int state) { m_slot->int_w(state); });
	m_pio->in_pa_callback().set(FUNC(trs80m2_fdc_device::pio_pa_r));
	m_pio->out_pa_callback().set(FUNC(trs80m2_fdc_device::pio_pa_w));
	m_pio->out_pb_callback().set(m_cent_data_out, FUNC(output_latch_device::write));
	m_pio->out_brdy_callback().set([this] (int state) { m_centronics->write_strobe(!state); });

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_pio, FUNC(z80pio_device::strobe_b));
	m_centronics->busy_handler().set([this] (int state) { m_centronics_busy = state; });
	m_centronics->fault_handler().set([this] (int state) { m_centronics_fault = state; });
	m_centronics->perror_handler().set([this] (int state) { m_centronics_perror = state; });

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);
}

void trs80m2_fdc_device::map(address_map &map)
{
	map(0x0, 0x3).rw(m_pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
	map(0x4, 0x7).rw(FUNC(trs80m2_fdc_device::fdc_r), FUNC(trs80m2_fdc_device::fdc_w));
	map(0xf, 0xf).w(FUNC(trs80m2_fdc_device::drvslt_w));
}

void trs80m2_fdc_device::device_start()
{
	m_slot->bus().io().install_device(0xe0, 0xef, *this, &trs80m2_fdc_device::map);

	save_item(NAME(m_drvslt));
	save_item(NAME(m_centronics_busy));
	save_item(NAME(m_centronics_fault));
	save_item(NAME(m_centronics_perror));
}

void trs80m2_fdc_device::device_post_load()
{
	select_floppy();
}

int trs80m2_fdc_device::z80daisy_irq_state()
{
	return static_cast<device_z80daisy_interface &>(*m_pio).z80daisy_irq_state();
}

int trs80m2_fdc_device::z80daisy_irq_ack()
{
	return static_cast<device_z80daisy_interface &>(*m_pio).z80daisy_irq_ack();
}

void trs80m2_fdc_device::z80daisy_irq_reti()
{
	static_cast<device_z80daisy_interface &>(*m_pio).z80daisy_irq_reti();
}

uint8_t trs80m2_fdc_device::fdc_r(offs_t offset)
{
	return m_fdc->read(offset) ^ 0xff;
}

void trs80m2_fdc_device::fdc_w(offs_t offset, uint8_t data)
{
	m_fdc->write(offset, data ^ 0xff);
}

void trs80m2_fdc_device::drvslt_w(uint8_t data)
{
	/*

	    bit     signal

	    0       DS1
	    1       DS2
	    2       DS3
	    3       DS4
	    4
	    5
	    6       SDSEL
	    7       FM/MFM

	*/

	m_drvslt = data;

	select_floppy();

	if (m_floppy)
		m_floppy->ss_w(!BIT(data, 6));

	m_fdc->dden_w(!BIT(data, 7));
}

void trs80m2_fdc_device::select_floppy()
{
	m_floppy = nullptr;

	for (int i = 0; i < 4; i++)
		if (!BIT(m_drvslt, i))
			m_floppy = m_floppies[i]->get_device();

	m_fdc->set_floppy(m_floppy);
}

uint8_t trs80m2_fdc_device::pio_pa_r()
{
	/*

	    bit     signal      description

	    0       INTRQ       FDC INT request
	    1       TWOSID      2-sided diskette
	    2       DSKCHG      disk change
	    3       PRIME       prime
	    4       FAULT       printer fault
	    5       PSEL        printer select
	    6       PE          paper empty
	    7       BUSY        printer busy

	*/

	uint8_t data = 0;

	data |= m_fdc->intrq_r() ? 0x01 : 0x00;
	data |= (m_floppy && !m_floppy->twosid_r()) ? 0x02 : 0x00;
	data |= (m_floppy && !m_floppy->dskchg_r()) ? 0x04 : 0x00;
	data |= m_centronics_fault << 4;
	data |= m_centronics_perror << 6;
	data |= m_centronics_busy << 7;

	return data;
}

void trs80m2_fdc_device::pio_pa_w(uint8_t data)
{
	m_centronics->write_init(BIT(data, 3));
}

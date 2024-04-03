// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * Tektronix 4404 Mass Storage Unit
 *
 * The MSU contains a 5 1/4" floppy drive, a 40MB hard disk and controllers
 * which implement the SASI protocol to communicate with the host system. This
 * device emulates the floppy disk portion, which consists of a 6502 CPU, EPROM,
 * RAM and a μPD765-compatible floppy drive controller chip. SASI signals are
 * directly controlled by the CPU using discrete logic.
 *
 * Sources:
 *  - 4404 Artificial Intelligence System, Component-Level Service Manual, Part No. 070-5610-01, Product Group 07, June 1987, Tektronix
 *
 * TODO:
 *  - decode address 0x0806
 */

#include "emu.h"
#include "tek_msu_fdc.h"

//#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(TEK_MSU_FDC, tek_msu_fdc_device, "tek_msu_fdc", "Tektronix 4404 Mass Storage Unit Floppy Drive Controller")

tek_msu_fdc_device::tek_msu_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nscsi_device(mconfig, TEK_MSU_FDC, tag, owner, clock)
	, nscsi_slot_card_interface(mconfig, *this, DEVICE_SELF)
	, m_cpu(*this, "cpu")
	, m_fdc(*this, "fdc")
	, m_fdd(*this, "fdc:0")
{
}

void tek_msu_fdc_device::device_start()
{
	nscsi_device::device_start();

	save_item(NAME(m_minisel));

	m_motor = timer_alloc(FUNC(tek_msu_fdc_device::motor), this);
}

void tek_msu_fdc_device::device_reset()
{
	nscsi_device::device_reset();

	// monitor SCSI reset
	scsi_bus->ctrl_wait(scsi_refid, S_RST, S_RST);

	m_minisel = true;
}

void tek_msu_fdc_device::scsi_ctrl_changed()
{
	bool const reset = scsi_bus->ctrl_r() & S_RST;

	LOG("scsi reset %d\n", reset);

	if (reset)
	{
		// release the SCSI bus
		scsi_bus->data_w(scsi_refid, 0);
		scsi_bus->ctrl_w(scsi_refid, 0, S_REQ | S_BSY | S_MSG | S_CTL | S_INP);

		m_minisel = true;
	}

	m_cpu->set_input_line(INPUT_LINE_RESET, reset);
	m_fdc->reset_w(reset);
}

void tek_msu_fdc_device::mem_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();

	map(0x0800, 0x0800).rw(FUNC(tek_msu_fdc_device::status_r), FUNC(tek_msu_fdc_device::control_w));
	map(0x0801, 0x0801).rw(FUNC(tek_msu_fdc_device::data_r), FUNC(tek_msu_fdc_device::data_w));

	map(0x0803, 0x0803).lw8([this](u8 data) { m_fdc->tc_w(1); m_fdc->tc_w(0); }, "tc_w");
	map(0x0804, 0x0805).rw(FUNC(tek_msu_fdc_device::fdc_r), FUNC(tek_msu_fdc_device::fdc_w));

	// TODO: how does this decode?
	map(0x0806, 0x0806).nopw();

	map(0xf000, 0xffff).rom().region("cpu", 0);
}

static void fdd_devices(device_slot_interface &device)
{
	device.option_add("525dd", FLOPPY_525_DD);
}

void tek_msu_fdc_device::device_add_mconfig(machine_config &config)
{
	M6502(config, m_cpu, 16_MHz_XTAL / 4);
	m_cpu->set_addrmap(AS_PROGRAM, &tek_msu_fdc_device::mem_map);

	I8272A(config, m_fdc, 16_MHz_XTAL / 4, false);
	m_fdc->intrq_wr_callback().set_inputline(m_cpu, INPUT_LINE_IRQ0);
	// drive selection (US) outputs are decoded, inverted and fed back to the
	// RDY input via a jumper block; units 0 and 1 are connected by default,
	// even though the unit has only a single 5 1/4" drive fitted
	m_fdc->us_wr_callback().set([this](u8 data) { m_fdc->ready_w(data > 1); });

	FLOPPY_CONNECTOR(config, m_fdd, fdd_devices, "525dd", floppy_image_device::default_mfm_floppy_formats).enable_sound(true);
}

ROM_START(tek_msu_fdc)
	ROM_REGION(0x1000, "cpu", 0)
	// Tektronix 1985
	// 160-2149-02
	// U-130 V1.2
	ROM_LOAD("160_2149_02__u_130_v1.2.u130", 0x000000, 0x001000, CRC(2c11a3f1) SHA1(b29b3705692d50f15f7e8bbba12a24c69817d52e))
ROM_END

const tiny_rom_entry *tek_msu_fdc_device::device_rom_region() const
{
	return ROM_NAME(tek_msu_fdc);
}

u8 tek_msu_fdc_device::status_r()
{
	u32 const ctrl = scsi_bus->ctrl_r();

	// bit  function
	//  7   SCSI ACK
	//  6   SCSI ATN
	//  5   SCSI SEL
	//  4   SCSI BSY
	//  3   MUTS (floppy spin-up delay?)
	//  2   ID2
	//  1   ID1
	//  0   ID0

	u8 const data =
		((ctrl & S_ACK) ? 0x80 : 0) |
		((ctrl & S_ATN) ? 0x40 : 0) |
		((ctrl & S_SEL) ? 0x20 : 0) |
		((ctrl & S_BSY) ? 0x10 : 0) |
		(m_motor->elapsed() > attotime::from_msec(500) ? 0x08 : 0) |
		scsi_id;

	return data;
}

u8 tek_msu_fdc_device::data_r()
{
	return scsi_bus->data_r();
}

void tek_msu_fdc_device::control_w(u8 data)
{
	LOG("control_w 0x%02x (%s)\n", data, machine().describe_context());

	u32 ctrl = 0;

	// bit  function
	//  7   SCSI REQ
	//  6   SCSI I/O
	//  5   SCSI C/D
	//  4   SCSI MSG
	//  3   SCSI BSY
	//  2   ENABLE
	//  1   N/C
	//  0   MINISEL

	// bit 2 enables output
	if (BIT(data, 2))
		ctrl =
			(BIT(data, 7) ? S_REQ : 0) |
			(BIT(data, 6) ? S_INP : 0) |
			(BIT(data, 5) ? S_CTL : 0) |
			(BIT(data, 4) ? S_MSG : 0) |
			(BIT(data, 3) ? S_BSY : 0);

	scsi_bus->ctrl_w(scsi_refid, ctrl, S_REQ | S_BSY | S_MSG | S_CTL | S_INP);

	// bit 0 is motor enable (active low)
	m_minisel = !BIT(data, 0);

	if (!m_minisel)
		motor(m_minisel);
}

void tek_msu_fdc_device::data_w(u8 data)
{
	scsi_bus->data_w(scsi_refid, data);
}

u8 tek_msu_fdc_device::fdc_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (m_minisel)
			motor(1);

		return offset ? m_fdc->fifo_r() : m_fdc->msr_r();
	}
	else
		return 0;
}

void tek_msu_fdc_device::fdc_w(offs_t offset, u8 data)
{
	if (m_minisel)
		motor(1);

	if (offset)
		m_fdc->fifo_w(data);
}

void tek_msu_fdc_device::motor(int param)
{
	floppy_image_device *fid = m_fdd->get_device();

	if (fid && fid->mon_r() == param)
	{
		LOG("motor %s\n", param ? "on" : "off");

		fid->mon_w(!param);
	}

	// restart or reset motor off timer
	if (param)
		m_motor->adjust(attotime::from_msec(2000));
	else
		m_motor->adjust(attotime::never);
}

// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    ASC Associates SASI Host Computer Adapter

    This is a simple host adapter using LSTTL latches and buffers to
    interface a SASI Winchester drive to the S-100 bus. The 2716 PROM
    contains 8080 bootstrap code for a CP/M system.

    For a technical description of this board by its designers, see
    "Building a Hard-Disk Interface for a S-100 Bus System" by Andrew
    C. Cruce and Scott A. Alexander in the March, April and May 1983
    issues of BYTE magazine. (The schematic diagrams contain various
    minor errors.)

**********************************************************************/

#include "emu.h"
#include "ascsasi.h"

#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"

class asc_sasi_device : public device_t, public device_s100_card_interface
{
	// N.B. actual pulse widths depend on S-100 bus characteristics
	static constexpr attotime s_pulse_width = attotime::from_nsec(500);

public:
	// construction/destruction
	asc_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_s100_card_interface overrides
	virtual u8 s100_smemr_r(offs_t offset) override;
	virtual u8 s100_sinp_r(offs_t offset) override;
	virtual void s100_sout_w(offs_t offset, u8 data) override;

private:
	void iio_w(int state);
	void req_w(int state);
	void sasi_sel_pulse();
	void sasi_rst_pulse();
	TIMER_CALLBACK_MEMBER(sel_off);
	TIMER_CALLBACK_MEMBER(rst_off);

	// object finders
	required_device<nscsi_bus_device> m_sasi;
	required_region_ptr<u8> m_bootstrap;
	required_ioport m_memsel;
	required_ioport m_iosel;

	// pulse timers
	emu_timer *m_sel_off_timer;
	emu_timer *m_rst_off_timer;

	// internal state
	u8 m_data_latch;
	bool m_boot;
};

constexpr attotime asc_sasi_device::s_pulse_width; // stupid non-inline semantics

DEFINE_DEVICE_TYPE_PRIVATE(S100_ASC_SASI, device_s100_card_interface, asc_sasi_device, "ascsasi", "ASC Associates SASI Host Computer Adapter")

asc_sasi_device::asc_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, S100_ASC_SASI, tag, owner, clock)
	, device_s100_card_interface(mconfig, *this)
	, m_sasi(*this, "sasi")
	, m_bootstrap(*this, "bootstrap")
	, m_memsel(*this, "MEMSEL")
	, m_iosel(*this, "IOSEL")
	, m_sel_off_timer(nullptr)
	, m_rst_off_timer(nullptr)
	, m_data_latch(0)
	, m_boot(false)
{
}

void asc_sasi_device::device_start()
{
	// initialize timers
	m_sel_off_timer = timer_alloc(FUNC(asc_sasi_device::sel_off), this);
	m_rst_off_timer = timer_alloc(FUNC(asc_sasi_device::rst_off), this);

	// save state
	save_item(NAME(m_data_latch));
	save_item(NAME(m_boot));
}

void asc_sasi_device::device_reset()
{
	// POC causes RST pulse on SASI bus (unless jumper is removed)
	sasi_rst_pulse();

	// CLR sets flip-flop (IC10a)
	m_boot = true;
}

void asc_sasi_device::iio_w(int state)
{
	// Release data bus when I/O is asserted
	if (state)
		m_sasi->data_w(7, 0);
	else
		m_sasi->data_w(7, m_data_latch);
}

void asc_sasi_device::req_w(int state)
{
	// Clear ACK when REQ is negated
	if (!state)
		m_sasi->ctrl_w(7, 0, nscsi_device::S_ACK);
}

void asc_sasi_device::sasi_sel_pulse()
{
	m_sasi->ctrl_w(7, nscsi_device::S_SEL, nscsi_device::S_SEL);
	m_sel_off_timer->adjust(s_pulse_width);
}

void asc_sasi_device::sasi_rst_pulse()
{
	m_sasi->ctrl_w(7, nscsi_device::S_RST, nscsi_device::S_RST);
	m_rst_off_timer->adjust(s_pulse_width);
}

TIMER_CALLBACK_MEMBER(asc_sasi_device::sel_off)
{
	m_sasi->ctrl_w(7, 0, nscsi_device::S_SEL);
}

TIMER_CALLBACK_MEMBER(asc_sasi_device::rst_off)
{
	m_sasi->ctrl_w(7, 0, nscsi_device::S_RST);
}

u8 asc_sasi_device::s100_smemr_r(offs_t offset)
{
	u8 memsel = m_memsel->read();
	if (m_boot && BIT(memsel, 0) && (offset & 0xf800) == (memsel & 0x3e) << 10)
		return m_bootstrap[offset & 0x7ff];
	else
		return 0xff;
}

u8 asc_sasi_device::s100_sinp_r(offs_t offset)
{
	// Only two input ports are decoded
	if ((offset & 0xfe) == (m_iosel->read() & 0x3f) << 2)
	{
		if (BIT(offset, 0))
		{
			// 74LS240 buffer (IC7)
			u32 ctrl = m_sasi->ctrl_r();
			return (ctrl & nscsi_device::S_INP ? 0x01 : 0x00)
				| (ctrl & nscsi_device::S_BSY ? 0x04 : 0x00)
				| (ctrl & nscsi_device::S_REQ ? 0x10 : 0x00)
				| (ctrl & nscsi_device::S_CTL ? 0x80 : 0x00);
		}
		else
		{
			// INDAT: 74LS240 buffer (IC13)
			if (!machine().side_effects_disabled() && (m_sasi->ctrl_r() & nscsi_device::S_REQ))
				m_sasi->ctrl_w(7, nscsi_device::S_ACK, nscsi_device::S_ACK);
			return m_sasi->data_r();
		}
	}
	else
		return 0xff;
}

void asc_sasi_device::s100_sout_w(offs_t offset, u8 data)
{
	if ((offset & 0xfc) == (m_iosel->read() & 0x3f) << 2)
	{
		switch (offset & 0x03)
		{
		case 0:
			// LATENA: 74LS373 latch (IC8) outputs to 74LS240 buffer (IC9)
			if (!machine().side_effects_disabled() && (m_sasi->ctrl_r() & nscsi_device::S_REQ))
				m_sasi->ctrl_w(7, nscsi_device::S_ACK, nscsi_device::S_ACK);
			m_data_latch = data;
			if (!(m_sasi->ctrl_r() & nscsi_device::S_INP))
				m_sasi->data_w(7, data);
			break;

		case 1:
			sasi_sel_pulse();
			break;

		case 2:
			// MEMCON: clock DO5 into flip-flop (IC10a)
			m_boot = BIT(data, 5);
			break;

		case 3:
			sasi_rst_pulse();
			break;
		}
	}
}


static INPUT_PORTS_START(ascsasi)
	PORT_START("MEMSEL")
	PORT_DIPNAME(0x01, 0x01, "Bootstrap PROM")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x01, DEF_STR(On))
	PORT_DIPNAME(0x3e, 0x38, "PROM Address Decode") PORT_DIPLOCATION("SW1:2,3,4,5,6")
	PORT_DIPSETTING(0x00, "0000-07FF")
	PORT_DIPSETTING(0x02, "0800-0FFF")
	PORT_DIPSETTING(0x04, "1000-17FF")
	PORT_DIPSETTING(0x06, "1800-1FFF")
	PORT_DIPSETTING(0x08, "2000-27FF")
	PORT_DIPSETTING(0x0a, "2800-2FFF")
	PORT_DIPSETTING(0x0c, "3000-37FF")
	PORT_DIPSETTING(0x0e, "3800-3FFF")
	PORT_DIPSETTING(0x10, "4000-47FF")
	PORT_DIPSETTING(0x12, "4800-4FFF")
	PORT_DIPSETTING(0x14, "5000-57FF")
	PORT_DIPSETTING(0x16, "5800-5FFF")
	PORT_DIPSETTING(0x18, "6000-67FF")
	PORT_DIPSETTING(0x1a, "6800-6FFF")
	PORT_DIPSETTING(0x1c, "7000-77FF")
	PORT_DIPSETTING(0x1e, "7800-7FFF")
	PORT_DIPSETTING(0x20, "8000-87FF")
	PORT_DIPSETTING(0x22, "8800-8FFF")
	PORT_DIPSETTING(0x24, "9000-97FF")
	PORT_DIPSETTING(0x26, "9800-9FFF")
	PORT_DIPSETTING(0x28, "A000-A7FF")
	PORT_DIPSETTING(0x2a, "A800-AFFF")
	PORT_DIPSETTING(0x2c, "B000-B7FF")
	PORT_DIPSETTING(0x2e, "B800-BFFF")
	PORT_DIPSETTING(0x30, "C000-C7FF")
	PORT_DIPSETTING(0x32, "C800-CFFF")
	PORT_DIPSETTING(0x34, "D000-D7FF")
	PORT_DIPSETTING(0x36, "D800-DFFF")
	PORT_DIPSETTING(0x38, "E000-E7FF")
	PORT_DIPSETTING(0x3a, "E800-EFFF")
	PORT_DIPSETTING(0x3c, "F000-F7FF")
	PORT_DIPSETTING(0x3e, "F800-FFFF")
	PORT_DIPNAME(0x40, 0x40, "PHANTOM Signal") PORT_DIPLOCATION("SW1:7") // not emulated
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, "Memory Wait State") PORT_DIPLOCATION("SW1:8") // not emulated
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))

	PORT_START("IOSEL")
	PORT_DIPNAME(0x3f, 0x28, "I/O Address Decode") PORT_DIPLOCATION("SW2:1,2,3,4,5,6")
	PORT_DIPSETTING(0x00, "00-03")
	PORT_DIPSETTING(0x01, "04-07")
	PORT_DIPSETTING(0x02, "08-0B")
	PORT_DIPSETTING(0x03, "0C-0F")
	PORT_DIPSETTING(0x04, "10-13")
	PORT_DIPSETTING(0x05, "14-17")
	PORT_DIPSETTING(0x06, "18-1B")
	PORT_DIPSETTING(0x07, "1C-1F")
	PORT_DIPSETTING(0x08, "20-23")
	PORT_DIPSETTING(0x09, "24-27")
	PORT_DIPSETTING(0x0a, "28-2B")
	PORT_DIPSETTING(0x0b, "2C-2F")
	PORT_DIPSETTING(0x0c, "30-33")
	PORT_DIPSETTING(0x0d, "34-37")
	PORT_DIPSETTING(0x0e, "38-3B")
	PORT_DIPSETTING(0x0f, "3C-3F")
	PORT_DIPSETTING(0x10, "40-43")
	PORT_DIPSETTING(0x11, "44-47")
	PORT_DIPSETTING(0x12, "48-4B")
	PORT_DIPSETTING(0x13, "4C-4F")
	PORT_DIPSETTING(0x14, "50-53")
	PORT_DIPSETTING(0x15, "54-57")
	PORT_DIPSETTING(0x16, "58-5B")
	PORT_DIPSETTING(0x17, "5C-5F")
	PORT_DIPSETTING(0x18, "60-63")
	PORT_DIPSETTING(0x19, "64-67")
	PORT_DIPSETTING(0x1a, "68-6B")
	PORT_DIPSETTING(0x1b, "6C-6F")
	PORT_DIPSETTING(0x1c, "70-73")
	PORT_DIPSETTING(0x1d, "74-77")
	PORT_DIPSETTING(0x1e, "78-7B")
	PORT_DIPSETTING(0x1f, "7C-7F")
	PORT_DIPSETTING(0x20, "80-83")
	PORT_DIPSETTING(0x21, "84-87")
	PORT_DIPSETTING(0x22, "88-8B")
	PORT_DIPSETTING(0x23, "8C-8F")
	PORT_DIPSETTING(0x24, "90-93")
	PORT_DIPSETTING(0x25, "94-97")
	PORT_DIPSETTING(0x26, "98-9B")
	PORT_DIPSETTING(0x27, "9C-9F")
	PORT_DIPSETTING(0x28, "A0-A3")
	PORT_DIPSETTING(0x29, "A4-A7")
	PORT_DIPSETTING(0x2a, "A8-AB")
	PORT_DIPSETTING(0x2b, "AC-AF")
	PORT_DIPSETTING(0x2c, "B0-B3")
	PORT_DIPSETTING(0x2d, "B4-B7")
	PORT_DIPSETTING(0x2e, "B8-BB")
	PORT_DIPSETTING(0x2f, "BC-BF")
	PORT_DIPSETTING(0x30, "C0-C3")
	PORT_DIPSETTING(0x31, "C4-C7")
	PORT_DIPSETTING(0x32, "C8-CB")
	PORT_DIPSETTING(0x33, "CC-CF")
	PORT_DIPSETTING(0x34, "D0-D3")
	PORT_DIPSETTING(0x35, "D4-D7")
	PORT_DIPSETTING(0x36, "D8-DB")
	PORT_DIPSETTING(0x37, "DC-DF")
	PORT_DIPSETTING(0x38, "E0-E3")
	PORT_DIPSETTING(0x39, "E4-E7")
	PORT_DIPSETTING(0x3a, "E8-EB")
	PORT_DIPSETTING(0x3b, "EC-EF")
	PORT_DIPSETTING(0x3c, "F0-F3")
	PORT_DIPSETTING(0x3d, "F4-F7")
	PORT_DIPSETTING(0x3e, "F8-FB")
	PORT_DIPSETTING(0x3f, "FC-FF")
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unused)) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(0x80, DEF_STR(Off))
	PORT_DIPSETTING(0x00, DEF_STR(On))
INPUT_PORTS_END


ioport_constructor asc_sasi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ascsasi);
}

void asc_sasi_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, m_sasi);
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "sasi:7", default_scsi_devices, "sasicb", true)
		.option_add_internal("sasicb", NSCSI_CB)
		.machine_config([this] (device_t *device) {
			downcast<nscsi_callback_device &>(*device).io_callback().set(*this, FUNC(asc_sasi_device::iio_w));
			downcast<nscsi_callback_device &>(*device).req_callback().set(*this, FUNC(asc_sasi_device::req_w));
		});
}


ROM_START(ascsasi)
	ROM_REGION(0x800, "bootstrap", 0)
	ROM_LOAD("asc_sasi.bin", 0x000, 0x800, CRC(aac84077) SHA1(e94b39875e29daff199d120c4e79dfb1adbedab2)) // MODEL 1 REV. 2
ROM_END

const tiny_rom_entry *asc_sasi_device::device_rom_region() const
{
	return ROM_NAME(ascsasi);
}

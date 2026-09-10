// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * IBM Fixed Disk and Diskette Drive Adapter
 *
 * Sources:
 *  - IBM RT PC Hardware Technical Reference, Volume II (75X0235), Second Edition (September 1986), International Business Machines Corporation.
 *
 * TODO:
 *  - alternate I/O and interrupt
 *  - diagnose a6/05 error on RT PC
 */

/*
 * Notes
 * -----
 * Rev 1
 * U18 WD1014   error detection/support logic device
 * U31 WD1015   buffer manager control processor (8049P (2K ROM)
 * U24 WD1010A  winchester disk controller
 *
 * U30 UPD765
 *
 * OSC 24MHz - 4.8MHz, 8MHz
 * OSC 10MHz - 10MHz
 *
 * Rev 2
 * U11 WD11C00-22
 * U1  WD2010B
 * U2  WD1015
 * U5  2016?
 *
 */

#include "emu.h"
#include "fddda.h"

#include "machine/input_merger.h"
#include "machine/upd765.h"

#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"

#define LOG_REGR (1U << 1)
#define LOG_REGW (1U << 2)
#define LOG_DATA (1U << 3)

//#define VERBOSE (LOG_GENERAL|LOG_REGR|LOG_REGW)
#include "logmacro.h"

namespace {

enum hdc_status_mask : u8
{
	HDC_STATUS_ERROR = 0x01, // error
	HDC_STATUS_INDEX = 0x02, // index
	HDC_STATUS_CORRD = 0x04, // corrected data
	HDC_STATUS_HDRQ  = 0x08, // data request
	HDC_STATUS_SC    = 0x10, // seek complete
	HDC_STATUS_WF    = 0x20, // write fault
	HDC_STATUS_RDY   = 0x40, // drive ready
	HDC_STATUS_BUSY  = 0x80, // busy
};

enum hdc_error_mask : u8
{
	// diagnostic mode
	HDC_DIAG_NONE       = 1, // no errors
	HDC_DIAG_CONTROLLER = 2, // controller error
	HDC_DIAG_BUFFER     = 3, // sector buffer error
	HDC_DIAG_ECC        = 4, // ecc device error
	HDC_DIAG_CONTROL    = 5, // control processor error

	// operational errors
	HDC_ERROR_DAM   = 0x01, // data address mark not found
	HDC_ERROR_TR000 = 0x02, // track 0 error
	HDC_ERROR_AC    = 0x04, // aborted command
	HDC_ERROR_IDNF  = 0x10, // id not found
	HDC_ERROR_ECC   = 0x40, // data ecc error
	HDC_ERROR_BAD   = 0x80, // bad block detect
};

enum sdh_mask : u8
{
	SDH_DS = 0x10, // drive select
	SDH_HS = 0x0f, // head select
};

enum hdc_state : int
{
	STATE_READ   = 0, // read sector
	STATE_WRITE  = 1, // write sector
	STATE_FORMAT = 2, // format track
	STATE_VERIFY = 3, // read verify
	STATE_RESET  = 4, // reset complete
};

static constexpr attotime delay = attotime::from_msec(10);

class isa16_fddda_device
	: public device_t
	, public device_isa16_card_interface
{
public:
	isa16_fddda_device(machine_config const &mconfig, char const *const tag, device_t *owner, u32 clock)
		: device_t(mconfig, ISA16_FDDDA, tag, owner, clock)
		, device_isa16_card_interface(mconfig, *this)
		, m_hdd(*this, "hdd%u", 0U)
		, m_fdc(*this, "fdc")
		, m_flc(*this, "fdc:%u", 0U)
		, m_fdc_irq(*this, "fdc_irq")
		, m_fdc_drq(*this, "fdc_drq")
		, m_hdc_irq(*this, "hdc_irq")
		, m_hdc_drive{ { 7, 0, 0xf }, { 7, 0, 0xf } }
	{
	}

	static constexpr feature_type imperfect_features() { return feature::DISK; }

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	void hdc_map_pio(address_map &map);
	void fdc_map_pio(address_map &map);

private:
	void fdr_w(u8 data);

	void hdc_reset();

	u16 hdc_data_r(offs_t offset, u16 mem_mask);
	u8 hdc_error_r();
	u8 hdc_count_r();
	u8 hdc_sector_r();
	u16 hdc_cylinder_r();
	u8 hdc_sdh_r();
	u8 hdc_status_r();

	void hdc_data_w(offs_t offset, u16 data, u16 mem_mask);
	void hdc_pcmp_w(u8 data);
	void hdc_count_w(u8 data);
	void hdc_sector_w(u8 data);
	void hdc_cylinder_w(offs_t offset, u16 data, u16 mem_mask);
	void hdc_sdh_w(u8 data);
	void hdc_command_w(u8 data);

	void hdc_irq_w(bool state);
	void hdc_timer(s32 param);

	// diskette stuff
	virtual u8 dack_r(int line) override { return m_fdc->dma_r(); }
	virtual void dack_w(int line, u8 data) override { return m_fdc->dma_w(data); }
	//virtual void dack_line_w(int line, int state) override;
	virtual void eop_w(int state) override { m_fdc->tc_w(state); }

	void dor_w(u8 data);
	void dcr_w(u8 data);
	u8 dir_r();

	required_device_array<harddisk_image_device, 2> m_hdd;

	required_device<upd765a_device> m_fdc;
	required_device_array<floppy_connector, 2> m_flc;
	required_device<input_merger_all_high_device> m_fdc_irq;
	required_device<input_merger_all_high_device> m_fdc_drq;
	required_device<input_merger_all_high_device> m_hdc_irq;

	u8 m_dor;
	u8 m_fdr;

	emu_timer *m_timer;

	struct drive_config
	{
		u8 hpc; // configured heads per cylinder (0=16)
		u8 spt; // configured sectors per track (0=256)

		u8 rate; // current stepping rate
	}
	m_hdc_drive[2];
	u8 m_hdc_error;
	u8 m_hdc_count;
	u8 m_hdc_sector;
	u16 m_hdc_cylinder;
	u8 m_hdc_sdh;
	u8 m_hdc_status;

	u16 m_hdc_buf[256];
	u8 m_hdc_buf_len;
	bool m_hdc_irq_state;
};

static void drive_options(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void isa16_fddda_device::device_add_mconfig(machine_config &config)
{
	// fixed disk section
	HARDDISK(config, m_hdd[0]);
	HARDDISK(config, m_hdd[1]);

	// diskette drive section
	UPD765A(config, m_fdc, 24_MHz_XTAL / 3, false, false);
	m_fdc->intrq_wr_callback().set(m_fdc_irq, FUNC(input_merger_all_high_device::in_w<1>));
	m_fdc->drq_wr_callback().set(m_fdc_drq, FUNC(input_merger_all_high_device::in_w<1>));

	FLOPPY_CONNECTOR(config, m_flc[0], drive_options, "525hd", floppy_image_device::default_pc_floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_flc[1], drive_options, nullptr, floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	INPUT_MERGER_ALL_HIGH(config, m_fdc_irq);
	m_fdc_irq->output_handler().set([this](int state) { m_isa->irq6_w(state); });

	INPUT_MERGER_ALL_HIGH(config, m_fdc_drq);
	m_fdc_drq->output_handler().set([this](int state) { m_isa->drq2_w(state); });

	INPUT_MERGER_ALL_HIGH(config, m_hdc_irq);
	m_hdc_irq->output_handler().set([this](int state) { m_isa->irq14_w(state); });
}

void isa16_fddda_device::device_start()
{
	save_item(NAME(m_dor));
	save_item(NAME(m_fdr));

	save_item(NAME(m_hdc_error));
	save_item(NAME(m_hdc_count));
	save_item(NAME(m_hdc_sector));
	save_item(NAME(m_hdc_cylinder));
	save_item(NAME(m_hdc_sdh));
	save_item(NAME(m_hdc_status));

	save_item(NAME(m_hdc_buf));
	save_item(NAME(m_hdc_buf_len));

	m_timer = timer_alloc(FUNC(isa16_fddda_device::hdc_timer), this);

	set_isa_device();

	m_isa->install_device(0x03f0, 0x03f7, *this, &isa16_fddda_device::fdc_map_pio);
	m_isa->set_dma_channel(2, this, true);

	// hardware only decodes 10 address lines (RT PC firmware depends on it)
	m_isa->install_device(0x01f0, 0x01f7, *this, &isa16_fddda_device::hdc_map_pio);
	m_isa->install_device(0x05f0, 0x05f7, *this, &isa16_fddda_device::hdc_map_pio);

	std::fill_n(m_hdc_buf, std::size(m_hdc_buf), 0);
	m_hdc_buf_len = 0;
	m_hdc_irq_state = false;
}

void isa16_fddda_device::device_reset()
{
	m_dor = 0;
	m_fdr = 0;

	// disable interrupts and dma
	m_fdc_irq->in_w<0>(0);
	m_fdc_drq->in_w<0>(0);
	m_hdc_irq->in_w<0>(0);

	// select diskette 0
	m_fdc->set_floppy(m_flc[0]->get_device());

	// stop diskette motors
	if (m_flc[0]->get_device())
		m_flc[0]->get_device()->mon_w(1);
	if (m_flc[1]->get_device())
		m_flc[1]->get_device()->mon_w(1);

	hdc_reset();
}

void isa16_fddda_device::hdc_reset()
{
	m_hdc_error = 0;
	m_hdc_count = 0;
	m_hdc_sector = 0;
	m_hdc_cylinder = 0;
	m_hdc_sdh = 0;
	m_hdc_status = HDC_STATUS_BUSY;

	hdc_irq_w(false);

	// start diagnostic
	m_timer->adjust(delay * 10, STATE_RESET);
}

void isa16_fddda_device::hdc_map_pio(address_map &map)
{
	map(0, 1).rw(FUNC(isa16_fddda_device::hdc_data_r), FUNC(isa16_fddda_device::hdc_data_w)).flags(0x0001);
	map(2, 2).rw(FUNC(isa16_fddda_device::hdc_count_r), FUNC(isa16_fddda_device::hdc_count_w));
	map(3, 3).rw(FUNC(isa16_fddda_device::hdc_sector_r), FUNC(isa16_fddda_device::hdc_sector_w));
	map(4, 5).rw(FUNC(isa16_fddda_device::hdc_cylinder_r), FUNC(isa16_fddda_device::hdc_cylinder_w));
	map(6, 6).rw(FUNC(isa16_fddda_device::hdc_sdh_r), FUNC(isa16_fddda_device::hdc_sdh_w));
	map(7, 7).rw(FUNC(isa16_fddda_device::hdc_status_r), FUNC(isa16_fddda_device::hdc_command_w));
}

void isa16_fddda_device::fdc_map_pio(address_map &map)
{
	map(0x2, 0x2).w(FUNC(isa16_fddda_device::dor_w));
	map(0x4, 0x5).m(m_fdc, FUNC(upd765a_device::map));
	map(0x6, 0x6).w(FUNC(isa16_fddda_device::fdr_w));
	map(0x7, 0x7).rw(FUNC(isa16_fddda_device::dir_r), FUNC(isa16_fddda_device::dcr_w));
}

/*
 * U44 LS175 FDR
 *  0  N/C
 *  1  N/C
 *  2  N/C
 *  3  HDMAEN  (enable fixed disk interrupts)
 *  4  N/C
 *  5  HRST-   (reset fixed disk function)
 *  6  HS3     (enable head select 3)
 *  7  HS3-
 *
 * bit  function
 *  0   reserved
 *  1   enable fixed disk interrupts
 *  2   enable reset fixed disk function
 *  3   enable head select 3
 * 4-7  not connected
 */
void isa16_fddda_device::fdr_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: fdr_w 0x%02x\n", machine().describe_context(), data);
	m_hdc_irq->in_w<0>(!BIT(data, 1));

	// TODO: bit 2 goes to wd1014 MR-, wd1015 RESET- and wd1010 MR-
	if (BIT(data, 2))
		hdc_reset();

	m_fdr = data;
}

/*
 *
 * U26 LS174 DOR
 *  0 FDSEL
 *  1 N/C
 *  2 FRST-
 *  3 FDMAEN
 *  4 MOEN1
 *  5 MOEN2
 *
 * bit  function
 *  0   drive select (0==drive A)
 *  1   always 0
 *  2   diskette function reset
 *  3   enable diskette interrupts and dma
 *  4   drive a motor enable
 *  5   drive b motor enable
 *  6   reserved
 *  7   reserved
 */
void isa16_fddda_device::dor_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: dor_w 0x%02x\n", machine().describe_context(), data);

	// diskette drive select
	if (BIT(data ^ m_dor, 0))
		m_fdc->set_floppy(m_flc[BIT(data, 0)]->get_device());

	// diskette motor enables
	if (BIT(data ^ m_dor, 4) && m_flc[0]->get_device())
		m_flc[0]->get_device()->mon_w(!BIT(data, 4));
	if (BIT(data ^ m_dor, 5) && m_flc[1]->get_device())
		m_flc[1]->get_device()->mon_w(!BIT(data, 5));

	// diskette irq and drq enable
	if (BIT(data ^ m_dor, 3))
	{
		m_fdc_irq->in_w<0>(BIT(data, 3));
		m_fdc_drq->in_w<0>(BIT(data, 3));
	}

	// diskette function reset
	if ((BIT(data ^ m_dor, 2)))
		m_fdc->reset_w(!BIT(data, 2));

	m_dor = data;
}

void isa16_fddda_device::dcr_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: dcr_w 0x%02x\n", machine().describe_context(), data);

	static const int rates[4] = { 500'000, 300'000, 250'000, 125'000 };

	m_fdc->set_rate(rates[BIT(data, 0, 2)]);
}

/*
 *
 * U25 LS373 DIR
 *  0 DS0-
 *  1 DS1-
 *  2 HS0-
 *  3 HS1-
 *  4 HS2-
 *  5 HS3-/RWC
 *  6 WG-
 *  7 DCHG
 *
 * bit  function
 *  0   drive select 0
 *  1   drive select 1
 *  2   head select 0
 *  3   head select 1
 *  4   head select 2
 *  5   head select 3/reduced write current
 *  6   write gate
 *  7   diskette change
 *
 * bits 0-6 apply to currently selected fixed disk drive (valid for 50us after write to drive head register)
 * bits 0-5 all sensed directly from drive wd1014, bit 6 from wd1010
 */
u8 isa16_fddda_device::dir_r()
{
	u8 data = 0;

	// 1014 2 hd   -> 0 ds0-
	// 1014 1 dsb1 -> 1 ds1-
	// 1014 36 sdh0 -> 2 hs0-
	//      37 sdh1 -> 3 hs1-
	//      38 sdh2 -> 4 hs2-
	//      39 dsb2 -> 5 hs3-

	// 0  - drive 0, set bit 1
	// 1  - drive 1
	// 2  - drive 0, set bit 1
	// 3  - floppy, set bit 0

	switch (BIT(m_hdc_sdh, 3, 2))
	{
	case 0: data |= 0x02; break; // hdd 0
	case 1: data |= 0x20; break; // hdd 1
	case 2: data |= 0x02; break; // hdd 0
	case 3: data |= 0x01; break; // fdd
	}

	data |= BIT(~m_hdc_sdh, 0, 3) << 2;

	floppy_image_device *const fld = m_flc[BIT(m_dor, 0)]->get_device();
	if (fld && !fld->dskchg_r())
		data |= 0x80;

	return data;
}

u16 isa16_fddda_device::hdc_data_r(offs_t offset, u16 mem_mask)
{
	u16 data = 0;

	if (mem_mask == 0xffffU)
	{
		m_hdc_buf_len--;

		data = m_hdc_buf[255 - m_hdc_buf_len];

		LOGMASKED(LOG_DATA, "%s: hdc_data_r 0x%04x\n", machine().describe_context(), data);

		if (m_hdc_buf_len == 0)
		{
			m_hdc_status &= ~HDC_STATUS_HDRQ;

			// read next sector
			if (m_hdc_count)
				m_timer->adjust(delay);
		}
	}
	else if (mem_mask == 0xff00U)
		return hdc_error_r();
	else
		LOGMASKED(LOG_REGR, "%s: hdc_data_r unknown read offset 0x%x mask 0x%04x\n", machine().describe_context(), offset, mem_mask);

	return data;
}
u8 isa16_fddda_device::hdc_error_r()
{
	LOGMASKED(LOG_REGR, "%s: hdc_error_r 0x%02x\n", machine().describe_context(), m_hdc_error);

	return m_hdc_error;
}
u8 isa16_fddda_device::hdc_count_r()
{
	LOGMASKED(LOG_REGR, "%s: hdc_count_r 0x%02x\n", machine().describe_context(), m_hdc_count);

	return m_hdc_count;
}
u8 isa16_fddda_device::hdc_sector_r()
{
	LOGMASKED(LOG_REGR, "%s: hdc_sector_r 0x%02x\n", machine().describe_context(), m_hdc_sector);

	return m_hdc_sector;
}
u16 isa16_fddda_device::hdc_cylinder_r()
{
	LOGMASKED(LOG_REGR, "%s: hdc_cylinder_r 0x%04x\n", machine().describe_context(), m_hdc_cylinder);

	return m_hdc_cylinder;
}
u8 isa16_fddda_device::hdc_sdh_r()
{
	LOGMASKED(LOG_REGR, "%s: hdc_sdh_r 0x%02x\n", machine().describe_context(), m_hdc_sdh);

	return m_hdc_sdh;
}
u8 isa16_fddda_device::hdc_status_r()
{
	//LOGMASKED(LOG_REGR, "%s: hdc_status_r 0x%02x\n", machine().describe_context(), m_hdc_status);

	hdc_irq_w(false);

	//m_hdc_status ^= HDC_STATUS_INDEX;

	if (!m_hdd[BIT(m_hdc_sdh, 4)]->exists())
		return HDC_STATUS_ERROR;

	return m_hdc_status;
}

void isa16_fddda_device::hdc_data_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (mem_mask == 0xffffU)
	{
		LOGMASKED(LOG_DATA, "%s: hdc_data_w 0x%04x buffer %u\n", machine().describe_context(), data, m_hdc_buf_len);

		m_hdc_buf[m_hdc_buf_len++] = data;

		if (m_hdc_buf_len == 0)
		{
			m_hdc_status &= ~HDC_STATUS_HDRQ;
			m_hdc_status |= HDC_STATUS_BUSY;

			m_timer->reset(attotime::zero);
		}
	}
	else if (mem_mask == 0xff00U)
		hdc_pcmp_w(data >> 8);
	else
		LOGMASKED(LOG_REGW, "%s: hdc_data_w unknown write offset 0x%x data 0x%04 mask 0x%04x\n", machine().describe_context(), offset, data, mem_mask);
}
void isa16_fddda_device::hdc_pcmp_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: hdc_pcmp_w 0x%02x\n", machine().describe_context(), data);
}
void isa16_fddda_device::hdc_count_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: hdc_count_w 0x%02x\n", machine().describe_context(), data);

	m_hdc_count = data;
}
void isa16_fddda_device::hdc_sector_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: hdc_sector_w 0x%02x\n", machine().describe_context(), data);

	m_hdc_sector = data;
}
void isa16_fddda_device::hdc_cylinder_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_REGW, "%s: hdc_cylinder_w 0x%04x mask 0x%04x\n", machine().describe_context(), data, mem_mask);

	COMBINE_DATA(&m_hdc_cylinder);

	m_hdc_cylinder &= 0x03ff;
}
void isa16_fddda_device::hdc_sdh_w(u8 data)
{
	LOGMASKED(LOG_REGW, "%s: hdc_sdh_w 0x%02x\n", machine().describe_context(), data);

	m_hdc_sdh = data;
}

void isa16_fddda_device::hdc_command_w(u8 data)
{
	static const char *const rates[] =
	{
		 "35 us", "0.5 ms", "1.0 ms", "1.5 ms",
		"2.0 ms", "2.5 ms", "3.0 ms", "3.5 ms",
		"4.0 ms", "4.5 ms", "5.0 ms", "5.5 ms",
		"6.0 ms", "6.5 ms", "7.0 ms", "7.5 ms",
	};

	LOGMASKED(LOG_REGW, "%s: hdc_command_w 0x%02x\n", machine().describe_context(), data);

	if ((m_hdc_status & HDC_STATUS_WF) || !(m_hdc_status & HDC_STATUS_RDY) || !(m_hdc_status & HDC_STATUS_SC))
	  LOG("cannot execute command status 0x%02x\n", m_hdc_status);

	m_hdc_status &= ~HDC_STATUS_ERROR;
	m_hdc_error = 0;
	m_timer->reset();
	hdc_irq_w(false);

	drive_config &config = m_hdc_drive[BIT(m_hdc_sdh, 4)];

	switch (data & 0xf0)
	{
	case 0x10:
		LOG("drive %u: restore (rate=%s)\n", BIT(m_hdc_sdh, 4), rates[BIT(data, 0, 4)]);
		config.rate = BIT(data, 0, 4);
		m_hdc_status |= HDC_STATUS_SC;
		hdc_irq_w(true);
		break;
	case 0x70:
		LOG("drive %u: seek (cylinder=%u, rate=%s)\n", BIT(m_hdc_sdh, 4), m_hdc_cylinder, rates[BIT(data, 0, 4)]);
		config.rate = BIT(data, 0, 4);
		m_hdc_status |= HDC_STATUS_SC;
		hdc_irq_w(true);
		break;
	case 0x20:
		LOG("drive %u: read sector\n", BIT(m_hdc_sdh, 4));

		if (false) //(BIT(m_hdc_sdh, 0, 4) > config.hpc) || (config.spt && m_hdc_sector > config.spt))
		{
			LOG("drive %u: read sector aborted\n", BIT(m_hdc_sdh, 4));
			m_hdc_error = HDC_ERROR_IDNF;
			m_hdc_status |= HDC_STATUS_ERROR;
			hdc_irq_w(true);
		}
		else
		{
			m_hdc_status |= HDC_STATUS_BUSY;
			m_timer->adjust(delay, STATE_READ);
		}
		break;
	case 0x30:
		LOG("drive %u: write sector\n", BIT(m_hdc_sdh, 4));

		if (false) //(BIT(m_hdc_sdh, 0, 4) > config.hpc) || (config.spt && m_hdc_sector > config.spt))
		{
			LOG("drive %u: write sector aborted\n", BIT(m_hdc_sdh, 4));
			m_hdc_error = HDC_ERROR_IDNF;
			m_hdc_status |= HDC_STATUS_ERROR;
			hdc_irq_w(true);
		}
		else
		{
			// indicate buffer ready
			m_hdc_status |= HDC_STATUS_HDRQ;
			m_hdc_buf_len = 0;

			m_timer->set_param(STATE_WRITE);
		}
		break;
	case 0x50:
		LOG("drive %u: format track\n", BIT(m_hdc_sdh, 4));

		// indicate buffer ready
		m_hdc_status |= HDC_STATUS_HDRQ;
		m_hdc_buf_len = 0;

		m_timer->set_param(STATE_FORMAT);
		break;
	case 0x40:
		LOG("drive %u: read verify\n", BIT(m_hdc_sdh, 4));

		m_hdc_status |= HDC_STATUS_BUSY;
		m_timer->adjust(delay, STATE_VERIFY);
		break;
	case 0x90:
		if (!BIT(data, 0))
		{
			LOG("diagnose\n");
			m_hdc_error = HDC_DIAG_NONE;
			config.rate = 0xf;
		}
		else
		{
			LOG("drive %u: set parameters (heads=%u, sectors=%u)\n", BIT(m_hdc_sdh, 4), BIT(m_hdc_sdh, 0, 4), m_hdc_count ? m_hdc_count : 256U);
			config.hpc = BIT(m_hdc_sdh, 0, 4);
			config.spt = m_hdc_count;
		}
		hdc_irq_w(true);
		break;
	default:
		LOG("aborted command 0x%02x\n", data);
		m_hdc_error = HDC_ERROR_AC;
		m_hdc_status |= HDC_STATUS_ERROR;
		break;
	}
}

void isa16_fddda_device::hdc_irq_w(bool state)
{
	if (m_hdc_irq_state != state)
	{
		LOG("hdc_irq %u\n", state);
		m_hdc_irq_state = state;
		m_hdc_irq->in_w<1>(state);
	}
}

void isa16_fddda_device::hdc_timer(s32 param)
{
	switch (param)
	{
	case STATE_READ:
		LOG("drive %u: read sector (cylinder=%u, head=%u, sector=%u, count=%u)\n",
			BIT(m_hdc_sdh, 4), m_hdc_cylinder, BIT(m_hdc_sdh, 0, 4), m_hdc_sector, m_hdc_count);

		// read buffer from disk
		if (harddisk_image_device *hdd = m_hdd[BIT(m_hdc_sdh, 4)])
		{
			hard_disk_file::info const &i = hdd->get_info();

			hdd->read(((m_hdc_cylinder * i.heads) + BIT(m_hdc_sdh, 0, 4)) * i.sectors + m_hdc_sector - 1, m_hdc_buf);
		}

		// advance sector, head, cylinder
		if (m_hdc_sector == m_hdc_drive[BIT(m_hdc_sdh, 4)].spt)
		{
			m_hdc_sector = 1;
			if (BIT(m_hdc_sdh, 0, 4) == m_hdc_drive[BIT(m_hdc_sdh, 4)].hpc)
			{
				m_hdc_sdh &= ~SDH_HS;
				m_hdc_cylinder = (m_hdc_cylinder + 1) & 0x03ffU;
			}
			else
				m_hdc_sdh++;
		}
		else
			m_hdc_sector++;

		// decrement sector count
		m_hdc_count--;

		// indicate buffer ready
		m_hdc_status |= HDC_STATUS_HDRQ;
		m_hdc_buf_len = 0;

		m_hdc_status &= ~HDC_STATUS_BUSY;
		hdc_irq_w(true);
		break;

	case STATE_WRITE:
		LOG("drive %u: write sector (cylinder=%u, head=%u, sector=%u, count=%u)\n",
			BIT(m_hdc_sdh, 4), m_hdc_cylinder, BIT(m_hdc_sdh, 0, 4), m_hdc_sector, m_hdc_count);

		// write buffer to disk
		if (harddisk_image_device *hdd = m_hdd[BIT(m_hdc_sdh, 4)])
		{
			hard_disk_file::info const &i = hdd->get_info();

			hdd->write(((m_hdc_cylinder * i.heads) + BIT(m_hdc_sdh, 0, 4)) * i.sectors + m_hdc_sector - 1, m_hdc_buf);
		}

		// advance sector, head, cylinder
		if (m_hdc_sector == m_hdc_drive[BIT(m_hdc_sdh, 4)].spt)
		{
			m_hdc_sector = 1;
			if (BIT(m_hdc_sdh, 0, 4) == m_hdc_drive[BIT(m_hdc_sdh, 4)].hpc)
			{
				m_hdc_sdh &= ~SDH_HS;
				m_hdc_cylinder = (m_hdc_cylinder + 1) & 0x03ffU;
			}
			else
				m_hdc_sdh++;
		}
		else
			m_hdc_sector++;

		// decrement sector count
		m_hdc_count--;
		if (m_hdc_count)
		{
			// indicate buffer ready
			m_hdc_status |= HDC_STATUS_HDRQ;
			m_hdc_buf_len = 0;
		}

		m_hdc_status &= ~HDC_STATUS_BUSY;
		hdc_irq_w(true);
		break;

	case STATE_FORMAT:
		LOG("drive %u: format track (cylinder=%u, head=%u, count=%u)\n",
			BIT(m_hdc_sdh, 4), m_hdc_cylinder, BIT(m_hdc_sdh, 0, 4), m_hdc_count);

		// implied seek
		m_hdc_status |= HDC_STATUS_SC;
#if 0
		if (BIT(m_hdc_sdh, 0, 4) == m_hdc_drive[BIT(m_hdc_sdh, 4)].hpc)
		{
			m_hdc_sdh &= ~SDH_HS;
			m_hdc_cylinder = (m_hdc_cylinder + 1) & 0x03ffU;
		}
		else
			m_hdc_sdh++;
#endif

		m_hdc_cylinder = 0;
		m_hdc_sdh &= ~SDH_HS;

		m_hdc_count = 0;

		m_timer->set_param(-1);
		m_hdc_status &= ~HDC_STATUS_BUSY;
		//m_hdc_status |= HDC_STATUS_HDRQ; // FIXME: why?

		hdc_irq_w(true);
		break;

	case STATE_VERIFY:
		LOG("drive %u: read verify (cylinder=%u, head=%u, sector=%u, count=%u)\n",
			BIT(m_hdc_sdh, 4), m_hdc_cylinder, BIT(m_hdc_sdh, 0, 4), m_hdc_sector, m_hdc_count);

		// advance sector, head, cylinder
		if (m_hdc_sector == m_hdc_drive[BIT(m_hdc_sdh, 4)].spt)
		{
			m_hdc_sector = 1;
			if (BIT(m_hdc_sdh, 0, 4) == m_hdc_drive[BIT(m_hdc_sdh, 4)].hpc)
			{
				m_hdc_sdh &= ~SDH_HS;
				m_hdc_cylinder = (m_hdc_cylinder + 1) & 0x03ffU;
			}
			else
				m_hdc_sdh++;
		}
		else
			m_hdc_sector++;

		// decrement sector count
		m_hdc_count--;

		m_hdc_status &= ~HDC_STATUS_BUSY;
		hdc_irq_w(true);
		break;

	case STATE_RESET:
		m_hdc_error = HDC_DIAG_NONE;
		m_hdc_status = HDC_STATUS_RDY | HDC_STATUS_SC;
		break;
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ISA16_FDDDA, device_isa16_card_interface, isa16_fddda_device, "fddda", "IBM Fixed Disk and Diskette Drive Adapter")

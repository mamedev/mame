// license:BSD-3-Clause
// copyright-holders: Sven Schnelle
/*********************************************************************

    hp9133.cpp

    HP9133 hard disk/floppy drive

*********************************************************************/

#include "emu.h"
#include "hp9133.h"

#include "ieee488.h"

#include "cpu/m6809/m6809.h"
#include "machine/i8291a.h"
#include "machine/wd2010.h"
#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"

#include <algorithm>

//#define VERBOSE 1
#include "logmacro.h"

namespace {

class hp9133_device : public device_t,
			  public device_ieee488_interface
{
public:
	// construction/destruction
	hp9133_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_ieee488_interface implementation
	virtual void ieee488_eoi(int state) override;
	virtual void ieee488_dav(int state) override;
	virtual void ieee488_nrfd(int state) override;
	virtual void ieee488_ndac(int state) override;
	virtual void ieee488_ifc(int state) override;
	virtual void ieee488_srq(int state) override;
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ren(int state) override;

private:
	static constexpr int IO1_R_BCS_N = 0x10;
	static constexpr int IO1_R_BDRQ = 0x20;
	static constexpr int IO1_R_HDINT = 0x40;
	static constexpr int IO1_R_HDIND = 0x80;

	static constexpr int IO2_R_FDCINT = 0x10;
	static constexpr int IO2_R_FDKDCH_N = 0x20;
	static constexpr int IO2_R_WRTFLTERR = 0x80;

	static constexpr int IO3_R_FASTDON = 0x20;
	static constexpr int IO3_R_SELTST = 0x40;
	static constexpr int IO3_R_TEST = 0x80;

	static constexpr int IO1_W_FLT_N = 0x01;
	static constexpr int IO1_W_DS1 = 0x02;
	static constexpr int IO1_W_BRDY = 0x40;
	static constexpr int IO1_W_WDRESET_N = 0x80;

	static constexpr int IO2_W_FDCRES_N = 0x01;
	static constexpr int IO2_W_CLRIND_N = 0x04;
	static constexpr int IO2_W_DKCHRES_N = 0x08;
	static constexpr int IO2_W_FDHDLD_N = 0x10;
	static constexpr int IO2_W_FDMOTON_N = 0x20;
	static constexpr int IO2_W_FDHDSEL_N = 0x40;

	static constexpr int IO3_W_INTSEL_MASK = 0x03;
	static constexpr int IO3_W_INTENBL_N = 0x04;
	static constexpr int IO3_W_FAST = 0x10;
	static constexpr int IO3_W_DMAR_W_N = 0x20;
	static constexpr int IO3_W_HPB_FDC_N = 0x40;
	static constexpr int IO3_W_DMA = 0x80;

	required_device<mc6809_device> m_cpu;
	required_device<i8291a_device> m_gpib;
	required_device<wd2010_device> m_hdc;
	required_device<harddisk_image_device> m_harddisk;
	required_device<wd2793_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_ioport m_hpib_addr;
	required_ioport m_selftest;
	required_ioport m_selftesten;
	required_ioport m_volcfg;
	required_ioport m_model;

	TIMER_CALLBACK_MEMBER(floppy_motor_timeout);
	TIMER_CALLBACK_MEMBER(index_timer);
	TIMER_CALLBACK_MEMBER(fast_timer);

	void i8291a_eoi_w(int state);
	void i8291a_dav_w(int state);
	void i8291a_nrfd_w(int state);
	void i8291a_ndac_w(int state);
	void i8291a_ifc_w(int state);
	void i8291a_srq_w(int state);
	void i8291a_atn_w(int state);
	void i8291a_ren_w(int state);
	void i8291a_dio_w(uint8_t data);
	void i8291a_int_w(int state);
	void i8291a_drq_w(int state);
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);
	void hdc_reg_w(offs_t offset, uint8_t data);
	uint8_t hdc_reg_r(offs_t offset);
	uint8_t i8291a_dio_r();
	void io1_w(uint8_t data);
	void io2_w(uint8_t data);
	void io3_w(uint8_t data);
	uint8_t io1_r();
	uint8_t io2_r();
	uint8_t io3_r();

	void dma_ram_w(uint8_t data);
	uint8_t dma_ram_r(offs_t offset);
	void cpu_map(address_map &map) ATTR_COLD;
	void floppy_index_cb(floppy_image_device *floppy, int state);
	void hdc_bcs_cb(int state);
	void hdc_bcr_cb(int state);
	void hdc_bdrq_cb(int state);
	void hdc_intrq_cb(int state);
	void hdc_wg_cb(int state);
	void hdc_readwrite_sector(bool write);
	void update_intsel();
	void cpu_sync_ack(int state);

	emu_timer *m_motor_timer;
	emu_timer *m_index_timer;
	emu_timer *m_fast_timer;
	uint8_t m_dma_ram[2048];
	int m_dma_addr;
	int m_intsel;
	int m_head;

	bool m_harddisk_index;
	bool m_floppy_index_int;
	bool m_intenable;
	bool m_dmaenable;
	bool m_dmaack_switch;
	bool m_dma_rwn;
	bool m_gpib_irq;
	bool m_gpib_drq;
	bool m_fdc_irq;
	bool m_fdc_drq;
	bool m_cpuirq;
	bool m_cpufirq;
	bool m_hdc_intrq;
	bool m_hdc_bdrq;
	bool m_hdc_bcs;
	bool m_hdc_wg;
	bool m_fast;

	uint16_t m_hdc_cylinder;
	uint8_t m_hdc_sdh;
	uint8_t m_hdc_sector_count;
	uint8_t m_hdc_sector_number;
	uint8_t m_hdc_cmd;
};

hp9133_device::hp9133_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t{mconfig, HP9133, tag, owner, clock},
	device_ieee488_interface{mconfig, *this},
	m_cpu{*this, "cpu"},
	m_gpib{*this, "i8291a"},
	m_hdc{*this, "wd2010"},
	m_harddisk{*this, "harddisk"},
	m_fdc{*this, "wd2793"},
	m_floppy{*this, "floppy0"},
	m_hpib_addr{*this, "ADDRESS"},
	m_selftest{*this, "SELFTEST"},
	m_selftesten{*this, "SELFTESTEN"},
	m_volcfg{*this, "VOLCFG"},
	m_model{*this, "MODEL"}
{
}

static INPUT_PORTS_START(hp9133_port)
	PORT_START("ADDRESS")
	PORT_CONFNAME(0xf, 0x00, "HPIB address")
	PORT_CONFSETTING(0, "0")
	PORT_CONFSETTING(1, "1")
	PORT_CONFSETTING(2, "2")
	PORT_CONFSETTING(3, "3")
	PORT_CONFSETTING(4, "4")
	PORT_CONFSETTING(5, "5")
	PORT_CONFSETTING(6, "6")
	PORT_CONFSETTING(7, "7")
	PORT_CONFSETTING(8, "8")
	PORT_CONFSETTING(9, "9")
	PORT_CONFSETTING(10, "10")
	PORT_CONFSETTING(11, "11")
	PORT_CONFSETTING(12, "12")
	PORT_CONFSETTING(13, "13")
	PORT_CONFSETTING(14, "14")
	PORT_CONFSETTING(15, "15")

	PORT_START("SELFTESTEN")
	PORT_CONFNAME(0x1, 0x00, "Selftest Enable")
	PORT_CONFSETTING(0x0, DEF_STR(Off))
	PORT_CONFSETTING(0x1, DEF_STR(On))

	PORT_START("SELFTEST")
	PORT_CONFNAME(0xf, 0x00, "Selftest")
	PORT_CONFSETTING(0x0, "RAM")
	PORT_CONFSETTING(0x1, "ROM checksum")
	PORT_CONFSETTING(0x2, "GPIB")
	PORT_CONFSETTING(0x3, "FDC Chip")
	PORT_CONFSETTING(0x4, "Floppy Seek")
	PORT_CONFSETTING(0x5, "Winchester Seek")
	PORT_CONFSETTING(0x6, "Floppy Speed")
	PORT_CONFSETTING(0x7, "Winchester Speed")
	PORT_CONFSETTING(0x8, "Floppy Write Verify")
	PORT_CONFSETTING(0x9, "Winchester Write Verify")
	PORT_CONFSETTING(0xa, "Floppy Verify")
	PORT_CONFSETTING(0xb, "Winchester Verify")
	PORT_CONFSETTING(0xc, "Floppy format")
	PORT_CONFSETTING(0xd, "HDC Check")
	PORT_CONFSETTING(0xe, "WD1100 Check")
	PORT_CONFSETTING(0xf, "WD1100 Buffer RAM")

	PORT_START("VOLCFG")
	PORT_CONFNAME(0x0f, 0x00, "Volume config")
	PORT_CONFSETTING(0, "0")
	PORT_CONFSETTING(1, "1")
	PORT_CONFSETTING(2, "2")
	PORT_CONFSETTING(3, "3")
	PORT_CONFSETTING(4, "4")
	PORT_CONFSETTING(5, "5")
	PORT_CONFSETTING(6, "6")
	PORT_CONFSETTING(7, "7")
	PORT_CONFSETTING(8, "8")
	PORT_CONFSETTING(9, "9")

	PORT_START("MODEL")
	PORT_CONFNAME(0x03, 0x00, "Disc Model")
	PORT_CONFSETTING(0x0, "9133D")
	PORT_CONFSETTING(0x1, "9133L")
	PORT_CONFSETTING(0x2, "9133H")
INPUT_PORTS_END

ioport_constructor hp9133_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp9133_port);
}

TIMER_CALLBACK_MEMBER(hp9133_device::floppy_motor_timeout)
{
	auto *floppy = m_floppy->get_device();

	floppy->mon_w(1);
}

void hp9133_device::device_start()
{
	save_item(NAME(m_dma_ram));
	save_item(NAME(m_dma_addr));
	save_item(NAME(m_head));
	save_item(NAME(m_intsel));
	save_item(NAME(m_harddisk_index));
	save_item(NAME(m_floppy_index_int));
	save_item(NAME(m_intenable));
	save_item(NAME(m_dmaenable));
	save_item(NAME(m_dmaack_switch));
	save_item(NAME(m_dma_rwn));
	save_item(NAME(m_gpib_irq));
	save_item(NAME(m_gpib_drq));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_cpuirq));
	save_item(NAME(m_cpufirq));
	save_item(NAME(m_hdc_intrq));
	save_item(NAME(m_hdc_bdrq));
	save_item(NAME(m_hdc_bcs));
	save_item(NAME(m_hdc_wg));
	save_item(NAME(m_fast));
	save_item(NAME(m_hdc_cylinder));
	save_item(NAME(m_hdc_sdh));
	save_item(NAME(m_hdc_sector_count));
	save_item(NAME(m_hdc_sector_number));
	save_item(NAME(m_hdc_cmd));

	// HP firmware tests whether ECC errors are detected. MAME's WD2010
	// doesn't emulate ECC so patch the ROM to ignore the ECC test result.
	uint8_t *rom = memregion("cpu")->base();
	rom[0x279e] = 0x12;
	rom[0x279f] = 0x12;
	// disable ROM checksum
	rom[0x36ea] = 0x20;

	m_motor_timer = timer_alloc(FUNC(hp9133_device::floppy_motor_timeout), this);
	m_index_timer = timer_alloc(FUNC(hp9133_device::index_timer), this);
	m_index_timer->adjust(attotime::from_hz(3600 / 60));
	m_fast_timer = timer_alloc(FUNC(hp9133_device::fast_timer), this);
	auto *floppy = m_floppy->get_device();
	floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&hp9133_device::floppy_index_cb, this));
}

void hp9133_device::device_reset()
{
	std::fill(std::begin(m_dma_ram), std::end(m_dma_ram), 0);
	m_dma_addr = 0;
	m_intsel = 0;
	m_head = 0;
	m_harddisk_index = false;
	m_floppy_index_int = false;
	m_intenable = false;
	m_dmaenable = false;
	m_dmaack_switch = false;
	m_dma_rwn = false;
	m_gpib_irq = false;
	m_gpib_drq = false;
	m_fdc_irq = false;
	m_fdc_drq = false;
	m_cpuirq = false;
	m_cpufirq = false;
	m_hdc_intrq = false;
	m_hdc_bdrq = false;
	m_hdc_bcs = false;
	m_hdc_wg = false;
	m_fast = false;
	m_fdc->set_floppy(m_floppy->get_device());
}

void hp9133_device::ieee488_eoi(int state)
{
	m_gpib->eoi_w(state);
}

void hp9133_device::ieee488_dav(int state)
{
	m_gpib->dav_w(state);
}

void hp9133_device::ieee488_nrfd(int state)
{
	m_gpib->nrfd_w(state);
}

void hp9133_device::ieee488_ndac(int state)
{
	m_gpib->ndac_w(state);
}

void hp9133_device::ieee488_srq(int state)
{
	m_gpib->srq_w(state);
}

void hp9133_device::ieee488_ifc(int state)
{
	m_gpib->ifc_w(state);
}

void hp9133_device::ieee488_atn(int state)
{
	m_gpib->atn_w(state);
}

void hp9133_device::ieee488_ren(int state)
{
	m_gpib->ren_w(state);
}

void hp9133_device::i8291a_eoi_w(int state)
{
	m_bus->eoi_w(this, state);
}

void hp9133_device::i8291a_dav_w(int state)
{
	m_bus->dav_w(this, state);
}

void hp9133_device::i8291a_nrfd_w(int state)
{
	m_bus->nrfd_w(this, state);
}

void hp9133_device::i8291a_ndac_w(int state)
{
	m_bus->ndac_w(this, state);
}

void hp9133_device::i8291a_srq_w(int state)
{
	m_bus->srq_w(this, state);
}

void hp9133_device::i8291a_int_w(int state)
{
	LOG("%s: %d\n", __func__, state);
	m_gpib_irq = state;
	update_intsel();
}

uint8_t hp9133_device::i8291a_dio_r()
{
	uint8_t ret = m_bus->dio_r();
	LOG("%s: %02x\n", __func__, (uint8_t)~ret);
	return ret;
}

void hp9133_device::i8291a_dio_w(uint8_t data)
{
	m_bus->dio_w(this, data);
}

void hp9133_device::i8291a_drq_w(int state)
{
	LOG("%s: %d\n", __func__, state);
	m_gpib_drq = state;
	update_intsel();
}

TIMER_CALLBACK_MEMBER(hp9133_device::index_timer)
{
	attotime next = attotime::from_hz(3600 / 60);

	m_harddisk_index ^= true;
	if (!m_harddisk_index)
		next /= 10;
	else
		next -= next / 10;
	m_index_timer->adjust(next);
}

void hp9133_device::cpu_sync_ack(int state)
{
	LOG("%s: %d: enabled %d\n", __func__, state, m_dmaenable);
	if (!m_dmaenable)
		return;
	if (state) {
		if (m_gpib_drq)
			m_cpu->set_input_line(M6809_IRQ_LINE, ASSERT_LINE);
	} else {
		uint8_t data;
		if (!m_dma_rwn) {
			data = dma_ram_r(0);
			m_gpib->dout_w(data);
		} else {
			data = m_gpib->din_r();
			dma_ram_w(data);
		}
		m_cpu->set_input_line(M6809_IRQ_LINE, CLEAR_LINE);
		LOG("%s: sync data: %02x\n", __func__, data);
		if (m_dma_addr > 256) {
			LOG("DMA: all bytes transferred\n");
			m_dmaenable = false;
		}
	}
}

void hp9133_device::update_intsel()
{
	bool irq = false, firq = false;

	// IRQ Multiplexing:
	// +--------+----------+-----------+-------------------+---------+
	// | INTSEL |    0     |      1    |          2        |    3    |
	// +--------+----------+-+---------+-------------------+---------+
	// | IRQ#   | GPIBDREQ |   GND     |     DMASYNC       | FDCDREQ |
	// | FIRQ#  | GPIBINT  | INDEXINT  | FDCINT || GPIBINT | FDCINT  |
	// +--------+---------+-----------+-------------------+----------+
	switch (m_intsel) {
	case 0:
		irq = m_gpib_drq;
		firq = m_gpib_irq;
		break;
	case 1:
		irq = false;
		firq = m_floppy_index_int;
		break;
	case 2:
		irq = (m_dmaenable && m_gpib_drq);
		firq = m_fdc_irq || m_gpib_irq;
		break;
	case 3:
		irq = m_fdc_drq;
		firq = m_fdc_irq;
		break;
	}

	if (!m_intenable) {
		irq = false;
		firq = false;
	}

	if (m_cpuirq != irq) {
		m_cpu->set_input_line(M6809_IRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);
		m_cpuirq = irq;
	}

	if (m_cpufirq != firq) {
		m_cpu->set_input_line(M6809_FIRQ_LINE, firq ? ASSERT_LINE : CLEAR_LINE);
		m_cpufirq = firq;
	}
}

TIMER_CALLBACK_MEMBER(hp9133_device::fast_timer)
{
	if (!m_fast)
		return;
	if (m_gpib_drq) {
		if (!m_dma_rwn)
			m_gpib->dout_w(dma_ram_r(0));
		else
			dma_ram_w(m_gpib->din_r());
	}
	m_fast_timer->adjust(attotime::from_usec(50));
}

void hp9133_device::io1_w(uint8_t data)
{
	// 7 - HDC Rst#
	// 6 - HDC BRDY
	// 5 - HD Precomp#
	// 4 - HD Head 2
	// 3 - HD Head 1
	// 2 - HD Head 0
	// 1 - Active LED
	// 0 - Fault LED#
	m_head = (data >> 2) & 7;
	LOG("%s: %02x Head %d %s%s%s%s\n", __func__,
			data, m_head,
			(data & IO1_W_BRDY) ? "BRDY " : "",
			(data & IO1_W_WDRESET_N) ? "" : "HDCRESET ",
			(data & IO1_W_DS1) ? "ACTIVE " : "",
			(data & IO1_W_FLT_N) ? "" : "FAULT ");
	if (!(data & IO1_W_WDRESET_N))
		m_hdc->reset();
	m_hdc->buffer_ready(data & IO1_W_BRDY);
}

void hp9133_device::io2_w(uint8_t data)
{
	auto *floppy = m_floppy->get_device();

	// 7 - HDRecMode
	// 6 - FlopHLO#?
	// 5 - FlopMotor#
	// 4 - FlopSide#
	// 3 - Floppy Disk change reset
	// 2 - ClrIdxInt
	// 1
	// 0 - FDCRst#
	LOG("%s: %02x\n", __func__, data);
	if (!(data & IO2_W_CLRIND_N))
		m_floppy_index_int = false;
	if (data & IO2_W_FDMOTON_N) {
		m_motor_timer->adjust(attotime::from_msec(2000));
	} else {
		m_motor_timer->reset();
		floppy->mon_w(0);
	}
	if (!(data & IO2_W_DKCHRES_N))
		m_floppy->get_device()->dskchg_w(false);
	m_floppy->get_device()->ss_w((data & IO2_W_FDHDSEL_N) ? 0 : 1);
	if (!(data & IO2_W_FDCRES_N))
		m_fdc->reset();
}

void hp9133_device::io3_w(uint8_t data)
{
	// 7 - DMA enable
	// 6 - 1 GPIPACK, 0 FDCACK
	// 5 - Write Precomp?
	// 4 - FAST enable?
	// 3 -
	// 2 - Interrupt Enable
	// 0:1 - Interrupt select

	m_dmaenable = data & IO3_W_DMA;
	m_dmaack_switch = data & IO3_W_HPB_FDC_N;
	m_dma_rwn = data & IO3_W_DMAR_W_N;
	m_intenable = !(data & IO3_W_INTENBL_N);
	m_fast = data & IO3_W_FAST;
	m_intsel = data & IO3_W_INTSEL_MASK;

	LOG("%s: %02x = INTSEL %d, INT %d DMA %d DMA start %d DMA ACK %s\n", __func__,
			data, m_intsel, m_intenable, m_dmaenable, BIT(data, 4),
			m_dmaack_switch ? "GPIB" : "FDC");
	update_intsel();
	if (m_fast)
		m_fast_timer->adjust(attotime::from_usec(1000));
	else
		m_fast_timer->adjust(attotime::never);
}

uint8_t hp9133_device::io1_r()
{
	uint8_t ret;

	// 7 - hdindex
	// 6 - HDC int
	// 5 - HDC DRQ
	// 4 - HDC BCS
	// 3 - 0 Ident
	ret = m_model->read() ^ 0xf;
	if (m_harddisk_index)
		ret |= IO1_R_HDIND;
	if (m_hdc_intrq)
		ret |= IO1_R_HDINT;
	if (m_hdc_bdrq)
		ret |= IO1_R_BDRQ;
	if (m_hdc_bcs)
		ret |= IO1_R_BCS_N;
	if (!machine().side_effects_disabled())
		LOG("%s: %02x\n", __func__, ret);
	return ret;
}

uint8_t hp9133_device::io2_r()
{
	uint8_t ret = 0;

	// 7 - Write Fault Latch
	// 5 - FlopDC#
	// 4 - FDC interrupt
	// 3 - 0 GPIB Address

	if (m_selftesten->read()) {
		ret = (m_selftest->read() & 7) ^ 0xf;
	} else {
		ret = m_hpib_addr->read() ^ 0xf;
	}
	if (m_fdc_irq)
		ret |= IO2_R_FDCINT;
	if (!m_floppy->get_device()->dskchg_r())
		ret |= IO2_R_FDKDCH_N;
	if (!machine().side_effects_disabled())
		LOG("%s: %02x\n", __func__, ret);
	return ret;
}

uint8_t hp9133_device::io3_r()
{
	uint8_t ret = 0;

	// 7 - self test enable
	// 6 - self test MSB
	// 5 - FASTDON
	// 3-0 Volume switch
	ret = m_volcfg->read() ^ 0xf;
	if (!m_selftesten->read()) {
		ret |= IO3_R_TEST;
	} else {
		if (m_selftest->read() & 8)
			ret |= IO3_R_SELTST;
	}

	if (m_dma_addr >= 256)
		ret |= IO3_R_FASTDON;
	if (!machine().side_effects_disabled())
		LOG("%s: %02x\n", __func__, ret);
	return ret;
}

void hp9133_device::dma_ram_w(uint8_t data)
{
	LOG("%s: %03x = %02x\n", __func__, m_dma_addr, data);
	if (m_dma_addr < sizeof(m_dma_ram))
		m_dma_ram[m_dma_addr++] = data;
}

uint8_t hp9133_device::dma_ram_r(offs_t offset)
{
	if (m_dma_addr >= sizeof(m_dma_ram))
		return 0;
	uint8_t ret = m_dma_ram[m_dma_addr];
	if (!machine().side_effects_disabled()) {
		LOG("%s: %03x = %02x\n", __func__, m_dma_addr, ret);
		m_dma_addr++;
	}
	return ret;
}

static int sdh_sector_size(uint8_t sdh)
{
	constexpr int sizes[4] = { 256, 512, 1024, 128 };

	return sizes[(sdh >> 5) & 3];
}

static int sdh_devsel(uint8_t sdh)
{
	return (sdh >> 3) & 3;
}

void hp9133_device::hdc_readwrite_sector(bool write)
{
	int sector = m_hdc->read(3);
	uint32_t lba;

	if (sdh_devsel(m_hdc_sdh))
		return;
	if (sdh_sector_size(m_hdc_sdh) != 256)
		return;
	if (!m_harddisk->exists())
		return;
	const auto &info = m_harddisk->get_info();
	lba = (m_hdc_cylinder * info.heads + m_head) * info.sectors + sector;
	LOG("%s: %s cyl %4d, head %d, sector %2d lba %8d\n", __func__,
			write ? "WRITE" : "READ ", m_hdc_cylinder, m_head, sector, lba);
	if (write) {
		m_harddisk->write(lba, m_dma_ram);
	} else {
		m_harddisk->read(lba, m_dma_ram);
		m_dma_addr = 256;
	}
}

void hp9133_device::fdc_intrq_w(int state)
{
	m_fdc_irq = state;
	update_intsel();
}

void hp9133_device::fdc_drq_w(int state)
{
	m_fdc_drq = state;
	update_intsel();
}

void hp9133_device::floppy_index_cb(floppy_image_device *floppy, int state)
{
	m_fdc->index_callback(floppy, state);
	if (state)
		m_floppy_index_int = true;
	update_intsel();
}

void hp9133_device::hdc_bcs_cb(int state)
{
	m_hdc_bcs = state;
}

void hp9133_device::hdc_bcr_cb(int state)
{
	m_dma_addr = 0;
}

void hp9133_device::hdc_bdrq_cb(int state)
{
	bool is_read = m_hdc_cmd >> 4 == 2;

	m_hdc_bdrq = state;
	if (state && is_read)
		hdc_readwrite_sector(false);
}


void hp9133_device::hdc_wg_cb(int state)
{
	bool is_write = (m_hdc_cmd >> 4) == 3;

	m_hdc_wg = state;
	if (state && is_write)
		hdc_readwrite_sector(true);
}

void hp9133_device::hdc_intrq_cb(int state)
{
	m_hdc_intrq = state;
	update_intsel();
}

uint8_t hp9133_device::hdc_reg_r(offs_t offset)
{
	return m_hdc->read(offset);
}

void hp9133_device::hdc_reg_w(offs_t offset, uint8_t data)
{
	switch (offset) {
	case 2:
		m_hdc_sector_count = data;
		break;
	case 3:
		m_hdc_sector_number = data;
		break;
	case 4:
		m_hdc_cylinder &= ~0xff;
		m_hdc_cylinder |= data;
		break;
	case 5:
		m_hdc_cylinder &= ~0xff00;
		m_hdc_cylinder |= data << 8;
		break;
	case 6:
		m_hdc_sdh = data;
		break;
	case 7:
		m_hdc_cmd = data;
		break;
	}
	m_hdc->write(offset, data);
}

void hp9133_device::cpu_map(address_map &map)
{
	map(0x0000, 0x0007).m(m_gpib, FUNC(i8291a_device::map));
	map(0x0008, 0x000b).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0x0010, 0x0017).rw(FUNC(hp9133_device::hdc_reg_r), FUNC(hp9133_device::hdc_reg_w));
	map(0x0020, 0x0027).rw(FUNC(hp9133_device::dma_ram_r), FUNC(hp9133_device::dma_ram_w));
	map(0x0028, 0x002f).lw8(NAME([this] (u8 data) { LOG("clear ram addr\n"); m_dma_addr = 0; }));
	map(0x003f, 0x003f).nopw();
	map(0x0040, 0x0040).rw(FUNC(hp9133_device::io1_r), FUNC(hp9133_device::io1_w)).mirror(0x3f3f);
	map(0x0080, 0x0080).rw(FUNC(hp9133_device::io2_r), FUNC(hp9133_device::io2_w)).mirror(0x3f3f);
	map(0x00c0, 0x00c0).rw(FUNC(hp9133_device::io3_r), FUNC(hp9133_device::io3_w)).mirror(0x3f3f);
	map(0x4000, 0x47ff).ram().mirror(0x3800);
	map(0x8000, 0xffff).rom().region("cpu", 0);
}

static void hp9133_floppies(device_slot_interface &device)
{
	device.option_add("d32w", SONY_OA_D32W);
}

void hp9133_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_cpu, XTAL(8'000'000));
	m_cpu->set_addrmap(AS_PROGRAM, &hp9133_device::cpu_map);
	m_cpu->sync_acknowledge_write().set(FUNC(hp9133_device::cpu_sync_ack));

	I8291A(config, m_gpib, XTAL(8'000'000));
	m_gpib->eoi_write().set(FUNC(hp9133_device::i8291a_eoi_w));
	m_gpib->dav_write().set(FUNC(hp9133_device::i8291a_dav_w));
	m_gpib->nrfd_write().set(FUNC(hp9133_device::i8291a_nrfd_w));
	m_gpib->ndac_write().set(FUNC(hp9133_device::i8291a_ndac_w));
	m_gpib->srq_write().set(FUNC(hp9133_device::i8291a_srq_w));
	m_gpib->dio_write().set(FUNC(hp9133_device::i8291a_dio_w));
	m_gpib->dio_read().set(FUNC(hp9133_device::i8291a_dio_r));
	m_gpib->int_write().set(FUNC(hp9133_device::i8291a_int_w));
	m_gpib->dreq_write().set(FUNC(hp9133_device::i8291a_drq_w));

	WD2010(config, m_hdc, XTAL(5'000'000));
	m_hdc->in_sc_callback().set_constant(1);
	m_hdc->in_drdy_callback().set_constant(1);
	m_hdc->out_bcs_callback().set(FUNC(hp9133_device::hdc_bcs_cb));
	m_hdc->out_bcr_callback().set(FUNC(hp9133_device::hdc_bcr_cb));
	m_hdc->out_bdrq_callback().set(FUNC(hp9133_device::hdc_bdrq_cb));
	m_hdc->out_intrq_callback().set(FUNC(hp9133_device::hdc_intrq_cb));
	m_hdc->out_wg_callback().set(FUNC(hp9133_device::hdc_wg_cb));
	HARDDISK(config, m_harddisk);

	WD2793(config, m_fdc, XTAL(2'000'000));
	m_fdc->intrq_wr_callback().set(FUNC(hp9133_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(hp9133_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, "floppy0", hp9133_floppies, "d32w",
			 floppy_image_device::default_mfm_floppy_formats, true).enable_sound(true);
}

ROM_START(hp9133)
	ROM_REGION(0x8000, "cpu", 0)
	ROM_LOAD("09133-89110_9133.bin", 0x0000, 0x4000, CRC(d7ff6b3e) SHA1(aeeae063fa43c3b163ba5c176f31df6ec1b73d0d))
	ROM_LOAD("09133-89210_9133.bin", 0x4000, 0x4000, CRC(08f825e2) SHA1(61766b1f64473c17f01cbc97bd817f6076827d6a))
ROM_END

const tiny_rom_entry *hp9133_device::device_rom_region() const
{
	return ROM_NAME(hp9133);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(HP9133, device_ieee488_interface, hp9133_device, "hp9133", "HP9133 Floppy/Fixed disk drive")

// license:BSD-3-Clause
// copyright-holders: Sven Schnelle
/*********************************************************************

    hp9122c.cpp

    HP9122c dual floppy disk drive

*********************************************************************/

#include "emu.h"
#include "hp9122c.h"
#include "cpu/m6809/m6809.h"
#include "machine/i8291a.h"
#include "machine/wd_fdc.h"
#include "hp9122c.lh"

DEFINE_DEVICE_TYPE(HP9122C, hp9122c_device, "hp9122c", "HP9122C Dual High density disk drive")

hp9122c_device::hp9122c_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t{mconfig, HP9122C, tag, owner, clock},
	  device_ieee488_interface{mconfig, *this},
	  m_cpu{*this , "cpu"},
	  m_i8291a{*this, "i8291a"},
	  m_fdc{*this, "mb8876"},
	  m_floppy{*this, "floppy%u", 0},
	  m_hpib_addr{*this , "ADDRESS"},
	  m_testmode{*this, "TESTMODE"},
	  m_leds{*this, "led%d", 0U},
	  m_intsel{3},
	  m_fdc_irq{false},
	  m_i8291a_irq{false},
	  m_fdc_drq{false},
	  m_i8291a_drq{false},
	  m_cpuirq{false},
	  m_cpufirq{false},
	  m_index_int{false},
	  m_ds0{false},
	  m_ds1{false}
{
}

static INPUT_PORTS_START(hp9122c_port)
	PORT_START("ADDRESS")
	PORT_CONFNAME(0x1f, 0x00 , "HPIB address")
	PORT_CONFSETTING(0, "0")
	PORT_CONFSETTING(1, "1")
	PORT_CONFSETTING(2, "2")
	PORT_CONFSETTING(3, "3")
	PORT_CONFSETTING(4, "4")
	PORT_CONFSETTING(5, "5")
	PORT_CONFSETTING(6, "6")
	PORT_CONFSETTING(7, "7")
	PORT_CONFSETTING(7, "8")
	PORT_CONFSETTING(7, "9")
	PORT_CONFSETTING(7, "10")
	PORT_CONFSETTING(7, "11")
	PORT_CONFSETTING(7, "12")
	PORT_CONFSETTING(7, "13")
	PORT_CONFSETTING(7, "14")
	PORT_CONFSETTING(7, "15")

	PORT_START("TESTMODE")
	PORT_CONFNAME(0x01, 0x00, "Testmode")
	PORT_CONFSETTING(0, DEF_STR(Off))
	PORT_CONFSETTING(1, DEF_STR(On))

INPUT_PORTS_END

ioport_constructor hp9122c_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(hp9122c_port);
}

void hp9122c_device::device_start()
{

	m_leds.resolve();

	for (auto &floppy : m_floppy)
		floppy->get_device()->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&hp9122c_device::index_pulse_cb, this));

	save_item(NAME(m_intsel));
	save_item(NAME(m_fdc_irq));
	save_item(NAME(m_fdc_drq));
	save_item(NAME(m_i8291a_irq));
	save_item(NAME(m_i8291a_drq));
	save_item(NAME(m_cpuirq));
	save_item(NAME(m_cpufirq));
	save_item(NAME(m_index_int));
	save_item(NAME(m_ds0));
	save_item(NAME(m_ds1));

	m_motor_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hp9122c_device::motor_timeout), this));
}

TIMER_CALLBACK_MEMBER(hp9122c_device::motor_timeout)
{
	floppy_image_device *floppy0 = m_floppy[0]->get_device();
	floppy_image_device *floppy1 = m_floppy[1]->get_device();
	floppy0->mon_w(1);
	floppy1->mon_w(1);
}

void hp9122c_device::device_reset()
{
	m_fdc_irq = false;
	m_fdc_drq = false;
	m_i8291a_irq = false;
	m_i8291a_drq = false;
	m_cpuirq = false;
	m_cpufirq = false;
	m_ds0 = false;
	m_ds1 = false;
	m_index_int = false;
}

void hp9122c_device::ieee488_eoi(int state)
{
	m_i8291a->eoi_w(state);
}

void hp9122c_device::ieee488_dav(int state)
{
	m_i8291a->dav_w(state);
}

void hp9122c_device::ieee488_nrfd(int state)
{
	m_i8291a->nrfd_w(state);
}

void hp9122c_device::ieee488_ndac(int state)
{
	m_i8291a->ndac_w(state);
}

void hp9122c_device::ieee488_ifc(int state)
{
	m_i8291a->ifc_w(state);
}

void hp9122c_device::ieee488_srq(int state)
{
	m_i8291a->srq_w(state);
}

void hp9122c_device::ieee488_atn(int state)
{
	m_i8291a->atn_w(state);
}

void hp9122c_device::ieee488_ren(int state)
{
	m_i8291a->ren_w(state);
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_eoi_w)
{
	m_bus->eoi_w(this, state);
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_dav_w)
{
	m_bus->dav_w(this, state);
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_nrfd_w)
{
	m_bus->nrfd_w(this, state);
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_ndac_w)
{
	m_bus->ndac_w(this, state);
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_ifc_w)
{
	m_bus->ifc_w(this, state);
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_srq_w)
{
	m_bus->srq_w(this, state);
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_atn_w)
{
	m_bus->atn_w(this, state);
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_ren_w)
{
	m_bus->ren_w(this, state);
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_int_w)
{
	m_i8291a_irq = state;
	update_intsel();
}

WRITE_LINE_MEMBER(hp9122c_device::i8291a_dreq_w)
{
	m_i8291a_drq = state;
	update_intsel();
}

WRITE_LINE_MEMBER(hp9122c_device::fdc_intrq_w)
{
	m_fdc_irq = state;
	update_intsel();
}

WRITE_LINE_MEMBER(hp9122c_device::fdc_drq_w)
{
	m_fdc_drq = state;
	update_intsel();
}

void hp9122c_device::update_intsel()
{
	bool irq = false, firq = false;

	switch (m_intsel & 3) {
	case 0:
		/* fdc int connected */
		irq = m_fdc_drq;
		firq = m_fdc_irq;
		break;
	case 1:
		/* i8291a ints connected */
		irq = m_i8291a_drq;
		firq = m_i8291a_irq;
		break;
	case 2:
		irq = m_index_int;
		firq = false;
		break;

	case 3:
		irq = false;
		firq = false;
		break;
	}

	if (m_cpufirq != firq)
		m_cpu->set_input_line(M6809_FIRQ_LINE, firq ? ASSERT_LINE : CLEAR_LINE);

	if (m_cpuirq != irq)
		m_cpu->set_input_line(M6809_IRQ_LINE, irq ? ASSERT_LINE : CLEAR_LINE);

	m_cpuirq = irq;
	m_cpufirq = firq;
}

READ8_MEMBER(hp9122c_device::i8291a_dio_r)
{
	return m_bus->read_dio();
}

WRITE8_MEMBER(hp9122c_device::i8291a_dio_w)
{
	m_bus->dio_w(this, data);
}

READ8_MEMBER(hp9122c_device::status_r)
{
	uint8_t ret = REG_STATUS_DUAL|REG_STATUS_DISKCHG;
	auto addr = m_hpib_addr->read();

	if (!m_testmode->read())
		ret |= 0x10;

	ret |= ((addr & 0x08) << 3) | (addr & 0x07);

	floppy_image_device *floppy = nullptr;
	if (m_ds0)
		floppy = m_floppy[0]->get_device();
	if (m_ds1)
		floppy = m_floppy[1]->get_device();

	if (floppy) {
		auto variant = floppy->get_variant();
		if (variant == floppy_image::DSDD || variant == floppy_image::SSDD)
			ret |= REG_STATUS_LOW_DENSITY;
	}

	return ret;
}

WRITE8_MEMBER(hp9122c_device::cmd_w)
{
	floppy_image_device *floppy0 = m_floppy[0]->get_device();
	floppy_image_device *floppy1 = m_floppy[1]->get_device();

	m_ds0 = !(data & REG_CNTL_DS0) && floppy0;
	m_ds1 = !(data & REG_CNTL_DS1) && floppy1;
	m_leds[2] = (data & 0x40) ? true : false;

	if (m_ds0) {
		floppy0->mon_w(0);
		floppy0->ss_w(!(data & REG_CNTL_HEADSEL));
		m_fdc->set_floppy(floppy0);
		m_motor_timer->reset();
		m_leds[0] = true;
		m_leds[1] = false;
	} else if (m_ds1) {
		floppy1->mon_w(0);
		floppy1->ss_w(!(data & REG_CNTL_HEADSEL));
		m_fdc->set_floppy(floppy1);
		m_motor_timer->reset();
		m_leds[0] = false;
		m_leds[1] = true;
	} else {
		m_motor_timer->adjust(attotime::from_msec(2000));
		m_leds[0] = false;
		m_leds[1] = false;
	}

	if (data & REG_CNTL_CLOCK_SEL)
		m_fdc->set_clock(XTAL(8'000'000)/4);
	else
		m_fdc->set_clock(XTAL(8'000'000)/8);

	m_intsel = ((data & REG_CNTL_INTSEL0) >> 7) | ((data & REG_CNTL_INTSEL1) >> 2);
	update_intsel();
}

void hp9122c_device::index_pulse_cb(floppy_image_device *floppy, int state)
{
	m_fdc->index_callback(floppy, state);

	if (state)
		m_index_int = true;
	update_intsel();

}

WRITE8_MEMBER(hp9122c_device::clridx_w)
{
	m_index_int = false;
	update_intsel();
}

ROM_START(hp9122c)
	ROM_REGION(0x4000 , "cpu" , 0)
	ROM_LOAD("09122-15515.bin" , 0x0000, 0x4000 , CRC(d385e488) SHA1(93b2015037d76cc68b6252df03d6f184104605b9))
ROM_END

static void hp9122c_floppies(device_slot_interface &device)
{
	device.option_add("35dd" , FLOPPY_35_DD);
	device.option_add("35hd" , FLOPPY_35_HD);
}

static const floppy_format_type hp9122c_floppy_formats[] = {
	FLOPPY_MFI_FORMAT,
	FLOPPY_TD0_FORMAT,
	nullptr
};

const tiny_rom_entry *hp9122c_device::device_rom_region() const
{
	return ROM_NAME(hp9122c);
}

void hp9122c_device::cpu_map(address_map &map)
{
	map(0x0000, 0x0800).ram();
	map(0x2000, 0x2003).rw(m_fdc, FUNC(mb8876_device::read), FUNC(mb8876_device::write)).mirror(0x1ff4);
	map(0x4000, 0x4007).m("i8291a", FUNC(i8291a_device::map)).mirror(0x1ff8);
	map(0x6000, 0x7fff).rw(FUNC(hp9122c_device::status_r), FUNC(hp9122c_device::cmd_w));
	map(0x8000, 0x8001).w(FUNC(hp9122c_device::clridx_w));
	map(0xc000, 0xffff).rom().region("cpu", 0);
}

void hp9122c_device::device_add_mconfig(machine_config &config)
{
	MC6809(config, m_cpu, XTAL(8'000'000));
	m_cpu->set_addrmap(AS_PROGRAM, &hp9122c_device::cpu_map);

	// without this flag, 'DMA' transfer via SYNC instruction will not work
	//config.set_perfect_quantum(m_cpu); FIXME: not safe in a slot device - add barriers

	MB8876(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(hp9122c_device::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(hp9122c_device::fdc_drq_w));

	I8291A(config, m_i8291a, XTAL(2'000'000));
	m_i8291a->eoi_write().set(FUNC(hp9122c_device::i8291a_eoi_w));
	m_i8291a->dav_write().set(FUNC(hp9122c_device::i8291a_dav_w));
	m_i8291a->nrfd_write().set(FUNC(hp9122c_device::i8291a_nrfd_w));
	m_i8291a->ndac_write().set(FUNC(hp9122c_device::i8291a_ndac_w));
	m_i8291a->srq_write().set(FUNC(hp9122c_device::i8291a_srq_w));
	m_i8291a->dio_write().set(FUNC(hp9122c_device::i8291a_dio_w));
	m_i8291a->dio_read().set(FUNC(hp9122c_device::i8291a_dio_r));
	m_i8291a->int_write().set(FUNC(hp9122c_device::i8291a_int_w));
	m_i8291a->dreq_write().set(FUNC(hp9122c_device::i8291a_dreq_w));

	FLOPPY_CONNECTOR(config, "floppy0" , hp9122c_floppies , "35hd" , hp9122c_floppy_formats, true).enable_sound(true);
	FLOPPY_CONNECTOR(config, "floppy1" , hp9122c_floppies , "35hd" , hp9122c_floppy_formats, true).enable_sound(true);
	config.set_default_layout(layout_hp9122c);
}

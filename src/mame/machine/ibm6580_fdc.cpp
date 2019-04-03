// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#include "emu.h"
#include "ibm6580_fdc.h"


//#define LOG_GENERAL (1U <<  0) //defined in logmacro.h already
#define LOG_DEBUG     (1U <<  1)

//#define VERBOSE (LOG_GENERAL | LOG_DEBUG)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"

#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


DEFINE_DEVICE_TYPE(DW_FDC, dw_fdc_device, "dw_fdc", "IBM Displaywriter Floppy")

ROM_START( dw_fdc )
	ROM_REGION(0x800, "mcu", 0)
	ROM_LOAD("4430030_FLP_8041.BIN", 0x0000, 0x400, CRC(2bb96799) SHA1(e30b0f2d790197f290858eab74ad5e151ded78c3))
ROM_END


const tiny_rom_entry *dw_fdc_device::device_rom_region() const
{
	return ROM_NAME( dw_fdc );
}

void dw_fdc_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_mcu, 24_MHz_XTAL / 4);    // divisor is unverified
//  m_mcu->bus_in_cb().set(FUNC(dw_fdc_device::bus_r));
//  m_mcu->bus_out_cb().set(FUNC(dw_fdc_device::bus_w));
	m_mcu->p1_out_cb().set(FUNC(dw_fdc_device::p1_w));
	m_mcu->p2_out_cb().set(FUNC(dw_fdc_device::p2_w));
//  m_mcu->t0_in_cb().set(FUNC(dw_fdc_device::t0_r));
	m_mcu->t1_in_cb().set(FUNC(dw_fdc_device::t1_r));

	I8255(config, "ppi8255", 0);

	UPD765A(config, "upd765", 24_MHz_XTAL / 3, false, false);
//  m_upd_fdc->intrq_wr_callback().set("pic8259", FUNC(pic8259_device::ir4_w));
//  m_upd_fdc->drq_wr_callback().set("dma8257", FUNC(dma8257_device::XXX));
//  FLOPPY_CONNECTOR(config, UPD765_TAG ":0", wangpc_floppies, "525dd", wangpc_state::floppy_formats);
//  FLOPPY_CONNECTOR(config, UPD765_TAG ":1", wangpc_floppies, "525dd", wangpc_state::floppy_formats);
}


dw_fdc_device::dw_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DW_FDC, tag, owner, clock)
	, m_out_data(*this)
	, m_out_clock(*this)
	, m_out_strobe(*this)
	, m_mcu(*this, "mcu")
{
}

void dw_fdc_device::device_start()
{
	m_out_data.resolve_safe();
	m_out_clock.resolve_safe();
	m_out_strobe.resolve_safe();
	m_reset_timer = timer_alloc();
}

void dw_fdc_device::device_reset()
{
	m_p1 = m_p2 = m_t0 = m_t1 = 0;
}

void dw_fdc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_mcu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

WRITE8_MEMBER( dw_fdc_device::p1_w )
{
	m_p1 = data;

	LOGDBG("p1 <- %02x\n", data, m_p1);
}

WRITE8_MEMBER( dw_fdc_device::p2_w )
{
	m_p2 = data;

	LOGDBG("p2 <- %02x\n", data);
}

READ8_MEMBER( dw_fdc_device::p2_r )
{
	uint8_t data = m_p2;

	LOGDBG("p2 == %02x\n", data);

	return data;
}

READ_LINE_MEMBER( dw_fdc_device::t0_r )
{
	LOGDBG("t0 == %d\n", m_t0);

	return m_t0;
}

READ_LINE_MEMBER( dw_fdc_device::t1_r )
{
	LOGDBG("t1 == %d\n", m_t1);

	return m_t1;
}

WRITE8_MEMBER( dw_fdc_device::bus_w )
{
	m_bus = data;
}

READ8_MEMBER( dw_fdc_device::bus_r )
{
	return m_bus;
}

WRITE_LINE_MEMBER( dw_fdc_device::reset_w )
{
	if(!state)
		m_reset_timer->adjust(attotime::from_msec(50));
	else
	{
		m_reset_timer->adjust(attotime::never);
		m_mcu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER( dw_fdc_device::ack_w )
{
	m_t0 = state;
}

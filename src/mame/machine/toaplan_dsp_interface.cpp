// license:BSD-3-Clause
// copyright-holders:Quench
/***************************************************************************

    Toaplan common DSP interface emulation

    Toaplan used custom GXC DSP* with common interface at 1987 to 1990
    for protection and DSP operations,
    Some bootleg hardware also cloned this configurations.

    Used at:
    - twincobr.cpp (All hardwares)
    - wardner.cpp (All hardwares)
    - toaplan1.cpp (demonwld)

    * it's actually TI TMS320C10 with pre-programmed on-chip ROM,
      see toaplan_gxc.cpp

***************************************************************************/

#include "emu.h"
#include "machine/toaplan_dsp_interface.h"


#define LOG_DSP_CALLS      (1U<<1)
#define LOG_DSP_WARN       (1U<<2)

#define VERBOSE ( LOG_DSP_WARN )

#include "logmacro.h"


DEFINE_DEVICE_TYPE(TOAPLAN_DSP_INTF, toaplan_dsp_intf_device, "toaplan_dsp_intf", "Toaplan DSP Interface")


toaplan_dsp_intf_device::toaplan_dsp_intf_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, TOAPLAN_DSP_INTF, tag, owner, clock),
	m_dsp(*this, finder_base::DUMMY_TAG),
	m_halt_cb(*this),
	m_dsp_addr_cb(*this),
	m_dsp_read_cb(*this),
	m_dsp_write_cb(*this)
{
}


void toaplan_dsp_intf_device::dsp_addrsel_w(u16 data)
{
	// This sets the main CPU RAM address the DSP should
	//  read/write, via the DSP IO port 0

	m_dsp_addr_cb(m_main_ram_seg, m_dsp_addr_w, data);
	LOGMASKED(LOG_DSP_CALLS, "DSP PC:%04x IO write %04x (%08x) at port 0\n", m_dsp->pcbase(), data, m_main_ram_seg + m_dsp_addr_w);
}


u16 toaplan_dsp_intf_device::dsp_r()
{
	// DSP can read data from main CPU RAM via DSP IO port 1

	u16 input_data = 0;

	if (m_dsp_read_cb.isnull() || (!m_dsp_read_cb(m_main_ram_seg, m_dsp_addr_w, input_data)))
		LOGMASKED(LOG_DSP_WARN, "DSP PC:%04x Warning !!! IO reading from %08x (port 1)\n", m_dsp->pcbase(), m_main_ram_seg + m_dsp_addr_w);

	LOGMASKED(LOG_DSP_CALLS, "DSP PC:%04x IO read %04x at %08x (port 1)\n", m_dsp->pcbase(), input_data, m_main_ram_seg + m_dsp_addr_w);
	return input_data;
}


void toaplan_dsp_intf_device::dsp_w(u16 data)
{
	// Data written to main CPU RAM via DSP IO port 1

	m_dsp_execute = false;

	if (m_dsp_write_cb.isnull() || (!m_dsp_write_cb(m_main_ram_seg, m_dsp_addr_w, m_dsp_execute, data)))
		LOGMASKED(LOG_DSP_WARN, "DSP PC:%04x Warning !!! IO writing to %08x (port 1)\n", m_dsp->pcbase(), m_main_ram_seg + m_dsp_addr_w);

	LOGMASKED(LOG_DSP_CALLS, "DSP PC:%04x IO write %04x at %08x (port 1)\n", m_dsp->pcbase(), data, m_main_ram_seg + m_dsp_addr_w);
}


void toaplan_dsp_intf_device::dsp_bio_w(u16 data)
{
	// data 0xffff  means inhibit BIO line to DSP and enable
	//              communication to main processor
	//              Actually only DSP data bit 15 controls this
	// data 0x0000  means set DSP BIO line active and disable
	//              communication to main processor

	LOGMASKED(LOG_DSP_CALLS, "DSP PC:%04x IO write %04x at port 3\n", m_dsp->pcbase(), data);
	if (data & 0x8000)
		m_dsp_bio = CLEAR_LINE;

	if (data == 0)
	{
		if (m_dsp_execute)
		{
			LOGMASKED(LOG_DSP_CALLS, "Turning the main CPU on\n");
			m_halt_cb(CLEAR_LINE);
			m_dsp_execute = false;
		}
		m_dsp_bio = ASSERT_LINE;
	}
}


READ_LINE_MEMBER(toaplan_dsp_intf_device::bio_r)
{
	return m_dsp_bio;
}


WRITE_LINE_MEMBER(toaplan_dsp_intf_device::dsp_int_w)
{
	m_dsp_on = state;
	if (state)
	{
		// assert the INT line to the DSP
		LOGMASKED(LOG_DSP_CALLS, "Turning DSP on and main CPU off\n");
		m_dsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_dsp->set_input_line(0, ASSERT_LINE); // TMS32010 INT
		m_halt_cb(ASSERT_LINE);
	}
	else
	{
		// inhibit the INT line to the DSP
		LOGMASKED(LOG_DSP_CALLS, "Turning DSP off\n");
		m_dsp->set_input_line(0, CLEAR_LINE); // TMS32010 INT
		m_dsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}


void toaplan_dsp_intf_device::device_resolve_objects()
{
	m_halt_cb.resolve_safe();
	m_dsp_addr_cb.resolve();
	m_dsp_read_cb.resolve();
	m_dsp_write_cb.resolve();
}


void toaplan_dsp_intf_device::device_post_load()
{
	dsp_int_w(m_dsp_on);
}


void toaplan_dsp_intf_device::device_start()
{
	save_item(NAME(m_dsp_on));
	save_item(NAME(m_dsp_addr_w));
	save_item(NAME(m_main_ram_seg));
	save_item(NAME(m_dsp_bio));
	save_item(NAME(m_dsp_execute));
}


void toaplan_dsp_intf_device::device_reset()
{
	m_dsp_addr_w = 0;
	m_main_ram_seg = 0;
	m_dsp_execute = false;
	m_dsp_bio = CLEAR_LINE;
}


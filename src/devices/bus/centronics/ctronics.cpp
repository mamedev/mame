// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    Centronics printer interface

***************************************************************************/

#include "emu.h"
#include "ctronics.h"

// class centronics_device

DEFINE_DEVICE_TYPE(CENTRONICS, centronics_device, "centronics", "Centronics")

centronics_device::centronics_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CENTRONICS, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_strobe_handler(*this),
	m_data0_handler(*this),
	m_data1_handler(*this),
	m_data2_handler(*this),
	m_data3_handler(*this),
	m_data4_handler(*this),
	m_data5_handler(*this),
	m_data6_handler(*this),
	m_data7_handler(*this),
	m_ack_handler(*this),
	m_busy_handler(*this),
	m_perror_handler(*this),
	m_select_handler(*this),
	m_autofd_handler(*this),
	m_fault_handler(*this),
	m_init_handler(*this),
	m_select_in_handler(*this),
	m_dev(nullptr)
{
}

void centronics_device::device_config_complete()
{
	m_dev = dynamic_cast<device_centronics_peripheral_interface *>(get_card_device());
}

void centronics_device::device_start()
{
	m_strobe_handler.resolve_safe();
	m_data0_handler.resolve_safe();
	m_data1_handler.resolve_safe();
	m_data2_handler.resolve_safe();
	m_data3_handler.resolve_safe();
	m_data4_handler.resolve_safe();
	m_data5_handler.resolve_safe();
	m_data6_handler.resolve_safe();
	m_data7_handler.resolve_safe();
	m_ack_handler.resolve_safe();
	m_busy_handler.resolve_safe();
	m_perror_handler.resolve_safe();
	m_select_handler.resolve_safe();
	m_autofd_handler.resolve_safe();
	m_fault_handler.resolve_safe();
	m_init_handler.resolve_safe();
	m_select_in_handler.resolve_safe();

	// pull up
	m_strobe_handler(1);
	m_data0_handler(1);
	m_data1_handler(1);
	m_data2_handler(1);
	m_data3_handler(1);
	m_data4_handler(1);
	m_data5_handler(1);
	m_data6_handler(1);
	m_data7_handler(1);
	m_ack_handler(1);
	m_busy_handler(1);
	m_perror_handler(1);
	m_select_handler(1);
	m_autofd_handler(1);
	m_fault_handler(1);
	m_init_handler(1);
	m_select_in_handler(1);
}

void centronics_device::set_output_latch(output_latch_device &latch)
{
	latch.bit_handler<0>().set(*this, FUNC(centronics_device::write_data0));
	latch.bit_handler<1>().set(*this, FUNC(centronics_device::write_data1));
	latch.bit_handler<2>().set(*this, FUNC(centronics_device::write_data2));
	latch.bit_handler<3>().set(*this, FUNC(centronics_device::write_data3));
	latch.bit_handler<4>().set(*this, FUNC(centronics_device::write_data4));
	latch.bit_handler<5>().set(*this, FUNC(centronics_device::write_data5));
	latch.bit_handler<6>().set(*this, FUNC(centronics_device::write_data6));
	latch.bit_handler<7>().set(*this, FUNC(centronics_device::write_data7));
}

WRITE_LINE_MEMBER( centronics_device::write_strobe ) { if (m_dev) m_dev->input_strobe(state); }
WRITE_LINE_MEMBER( centronics_device::write_data0 ) { if (m_dev) m_dev->input_data0(state); }
WRITE_LINE_MEMBER( centronics_device::write_data1 ) { if (m_dev) m_dev->input_data1(state); }
WRITE_LINE_MEMBER( centronics_device::write_data2 ) { if (m_dev) m_dev->input_data2(state); }
WRITE_LINE_MEMBER( centronics_device::write_data3 ) { if (m_dev) m_dev->input_data3(state); }
WRITE_LINE_MEMBER( centronics_device::write_data4 ) { if (m_dev) m_dev->input_data4(state); }
WRITE_LINE_MEMBER( centronics_device::write_data5 ) { if (m_dev) m_dev->input_data5(state); }
WRITE_LINE_MEMBER( centronics_device::write_data6 ) { if (m_dev) m_dev->input_data6(state); }
WRITE_LINE_MEMBER( centronics_device::write_data7 ) { if (m_dev) m_dev->input_data7(state); }
WRITE_LINE_MEMBER( centronics_device::write_ack ) { if (m_dev) m_dev->input_ack(state); }
WRITE_LINE_MEMBER( centronics_device::write_busy ) { if (m_dev) m_dev->input_busy(state); }
WRITE_LINE_MEMBER( centronics_device::write_perror ) { if (m_dev) m_dev->input_perror(state); }
WRITE_LINE_MEMBER( centronics_device::write_select ) { if (m_dev) m_dev->input_select(state); }
WRITE_LINE_MEMBER( centronics_device::write_autofd ) { if (m_dev) m_dev->input_autofd(state); }
WRITE_LINE_MEMBER( centronics_device::write_fault ) { if (m_dev) m_dev->input_fault(state); }
WRITE_LINE_MEMBER( centronics_device::write_init ) { if (m_dev) m_dev->input_init(state); }
WRITE_LINE_MEMBER( centronics_device::write_select_in ) { if (m_dev) m_dev->input_select_in(state); }


// class device_centronics_peripheral_interface

device_centronics_peripheral_interface::device_centronics_peripheral_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "centronics")
{
	m_slot = dynamic_cast<centronics_device *>(device.owner());
}

device_centronics_peripheral_interface::~device_centronics_peripheral_interface()
{
}


#include "comxpl80.h"
#include "epson_ex800.h"
#include "epson_lx800.h"
#include "epson_lx810l.h"
#include "nec_p72.h"
#include "printer.h"
#include "covox.h"
#include "chessmec.h"

void centronics_devices(device_slot_interface &device)
{
	device.option_add("pl80", COMX_PL80);
	device.option_add("ex800", EPSON_EX800);
	device.option_add("lx800", EPSON_LX800);
	device.option_add("lx810l", EPSON_LX810L);
	device.option_add("ap2000", EPSON_AP2000);
	device.option_add("p72", NEC_P72);
	device.option_add("printer", CENTRONICS_PRINTER);
	device.option_add("covox", CENTRONICS_COVOX);
	device.option_add("covox_stereo", CENTRONICS_COVOX_STEREO);
	device.option_add("chessmec", CENTRONICS_CHESSMEC);
}

// license: BSD-3-Clause
// copyright-holders: Peter Ferrie, Angelo Salese
/**************************************************************************************************

Alladin HASP dongle for Savage Quest

TODO:
- convert legacy implementation from misc/savquest.cpp

**************************************************************************************************/

#include "emu.h"
#include "hasp_savquest.h"

DEFINE_DEVICE_TYPE(HASP_SAVQUEST, hasp_savquest_device, "hasp_savquest", "Alladin HASP dongle for Savage Quest")

hasp_savquest_device::hasp_savquest_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, HASP_SAVQUEST, tag, owner, clock),
	device_centronics_peripheral_interface(mconfig, *this)
{ }

void hasp_savquest_device::device_start()
{
	save_item(NAME(m_data_in));
}

void hasp_savquest_device::device_reset()
{
	m_select_in_state = 1;
	m_haspstate = HASPSTATE_NONE;
}

void hasp_savquest_device::input_select_in(int state)
{
	m_select_in_state = state;
	logerror("input_select_in %d\n", state);
}

void hasp_savquest_device::update_state()
{
//	const uint8_t s_hasp_cmppass[] = {
//		0xc3, 0xd9, 0xd3, 0xfb, 0x9d, 0x89, 0xb9, 0xa1, 0xb3, 0xc1, 0xf1, 0xcd, 0xdf, 0x9d
//	}; /* 0x9d or 0x9e */

//	const uint8_t s_hasp_prodinfo[] = {
//		0x51, 0x4c, 0x52, 0x4d, 0x53, 0x4e, 0x53, 0x4e, 0x53, 0x49, 0x53, 0x48, 0x53, 0x4b, 0x53, 0x4a,
//		0x53, 0x43, 0x53, 0x45, 0x52, 0x46, 0x53, 0x43, 0x53, 0x41, 0xac, 0x40, 0x53, 0xbc, 0x53, 0x42,
//		0x53, 0x57, 0x53, 0x5d, 0x52, 0x5e, 0x53, 0x5b, 0x53, 0x59, 0xac, 0x58, 0x53, 0xa4
//	};

	// ignore if not selected
	if (m_select_in_state)
		return;

	output_fault(0);
	output_select(0);
	output_ack(0);
	output_busy(1);

	// TODO: implementation here
	// need to collect states from the previous data input transitions,
	// no way get the "raw" value otherwise.
	// SW doesn't program any centronics line beyond select_in and init lines, at ODNGLV.EXE startup.

	m_prev_data = m_data_in;

//	printf("%02x\n", m_data_in);
	output_perror(1);
}

// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
 * An implementation of the ARBGA (SRX Arbiter Gate Array) device found in
 * Intergraph InterPro family systems. There is no public documentation on this
 * device, so the implementation is being built to follow the logic of the
 * boot ROM and its diagnostic tests.
 *
 * Please be aware that code in here is not only broken, it's likely wrong in
 * many cases.
 *
 * TODO
 *   - too long to list
 */

#include "emu.h"
#include "interpro_arbga.h"

#define VERBOSE 0
#include "logmacro.h"

#if 0
// this might be for an earlier revision of the device
void interpro_arbga_device::map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(interpro_arbga_device::sdepid_r), FUNC(interpro_arbga_device::sdepid_w));
	map(0x04, 0x07).rw(FUNC(interpro_arbga_device::snapid_r), FUNC(interpro_arbga_device::snapid_w));
	map(0x08, 0x0b).rw(FUNC(interpro_arbga_device::prilo_r), FUNC(interpro_arbga_device::prilo_w));
	map(0x0c, 0x0f).rw(FUNC(interpro_arbga_device::prihi_r), FUNC(interpro_arbga_device::prihi_w));
	map(0x10, 0x13).rw(FUNC(interpro_arbga_device::errdomlo_r), FUNC(interpro_arbga_device::errdomlo_w));
	map(0x14, 0x17).rw(FUNC(interpro_arbga_device::errdomhi_r), FUNC(interpro_arbga_device::errdomhi_w));
	map(0x18, 0x1b).rw(FUNC(interpro_arbga_device::tmctrl_r), FUNC(interpro_arbga_device::tmctrl_w));

	map(0x24, 0x27).rw(FUNC(interpro_arbga_device::tmsrnem_r), FUNC(interpro_arbga_device::tmsrnem_w));
	map(0x28, 0x2b).rw(FUNC(interpro_arbga_device::tmsrhog_r), FUNC(interpro_arbga_device::tmsrhog_w));
	map(0x2c, 0x2f).rw(FUNC(interpro_arbga_device::tmscale_r), FUNC(interpro_arbga_device::tmscale_w));
}
#endif

// derived from the FDM "dump_arb" command
void interpro_arbga_device::map(address_map &map)
{
	map(0x00, 0x03).rw(FUNC(interpro_arbga_device::sdepid_r), FUNC(interpro_arbga_device::sdepid_w));
	map(0x04, 0x07).rw(FUNC(interpro_arbga_device::arbsnap_r), FUNC(interpro_arbga_device::arbsnap_w));
	map(0x08, 0x0b).rw(FUNC(interpro_arbga_device::fixprils_r), FUNC(interpro_arbga_device::fixprils_w));
	map(0x0c, 0x0f).rw(FUNC(interpro_arbga_device::fixprims_r), FUNC(interpro_arbga_device::fixprims_w));
	map(0x10, 0x13).rw(FUNC(interpro_arbga_device::sysdomls_r), FUNC(interpro_arbga_device::sysdomls_w));
	map(0x14, 0x17).rw(FUNC(interpro_arbga_device::sysdomms_r), FUNC(interpro_arbga_device::sysdomms_w));
	map(0x18, 0x1b).rw(FUNC(interpro_arbga_device::tctrl_r), FUNC(interpro_arbga_device::tctrl_w));
	map(0x1d, 0x1d).rw(FUNC(interpro_arbga_device::inem_r), FUNC(interpro_arbga_device::inem_w)); // boot code writes 0x10
	map(0x21, 0x21).rw(FUNC(interpro_arbga_device::enem_r), FUNC(interpro_arbga_device::enem_w)); // boot code writes 0x07
	map(0x24, 0x27).rw(FUNC(interpro_arbga_device::hog_r), FUNC(interpro_arbga_device::hog_w));
	map(0x28, 0x2b).rw(FUNC(interpro_arbga_device::lock_r), FUNC(interpro_arbga_device::lock_w));
	map(0x2c, 0x2f).rw(FUNC(interpro_arbga_device::lockprs_r), FUNC(interpro_arbga_device::lockprs_w));
	map(0x30, 0x33).rw(FUNC(interpro_arbga_device::hiblockls_r), FUNC(interpro_arbga_device::hiblockls_w));
	map(0x34, 0x37).rw(FUNC(interpro_arbga_device::hiblockms_r), FUNC(interpro_arbga_device::hiblockms_w));

	map(0x3c, 0x3f).rw(FUNC(interpro_arbga_device::arbrev_r), FUNC(interpro_arbga_device::arbrev_w));
}

DEFINE_DEVICE_TYPE(INTERPRO_ARBGA, interpro_arbga_device, "arbga", "SRX Arbiter Gate Array")

interpro_arbga_device::interpro_arbga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_ARBGA, tag, owner, clock)
{
}

void interpro_arbga_device::device_start()
{
	save_item(NAME(m_sdepid));
	save_item(NAME(m_arbsnap));
	save_item(NAME(m_fixprils));
	save_item(NAME(m_fixprims));
	save_item(NAME(m_sysdomls));
	save_item(NAME(m_sysdomms));
	save_item(NAME(m_tctrl));
	save_item(NAME(m_inem));
	save_item(NAME(m_enem));
	save_item(NAME(m_hog));
	save_item(NAME(m_lock));
	save_item(NAME(m_lockprs));
	save_item(NAME(m_hiblockls));
	save_item(NAME(m_hiblockms));
	save_item(NAME(m_arbrev));
}

void interpro_arbga_device::device_reset()
{
}

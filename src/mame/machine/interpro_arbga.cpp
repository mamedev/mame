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
ADDRESS_MAP_START(interpro_arbga_device::map)
	AM_RANGE(0x00, 0x03) AM_READWRITE(sdepid_r, sdepid_w)
	AM_RANGE(0x04, 0x07) AM_READWRITE(snapid_r, snapid_w)
	AM_RANGE(0x08, 0x0b) AM_READWRITE(prilo_r, prilo_w)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE(prihi_r, prihi_w)
	AM_RANGE(0x10, 0x13) AM_READWRITE(errdomlo_r, errdomlo_w)
	AM_RANGE(0x14, 0x17) AM_READWRITE(errdomhi_r, errdomhi_w)
	AM_RANGE(0x18, 0x1b) AM_READWRITE(tmctrl_r, tmctrl_w)

	AM_RANGE(0x24, 0x27) AM_READWRITE(tmsrnem_r, tmsrnem_w)
	AM_RANGE(0x28, 0x2b) AM_READWRITE(tmsrhog_r, tmsrhog_w)
	AM_RANGE(0x2c, 0x2f) AM_READWRITE(tmscale_r, tmscale_w)
ADDRESS_MAP_END
#endif

// derived from the FDM "dump_arb" command
ADDRESS_MAP_START(interpro_arbga_device::map)
	AM_RANGE(0x00, 0x03) AM_READWRITE(sdepid_r, sdepid_w)
	AM_RANGE(0x04, 0x07) AM_READWRITE(arbsnap_r, arbsnap_w)
	AM_RANGE(0x08, 0x0b) AM_READWRITE(fixprils_r, fixprils_w)
	AM_RANGE(0x0c, 0x0f) AM_READWRITE(fixprims_r, fixprims_w)
	AM_RANGE(0x10, 0x13) AM_READWRITE(sysdomls_r, sysdomls_w)
	AM_RANGE(0x14, 0x17) AM_READWRITE(sysdomms_r, sysdomms_w)
	AM_RANGE(0x18, 0x1b) AM_READWRITE(tctrl_r, tctrl_w)
	AM_RANGE(0x1c, 0x1f) AM_READWRITE8(inem_r, inem_w, 0x0000ff00) // boot code writes 0x10
	AM_RANGE(0x20, 0x23) AM_READWRITE8(enem_r, enem_w, 0x0000ff00) // boot code writes 0x07
	AM_RANGE(0x24, 0x27) AM_READWRITE(hog_r, hog_w)
	AM_RANGE(0x28, 0x2b) AM_READWRITE(lock_r, lock_w)
	AM_RANGE(0x2c, 0x2f) AM_READWRITE(lockprs_r, lockprs_w)
	AM_RANGE(0x30, 0x33) AM_READWRITE(hiblockls_r, hiblockls_w)
	AM_RANGE(0x34, 0x37) AM_READWRITE(hiblockms_r, hiblockms_w)

	AM_RANGE(0x3c, 0x3f) AM_READWRITE(arbrev_r, arbrev_w)
ADDRESS_MAP_END

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

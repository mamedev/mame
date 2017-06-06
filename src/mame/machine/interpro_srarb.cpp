// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

/*
* An implementation of the SRX Abiter Gate Array device found on Intergraph InterPro family workstations. There is no
* public documentation on this device, so the implementation is being built to follow the logic of the
* system boot ROM and its diagnostic tests.
*
* Please be aware that code in here is not only broken, it's likely wrong in many cases.
*
* TODO
*   - too long to list
*/
#include "emu.h"
#include "interpro_srarb.h"

#define VERBOSE 0

DEVICE_ADDRESS_MAP_START(map, 32, interpro_srarb_device)
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

DEFINE_DEVICE_TYPE(INTERPRO_SRARB, interpro_srarb_device, "srarb", "InterPro SRARB")

interpro_srarb_device::interpro_srarb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, INTERPRO_SRARB, tag, owner, clock)
{
}

void interpro_srarb_device::device_start()
{
}

void interpro_srarb_device::device_reset()
{
}

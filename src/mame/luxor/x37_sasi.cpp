// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Luxor X37 SASI hard disk controller emulation

*********************************************************************/

#include "emu.h"
#include "x37_sasi.h"

DEFINE_DEVICE_TYPE(LUXOR_X37_SASI, luxor_x37_sasi_device, "luxor_x37_sasi", "Luxor X37 SASI")

void luxor_x37_sasi_device::device_add_mconfig(machine_config &config)
{
	auto &sasi(NSCSI_BUS(config, "sasi"));
	NSCSI_CONNECTOR(config, "sasi:0", default_scsi_devices, "s1410");
	sasi.set_external_device(7, *this);
}

luxor_x37_sasi_device::luxor_x37_sasi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LUXOR_X37_SASI, tag, owner, clock),
	nscsi_device_interface(mconfig, *this),
	m_write_int(*this)
{
}

void luxor_x37_sasi_device::device_start()
{
}

void luxor_x37_sasi_device::device_reset()
{
}

void luxor_x37_sasi_device::scsi_ctrl_changed()
{
}

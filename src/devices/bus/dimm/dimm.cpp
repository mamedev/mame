// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    DIMM "Dual In-line Memory Module"

***************************************************************************/

#include "emu.h"
#include "dimm.h"


DEFINE_DEVICE_TYPE(SDR_SDRAM_SLOT, sdr_sdram_slot_device, "sdr_sdram_slot", "SDR SDRAM Slot")
DEFINE_DEVICE_TYPE(DDR_SDRAM_SLOT, ddr_sdram_slot_device, "ddr_sdram_slot", "DDR SDRAM Slot")

dimm_slot_device::dimm_slot_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_single_card_slot_interface<device_dimm_interface>(mconfig, *this),
	m_module(nullptr),
	m_sda_cb(*this),
	m_address(0x50)
{
}

dimm_slot_device::~dimm_slot_device()
{
}

void dimm_slot_device::device_start()
{
	m_module = get_card_device();

	if (m_module)
		m_module->set_address(m_address);
}

void dimm_slot_device::sda_w(int state)
{
	if (m_module)
		m_module->sda_w(state);
}

void dimm_slot_device::scl_w(int state)
{
	if (m_module)
		m_module->scl_w(state);
}

sdr_sdram_slot_device::sdr_sdram_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dimm_slot_device(mconfig, SDR_SDRAM_SLOT, tag, owner, clock)
{
	set_options(sdr_sdram_modules, nullptr, false);
}

ddr_sdram_slot_device::ddr_sdram_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dimm_slot_device(mconfig, DDR_SDRAM_SLOT, tag, owner, clock)
{
	set_options(ddr_sdram_modules, nullptr, false);
}


device_dimm_interface::device_dimm_interface(const machine_config &mconfig, device_t &device) :
	i2c_hle_interface(mconfig, device, 0x50),
	m_slot(dynamic_cast<dimm_slot_device *>(device.owner()))
{
	sda_callback().set(*m_slot, FUNC(dimm_slot_device::host_sda_w));
}

device_dimm_interface::~device_dimm_interface()
{
}


spd_dimm_device::spd_dimm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dimm_interface(mconfig, *this),
	m_spd(*this, "spd")
{
}

void spd_dimm_device::device_start()
{
}

uint8_t spd_dimm_device::read_data(uint16_t offset)
{
	if (offset < m_spd.length())
		return m_spd[offset];

	return 0xff;
}

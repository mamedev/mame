// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "fp1060io.h"

DEFINE_DEVICE_TYPE(FP1060IO, fp1060io_device, "fp1060io", "FP-1060I/O Expansion Box")

fp1060io_device::fp1060io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fp1000_exp_device(mconfig, FP1060IO, tag, owner, clock)
	, m_subslot(*this, "%u", 0U)
{
}


void fp1060io_device::device_add_mconfig(machine_config &config)
{
	for (auto slot : m_subslot)
	{
		// default is just for CLI usability, what a retail '1060 gives back on its own is unconfirmed.
		FP1060IO_EXP_SLOT(config, slot, fp1060io_slot_devices, "rampack");
	}
}

void fp1060io_device::device_start()
{
}

void fp1060io_device::device_reset()
{
}

u8 fp1060io_device::id_r(offs_t offset)
{
	logerror("%s: ID select %02x\n", machine().describe_context(), m_slot_select);
	if (m_slot_select & 0xc)
		return 0xff;
	const auto dev = m_subslot[m_slot_select]->m_dev;
	//logerror("\texists: %d\n", dev != nullptr);

	if (dev == nullptr)
		return 0xff;

	return dev->get_id();
}

void fp1060io_device::cs_w(offs_t offset, u8 data)
{
	m_slot_select = data & 0xf;
}

void fp1060io_device::remap_cb()
{
	logerror("%s: remap_cb %02x\n", machine().describe_context(), m_slot_select);
	if (m_slot_select & 0xc)
		m_slot->iospace().unmap_readwrite(0x0000, 0xfeff);
	else
	{
		const auto dev = m_subslot[m_slot_select]->m_dev;
		//logerror("\texists %d\n", dev != nullptr);
		if (dev == nullptr)
			m_slot->iospace().unmap_readwrite(0x0000, 0xfeff);
		else
			m_slot->iospace().install_device(0x0000, 0xfeff, *m_subslot[m_slot_select]->m_dev, &device_fp1060io_exp_interface::io_map);
	}
}

// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#include "emu.h"
#include "fp1060io.h"

#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

DEFINE_DEVICE_TYPE(FP1060IO, fp1060io_device, "fp1060io", "FP-1060I/O Expansion Box")

fp1060io_device::fp1060io_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: fp1000_exp_device(mconfig, FP1060IO, tag, owner, clock)
	, m_subslot(*this, "%u", 0U)
	, m_irqs_int(*this, { "irqs_inta", "irqs_intb", "irqs_intc", "irqs_intd"})
{
}


void fp1060io_device::device_add_mconfig(machine_config &config)
{
	INPUT_MERGER_ANY_HIGH(config, m_irqs_int[0]).output_handler().set(FUNC(fp1060io_device::inta_w));
	INPUT_MERGER_ANY_HIGH(config, m_irqs_int[1]).output_handler().set(FUNC(fp1060io_device::intb_w));
	INPUT_MERGER_ANY_HIGH(config, m_irqs_int[2]).output_handler().set(FUNC(fp1060io_device::intc_w));
	INPUT_MERGER_ANY_HIGH(config, m_irqs_int[3]).output_handler().set(FUNC(fp1060io_device::intd_w));

	// default is just for CLI usability, what a retail '1060 gives back on its own is unconfirmed.
	FP1060IO_EXP_SLOT(config, m_subslot[0], fp1060io_slot_devices, "rampack");
	m_subslot[0]->inta_callback().set(m_irqs_int[0], FUNC(input_merger_device::in_w<0>));
	m_subslot[0]->intb_callback().set(m_irqs_int[1], FUNC(input_merger_device::in_w<0>));
	m_subslot[0]->intc_callback().set(m_irqs_int[2], FUNC(input_merger_device::in_w<0>));
	m_subslot[0]->intd_callback().set(m_irqs_int[3], FUNC(input_merger_device::in_w<0>));

	FP1060IO_EXP_SLOT(config, m_subslot[1], fp1060io_slot_devices, "rampack");
	m_subslot[1]->inta_callback().set(m_irqs_int[0], FUNC(input_merger_device::in_w<1>));
	m_subslot[1]->intb_callback().set(m_irqs_int[1], FUNC(input_merger_device::in_w<1>));
	m_subslot[1]->intc_callback().set(m_irqs_int[2], FUNC(input_merger_device::in_w<1>));
	m_subslot[1]->intd_callback().set(m_irqs_int[3], FUNC(input_merger_device::in_w<1>));

	FP1060IO_EXP_SLOT(config, m_subslot[2], fp1060io_slot_devices, "rampack");
	m_subslot[2]->inta_callback().set(m_irqs_int[0], FUNC(input_merger_device::in_w<2>));
	m_subslot[2]->intb_callback().set(m_irqs_int[1], FUNC(input_merger_device::in_w<2>));
	m_subslot[2]->intc_callback().set(m_irqs_int[2], FUNC(input_merger_device::in_w<2>));
	m_subslot[2]->intd_callback().set(m_irqs_int[3], FUNC(input_merger_device::in_w<2>));

	FP1060IO_EXP_SLOT(config, m_subslot[3], fp1060io_slot_devices, "rampack");
	m_subslot[3]->inta_callback().set(m_irqs_int[0], FUNC(input_merger_device::in_w<3>));
	m_subslot[3]->intb_callback().set(m_irqs_int[1], FUNC(input_merger_device::in_w<3>));
	m_subslot[3]->intc_callback().set(m_irqs_int[2], FUNC(input_merger_device::in_w<3>));
	m_subslot[3]->intd_callback().set(m_irqs_int[3], FUNC(input_merger_device::in_w<3>));
}

void fp1060io_device::device_start()
{
}

void fp1060io_device::device_reset()
{
}

u8 fp1060io_device::id_r(offs_t offset)
{
	LOG("ID select %02x\n", m_slot_select);
	if (m_slot_select & 0xc)
		return 0xff;
	const auto dev = m_subslot[m_slot_select]->m_dev;
	//LOG("\texists: %d\n", dev != nullptr);

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
	LOG("remap_cb %02x\n", m_slot_select);
	if (m_slot_select & 0xc)
		m_slot->iospace().unmap_readwrite(0x0000, 0xfeff);
	else
	{
		const auto dev = m_subslot[m_slot_select]->m_dev;
		//LOG("\texists %d\n", dev != nullptr);
		if (dev == nullptr)
			m_slot->iospace().unmap_readwrite(0x0000, 0xfeff);
		else
			m_slot->iospace().install_device(0x0000, 0xfeff, *m_subslot[m_slot_select]->m_dev, &device_fp1060io_exp_interface::io_map);
	}
}

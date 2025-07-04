// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    DMA Slot for ZX Spectrum

DMA Controller mod is available in two variants:
- Data-Gear: uses I/O port 107 (0x6B)
- MB-02+: uses I/O port 11 (0x0B)

The most commonly used implementation was based on the UA858D chip, which differs in behavior from
the original Zilog DMA controller.
For example, several source code examples published in Czech magazines were written for the UA858D
and did not account for differences in control logic?such as the lack of automatic transfer enable
via WR3, a feature that is present in the Zilog chip.
As a result, there are two versions of the "DMA DEMO LEVEL 3" by Busysoft:
- The original, compatible with the UA858D
- A fixed version, modified to support the Zilog DMA controller

ref: https://web.archive.org/web/20250524155336/https://velesoft.speccy.cz/data-gear.htm

**********************************************************************/

#include "emu.h"
#include "slot.h"

DEFINE_DEVICE_TYPE(DMA_SLOT, dma_slot_device, "dma_slot", "Spectrum DMA Slot")

dma_slot_device::dma_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, DMA_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_dma_card_interface>(mconfig, *this)
	, m_io(*this, finder_base::DUMMY_TAG, -1)
	, m_out_busreq_cb(*this)
	, m_out_int_cb(*this)
	, m_in_mreq_cb(*this, 0)
	, m_out_mreq_cb(*this)
	, m_in_iorq_cb(*this, 0)
	, m_out_iorq_cb(*this)
	, m_card(nullptr)
{
}

dma_slot_device::~dma_slot_device()
{
}

void dma_slot_device::device_start()
{
	m_card = get_card_device();
}

void dma_slot_device::busreq_w(int state) { m_out_busreq_cb(state); }
void dma_slot_device::int_w(int state) { m_out_int_cb(state); }
void dma_slot_device::mreq_w(int state) { m_out_mreq_cb(state); }
u8 dma_slot_device::mreq_r(offs_t offset) { return m_in_mreq_cb(offset); }
void dma_slot_device::iorq_w(int state) { m_out_iorq_cb(state); }
u8 dma_slot_device::iorq_r(offs_t offset) { return m_in_iorq_cb(offset); }
void dma_slot_device::bai_w(int state) { if(m_card) m_card->bai_w(state); };


device_dma_card_interface::device_dma_card_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "dma")
{
	m_slot = dynamic_cast<dma_slot_device *>(device.owner());
}

device_dma_card_interface::~device_dma_card_interface()
{
}


dma_device::dma_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_dma_card_interface(mconfig, *this)
	, m_dma(*this, "dma")
	, m_dma_port(0)
{
}

void dma_device::bai_w(int state)
{
	m_dma->bai_w(state);
}

void dma_device::device_validity_check(validity_checker &valid) const
{
	if (!m_dma_port)
		osd_printf_error("DMA port is not configured\n");
}

void dma_device::device_add_mconfig(machine_config &config)
{
	m_dma->out_busreq_callback().set([this](int state) { m_slot->busreq_w(state); } );
	m_dma->out_int_callback().set([this](int state) { m_slot->int_w(state); } );
	m_dma->in_mreq_callback().set([this](offs_t offset) { return m_slot->mreq_r(offset); });
	m_dma->out_mreq_callback().set([this](int state) { m_slot->mreq_w(state); } );
	m_dma->in_iorq_callback().set([this](offs_t offset) { return m_slot->iorq_r(offset); });
	m_dma->out_iorq_callback().set([this](int state) { m_slot->iorq_w(state); } );
}


void dma_device::device_start()
{
	m_slot->io().install_readwrite_handler(m_dma_port, m_dma_port, 0, 0xff00, 0, emu::rw_delegate(*m_dma, FUNC(z80dma_device::read)), emu::rw_delegate(*m_dma, FUNC(z80dma_device::write)));
}


#include "cards.h"

template class device_finder<device_dma_card_interface, false>;
template class device_finder<device_dma_card_interface, true>;

void default_dma_slot_devices(device_slot_interface &device)
{
	device.option_add("datagear",  DMA_SLOT_DATAGEAR);
	device.option_add("datagear_zilog",  DMA_SLOT_DATAGEAR_ZILOG);
	device.option_add("mb02p", DMA_SLOT_MB02P);
	device.option_add("mb02p_zilog", DMA_SLOT_MB02P_ZILOG);
}

// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"
#include "cards.h"

namespace bus::spectrum::dma_slot {

namespace {

/**********************************************************************
    DATAGEAR: UA858D, Port 0x6b
**********************************************************************/
class datagear_device : public dma_device
{
public:
	datagear_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: dma_device(mconfig, DMA_SLOT_DATAGEAR, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

void datagear_device::device_add_mconfig(machine_config &config)
{
	UA858D(config, m_dma, clock());
	m_dma_port = 0x6b;

	dma_device::device_add_mconfig(config);
}


/**********************************************************************
    DATAGEAR: ZILOG, Port 0x6b
**********************************************************************/
class datagear_zilog_device : public dma_device
{
public:
	datagear_zilog_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: dma_device(mconfig, DMA_SLOT_DATAGEAR_ZILOG, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


/**********************************************************************
    DATAGEAR: UA858D, Port 0x6b
**********************************************************************/
void datagear_zilog_device::device_add_mconfig(machine_config &config)
{
	Z80DMA(config, m_dma, clock());
	m_dma_port = 0x6b;

	dma_device::device_add_mconfig(config);
}


/**********************************************************************
    MB02+: UA858D, Port 0x0b
**********************************************************************/
class mb02p_device : public dma_device
{
public:
	mb02p_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: dma_device(mconfig, DMA_SLOT_MB02P, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

void mb02p_device::device_add_mconfig(machine_config &config)
{
	UA858D(config, m_dma, clock());
	m_dma_port = 0x0b;

	dma_device::device_add_mconfig(config);
}


/**********************************************************************
    MB02+: ZILOG, Port 0x0b
**********************************************************************/
class mb02p_zilog_device : public dma_device
{
public:
	mb02p_zilog_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: dma_device(mconfig, DMA_SLOT_MB02P_ZILOG, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

void mb02p_zilog_device::device_add_mconfig(machine_config &config)
{
	Z80DMA(config, m_dma, clock());
	m_dma_port = 0x0b;

	dma_device::device_add_mconfig(config);
}

} // anonymous namespace

} // namespace bus::spectrum::dma_slot

DEFINE_DEVICE_TYPE_PRIVATE(DMA_SLOT_DATAGEAR,       device_dma_card_interface, bus::spectrum::dma_slot::datagear_device,       "dma_datagear",       "DATEGEAR 107(#6b) UA858D")
DEFINE_DEVICE_TYPE_PRIVATE(DMA_SLOT_DATAGEAR_ZILOG, device_dma_card_interface, bus::spectrum::dma_slot::datagear_zilog_device, "dma_datagear_zilog", "DATEGEAR 107(#6b) ZILOG")
DEFINE_DEVICE_TYPE_PRIVATE(DMA_SLOT_MB02P,          device_dma_card_interface, bus::spectrum::dma_slot::mb02p_device,          "dma_mb02p",          "MB02+ 11(#0b) UA858D")
DEFINE_DEVICE_TYPE_PRIVATE(DMA_SLOT_MB02P_ZILOG,    device_dma_card_interface, bus::spectrum::dma_slot::mb02p_zilog_device,    "dma_mb02p_zilog",    "MB02+ 11(#0b) ZILOG")

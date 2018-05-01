// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HPC3 "High-performance Peripheral Controller" emulation

**********************************************************************/

#ifndef MAME_MACHINE_HPC3_H
#define MAME_MACHINE_HPC3_H

#pragma once

#include "cpu/mips/mips3.h"
#include "machine/ioc2.h"
#include "machine/wd33c93.h"

#define MCFG_SGI_HPC3_ADD(_tag)  \
	MCFG_DEVICE_ADD(_tag, SGI_HPC3, 0)

#define MCFG_HPC3_CPU_TAG(cpu_tag) \
	downcast<hpc3_device &>(*device).set_cpu_tag(cpu_tag);

#define MCFG_HPC3_SCSI_TAG(scsi_tag) \
	downcast<hpc3_device &>(*device).set_scsi_tag(scsi_tag);

#define MCFG_HPC3_IOC2_TAG(ioc2_tag) \
	downcast<hpc3_device &>(*device).set_ioc2_tag(ioc2_tag);

class hpc3_device : public device_t
{
public:
	hpc3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_cpu_tag(const char *tag) { m_maincpu.set_tag(tag); }
	void set_scsi_tag(const char *tag) { m_wd33c93.set_tag(tag); }
	void set_ioc2_tag(const char *tag) { m_ioc2.set_tag(tag); }

	DECLARE_READ32_MEMBER(hd_enet_r);
	DECLARE_WRITE32_MEMBER(hd_enet_w);
	DECLARE_READ32_MEMBER(hd0_r);
	DECLARE_WRITE32_MEMBER(hd0_w);
	DECLARE_READ32_MEMBER(pbus4_r);
	DECLARE_WRITE32_MEMBER(pbus4_w);
	DECLARE_READ32_MEMBER(pbusdma_r);
	DECLARE_WRITE32_MEMBER(pbusdma_w);
	DECLARE_READ32_MEMBER(unkpbus0_r);
	DECLARE_WRITE32_MEMBER(unkpbus0_w);

	DECLARE_WRITE_LINE_MEMBER(scsi_irq);

	TIMER_CALLBACK_MEMBER(do_dma);

protected:
	void device_start() override;
	void device_reset() override;
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void dump_chain(address_space &space, uint32_t ch_base);

	struct pbus_dma_t
	{
		uint8_t m_active;
		uint32_t m_cur_ptr;
		uint32_t m_desc_ptr;
		uint32_t m_next_ptr;
		uint32_t m_words_left;
	};

	enum
	{
		TIMER_DMA
	};

	enum
	{
		PBUS_CTRL_ENDIAN    = 0x00000002,
		PBUS_CTRL_RECV      = 0x00000004,
		PBUS_CTRL_FLUSH     = 0x00000008,
		PBUS_CTRL_DMASTART  = 0x00000010,
		PBUS_CTRL_LOAD_EN   = 0x00000020,
		PBUS_CTRL_REALTIME  = 0x00000040,
		PBUS_CTRL_HIGHWATER = 0x0000ff00,
		PBUS_CTRL_FIFO_BEG  = 0x003f0000,
		PBUS_CTRL_FIFO_END  = 0x3f000000,
	};

	enum
	{
		PBUS_DMADESC_EOX  = 0x80000000,
		PBUS_DMADESC_EOXP = 0x40000000,
		PBUS_DMADESC_XIE  = 0x20000000,
		PBUS_DMADESC_IPG  = 0x00ff0000,
		PBUS_DMADESC_TXD  = 0x00008000,
		PBUS_DMADESC_BC   = 0x00003fff,
	};

	enum
	{
		HPC3_DMACTRL_IRQ    = 0x01,
		HPC3_DMACTRL_ENDIAN = 0x02,
		HPC3_DMACTRL_DIR    = 0x04,
		HPC3_DMACTRL_ENABLE = 0x10,
	};

	required_device<mips3_device> m_maincpu;
	required_device<wd33c93_device> m_wd33c93;
	required_device<ioc2_device> m_ioc2;
	required_shared_ptr<uint32_t> m_mainram;
	required_shared_ptr<uint32_t> m_unkpbus0;

	uint32_t m_enetr_nbdp;
	uint32_t m_enetr_cbp;
	uint32_t m_unk0;
	uint32_t m_unk1;
	uint32_t m_ic_unk0;
	uint32_t m_scsi0_desc;
	uint32_t m_scsi0_dma_ctrl;
	pbus_dma_t m_pbus_dma;

	uint8_t m_dma_buffer[4096];

	inline void ATTR_PRINTF(3,4) verboselog(int n_level, const char *s_fmt, ... );
};

DECLARE_DEVICE_TYPE(SGI_HPC3, hpc3_device)

#endif // MAME_MACHINE_HAL2_H

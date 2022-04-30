// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI HPC1 "High-performance Peripheral Controller" emulation

**********************************************************************/

#ifndef MAME_MACHINE_HPC1_H
#define MAME_MACHINE_HPC1_H

#pragma once

#include "machine/dp8573.h"
#include "machine/eepromser.h"
#include "machine/pit8253.h"
#include "machine/wd33c9x.h"
#include "machine/z80scc.h"

class hpc1_device : public device_t
{
public:
	template <typename T, typename U>
	hpc1_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, U &&eeprom_tag)
		: hpc1_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_eeprom.set_tag(std::forward<U>(eeprom_tag));
	}

	hpc1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	DECLARE_WRITE_LINE_MEMBER(scsi_irq);
	DECLARE_WRITE_LINE_MEMBER(scsi_drq);

	void set_timer_int_clear(uint32_t data);
	DECLARE_WRITE_LINE_MEMBER(timer0_int);
	DECLARE_WRITE_LINE_MEMBER(timer1_int);
	DECLARE_WRITE_LINE_MEMBER(timer2_int);
	DECLARE_WRITE_LINE_MEMBER(duart0_int_w);
	DECLARE_WRITE_LINE_MEMBER(duart1_int_w);
	DECLARE_WRITE_LINE_MEMBER(duart2_int_w);

	void duart_int_w(int channel, int status);
	void raise_local_irq(int channel, uint8_t source_mask);
	void lower_local_irq(int channel, uint8_t source_mask);
	void update_irq(int channel);

	void do_scsi_dma();

	void dump_chain(uint32_t base);
	void fetch_chain();
	void decrement_chain();

	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<wd33c93_device> m_wd33c93;
	required_device_array<scc85c30_device, 3> m_scc;
	required_device<pit8254_device> m_pit;
	required_device<dp8573_device> m_rtc;

	enum
	{
		LOCAL0_FIFO_GIO0    = 0x01,
		LOCAL0_PARALLEL     = 0x02,
		LOCAL0_SCSI         = 0x04,
		LOCAL0_ETHERNET     = 0x08,
		LOCAL0_GFX_DMA      = 0x10,
		LOCAL0_DUART        = 0x20,
		LOCAL0_GIO1         = 0x40,
		LOCAL0_VME0         = 0x80,

		LOCAL1_GR1_CASE     = 0x02,
		LOCAL1_VME1         = 0x08,
		LOCAL1_DSP          = 0x10,
		LOCAL1_ACFAIL       = 0x20,
		LOCAL1_VIDEO        = 0x40,
		LOCAL1_RETRACE_GIO2 = 0x80
	};

	enum
	{
		HPC_DMACTRL_RESET   = 0x01,
		HPC_DMACTRL_FLUSH   = 0x02,
		HPC_DMACTRL_TO_MEM  = 0x10,
		HPC_DMACTRL_ENABLE  = 0x80
	};

	struct scsi_dma_t
	{
		uint32_t m_desc = 0;
		uint32_t m_addr = 0;
		uint32_t m_ctrl = 0;
		uint32_t m_length = 0;
		uint32_t m_next = 0;
		bool m_irq = false;
		bool m_drq = false;
		bool m_to_mem = false;
		bool m_active = false;
	};

	static void cdrom_config(device_t *device);
	static void scsi_devices(device_slot_interface &device);
	static void indigo_mice(device_slot_interface &device);
	void wd33c93(device_t *device);

	uint8_t m_misc_status = 0;
	uint32_t m_cpu_aux_ctrl = 0;
	uint32_t m_parbuf_ptr = 0;
	uint32_t m_local_int_status[2]{};
	uint32_t m_local_int_mask[2]{};
	bool m_int_status[2]{};
	uint32_t m_vme_int_mask[2]{};

	scsi_dma_t m_scsi_dma;

	uint8_t m_duart_int_status = 0;

	address_space *m_cpu_space = nullptr;

	static char const *const RS232A_TAG;
	static char const *const RS232B_TAG;

	static const XTAL SCC_PCLK;
	static const XTAL SCC_RXA_CLK;
	static const XTAL SCC_TXA_CLK;
	static const XTAL SCC_RXB_CLK;
	static const XTAL SCC_TXB_CLK;
};

DECLARE_DEVICE_TYPE(SGI_HPC1, hpc1_device)

#endif // MAME_MACHINE_HPC1_H

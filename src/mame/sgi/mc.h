// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    mc.h

    Silicon Graphics MC (Memory Controller) code

*********************************************************************/

#ifndef MAME_SGI_MC_H
#define MAME_SGI_MC_H

#pragma once

#include "machine/eepromser.h"

class sgi_mc_device : public device_t
{
public:
	template <typename T, typename U>
	sgi_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, U &&eeprom_tag, uint32_t clock)
		: sgi_mc_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_eeprom.set_tag(std::forward<U>(eeprom_tag));
	}
	sgi_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_dma_done_cb() { return m_int_dma_done_cb.bind(); }
	auto eisa_present() { return m_eisa_present.bind(); }

	uint32_t read(offs_t offset, uint32_t mem_mask = ~0);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void set_cpu_buserr(uint32_t address, uint64_t mem_mask);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	enum
	{
		MODE_TO_HOST    = (1 << 1),
		MODE_SYNC       = (1 << 2),
		MODE_FILL       = (1 << 3),
		MODE_DIR        = (1 << 4),
		MODE_SNOOP      = (1 << 5)
	};

	uint32_t dma_translate(uint32_t address);
	TIMER_CALLBACK_MEMBER(perform_dma);

	uint32_t get_line_count() { return m_dma_size >> 16; }
	uint32_t get_line_width() { return (uint16_t)m_dma_size; }
	uint32_t get_line_zoom() { return (m_dma_stride >> 16) & 0x3ff; }
	int16_t get_stride() { return (int16_t)m_dma_stride; }
	uint32_t get_zoom_count() { return (m_dma_count >> 16) & 0x3ff; }
	uint32_t get_byte_count() { return (uint16_t)m_dma_count; }

	void update_count();

	void memcfg_w(offs_t offset, u32 data);

	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	required_ioport m_simms;

	devcb_write_line m_int_dma_done_cb;
	devcb_read_line m_eisa_present;

	address_space *m_space;

	emu_timer *m_dma_timer;

	attotime m_last_update_time;

	uint32_t m_cpu_control[2];
	uint32_t m_watchdog;
	uint32_t m_sys_id;
	uint32_t m_rpss_divider;
	uint32_t m_refcnt_preload;
	uint32_t m_refcnt;
	uint32_t m_gio64_arb_param;
	uint32_t m_arb_cpu_time;
	uint32_t m_arb_burst_time;
	uint32_t m_memcfg[2];
	uint32_t m_cpu_mem_access_config;
	uint32_t m_gio_mem_access_config;
	uint32_t m_cpu_error_addr;
	uint32_t m_cpu_error_status;
	uint32_t m_gio_error_addr;
	uint32_t m_gio_error_status;
	uint32_t m_sys_semaphore;
	uint32_t m_gio_lock;
	uint32_t m_eisa_lock;
	uint32_t m_gio64_translate_mask;
	uint32_t m_gio64_substitute_bits;
	uint32_t m_dma_int_cause;
	uint32_t m_dma_control;
	uint32_t m_dma_tlb_entry_hi[4];
	uint32_t m_dma_tlb_entry_lo[4];
	uint32_t m_rpss_counter;
	uint32_t m_dma_mem_addr;
	uint32_t m_dma_size;
	uint32_t m_dma_stride;
	uint32_t m_dma_gio64_addr;
	uint32_t m_dma_mode;
	uint32_t m_dma_count;
	uint32_t m_dma_run;
	uint32_t m_eeprom_ctrl;
	uint32_t m_semaphore[16];

	std::unique_ptr<u8[]> m_ram[4];
};

DECLARE_DEVICE_TYPE(SGI_MC, sgi_mc_device)

#endif // MAME_SGI_MC_H

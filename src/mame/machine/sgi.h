// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    sgi.h

    Silicon Graphics MC (Memory Controller) code

*********************************************************************/

#ifndef MAME_MACHINE_SGI_H
#define MAME_MACHINE_SGI_H

#pragma once

#include "machine/eepromser.h"

class sgi_mc_device : public device_t
{
public:
	template <typename T, typename U>
	sgi_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu_tag, U &&eeprom_tag)
		: sgi_mc_device(mconfig, tag, owner, (uint32_t)0)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_eeprom.set_tag(std::forward<U>(eeprom_tag));
	}
	sgi_mc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_dma_done_cb() { return m_int_dma_done_cb.bind(); }

	DECLARE_READ32_MEMBER(read);
	DECLARE_WRITE32_MEMBER(write);

	void set_cpu_buserr(uint32_t address);
	uint32_t get_mem_config(int channel) const { return m_mem_config[channel]; }

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static const device_timer_id TIMER_RPSS = 0;
	static const device_timer_id TIMER_DMA = 1;

private:
	enum
	{
		MODE_TO_HOST	= (1 << 1),
		MODE_SYNC		= (1 << 2),
		MODE_FILL		= (1 << 3),
		MODE_DIR		= (1 << 4),
		MODE_SNOOP		= (1 << 5)
	};

	uint32_t dma_translate(uint32_t address);
	void dma_immediate();

	uint32_t get_line_count() { return m_dma_size >> 16; }
	uint32_t get_line_width() { return (uint16_t)m_dma_size; }
	uint32_t get_line_zoom() { return (m_dma_stride >> 16) & 0x3ff; }
	int16_t get_stride() { return (int16_t)m_dma_stride; }
	uint32_t get_zoom_count() { return (m_dma_count >> 16) & 0x3ff; }
	uint32_t get_byte_count() { return (uint16_t)m_dma_count; }

	required_device<cpu_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;

	devcb_write_line m_int_dma_done_cb;

	address_space *m_space;

	emu_timer *m_rpss_timer;
	emu_timer *m_dma_timer;

	uint32_t m_cpu_control[2];
	uint32_t m_watchdog;
	uint32_t m_sys_id;
	uint32_t m_rpss_divider;
	uint32_t m_refcnt_preload;
	uint32_t m_refcnt;
	uint32_t m_gio64_arb_param;
	uint32_t m_arb_cpu_time;
	uint32_t m_arb_burst_time;
	uint32_t m_mem_config[2];
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
	int m_rpss_divide_counter;
	int m_rpss_divide_count;
	uint8_t m_rpss_increment;
};

DECLARE_DEVICE_TYPE(SGI_MC, sgi_mc_device)


#endif // MAME_MACHINE_SGI_H

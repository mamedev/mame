// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    ARM IOMD device emulation

***************************************************************************/

#ifndef MAME_MACHINE_ARM_IOMD_H
#define MAME_MACHINE_ARM_IOMD_H

#pragma once

#include "cpu/arm7/arm7.h"
#include "machine/acorn_vidc.h"
#include "machine/at_keybc.h"
#include "bus/pc_kbd/pc_kbdc.h"
#include "bus/pc_kbd/keyboards.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> arm_iomd_device

class arm_iomd_device : public device_t
{
public:
	// construction/destruction
	arm_iomd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	arm_iomd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto iocr_read_od() { return m_iocr_read_od_cb[N].bind(); }
	template <unsigned N> auto iocr_write_od() { return m_iocr_write_od_cb[N].bind(); }
	auto iocr_read_id() { return m_iocr_read_id_cb.bind(); }
	auto iocr_write_id() { return m_iocr_write_id_cb.bind(); }
	// IRQA
	void vblank_irq(int state);
	// IRQB
	void keyboard_irq(int state);
	// DRQs
	void sound_drq(int state);
	// Reset
	void keyboard_reset(int state);

	// I/O operations
	virtual void map(address_map &map) ATTR_COLD;
	template<class T> void set_host_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	template<class T> void set_vidc_tag(T &&tag) { m_vidc.set_tag(std::forward<T>(tag)); }
	template<class T> void set_kbdc_tag(T &&tag) { m_kbdc.set_tag(std::forward<T>(tag)); }

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(timer_elapsed);

	void base_map(address_map &map) ATTR_COLD;
	u16 m_id;
	u8 m_version;

	enum {
		IRQA = 0,
		IRQB,
		IRQC,
		IRQD,
		IRQDMA,
		IRQ_SOURCES_SIZE
	};
	template <unsigned Which> u32 irqst_r();
	template <unsigned Which> u32 irqrq_r();
	template <unsigned Which> void irqrq_w(u32 data);
	template <unsigned Which> u32 irqmsk_r();
	template <unsigned Which> void irqmsk_w(u32 data);

	// TODO: convert to ARM7 device instead, enums shouldn't be public
	required_device<cpu_device> m_host_cpu;
	required_device<arm_vidc20_device> m_vidc;
	optional_device<ps2_keyboard_controller_device> m_kbdc;
	address_space *m_host_space; /**< reference to the host cpu space for DMA ops */
private:
	u8 m_iocr_ddr;

	devcb_read_line::array<2> m_iocr_read_od_cb;
	devcb_write_line::array<2> m_iocr_write_od_cb;
	devcb_read_line m_iocr_read_id_cb;
	devcb_write_line m_iocr_write_id_cb;

	u32 iocr_r();
	void iocr_w(u32 data);

	u32 kbddat_r();
	void kbddat_w(u32 data);
	u32 kbdcr_r();
	void kbdcr_w(u32 data);

	u32 m_vidinita, m_vidend;
	bool m_vidlast, m_videqual;
	bool m_video_enable;
	u32 vidcr_r();
	void vidcr_w(u32 data);
	u32 vidend_r();
	void vidend_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 vidinita_r();
	void vidinita_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 m_cursinit;
	bool m_cursor_enable;
	u32 cursinit_r();
	void cursinit_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 m_sndcur;
	u32 m_sndend;
	u32 m_sndcur_reg[2];
	u32 m_sndend_reg[2];
	bool m_sndstop_reg[2];
	bool m_sndlast_reg[2];
	bool m_sndbuffer_ok[2];
	bool m_sound_dma_on;
	u8 m_sndcur_buffer;
	inline void sounddma_swap_buffer();
	template <unsigned Which> u32 sdcur_r();
	template <unsigned Which> void sdcur_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <unsigned Which> u32 sdend_r();
	template <unsigned Which> void sdend_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 sdcr_r();
	void sdcr_w(u32 data);
	u32 sdst_r();

	u8 m_irq_status[IRQ_SOURCES_SIZE], m_irq_mask[IRQ_SOURCES_SIZE];
	inline u8 update_irqa_type(u8 data);
	inline void flush_irq(unsigned Which);
	template <unsigned Which> inline void trigger_irq(u8 irq_type);

	inline void trigger_timer(unsigned Which);
	u16 m_timer_in[2];
	u16 m_timer_out[2];
	int m_timer_counter[2];
	u8  m_timer_readinc[2];
	emu_timer *m_timer[2];

	template <unsigned Which> u32 tNlow_r();
	template <unsigned Which> u32 tNhigh_r();
	template <unsigned Which> void tNlow_w(u32 data);
	template <unsigned Which> void tNhigh_w(u32 data);
	template <unsigned Which> void tNgo_w(u32 data);
	template <unsigned Which> void tNlatch_w(u32 data);

	template <unsigned Nibble> u32 id_r();
	u32 version_r();

	// used in vidcr_r / sndcr_r, documentation hints this is a purged idea during chip development, to be checked out
	static constexpr u8 dmaid_size = 0x10; // qword transfer
//  constexpr u8 dmaid_mask = 0x1f;
};

class arm7500fe_iomd_device : public arm_iomd_device
{
public:
	// construction/destruction
	arm7500fe_iomd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override ATTR_COLD;
	auto iolines_read() { return m_iolines_read_cb.bind(); }
	auto iolines_write() { return m_iolines_write_cb.bind(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
private:
	devcb_read8 m_iolines_read_cb;
	devcb_write8 m_iolines_write_cb;

	bool m_cpuclk_divider, m_memclk_divider, m_ioclk_divider;
	inline void refresh_host_cpu_clocks();
	u32 clkctl_r();
	void clkctl_w(u32 data);

	u8 m_iolines_ddr;
	u32 iolines_r();
	void iolines_w(u32 data);

	u32 msecr_r();
	void msecr_w(u32 data);
};

// device type definition
DECLARE_DEVICE_TYPE(ARM_IOMD, arm_iomd_device)
DECLARE_DEVICE_TYPE(ARM7500FE_IOMD, arm7500fe_iomd_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_ARM_IOMD_H

// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    ARM IOMD device emulation

***************************************************************************/

#ifndef MAME_MACHINE_ARM_IOMD_H
#define MAME_MACHINE_ARM_IOMD_H

#pragma once

#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
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
	DECLARE_WRITE_LINE_MEMBER( vblank_irq );
	// IRQB
	DECLARE_WRITE_LINE_MEMBER( keyboard_irq );
	// DRQs
	DECLARE_WRITE_LINE_MEMBER( sound_drq );
	// Reset
	DECLARE_WRITE_LINE_MEMBER( keyboard_reset );

	// I/O operations
	virtual void map(address_map &map);
	template<class T> void set_host_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	template<class T> void set_vidc_tag(T &&tag) { m_vidc.set_tag(std::forward<T>(tag)); }
	template<class T> void set_kbdc_tag(T &&tag) { m_kbdc.set_tag(std::forward<T>(tag)); }

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void base_map(address_map &map);
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
	template <unsigned Which> DECLARE_READ32_MEMBER( irqst_r );
	template <unsigned Which> DECLARE_READ32_MEMBER( irqrq_r );
	template <unsigned Which> DECLARE_WRITE32_MEMBER( irqrq_w );
	template <unsigned Which> DECLARE_READ32_MEMBER( irqmsk_r );
	template <unsigned Which> DECLARE_WRITE32_MEMBER( irqmsk_w );

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

	DECLARE_READ32_MEMBER( iocr_r );
	DECLARE_WRITE32_MEMBER( iocr_w );

	DECLARE_READ32_MEMBER( kbddat_r );
	DECLARE_WRITE32_MEMBER( kbddat_w );
	DECLARE_READ32_MEMBER( kbdcr_r );
	DECLARE_WRITE32_MEMBER( kbdcr_w );

	u32 m_vidinita, m_vidend;
	bool m_vidlast, m_videqual;
	bool m_video_enable;
	DECLARE_READ32_MEMBER( vidcr_r );
	DECLARE_WRITE32_MEMBER( vidcr_w );
	DECLARE_READ32_MEMBER( vidend_r );
	DECLARE_WRITE32_MEMBER( vidend_w );
	DECLARE_READ32_MEMBER( vidinita_r );
	DECLARE_WRITE32_MEMBER( vidinita_w );
	u32 m_cursinit;
	bool m_cursor_enable;
	DECLARE_READ32_MEMBER( cursinit_r );
	DECLARE_WRITE32_MEMBER( cursinit_w );

	static constexpr int sounddma_ch_size = 2;
	u32 m_sndcur, m_sndend;
	u32 m_sndcur_reg[sounddma_ch_size], m_sndend_reg[sounddma_ch_size];
	bool m_sndstop_reg[sounddma_ch_size], m_sndlast_reg[sounddma_ch_size];
	bool m_sndbuffer_ok[sounddma_ch_size];
	bool m_sound_dma_on;
	u8 m_sndcur_buffer;
	bool m_snd_overrun, m_snd_int;
	inline void sounddma_swap_buffer();
	template <unsigned Which> DECLARE_READ32_MEMBER( sdcur_r );
	template <unsigned Which> DECLARE_WRITE32_MEMBER( sdcur_w );
	template <unsigned Which> DECLARE_READ32_MEMBER( sdend_r );
	template <unsigned Which> DECLARE_WRITE32_MEMBER( sdend_w );
	DECLARE_READ32_MEMBER( sdcr_r );
	DECLARE_WRITE32_MEMBER( sdcr_w );
	DECLARE_READ32_MEMBER( sdst_r );

	u8 m_irq_status[IRQ_SOURCES_SIZE], m_irq_mask[IRQ_SOURCES_SIZE];
	inline u8 update_irqa_type(u8 data);
	inline void flush_irq(unsigned Which);
	template <unsigned Which> inline void trigger_irq(u8 irq_type);

	static constexpr int timer_ch_size = 2;
	enum {
		T0_TIMER = 1,
		T1_TIMER
	};
	inline void trigger_timer(unsigned Which);
	u16 m_timer_in[2];
	u16 m_timer_out[2];
	int m_timer_counter[2];
	u8  m_timer_readinc[2];
	emu_timer *m_timer[2];

	template <unsigned Which> DECLARE_READ32_MEMBER( tNlow_r );
	template <unsigned Which> DECLARE_READ32_MEMBER( tNhigh_r );
	template <unsigned Which> DECLARE_WRITE32_MEMBER( tNlow_w );
	template <unsigned Which> DECLARE_WRITE32_MEMBER( tNhigh_w );
	template <unsigned Which> DECLARE_WRITE32_MEMBER( tNgo_w );
	template <unsigned Which> DECLARE_WRITE32_MEMBER( tNlatch_w );

	template <unsigned Nibble> DECLARE_READ32_MEMBER( id_r );
	DECLARE_READ32_MEMBER( version_r );

	// used in vidcr_r / sndcr_r, documentation hints this is a purged idea during chip development, to be checked out
	static constexpr u8 dmaid_size = 0x10; // qword transfer
//  constexpr u8 dmaid_mask = 0x1f;
};

class arm7500fe_iomd_device : public arm_iomd_device
{
public:
	// construction/destruction
	arm7500fe_iomd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
	auto iolines_read() { return m_iolines_read_cb.bind(); }
	auto iolines_write() { return m_iolines_write_cb.bind(); }

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
private:
	devcb_read8 m_iolines_read_cb;
	devcb_write8 m_iolines_write_cb;

	bool m_cpuclk_divider, m_memclk_divider, m_ioclk_divider;
	inline void refresh_host_cpu_clocks();
	DECLARE_READ32_MEMBER( clkctl_r );
	DECLARE_WRITE32_MEMBER( clkctl_w );

	u8 m_iolines_ddr;
	DECLARE_READ32_MEMBER( iolines_r );
	DECLARE_WRITE32_MEMBER( iolines_w );

	DECLARE_READ32_MEMBER( msecr_r );
	DECLARE_WRITE32_MEMBER( msecr_w );
};

// device type definition
DECLARE_DEVICE_TYPE(ARM_IOMD, arm_iomd_device)
DECLARE_DEVICE_TYPE(ARM7500FE_IOMD, arm7500fe_iomd_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_MACHINE_ARM_IOMD_H

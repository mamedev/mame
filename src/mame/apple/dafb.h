// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_APPLE_DAFB_H
#define MAME_APPLE_DAFB_H

#pragma once

#include "cpu/m68000/m68040.h"
#include "machine/ncr53c90.h"

#include "emupal.h"
#include "screen.h"

class dafb_base : public device_t
{
public:
	dafb_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
	virtual ~dafb_base() = default;

	void map(address_map &map) ATTR_COLD;

	u32 vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask);

	auto dafb_irq() { return m_irq.bind(); }

	virtual u32 dafb_r(offs_t offset);
	virtual void dafb_w(offs_t offset, u32 data);
	u32 swatch_r(offs_t offset);
	void swatch_w(offs_t offset, u32 data);
	virtual u32 ramdac_r(offs_t offset);
	virtual void ramdac_w(offs_t offset, u32 data);
	virtual u8 clockgen_r(offs_t offset);
	virtual void clockgen_w(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	// Registers are matched on their byte offset with DAFB's 4-byte packing.  Derivatives on a
	// PowerPC bus space the same register file 16 bytes apart, which m_reg_shift normalizes away.
	u32 reg_offset(offs_t offset) const { return (offset >> m_reg_shift) << 2; }

	// The pixel clock divider comes out of the RAMDAC, which isn't the same part on every derivative.
	virtual int clock_divider() const { return 1 << ((m_ac842_pbctrl & 0x60) >> 5); }

	u8 read_monitor_sense();
	void recalc_ints();
	virtual void recalc_mode();

	u32 m_vram_size;
	int m_dafb_version;
	u32 m_pixel_clock;
	u8 m_reg_shift;

	u8 m_pal_address, m_pal_idx, m_ac842_pbctrl, m_mode;

	// the Swatch CRTC state and the frame buffer itself are shared with derivatives that
	// substitute their own RAMDAC and pixel readout
	required_device<palette_device> m_palette;
	std::unique_ptr<u32[]> m_vram;
	u8 m_timing_control, m_monitor_id;
	u32 m_base, m_stride;
	u8 m_swatch_mode;
	u32 m_hres, m_vres, m_config;

private:
	required_device<screen_device> m_screen;
	required_ioport m_monitor_config;
	devcb_write_line m_irq;

	enum
	{
		HSERR   = 0,
		HLFLN,
		HEQ,
		HSP,
		HBWAY,
		HBRST,
		HBP,
		HAL,
		HFP,
		HPIX
	};

	enum
	{
		VHLINE = 0,
		VSYNC,
		VBPEQ,
		VBP,
		VAL,
		VFP,
		VFPEQ
	};

	emu_timer *m_vbl_timer, *m_cursor_timer;
	u32 m_vram_offset;
	u32 m_test;
	u32 m_horizontal_params[10], m_vertical_params[7];
	u16 m_cursor_line, m_anim_line;
	s32 m_int_status;
	u8 m_dp8531_regs[16];
	u32 m_htotal, m_vtotal, m_block_control, m_swatch_test;

	TIMER_CALLBACK_MEMBER(vbl_tick);
	TIMER_CALLBACK_MEMBER(cursor_tick);
};

// Discrete DAFB II: Quadra 950, includes "TurboSCSI"
class dafb_device: public dafb_base
{
public:
	dafb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }

	void set_turboscsi1_device(ncr53c94_device *device) { m_ncr[0] = device; }
	void set_turboscsi2_device(ncr53c94_device *device) { m_ncr[1] = device; }

	template <int bus> void turboscsi_drq_w(int state);

	template <int bus> u8 turboscsi_r(offs_t offset);
	template <int bus> void turboscsi_w(offs_t offset, u8 data);
	template <int bus> u16 turboscsi_dma_r(offs_t offset, u16 mem_mask);
	template <int bus> void turboscsi_dma_w(offs_t offset, u16 data, u16 mem_mask);

	virtual u32 dafb_r(offs_t offset) override;
	virtual void dafb_w(offs_t offset, u32 data) override;

protected:
	dafb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;

private:
	required_device<m68000_musashi_device> m_maincpu;
	ncr53c94_device *m_ncr[2];
	u16 m_scsi_ctrl[2];
	int m_drq[2];
	int m_scsi_read_cycles[2], m_scsi_write_cycles[2], m_scsi_dma_read_cycles[2], m_scsi_dma_write_cycles[2];
};

class dafb_q950_device : public dafb_device
{
public:
	dafb_q950_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u32 ramdac_r(offs_t offset) override;
	virtual void ramdac_w(offs_t offset, u32 data) override;

private:
	u8 m_pcbr1;
};

class dafb_memc_device: public dafb_base
{
public:
	dafb_memc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u8 clockgen_r(offs_t offset) override;
	virtual void clockgen_w(offs_t offset, u8 data) override;
	virtual u32 ramdac_r(offs_t offset) override;
	virtual void ramdac_w(offs_t offset, u32 data) override;

private:
	u8 m_pcbr1;
	u64 m_clock_shift;
	u64 m_clock_params;
};

class dafb_memcjr_device: public dafb_base
{
public:
	dafb_memcjr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual u8 clockgen_r(offs_t offset) override;
	virtual void clockgen_w(offs_t offset, u8 data) override;
	virtual u32 ramdac_r(offs_t offset) override;
	virtual void ramdac_w(offs_t offset, u32 data) override;

private:
	u8 m_pcbr1;
	u8 m_last_clock;
	u32 m_clock_shift;
	s32 m_bit_clock;
	u8 m_M, m_N, m_P;
	u32 m_mclk, m_pclk;
};

DECLARE_DEVICE_TYPE(DAFB, dafb_device)
DECLARE_DEVICE_TYPE(DAFB_Q950, dafb_q950_device)
DECLARE_DEVICE_TYPE(DAFB_MEMC, dafb_memc_device)
DECLARE_DEVICE_TYPE(DAFB_MEMCJR, dafb_memcjr_device)

#endif  /* MAME_APPLE_DAFB_H */

// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_BUS_INTERPRO_SR_EDGE_H
#define MAME_BUS_INTERPRO_SR_EDGE_H

#pragma once

#include "sr.h"

#include "screen.h"
#include "machine/ram.h"
#include "cpu/tms32031/tms32031.h"
#include "video/bt45x.h"
#include "machine/z80scc.h"
#include "bus/interpro/keyboard/keyboard.h"

class edge1_device_base : public device_t, public device_srx_card_interface
{
public:
	DECLARE_WRITE_LINE_MEMBER(holda);
	DECLARE_WRITE_LINE_MEMBER(vblank);

protected:
	edge1_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
	virtual void map_dynamic(address_map &map);
	virtual void device_start() override;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(scc_irq);

	DECLARE_READ32_MEMBER(reg0_r);
	DECLARE_WRITE32_MEMBER(reg0_w) { COMBINE_DATA(&m_reg0); }

	DECLARE_READ32_MEMBER(control_r) { return m_control; };
	DECLARE_WRITE32_MEMBER(control_w);
	DECLARE_READ32_MEMBER(status_r) { return m_status; };
	DECLARE_WRITE32_MEMBER(status_w) { COMBINE_DATA(&m_status); }
	DECLARE_READ32_MEMBER(fifo_r) { return m_fifo; };
	DECLARE_WRITE32_MEMBER(fifo_w) { COMBINE_DATA(&m_fifo); }
	DECLARE_READ32_MEMBER(kernel_r) { return m_kernel; };
	DECLARE_WRITE32_MEMBER(kernel_w);

	DECLARE_READ32_MEMBER(attention_r) { return m_attention; };
	DECLARE_WRITE32_MEMBER(attention_w) { COMBINE_DATA(&m_attention); }

	DECLARE_WRITE32_MEMBER(ififo_lwm_w) { COMBINE_DATA(&m_ififo_lwm); }
	DECLARE_WRITE32_MEMBER(ififo_hwm_w) { COMBINE_DATA(&m_ififo_hwm); }

	DECLARE_READ32_MEMBER(srx_master_control_r) { return m_srx_master_control; }
	DECLARE_WRITE32_MEMBER(srx_master_control_w) { COMBINE_DATA(&m_srx_master_control); }

	required_device<screen_device> m_screen;
	required_device<ram_device> m_sram;
	required_device<ram_device> m_vram;
	required_device<tms3203x_device> m_dsp;
	required_device<bt458_device> m_ramdac;
	required_device<z80scc_device> m_scc;

private:
	u32 m_reg0;

	u32 m_control;
	u32 m_status;
	u32 m_fifo;
	u32 m_kernel;
	u32 m_attention;

	u32 m_ififo_lwm;
	u32 m_ififo_hwm;

	u32 m_srx_master_control;
};

class edge2_processor_device_base : public device_t, public device_srx_card_interface
{
protected:
	edge2_processor_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	virtual void device_start() override {};
};

class edge2_framebuffer_device_base : public device_t, public device_srx_card_interface
{
protected:
	edge2_framebuffer_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	virtual void device_start() override {};
};

class edge2plus_processor_device_base : public device_t, public device_srx_card_interface
{
public:
	void register_screen(screen_device *screen, ram_device *ram) { m_screen = screen; m_sram = ram; }
	required_device<tms3203x_device> m_dsp1;

protected:
	edge2plus_processor_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	void dsp1_map(address_map &map);

	virtual void device_start() override {}

	DECLARE_WRITE_LINE_MEMBER(holda);
	DECLARE_WRITE_LINE_MEMBER(scc_irq);

	DECLARE_READ32_MEMBER(control_r) { return m_control; };
	DECLARE_WRITE32_MEMBER(control_w);
	DECLARE_READ32_MEMBER(status_r) { return m_status; };
	DECLARE_WRITE32_MEMBER(status_w) { COMBINE_DATA(&m_status); }
	DECLARE_READ32_MEMBER(kernel_r) { return m_kernel; };
	DECLARE_WRITE32_MEMBER(kernel_w);
	DECLARE_WRITE32_MEMBER(mapping_w) { COMBINE_DATA(&m_mapping); }
	DECLARE_READ32_MEMBER(attention_r) { return m_attention; };
	DECLARE_WRITE32_MEMBER(attention_w) { COMBINE_DATA(&m_attention); }

	DECLARE_WRITE32_MEMBER(ififo_lwm_w) { COMBINE_DATA(&m_ififo_lwm); }
	DECLARE_WRITE32_MEMBER(ififo_hwm_w) { COMBINE_DATA(&m_ififo_hwm); }

	DECLARE_READ32_MEMBER(reg0_r);
	DECLARE_WRITE32_MEMBER(reg0_w) { COMBINE_DATA(&m_reg0); }


private:
	u32 m_control;
	u32 m_status;
	u32 m_attention;
	u32 m_mapping;
	u32 m_kernel;

	u32 m_ififo_lwm;
	u32 m_ififo_hwm;

	u32 m_reg0;

	screen_device *m_screen;
	ram_device *m_sram;
};

class edge2plus_framebuffer_device_base : public device_t, public device_srx_card_interface
{
protected:
	friend class edge2plus_processor_device_base;

	edge2plus_framebuffer_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;
	virtual void device_start() override;

	virtual void map_dynamic(address_map &map);

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE32_MEMBER(lut_select_w);

	DECLARE_WRITE32_MEMBER(unk_300_w) { m_unk_304 = 0x1000; }
	DECLARE_READ32_MEMBER(unk_304_r) { m_unk_304 ^= 0x1000; return m_unk_304; }

	DECLARE_WRITE32_MEMBER(select_w) { COMBINE_DATA(&m_select); }

	required_device<screen_device> m_screen;
	required_device<ram_device> m_sram;
	required_device<ram_device> m_vram;
	required_device_array<bt457_device, 3> m_ramdac;

private:
	u32 m_unk_304;
	u32 m_select;
};

class mpcb828_device : public edge1_device_base
{
public:
	mpcb828_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

class mpcb849_device : public edge1_device_base
{
public:
	mpcb849_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

class mpcb030_device : public edge2_processor_device_base
{
public:
	mpcb030_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

class msmt094_device : public edge2plus_processor_device_base
{
public:
	msmt094_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

class mpcba63_device : public edge2_framebuffer_device_base
{
public:
	mpcba63_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

class mpcb896_device : public edge2plus_framebuffer_device_base
{
public:
	mpcb896_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
};

// device type definition
DECLARE_DEVICE_TYPE(MPCB828, mpcb828_device)
DECLARE_DEVICE_TYPE(MPCB849, mpcb849_device)
DECLARE_DEVICE_TYPE(MPCB030, mpcb030_device)
DECLARE_DEVICE_TYPE(MPCBA63, mpcba63_device)
DECLARE_DEVICE_TYPE(MSMT094, msmt094_device)
DECLARE_DEVICE_TYPE(MPCB896, mpcb896_device)

#endif // MAME_BUS_INTERPRO_SR_EDGE_H

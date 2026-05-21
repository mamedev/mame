// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_NINTENDO_NES_VT4FFX_SOC_H
#define MAME_NINTENDO_NES_VT4FFX_SOC_H

#pragma once

#include "nes_vt_soc.h"

class vt4ffx_soc_base_device : public nes_vt02_vt03_soc_device
{
public:
	auto io_4139_read_callback() { return m_io_413x_read_callback.bind(); }
	auto io_4139_write_callback() { return m_io_413x_write_callback.bind(); }

protected:
	vt4ffx_soc_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	void vt369_map(address_map &map) ATTR_COLD;

	u8 vt_413x_port_direction_r();
	void vt_413x_port_direction_w(u8 data);
	void vt_413x_port_out_w(u8 data);
	u8 vt_413x_port_in_r();

	u8 vt369_6000_r(offs_t offset);
	void vt369_6000_w(offs_t offset, u8 data);

	void vt369_4112_bank6000_select_w(u8 data);
	void vt369_411c_bank6000_enable_w(u8 data);
	void vt369_411d_w(u8 data);
	void vt369_411e_w(u8 data);

	void vt4ffx_ctrl_w(u8 data);
	void vt4ffx_data_w(offs_t offset, u8 data);

private:
	u8 vt3xx_palette_r(offs_t offset);
	void vt3xx_palette_w(offs_t offset, u8 data);

	u8 read_onespace_bus_with_relative_offset(offs_t offset);

	virtual void vt_dma_w(u8 data) override;

	std::vector<u8> m_6000_ram;

	u8 m_bank6000 = 0;
	u8 m_bank6000_enable = 0;

	u8 m_413x_port_direction;
	u8 m_413x_port_data;

	u8 m_4ffx_ctrl;
	u16 m_4ffx_data;

	devcb_write8 m_io_413x_write_callback;
	devcb_read8 m_io_413x_read_callback;
};

class vt4ffx_soc_noswap_device : public vt4ffx_soc_base_device
{
public:
	vt4ffx_soc_noswap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	vt4ffx_soc_noswap_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	bool m_encryption_allowed;
};

class vt4ffx_soc_vibesswap_device : public vt4ffx_soc_noswap_device
{
public:
	vt4ffx_soc_vibesswap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	vt4ffx_soc_vibesswap_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void vibes_411c_w(u8 data);

	void nes_vt_vibes_map(address_map &map) ATTR_COLD;
};

class vt4ffx_soc_gbox2020_device : public vt4ffx_soc_vibesswap_device
{
public:
	vt4ffx_soc_gbox2020_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void gbox_411c_w(u8 data);

	void nes_vt_gbox_map(address_map &map) ATTR_COLD;
};

class vt4ffx_soc_s10swap_device : public vt4ffx_soc_vibesswap_device
{
public:
	vt4ffx_soc_s10swap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class vt4ffx_soc_rsps300swap_device : public vt4ffx_soc_noswap_device
{
public:
	vt4ffx_soc_rsps300swap_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	void rsps_411c_w(u8 data);

	void nes_vt_rsps_map(address_map &map) ATTR_COLD;

};


DECLARE_DEVICE_TYPE(VT4FFX_SOC_NOSWAP, vt4ffx_soc_noswap_device)
DECLARE_DEVICE_TYPE(VT4FFX_SOC_VIBESSWAP, vt4ffx_soc_vibesswap_device)
DECLARE_DEVICE_TYPE(VT4FFX_SOC_GBOX2020, vt4ffx_soc_gbox2020_device)
DECLARE_DEVICE_TYPE(VT4FFX_SOC_S10SWAP, vt4ffx_soc_s10swap_device)
DECLARE_DEVICE_TYPE(VT4FFX_SOC_RSPS300SWAP, vt4ffx_soc_rsps300swap_device)

#endif // MAME_NINTENDO_NES_VT4FFX_SOC_H

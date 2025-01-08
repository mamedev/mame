// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    sh_port.h

    SH i/o ports

***************************************************************************/

#ifndef MAME_CPU_SH_SH_PORT_H
#define MAME_CPU_SH_SH_PORT_H

#pragma once

class sh7042_device;

class sh_port16_device : public device_t {
public:
	sh_port16_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template<typename T> sh_port16_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, int index, u16 default_io, u16 mask) :
		sh_port16_device(mconfig, tag, owner)
	{
		m_index = index;
		m_default_io = default_io;
		m_mask = mask;
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	u16 dr_r();
	void dr_w(offs_t, u16 data, u16 mem_mask);
	u16 io_r();
	void io_w(offs_t, u16 data, u16 mem_mask);

protected:
	required_device<sh7042_device> m_cpu;
	int m_index;
	u16 m_default_io, m_mask;
	u16 m_dr, m_io;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

class sh_port32_device : public device_t {
public:
	sh_port32_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	template<typename T> sh_port32_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cpu, int index, u32 default_io, u32 mask) :
		sh_port32_device(mconfig, tag, owner)
	{
		m_index = index;
		m_default_io = default_io;
		m_mask = mask;
		m_cpu.set_tag(std::forward<T>(cpu));
	}

	u32 dr_r();
	void dr_w(offs_t, u32 data, u32 mem_mask);
	u32 io_r();
	void io_w(offs_t, u32 data, u32 mem_mask);

protected:
	required_device<sh7042_device> m_cpu;
	int m_index;
	u32 m_default_io, m_mask;
	u32 m_dr, m_io;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SH_PORT16, sh_port16_device)
DECLARE_DEVICE_TYPE(SH_PORT32, sh_port32_device)

#endif // MAME_CPU_SH_SH_PORT_H

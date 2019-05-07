// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

  SunPlus "GCM394" (based on die pictures)

**********************************************************************/

#ifndef MAME_MACHINE_SUNPLUS_GCM394_H
#define MAME_MACHINE_SUNPLUS_GCM394_H

#pragma once

#include "cpu/unsp/unsp.h"
#include "screen.h"

class sunplus_gcm394_base_device : public device_t, public device_mixer_interface
{
public:
	sunplus_gcm394_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_mixer_interface(mconfig, *this, 2)
	, m_cpu(*this, finder_base::DUMMY_TAG)
	, m_screen(*this, finder_base::DUMMY_TAG)
	{
	}

	void map(address_map &map);

	DECLARE_WRITE_LINE_MEMBER(vblank) { /*m_spg_video->vblank(state);*/ }
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect) { return 0; /* m_spg_video->screen_update(screen, bitmap, cliprect);*/ }

protected:

	virtual void device_start() override;
	virtual void device_reset() override;

	required_device<unsp_device> m_cpu;
	required_device<screen_device> m_screen;

	uint16_t m_78fb;
	uint16_t m_dma_params[7];

private:
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_WRITE16_MEMBER(unk_w);

};

class sunplus_gcm394_device : public sunplus_gcm394_base_device
{
public:
	template <typename T, typename U>
	sunplus_gcm394_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock, T&& cpu_tag, U&& screen_tag)
		: sunplus_gcm394_device(mconfig, tag, owner, clock)
	{
		m_cpu.set_tag(std::forward<T>(cpu_tag));
		m_screen.set_tag(std::forward<U>(screen_tag));
	}

	sunplus_gcm394_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

	virtual void device_add_mconfig(machine_config& config) override;
};


DECLARE_DEVICE_TYPE(GCM394, sunplus_gcm394_device)

#endif // MAME_MACHINE_SUNPLUS_GCM394_H

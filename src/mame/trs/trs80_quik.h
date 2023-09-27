// license:BSD-3-Clause
// copyright-holders:Robbbert

#ifndef MAME_TRS_TRS80_QUIK_H
#define MAME_TRS_TRS80_QUIK_H

#pragma once

#include "imagedev/snapquik.h"

class trs80_quickload_device : public snapshot_image_device
{
public:
	template <typename T>
	trs80_quickload_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cputag, attotime delay = attotime::zero)
		: trs80_quickload_device(mconfig, tag, owner, 0U)
	{
		set_cpu(cputag);
		set_delay(delay);
	}
	trs80_quickload_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0U);

	template <typename T> void set_cpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }

protected:
	virtual const char *image_interface() const noexcept override { return "trs80_quik"; }
	virtual const char *file_extensions() const noexcept override { return "cmd"; }
	virtual const char *image_type_name() const noexcept override { return "quickload"; }
	virtual const char *image_brief_type_name() const noexcept override { return "quik"; }

private:
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	required_device<cpu_device> m_maincpu;
};

DECLARE_DEVICE_TYPE(TRS80_QUICKLOAD, trs80_quickload_device)

#endif // MAME_TRS_TRS80_QUIK_H

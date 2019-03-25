// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_VIDEO_DECSFB_H
#define MAME_VIDEO_DECSFB_H

#pragma once

class decsfb_device : public device_t
{
public:
	decsfb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_cb() { return m_int_cb.bind(); }

	DECLARE_READ32_MEMBER( read );
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( vram_r );
	DECLARE_WRITE32_MEMBER( vram_w );

	u32 *get_vram() { return m_vram; }

protected:
	// standard device_interface overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	devcb_write_line m_int_cb;

private:
	u32 m_vram[0x200000/4];
	u32 m_regs[0x80];
	int m_copy_src;
};

DECLARE_DEVICE_TYPE(DECSFB, decsfb_device)

#endif // MAME_VIDEO_DECSFB_H

// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_DEC_SFB_H
#define MAME_DEC_SFB_H

#pragma once

class decsfb_device : public device_t
{
public:
	decsfb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_cb() { return m_int_cb.bind(); }

	u32 read(offs_t offset);
	void write(offs_t offset, u32 data, u32 mem_mask = ~0);
	uint32_t vram_r(offs_t offset);
	void vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 *get_vram() { return m_vram; }

protected:
	// standard device_interface overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	devcb_write_line m_int_cb;

private:
	u32 m_vram[0x200000/4];
	u32 m_regs[0x80];
	int m_copy_src;
};

DECLARE_DEVICE_TYPE(DECSFB, decsfb_device)

#endif // MAME_DEC_SFB_H

// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_BUS_INTERPRO_SR_GT_H
#define MAME_BUS_INTERPRO_SR_GT_H

#pragma once

#include "sr.h"
#include "video/bt459.h"

class gt_device_base : public sr_card_device_base
{
protected:
	gt_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

public:
	static const u32 GT_PIXCLOCK    = 80'000'000; // just a guess
	static const int GT_XPIXELS     = 1184;
	static const int GT_YPIXELS     = 884;
	static const int GT_BUFFER_SIZE = 0x100000;   // 1 megabyte
	static const int GT_VRAM_SIZE   = 0x200000;   // 1 megabyte double buffered

	enum control_mask
	{
		CONTROL_SCREEN = 0x1000, // possibly selects screen 0 or 1?
		CONTROL_BUSY   = 0x8000
	};

protected:
	typedef struct
	{
		required_device<bt459_device> ramdac;
		std::unique_ptr<u8[]> vram;

		bool primary;
	} gt_screen_t;

	virtual DECLARE_READ16_MEMBER(control_r) = 0;
	virtual DECLARE_WRITE16_MEMBER(control_w) = 0;
	virtual DECLARE_READ32_MEMBER(vram_r) = 0;
	virtual DECLARE_WRITE32_MEMBER(vram_w) = 0;
};

class mpcb963_device : public gt_device_base
{
public:
	mpcb963_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static const int GT_SCREEN_COUNT = 1;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	virtual void map(address_map &map) override;

	virtual DECLARE_READ16_MEMBER(control_r) override { return m_control; }
	virtual DECLARE_WRITE16_MEMBER(control_w) override;
	virtual DECLARE_READ32_MEMBER(vram_r) override;
	virtual DECLARE_WRITE32_MEMBER(vram_w) override;

	u32 screen_update0(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	u16 m_control;

	gt_screen_t m_screen[GT_SCREEN_COUNT];
};

class mpcba79_device : public gt_device_base
{
public:
	mpcba79_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static const int GT_SCREEN_COUNT = 2;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	u32 screen_update0(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update1(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void map(address_map &map) override;

	virtual DECLARE_READ16_MEMBER(control_r) override { return m_control; }
	virtual DECLARE_WRITE16_MEMBER(control_w) override;
	virtual DECLARE_READ32_MEMBER(vram_r) override;
	virtual DECLARE_WRITE32_MEMBER(vram_w) override;

private:
	u16 m_control;
	gt_screen_t m_screen[GT_SCREEN_COUNT];
};

// device type definition
DECLARE_DEVICE_TYPE(MPCB963, mpcb963_device)
DECLARE_DEVICE_TYPE(MPCBA79, mpcba79_device)

#endif // MAME_BUS_INTERPRO_SR_GT_H

// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller Dot Matrix Display

// A 128x32 plasma display with 16 pages and refreshed at 240Hz (for PWM luminosity control)

#ifndef WPC_DMD_H
#define WPC_DMD_H

#define MCFG_WPC_DMD_ADD( _tag, _scanline_cb ) \
	MCFG_DEVICE_ADD( _tag, WPC_DMD, 0 ) \
	devcb = &wpc_dmd_device::set_scanline_cb(*device, DEVCB_##_scanline_cb);

class wpc_dmd_device : public device_t
{
public:
	wpc_dmd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	virtual ~wpc_dmd_device();

	DECLARE_ADDRESS_MAP(registers, 8);

	DECLARE_WRITE8_MEMBER(bank0_w);
	DECLARE_WRITE8_MEMBER(bank2_w);
	DECLARE_WRITE8_MEMBER(bank4_w);
	DECLARE_WRITE8_MEMBER(bank6_w);
	DECLARE_WRITE8_MEMBER(bank8_w);
	DECLARE_WRITE8_MEMBER(banka_w);
	DECLARE_WRITE8_MEMBER(visible_page_w);
	DECLARE_WRITE8_MEMBER(firq_scanline_w);

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_timer);
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	template<class _Object> static devcb_base &set_scanline_cb(device_t &device, _Object object) { return downcast<wpc_dmd_device &>(device).scanline_cb.set_callback(object); }

protected:
	devcb_write_line scanline_cb;
	required_memory_bank dmd0, dmd2, dmd4, dmd6, dmd8, dmda;

	UINT8 cur_scanline, visible_page, firq_scanline;
	std::vector<UINT8> ram, screen_buffer, bitcounts;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
};

extern const device_type WPC_DMD;

#endif

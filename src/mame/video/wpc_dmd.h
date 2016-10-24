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
	wpc_dmd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_dmd_device();

	DECLARE_ADDRESS_MAP(registers, 8);

	void bank0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank4_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank6_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bank8_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void banka_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void visible_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void firq_scanline_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void scanline_timer(timer_device &timer, void *ptr, int32_t param);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	template<class _Object> static devcb_base &set_scanline_cb(device_t &device, _Object object) { return downcast<wpc_dmd_device &>(device).scanline_cb.set_callback(object); }

protected:
	devcb_write_line scanline_cb;
	required_memory_bank dmd0, dmd2, dmd4, dmd6, dmd8, dmda;

	uint8_t cur_scanline, visible_page, firq_scanline;
	std::vector<uint8_t> ram, screen_buffer, bitcounts;

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
};

extern const device_type WPC_DMD;

#endif

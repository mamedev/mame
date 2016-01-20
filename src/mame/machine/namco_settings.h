// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Namco system 12/23 settings device

#ifndef __NAMCO_SETTINGS_H__
#define __NAMCO_SETTINGS_H__

#define MCFG_NAMCO_SETTINGS_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NAMCO_SETTINGS, 0)

class namco_settings_device : public device_t {
public:
	namco_settings_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( ce_w );
	DECLARE_WRITE_LINE_MEMBER( clk_w );
	DECLARE_WRITE_LINE_MEMBER( data_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	int ce, clk, data;
	int cur_bit;
	UINT8 adr, value;
};

extern const device_type NAMCO_SETTINGS;

#endif

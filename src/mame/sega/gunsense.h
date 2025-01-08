// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_SEGA_GUNSENSE_H
#define MAME_SEGA_GUNSENSE_H

class sega_gunsense_board_device : public device_t
{
public:
	sega_gunsense_board_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
};

// device type declaration
DECLARE_DEVICE_TYPE(SEGA_GUNSENSE, sega_gunsense_board_device)

#endif // MAME_SEGA_GUNSENSE_H

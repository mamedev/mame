// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    SED1200

    A LCD controller.

    The D/F variants are a packaging difference (QFP80 vs. bare chip).
    The A/B variants are an internal CGROM difference (jis
    vs. european characters)

***************************************************************************/

#ifndef MAME_VIDEO_SED1200_H
#define MAME_VIDEO_SED1200_H

#pragma once


class sed1200_device : public device_t {
public:
	void cs_w(int state);
	void control_w(uint8_t data);
	uint8_t busy_r();
	void data_w(uint8_t data);

	const uint8_t *render();

protected:
	sed1200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;

private:
	uint8_t cgram[4*8];
	uint8_t ddram[10*2];
	uint8_t render_buf[20*8];
	bool cursor_direction, cursor_blinking, cursor_full, cursor_on, display_on, two_lines;
	uint8_t cursor_address, cgram_address;
	const uint8_t *cgrom;

	bool chip_select, first_input;
	uint8_t first_data;

	void control_write(uint8_t data);
	void data_write(uint8_t data);

	void soft_reset();
	void cursor_step();
};

class sed1200d0a_device : public sed1200_device {
public:
	sed1200d0a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sed1200f0a_device : public sed1200_device {
public:
	sed1200f0a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sed1200d0b_device : public sed1200_device {
public:
	sed1200d0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class sed1200f0b_device : public sed1200_device {
public:
	sed1200f0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(SED1200D0A, sed1200d0a_device)
DECLARE_DEVICE_TYPE(SED1200F0A, sed1200f0a_device)
DECLARE_DEVICE_TYPE(SED1200D0B, sed1200d0b_device)
DECLARE_DEVICE_TYPE(SED1200F0B, sed1200f0b_device)

#endif // MAME_VIDEO_SED1200_H

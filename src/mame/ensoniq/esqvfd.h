// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_ENSONIQ_ESQVFD_H
#define MAME_ENSONIQ_ESQVFD_H

#include <memory>
#include <tuple>


class esqvfd_device : public device_t {
public:
	void write(uint8_t data) { write_char(data); }

	virtual void write_char(uint8_t data) = 0;
	virtual void update_display();
	virtual bool write_contents(std::ostream &o) { return false; }
	virtual void clear();
	virtual void cursor_left();
	virtual void cursor_right();
	virtual void set_blink_on(bool blink_on);

	// why isn't the font just stored in this order?
	static uint32_t conv_segments(uint16_t segin) { return bitswap<15>(segin, 12, 11, 7, 6, 4, 10, 3, 14, 15, 0, 13, 9, 5, 1, 2); }

protected:
	esqvfd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int rows, int cols);

	static constexpr uint8_t AT_NORMAL      = 0x00;
	static constexpr uint8_t AT_UNDERLINE   = 0x01;
	static constexpr uint8_t AT_BLINK       = 0x02;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	output_finder<160> m_vfds;
	bool m_blink_on = false;
	int m_cursx = 0, m_cursy = 0;
	int m_savedx = 0, m_savedy = 0;
	int const m_rows = 0, m_cols = 0;
	uint8_t m_curattr = 0;
	uint8_t m_lastchar = 0;
	uint8_t m_chars[2][40]{};
	uint8_t m_attrs[2][40]{};
	uint8_t m_dirty[2][40]{};
};

class esq1x22_device : public esqvfd_device {
public:
	esq1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
};

class esq2x40_device : public esqvfd_device {
public:
	esq2x40_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	esq2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(uint8_t data) override;
	virtual bool write_contents(std::ostream &o) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class esq2x40_vfx_device : public esq2x40_device {
public:
	esq2x40_vfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void update_display() override;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override;

private:
	required_region_ptr<u16> m_font;
};

class esq2x40_sq1_device : public esqvfd_device {
public:
	esq2x40_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(uint8_t data) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	bool m_wait87shift, m_wait88shift;
};

DECLARE_DEVICE_TYPE(ESQ1X22,     esq1x22_device)
DECLARE_DEVICE_TYPE(ESQ2X40,     esq2x40_device)
DECLARE_DEVICE_TYPE(ESQ2X40_SQ1, esq2x40_sq1_device)
DECLARE_DEVICE_TYPE(ESQ2X40_VFX, esq2x40_vfx_device)

#endif // MAME_ENSONIQ_ESQVFD_H

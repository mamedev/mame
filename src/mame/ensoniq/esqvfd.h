// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_ENSONIQ_ESQVFD_H
#define MAME_ENSONIQ_ESQVFD_H

#include <memory>
#include <span>
#include <tuple>


class esqvfd_device : public device_t
{
public:
	void write(uint8_t data) { write_char(data); }

	virtual void write_char(uint8_t data) = 0;
	virtual void update_display();
	virtual bool write_contents(std::ostream &o) { return false; }
	virtual void clear_display();
	virtual void clear();
	virtual void cursor_left();
	virtual void cursor_right();
	virtual void set_blink_on(bool blink_on);

	// why isn't the font just stored in this order?
	static uint32_t conv_segments(uint16_t segin) { return bitswap<15>(segin, 12, 11, 7, 6, 4, 10, 3, 14, 15, 0, 13, 9, 5, 1, 2); }

protected:
	esqvfd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int rows, int cols);

	virtual void set_vfd_at_index(unsigned index, uint32_t value) = 0;

	int index_for(unsigned row, unsigned col) { return row * m_cols + col; }

	void set_vfd(unsigned row, unsigned col, uint32_t value) { set_vfd_at_index(index_for(row, col), value); }
	uint8_t &chars(unsigned row, unsigned col) { return m_chars[index_for(row, col)]; }
	uint8_t &attrs(unsigned row, unsigned col) { return m_attrs[index_for(row, col)]; }
	uint8_t &dirty(unsigned row, unsigned col) { return m_dirty[index_for(row, col)]; }

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	static constexpr uint8_t AT_NORMAL      = 0x00;
	static constexpr uint8_t AT_UNDERLINE   = 0x01;
	static constexpr uint8_t AT_BLINK       = 0x02;

	int const m_rows, m_cols;
	std::unique_ptr<uint8_t []> m_storage;
	std::span<uint8_t> m_chars;
	std::span<uint8_t> m_attrs;
	std::span<uint8_t> m_dirty;
	bool m_blink_on;
	int16_t m_cursx, m_cursy;
	int16_t m_savedx, m_savedy;
	uint8_t m_curattr;
	uint8_t m_lastchar;
};

class esq1x22_device : public esqvfd_device
{
public:
	esq1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(uint8_t data) override;

protected:
	virtual void set_vfd_at_index(unsigned index, uint32_t value) override { m_vfds[index] = value; }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	output_finder<1 * 22 * 2> m_vfds;
};

// Virtual superclass for ESQ1- and VFX-family 2x40 displays,
// allowing common code to be shared.
class esq2x40_device : public esqvfd_device
{
public:
	esq2x40_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(uint8_t data) override;
	virtual bool write_contents(std::ostream &o) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class esq2x40_esq1_device : public esq2x40_device
{
public:
	esq2x40_esq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void set_vfd_at_index(unsigned index, uint32_t value) override { m_vfds[index] = value; }

private:
	output_finder<2 * 40 * 2> m_vfds;
};

class esq2x40_vfx_device : public esq2x40_device
{
public:
	esq2x40_vfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual void update_display() override;

protected:
	virtual void set_vfd_at_index(unsigned index, uint32_t value) override { m_vfds[index] = value; }

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	required_region_ptr<u16> m_font;
	output_finder<2 * 40> m_vfds;
};

class esq2x40_sq1_device : public esqvfd_device {
public:
	esq2x40_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void write_char(uint8_t data) override;

protected:
	virtual void set_vfd_at_index(unsigned index, uint32_t value) override { m_vfds[index] = value; }

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	output_finder<2 * 40 * 2> m_vfds;
	bool m_wait87shift, m_wait88shift;
};

DECLARE_DEVICE_TYPE(ESQ1X22,      esq1x22_device)
DECLARE_DEVICE_TYPE(ESQ2X40_ESQ1, esq2x40_esq1_device)
DECLARE_DEVICE_TYPE(ESQ2X40_SQ1,  esq2x40_sq1_device)
DECLARE_DEVICE_TYPE(ESQ2X40_VFX,  esq2x40_vfx_device)

#endif // MAME_ENSONIQ_ESQVFD_H

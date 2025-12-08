// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_TC0110PCR_H
#define MAME_TAITO_TC0110PCR_H

#pragma once


class tc0110pcr_device : public device_t, public device_palette_interface
{
public:
	using color_delegate = device_delegate<rgb_t (uint16_t data)>;
	tc0110pcr_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configurations
	void set_shift(uint8_t shift) { m_shift = shift; }
	template <typename... T> void set_color_callback(T &&... args) { m_color_cb.set(std::forward<T>(args)...); }

	u16 word_r(offs_t offset);
	void word_w(offs_t offset, u16 data);

	void restore_colors();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// device_palette_interface overrides
	virtual u32 palette_entries() const noexcept override { return TC0110PCR_RAM_SIZE; }

private:
	static const unsigned TC0110PCR_RAM_SIZE = 0x2000 / 2;

	// internal states
	std::unique_ptr<uint16_t[]> m_ram;
	uint16_t m_addr;

	// configurations
	uint8_t m_shift;
	color_delegate m_color_cb;
};

DECLARE_DEVICE_TYPE(TC0110PCR, tc0110pcr_device)

#endif // MAME_TAITO_TC0110PCR_H

// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_KONAMI_K001006_H
#define MAME_KONAMI_K001006_H

#pragma once



class k001006_device : public device_t, public device_palette_interface
{
public:
	k001006_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k001006_device() {}

	// configuration
	template <typename T> void set_gfx_region(T &&tag) { m_gfxrom.set_tag(std::forward<T>(tag)); }

	uint32_t fetch_texel(int page, int pal_index, int u, int v);
	void preprocess_texture_data(uint8_t *dst, uint8_t *src, int length);

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	bool bilinear_enabled() { return m_enable_bilinear; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface overrides
	virtual u32 palette_entries() const noexcept override { return 0x800; }

private:
	// internal state
	std::unique_ptr<uint16_t[]>      m_pal_ram;
	std::unique_ptr<uint16_t[]>     m_unknown_ram;
	uint32_t       m_addr;
	int          m_device_sel;

	std::unique_ptr<uint8_t[]>     m_texrom;

	required_region_ptr<uint8_t> m_gfxrom;
	bool m_enable_bilinear = false;
};


DECLARE_DEVICE_TYPE(K001006, k001006_device)

#endif // MAME_KONAMI_K001006_H

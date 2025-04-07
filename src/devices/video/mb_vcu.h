// license:LGPL-2.1+
// copyright-holders:Angelo Salese
#ifndef MAME_VIDEO_MB_VCU_H
#define MAME_VIDEO_MB_VCU_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb_vcu_device

class mb_vcu_device : public device_t,
						public device_memory_interface,
						public device_video_interface,
						public device_palette_interface
{
public:
	// construction/destruction
	mb_vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_host_space(T &&tag, int spacenum) { m_host_space.set_tag(std::forward<T>(tag), spacenum); }

	// I/O operations
	void write_vregs(offs_t offset, uint8_t data);
	uint8_t read_ram(offs_t offset);
	void write_ram(offs_t offset, uint8_t data);
	uint8_t load_params(offs_t offset);
	uint8_t load_gfx(offs_t offset);
	uint8_t load_set_clr(offs_t offset);
	void background_color_w(uint8_t data);
	uint8_t status_r();
	void vbank_w(uint8_t data);
	void vbank_clear_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof();

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	virtual uint32_t palette_entries() const noexcept override { return 256 + 1; }
	virtual uint32_t palette_indirect_entries() const noexcept override { return 256; }

private:
	inline uint8_t read_byte(offs_t address);
	inline void write_byte(offs_t address, uint8_t data);
	inline uint8_t read_io(offs_t address);
	inline void write_io(offs_t address, uint8_t data);

	uint8_t paletteram_r(offs_t offset);
	void paletteram_w(offs_t offset, uint8_t data);

	void pal_ram(address_map &map) ATTR_COLD;
	void vram(address_map &map) ATTR_COLD;

	const address_space_config m_videoram_space_config;
	const address_space_config m_paletteram_space_config;
	required_address_space m_host_space;
	memory_access<16, 0, 0, ENDIANNESS_LITTLE>::cache m_memory;

	uint8_t m_status;
	std::unique_ptr<uint8_t[]> m_ram;
	std::unique_ptr<uint8_t[]> m_palram;
	uint16_t m_param_offset_latch;

	int16_t m_xpos, m_ypos;
	uint16_t m_color;
	uint8_t m_mode;
	uint16_t m_pix_xsize, m_pix_ysize;
	uint8_t m_vregs[4];
	uint8_t m_bk_color;
	uint8_t m_vbank;

};


// device type definition
DECLARE_DEVICE_TYPE(MB_VCU, mb_vcu_device)

#endif // MAME_VIDEO_MB_VCU_H

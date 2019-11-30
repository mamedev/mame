// license:LGPL-2.1+
// copyright-holders:Angelo Salese
#ifndef MAME_VIDEO_MB_VCU_H
#define MAME_VIDEO_MB_VCU_H

#pragma once

#include "emupal.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb_vcu_device

class mb_vcu_device : public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	mb_vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_cpu_tag(T &&tag) { m_cpu.set_tag(std::forward<T>(tag)); }

	// I/O operations
	DECLARE_WRITE8_MEMBER( write_vregs );
	DECLARE_READ8_MEMBER( read_ram );
	DECLARE_WRITE8_MEMBER( write_ram );
	DECLARE_READ8_MEMBER( load_params );
	DECLARE_READ8_MEMBER( load_gfx );
	DECLARE_READ8_MEMBER( load_set_clr );
	DECLARE_WRITE8_MEMBER( background_color_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_WRITE8_MEMBER( vbank_w );
	DECLARE_WRITE8_MEMBER( vbank_clear_w );

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof(void);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual space_config_vector memory_space_config() const override;

private:
	inline uint8_t read_byte(offs_t address);
	inline void write_byte(offs_t address, uint8_t data);
	inline uint8_t read_io(offs_t address);
	inline void write_io(offs_t address, uint8_t data);

	DECLARE_READ8_MEMBER( mb_vcu_paletteram_r );
	DECLARE_WRITE8_MEMBER( mb_vcu_paletteram_w );

	void mb_vcu_pal_ram(address_map &map);
	void mb_vcu_vram(address_map &map);

	const address_space_config      m_videoram_space_config;
	const address_space_config      m_paletteram_space_config;
	uint8_t m_status;
	std::unique_ptr<uint8_t[]> m_ram;
	std::unique_ptr<uint8_t[]> m_palram;
	uint16_t m_param_offset_latch;

	int16_t m_xpos, m_ypos;
	uint8_t m_color1, m_color2;
	uint8_t m_mode;
	uint16_t m_pix_xsize, m_pix_ysize;
	uint8_t m_vregs[4];
	uint8_t m_bk_color;
	uint8_t m_vbank;

	double m_weights_r[2];
	double m_weights_g[3];
	double m_weights_b[3];
	required_device<cpu_device>     m_cpu;
	required_device<palette_device> m_palette;
};


// device type definition
DECLARE_DEVICE_TYPE(MB_VCU, mb_vcu_device)

#endif // MAME_VIDEO_MB_VCU_H

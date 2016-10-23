// license:LGPL-2.1+
// copyright-holders:Angelo Salese
#pragma once

#ifndef __MB_VCUDEV_H__
#define __MB_VCUDEV_H__


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

	// static configuration
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_cpu_tag(device_t &device, const char *tag) { downcast<mb_vcu_device &>(device).m_cpu.set_tag(tag); }

	// I/O operations
	void write_vregs(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t read_ram(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write_ram(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t load_params(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t load_gfx(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t load_set_clr(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void background_color_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void vbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mb_vcu_paletteram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mb_vcu_paletteram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof(void);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
private:
	inline uint8_t read_byte(offs_t address);
	inline void write_byte(offs_t address, uint8_t data);
	inline uint8_t read_io(offs_t address);
	inline void write_io(offs_t address, uint8_t data);

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
extern const device_type MB_VCU;


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MB_VCU_CPU(_tag) \
	mb_vcu_device::set_cpu_tag(*device, "^" _tag);

#define MCFG_MB_VCU_PALETTE(_palette_tag) \
	mb_vcu_device::static_set_palette_tag(*device, "^" _palette_tag);

#endif

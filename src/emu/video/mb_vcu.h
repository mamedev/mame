// license: ?
// copyright-holders: Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __MB_VCUDEV_H__
#define __MB_VCUDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MB_VCU_ADD(_tag,_freq,_config, _palette_tag) \
	MCFG_DEVICE_ADD(_tag, MB_VCU, _freq) \
	MCFG_DEVICE_CONFIG(_config) \
	mb_vcu_device::static_set_palette_tag(*device, "^" _palette_tag);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb_vcu_interface

struct mb_vcu_interface
{
	const char         *m_cpu_tag;
};

// ======================> mb_vcu_device

class mb_vcu_device : public device_t,
						public device_memory_interface,
						public device_video_interface,
						public mb_vcu_interface
{
public:
	// construction/destruction
	mb_vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_palette_tag(device_t &device, const char *tag);

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
	DECLARE_READ8_MEMBER( mb_vcu_paletteram_r );
	DECLARE_WRITE8_MEMBER( mb_vcu_paletteram_w );

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void screen_eof(void);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;
private:
	inline UINT8 read_byte(offs_t address);
	inline void write_byte(offs_t address, UINT8 data);
	inline UINT8 read_io(offs_t address);
	inline void write_io(offs_t address, UINT8 data);

	const address_space_config      m_videoram_space_config;
	const address_space_config      m_paletteram_space_config;
	UINT8 m_status;
	UINT8 *m_ram;
	UINT8 *m_palram;
	cpu_device *m_cpu;
	UINT16 m_param_offset_latch;

	INT16 m_xpos, m_ypos;
	UINT8 m_color1, m_color2;
	UINT8 m_mode;
	UINT16 m_pix_xsize, m_pix_ysize;
	UINT8 m_vregs[4];
	UINT8 m_bk_color;
	UINT8 m_vbank;

	double m_weights_r[2];
	double m_weights_g[3];
	double m_weights_b[3];
	required_device<palette_device> m_palette;
};


// device type definition
extern const device_type MB_VCU;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif

// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*
  pc cga/mda combi adapters

  one type hardware switchable between cga and mda/hercules
  another type software switchable between cga and mda/hercules

  some support additional modes like
  commodore pc10 320x200 in 16 colors


    // aga
    // 256 8x8 thick chars
    // 256 8x8 thin chars
    // 256 9x14 in 8x16 chars, line 3 is connected to a10
    ROM_LOAD("aga.chr",      0x0000, 0x2000, CRC(aca81498) SHA1(0d84c89487ee7a6ac4c9e73fdb30c5fd8aa595f8) )
    // hercules font of above
    ROM_LOAD("hercules.chr", 0x0000, 0x1000, CRC(7e8c9d76))

*/
#ifndef MAME_BUS_ISA_AGA_H
#define MAME_BUS_ISA_AGA_H

#include "isa.h"
#include "cga.h"
#include "video/mc6845.h"
#include "emupal.h"


class isa8_aga_device :
		public device_t,
		public device_isa8_card_interface
{
public:
	enum mode_t { AGA_OFF, AGA_COLOR, AGA_MONO };

	// construction/destruction
	isa8_aga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	isa8_aga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	uint8_t pc_aga_mda_r(offs_t offset);
	void pc_aga_mda_w(offs_t offset, uint8_t data);
	uint8_t pc_aga_cga_r(offs_t offset);
	void pc_aga_cga_w(offs_t offset, uint8_t data);
	void set_palette_luts();
	void pc_aga_set_mode(mode_t mode);
	void pc_aga_videoram_w(offs_t offset, uint8_t data);
	uint8_t pc_aga_videoram_r(offs_t offset);

	MC6845_UPDATE_ROW( aga_update_row );
	MC6845_UPDATE_ROW( mda_text_inten_update_row );
	MC6845_UPDATE_ROW( mda_text_blink_update_row );
	MC6845_UPDATE_ROW( cga_text_inten_update_row );
	MC6845_UPDATE_ROW( cga_text_inten_alt_update_row );
	MC6845_UPDATE_ROW( cga_text_blink_update_row );
	MC6845_UPDATE_ROW( cga_text_blink_alt_update_row );
	MC6845_UPDATE_ROW( cga_gfx_4bppl_update_row );
	MC6845_UPDATE_ROW( cga_gfx_4bpph_update_row );
	MC6845_UPDATE_ROW( cga_gfx_2bpp_update_row );
	MC6845_UPDATE_ROW( cga_gfx_1bpp_update_row );

	void hsync_changed(int state);
	void vsync_changed(int state);

	required_device<palette_device> m_palette;
	required_device<mc6845_device> m_mc6845;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_ioport m_cga_config;

	int       m_update_row_type;
	mode_t    m_mode;
	uint8_t   m_mda_mode_control;
	uint8_t   m_mda_status;
	uint8_t   *m_mda_chr_gen;

	uint8_t   m_cga_mode_control;
	uint8_t   m_cga_color_select;
	uint8_t   m_cga_status;
	uint8_t   *m_cga_chr_gen;

	int       m_framecnt;
	uint8_t   m_vsync;
	uint8_t   m_hsync;


	uint8_t   m_cga_palette_lut_2bpp[4];

	std::unique_ptr<uint8_t[]>  m_videoram;
};


class isa8_aga_pc200_device :
		public isa8_aga_device
{
public:
	// construction/destruction
	isa8_aga_pc200_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	uint8_t pc200_videoram_r(offs_t offset);
	void pc200_videoram_w(offs_t offset, uint8_t data);
	void pc200_cga_w(offs_t offset, uint8_t data);
	uint8_t pc200_cga_r(offs_t offset);

	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	uint8_t m_port8;
	uint8_t m_portd;
	uint8_t m_porte;
};


DECLARE_DEVICE_TYPE(ISA8_AGA, isa8_aga_device)
DECLARE_DEVICE_TYPE(ISA8_AGA_PC200, isa8_aga_pc200_device)

#endif // MAME_BUS_ISA_AGA_H

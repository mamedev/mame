// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Gaelco 3D games

    driver by Aaron Giles

**************************************************************************/

#include "sound/dmadac.h"
#include "video/poly.h"
#include "machine/eepromser.h"
#include "machine/gaelco3d.h"
#include "cpu/adsp2100/adsp2100.h"

#define SOUND_CHANNELS  4

struct gaelco3d_object_data
{
	UINT32 tex, color;
	float ooz_dx, ooz_dy, ooz_base;
	float uoz_dx, uoz_dy, uoz_base;
	float voz_dx, voz_dy, voz_base;
	float z0;
};

class gaelco3d_state;

class gaelco3d_renderer : public poly_manager<float, gaelco3d_object_data, 1, 2000>
{
public:
	gaelco3d_renderer(gaelco3d_state &state);

	bitmap_ind16 &screenbits() { return m_screenbits; }
	UINT32 polygons() { UINT32 result = m_polygons; m_polygons = 0; return result; }

	void render_poly(screen_device &screen, UINT32 *polydata);

private:
	gaelco3d_state &m_state;
	bitmap_ind16 m_screenbits;
	bitmap_ind16 m_zbuffer;
	UINT32 m_polygons;
	offs_t m_texture_size;
	offs_t m_texmask_size;
	std::unique_ptr<UINT8[]> m_texture;
	std::unique_ptr<UINT8[]> m_texmask;

	void render_noz_noperspective(INT32 scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
	void render_normal(INT32 scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
	void render_alphablend(INT32 scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
};

class gaelco3d_state : public driver_device
{
public:
	gaelco3d_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_adsp_ram_base(*this,"adsp_ram_base"),
			m_m68k_ram_base(*this,"m68k_ram_base",0),
			m_tms_comm_base(*this,"tms_comm_base",0),
			m_adsp_control_regs(*this,"adsp_regs"),
			m_adsp_fastram_base(*this,"adsp_fastram") ,
		m_maincpu(*this, "maincpu"),
		m_adsp(*this, "adsp"),
		m_eeprom(*this, "eeprom"),
		m_tms(*this, "tms"),
		m_serial(*this, "serial"),
		m_screen(*this, "screen"),
		m_paletteram16(*this, "paletteram"),
		m_paletteram32(*this, "paletteram") { }

	required_shared_ptr<UINT32> m_adsp_ram_base;
	required_shared_ptr<UINT16> m_m68k_ram_base;
	required_shared_ptr<UINT16> m_tms_comm_base;
	required_shared_ptr<UINT16> m_adsp_control_regs;
	required_shared_ptr<UINT16> m_adsp_fastram_base;
	required_device<cpu_device> m_maincpu;
	required_device<adsp21xx_device> m_adsp;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<cpu_device> m_tms;
	required_device<gaelco_serial_device> m_serial;
	required_device<screen_device> m_screen;
	optional_shared_ptr<UINT16> m_paletteram16;
	optional_shared_ptr<UINT32> m_paletteram32;

	UINT16 m_sound_data;
	UINT8 m_sound_status;
	offs_t m_tms_offset_xor;
	UINT8 m_analog_ports[4];
	UINT8 m_framenum;
	timer_device *m_adsp_autobuffer_timer;
	UINT8 m_adsp_ireg;
	offs_t m_adsp_ireg_base;
	offs_t m_adsp_incs;
	offs_t m_adsp_size;
	dmadac_sound_device *m_dmadac[SOUND_CHANNELS];
	std::unique_ptr<rgb_t[]> m_palette;
	std::unique_ptr<UINT32[]> m_polydata_buffer;
	UINT32 m_polydata_count;
	int m_lastscan;
	int m_video_changed;
	std::unique_ptr<gaelco3d_renderer> m_poly;
	DECLARE_WRITE16_MEMBER(irq_ack_w);
	DECLARE_WRITE32_MEMBER(irq_ack32_w);
	DECLARE_WRITE16_MEMBER(sound_data_w);
	DECLARE_READ16_MEMBER(sound_data_r);
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE16_MEMBER(sound_status_w);
	DECLARE_WRITE16_MEMBER(analog_port_clock_w);
	DECLARE_WRITE16_MEMBER(analog_port_latch_w);
	DECLARE_READ32_MEMBER(tms_m68k_ram_r);
	DECLARE_WRITE32_MEMBER(tms_m68k_ram_w);
	DECLARE_WRITE8_MEMBER(tms_iack_w);
	DECLARE_WRITE16_MEMBER(tms_reset_w);
	DECLARE_WRITE16_MEMBER(tms_irq_w);
	DECLARE_WRITE16_MEMBER(tms_control3_w);
	DECLARE_WRITE16_MEMBER(tms_comm_w);
	DECLARE_WRITE16_MEMBER(adsp_control_w);
	DECLARE_WRITE16_MEMBER(adsp_rombank_w);
	DECLARE_WRITE32_MEMBER(radikalb_lamp_w);
	DECLARE_WRITE32_MEMBER(unknown_137_w);
	DECLARE_WRITE32_MEMBER(unknown_13a_w);
	DECLARE_WRITE32_MEMBER(gaelco3d_render_w);
	DECLARE_WRITE16_MEMBER(gaelco3d_paletteram_w);
	DECLARE_WRITE32_MEMBER(gaelco3d_paletteram_020_w);
	DECLARE_CUSTOM_INPUT_MEMBER(analog_bit_r);
	DECLARE_WRITE_LINE_MEMBER(ser_irq);
	DECLARE_READ16_MEMBER(eeprom_data_r);
	DECLARE_READ32_MEMBER(eeprom_data32_r);
	DECLARE_WRITE16_MEMBER(eeprom_data_w);
	DECLARE_WRITE16_MEMBER(eeprom_clock_w);
	DECLARE_WRITE16_MEMBER(eeprom_cs_w);
	DECLARE_DRIVER_INIT(gaelco3d);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_MACHINE_RESET(gaelco3d2);
	DECLARE_MACHINE_RESET(common);
	UINT32 screen_update_gaelco3d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_gen);
	TIMER_CALLBACK_MEMBER(delayed_sound_w);
	TIMER_DEVICE_CALLBACK_MEMBER(adsp_autobuffer_irq);
	void gaelco3d_render(screen_device &screen);
	DECLARE_WRITE32_MEMBER(adsp_tx_callback);
};

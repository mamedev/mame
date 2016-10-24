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
	uint32_t tex, color;
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
	uint32_t polygons() { uint32_t result = m_polygons; m_polygons = 0; return result; }

	void render_poly(screen_device &screen, uint32_t *polydata);

private:
	gaelco3d_state &m_state;
	bitmap_ind16 m_screenbits;
	bitmap_ind16 m_zbuffer;
	uint32_t m_polygons;
	offs_t m_texture_size;
	offs_t m_texmask_size;
	std::unique_ptr<uint8_t[]> m_texture;
	std::unique_ptr<uint8_t[]> m_texmask;

	void render_noz_noperspective(int32_t scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
	void render_normal(int32_t scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
	void render_alphablend(int32_t scanline, const extent_t &extent, const gaelco3d_object_data &extra, int threadid);
};

class gaelco3d_state : public driver_device
{
public:
	gaelco3d_state(const machine_config &mconfig, device_type type, const char *tag)
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
		m_paletteram32(*this, "paletteram"),
		m_analog(*this, {"ANALOG0", "ANALOG1", "ANALOG2", "ANALOG3"})
		{ }

	required_shared_ptr<uint32_t> m_adsp_ram_base;
	required_shared_ptr<uint16_t> m_m68k_ram_base;
	required_shared_ptr<uint16_t> m_tms_comm_base;
	required_shared_ptr<uint16_t> m_adsp_control_regs;
	required_shared_ptr<uint16_t> m_adsp_fastram_base;
	required_device<cpu_device> m_maincpu;
	required_device<adsp21xx_device> m_adsp;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<cpu_device> m_tms;
	required_device<gaelco_serial_device> m_serial;
	required_device<screen_device> m_screen;
	optional_shared_ptr<uint16_t> m_paletteram16;
	optional_shared_ptr<uint32_t> m_paletteram32;
	optional_ioport_array<4> m_analog;

	uint16_t m_sound_data;
	uint8_t m_sound_status;
	offs_t m_tms_offset_xor;
	uint8_t m_analog_ports[4];
	uint8_t m_framenum;
	timer_device *m_adsp_autobuffer_timer;
	uint8_t m_adsp_ireg;
	offs_t m_adsp_ireg_base;
	offs_t m_adsp_incs;
	offs_t m_adsp_size;
	dmadac_sound_device *m_dmadac[SOUND_CHANNELS];
	std::unique_ptr<rgb_t[]> m_palette;
	std::unique_ptr<uint32_t[]> m_polydata_buffer;
	uint32_t m_polydata_count;
	int m_lastscan;
	int m_video_changed;
	std::unique_ptr<gaelco3d_renderer> m_poly;
	void irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void irq_ack32_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void sound_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sound_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sound_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void sound_status_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void analog_port_clock_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void analog_port_latch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t tms_m68k_ram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void tms_m68k_ram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void tms_iack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tms_reset_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tms_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tms_control3_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tms_comm_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void adsp_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void adsp_rombank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void radikalb_lamp_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void unknown_137_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void unknown_13a_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void gaelco3d_render_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void gaelco3d_paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void gaelco3d_paletteram_020_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	ioport_value analog_bit_r(ioport_field &field, void *param);
	void ser_irq(int state);
	uint16_t eeprom_data_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t eeprom_data32_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void eeprom_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void eeprom_clock_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void eeprom_cs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_gaelco3d();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void machine_reset_gaelco3d2();
	void machine_reset_common();
	uint32_t screen_update_gaelco3d(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_gen(device_t &device);
	void delayed_sound_w(void *ptr, int32_t param);
	void adsp_autobuffer_irq(timer_device &timer, void *ptr, int32_t param);
	void gaelco3d_render(screen_device &screen);
	void adsp_tx_callback(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
};

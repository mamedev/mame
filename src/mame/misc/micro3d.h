// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

     Microprose Games 3D hardware

*************************************************************************/
#ifndef MAME_MISC_MICRO3D_H
#define MAME_MISC_MICRO3D_H

#pragma once

#include "cpu/tms34010/tms34010.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/adc0844.h"
#include "machine/mc68681.h"
#include "machine/scn_pci.h"
#include "sound/upd7759.h"

#include "emupal.h"


#define HOST_MONITOR_DISPLAY        0
#define VGB_MONITOR_DISPLAY         0
#define DRMATH_MONITOR_DISPLAY      0


class micro3d_sound_device;

class micro3d_state : public driver_device
{
public:
	micro3d_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_upd7759(*this, "upd7759"),
		m_drmath(*this, "drmath"),
		m_vgb(*this, "vgb"),
		m_vgb_uart(*this, "uart"),
		m_palette(*this, "palette"),
		m_duart(*this, "duart"),
		m_noise(*this, "noise_%u", 1U),
		m_adc(*this, "adc"),
		m_vertex(*this, "vertex"),
		m_sound_sw(*this, "SOUND_SW"),
		m_volume(*this, "VOLUME"),
		m_joystick_x(*this, "JOYSTICK_X"),
		m_joystick_y(*this, "JOYSTICK_Y"),
		m_shared_ram(*this, "shared_ram"),
		m_mac_sram(*this, "mac_sram"),
		m_sprite_vram(*this, "sprite_vram")
	{ }

	void micro3d(machine_config &config);
	void botss11(machine_config &config);

	void init_micro3d();
	void init_botss();

	int botss_hwchk_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void video_reset() override ATTR_COLD;

private:
	enum planes
	{
		CLIP_Z_MIN,
		CLIP_Z_MAX,
		CLIP_X_MIN,
		CLIP_X_MAX,
		CLIP_Y_MIN,
		CLIP_Y_MAX
	};

	struct micro3d_vtx
	{
		int32_t x = 0, y = 0, z = 0;

		constexpr int64_t dot_product(micro3d_vtx const &that) const;
	};

	required_device<cpu_device> m_maincpu;
	required_device<i8051_device> m_audiocpu;
	required_device<upd7759_device> m_upd7759;
	required_device<cpu_device> m_drmath;
	required_device<tms34010_device> m_vgb;
	required_device<scn2651_device> m_vgb_uart;
	required_device<palette_device> m_palette;
	required_device<mc68681_device> m_duart;
	required_device_array<micro3d_sound_device, 2> m_noise;
	optional_device<adc0844_device> m_adc;
	required_memory_region m_vertex;

	required_ioport m_sound_sw;
	required_ioport m_volume;
	optional_ioport m_joystick_x;
	optional_ioport m_joystick_y;

	required_shared_ptr<uint16_t> m_shared_ram;
	uint8_t             m_m68681_tx0 = 0;

	// Sound
	uint8_t             m_sound_port_latch[4]{};

	// Hardware version-check latch for BOTSS 1.1a
	uint8_t             m_botss_latch = 0;

	// MAC
	required_shared_ptr<uint32_t> m_mac_sram;
	emu_timer           *m_mac_done_timer = nullptr;
	uint32_t            m_sram_r_addr = 0;
	uint32_t            m_sram_w_addr = 0;
	uint32_t            m_vtx_addr = 0;
	uint32_t            m_mrab11 = 0;
	uint32_t            m_mac_stat = 0;
	uint32_t            m_mac_inst = 0;

	// 2D video
	required_shared_ptr<uint16_t> m_sprite_vram;
	uint16_t            m_creg = 0;
	uint16_t            m_xfer3dk = 0;

	// 3D pipeline
	uint32_t            m_pipe_data = 0;
	uint32_t            m_pipeline_state = 0;
	int32_t             m_vtx_fifo[512]{};
	uint32_t            m_fifo_idx = 0;
	uint32_t            m_draw_cmd = 0;
	uint32_t            m_draw_state = 0;
	int32_t             m_x_min = 0;
	int32_t             m_x_max = 0;
	int32_t             m_y_min = 0;
	int32_t             m_y_max = 0;
	int32_t             m_z_min = 0;
	int32_t             m_z_max = 0;
	int32_t             m_x_mid = 0;
	int32_t             m_y_mid = 0;
	uint32_t            m_dpram_bank = 0;
	uint32_t            m_draw_dpram[1024]{};
	std::unique_ptr<uint16_t[]> m_frame_buffers[2];
	std::unique_ptr<uint16_t[]> m_tmp_buffer;
	uint8_t             m_drawing_buffer = 0;
	uint8_t             m_display_buffer = 0;

	void vgb_uart_w(offs_t offset, uint8_t data);
	uint8_t vgb_uart_r(offs_t offset);
	void mac1_w(uint32_t data);
	uint32_t mac2_r();
	void mac2_w(uint32_t data);
	uint16_t encoder_h_r();
	uint16_t encoder_l_r();
	uint8_t adc_volume_r();
	uint16_t botss_140000_r();
	uint16_t botss_180000_r();
	void reset_w(uint16_t data);
	void host_drmath_int_w(uint16_t data);
	void shared_w(offs_t offset, uint32_t data);
	uint32_t shared_r(offs_t offset);
	void drmath_int_w(uint32_t data);
	void drmath_intr2_ack(uint32_t data);
	void creg_w(uint16_t data);
	void xfer3dk_w(uint16_t data);
	void fifo_w(uint32_t data);
	void alt_fifo_w(uint32_t data);
	uint32_t pipe_r();
	void snd_dac_a(uint8_t data);
	void snd_dac_b(uint8_t data);
	void sound_p1_w(uint8_t data);
	void sound_p3_w(uint8_t data);
	uint8_t sound_p1_r();
	uint8_t sound_p3_r();
	INTERRUPT_GEN_MEMBER(vblank);
	TIMER_CALLBACK_MEMBER(mac_done_callback);
	void upd7759_w(uint8_t data);
	void duart_irq_handler(int state);
	uint8_t duart_input_r();
	void duart_output_w(uint8_t data);
	void duart_txb(int state);
	void tms_interrupt(int state);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	// 3D graphics
	int inside(micro3d_vtx *v, enum planes plane);
	micro3d_vtx intersect(micro3d_vtx *v1, micro3d_vtx *v2, enum planes plane);
	inline void write_span(uint32_t y, uint32_t x);
	void draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
	void rasterise_spans(uint32_t min_y, uint32_t max_y, uint32_t attr);
	int clip_triangle(micro3d_vtx *v, micro3d_vtx *vout, int num_vertices, enum planes plane);
	void draw_triangles(uint32_t attr);

	void cpu_space_map(address_map &map) ATTR_COLD;
	void drmath_data(address_map &map) ATTR_COLD;
	void drmath_prg(address_map &map) ATTR_COLD;
	void hostmem(address_map &map) ATTR_COLD;
	void soundmem_io(address_map &map) ATTR_COLD;
	void soundmem_prg(address_map &map) ATTR_COLD;
	void vgbmem(address_map &map) ATTR_COLD;
};

#endif // MAME_MISC_MICRO3D_H

// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/*************************************************************************

     Microprose Games 3D hardware

*************************************************************************/
#ifndef MAME_INCLUDES_MICRO3D_H
#define MAME_INCLUDES_MICRO3D_H

#pragma once

#include "cpu/tms34010/tms34010.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/upd7759.h"
#include "machine/adc0844.h"
#include "machine/mc68681.h"
#include "machine/scn_pci.h"
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
		m_palette(*this, "palette"),
		m_duart(*this, "duart"),
		m_noise_1(*this, "noise_1"),
		m_noise_2(*this, "noise_2"),
		m_adc(*this, "adc"),
		m_vertex(*this, "vertex"),
		m_sound_sw(*this, "SOUND_SW"),
		m_volume(*this, "VOLUME"),
		m_joystick_x(*this, "JOYSTICK_X"),
		m_joystick_y(*this, "JOYSTICK_Y"),
		m_shared_ram(*this, "shared_ram"),
		m_mac_sram(*this, "mac_sram"),
		m_sprite_vram(*this, "sprite_vram"),
		m_vgb_uart(*this, "uart")
	{ }

	void micro3d(machine_config &config);
	void botss11(machine_config &config);

	void init_micro3d();
	void init_botss();

	DECLARE_READ_LINE_MEMBER(botss_hwchk_r);

protected:
	enum
	{
		TIMER_MAC_DONE
	};

	virtual void machine_reset() override;
	virtual void video_start() override;
	virtual void video_reset() override;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

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
		int32_t x, y, z;

		constexpr int64_t dot_product(micro3d_vtx const &that) const;
	};

	required_device<cpu_device> m_maincpu;
	required_device<i8051_device> m_audiocpu;
	required_device<upd7759_device> m_upd7759;
	required_device<cpu_device> m_drmath;
	required_device<tms34010_device> m_vgb;
	required_device<palette_device> m_palette;
	required_device<mc68681_device> m_duart;
	required_device<micro3d_sound_device> m_noise_1;
	required_device<micro3d_sound_device> m_noise_2;
	optional_device<adc0844_device> m_adc;
	required_memory_region m_vertex;

	required_ioport m_sound_sw;
	required_ioport m_volume;
	optional_ioport m_joystick_x;
	optional_ioport m_joystick_y;

	required_shared_ptr<uint16_t> m_shared_ram;
	uint8_t               m_m68681_tx0;

	/* Sound */
	uint8_t               m_sound_port_latch[4];

	/* Hardware version-check latch for BOTSS 1.1a */
	uint8_t               m_botss_latch;

	/* MAC */
	required_shared_ptr<uint32_t> m_mac_sram;
	uint32_t              m_sram_r_addr;
	uint32_t              m_sram_w_addr;
	uint32_t              m_vtx_addr;
	uint32_t              m_mrab11;
	uint32_t              m_mac_stat;
	uint32_t              m_mac_inst;

	/* 2D video */
	required_shared_ptr<uint16_t> m_sprite_vram;
	uint16_t              m_creg;
	uint16_t              m_xfer3dk;

	/* 3D pipeline */
	uint32_t              m_pipe_data;
	uint32_t              m_pipeline_state;
	int32_t               m_vtx_fifo[512];
	uint32_t              m_fifo_idx;
	uint32_t              m_draw_cmd;
	int                 m_draw_state;
	int32_t               m_x_min;
	int32_t               m_x_max;
	int32_t               m_y_min;
	int32_t               m_y_max;
	int32_t               m_z_min;
	int32_t               m_z_max;
	int32_t               m_x_mid;
	int32_t               m_y_mid;
	int                 m_dpram_bank;
	uint32_t              m_draw_dpram[1024];
	std::unique_ptr<uint16_t[]>              m_frame_buffers[2];
	std::unique_ptr<uint16_t[]>              m_tmp_buffer;
	int                 m_drawing_buffer;
	int                 m_display_buffer;

	void vgb_uart_w(offs_t offset, uint8_t data);
	uint8_t vgb_uart_r(offs_t offset);
	void micro3d_mac1_w(uint32_t data);
	uint32_t micro3d_mac2_r();
	void micro3d_mac2_w(uint32_t data);
	uint16_t micro3d_encoder_h_r();
	uint16_t micro3d_encoder_l_r();
	uint8_t adc_volume_r();
	uint16_t botss_140000_r();
	uint16_t botss_180000_r();
	void micro3d_reset_w(uint16_t data);
	void host_drmath_int_w(uint16_t data);
	void micro3d_shared_w(offs_t offset, uint32_t data);
	uint32_t micro3d_shared_r(offs_t offset);
	void drmath_int_w(uint32_t data);
	void drmath_intr2_ack(uint32_t data);
	void micro3d_creg_w(uint16_t data);
	void micro3d_xfer3dk_w(uint16_t data);
	void micro3d_fifo_w(uint32_t data);
	void micro3d_alt_fifo_w(uint32_t data);
	uint32_t micro3d_pipe_r();
	void micro3d_snd_dac_a(uint8_t data);
	void micro3d_snd_dac_b(uint8_t data);
	void micro3d_sound_p1_w(uint8_t data);
	void micro3d_sound_p3_w(uint8_t data);
	uint8_t micro3d_sound_p1_r();
	uint8_t micro3d_sound_p3_r();
	INTERRUPT_GEN_MEMBER(micro3d_vblank);
	TIMER_CALLBACK_MEMBER(mac_done_callback);
	void micro3d_upd7759_w(uint8_t data);
	void data_from_i8031(uint8_t data);
	uint8_t data_to_i8031();
	DECLARE_WRITE_LINE_MEMBER(duart_irq_handler);
	uint8_t duart_input_r();
	void duart_output_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(duart_txb);
	DECLARE_WRITE_LINE_MEMBER(tms_interrupt);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);

	/* 3D graphics */
	int inside(micro3d_vtx *v, enum planes plane);
	micro3d_vtx intersect(micro3d_vtx *v1, micro3d_vtx *v2, enum planes plane);
	inline void write_span(uint32_t y, uint32_t x);
	void draw_line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
	void rasterise_spans(uint32_t min_y, uint32_t max_y, uint32_t attr);
	int clip_triangle(micro3d_vtx *v, micro3d_vtx *vout, int num_vertices, enum planes plane);
	void draw_triangles(uint32_t attr);

	void cpu_space_map(address_map &map);
	void drmath_data(address_map &map);
	void drmath_prg(address_map &map);
	void hostmem(address_map &map);
	void soundmem_io(address_map &map);
	void soundmem_prg(address_map &map);
	void vgbmem(address_map &map);

	required_device<scn2651_device> m_vgb_uart;
};

#endif // MAME_INCLUDES_MICRO3D_H

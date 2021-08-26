// license:BSD-3-Clause
// copyright-holders:Mario Montminy
#ifndef MAME_VIDEO_ALT_VECTOR_H
#define MAME_VIDEO_ALT_VECTOR_H

#pragma once

#include "osdcore.h"
#include "screen.h"

#define ALT_VECTOR_DRIVER_INSTANTIATE(driver_name, config, opt_device) \
									if (!strcmp(driver_name, "usb_dvg")) \
									{ \
										ALT_VECTOR_USB_DVG(config, opt_device); \
									}

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class alt_vector_device_base : public device_t
{
public:
	// construction/destruction
	alt_vector_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock = 0);

	// 0 = continue, 1 = handled (skip)
	virtual int add_point(int x, int y, rgb_t color, int intensity);
	virtual int update(screen_device &screen, const rectangle &cliprect);

};


class alt_vector_device_usb_dvg : public alt_vector_device_base
{
public:
	typedef struct vec_t {
		struct vec_t *next;
		int32_t       x0;
		int32_t       y0;
		int32_t       x1;
		int32_t       y1;
		rgb_t         color;
		bool          screen_coords;
	} vector_t;

	typedef struct {
		const char  *name;
		bool         exclude_blank_vectors;
		uint32_t     artwork;
		bool         bw_game;
	} game_info_t;

	alt_vector_device_usb_dvg(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_stop() override;
	virtual int add_point(int x, int y, rgb_t color, int intensity) override;
	virtual int update(screen_device &screen, const rectangle &cliprect) override;

private:

	uint32_t compute_code(int32_t x, int32_t y);
	uint32_t line_clip(int32_t *pX1, int32_t *pY1, int32_t *pX2, int32_t *pY2);
	void     cmd_vec_postproc();
	void     cmd_reset(uint32_t initial);
	void     cmd_add_vec(int x, int y, rgb_t color, bool screen_coords);
	void     cmd_add_point(int x, int y, rgb_t color);
	void     get_dvg_info();
	int      serial_read(uint8_t *buf, int size);
	int      serial_write(uint8_t *buf, int size);
	int      serial_send();
	void     transform_and_scale_coords(int *px, int *py);
	int      determine_game_settings() ;
	void     transform_final(int *px, int *py);

	static const game_info_t    s_games[];
	bool                        m_exclude_blank_vectors;
	int32_t                     m_xmin;
	int32_t                     m_xmax;
	int32_t                     m_ymin;
	int32_t                     m_ymax;
	float                       m_xscale;
	float                       m_yscale;
	uint32_t                    m_cmd_offs; 
	std::unique_ptr<uint8_t[]>  m_cmd_buf;
	bool                        m_swap_xy;
	bool                        m_flip_x;
	bool                        m_flip_y;
	int32_t                     m_clipx_min;
	int32_t                     m_clipx_max;
	int32_t                     m_clipy_min;
	int32_t                     m_clipy_max;
	int32_t                     m_last_r;
	int32_t                     m_last_g;
	int32_t                     m_last_b;
	uint32_t                    m_artwork;
	bool                        m_bw_game;
	std::unique_ptr<vector_t[]> m_in_vec_list;
	uint32_t                    m_in_vec_cnt;
	uint32_t                    m_in_vec_last_x;
	uint32_t                    m_in_vec_last_y;
	std::unique_ptr<vector_t[]> m_out_vec_list;
	uint32_t                    m_out_vec_cnt;
	uint32_t                    m_vertical_display;
	bool                        m_mirror;
	osd_file::ptr               m_serial; 
	int                         m_json_length;
	std::unique_ptr<uint8_t[]>  m_json_buf;
	std::unique_ptr<float[]>    m_gamma_table;
};


// device type definition
DECLARE_DEVICE_TYPE(ALT_VECTOR_USB_DVG, alt_vector_device_usb_dvg)

#endif // MAME_VIDEO_ALT_VECTOR_H
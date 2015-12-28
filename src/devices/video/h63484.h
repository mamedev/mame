// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Sandro Ronco
/*************************************************************************

  HD63484 ACRTC
  Advanced CRT Controller.

**************************************************************************/

#pragma once

#ifndef __H63484__
#define __H63484__


#include "emu.h"


typedef device_delegate<void (bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, UINT16 data)> h63484_display_delegate;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_H63484_ADD(_tag, _clock, _map) \
	MCFG_DEVICE_ADD(_tag, H63484, _clock) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map)

#define MCFG_H63484_ADDRESS_MAP(_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map)

#define MCFG_H63484_DISPLAY_CALLBACK_OWNER(_class, _method) \
	h63484_device::static_set_display_callback(*device, h63484_display_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define H63484_DISPLAY_PIXELS_MEMBER(_name) void _name(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, UINT16 data)


// ======================> h63484_device

class h63484_device :   public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	h63484_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_display_callback(device_t &device, h63484_display_delegate callback) { downcast<h63484_device &>(device).m_display_cb = callback; }

	DECLARE_WRITE16_MEMBER( address_w );
	DECLARE_WRITE16_MEMBER( data_w );
	DECLARE_READ16_MEMBER( status_r );
	DECLARE_READ16_MEMBER( data_r );

	DECLARE_WRITE8_MEMBER( address_w );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_READ8_MEMBER( data_r );

	UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual const rom_entry *device_rom_region() const override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	inline UINT16 readword(offs_t address);
	inline void writeword(offs_t address, UINT16 data);

private:
	inline void inc_ar(int value);
	inline void fifo_w_clear();
	inline void queue_w(UINT8 data);
	inline void dequeue_w(UINT8 *data);
	inline void fifo_r_clear();
	inline void queue_r(UINT8 data);
	inline void dequeue_r(UINT8 *data);
	inline void recompute_parameters();
	inline void command_end_seq();
	void calc_offset(INT16 x, INT16 y, UINT32 &offset, UINT8 &bit_pos);
	int get_bpp();
	UINT16 get_dot(INT16 x, INT16 y);
	bool set_dot(INT16 x, INT16 y, INT16 px, INT16 py);
	bool set_dot(INT16 x, INT16 y, UINT16 color);
	void draw_line(INT16 sx, INT16 sy, INT16 ex, INT16 ey);
	void draw_ellipse(INT16 cx, INT16 cy, double dx, double dy, double s_angol, double e_angol, bool c);
	void paint(INT16 sx, INT16 sy);

	void command_wpr_exec();
	UINT16 command_rpr_exec();
	void command_clr_exec();
	void command_cpy_exec();
	void command_rct_exec();
	void command_line_exec();
	void command_gcpy_exec();
	void command_ptn_exec();
	void command_plg_exec();
	void command_frct_exec();
	void command_arc_exec();
	void command_earc_exec();

	void process_fifo();
	void exec_abort_sequence();
	UINT16 video_registers_r(int offset);
	void video_registers_w(int offset);
	int translate_command(UINT16 data);
	void draw_graphics_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int vs, int y, int layer_n, bool active, bool ins_window);

	void register_save_state();

	h63484_display_delegate  m_display_cb;

	UINT8 m_ar;
	UINT8 m_vreg[0x100];
	UINT8 m_sr;

	UINT8 m_fifo[16];                   /* FIFO W data queue */
	int m_fifo_ptr;                 /* FIFO W pointer */

	UINT8 m_fifo_r[16];             /* FIFO R data queue */
	int m_fifo_r_ptr;                   /* FIFO R pointer */


	UINT16 m_cr;
	UINT16 m_pr[0x100];                  /* parameter byte register */
	int m_param_ptr;                    /* parameter pointer */

	UINT32 m_rwp[4];
	UINT8 m_rwp_dn;

	UINT32 m_org_dpa;
	UINT8 m_org_dn;
	UINT8 m_org_dpd;
	UINT16 m_cl0;
	UINT16 m_cl1;
	UINT16 m_ccmp;
	UINT16 m_mask;

	INT16 m_cpx;
	INT16 m_cpy;

	UINT16 m_mwr[4];
	UINT8  m_mwr_chr[4];

	UINT32 m_sar[4];
	UINT8 m_sda[4];

	UINT16 m_pram[0x10];
	UINT8 m_dn;

	UINT16 m_ccr;
	UINT16 m_omr;
	UINT16 m_edg;
	UINT16 m_dcr;

	UINT16 m_hc, m_hds, m_hdw, m_hws, m_hww;
	UINT16 m_sp[3];
	UINT8 m_hsw;

	UINT16 m_vc, m_vws, m_vww, m_vds;
	UINT8 m_vsw;

	UINT16 m_ppy;
	UINT16 m_pzcy;
	UINT16 m_ppx;
	UINT16 m_pzcx;
	UINT16 m_psx;
	UINT16 m_pex;
	UINT16 m_pzx;
	UINT16 m_psy;
	UINT16 m_pzy;
	UINT16 m_pey;

	UINT16 m_xmin;
	UINT16 m_ymin;
	UINT16 m_xmax;
	UINT16 m_ymax;

	const address_space_config      m_space_config;
};

// device type definition
extern const device_type H63484;

#endif /* __H63484_H__ */

// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Sandro Ronco
/*************************************************************************

  HD63484 ACRTC
  Advanced CRT Controller.

**************************************************************************/

#pragma once

#ifndef __HD63484__
#define __HD63484__




typedef device_delegate<void (bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint16_t data)> hd63484_display_delegate;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_HD63484_ADD(_tag, _clock, _map) \
	MCFG_DEVICE_ADD(_tag, HD63484, _clock) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map)

#define MCFG_HD63484_ADDRESS_MAP(_map) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map)

#define MCFG_HD63484_DISPLAY_CALLBACK_OWNER(_class, _method) \
	hd63484_device::static_set_display_callback(*device, hd63484_display_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_HD63484_AUTO_CONFIGURE_SCREEN(_val) \
	hd63484_device::static_set_auto_configure_screen(*device, _val);

#define HD63484_DISPLAY_PIXELS_MEMBER(_name) void _name(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int x, uint16_t data)


// ======================> hd63484_device

class hd63484_device :   public device_t,
						public device_memory_interface,
						public device_video_interface
{
public:
	// construction/destruction
	hd63484_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void static_set_display_callback(device_t &device, hd63484_display_delegate callback) { downcast<hd63484_device &>(device).m_display_cb = callback; }
	static void static_set_auto_configure_screen(device_t &device, bool auto_configure_screen) { downcast<hd63484_device &>(device).m_auto_configure_screen = auto_configure_screen; }

	DECLARE_WRITE16_MEMBER( address_w );
	DECLARE_WRITE16_MEMBER( data_w );
	DECLARE_READ16_MEMBER( status_r );
	DECLARE_READ16_MEMBER( data_r );

	DECLARE_WRITE8_MEMBER( address_w );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_READ8_MEMBER( data_r );

	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	inline uint16_t readword(offs_t address);
	inline void writeword(offs_t address, uint16_t data);

private:
	inline void inc_ar(int value);
	inline void fifo_w_clear();
	inline void queue_w(uint8_t data);
	inline void dequeue_w(uint8_t *data);
	inline void fifo_r_clear();
	inline void queue_r(uint8_t data);
	inline void dequeue_r(uint8_t *data);
	inline void recompute_parameters();
	inline void command_end_seq();
	void calc_offset(int16_t x, int16_t y, uint32_t &offset, uint8_t &bit_pos);
	int get_bpp();
	uint16_t get_dot(int16_t x, int16_t y);
	bool set_dot(int16_t x, int16_t y, int16_t px, int16_t py);
	bool set_dot(int16_t x, int16_t y, uint16_t color);
	void draw_line(int16_t sx, int16_t sy, int16_t ex, int16_t ey);
	void draw_ellipse(int16_t cx, int16_t cy, double dx, double dy, double s_angol, double e_angol, bool c);
	void paint(int16_t sx, int16_t sy);

	void command_wpr_exec();
	uint16_t command_rpr_exec();
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
	uint16_t video_registers_r(int offset);
	void video_registers_w(int offset);
	int translate_command(uint16_t data);
	void draw_graphics_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int vs, int y, int layer_n, bool active, bool ins_window);

	void register_save_state();

	hd63484_display_delegate  m_display_cb;
	bool m_auto_configure_screen;

	uint8_t m_ar;
	uint8_t m_vreg[0x100];
	uint8_t m_sr;

	uint8_t m_fifo[16];                   /* FIFO W data queue */
	int m_fifo_ptr;                 /* FIFO W pointer */

	uint8_t m_fifo_r[16];             /* FIFO R data queue */
	int m_fifo_r_ptr;                   /* FIFO R pointer */


	uint16_t m_cr;
	uint16_t m_pr[0x100];                  /* parameter byte register */
	int m_param_ptr;                    /* parameter pointer */

	uint32_t m_rwp[4];
	uint8_t m_rwp_dn;

	uint32_t m_org_dpa;
	uint8_t m_org_dn;
	uint8_t m_org_dpd;
	uint16_t m_cl0;
	uint16_t m_cl1;
	uint16_t m_ccmp;
	uint16_t m_mask;

	int16_t m_cpx;
	int16_t m_cpy;

	uint16_t m_mwr[4];
	uint8_t  m_mwr_chr[4];

	uint32_t m_sar[4];
	uint8_t m_sda[4];

	uint16_t m_pram[0x10];
	uint8_t m_dn;

	uint16_t m_ccr;
	uint16_t m_omr;
	uint16_t m_edg;
	uint16_t m_dcr;

	uint16_t m_hc, m_hds, m_hdw, m_hws, m_hww;
	uint16_t m_sp[3];
	uint8_t m_hsw;

	uint16_t m_vc, m_vws, m_vww, m_vds;
	uint8_t m_vsw;

	uint16_t m_ppy;
	uint16_t m_pzcy;
	uint16_t m_ppx;
	uint16_t m_pzcx;
	uint16_t m_psx;
	uint16_t m_pex;
	uint16_t m_pzx;
	uint16_t m_psy;
	uint16_t m_pzy;
	uint16_t m_pey;

	uint16_t m_xmin;
	uint16_t m_ymin;
	uint16_t m_xmax;
	uint16_t m_ymax;

	const address_space_config      m_space_config;
};

// device type definition
extern const device_type HD63484;

#endif /* __HD63484_H__ */

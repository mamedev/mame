/*************************************************************************

  HD63484 ACRTC
  Advanced CRT Controller.

**************************************************************************/

#pragma once

#ifndef __H63484__
#define __H63484__


#include "emu.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_H63484_ADD(_tag, _clock, _config, _map) \
	MCFG_DEVICE_ADD(_tag, H63484, _clock) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map)

#define H63484_INTERFACE(name) \
	const h63484_interface (name) =

typedef void (*h63484_display_pixels_func)(device_t *device, bitmap_ind16 &bitmap, int y, int x, UINT16 data);
#define H63484_DISPLAY_PIXELS(name) void name(device_t *device, bitmap_ind16 &bitmap, int y, int x, UINT16 data)

// ======================> h63484_interface

struct h63484_interface
{
	const char *m_screen_tag;       /* screen we are acting on */
	h63484_display_pixels_func  m_display_cb;
};

// ======================> upd7220_device

class h63484_device :   public device_t,
						public device_memory_interface,
						public h63484_interface
{
public:
	// construction/destruction
	h63484_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE16_MEMBER( address_w );
	DECLARE_WRITE16_MEMBER( data_w );

	DECLARE_READ16_MEMBER( status_r );
	DECLARE_READ16_MEMBER( data_r );

	DECLARE_READ8_MEMBER( vram_r );
	DECLARE_WRITE8_MEMBER( vram_w );

	UINT32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual const rom_entry *device_rom_region() const;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	//virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
	virtual void device_config_complete();

	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);

private:
	inline void fifo_w_clear();
	inline void queue_w(UINT8 data);
	inline void dequeue_w(UINT8 *data);
	inline void fifo_r_clear();
	inline void queue_r(UINT8 data);
	inline void dequeue_r(UINT8 *data);
	inline void recompute_parameters();
	inline void command_end_seq();

	void command_wpr_exec();
	void command_clr_exec();
	void command_cpy_exec();
	void command_rct_exec();
	void process_fifo();
	void exec_abort_sequence();
	UINT16 video_registers_r(int offset);
	void video_registers_w(int offset);
	int translate_command(UINT16 data);
	void draw_graphics_line(bitmap_ind16 &bitmap, const rectangle &cliprect, int y, int layer_n);


	screen_device *m_screen;

	UINT8 *m_vram;
	UINT8 m_ar;
	UINT8 m_vreg[0x100];
	UINT8 m_sr;

	UINT8 m_fifo[16];                   /* FIFO W data queue */
	int m_fifo_ptr;                 /* FIFO W pointer */

	UINT8 m_fifo_r[16];             /* FIFO R data queue */
	int m_fifo_r_ptr;                   /* FIFO R pointer */


	UINT16 m_cr;
	UINT16 m_pr[0x10];                  /* parameter byte register */
	int m_param_ptr;                    /* parameter pointer */

	UINT32 m_rwp[4];
	UINT8 m_rwp_dn;

	UINT32 m_org_dpa;
	UINT8 m_org_dn;
	UINT8 m_org_dpd;
	UINT16 m_cl0;
	UINT16 m_cl1;

	INT16 m_cpx;
	INT16 m_cpy;

	UINT16 m_mwr[4];
	UINT8  m_mwr_chr[4];

	UINT32 m_sar[4];
	UINT8 m_sda[4];

	UINT16 m_pram[0x10];
	UINT8 m_dn;

	UINT16 m_ccr;
	UINT16 m_dcr;

	UINT16 m_hc, m_hds, m_hdw, m_hws, m_hww;
	UINT8 m_hsw;

	UINT16 m_vc, m_vws, m_vww, m_vds;
	UINT8 m_vsw;

	const address_space_config      m_space_config;
};

// device type definition
extern const device_type H63484;

#endif /* __H63484_H__ */

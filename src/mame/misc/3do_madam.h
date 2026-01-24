// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Wilbert Pol

#ifndef MAME_MISC_3DO_MADAM_H
#define MAME_MISC_3DO_MADAM_H

#pragma once

#include "3do_amy.h"

class madam_device : public device_t
{
public:
	// construction/destruction
	madam_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_amy_tag(T &&tag) { m_amy.set_tag(std::forward<T>(tag)); }

	auto diag_cb()              { return m_diag_cb.bind(); }
	auto dma8_read_cb()         { return m_dma8_read_cb.bind(); }
	auto dma32_read_cb()        { return m_dma32_read_cb.bind(); }
	auto dma32_write_cb()       { return m_dma32_write_cb.bind(); }
	auto playerbus_read_cb()    { return m_playerbus_read_cb.bind(); }
	auto irq_dply_cb()          { return m_irq_dply_cb.bind(); }

	void map(address_map &map);

	void vdlp_start_w(int state);
	void vdlp_continue_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device<amy_device> m_amy;
	devcb_write8     m_diag_cb;
	devcb_read8      m_dma8_read_cb;
	devcb_read32     m_dma32_read_cb;
	devcb_write32    m_dma32_write_cb;
	devcb_read32     m_playerbus_read_cb;
	devcb_write_line m_irq_dply_cb;

	uint32_t  m_revision = 0;       /* 03300000 */
	uint32_t  m_msysbits = 0;       /* 03300004 */
	uint32_t  m_mctl = 0;           /* 03300008 */
	uint32_t  m_sltime = 0;         /* 0330000c */
	uint32_t  m_abortbits = 0;      /* 03300020 */
	uint32_t  m_privbits = 0;       /* 03300024 */
	uint32_t  m_statbits = 0;       /* 03300028 */
	uint32_t  m_diag = 0;           /* 03300040 */

	uint32_t  m_ccobctl0 = 0;       /* 03300110 */
	uint32_t  m_ppmpc = 0;          /* 03300120 */

	uint32_t  m_regctl0 = 0;        /* 03300130 */
	uint32_t  m_regctl1 = 0;        /* 03300134 */
	uint32_t  m_regctl2 = 0;        /* 03300138 */
	uint32_t  m_regctl3 = 0;        /* 0330013c */
	uint32_t  m_xyposh = 0;         /* 03300140 */
	uint32_t  m_xyposl = 0;         /* 03300144 */
	uint32_t  m_linedxyh = 0;       /* 03300148 */
	uint32_t  m_linedxyl = 0;       /* 0330014c */
	uint32_t  m_dxyh = 0;           /* 03300150 */
	uint32_t  m_dxyl = 0;           /* 03300154 */
	uint32_t  m_ddxyh = 0;          /* 03300158 */
	uint32_t  m_ddxyl = 0;          /* 0330015c */

	uint32_t  m_pip[16]{};          /* 03300180-033001bc (W); 03300180-033001fc (R) */
	uint32_t  m_fence[4]{};         /* 03300200-0330023c (W); 03300200-0330027c (R) */
	uint32_t  m_mmu[64]{};          /* 03300300-033003fc */
	uint32_t  m_dma[32][4]{};       /* 03300400-033005fc */
	uint32_t  m_mult[40]{};         /* 03300600-0330069c */
	uint32_t  m_mult_control = 0;   /* 033007f0-033007f4 */
	uint32_t  m_mult_status = 0;    /* 033007f8 */

	struct {
		u32 address;
		u16 scanlines;
		u16 modulo;
		u32 fb_address;
		bool fetch;
		u16 y_dest;
		u16 y_src;
		u32 link;
		bool video_dma;
	} m_vdlp;

	enum cel_state_t : u8 {
		IDLE,
		FETCH_PARAMS,
		DECOMPRESS,
		DRAW
	};

	struct {
		cel_state_t state;
		u32 address;
		u32 current_ccb;
		bool skip, last, ccbpre, packed, bgnd;
		u32 next_ptr;
		u32 source_ptr;
		u32 plut_ptr;
		s32 xpos, ypos;
		u32 hdx, hdy, vdx, vdy;
		u32 hddx, hddy;
		u32 pixc, pre0, pre1;
		std::vector<u16> buffer;
	} m_cel;

	struct {
		u16 fb_pitch[2]; // regctl0 helper
		u16 xclip, yclip; // regctl1 helper
	} m_regis;

	u32 mctl_r();
	void mctl_w(offs_t offset, u32 data, u32 mem_mask);
	u32 regctl0_r();
	void regctl0_w(offs_t offset, u32 data, u32 mem_mask);

	void cel_start_w(offs_t offset, u32 data, u32 mem_mask);
	void cel_stop_w(offs_t offset, u32 data, u32 mem_mask);
	u32 cel_decompress();

	typedef u16 (madam_device::*get_pixel_func)(int x, int y, u16 woffset);
	static const get_pixel_func get_pixel_table[4];
	u16 get_uncompressed_16bpp_lrform0(int x, int y, u16 woffset);
	u16 get_uncompressed_16bpp_lrform1(int x, int y, u16 woffset);
	u16 get_compressed_16bpp(int x, int y, u16 woffset);

	typedef u16 (madam_device::*get_woffset_func)(u32 ptr);
	static const get_woffset_func get_woffset_table[2];
	u16 get_woffset8(u32 ptr);
	u16 get_woffset10(u32 ptr);

	std::tuple<u8, u32> fetch_byte(u32 ptr, u8 frac);

	typedef std::tuple<u16, u32> (madam_device::*fetch_rle_func)(u32 ptr, u8 frac);
	static const fetch_rle_func fetch_rle_table[16];

	std::tuple<u16, u32> get_unemulated(u32 ptr, u8 frac);
	std::tuple<u16, u32> get_coded_4bpp(u32 ptr, u8 frac);
	std::tuple<u16, u32> get_coded_6bpp(u32 ptr, u8 frac);
	std::tuple<u16, u32> get_coded_16bpp(u32 ptr, u8 frac);
	std::tuple<u16, u32> get_uncoded_16bpp(u32 ptr, u8 frac);

	TIMER_CALLBACK_MEMBER(cel_tick_cb);
	emu_timer *m_cel_timer;

	emu_timer *m_dma_playerbus_timer;
	TIMER_CALLBACK_MEMBER(dma_playerbus_cb);
};

DECLARE_DEVICE_TYPE(MADAM, madam_device)


#endif // MAME_MISC_3DO_MADAM_H

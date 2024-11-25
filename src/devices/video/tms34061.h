// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Aaron Giles
/****************************************************************************
 *                                                                          *
 *  Function prototypes and constants used by the TMS34061 emulator         *
 *                                                                          *
 *  Created by Zsolt Vasvari on 5/26/1998.                                  *
 *  Updated by Aaron Giles on 11/21/2000.                                   *
 *                                                                          *
 ****************************************************************************/

#ifndef MAME_VIDEO_TMS34061_H
#define MAME_VIDEO_TMS34061_H

#pragma once


// ======================> tms34061_device

class tms34061_device :  public device_t, public device_video_interface
{
public:
	/* display state structure */
	struct tms34061_display
	{
		u8      blanked;        /* true if blanked */
		u8      *vram;          /* base of VRAM */
		u8      *latchram;      /* base of latch RAM */
		u16     *regs;          /* pointer to array of registers */
		offs_t  dispstart;      /* display start */
	};

	// construction/destruction
	tms34061_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void set_rowshift(u8 rowshift) { m_rowshift = rowshift; }
	void set_vram_size(u32 vramsize) { m_vramsize = vramsize; }
	auto int_callback() { return m_interrupt_cb.bind(); }

	/* reads/writes to the 34061 */
	u8 read(int col, int row, int func);
	void write(int col, int row, int func, u8 data);

	/* latch settings */
	u8 latch_r();
	void latch_w(u8 data);

	/* video update handling */
	void get_display_state();

	bool blanked() const { return bool(m_display.blanked); }
	u8 const &vram(unsigned row) const { return m_display.vram[row << m_rowshift]; }
	u16 xyoffset() const { return m_display.regs[TMS34061_XYOFFSET]; }
	u16 xyaddress() const { return m_display.regs[TMS34061_XYADDRESS]; }
	u16 hvisible() const { return (m_display.regs[TMS34061_HORENDBLNK] - m_display.regs[TMS34061_HORSTARTBLNK]) << 2; }
	u16 vvisible() const { return (m_display.regs[TMS34061_VERENDBLNK] - m_display.regs[TMS34061_VERSTARTBLNK]); }
	u16 htotal() const { return m_display.regs[TMS34061_HORTOTAL]; }
	u16 vtotal() const { return m_display.regs[TMS34061_VERTOTAL]; }

	// TODO: encapsulate this properly
	tms34061_display m_display;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	/* register constants */
	enum
	{
		TMS34061_HORENDSYNC = 0,
		TMS34061_HORENDBLNK,
		TMS34061_HORSTARTBLNK,
		TMS34061_HORTOTAL,
		TMS34061_VERENDSYNC,
		TMS34061_VERENDBLNK,
		TMS34061_VERSTARTBLNK,
		TMS34061_VERTOTAL,
		TMS34061_DISPUPDATE,
		TMS34061_DISPSTART,
		TMS34061_VERINT,
		TMS34061_CONTROL1,
		TMS34061_CONTROL2,
		TMS34061_STATUS,
		TMS34061_XYOFFSET,
		TMS34061_XYADDRESS,
		TMS34061_DISPADDRESS,
		TMS34061_VERCOUNTER,
		TMS34061_REGCOUNT
	};

	u8               m_rowshift;         /* VRAM address is (row << rowshift) | col */
	u32              m_vramsize;         /* size of video RAM */
	devcb_write_line   m_interrupt_cb;     /* interrupt gen callback */

	u16              m_regs[TMS34061_REGCOUNT];
	u16              m_xmask;
	u8               m_yshift;
	u32              m_vrammask;
	u8 *             m_vram;
	u8 *             m_latchram;
	u8               m_latchdata;
	u8 *             m_shiftreg;
	emu_timer *      m_timer;
	std::unique_ptr<u8[]> m_vram_alloc;
	std::unique_ptr<u8[]> m_latchram_alloc;

	void update_interrupts();
	TIMER_CALLBACK_MEMBER( interrupt );
	void register_w(offs_t offset, u8 data);
	u8 register_r(offs_t offset);
	void adjust_xyaddress(int offset);
	void xypixel_w(int offset, u8 data);
	u8 xypixel_r(int offset);
};

// device type definition
DECLARE_DEVICE_TYPE(TMS34061, tms34061_device)

#endif // MAME_VIDEO_TMS34061_H

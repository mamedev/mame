/****************************************************************************
 *                                                                          *
 *  Function prototypes and constants used by the TMS34061 emulator         *
 *                                                                          *
 *  Created by Zsolt Vasvari on 5/26/1998.                                  *
 *  Updated by Aaron Giles on 11/21/2000.                                   *
 *                                                                          *
 ****************************************************************************/


#ifndef __TMS34061_H__
#define __TMS34061_H__

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

/* interface structure */
struct tms34061_interface
{
	UINT8       m_rowshift;       /* VRAM address is (row << rowshift) | col */
	UINT32      m_vramsize;       /* size of video RAM */
	void        (*m_interrupt)(running_machine &machine, int state);  /* interrupt gen callback */
};

/* display state structure */
struct tms34061_display
{
	UINT8   blanked;        /* true if blanked */
	UINT8   *vram;          /* base of VRAM */
	UINT8   *latchram;      /* base of latch RAM */
	UINT16  *regs;          /* pointer to array of registers */
	offs_t  dispstart;      /* display start */
};




// ======================> tms34061_device

class tms34061_device :  public device_t,
						public device_video_interface,
						public tms34061_interface
{
public:
	// construction/destruction
	tms34061_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	/* reads/writes to the 34061 */
	UINT8 read(address_space &space, int col, int row, int func);
	void write(address_space &space, int col, int row, int func, UINT8 data);

	/* latch settings */
	DECLARE_READ8_MEMBER( latch_r );
	DECLARE_WRITE8_MEMBER( latch_w );

	/* video update handling */
	void get_display_state();

	struct tms34061_display m_display;

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	UINT16              m_regs[TMS34061_REGCOUNT];
	UINT16              m_xmask;
	UINT8               m_yshift;
	UINT32              m_vrammask;
	UINT8 *             m_vram;
	UINT8 *             m_latchram;
	UINT8               m_latchdata;
	UINT8 *             m_shiftreg;
	emu_timer *         m_timer;

	void update_interrupts(void);
	TIMER_CALLBACK_MEMBER( interrupt );
	void register_w(address_space &space, offs_t offset, UINT8 data);
	UINT8 register_r(address_space &space, offs_t offset);
	void adjust_xyaddress(int offset);
	void xypixel_w(address_space &space, int offset, UINT8 data);
	UINT8 xypixel_r(address_space &space, int offset);
};

// device type definition
extern const device_type TMS34061;

#define MCFG_TMS34061_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, TMS34061, 0) \
	MCFG_DEVICE_CONFIG(_interface)

#endif

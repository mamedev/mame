/*
** File: tms9928a.h -- software implementation of the TMS9928A VDP.
**
** By Sean Young 1999 (sean@msxnet.org).
*/

/*

    Model           Video       Hz

    TMS9918         NTSC        60
    TMS9918A        NTSC        60
    TMS9118         NTSC        60
    V9938
    V9958
    V9990

    TMS9928A        YPbPr       60
    TMS9128         YPbPr       60

    TMS9929A        YPbPr       50
    TMS9129         YPbPr       50

*/

#ifndef __TMS9928A_H__
#define __TMS9928A_H__

#include "emu.h"
#include "machine//devhelpr.h"


#define TMS9928A_PALETTE_SIZE           16


/* Some defines used in defining the screens */
#define TMS9928A_TOTAL_HORZ					342
#define TMS9928A_TOTAL_VERT_NTSC			262
#define TMS9928A_TOTAL_VERT_PAL				313

#define TMS9928A_HORZ_DISPLAY_START			(2 + 14 + 8 + 13)
#define TMS9928A_VERT_DISPLAY_START_PAL		(13 + 51)
#define TMS9928A_VERT_DISPLAY_START_NTSC	(13 + 27)


#define TMS9928A_INTERFACE(name) \
	const tms9928a_interface (name) =


#define MCFG_TMS9928A_ADD(_tag, _variant, _config) \
	MCFG_DEVICE_ADD(_tag, _variant, XTAL_10_738635MHz / 2 ) \
	MCFG_DEVICE_CONFIG(_config) \
	MCFG_PALETTE_LENGTH(TMS9928A_PALETTE_SIZE) \
	MCFG_PALETTE_INIT(tms9928a) \


#define MCFG_TMS9928A_SCREEN_ADD_NTSC(_screen_tag) \
	MCFG_SCREEN_ADD( _screen_tag, RASTER ) \
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16) \
	MCFG_SCREEN_RAW_PARAMS( XTAL_10_738635MHz / 2, TMS9928A_TOTAL_HORZ, TMS9928A_HORZ_DISPLAY_START-12, TMS9928A_HORZ_DISPLAY_START + 256 + 12, \
		 TMS9928A_TOTAL_VERT_NTSC, TMS9928A_VERT_DISPLAY_START_NTSC - 12, TMS9928A_VERT_DISPLAY_START_NTSC + 192 + 12 )


#define MCFG_TMS9928A_SCREEN_ADD_PAL(_screen_tag) \
	MCFG_SCREEN_ADD(_screen_tag, RASTER ) \
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16) \
	MCFG_SCREEN_RAW_PARAMS( XTAL_10_738635MHz / 2, TMS9928A_TOTAL_HORZ, TMS9928A_HORZ_DISPLAY_START-12, TMS9928A_HORZ_DISPLAY_START + 256 + 12,	\
		 TMS9928A_TOTAL_VERT_PAL, TMS9928A_VERT_DISPLAY_START_PAL - 12, TMS9928A_VERT_DISPLAY_START_PAL + 192 + 12 )


extern const device_type TMS9918;
extern const device_type TMS9918A;
extern const device_type TMS9118;
extern const device_type TMS9928A;
extern const device_type TMS9128;
extern const device_type TMS9929;
extern const device_type TMS9929A;
extern const device_type TMS9129;


typedef struct _tms9928a_interface tms9928a_interface;
struct _tms9928a_interface
{
	const char			*m_screen_tag;
	int					m_vram_size;	/* 4K, 8K, or 16K. This should be replaced by fetching data from an address space? */
	devcb_write_line	m_out_int_line;	/* Callback is called whenever the state of the INT output changes */
};


PALETTE_INIT( tms9928a );


class tms9928a_device :	public device_t,
						public tms9928a_interface
{
public:
	// construction/destruction
	tms9928a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms9928a_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, bool is_50hz = false, bool is_reva = true);

	DECLARE_READ8_MEMBER( vram_read );
	DECLARE_WRITE8_MEMBER( vram_write );
	DECLARE_READ8_MEMBER( register_read );
	DECLARE_WRITE8_MEMBER( register_write );

	/* update the screen */
	void update( bitmap_t *bitmap, const rectangle *cliprect );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return (spacenum == AS_0) ? &m_space_config : NULL; }

private:
	void change_register(UINT8 reg, UINT8 val);
	void check_interrupt();

	static const device_timer_id TIMER_LINE = 0;

	screen_device	*m_screen;

	/* TMS9928A internal settings */
	UINT8	m_ReadAhead;
	UINT8	m_Regs[8];
	UINT8	m_StatusReg;
	UINT8	m_FifthSprite;
	UINT8	m_latch;
	UINT8	m_INT;
	UINT16	m_Addr;
	UINT16	m_colour;
	UINT16	m_pattern;
	UINT16	m_nametbl;
	UINT16	m_spriteattribute;
	UINT16	m_spritepattern;
	int		m_colourmask;
	int		m_patternmask;
	devcb_resolved_write_line	m_irq_changed;
	bool	m_50hz;
	bool	m_reva;

	/* memory */
	const address_space_config		m_space_config;

	UINT8		*m_vMem;
	bitmap_t	*m_tmpbmp;
	emu_timer	*m_line_timer;
	UINT8		m_mode;

	/* emulation settings */
	int			m_top_border;
};


class tms9918_device : public tms9928a_device
{
public:
	tms9918_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: tms9928a_device( mconfig, TMS9918, "tms9918", tag, owner, clock, false, false ) { }
};


class tms9918a_device :	public tms9928a_device
{
public:
	tms9918a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: tms9928a_device( mconfig, TMS9918A, "tms9918a", tag, owner, clock, false, true ) { }
};


class tms9118_device : public tms9928a_device
{
public:
	tms9118_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: tms9928a_device( mconfig, TMS9118, "tms9118", tag, owner, clock, false, true ) { }
};


class tms9128_device : public tms9928a_device
{
public:
	tms9128_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: tms9928a_device( mconfig, TMS9128, "tms9128", tag, owner, clock, false, true ) { }
};


class tms9929_device : public tms9928a_device
{
public:
	tms9929_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: tms9928a_device( mconfig, TMS9929, "tms9929", tag, owner, clock, true, false ) { }
};


class tms9929a_device : public tms9928a_device
{
public:
	tms9929a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: tms9928a_device( mconfig, TMS9929A, "tms9929a", tag, owner, clock, true, true ) { }
};


class tms9129_device : public tms9928a_device
{
public:
	tms9129_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: tms9928a_device( mconfig, TMS9129, "tms9129", tag, owner, clock, true, true ) { }
};


#endif

/*****************************************************************************
 *
 * video/vic4567.h
 *
 ****************************************************************************/

#ifndef __VIC4567_H__
#define __VIC4567_H__

#include "devcb.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef UINT8 (*vic3_lightpen_x_callback)(running_machine &machine);
typedef UINT8 (*vic3_lightpen_y_callback)(running_machine &machine);
typedef UINT8 (*vic3_lightpen_button_callback)(running_machine &machine);

typedef int (*vic3_dma_read)(running_machine &machine, int);
typedef int (*vic3_dma_read_color)(running_machine &machine, int);
typedef void (*vic3_irq) (running_machine &, int);

typedef void (*vic3_port_changed_callback) (running_machine &, int);

typedef UINT8 (*vic3_c64mem_callback)(running_machine &machine, int offset);

enum vic3_type
{
	VIC4567_NTSC,
	VIC4567_PAL
};

struct vic3_interface
{
	const char         *screen;
	const char         *cpu;

	vic3_type type;

	vic3_lightpen_x_callback        x_cb;
	vic3_lightpen_y_callback        y_cb;
	vic3_lightpen_button_callback   button_cb;

	vic3_dma_read          dma_read;
	vic3_dma_read_color    dma_read_color;
	vic3_irq               irq;

	vic3_port_changed_callback        port_changed;

	vic3_c64mem_callback      c64_mem_r;
};

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VIC6567_CLOCK		(1022700 /* = 8181600 / 8) */ )
#define VIC6569_CLOCK		( 985248 /* = 7881984 / 8) */ )

#define VIC6567_CYCLESPERLINE	65
#define VIC6569_CYCLESPERLINE	63

#define VIC6567_LINES		263
#define VIC6569_LINES		312

#define VIC6567_VRETRACERATE	(59.8245100906698 /* = 1022700 / (65 * 263) */ )
#define VIC6569_VRETRACERATE	(50.1245421245421 /* =  985248 / (63 * 312) */ )

#define VIC6567_HRETRACERATE	(VIC6567_CLOCK / VIC6567_CYCLESPERLINE)
#define VIC6569_HRETRACERATE	(VIC6569_CLOCK / VIC6569_CYCLESPERLINE)

#define VIC2_HSIZE		320
#define VIC2_VSIZE		200

#define VIC6567_VISIBLELINES	235
#define VIC6569_VISIBLELINES	284

#define VIC6567_FIRST_DMA_LINE	0x30
#define VIC6569_FIRST_DMA_LINE	0x30

#define VIC6567_LAST_DMA_LINE	0xf7
#define VIC6569_LAST_DMA_LINE	0xf7

#define VIC6567_FIRST_DISP_LINE	0x29
#define VIC6569_FIRST_DISP_LINE	0x10

#define VIC6567_LAST_DISP_LINE	(VIC6567_FIRST_DISP_LINE + VIC6567_VISIBLELINES - 1)
#define VIC6569_LAST_DISP_LINE	(VIC6569_FIRST_DISP_LINE + VIC6569_VISIBLELINES - 1)

#define VIC6567_RASTER_2_EMU(a) ((a >= VIC6567_FIRST_DISP_LINE) ? (a - VIC6567_FIRST_DISP_LINE) : (a + 222))
#define VIC6569_RASTER_2_EMU(a) (a - VIC6569_FIRST_DISP_LINE)

#define VIC6567_FIRSTCOLUMN	50
#define VIC6569_FIRSTCOLUMN	50

#define VIC6567_VISIBLECOLUMNS	418
#define VIC6569_VISIBLECOLUMNS	403

#define VIC6567_X_2_EMU(a)	(a)
#define VIC6569_X_2_EMU(a)	(a)

#define VIC6567_STARTVISIBLELINES ((VIC6567_LINES - VIC6567_VISIBLELINES)/2)
#define VIC6569_STARTVISIBLELINES 16 /* ((VIC6569_LINES - VIC6569_VISIBLELINES)/2) */

#define VIC6567_FIRSTRASTERLINE 34
#define VIC6569_FIRSTRASTERLINE 0

#define VIC6567_COLUMNS 512
#define VIC6569_COLUMNS 504


#define VIC6567_STARTVISIBLECOLUMNS ((VIC6567_COLUMNS - VIC6567_VISIBLECOLUMNS)/2)
#define VIC6569_STARTVISIBLECOLUMNS ((VIC6569_COLUMNS - VIC6569_VISIBLECOLUMNS)/2)

#define VIC6567_FIRSTRASTERCOLUMNS 412
#define VIC6569_FIRSTRASTERCOLUMNS 404

#define VIC6569_FIRST_X 0x194
#define VIC6567_FIRST_X 0x19c

#define VIC6569_FIRST_VISIBLE_X 0x1e0
#define VIC6567_FIRST_VISIBLE_X 0x1e8

#define VIC6569_MAX_X 0x1f7
#define VIC6567_MAX_X 0x1ff

#define VIC6569_LAST_VISIBLE_X 0x17c
#define VIC6567_LAST_VISIBLE_X 0x184

#define VIC6569_LAST_X 0x193
#define VIC6567_LAST_X 0x19b

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

class vic3_device : public device_t
{
public:
	vic3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~vic3_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type VIC3;


#define MCFG_VIC3_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, VIC3, 0) \
	MCFG_DEVICE_CONFIG(_interface)


/*----------- defined in video/vic4567.c -----------*/

DECLARE_WRITE8_DEVICE_HANDLER( vic3_port_w );
DECLARE_WRITE8_DEVICE_HANDLER( vic3_palette_w );
DECLARE_READ8_DEVICE_HANDLER( vic3_port_r );

void vic3_raster_interrupt_gen( device_t *device );
UINT32 vic3_video_update( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect );


#endif /* __VIC4567_H__ */

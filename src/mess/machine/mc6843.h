/**********************************************************************

  Copyright (C) Antoine Mine' 2007

  Motorola 6843 Floppy Disk Controller emulation.

**********************************************************************/

#ifndef MC6843_H
#define MC6843_H

DECLARE_LEGACY_DEVICE(MC6843, mc6843);


/* ---------- configuration ------------ */

typedef struct _mc6843_interface mc6843_interface;
struct _mc6843_interface
{
	void ( * irq_func ) ( device_t *device, int state );
};


#define MCFG_MC6843_ADD(_tag, _intrf) \
  MCFG_DEVICE_ADD(_tag, MC6843, 0)	      \
  MCFG_DEVICE_CONFIG(_intrf)

#define MCFG_MC6843_REMOVE(_tag)		\
  MCFG_DEVICE_REMOVE(_tag)


/* ---------- functions ------------ */

extern READ8_DEVICE_HANDLER  ( mc6843_r );
extern WRITE8_DEVICE_HANDLER ( mc6843_w );

extern void mc6843_set_drive ( device_t *device, int drive );
extern void mc6843_set_side  ( device_t *device, int side );
extern void mc6843_set_index_pulse ( device_t *device, int index_pulse );

#endif

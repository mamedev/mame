#pragma once

#ifndef __ABC830__
#define __ABC830__


#include "emu.h"
#include "formats/basicdsk.h"
#include "imagedev/flopdrv.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ABC830_ADD() \
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(abc830_floppy_interface)

#define MCFG_ABC832_ADD() \
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(abc832_floppy_interface)

#define MCFG_ABC834_ADD() \
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(abc832_floppy_interface)

#define MCFG_ABC838_ADD() \
	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(abc838_floppy_interface)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// floppy configuration
extern const floppy_interface abc830_floppy_interface;
extern const floppy_interface abc832_floppy_interface;
extern const floppy_interface abc834_floppy_interface;
extern const floppy_interface abc838_floppy_interface;
extern const floppy_interface fd2_floppy_interface;



#endif

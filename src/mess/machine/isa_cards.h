/**********************************************************************

    ISA cards

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

**********************************************************************/

#pragma once

#ifndef __ISA_CARDS_H__
#define __ISA_CARDS_H__

#include "emu.h"

// video
#include "video/isa_mda.h"
#include "video/isa_cga.h"
#include "video/isa_ega.h"
#include "video/isa_vga.h"
#include "video/isa_vga_ati.h"
#include "video/isa_svga_cirrus.h"
#include "video/isa_svga_s3.h"
#include "video/isa_svga_tseng.h"

// storage
#include "machine/isa_fdc.h"
#include "machine/isa_hdc.h"
#include "machine/isa_wdxt_gen.h"
#include "machine/isa_ide.h"
#include "machine/isa_xtide.h"
#include "machine/isa_side116.h"
#include "machine/isa_aha1542.h"
#include "machine/isa_wd1002a_wx1.h"

// sound
#include "machine/isa_adlib.h"
#include "machine/isa_gblaster.h"
#include "machine/isa_gus.h"
#include "machine/isa_ibm_mfc.h"
#include "machine/isa_mpu401.h"
#include "machine/isa_sblaster.h"
#include "machine/isa_ssi2001.h"
#include "machine/isa_stereo_fx.h"
#include "machine/isa_dectalk.h"

// network
#include "machine/3c503.h"
#include "machine/ne1000.h"
#include "machine/ne2000.h"

// communication ports
#include "machine/pc_lpt.h"
#include "machine/isa_com.h"
#include "machine/isa_pds.h"

// other
#include "machine/isa_finalchs.h"

// supported devices
SLOT_INTERFACE_EXTERN( pc_isa8_cards );
SLOT_INTERFACE_EXTERN( pc_isa16_cards );

#endif // __ISA_CARDS_H__

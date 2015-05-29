// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/**********************************************************************

    NES carts

**********************************************************************/

#pragma once

#ifndef __NES_CARTS_H__
#define __NES_CARTS_H__

#include "emu.h"

// official PCBs
#include "nxrom.h"
#include "mmc1.h"
#include "mmc2.h"
#include "mmc3.h"
#include "mmc5.h"
#include "bandai.h"
#include "datach.h"
#include "discrete.h"
#include "disksys.h"
#include "event.h"
#include "irem.h"
#include "jaleco.h"
#include "karastudio.h"
#include "konami.h"
#include "namcot.h"
#include "pt554.h"
#include "sunsoft.h"
#include "sunsoft_dcs.h"
#include "taito.h"
// unlicensed/bootleg/pirate PCBs
#include "2a03pur.h"
#include "act53.h"
#include "aladdin.h"
#include "ave.h"
#include "benshieng.h"
#include "camerica.h"
#include "cne.h"
#include "cony.h"
#include "ggenie.h"
#include "hes.h"
#include "henggedianzi.h"
#include "hosenkan.h"
#include "jy.h"
#include "kaiser.h"
#include "legacy.h"
#include "nanjing.h"
#include "ntdec.h"
#include "racermate.h"
#include "rcm.h"
#include "rexsoft.h"
#include "sachen.h"
#include "somari.h"
#include "tengen.h"
#include "txc.h"
#include "waixing.h"
// misc unlicensed/bootleg/pirate PCBs
#include "bootleg.h"
#include "multigame.h"
#include "pirate.h"
#include "mmc3_clones.h"


// supported devices
SLOT_INTERFACE_EXTERN(nes_cart);
SLOT_INTERFACE_EXTERN(disksys_only);

#endif // __NES_CARTS_H__

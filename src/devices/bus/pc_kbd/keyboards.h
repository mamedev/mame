// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    PC Keyboards

***************************************************************************/

#ifndef MAME_BUS_PC_KBD_KEYBOARDS_H
#define MAME_BUS_PC_KBD_KEYBOARDS_H

#pragma once


// PC XT protocol keyboards
#define STR_KBD_KEYTRONIC_PC3270    "keytronc_pc3270"
#define STR_KBD_IBM_PC_83           "pc"
#define STR_KBD_IBM_PC_XT_83        "pcxt"
#define STR_KBD_EC_1841             "ec1841"
#define STR_KBD_ISKR_1030           "iskr1030"

void pc_xt_keyboards(device_slot_interface &device);

// PC AT protocol keyboards
// Reuses STR_KBD_KEYTRONIC_PC3270 (same keyboard in AT protocol mode by default)

#define STR_KBD_MICROSOFT_NATURAL   "ms_naturl"
#define STR_KBD_IBM_PC_AT_84        "pcat"
#define STR_KBD_IBM_3270PC_122      "3270pc"
#define STR_KBD_IBM_PC_AT_101       "pcat101"

void pc_at_keyboards(device_slot_interface &device);

#endif // MAME_BUS_PC_KBD_KEYBOARDS_H

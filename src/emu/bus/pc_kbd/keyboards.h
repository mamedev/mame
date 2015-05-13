// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    PC Keyboards

***************************************************************************/

#ifndef __PC_KEYBOARDS_H__
#define __PC_KEYBOARDS_H__


// PC XT protocol keyboards
#define STR_KBD_KEYTRONIC_PC3270    "keytronc_pc3270"
#define STR_KBD_IBM_PC_83           "pc"
#define STR_KBD_IBM_PC_XT_83        "pcxt"
#define STR_KBD_EC_1841             "ec1841"
#define STR_KBD_ISKR_1030           "iskr1030"

SLOT_INTERFACE_EXTERN(pc_xt_keyboards);

// PC AT protocol keyboards
// Reuses STR_KBD_KEYTRONIC_PC3270 (same keyboard in AT protocol mode by default)

#define STR_KBD_MICROSOFT_NATURAL   "ms_naturl"
#define STR_KBD_IBM_PC_AT_84        "pcat"
#define STR_KBD_IBM_3270PC_122      "3270pc"

SLOT_INTERFACE_EXTERN(pc_at_keyboards);

#endif  /* __KB_KEYBOARDS_H__ */

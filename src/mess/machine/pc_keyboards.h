/***************************************************************************

    PC Keyboards

***************************************************************************/

#ifndef __PC_KEYBOARDS_H__
#define __PC_KEYBOARDS_H__


// PC XT protocol keyboards
#define STR_KBD_KEYTRONIC_PC3270    "keytronc_pc3270"

SLOT_INTERFACE_EXTERN(pc_xt_keyboards);

// PC AT protocol keyboards
// Reuses STR_KBD_KEYTRONIC_PC3270 (same keyboard in AT protocol mode by default)

#define STR_KBD_MICROSOFT_NATURAL   "ms_naturl"

SLOT_INTERFACE_EXTERN(pc_at_keyboards);

#endif  /* __KB_KEYBOARDS_H__ */

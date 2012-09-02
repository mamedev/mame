#pragma once

#ifndef CBMIPT_H_
#define CBMIPT_H_

#include "machine/c2n.h"
#include "machine/c64_4cga.h"
#include "machine/c64_4dxh.h"
#include "machine/c64_4ksa.h"
#include "machine/c64_4tba.h"
#include "machine/c64_16kb.h"
#include "machine/c64_bn1541.h"
#include "machine/c64_comal80.h"
#include "machine/c64_cpm.h"
#include "machine/c64_currah_speech.h"
#include "machine/c64_dela_ep256.h"
#include "machine/c64_dela_ep64.h"
#include "machine/c64_dela_ep7x8.h"
#include "machine/c64_dinamic.h"
#include "machine/c64_dqbb.h"
#include "machine/c64_easy_calc_result.h"
#include "machine/c64_easyflash.h"
#include "machine/c64_epyx_fast_load.h"
#include "machine/c64_exos.h"
#include "machine/c64_final.h"
#include "machine/c64_final3.h"
#include "machine/c64_fun_play.h"
#include "machine/c64_geocable.h"
#include "machine/c64_georam.h"
#include "machine/c64_ide64.h"
#include "machine/c64_ieee488.h"
#include "machine/c64_kingsoft.h"
#include "machine/c64_mach5.h"
#include "machine/c64_magic_desk.h"
#include "machine/c64_magic_formel.h"
#include "machine/c64_mikro_assembler.h"
#include "machine/c64_multiscreen.h"
#include "machine/c64_neoram.h"
#include "machine/c64_ocean.h"
#include "machine/c64_pagefox.h"
#include "machine/c64_prophet64.h"
#include "machine/c64_ps64.h"
#include "machine/c64_rex.h"
#include "machine/c64_rex_ep256.h"
#include "machine/c64_ross.h"
#include "machine/c64_sfx_sound_expander.h"
#include "machine/c64_silverrock.h"
#include "machine/c64_simons_basic.h"
#include "machine/c64_stardos.h"
#include "machine/c64_std.h"
#include "machine/c64_structured_basic.h"
#include "machine/c64_super_explode.h"
#include "machine/c64_super_games.h"
#include "machine/c64_sw8k.h"
#include "machine/c64_system3.h"
#include "machine/c64_tdos.h"
#include "machine/c64_vw64.h"
#include "machine/c64_warp_speed.h"
#include "machine/c64_westermann.h"
#include "machine/c64_xl80.h"
#include "machine/c64_zaxxon.h"
#include "machine/c128_comal80.h"
#include "machine/c1541.h"
#include "machine/c1551.h"
#include "machine/c1571.h"
#include "machine/c1581.h"
#include "machine/c2031.h"
#include "machine/c2040.h"
#include "machine/c8280.h"
#include "machine/d9060.h"
#include "machine/cmdhd.h"
#include "machine/diag264_lb_iec.h"
#include "machine/diag264_lb_tape.h"
#include "machine/diag264_lb_user.h"
#include "machine/fd2000.h"
#include "machine/interpod.h"
#include "machine/plus4_sid.h"
#include "machine/plus4_std.h"
#include "machine/serialbox.h"
#include "machine/softbox.h"
#include "machine/vic1010.h"
#include "machine/vic1110.h"
#include "machine/vic1111.h"
#include "machine/vic1112.h"
#include "machine/vic1210.h"
#include "machine/vic10std.h"
#include "machine/vic20std.h"
#include "machine/vic20_megacart.h"
#include "machine/vcs_joy.h"


#define MCFG_CBM_IEC_ADD(_intf, _default_drive) \
	MCFG_CBM_IEC_BUS_ADD(_intf) \
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, NULL, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, cbm_iec_devices, _default_drive, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, NULL, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, NULL, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, NULL, NULL)


#define MCFG_CBM_IEEE488_ADD(_intf, _default_drive) \
	MCFG_IEEE488_BUS_ADD(_intf) \
	MCFG_IEEE488_SLOT_ADD("ieee8", 8, cbm_ieee488_devices, _default_drive, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee9", 9, cbm_ieee488_devices, NULL, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee10", 10, cbm_ieee488_devices, NULL, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee11", 11, cbm_ieee488_devices, NULL, NULL)


/* Commodore 64 */

INPUT_PORTS_EXTERN( common_cbm_keyboard );	/* shared with c16, c65, c128 */
INPUT_PORTS_EXTERN( c64_special );
INPUT_PORTS_EXTERN( c64_controls );			/* shared with c65, c128, cbmb */


/* Commodore 16 */

INPUT_PORTS_EXTERN( c16_special );
INPUT_PORTS_EXTERN( c16_controls );


/* Commodore 65 */

INPUT_PORTS_EXTERN( c65_special );


/* Commodore 128 */

INPUT_PORTS_EXTERN( c128_special );


/* PET2001 */

INPUT_PORTS_EXTERN( pet_keyboard );
INPUT_PORTS_EXTERN( pet_business_keyboard );
INPUT_PORTS_EXTERN( pet_special );
INPUT_PORTS_EXTERN( pet_config );


/* CBMB 500 / 600/ 700 */

INPUT_PORTS_EXTERN( cbmb_keyboard );
INPUT_PORTS_EXTERN( cbmb_special );


/* Vic 20 */

INPUT_PORTS_EXTERN( vic_keyboard );
INPUT_PORTS_EXTERN( vic_special );
INPUT_PORTS_EXTERN( vic_controls );



SLOT_INTERFACE_EXTERN( cbm_datassette_devices );
SLOT_INTERFACE_EXTERN( cbm_iec_devices );
SLOT_INTERFACE_EXTERN( sx1541_iec_devices );
SLOT_INTERFACE_EXTERN( c128dcr_iec_devices );
SLOT_INTERFACE_EXTERN( c128d81_iec_devices );
SLOT_INTERFACE_EXTERN( cbm_ieee488_devices );
SLOT_INTERFACE_EXTERN( vic20_control_port_devices );
SLOT_INTERFACE_EXTERN( vic20_expansion_cards );
SLOT_INTERFACE_EXTERN( vic20_user_port_cards );
SLOT_INTERFACE_EXTERN( vic10_expansion_cards );
SLOT_INTERFACE_EXTERN( c64_expansion_cards );
SLOT_INTERFACE_EXTERN( c64_user_port_cards );
SLOT_INTERFACE_EXTERN( c128_expansion_cards );
SLOT_INTERFACE_EXTERN( plus4_datassette_devices );
SLOT_INTERFACE_EXTERN( plus4_expansion_cards );
SLOT_INTERFACE_EXTERN( plus4_user_port_cards );



#endif /* CBMIPT_H_ */

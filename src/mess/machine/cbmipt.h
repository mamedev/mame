#pragma once

#ifndef CBMIPT_H_
#define CBMIPT_H_

#include "machine/c2n.h"
#include "machine/c64/4cga.h"
#include "machine/c64/4dxh.h"
#include "machine/c64/4ksa.h"
#include "machine/c64/4tba.h"
#include "machine/c64/16kb.h"
#include "machine/c64/bn1541.h"
#include "machine/c64/comal80.h"
#include "machine/c64/cpm.h"
#include "machine/c64/currah_speech.h"
#include "machine/c64/dela_ep256.h"
#include "machine/c64/dela_ep64.h"
#include "machine/c64/dela_ep7x8.h"
#include "machine/c64/dinamic.h"
#include "machine/c64/dqbb.h"
#include "machine/c64/easy_calc_result.h"
#include "machine/c64/easyflash.h"
#include "machine/c64/epyx_fast_load.h"
#include "machine/c64/exos.h"
#include "machine/c64/fcc.h"
#include "machine/c64/final.h"
#include "machine/c64/final3.h"
#include "machine/c64/fun_play.h"
#include "machine/c64/geocable.h"
#include "machine/c64/georam.h"
#include "machine/c64/ide64.h"
#include "machine/c64/ieee488.h"
#include "machine/c64/kingsoft.h"
#include "machine/c64/mach5.h"
#include "machine/c64/magic_desk.h"
#include "machine/c64/magic_formel.h"
#include "machine/c64/magic_voice.h"
#include "machine/c64/midi_maplin.h"
#include "machine/c64/midi_namesoft.h"
#include "machine/c64/midi_passport.h"
#include "machine/c64/midi_sci.h"
#include "machine/c64/midi_siel.h"
#include "machine/c64/mikro_assembler.h"
#include "machine/c64/multiscreen.h"
#include "machine/c64/neoram.h"
#include "machine/c64/ocean.h"
#include "machine/c64/pagefox.h"
#include "machine/c64/prophet64.h"
#include "machine/c64/ps64.h"
#include "machine/c64/reu.h"
#include "machine/c64/rex.h"
#include "machine/c64/rex_ep256.h"
#include "machine/c64/ross.h"
#include "machine/c64/sfx_sound_expander.h"
#include "machine/c64/silverrock.h"
#include "machine/c64/simons_basic.h"
#include "machine/c64/stardos.h"
#include "machine/c64/std.h"
#include "machine/c64/structured_basic.h"
#include "machine/c64/super_explode.h"
#include "machine/c64/super_games.h"
#include "machine/c64/supercpu.h"
#include "machine/c64/sw8k.h"
#include "machine/c64/swiftlink.h"
#include "machine/c64/system3.h"
#include "machine/c64/tdos.h"
#include "machine/c64/turbo232.h"
#include "machine/c64/vizastar.h"
#include "machine/c64/vic1011.h"
#include "machine/c64/vw64.h"
#include "machine/c64/warp_speed.h"
#include "machine/c64/westermann.h"
#include "machine/c64/xl80.h"
#include "machine/c64/zaxxon.h"
#include "machine/c128_comal80.h"
#include "machine/cbm2_std.h"
#include "machine/cbm2_24k.h"
#include "machine/cbm2_graphic.h"
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
#include "machine/hardbox.h"
#include "machine/interpod.h"
#include "machine/pet_64k.h"
#include "machine/plus4_sid.h"
#include "machine/plus4_std.h"
#include "machine/serialbox.h"
#include "machine/shark.h"
#include "machine/softbox.h"
#include "machine/superpet.h"
#include "machine/vic1010.h"
#include "machine/vic1011.h"
#include "machine/vic1110.h"
#include "machine/vic1111.h"
#include "machine/vic1112.h"
#include "machine/vic1210.h"
#include "machine/vic10std.h"
#include "machine/vic20std.h"
#include "machine/vic20_megacart.h"


#define MCFG_CBM_IEC_ADD(_default_drive) \
	MCFG_CBM_IEC_SLOT_ADD("iec4", 4, cbm_iec_devices, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec8", 8, cbm_iec_devices, _default_drive) \
	MCFG_CBM_IEC_SLOT_ADD("iec9", 9, cbm_iec_devices, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec10", 10, cbm_iec_devices, NULL) \
	MCFG_CBM_IEC_SLOT_ADD("iec11", 11, cbm_iec_devices, NULL) \
	MCFG_CBM_IEC_BUS_ADD()


#define MCFG_CBM_IEEE488_ADD(_default_drive) \
	MCFG_IEEE488_SLOT_ADD("ieee8", 8, cbm_ieee488_devices, _default_drive) \
	MCFG_IEEE488_SLOT_ADD("ieee9", 9, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee10", 10, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_SLOT_ADD("ieee11", 11, cbm_ieee488_devices, NULL) \
	MCFG_IEEE488_BUS_ADD()


/* Commodore 64 */

INPUT_PORTS_EXTERN( common_cbm_keyboard );  /* shared with c16, c65, c128 */
INPUT_PORTS_EXTERN( c64_special );
INPUT_PORTS_EXTERN( c64_controls );         /* shared with c65, c128, cbmb */


/* Commodore 16 */

INPUT_PORTS_EXTERN( c16_special );
INPUT_PORTS_EXTERN( c16_controls );


/* Commodore 65 */

INPUT_PORTS_EXTERN( c65_special );


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



SLOT_INTERFACE_EXTERN( pet_expansion_cards );
SLOT_INTERFACE_EXTERN( pet_user_port_cards );
SLOT_INTERFACE_EXTERN( cbm2_expansion_cards );
SLOT_INTERFACE_EXTERN( cbm2_user_port_cards );
SLOT_INTERFACE_EXTERN( cbm_datassette_devices );
SLOT_INTERFACE_EXTERN( cbm_iec_devices );
SLOT_INTERFACE_EXTERN( sx1541_iec_devices );
SLOT_INTERFACE_EXTERN( c128dcr_iec_devices );
SLOT_INTERFACE_EXTERN( c128d81_iec_devices );
SLOT_INTERFACE_EXTERN( cbm_ieee488_devices );
SLOT_INTERFACE_EXTERN( cbm8296d_ieee488_devices );
SLOT_INTERFACE_EXTERN( vic20_control_port_devices );
SLOT_INTERFACE_EXTERN( vic20_expansion_cards );
SLOT_INTERFACE_EXTERN( vic20_user_port_cards );
SLOT_INTERFACE_EXTERN( vic10_expansion_cards );
SLOT_INTERFACE_EXTERN( c64_expansion_cards );
SLOT_INTERFACE_EXTERN( c64_user_port_cards );
SLOT_INTERFACE_EXTERN( plus4_datassette_devices );
SLOT_INTERFACE_EXTERN( plus4_expansion_cards );
SLOT_INTERFACE_EXTERN( plus4_user_port_cards );



#endif /* CBMIPT_H_ */

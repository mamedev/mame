// license:LGPL-2.1+
// copyright-holders:Angelo Salese, Olivier Galibert, David Haywood, Samuele Zannoli, R. Belmont, ElSemi
/*

naomi.h -> NAOMI includes

*/
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "cpu/arm7/arm7.h"
#include "cpu/z80/z80.h"
#include "machine/x76f100.h"
#include "machine/maple-dc.h"
#include "machine/dc-ctrl.h"
#include "machine/mie.h"
#include "machine/naomirom.h"
#include "machine/naomigd.h"
#include "machine/naomim1.h"
#include "machine/naomim2.h"
#include "machine/naomim4.h"
#include "machine/awboard.h"
#include "cpu/sh4/sh4.h"
#include "cpu/arm7/arm7core.h"
#include "sound/aica.h"
#include "machine/aicartc.h"
#include "machine/jvsdev.h"
#include "machine/jvs13551.h"
#include "dc.h"

enum {
	JVSBD_DEFAULT = 0,
	JVSBD_ADSTICK,
	JVSBD_LIGHTGUN,
	JVSBD_MAHJONG,
	JVSBD_KEYBOARD
};
class naomi_state : public dc_state
{
	public:
		naomi_state(const machine_config &mconfig, device_type type, std::string tag)
		: dc_state(mconfig, type, tag),
		pvr2_texture_ram(*this, "textureram2"),
		pvr2_framebuffer_ram(*this, "frameram2"),
		elan_ram(*this, "elan_ram"),
		m_awflash(*this, "awflash"),
		m_eeprom(*this, "main_eeprom")  { }

	/* Naomi 2 specific (To be moved) */
	optional_shared_ptr<UINT64> pvr2_texture_ram;
	optional_shared_ptr<UINT64> pvr2_framebuffer_ram;
	optional_shared_ptr<UINT64> elan_ram;
	optional_device<macronix_29l001mc_device> m_awflash;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	DECLARE_WRITE_LINE_MEMBER(aica_irq);
	DECLARE_WRITE_LINE_MEMBER(sh4_aica_irq);
	DECLARE_MACHINE_RESET(naomi);
	DECLARE_DRIVER_INIT(atomiswave);
	DECLARE_DRIVER_INIT(xtrmhnt2);
	DECLARE_DRIVER_INIT(naomigd);
	DECLARE_DRIVER_INIT(ggxx);
	DECLARE_DRIVER_INIT(ggxxrl);
	DECLARE_DRIVER_INIT(ggxxsla);
	DECLARE_DRIVER_INIT(naomi2);
	DECLARE_DRIVER_INIT(naomi);
	DECLARE_DRIVER_INIT(naomigd_mp);
	DECLARE_DRIVER_INIT(sfz3ugd);
	DECLARE_DRIVER_INIT(hotd2);
	DECLARE_DRIVER_INIT(qmegamis);
	DECLARE_DRIVER_INIT(gram2000);
	DECLARE_DRIVER_INIT(kick4csh);
	DECLARE_DRIVER_INIT(vf4evoct);
	DECLARE_DRIVER_INIT(naomi_mp);
	DECLARE_DRIVER_INIT(mvsc2);

	DECLARE_READ64_MEMBER( naomi_arm_r );
	DECLARE_WRITE64_MEMBER( naomi_arm_w );
	DECLARE_READ64_MEMBER( naomi_unknown1_r );
	DECLARE_WRITE64_MEMBER( naomi_unknown1_w );
	DECLARE_READ64_MEMBER( eeprom_93c46a_r );
	DECLARE_WRITE64_MEMBER( eeprom_93c46a_w );
	DECLARE_READ64_MEMBER( aw_flash_r );
	DECLARE_WRITE64_MEMBER( aw_flash_w );
	DECLARE_READ64_MEMBER( aw_modem_r );
	DECLARE_WRITE64_MEMBER( aw_modem_w );

	UINT8 m_mp_mux;
	DECLARE_CUSTOM_INPUT_MEMBER(naomi_mp_r);
	DECLARE_INPUT_CHANGED_MEMBER(naomi_mp_w);

	inline int decode_reg32_64(UINT32 offset, UINT64 mem_mask, UINT64 *shift);

	UINT16 actel_id;

	UINT8 aw_ctrl_type;

	UINT8 asciihex_to_dec(UINT8 in);
	void create_pic_from_retdat();

	DECLARE_READ64_MEMBER( naomi_biose_idle_skip_r );
	DECLARE_READ64_MEMBER( naomi_biosh_idle_skip_r );
	DECLARE_READ64_MEMBER( naomi2_biose_idle_skip_r );
	DECLARE_READ64_MEMBER( naomigd_ggxxsla_idle_skip_r );
	DECLARE_READ64_MEMBER( naomigd_ggxx_idle_skip_r );
	DECLARE_READ64_MEMBER( naomigd_ggxxrl_idle_skip_r );
	DECLARE_READ64_MEMBER( naomigd_sfz3ugd_idle_skip_r );
	DECLARE_READ64_MEMBER( hotd2_idle_skip_r );
	DECLARE_READ64_MEMBER( xtrmhnt2_hack_r );
};

MACHINE_CONFIG_EXTERN(naomi_aw_base);
INPUT_PORTS_EXTERN( naomi_debug );

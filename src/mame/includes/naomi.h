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
#include "machine/m3comm.h"
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
		naomi_state(const machine_config &mconfig, device_type type, const char *tag)
		: dc_state(mconfig, type, tag),
		pvr2_texture_ram(*this, "textureram2"),
		pvr2_framebuffer_ram(*this, "frameram2"),
		elan_ram(*this, "elan_ram"),
		m_awflash(*this, "awflash"),
		m_eeprom(*this, "main_eeprom")  { }

	/* Naomi 2 specific (To be moved) */
	optional_shared_ptr<uint64_t> pvr2_texture_ram;
	optional_shared_ptr<uint64_t> pvr2_framebuffer_ram;
	optional_shared_ptr<uint64_t> elan_ram;
	optional_device<macronix_29l001mc_device> m_awflash;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	void aica_irq(int state);
	void sh4_aica_irq(int state);
	void machine_reset_naomi();
	void init_atomiswave();
	void init_xtrmhnt2();
	void init_naomigd();
	void init_ggxx();
	void init_ggxxrl();
	void init_ggxxsla();
	void init_naomi2();
	void init_naomi();
	void init_naomigd_mp();
	void init_sfz3ugd();
	void init_hotd2();
	void init_naomi_mp();

	uint64_t naomi_arm_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void naomi_arm_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t naomi_unknown1_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void naomi_unknown1_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t eeprom_93c46a_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void eeprom_93c46a_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t aw_flash_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void aw_flash_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t aw_modem_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	void aw_modem_w(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask = U64(0xffffffffffffffff));

	uint8_t m_mp_mux;
	ioport_value naomi_mp_r(ioport_field &field, void *param);
	void naomi_mp_w(ioport_field &field, void *param, ioport_value oldval, ioport_value newval);

	inline int decode_reg32_64(uint32_t offset, uint64_t mem_mask, uint64_t *shift);

	uint8_t aw_ctrl_type;

	uint8_t asciihex_to_dec(uint8_t in);
	void create_pic_from_retdat();

	uint64_t naomi_biose_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t naomi_biosh_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t naomi2_biose_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t naomigd_ggxxsla_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t naomigd_ggxx_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t naomigd_ggxxrl_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t naomigd_sfz3ugd_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t hotd2_idle_skip_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
	uint64_t xtrmhnt2_hack_r(address_space &space, offs_t offset, uint64_t mem_mask = U64(0xffffffffffffffff));
};

MACHINE_CONFIG_EXTERN(naomi_aw_base);
INPUT_PORTS_EXTERN( naomi_debug );

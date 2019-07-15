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
#include "machine/nvram.h"
#include "cpu/sh/sh4.h"
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
		m_eeprom(*this, "main_eeprom"),
		m_rombase(*this, "rombase")
		{ }

	void naomi_base(machine_config &config);
	void naomim2(machine_config &config);
	void naomim2_kb(machine_config &config);
	void naomi(machine_config &config);
	void naomim1(machine_config &config);
	void naomigd(machine_config &config);
	void naomigd_kb(machine_config &config);
	void naomim4(machine_config &config);

	void init_naomigd();
	void init_ggxx();
	void init_ggxxrl();
	void init_ggxxsla();
	void init_naomi();
	void init_naomigd_mp();
	void init_sfz3ugd();
	void init_hotd2();
	void init_naomi_mp();

	DECLARE_CUSTOM_INPUT_MEMBER(naomi_mp_r);
	DECLARE_CUSTOM_INPUT_MEMBER(naomi_kb_r);
	DECLARE_INPUT_CHANGED_MEMBER(naomi_mp_w);

	DECLARE_READ64_MEMBER( naomi2_biose_idle_skip_r );

protected:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_shared_ptr<uint64_t> m_rombase;

	DECLARE_MACHINE_RESET(naomi);

	DECLARE_READ16_MEMBER( naomi_g2bus_r );
	DECLARE_READ64_MEMBER( eeprom_93c46a_r );
	DECLARE_WRITE64_MEMBER( eeprom_93c46a_w );

	uint8_t m_mp_mux;

	uint8_t asciihex_to_dec(uint8_t in);
	void create_pic_from_retdat();

	DECLARE_READ64_MEMBER( naomi_biose_idle_skip_r );
	DECLARE_READ64_MEMBER( naomi_biosh_idle_skip_r );
	DECLARE_READ64_MEMBER( naomigd_ggxxsla_idle_skip_r );
	DECLARE_READ64_MEMBER( naomigd_ggxx_idle_skip_r );
	DECLARE_READ64_MEMBER( naomigd_ggxxrl_idle_skip_r );
	DECLARE_READ64_MEMBER( naomigd_sfz3ugd_idle_skip_r );
	DECLARE_READ64_MEMBER( hotd2_idle_skip_r );

	void naomi_map(address_map &map);
	void naomi_port(address_map &map);

	void set_drc_options();
};

class naomi2_state : public naomi_state
{
public:
	naomi2_state(const machine_config &mconfig, device_type type, const char *tag)
		: naomi_state(mconfig, type, tag),
		m_pvr2_texture_ram(*this, "textureram2"),
		m_pvr2_framebuffer_ram(*this, "frameram2"),
		m_elan_ram(*this, "elan_ram"),
		m_powervr2_slave(*this, "powervr2_slave") { }

	void naomi2_base(machine_config &config);
	void naomi2m2(machine_config &config);
	void naomi2gd(machine_config &config);
	void naomi2m1(machine_config &config);

	void init_naomi2();

private:
	required_shared_ptr<uint64_t> m_pvr2_texture_ram;
	required_shared_ptr<uint64_t> m_pvr2_framebuffer_ram;
	required_shared_ptr<uint64_t> m_elan_ram;
	required_device<powervr2_device> m_powervr2_slave;

	DECLARE_WRITE32_MEMBER(both_pvr2_ta_w);
	void naomi2_map(address_map &map);
};

class atomiswave_state : public dc_state
{
public:
	atomiswave_state(const machine_config &mconfig, device_type type, const char *tag)
		: dc_state(mconfig, type, tag),
		m_awflash(*this, "awflash")
	   { }

	void aw_base(machine_config &config);
	void aw1c(machine_config &config);
	void aw2c(machine_config &config);

	void init_atomiswave();
	void init_xtrmhnt2();

private:
	required_device<macronix_29l001mc_device> m_awflash;

	DECLARE_READ64_MEMBER( aw_flash_r );
	DECLARE_WRITE64_MEMBER( aw_flash_w );
	DECLARE_READ64_MEMBER( aw_modem_r );
	DECLARE_WRITE64_MEMBER( aw_modem_w );

	DECLARE_READ64_MEMBER( xtrmhnt2_hack_r );

	void aw_map(address_map &map);
	void aw_port(address_map &map);

	uint8_t aw_ctrl_type;
	inline int decode_reg32_64(uint32_t offset, uint64_t mem_mask, uint64_t *shift);
};

INPUT_PORTS_EXTERN( naomi_debug );

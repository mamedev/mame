// license:BSD-3-Clause
// copyright-holders:Luca Elia, Mirko Buffoni, Takahiro Nogi

#include "sound/dac.h"
#include "sound/samples.h"
#include "video/seta001.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"
#include "machine/gen_latch.h"

#define MAX_SAMPLES 0x2f        /* max samples */

enum
{
	MCU_NONE_INSECTX = 0,
	MCU_NONE_KAGEKI,
	MCU_NONE_TNZSB,
	MCU_NONE_KABUKIZ,
	MCU_EXTRMATN,
	MCU_ARKANOID,
	MCU_PLUMPOP,
	MCU_DRTOPPEL,
	MCU_CHUKATAI,
	MCU_TNZS
};

class tnzs_state : public driver_device
{
public:
	tnzs_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_seta001(*this, "spritegen"),
		m_samples(*this, "samples"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_mainbank(*this, "mainbank"),
		m_subbank(*this, "subbank"),
		m_audiobank(*this, "audiobank"),
		m_dswa(*this, "DSWA"),
		m_dswb(*this, "DSWB"),
		m_in0(*this, "IN0"),
		m_in1(*this, "IN1"),
		m_in2(*this, "IN2"),
		m_coin1(*this, "COIN1"),
		m_coin2(*this, "COIN2"),
		m_an1(*this, "AN1"),
		m_an2(*this, "AN2")
		{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	optional_device<upi41_cpu_device> m_mcu;
	optional_device<seta001_device> m_seta001;
	optional_device<samples_device> m_samples;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;
	optional_device<address_map_bank_device> m_mainbank;
	optional_memory_bank m_subbank; /* optional because of reuse from cchance.c */
	optional_memory_bank m_audiobank;
	required_ioport m_dswa;
	required_ioport m_dswb;
	required_ioport m_in0;
	required_ioport m_in1;
	required_ioport m_in2;
	optional_ioport m_coin1;
	optional_ioport m_coin2;
	optional_ioport m_an1;
	optional_ioport m_an2;

	/* sound-related */
	std::unique_ptr<int16_t[]>    m_sampledata[MAX_SAMPLES];
	int      m_samplesize[MAX_SAMPLES];

	/* misc / mcu */
	int      m_kageki_csport_sel;
	int      m_input_select;
	int      m_mcu_type;
	int      m_mcu_initializing;
	int      m_mcu_coinage_init;
	int      m_mcu_command;
	int      m_mcu_readcredits;
	int      m_mcu_reportcoin;
	int      m_insertcoin;
	uint8_t    m_mcu_coinage[4];
	uint8_t    m_mcu_coins_a;
	uint8_t    m_mcu_coins_b;
	uint8_t    m_mcu_credits;
	int      m_bank2;

	void tnzsb_sound_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void jpopnics_subbankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t tnzs_port1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tnzs_port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tnzs_port2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t arknoid2_sh_f000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tnzs_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void tnzs_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tnzs_ramrom_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tnzs_bankswitch1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_tnzs_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_tnzs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_arknoid2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_arknoid2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_extrmatn_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_extrmatn_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t kageki_csport_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void kageki_csport_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void kabukiz_sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irqhandler(int state);

	SAMPLES_START_CB_MEMBER(kageki_init_samples);

	void init_arknoid2();
	void init_extrmatn();
	void init_drtoppel();
	void init_kabukiz();
	void init_plumpop();
	void init_insectx();
	void init_tnzs();
	void init_kageki();
	void init_chukatai();
	void init_tnzsb();
	void machine_start_tnzs();
	void machine_reset_tnzs();
	void palette_init_arknoid2(palette_device &palette);
	void machine_start_tnzs_common();
	void machine_reset_jpopnics();

	uint32_t screen_update_tnzs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_tnzs(screen_device &screen, bool state);

	void arknoid2_interrupt(device_t &device);

	void mcu_reset();
	void mcu_handle_coins(int coin);
};

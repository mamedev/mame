// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "machine/i8255.h"
#include "includes/galaxold.h"
#include "sound/tms5110.h"
#include "sound/digitalk.h"

class scramble_state : public galaxold_state
{
public:
	scramble_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxold_state(mconfig, type, tag),
		m_konami_7474(*this, "konami_7474"),
		m_ppi8255_0(*this, "ppi8255_0"),
		m_ppi8255_1(*this, "ppi8255_1"),
		m_tmsprom(*this, "tmsprom"),
		m_soundram(*this, "soundram"),
		m_digitalker(*this, "digitalker")
	{
	}

	optional_device<ttl7474_device> m_konami_7474;
	optional_device<i8255_device>  m_ppi8255_0;
	optional_device<i8255_device>  m_ppi8255_1;
	optional_device<tmsprom_device>  m_tmsprom;
	optional_shared_ptr<UINT8> m_soundram;
	optional_device<digitalker_device> m_digitalker;

	UINT8 m_cavelon_bank;

	// harem
	UINT8 m_harem_decrypt_mode;
	UINT8 m_harem_decrypt_bit;
	UINT8 m_harem_decrypt_clk;
	UINT8 m_harem_decrypt_count;
	std::unique_ptr<UINT8[]> m_harem_decrypted_data;
	std::unique_ptr<UINT8[]> m_harem_decrypted_opcodes;

	DECLARE_CUSTOM_INPUT_MEMBER(darkplnt_custom_r);
	DECLARE_CUSTOM_INPUT_MEMBER(ckongs_coinage_r);
	DECLARE_READ8_MEMBER(hncholms_prot_r);
	DECLARE_READ8_MEMBER(scramble_soundram_r);
	DECLARE_READ8_MEMBER(mars_ppi8255_0_r);
	DECLARE_READ8_MEMBER(mars_ppi8255_1_r);
	DECLARE_WRITE8_MEMBER(scramble_soundram_w);
	DECLARE_READ8_MEMBER(scramble_portB_r);
	DECLARE_READ8_MEMBER(hustler_portB_r);
	DECLARE_WRITE8_MEMBER(hotshock_sh_irqtrigger_w);
	DECLARE_READ8_MEMBER(hotshock_soundlatch_r);
	DECLARE_WRITE8_MEMBER(scramble_filter_w);
	DECLARE_WRITE8_MEMBER(frogger_filter_w);
	DECLARE_WRITE8_MEMBER(mars_ppi8255_0_w);
	DECLARE_WRITE8_MEMBER(mars_ppi8255_1_w);
	DECLARE_WRITE8_MEMBER(ad2083_tms5110_ctrl_w);

	// harem
	DECLARE_WRITE8_MEMBER(harem_decrypt_bit_w);
	DECLARE_WRITE8_MEMBER(harem_decrypt_clk_w);
	DECLARE_WRITE8_MEMBER(harem_decrypt_rst_w);
	DECLARE_READ8_MEMBER(harem_digitalker_intr_r);
	DECLARE_WRITE8_MEMBER(harem_digitalker_control_w);

	DECLARE_DRIVER_INIT(cavelon);
	DECLARE_DRIVER_INIT(mariner);
	DECLARE_DRIVER_INIT(mrkougb);
	DECLARE_DRIVER_INIT(scramble_ppi);
	DECLARE_DRIVER_INIT(mars);
	DECLARE_DRIVER_INIT(ckongs);
	DECLARE_DRIVER_INIT(mimonscr);
	DECLARE_DRIVER_INIT(hotshock);
	DECLARE_DRIVER_INIT(ad2083);
	DECLARE_DRIVER_INIT(devilfsh);
	DECLARE_DRIVER_INIT(mrkougar);
	DECLARE_DRIVER_INIT(harem);
	DECLARE_DRIVER_INIT(newsin7a);

	DECLARE_DRIVER_INIT(scobra);
	DECLARE_DRIVER_INIT(stratgyx);
	DECLARE_DRIVER_INIT(tazmani2);
	DECLARE_DRIVER_INIT(darkplnt);
	DECLARE_DRIVER_INIT(mimonkey);
	DECLARE_DRIVER_INIT(mimonsco);
	DECLARE_DRIVER_INIT(rescue);
	DECLARE_DRIVER_INIT(minefld);
	DECLARE_DRIVER_INIT(hustler);
	DECLARE_DRIVER_INIT(hustlerd);
	DECLARE_DRIVER_INIT(billiard);
	DECLARE_MACHINE_RESET(scramble);
	DECLARE_MACHINE_RESET(explorer);
	DECLARE_WRITE_LINE_MEMBER(scramble_sh_7474_q_callback);
	void cavelon_banksw();
	inline int bit(int i,int n);
	DECLARE_READ8_MEMBER( mariner_protection_1_r );
	DECLARE_READ8_MEMBER( mariner_protection_2_r );
	DECLARE_READ8_MEMBER( triplep_pip_r );
	DECLARE_READ8_MEMBER( triplep_pap_r );
	DECLARE_READ8_MEMBER( cavelon_banksw_r );
	DECLARE_WRITE8_MEMBER( cavelon_banksw_w );
	DECLARE_READ8_MEMBER( hunchbks_mirror_r );
	DECLARE_WRITE8_MEMBER( hunchbks_mirror_w );
	void sh_init();
	DECLARE_WRITE8_MEMBER( scramble_sh_irqtrigger_w );
	DECLARE_WRITE8_MEMBER( mrkougar_sh_irqtrigger_w );
	IRQ_CALLBACK_MEMBER( scramble_sh_irq_callback );
};

/*----------- defined in audio/scramble.c -----------*/

MACHINE_CONFIG_EXTERN( ad2083_audio );

#include "sound/dac.h"
#include "sound/samples.h"
#include "video/seta001.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/bankdev.h"

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
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_mcu(*this, "mcu"),
		m_maincpu(*this, "maincpu"),
		m_seta001(*this, "spritegen"),
		m_dac(*this, "dac"),
		m_samples(*this, "samples"),
		m_palette(*this, "palette"),
		m_mainbank(*this, "mainbank")
		{ }

	/* video-related */
	int      m_screenflip;

	/* sound-related */
	INT16    *m_sampledata[MAX_SAMPLES];
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
	UINT8    m_mcu_coinage[4];
	UINT8    m_mcu_coins_a;
	UINT8    m_mcu_coins_b;
	UINT8    m_mcu_credits;
	int      m_bank2;

	UINT8*   m_ROM;

	/* devices */
	optional_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_subcpu;
	optional_device<upi41_cpu_device> m_mcu;
	DECLARE_WRITE8_MEMBER(tnzsb_sound_command_w);
	DECLARE_WRITE8_MEMBER(jpopnics_subbankswitch_w);
	DECLARE_READ8_MEMBER(tnzs_port1_r);
	DECLARE_READ8_MEMBER(tnzs_port2_r);
	DECLARE_WRITE8_MEMBER(tnzs_port2_w);
	DECLARE_READ8_MEMBER(arknoid2_sh_f000_r);
	DECLARE_READ8_MEMBER(tnzs_mcu_r);
	DECLARE_WRITE8_MEMBER(tnzs_mcu_w);
	DECLARE_WRITE8_MEMBER(tnzs_ramrom_bankswitch_w);
	DECLARE_WRITE8_MEMBER(tnzs_bankswitch1_w);
	DECLARE_READ8_MEMBER(mcu_tnzs_r);
	DECLARE_WRITE8_MEMBER(mcu_tnzs_w);
	DECLARE_READ8_MEMBER(mcu_arknoid2_r);
	DECLARE_WRITE8_MEMBER(mcu_arknoid2_w);
	DECLARE_READ8_MEMBER(mcu_extrmatn_r);
	DECLARE_WRITE8_MEMBER(mcu_extrmatn_w);
	DECLARE_WRITE8_MEMBER(tnzs_sync_kludge_w);
	DECLARE_READ8_MEMBER(kageki_csport_r);
	DECLARE_WRITE8_MEMBER(kageki_csport_w);
	DECLARE_WRITE8_MEMBER(kabukiz_sound_bank_w);
	DECLARE_WRITE8_MEMBER(kabukiz_sample_w);

	SAMPLES_START_CB_MEMBER(kageki_init_samples);

	DECLARE_DRIVER_INIT(arknoid2);
	DECLARE_DRIVER_INIT(extrmatn);
	DECLARE_DRIVER_INIT(drtoppel);
	DECLARE_DRIVER_INIT(kabukiz);
	DECLARE_DRIVER_INIT(plumpop);
	DECLARE_DRIVER_INIT(insectx);
	DECLARE_DRIVER_INIT(tnzs);
	DECLARE_DRIVER_INIT(kageki);
	DECLARE_DRIVER_INIT(chukatai);
	DECLARE_DRIVER_INIT(tnzsb);
	DECLARE_MACHINE_START(tnzs);
	DECLARE_MACHINE_RESET(tnzs);
	DECLARE_PALETTE_INIT(arknoid2);
	DECLARE_MACHINE_START(tnzs_common);
	DECLARE_MACHINE_RESET(jpopnics);
	UINT32 screen_update_tnzs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_tnzs(screen_device &screen, bool state);
	INTERRUPT_GEN_MEMBER(arknoid2_interrupt);
	TIMER_CALLBACK_MEMBER(kludge_callback);
	void tnzs_postload();
	void mcu_reset(  );
	void mcu_handle_coins( int coin );
	DECLARE_WRITE_LINE_MEMBER(irqhandler);
	required_device<cpu_device> m_maincpu;
	optional_device<seta001_device> m_seta001;
	optional_device<dac_device> m_dac;
	optional_device<samples_device> m_samples;
	required_device<palette_device> m_palette;
	optional_device<address_map_bank_device> m_mainbank;
};

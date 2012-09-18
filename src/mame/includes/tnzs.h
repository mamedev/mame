
#define	MAX_SAMPLES	0x2f		/* max samples */

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
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT8 *  m_paletteram;    // currently this uses generic palette handling

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
	int      m_bank1;
	int      m_bank2;

	UINT8*	 m_bankedram;
	UINT8*	 m_ROM;

	/* devices */
	cpu_device *m_audiocpu;
	cpu_device *m_subcpu;
	device_t *m_mcu;
	DECLARE_WRITE8_MEMBER(tnzsb_sound_command_w);
	DECLARE_WRITE8_MEMBER(jpopnics_palette_w);
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

	DECLARE_READ8_MEMBER(tnzs_ramrom_bank_r);
	DECLARE_WRITE8_MEMBER(tnzs_ramrom_bank_w);

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
	DECLARE_MACHINE_START(jpopnics);
	DECLARE_MACHINE_RESET(jpopnics);
	UINT32 screen_update_tnzs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_tnzs(screen_device &screen, bool state);
};


/*----------- defined in machine/tnzs.c -----------*/
INTERRUPT_GEN( arknoid2_interrupt );

// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Machine Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi <nogi@kt.rim.or.jp> 1999/11/05 -

******************************************************************************/

#define NB1413M3_VCR_NOP        0x00
#define NB1413M3_VCR_POWER      0x01
#define NB1413M3_VCR_STOP       0x02
#define NB1413M3_VCR_REWIND     0x04
#define NB1413M3_VCR_PLAY       0x08
#define NB1413M3_VCR_FFORWARD   0x10
#define NB1413M3_VCR_PAUSE      0x20

enum {
	NB1413M3_NONE = 0,
	// unknown
	NB1413M3_JOKERMJN,
	NB1413M3_JANGOU,
	NB1413M3_JNGOLADY,
	NB1413M3_JNGNIGHT,
	NB1413M3_MJKING,
	NB1413M3_NIGHTGAL,
	NB1413M3_NGALSUMR,
	NB1413M3_ROYALNGT,
	NB1413M3_RYLQUEEN,
	NB1413M3_SWEETGAL,
	NB1413M3_SEXYGAL,
	// NB1411M1
	NB1413M3_PASTELG,
	NB1413M3_THREEDS,
	// NB1413M3
	NB1413M3_CRYSTALG,
	NB1413M3_CRYSTAL2,
	NB1413M3_NIGHTLOV,
	NB1413M3_CITYLOVE,
	NB1413M3_MCITYLOV,
	NB1413M3_SECOLOVE,
	NB1413M3_BARLINE,
	NB1413M3_HOUSEMNQ,
	NB1413M3_HOUSEMN2,
	NB1413M3_LIVEGAL,
	NB1413M3_BIJOKKOY,
	NB1413M3_IEMOTO,
	NB1413M3_IEMOTOM,
	NB1413M3_RYUUHA,
	NB1413M3_SEIHA,
	NB1413M3_SEIHAM,
	NB1413M3_HYHOO,
	NB1413M3_HYHOO2,
	NB1413M3_SWINGGAL,
	NB1413M3_BIJOKKOG,
	NB1413M3_OJOUSAN,
	NB1413M3_OJOUSANM,
	NB1413M3_KORINAI,
	NB1413M3_KORINAIM,
	NB1413M3_MJCAMERA,
	NB1413M3_MJCAMERB,
	NB1413M3_MMCAMERA,
	NB1413M3_TAIWANMJ,
	NB1413M3_TAIWANMB,
	NB1413M3_OTONANO,
	NB1413M3_ABUNAI,
	NB1413M3_MJSIKAKU,
	NB1413M3_MMSIKAKU,
	NB1413M3_MSJIKEN,
	NB1413M3_HANAMOMO,
	NB1413M3_TELMAHJN,
	NB1413M3_GIONBANA,
	NB1413M3_MGION,
	NB1413M3_OMOTESND,
	NB1413M3_SCANDAL,
	NB1413M3_SCANDALM,
	NB1413M3_MGMEN89,
	NB1413M3_OHPAIPEE,
	NB1413M3_TOGENKYO,
	NB1413M3_MJFOCUS,
	NB1413M3_MJFOCUSM,
	NB1413M3_GALKOKU,
	NB1413M3_MJNANPAS,
	NB1413M3_BANANADR,
	NB1413M3_GALKAIKA,
	NB1413M3_MCONTEST,
	NB1413M3_UCHUUAI,
	NB1413M3_TOKIMBSJ,
	NB1413M3_TOKYOGAL,
	NB1413M3_TRIPLEW1,
	NB1413M3_NTOPSTAR,
	NB1413M3_MLADYHTR,
	NB1413M3_PSTADIUM,
	NB1413M3_TRIPLEW2,
	NB1413M3_CLUB90S,
	NB1413M3_LOVEHOUS,
	NB1413M3_CHINMOKU,
	NB1413M3_VANILLA,
	NB1413M3_MJLSTORY,
	NB1413M3_QMHAYAKU,
	NB1413M3_MJGOTTUB,
	NB1413M3_MAIKO,
	NB1413M3_MMAIKO,
	NB1413M3_HANAOJI,
	NB1413M3_KAGUYA,
	NB1413M3_KAGUYA2,
	NB1413M3_APPAREL,
	NB1413M3_AV2MJ1BB,
	NB1413M3_AV2MJ2RG,
	NB1413M3_FINALBNY,
	NB1413M3_HYOUBAN,
	NB1413M3_ORANGEC,
	NB1413M3_ORANGECI,
	NB1413M3_VIPCLUB,
	NB1413M3_IDHIMITU,
	NB1413M3_KANATUEN,
	NB1413M3_KYUHITO,
	NB1413M3_PAIRSNB,
	NB1413M3_PAIRSTEN
};

#define MCFG_NB1413M3_TYPE(_type) \
	nb1413m3_device::set_type(*device, _type);

class nb1413m3_device : public device_t
{
public:
	nb1413m3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~nb1413m3_device() {}

	// (static) configuration helpers
	static void set_type(device_t &device, int type) { downcast<nb1413m3_device &>(device).m_nb1413m3_type = type; }

	enum
	{
		TIMER_CB
	};

	DECLARE_WRITE8_MEMBER( nmi_clock_w );
	DECLARE_READ8_MEMBER( sndrom_r );
	DECLARE_WRITE8_MEMBER( sndrombank1_w );
	DECLARE_WRITE8_MEMBER( sndrombank2_w );
	DECLARE_READ8_MEMBER( gfxrom_r );
	DECLARE_WRITE8_MEMBER( gfxrombank_w );
	DECLARE_WRITE8_MEMBER( gfxradr_l_w );
	DECLARE_WRITE8_MEMBER( gfxradr_h_w );
	DECLARE_WRITE8_MEMBER( inputportsel_w );
	DECLARE_READ8_MEMBER( inputport0_r );
	DECLARE_READ8_MEMBER( inputport1_r );
	DECLARE_READ8_MEMBER( inputport2_r );
	DECLARE_READ8_MEMBER( inputport3_r );
	DECLARE_READ8_MEMBER( dipsw1_r );
	DECLARE_READ8_MEMBER( dipsw2_r );
	DECLARE_READ8_MEMBER( dipsw3_l_r );
	DECLARE_READ8_MEMBER( dipsw3_h_r );
	DECLARE_WRITE8_MEMBER( outcoin_w );
	DECLARE_WRITE8_MEMBER( vcrctrl_w );

	const char * m_sndromrgntag;
	int m_sndrombank1;
	int m_sndrombank2;
	int m_busyctr;
	int m_busyflag;
	int m_outcoin_flag;
	int m_inputport;
	int m_nb1413m3_type;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:

	int m_74ls193_counter;
	int m_nmi_count;          // for debug
	int m_nmi_clock;
	int m_nmi_enable;
	int m_counter;
	int m_gfxradr_l;
	int m_gfxradr_h;
	int m_gfxrombank;
	int m_outcoin_enable;
	emu_timer *m_timer_cb;
	TIMER_CALLBACK_MEMBER( timer_callback );
};

/* used in: hyhoo.c, niyanpai.c, pastelg.c, nbmj8688.c, nbmj8891.c, nbmj8991.c, nbmj9195.c*/
INPUT_PORTS_EXTERN( nbmjcontrols );
INPUT_PORTS_EXTERN( nbhf1_ctrl );
INPUT_PORTS_EXTERN( nbhf2_ctrl );

extern const device_type NB1413M3;


#define MCFG_NB1413M3_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NB1413M3, 0)

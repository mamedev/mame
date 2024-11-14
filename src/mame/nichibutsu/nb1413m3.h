// license:BSD-3-Clause
// copyright-holders:Takahiro Nogi
/******************************************************************************

    Machine Hardware for Nichibutsu Mahjong series.

    Driver by Takahiro Nogi 1999/11/05 -

******************************************************************************/
#ifndef MAME_NICHIBUTSU_NB1413M3_H
#define MAME_NICHIBUTSU_NB1413M3_H

#pragma once

/*
#define NB1413M3_VCR_NOP        0x00
#define NB1413M3_VCR_POWER      0x01
#define NB1413M3_VCR_STOP       0x02
#define NB1413M3_VCR_REWIND     0x04
#define NB1413M3_VCR_PLAY       0x08
#define NB1413M3_VCR_FFORWARD   0x10
#define NB1413M3_VCR_PAUSE      0x20
*/

class nb1413m3_device : public device_t
{
public:
	nb1413m3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, int type) :
		nb1413m3_device(mconfig, tag, owner, clock)
	{
		set_type(type);
	}

	nb1413m3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~nb1413m3_device() {}

	// configuration helpers
	void set_type(int type) { m_nb1413m3_type = type; }
	template <typename T> void set_blitter_rom_tag(T &&tag) { m_blitter_rom.set_tag(std::forward<T>(tag)); }

	void nmi_clock_w(uint8_t data);
	uint8_t sndrom_r(address_space &space, offs_t offset);
	void sndrombank1_w(uint8_t data);
	void sndrombank1_alt_w(uint8_t data);
	void sndrombank2_w(uint8_t data);
	uint8_t gfxrom_r(offs_t offset);
	void gfxrombank_w(uint8_t data);
	void gfxradr_l_w(uint8_t data);
	void gfxradr_h_w(uint8_t data);
	void inputportsel_w(uint8_t data);
	uint8_t inputport0_r();
	uint8_t inputport1_r();
	uint8_t inputport2_r();
	uint8_t inputport3_r();
	uint8_t dipsw1_r();
	uint8_t dipsw2_r();
	uint8_t dipsw3_l_r();
	uint8_t dipsw3_h_r();
	void outcoin_w(uint8_t data);
	void vcrctrl_w(uint8_t data);
	int busyflag_r();
	void busyflag_w(int state);

	required_device<cpu_device> m_maincpu;
	const char * m_sndromrgntag;
	int m_sndrombank1;
	int m_sndrombank2;
	int m_busyctr;
	int m_outcoin_flag;
	int m_inputport;
	int m_nb1413m3_type = 0;

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

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_region_ptr<uint8_t> m_blitter_rom;
	output_finder<> m_led;

	int m_busyflag;
	int m_74ls193_counter;
	int m_nmi_count;          // for debug
	int m_nmi_clock;
	int m_nmi_enable;
	int m_counter;
	int m_gfxradr_l;
	int m_gfxradr_h;
	int m_gfxrombank;
	int m_outcoin_enable;
	emu_timer *m_timer_cb = nullptr;
	TIMER_CALLBACK_MEMBER( timer_callback );
};

/* used in: hyhoo.cpp, niyanpai.cpp, pastelg.cpp, nbmj8688.cpp, nbmj8891.cpp, nbmj8991.cpp, nbmj9195.cpp */
INPUT_PORTS_EXTERN( nbmjcontrols );
INPUT_PORTS_EXTERN( nbhf1_ctrl );
INPUT_PORTS_EXTERN( nbhf2_ctrl );

DECLARE_DEVICE_TYPE(NB1413M3, nb1413m3_device)

#endif // MAME_NICHIBUTSU_NB1413M3_H

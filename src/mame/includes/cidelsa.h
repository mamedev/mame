#ifndef __CIDELSA__
#define __CIDELSA__

#include "cpu/cosmac/cosmac.h"

#define SCREEN_TAG	"screen"
#define CDP1802_TAG	"cdp1802"
#define CDP1869_TAG	"cdp1869"
#define COP402N_TAG	"cop402n"
#define AY8910_TAG	"ay8910"

#define DESTRYER_CHR1	3579000.0 // unverified
#define DESTRYER_CHR2	XTAL_5_7143MHz
#define ALTAIR_CHR1		3579000.0 // unverified
#define ALTAIR_CHR2		CDP1869_DOT_CLK_PAL // unverified
#define DRACO_CHR1		XTAL_4_43361MHz
#define DRACO_CHR2		CDP1869_DOT_CLK_PAL // unverified
#define DRACO_SND_CHR1	XTAL_2_01216MHz

#define CIDELSA_PAGERAM_SIZE	0x400
#define DRACO_PAGERAM_SIZE		0x800
#define CIDELSA_CHARRAM_SIZE	0x800

#define CIDELSA_PAGERAM_MASK	0x3ff
#define DRACO_PAGERAM_MASK		0x7ff
#define CIDELSA_CHARRAM_MASK	0x7ff

class cidelsa_state : public driver_device
{
public:
	cidelsa_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_maincpu(*this, CDP1802_TAG),
		  m_vis(*this, CDP1869_TAG),
		  m_psg(*this, AY8910_TAG)
	{ }

	required_device<cosmac_device> m_maincpu;
	required_device<cdp1869_device> m_vis;
	optional_device<running_device> m_psg;

	virtual void machine_reset();

	virtual void video_start();
	bool video_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect) { m_vis->update_screen(&bitmap, &cliprect); return false; }

	DECLARE_READ8_MEMBER( draco_sound_in_r );
	DECLARE_READ8_MEMBER( draco_sound_ay8910_r );

	DECLARE_WRITE8_MEMBER( cdp1869_w );
	DECLARE_WRITE8_MEMBER( draco_sound_bankswitch_w );
	DECLARE_WRITE8_MEMBER( draco_sound_g_w );
	DECLARE_WRITE8_MEMBER( draco_sound_ay8910_w );
	DECLARE_WRITE8_MEMBER( destryer_out1_w );
	DECLARE_WRITE8_MEMBER( altair_out1_w );
	DECLARE_WRITE8_MEMBER( draco_out1_w );
	DECLARE_WRITE8_MEMBER( draco_ay8910_port_b_w );

	DECLARE_READ_LINE_MEMBER( clear_r );

	DECLARE_WRITE_LINE_MEMBER( q_w );
	DECLARE_WRITE_LINE_MEMBER( prd_w );

	// cpu state
	int m_reset;

	// video state
	int m_cdp1802_q;
	int m_cdp1869_pcb;

	UINT8 *m_pageram;
	UINT8 *m_pcbram;
	UINT8 *m_charram;

	// sound state
	int m_draco_sound;
	int m_draco_ay_latch;
};

/*----------- defined in video/cidelsa.c -----------*/

MACHINE_CONFIG_EXTERN( destryer_video );
MACHINE_CONFIG_EXTERN( altair_video );
MACHINE_CONFIG_EXTERN( draco_video );

#endif

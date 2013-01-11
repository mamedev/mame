#ifndef __PECOM__
#define __PECOM__

#include "cpu/cosmac/cosmac.h"
#include "imagedev/cassette.h"

#define SCREEN_TAG  "screen"
#define CDP1802_TAG "cdp1802"
#define CDP1869_TAG "cdp1869"

#define PECOM_CHAR_RAM_SIZE 0x800

class pecom_state : public driver_device
{
public:
	pecom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_cdp1802(*this, CDP1802_TAG),
			m_cdp1869(*this, CDP1869_TAG)
	{ }

	required_device<cosmac_device> m_cdp1802;
	required_device<cdp1869_device> m_cdp1869;

	UINT8 *m_charram;           /* character generator ROM */
	int m_reset;                /* CPU mode */
	int m_dma;              /* memory refresh DMA */

	/* timers */
	emu_timer *m_reset_timer;   /* power on reset timer */

	DECLARE_READ8_MEMBER(pecom_cdp1869_charram_r);
	DECLARE_WRITE8_MEMBER(pecom_cdp1869_charram_w);
	DECLARE_READ8_MEMBER(pecom_cdp1869_pageram_r);
	DECLARE_WRITE8_MEMBER(pecom_cdp1869_pageram_w);
	DECLARE_WRITE8_MEMBER(pecom_bank_w);
	DECLARE_READ8_MEMBER(pecom_keyboard_r);
	DECLARE_WRITE8_MEMBER(pecom_cdp1869_w);
	virtual void machine_start();
	virtual void machine_reset();
	DECLARE_VIDEO_START(pecom);
	DECLARE_INPUT_CHANGED_MEMBER(ef_w);
	TIMER_CALLBACK_MEMBER(reset_tick);
	DECLARE_READ_LINE_MEMBER(clear_r);
	DECLARE_READ_LINE_MEMBER(ef2_r);
	DECLARE_WRITE_LINE_MEMBER(pecom64_q_w);
	DECLARE_WRITE_LINE_MEMBER(pecom_prd_w);
};

/*----------- defined in machine/pecom.c -----------*/
extern const cosmac_interface pecom64_cdp1802_config;

/* ---------- defined in video/pecom.c ---------- */

MACHINE_CONFIG_EXTERN( pecom_video );

#endif

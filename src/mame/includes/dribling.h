// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Model Racing Dribbling hardware

*************************************************************************/

#include "machine/i8255.h"


class dribling_state : public driver_device
{
public:
	dribling_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi8255_0(*this, "ppi8255_0"),
		m_ppi8255_1(*this, "ppi8255_1"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<i8255_device>  m_ppi8255_0;
	optional_device<i8255_device>  m_ppi8255_1;
	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;

	/* misc */
	UINT8    m_abca;
	UINT8    m_dr;
	UINT8    m_ds;
	UINT8    m_sh;
	UINT8    m_input_mux;
	UINT8    m_di;

	DECLARE_READ8_MEMBER(ioread);
	DECLARE_WRITE8_MEMBER(iowrite);
	DECLARE_WRITE8_MEMBER(dribling_colorram_w);
	DECLARE_READ8_MEMBER(dsr_r);
	DECLARE_READ8_MEMBER(input_mux0_r);
	DECLARE_WRITE8_MEMBER(misc_w);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(pb_w);
	DECLARE_WRITE8_MEMBER(shr_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_PALETTE_INIT(dribling);
	UINT32 screen_update_dribling(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(dribling_irq_gen);
};

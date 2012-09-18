/*****************************************************************************
 *
 * includes/ondra.h
 *
 ****************************************************************************/

#ifndef ONDRA_H_
#define ONDRA_H_

class ondra_state : public driver_device
{
public:
	ondra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_video_enable;
	UINT8 m_bank1_status;
	UINT8 m_bank2_status;
	DECLARE_READ8_MEMBER(ondra_keyboard_r);
	DECLARE_WRITE8_MEMBER(ondra_port_03_w);
	DECLARE_WRITE8_MEMBER(ondra_port_09_w);
	DECLARE_WRITE8_MEMBER(ondra_port_0a_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_ondra(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ondra_interrupt);
};

#endif

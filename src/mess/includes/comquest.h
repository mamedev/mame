// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 * includes/comquest.h
 *
 ****************************************************************************/

#ifndef COMQUEST_H_
#define COMQUEST_H_


class comquest_state : public driver_device
{
public:
	comquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	UINT8 m_data[128][8];
	void *m_timer;
	int m_line;
	int m_dma_activ;
	int m_state;
	int m_count;
	DECLARE_READ8_MEMBER(comquest_read);
	DECLARE_WRITE8_MEMBER(comquest_write);
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_comquest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};

#endif /* COMQUEST_H_ */

/*****************************************************************************
 *
 * includes/banctec.h
 *
 ****************************************************************************/

#ifndef BANCTEC_H_
#define BANCTEC_H_


class banctec_state : public driver_device
{
public:
	banctec_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	DECLARE_READ8_MEMBER(banctec_read);
	DECLARE_WRITE8_MEMBER(banctec_write);
	virtual void machine_reset();
//	virtual void video_start();
//	UINT32 screen_update_banctec(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
};

#endif /* BANCTEC_H_ */

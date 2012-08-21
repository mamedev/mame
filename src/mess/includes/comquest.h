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
		: driver_device(mconfig, type, tag) { }

	UINT8 m_data[128][8];
	void *m_timer;
	int m_line;
	int m_dma_activ;
	int m_state;
	int m_count;
	DECLARE_READ8_MEMBER(comquest_read);
	DECLARE_WRITE8_MEMBER(comquest_write);
};


/*----------- defined in video/comquest.c -----------*/

VIDEO_START( comquest );
SCREEN_UPDATE_IND16( comquest );


#endif /* COMQUEST_H_ */

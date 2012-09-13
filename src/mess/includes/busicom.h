/*****************************************************************************
 *
 * includes/busicom.h
 *
 ****************************************************************************/

#ifndef BUSICOM_H_
#define BUSICOM_H_


class busicom_state : public driver_device
{
public:
	busicom_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_drum_index;
	UINT16 m_keyboard_shifter;
	UINT32 m_printer_shifter;
	UINT8 m_timer;
	UINT8 m_printer_line[11][17];
	UINT8 m_printer_line_color[11];
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_READ8_MEMBER(printer_r);
	DECLARE_WRITE8_MEMBER(shifter_w);
	DECLARE_WRITE8_MEMBER(printer_w);
	DECLARE_WRITE8_MEMBER(status_w);
	DECLARE_WRITE8_MEMBER(printer_ctrl_w);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in video/busicom.c -----------*/

extern PALETTE_INIT( busicom );
extern VIDEO_START( busicom );
extern SCREEN_UPDATE_IND16( busicom );

#endif /* BUSICOM_H_ */

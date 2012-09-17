/*****************************************************************************
 *
 * includes/ssystem3.h
 *
 ****************************************************************************/

#ifndef SSYSTEM3_H_
#define SSYSTEM3_H_


struct playfield_t
{
	int signal;
	//  int on;

	int count, bit, started;
	UINT8 data;
	attotime time, high_time, low_time;
	union {
		struct {
			UINT8 header[7];
			UINT8 field[8][8/2];
			UINT8 unknown[5];
		} s;
		UINT8 data[7+8*8/2+5];
	} u;
};

struct lcd_t
{
	UINT8 data[5];
	int clock;
	int count;
};


class ssystem3_state : public driver_device
{
public:
	ssystem3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_porta;
	UINT8 *m_videoram;
	playfield_t m_playfield;
	lcd_t m_lcd;
	DECLARE_DRIVER_INIT(ssystem3);
	virtual void video_start();
	virtual void palette_init();
};


/*----------- defined in drivers/ssystem3.c -----------*/

void ssystem3_playfield_getfigure(running_machine &machine, int x, int y, int *figure, int *black);

/*----------- defined in video/ssystem3.c -----------*/

extern PALETTE_INIT( ssystem3 );
extern VIDEO_START( ssystem3 );
extern SCREEN_UPDATE_IND16( ssystem3 );

void ssystem3_lcd_reset(running_machine &machine);
void ssystem3_lcd_write(running_machine &machine, int clock, int data);

#endif /* SSYSTEM3_H_ */

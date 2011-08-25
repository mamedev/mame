/*************************************************************************


    Taito Z system

*************************************************************************/

#include "machine/eeprom.h"

class taitoz_state : public driver_device
{
public:
	taitoz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	offs_t      m_spriteram_size;

	/* video-related */
	int         m_sci_spriteframe;
	int         m_road_palbank;

	/* misc */
	int         m_chasehq_lamps;
	INT32       m_banknum;
	UINT16      m_cpua_ctrl;
	INT32       m_sci_int6;
	INT32       m_dblaxle_int6;
	INT32       m_ioc220_port;
	UINT16      m_eep_latch;

//  UINT8       pandata[4];

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_subcpu;
	eeprom_device *m_eeprom;
	device_t *m_tc0480scp;
	device_t *m_tc0150rod;
	device_t *m_tc0100scn;
	device_t *m_tc0220ioc;
	device_t *m_tc0140syt;

	/* dblaxle motor flag */
	int	    m_dblaxle_vibration;
};

/*----------- defined in video/taito_z.c -----------*/

WRITE16_HANDLER( contcirc_out_w );
READ16_HANDLER ( sci_spriteframe_r );
WRITE16_HANDLER( sci_spriteframe_w );

VIDEO_START( taitoz );

SCREEN_UPDATE( contcirc );
SCREEN_UPDATE( chasehq );
SCREEN_UPDATE( bshark );
SCREEN_UPDATE( sci );
SCREEN_UPDATE( aquajack );
SCREEN_UPDATE( spacegun );
SCREEN_UPDATE( dblaxle );

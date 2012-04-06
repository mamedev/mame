/***************************************************************************

    IREM M-10,M-11 and M-15 based hardware

****************************************************************************/

#include "sound/samples.h"

#define IREMM10_MASTER_CLOCK		(12500000)

#define IREMM10_CPU_CLOCK		(IREMM10_MASTER_CLOCK/16)
#define IREMM10_PIXEL_CLOCK		(IREMM10_MASTER_CLOCK/2)
#define IREMM10_HTOTAL			(360)	/* (0x100-0xd3)*8 */
#define IREMM10_HBSTART			(248)
#define IREMM10_HBEND			(8)
#define IREMM10_VTOTAL			(281)	/* (0x200-0xe7) */
#define IREMM10_VBSTART			(240)
#define IREMM10_VBEND			(16)

#define IREMM15_MASTER_CLOCK	(11730000)

#define IREMM15_CPU_CLOCK		(IREMM15_MASTER_CLOCK/10)
#define IREMM15_PIXEL_CLOCK		(IREMM15_MASTER_CLOCK/2)
#define IREMM15_HTOTAL			(372)
#define IREMM15_HBSTART			(256)
#define IREMM15_HBEND			(0)
#define IREMM15_VTOTAL			(262)
#define IREMM15_VBSTART			(240)
#define IREMM15_VBEND			(16)

class m10_state : public driver_device
{
public:
	m10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *             m_chargen;
	UINT8 *             m_memory;
	UINT8 *             m_rom;
	UINT8 *             m_videoram;
	UINT8 *             m_colorram;
	size_t              m_videoram_size;

	/* video-related */
	tilemap_t *           m_tx_tilemap;
	gfx_element *       m_back_gfx;

	/* this is currently unused, because it is needed by gfx_layout (which has no machine) */
	UINT32              extyoffs[32 * 8];

	/* video state */
	UINT8	              m_bottomline;
	UINT8               m_flip;

	/* misc */
	int                 m_last;

	/* devices */
	device_t *m_maincpu;
	device_t *m_ic8j1;
	device_t *m_ic8j2;
	samples_device *m_samples;
	DECLARE_WRITE8_MEMBER(m10_ctrl_w);
	DECLARE_WRITE8_MEMBER(m11_ctrl_w);
	DECLARE_WRITE8_MEMBER(m15_ctrl_w);
	DECLARE_WRITE8_MEMBER(m10_a500_w);
	DECLARE_WRITE8_MEMBER(m11_a100_w);
	DECLARE_WRITE8_MEMBER(m15_a100_w);
	DECLARE_READ8_MEMBER(m10_a700_r);
	DECLARE_READ8_MEMBER(m11_a700_r);
	DECLARE_WRITE8_MEMBER(m10_colorram_w);
	DECLARE_WRITE8_MEMBER(m10_chargen_w);
	DECLARE_WRITE8_MEMBER(m15_chargen_w);
};


/*----------- defined in video/m10.c -----------*/



SCREEN_UPDATE_IND16( m10 );
SCREEN_UPDATE_IND16( m15 );

VIDEO_START( m10 );
VIDEO_START( m15 );

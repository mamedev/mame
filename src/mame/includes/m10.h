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
		: driver_device(mconfig, type, tag) ,
		m_memory(*this, "memory"),
		m_rom(*this, "rom"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_chargen(*this, "chargen"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_memory;
	required_shared_ptr<UINT8> m_rom;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_chargen;

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
	cpu_device *m_maincpu;
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
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_DRIVER_INIT(andromed);
	DECLARE_DRIVER_INIT(ipminva1);
	TILEMAP_MAPPER_MEMBER(tilemap_scan);
	TILE_GET_INFO_MEMBER(get_tile_info);
	DECLARE_MACHINE_START(m10);
	DECLARE_MACHINE_RESET(m10);
	DECLARE_VIDEO_START(m10);
	DECLARE_PALETTE_INIT(m10);
	DECLARE_VIDEO_START(m15);
	UINT32 screen_update_m10(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_m15(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(m11_interrupt);
	INTERRUPT_GEN_MEMBER(m10_interrupt);
	INTERRUPT_GEN_MEMBER(m15_interrupt);
	TIMER_CALLBACK_MEMBER(interrupt_callback);
};


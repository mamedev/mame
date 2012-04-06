/*************************************************************************

    TX-1/Buggy Boy hardware

*************************************************************************/

#include "devlegcy.h"

#define TX1_PIXEL_CLOCK		(XTAL_18MHz / 3)
#define TX1_HBSTART			256
#define TX1_HBEND			0
#define TX1_HTOTAL			384
#define TX1_VBSTART			240
#define TX1_VBEND			0
#define TX1_VTOTAL			264

/*
 * HACK! Increased VTOTAL to 'fix' a timing issue
 * that prevents one of the start countdown tones
 * from playing.
 */
#define BB_PIXEL_CLOCK		(XTAL_18MHz / 3)
#define BB_HBSTART			256
#define BB_HBEND			0
#define BB_HTOTAL			384
#define BB_VBSTART			240
#define BB_VBEND			0
#define BB_VTOTAL			288 + 1

#define CPU_MASTER_CLOCK	(XTAL_15MHz)
#define BUGGYBOY_ZCLK		(CPU_MASTER_CLOCK / 2)

struct math_t
{
	UINT16	cpulatch;
	UINT16	promaddr;
	UINT16	inslatch;
	UINT32	mux;
	UINT16	ppshift;
	UINT32	i0ff;

	UINT16	retval;

	UINT16  muxlatch;	// TX-1

	int dbgaddr;
	int dbgpc;
};

/*
    SN74S516 16x16 Multiplier/Divider
*/
struct sn74s516_t
{
	INT16	X;
	INT16	Y;

	union
	{
	#ifdef LSB_FIRST
		struct { UINT16 W; INT16 Z; };
	#else
		struct { INT16 Z; UINT16 W; };
	#endif
		INT32 ZW32;
	} ZW;

	int		code;
	int		state;
	int		ZWfl;
};

struct vregs_t
{
	UINT16	scol;		/* Road colours */
	UINT32  slock;		/* Scroll lock */
	UINT8	flags;		/* Road flags */

	UINT32	ba_val;		/* Accumulator */
	UINT32	ba_inc;
	UINT32	bank_mode;

	UINT16	h_val;		/* Accumulator */
	UINT16	h_inc;
	UINT16	h_init;

	UINT8	slin_val;	/* Accumulator */
	UINT8	slin_inc;

	/* Buggyboy only */
	UINT8	wa8;
	UINT8	wa4;

	UINT16	wave_lfsr;
	UINT8	sky;
	UINT16	gas;
	UINT8	shift;
};


class tx1_state : public driver_device
{
public:
	tx1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	math_t m_math;
	sn74s516_t m_sn74s516;
	UINT8 *m_z80_ram;
	UINT8 m_ppi_latch_a;
	UINT8 m_ppi_latch_b;
	UINT32 m_ts;
	UINT16 *m_math_ram;
	vregs_t m_vregs;
	UINT16 *m_vram;
	UINT16 *m_objram;
	UINT16 *m_rcram;
	emu_timer *m_interrupt_timer;
	UINT8 *m_chr_bmp;
	UINT8 *m_obj_bmp;
	UINT8 *m_rod_bmp;
	bitmap_ind16 *m_bitmap;
	DECLARE_READ16_MEMBER(tx1_math_r);
	DECLARE_WRITE16_MEMBER(tx1_math_w);
	DECLARE_READ16_MEMBER(tx1_spcs_rom_r);
	DECLARE_READ16_MEMBER(tx1_spcs_ram_r);
	DECLARE_WRITE16_MEMBER(tx1_spcs_ram_w);
	DECLARE_READ16_MEMBER(buggyboy_math_r);
	DECLARE_WRITE16_MEMBER(buggyboy_math_w);
	DECLARE_READ16_MEMBER(buggyboy_spcs_rom_r);
	DECLARE_WRITE16_MEMBER(buggyboy_spcs_ram_w);
	DECLARE_READ16_MEMBER(buggyboy_spcs_ram_r);
	DECLARE_READ16_MEMBER(tx1_crtc_r);
	DECLARE_WRITE16_MEMBER(tx1_crtc_w);
	DECLARE_WRITE16_MEMBER(tx1_bankcs_w);
	DECLARE_WRITE16_MEMBER(tx1_slincs_w);
	DECLARE_WRITE16_MEMBER(tx1_slock_w);
	DECLARE_WRITE16_MEMBER(tx1_scolst_w);
	DECLARE_WRITE16_MEMBER(tx1_flgcs_w);
	DECLARE_WRITE16_MEMBER(buggyboy_gas_w);
	DECLARE_WRITE16_MEMBER(buggyboy_sky_w);
	DECLARE_WRITE16_MEMBER(buggyboy_scolst_w);
};


/*----------- defined in machine/tx1.c -----------*/
MACHINE_RESET( tx1 );


MACHINE_RESET( buggyboy );

/*----------- defined in audio/tx1.c -----------*/
READ8_DEVICE_HANDLER( tx1_pit8253_r );
WRITE8_DEVICE_HANDLER( tx1_pit8253_w );

WRITE8_DEVICE_HANDLER( bb_ym1_a_w );
WRITE8_DEVICE_HANDLER( bb_ym2_a_w );
WRITE8_DEVICE_HANDLER( bb_ym2_b_w );

DECLARE_LEGACY_SOUND_DEVICE(BUGGYBOY, buggyboy_sound);

WRITE8_DEVICE_HANDLER( tx1_ay8910_a_w );
WRITE8_DEVICE_HANDLER( tx1_ay8910_b_w );

DECLARE_LEGACY_SOUND_DEVICE(TX1, tx1_sound);


/*----------- defined in video/tx1.c -----------*/

PALETTE_INIT( tx1 );
VIDEO_START( tx1 );
SCREEN_UPDATE_IND16( tx1_left );
SCREEN_UPDATE_IND16( tx1_middle );
SCREEN_UPDATE_IND16( tx1_right );
SCREEN_VBLANK( tx1 );

PALETTE_INIT( buggyboy );
VIDEO_START( buggyboy );
SCREEN_UPDATE_IND16( buggyboy_left );
SCREEN_UPDATE_IND16( buggyboy_middle );
SCREEN_UPDATE_IND16( buggyboy_right );
SCREEN_VBLANK( buggyboy );

VIDEO_START( buggybjr );
SCREEN_UPDATE_IND16( buggybjr );

#include "devlegcy.h"

#ifdef MAME_DEBUG
#define LOGLEVEL  5
#else
#define LOGLEVEL  0
#endif
#define LOG(n,x)  do { if (LOGLEVEL >= n) logerror x; } while (0)


DECLARE_LEGACY_DEVICE(DECOCASS_TAPE, decocass_tape);

#define MCFG_DECOCASS_TAPE_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DECOCASS_TAPE, 0)




class decocass_state : public driver_device
{
public:
	decocass_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_rambase(*this, "rambase"),
		  m_charram(*this, "charram"),
		  m_fgvideoram(*this, "fgvideoram"),
		  m_colorram(*this, "colorram"),
		  m_tileram(*this, "tileram"),
		  m_objectram(*this, "objectram"),
		  m_paletteram(*this, "paletteram") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_rambase;
	required_shared_ptr<UINT8> m_charram;
	required_shared_ptr<UINT8> m_fgvideoram;
	required_shared_ptr<UINT8> m_colorram;
	UINT8 *   m_bgvideoram;	/* shares bits D0-3 with tileram! */
	required_shared_ptr<UINT8> m_tileram;
	required_shared_ptr<UINT8> m_objectram;
	required_shared_ptr<UINT8> m_paletteram;
	size_t    m_bgvideoram_size;

	/* video-related */
	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_bg_tilemap_l;
	tilemap_t   *m_bg_tilemap_r;
	INT32     m_watchdog_count;
	INT32     m_watchdog_flip;
	INT32     m_color_missiles;
	INT32     m_color_center_bot;
	INT32     m_mode_set;
	INT32     m_back_h_shift;
	INT32     m_back_vl_shift;
	INT32     m_back_vr_shift;
	INT32     m_part_h_shift;
	INT32     m_part_v_shift;
	INT32     m_center_h_shift_space;
	INT32     m_center_v_shift;
	rectangle m_bg_tilemap_l_clip;
	rectangle m_bg_tilemap_r_clip;

	/* sound-related */
	UINT8     m_sound_ack;	/* sound latches, ACK status bits and NMI timer */
	UINT8     m_audio_nmi_enabled;
	UINT8     m_audio_nmi_state;

	/* misc */
	UINT8     *m_decrypted;
	UINT8     *m_decrypted2;
	INT32     m_firsttime;
	UINT8     m_latch1;
	UINT8     m_decocass_reset;
	INT32     m_de0091_enable;	/* DE-0091xx daughter board enable */
	UINT8     m_quadrature_decoder[4];	/* four inputs from the quadrature decoder (H1, V1, H2, V2) */
	int       m_showmsg;		// for debugging purposes

	/* i8041 */
	UINT8     m_i8041_p1;
	UINT8     m_i8041_p2;
	int       m_i8041_p1_write_latch;
	int       m_i8041_p1_read_latch;
	int       m_i8041_p2_write_latch;
	int       m_i8041_p2_read_latch;

	/* dongles-related */
	read8_space_func    m_dongle_r;
	write8_space_func   m_dongle_w;

	/* dongle type #1 */
	UINT32    m_type1_inmap;
	UINT32    m_type1_outmap;

	/* dongle type #2: status of the latches */
	INT32     m_type2_d2_latch;	/* latched 8041-STATUS D2 value */
	INT32     m_type2_xx_latch;	/* latched value (D7-4 == 0xc0) ? 1 : 0 */
	INT32     m_type2_promaddr;	/* latched PROM address A0-A7 */

	/* dongle type #3: status and patches */
	INT32     m_type3_ctrs;		/* 12 bit counter stage */
	INT32     m_type3_d0_latch;	/* latched 8041-D0 value */
	INT32     m_type3_pal_19;		/* latched 1 for PAL input pin-19 */
	INT32     m_type3_swap;

	/* dongle type #4: status */
	INT32     m_type4_ctrs;		/* latched PROM address (E5x0 LSB, E5x1 MSB) */
	INT32     m_type4_latch;		/* latched enable PROM (1100xxxx written to E5x1) */

	/* dongle type #5: status */
	INT32     m_type5_latch;		/* latched enable PROM (1100xxxx written to E5x1) */

	/* DS Telejan */
	UINT8     m_mux_data;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_mcu;
	device_t *m_cassette;
};



WRITE8_HANDLER( decocass_coin_counter_w );
WRITE8_HANDLER( decocass_sound_command_w );
READ8_HANDLER( decocass_sound_data_r );
READ8_HANDLER( decocass_sound_ack_r );
WRITE8_HANDLER( decocass_sound_data_w );
READ8_HANDLER( decocass_sound_command_r );
TIMER_DEVICE_CALLBACK( decocass_audio_nmi_gen );
WRITE8_HANDLER( decocass_sound_nmi_enable_w );
READ8_HANDLER( decocass_sound_nmi_enable_r );
READ8_HANDLER( decocass_sound_data_ack_reset_r );
WRITE8_HANDLER( decocass_sound_data_ack_reset_w );
WRITE8_HANDLER( decocass_nmi_reset_w );
WRITE8_HANDLER( decocass_quadrature_decoder_reset_w );
WRITE8_HANDLER( decocass_adc_w );
READ8_HANDLER( decocass_input_r );

WRITE8_HANDLER( decocass_reset_w );

READ8_HANDLER( decocass_e5xx_r );
WRITE8_HANDLER( decocass_e5xx_w );
WRITE8_HANDLER( decocass_de0091_w );
WRITE8_HANDLER( decocass_e900_w );

MACHINE_START( decocass );
MACHINE_RESET( decocass );
MACHINE_RESET( ctsttape );
MACHINE_RESET( chwy );
MACHINE_RESET( cdsteljn );
MACHINE_RESET( cfishing );
MACHINE_RESET( cterrani );
MACHINE_RESET( castfant );
MACHINE_RESET( csuperas );
MACHINE_RESET( clocknch );
MACHINE_RESET( cprogolf );
MACHINE_RESET( cprogolfj );
MACHINE_RESET( cluckypo );
MACHINE_RESET( ctisland );
MACHINE_RESET( cexplore );
MACHINE_RESET( cdiscon1 );
MACHINE_RESET( ctornado );
MACHINE_RESET( cmissnx );
MACHINE_RESET( cptennis );
MACHINE_RESET( cbtime );
MACHINE_RESET( cburnrub );
MACHINE_RESET( cgraplop );
MACHINE_RESET( cgraplop2 );
MACHINE_RESET( cskater );
MACHINE_RESET( clapapa );
MACHINE_RESET( cprobowl );
MACHINE_RESET( cnightst );
MACHINE_RESET( cpsoccer );
MACHINE_RESET( csdtenis );
MACHINE_RESET( czeroize );
MACHINE_RESET( cppicf );
MACHINE_RESET( cfghtice );
MACHINE_RESET( type4 );
MACHINE_RESET( cflyball );
MACHINE_RESET( cbdash );

WRITE8_HANDLER( i8041_p1_w );
READ8_HANDLER( i8041_p1_r );
WRITE8_HANDLER( i8041_p2_w );
READ8_HANDLER( i8041_p2_r );

void decocass_machine_state_save_init(running_machine &machine);


/*----------- defined in video/decocass.c -----------*/

WRITE8_HANDLER( decocass_paletteram_w );
WRITE8_HANDLER( decocass_charram_w );
WRITE8_HANDLER( decocass_fgvideoram_w );
WRITE8_HANDLER( decocass_colorram_w );
WRITE8_HANDLER( decocass_bgvideoram_w );
WRITE8_HANDLER( decocass_tileram_w );
WRITE8_HANDLER( decocass_objectram_w );

WRITE8_HANDLER( decocass_watchdog_count_w );
WRITE8_HANDLER( decocass_watchdog_flip_w );
WRITE8_HANDLER( decocass_color_missiles_w );
WRITE8_HANDLER( decocass_mode_set_w );
WRITE8_HANDLER( decocass_color_center_bot_w );
WRITE8_HANDLER( decocass_back_h_shift_w );
WRITE8_HANDLER( decocass_back_vl_shift_w );
WRITE8_HANDLER( decocass_back_vr_shift_w );
WRITE8_HANDLER( decocass_part_h_shift_w );
WRITE8_HANDLER( decocass_part_v_shift_w );
WRITE8_HANDLER( decocass_center_h_shift_space_w );
WRITE8_HANDLER( decocass_center_v_shift_w );

VIDEO_START( decocass );
SCREEN_UPDATE_IND16( decocass );

void decocass_video_state_save_init(running_machine &machine);

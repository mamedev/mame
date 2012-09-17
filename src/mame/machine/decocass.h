#include "devlegcy.h"

#ifdef MAME_DEBUG
#define LOGLEVEL  5
#else
#define LOGLEVEL  0
#endif
#define LOG(n,x)  do { if (LOGLEVEL >= n) logerror x; } while (0)


class decocass_tape_device : public device_t
{
public:
	decocass_tape_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~decocass_tape_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	void *m_token;
};

extern const device_type DECOCASS_TAPE;


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
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_mcu;
	device_t *m_cassette;
	DECLARE_DRIVER_INIT(decocass);
	DECLARE_DRIVER_INIT(decocrom);
	DECLARE_DRIVER_INIT(cdsteljn);
	TILEMAP_MAPPER_MEMBER(fgvideoram_scan_cols);
	TILEMAP_MAPPER_MEMBER(bgvideoram_scan_cols);
	TILE_GET_INFO_MEMBER(get_bg_l_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_r_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_MACHINE_RESET(ctsttape);
	DECLARE_MACHINE_RESET(cprogolfj);
	DECLARE_MACHINE_RESET(cdsteljn);
	DECLARE_MACHINE_RESET(cfishing);
	DECLARE_MACHINE_RESET(chwy);
	DECLARE_MACHINE_RESET(cterrani);
	DECLARE_MACHINE_RESET(castfant);
	DECLARE_MACHINE_RESET(csuperas);
	DECLARE_MACHINE_RESET(clocknch);
	DECLARE_MACHINE_RESET(cprogolf);
	DECLARE_MACHINE_RESET(cluckypo);
	DECLARE_MACHINE_RESET(ctisland);
	DECLARE_MACHINE_RESET(cexplore);
	DECLARE_MACHINE_RESET(cdiscon1);
	DECLARE_MACHINE_RESET(ctornado);
	DECLARE_MACHINE_RESET(cmissnx);
	DECLARE_MACHINE_RESET(cptennis);
	DECLARE_MACHINE_RESET(cbtime);
	DECLARE_MACHINE_RESET(cburnrub);
	DECLARE_MACHINE_RESET(cgraplop);
	DECLARE_MACHINE_RESET(cgraplop2);
	DECLARE_MACHINE_RESET(clapapa);
	DECLARE_MACHINE_RESET(cskater);
	DECLARE_MACHINE_RESET(cprobowl);
	DECLARE_MACHINE_RESET(cnightst);
	DECLARE_MACHINE_RESET(cpsoccer);
	DECLARE_MACHINE_RESET(csdtenis);
	DECLARE_MACHINE_RESET(czeroize);
	DECLARE_MACHINE_RESET(cppicf);
	DECLARE_MACHINE_RESET(cfghtice);
	DECLARE_MACHINE_RESET(type4);
	DECLARE_MACHINE_RESET(cbdash);
	DECLARE_MACHINE_RESET(cflyball);
	UINT32 screen_update_decocass(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};



DECLARE_WRITE8_HANDLER( decocass_coin_counter_w );
DECLARE_WRITE8_HANDLER( decocass_sound_command_w );
DECLARE_READ8_HANDLER( decocass_sound_data_r );
DECLARE_READ8_HANDLER( decocass_sound_ack_r );
DECLARE_WRITE8_HANDLER( decocass_sound_data_w );
DECLARE_READ8_HANDLER( decocass_sound_command_r );
TIMER_DEVICE_CALLBACK( decocass_audio_nmi_gen );
DECLARE_WRITE8_HANDLER( decocass_sound_nmi_enable_w );
DECLARE_READ8_HANDLER( decocass_sound_nmi_enable_r );
DECLARE_READ8_HANDLER( decocass_sound_data_ack_reset_r );
DECLARE_WRITE8_HANDLER( decocass_sound_data_ack_reset_w );
DECLARE_WRITE8_HANDLER( decocass_nmi_reset_w );
DECLARE_WRITE8_HANDLER( decocass_quadrature_decoder_reset_w );
DECLARE_WRITE8_HANDLER( decocass_adc_w );
DECLARE_READ8_HANDLER( decocass_input_r );

DECLARE_WRITE8_HANDLER( decocass_reset_w );

DECLARE_READ8_HANDLER( decocass_e5xx_r );
DECLARE_WRITE8_HANDLER( decocass_e5xx_w );
DECLARE_WRITE8_HANDLER( decocass_de0091_w );
DECLARE_WRITE8_HANDLER( decocass_e900_w );





































DECLARE_WRITE8_HANDLER( i8041_p1_w );
DECLARE_READ8_HANDLER( i8041_p1_r );
DECLARE_WRITE8_HANDLER( i8041_p2_w );
DECLARE_READ8_HANDLER( i8041_p2_r );

void decocass_machine_state_save_init(running_machine &machine);


/*----------- defined in video/decocass.c -----------*/

DECLARE_WRITE8_HANDLER( decocass_paletteram_w );
DECLARE_WRITE8_HANDLER( decocass_charram_w );
DECLARE_WRITE8_HANDLER( decocass_fgvideoram_w );
DECLARE_WRITE8_HANDLER( decocass_colorram_w );
DECLARE_WRITE8_HANDLER( decocass_bgvideoram_w );
DECLARE_WRITE8_HANDLER( decocass_tileram_w );
DECLARE_WRITE8_HANDLER( decocass_objectram_w );

DECLARE_WRITE8_HANDLER( decocass_watchdog_count_w );
DECLARE_WRITE8_HANDLER( decocass_watchdog_flip_w );
DECLARE_WRITE8_HANDLER( decocass_color_missiles_w );
DECLARE_WRITE8_HANDLER( decocass_mode_set_w );
DECLARE_WRITE8_HANDLER( decocass_color_center_bot_w );
DECLARE_WRITE8_HANDLER( decocass_back_h_shift_w );
DECLARE_WRITE8_HANDLER( decocass_back_vl_shift_w );
DECLARE_WRITE8_HANDLER( decocass_back_vr_shift_w );
DECLARE_WRITE8_HANDLER( decocass_part_h_shift_w );
DECLARE_WRITE8_HANDLER( decocass_part_v_shift_w );
DECLARE_WRITE8_HANDLER( decocass_center_h_shift_space_w );
DECLARE_WRITE8_HANDLER( decocass_center_v_shift_w );




void decocass_video_state_save_init(running_machine &machine);

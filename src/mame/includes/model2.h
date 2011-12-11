#include "video/poly.h"


typedef struct _raster_state raster_state;
typedef struct _geo_state geo_state;


class model2_state : public driver_device
{
public:
	model2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT32 *m_workram;
	UINT32 m_intreq;
	UINT32 m_intena;
	UINT32 m_coproctl;
	UINT32 m_coprocnt;
	UINT32 m_geoctl;
	UINT32 m_geocnt;
	UINT16 *m_soundram;
	UINT32 m_timervals[4];
	UINT32 m_timerorig[4];
	int m_timerrun[4];
	timer_device *m_timers[4];
	int m_ctrlmode;
	int m_analog_channel;
	UINT32 *m_tgp_program;
	int m_dsp_type;
	int m_copro_fifoin_rpos;
	int m_copro_fifoin_wpos;
	UINT32 *m_copro_fifoin_data;
	int m_copro_fifoin_num;
	int m_copro_fifoout_rpos;
	int m_copro_fifoout_wpos;
	UINT32 *m_copro_fifoout_data;
	int m_copro_fifoout_num;
	UINT16 m_cmd_data;
	UINT8 m_driveio_comm_data;
	int m_iop_write_num;
	UINT32 m_iop_data;
	int m_geo_iop_write_num;
	UINT32 m_geo_iop_data;
	int m_to_68k;
	int m_protstate;
	int m_protpos;
	UINT8 m_protram[256];
	int m_prot_a;
	int m_maxxstate;
	UINT32 m_netram[0x8000/4];
	int m_zflagi;
	int m_zflag;
	int m_sysres;
	int m_scsp_last_line;
	int m_jnet_time_out;
	UINT32 m_geo_read_start_address;
	UINT32 m_geo_write_start_address;
	UINT32 *m_bufferram;
	UINT32 *m_colorxlat;
	UINT32 *m_textureram0;
	UINT32 *m_textureram1;
	UINT32 *m_lumaram;
	UINT32 *m_paletteram32;
	poly_manager *m_poly;
	raster_state *m_raster;
	geo_state *m_geo;
	bitmap_t *m_sys24_bitmap;

	required_device<cpu_device> m_maincpu;
};


/*----------- defined in video/model2.c -----------*/

VIDEO_START(model2);
SCREEN_UPDATE(model2);

void model2_3d_set_zclip( running_machine &machine, UINT8 clip );

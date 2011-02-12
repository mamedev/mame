#include "video/poly.h"


typedef struct _raster_state raster_state;
typedef struct _geo_state geo_state;


class model2_state : public driver_device
{
public:
	model2_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 *workram;
	UINT32 intreq;
	UINT32 intena;
	UINT32 coproctl;
	UINT32 coprocnt;
	UINT32 geoctl;
	UINT32 geocnt;
	UINT16 *soundram;
	UINT32 timervals[4];
	UINT32 timerorig[4];
	int timerrun[4];
	timer_device *timers[4];
	int ctrlmode;
	int analog_channel;
	UINT32 *tgp_program;
	int dsp_type;
	int copro_fifoin_rpos;
	int copro_fifoin_wpos;
	UINT32 *copro_fifoin_data;
	int copro_fifoin_num;
	int copro_fifoout_rpos;
	int copro_fifoout_wpos;
	UINT32 *copro_fifoout_data;
	int copro_fifoout_num;
	UINT16 cmd_data;
	UINT8 driveio_comm_data;
	int iop_write_num;
	UINT32 iop_data;
	int geo_iop_write_num;
	UINT32 geo_iop_data;
	int to_68k;
	int protstate;
	int protpos;
	UINT8 protram[256];
	int prot_a;
	int maxxstate;
	UINT32 netram[0x8000/4];
	int zflagi;
	int zflag;
	int sysres;
	int scsp_last_line;
	int jnet_time_out;
	UINT32 geo_read_start_address;
	UINT32 geo_write_start_address;
	UINT32 *bufferram;
	UINT32 *colorxlat;
	UINT32 *textureram0;
	UINT32 *textureram1;
	UINT32 *lumaram;
	UINT32 *paletteram32;
	poly_manager *poly;
	raster_state *raster;
	geo_state *geo;
	bitmap_t *sys24_bitmap;
};


/*----------- defined in video/model2.c -----------*/

VIDEO_START(model2);
VIDEO_UPDATE(model2);

void model2_3d_set_zclip( running_machine *machine, UINT8 clip );

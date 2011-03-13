/*************************************************************************

    P&P Marketing Police Trainer hardware

**************************************************************************/

class policetr_state : public driver_device
{
public:
	policetr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT32 control_data;
	UINT32 bsmt_data_bank;
	UINT32 bsmt_data_offset;
	UINT32 *speedup_data;
	UINT64 last_cycles;
	UINT32 loop_count;
	offs_t speedup_pc;
	UINT32 *	rambase;
	UINT32 palette_offset;
	UINT8 palette_index;
	UINT8 palette_data[3];
	rectangle render_clip;
	UINT8 *srcbitmap;
	UINT8 *dstbitmap;
	UINT16 src_xoffs;
	UINT16 src_yoffs;
	UINT16 dst_xoffs;
	UINT16 dst_yoffs;
	UINT8 video_latch;
	UINT32 srcbitmap_height_mask;
};


/*----------- defined in video/policetr.c -----------*/

WRITE32_HANDLER( policetr_video_w );
READ32_HANDLER( policetr_video_r );

WRITE32_HANDLER( policetr_palette_offset_w );
WRITE32_HANDLER( policetr_palette_data_w );

VIDEO_START( policetr );
SCREEN_UPDATE( policetr );

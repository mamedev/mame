
#define RLT_NUM_BLITTER_REGS	8
#define RLT_NUM_BITMAPS			8

class rltennis_state : public driver_device
{
public:
	rltennis_state(const machine_config &mconfig, device_type type, const char *tag) : driver_device(mconfig, type, tag),
		m_data760000(0), m_data740000(0), m_dac_counter(0), m_sample_rom_offset_1(0), m_sample_rom_offset_2(0),
		m_offset_shift(0){ }

	device_t *m_maincpu;
	device_t *m_screen;

	UINT16 m_blitter[RLT_NUM_BLITTER_REGS];

	UINT8 *m_palette;
	INT32 m_palpos_r;
	INT32 m_palpos_w;

	INT32 m_data760000;
	INT32 m_data740000;
	INT32 m_dac_counter;
	INT32 m_sample_rom_offset_1;
	INT32 m_sample_rom_offset_2;

	INT32 m_offset_shift;

	INT32 m_unk_counter;

	bitmap_t *m_tmp_bitmap[RLT_NUM_BITMAPS];

	device_t *m_dac_1;
	device_t *m_dac_2;

	UINT8 *m_samples_1;
	UINT8 *m_samples_2;

	UINT8 *m_gfx;

	emu_timer *m_timer;

};



WRITE16_HANDLER( rlt_blitter_w );
WRITE16_HANDLER( rlt_ramdac_address_wm_w );
WRITE16_HANDLER( rlt_ramdac_address_rm_w );
WRITE16_HANDLER( rlt_ramdac_data_w );
READ16_HANDLER( rlt_ramdac_data_r );
VIDEO_START( rltennis );
SCREEN_UPDATE( rltennis );


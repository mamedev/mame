/*************************************************************************

    Incredible Technologies/Strata system
    (8-bit blitter variant)

**************************************************************************/

#include "machine/nvram.h"

#define VIDEO_CLOCK		XTAL_8MHz			/* video (pixel) clock */
#define CPU_CLOCK		XTAL_12MHz			/* clock for 68000-based systems */
#define CPU020_CLOCK	XTAL_25MHz			/* clock for 68EC020-based systems */
#define SOUND_CLOCK		XTAL_16MHz			/* clock for sound board */
#define TMS_CLOCK		XTAL_40MHz			/* TMS320C31 clocks on drivedge */


class itech32_state : public driver_device
{
public:
	itech32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	void nvram_init(nvram_device &nvram, void *base, size_t length);

	UINT16 *m_videoram;
	UINT8 m_vint_state;
	UINT8 m_xint_state;
	UINT8 m_qint_state;
	UINT8 m_sound_data;
	UINT8 m_sound_return;
	UINT8 m_sound_int_state;
	UINT16 *m_main_rom;
	UINT16 *m_main_ram;
	offs_t m_itech020_prot_address;
	UINT32 *m_tms1_ram;
	UINT32 *m_tms2_ram;
	UINT32 *m_tms1_boot;
	UINT8 m_tms_spinning[2];
	int m_special_result;
	int m_p1_effx;
	int m_p1_effy;
	int m_p1_lastresult;
	attotime m_p1_lasttime;
	int m_p2_effx;
	int m_p2_effy;
	int m_p2_lastresult;
	attotime m_p2_lasttime;
	UINT8 m_written[0x8000];
	int m_is_drivedge;
	UINT16 *m_video;
	UINT32 *m_drivedge_zbuf_control;
	UINT8 m_planes;
	UINT16 m_vram_height;
	UINT16 m_xfer_xcount;
	UINT16 m_xfer_ycount;
	UINT16 m_xfer_xcur;
	UINT16 m_xfer_ycur;
	rectangle m_clip_rect;
	rectangle m_scaled_clip_rect;
	rectangle m_clip_save;
	emu_timer *m_scanline_timer;
	UINT8 *m_grom_base;
	UINT32 m_grom_size;
	UINT32 m_grom_bank;
	UINT32 m_grom_bank_mask;
	UINT16 m_color_latch[2];
	UINT8 m_enable_latch[2];
	UINT16 *m_videoplane[2];
	UINT32 m_vram_mask;
	UINT32 m_vram_xmask;
	UINT32 m_vram_ymask;
	DECLARE_WRITE16_MEMBER(int1_ack_w);
	DECLARE_READ16_MEMBER(trackball_r);
	DECLARE_READ32_MEMBER(trackball32_8bit_r);
	DECLARE_READ32_MEMBER(trackball32_4bit_p1_r);
	DECLARE_READ32_MEMBER(trackball32_4bit_p2_r);
	DECLARE_READ32_MEMBER(trackball32_4bit_combined_r);
	DECLARE_READ32_MEMBER(drivedge_steering_r);
	DECLARE_READ32_MEMBER(drivedge_gas_r);
	DECLARE_READ16_MEMBER(wcbowl_prot_result_r);
	DECLARE_READ32_MEMBER(itech020_prot_result_r);
	DECLARE_READ32_MEMBER(gt2kp_prot_result_r);
	DECLARE_READ32_MEMBER(gtclass_prot_result_r);
	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_WRITE16_MEMBER(sound_data_w);
	DECLARE_READ32_MEMBER(sound_data32_r);
	DECLARE_WRITE32_MEMBER(sound_data32_w);
	DECLARE_READ8_MEMBER(sound_data_r);
	DECLARE_WRITE8_MEMBER(sound_return_w);
	DECLARE_READ8_MEMBER(sound_data_buffer_r);
	DECLARE_WRITE8_MEMBER(firq_clear_w);
	DECLARE_WRITE32_MEMBER(tms_reset_assert_w);
	DECLARE_WRITE32_MEMBER(tms_reset_clear_w);
	DECLARE_WRITE32_MEMBER(tms1_68k_ram_w);
	DECLARE_WRITE32_MEMBER(tms2_68k_ram_w);
	DECLARE_WRITE32_MEMBER(tms1_trigger_w);
	DECLARE_WRITE32_MEMBER(tms2_trigger_w);
	DECLARE_READ32_MEMBER(drivedge_tms1_speedup_r);
	DECLARE_READ32_MEMBER(drivedge_tms2_speedup_r);
	DECLARE_WRITE32_MEMBER(int1_ack32_w);
	DECLARE_READ32_MEMBER(test1_r);
	DECLARE_WRITE32_MEMBER(test1_w);
	DECLARE_READ32_MEMBER(test2_r);
	DECLARE_WRITE32_MEMBER(test2_w);
	DECLARE_WRITE16_MEMBER(timekill_colora_w);
	DECLARE_WRITE16_MEMBER(timekill_colorbc_w);
	DECLARE_WRITE16_MEMBER(timekill_intensity_w);
	DECLARE_WRITE16_MEMBER(bloodstm_color1_w);
	DECLARE_WRITE16_MEMBER(bloodstm_color2_w);
	DECLARE_WRITE16_MEMBER(bloodstm_plane_w);
	DECLARE_WRITE32_MEMBER(drivedge_color0_w);
	DECLARE_WRITE32_MEMBER(itech020_color1_w);
	DECLARE_WRITE32_MEMBER(itech020_color2_w);
	DECLARE_WRITE32_MEMBER(itech020_plane_w);
	DECLARE_WRITE16_MEMBER(timekill_paletteram_w);
	DECLARE_WRITE16_MEMBER(bloodstm_paletteram_w);
	DECLARE_WRITE32_MEMBER(drivedge_paletteram_w);
	DECLARE_WRITE32_MEMBER(itech020_paletteram_w);
	DECLARE_WRITE16_MEMBER(itech32_video_w);
	DECLARE_READ16_MEMBER(itech32_video_r);
	DECLARE_WRITE16_MEMBER(bloodstm_video_w);
	DECLARE_READ16_MEMBER(bloodstm_video_r);
	DECLARE_WRITE32_MEMBER(itech020_video_w);
	DECLARE_WRITE32_MEMBER(drivedge_zbuf_control_w);
	DECLARE_READ32_MEMBER(itech020_video_r);
};


/*----------- defined in drivers/itech32.c -----------*/

void itech32_update_interrupts(running_machine &machine, int vint, int xint, int qint);


/*----------- defined in video/itech32.c -----------*/

VIDEO_START( itech32 );








SCREEN_UPDATE_IND16( itech32 );

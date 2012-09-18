/*************************************************************************

    Exidy 6502 hardware

*************************************************************************/

#define EXIDY_MASTER_CLOCK				(XTAL_11_289MHz)
#define EXIDY_CPU_CLOCK					(EXIDY_MASTER_CLOCK / 16)
#define EXIDY_PIXEL_CLOCK				(EXIDY_MASTER_CLOCK / 2)
#define EXIDY_HTOTAL					(0x150)
#define EXIDY_HBEND						(0x000)
#define EXIDY_HBSTART					(0x100)
#define EXIDY_HSEND						(0x140)
#define EXIDY_HSSTART					(0x120)
#define EXIDY_VTOTAL					(0x118)
#define EXIDY_VBEND						(0x000)
#define EXIDY_VBSTART					(0x100)
#define EXIDY_VSEND						(0x108)
#define EXIDY_VSSTART					(0x100)


class exidy_state : public driver_device
{
public:
	exidy_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_sprite1_xpos(*this, "sprite1_xpos"),
		m_sprite1_ypos(*this, "sprite1_ypos"),
		m_sprite2_xpos(*this, "sprite2_xpos"),
		m_sprite2_ypos(*this, "sprite2_ypos"),
		m_spriteno(*this, "spriteno"),
		m_sprite_enable(*this, "sprite_enable"),
		m_color_latch(*this, "color_latch"),
		m_characterram(*this, "characterram"){ }

	UINT8 m_last_dial;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_sprite1_xpos;
	required_shared_ptr<UINT8> m_sprite1_ypos;
	required_shared_ptr<UINT8> m_sprite2_xpos;
	required_shared_ptr<UINT8> m_sprite2_ypos;
	required_shared_ptr<UINT8> m_spriteno;
	required_shared_ptr<UINT8> m_sprite_enable;
	required_shared_ptr<UINT8> m_color_latch;
	required_shared_ptr<UINT8> m_characterram;
	UINT8 m_collision_mask;
	UINT8 m_collision_invert;
	int m_is_2bpp;
	UINT8 m_int_condition;
	bitmap_ind16 m_background_bitmap;
	bitmap_ind16 m_motion_object_1_vid;
	bitmap_ind16 m_motion_object_2_vid;
	bitmap_ind16 m_motion_object_2_clip;
	DECLARE_WRITE8_MEMBER(fax_bank_select_w);
	DECLARE_READ8_MEMBER(exidy_interrupt_r);
	DECLARE_CUSTOM_INPUT_MEMBER(teetert_input_r);
	DECLARE_DRIVER_INIT(fax);
	DECLARE_DRIVER_INIT(sidetrac);
	DECLARE_DRIVER_INIT(pepper2);
	DECLARE_DRIVER_INIT(targ);
	DECLARE_DRIVER_INIT(rallys);
	DECLARE_DRIVER_INIT(mtrap);
	DECLARE_DRIVER_INIT(teetert);
	DECLARE_DRIVER_INIT(venture);
	DECLARE_DRIVER_INIT(spectar);
	DECLARE_DRIVER_INIT(phantoma);
	virtual void video_start();
	DECLARE_MACHINE_START(teetert);
	UINT32 screen_update_exidy(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(exidy_vblank_interrupt);
};

/*----------- defined in video/exidy.c -----------*/

void exidy_video_config(running_machine &machine, UINT8 _collision_mask, UINT8 _collision_invert, int _is_2bpp);

// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#include "machine/eepromser.h"
#include "video/tc0100scn.h"
#include "video/tc0480scp.h"

struct uf_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class undrfire_state : public driver_device
{
public:
	enum
	{
		TIMER_INTERRUPT5
	};

	undrfire_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0480scp(*this, "tc0480scp"),
		m_eeprom(*this, "eeprom"),
		m_ram(*this, "ram"),
		m_shared_ram(*this, "shared_ram"),
		m_spriteram(*this, "spriteram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_shared_ptr<UINT32> m_ram;
	optional_shared_ptr<UINT32> m_shared_ram;
	UINT16 m_coin_word;
	UINT16 m_port_sel;
	int m_frame_counter;
	std::unique_ptr<uf_tempsprite[]> m_spritelist;
	UINT16 m_rotate_ctrl[8];
	UINT8 m_dislayer[6];
	required_shared_ptr<UINT32> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	DECLARE_READ32_MEMBER(undrfire_input_r);
	DECLARE_WRITE32_MEMBER(undrfire_input_w);
	DECLARE_READ16_MEMBER(shared_ram_r);
	DECLARE_WRITE16_MEMBER(shared_ram_w);
	DECLARE_READ32_MEMBER(unknown_hardware_r);
	DECLARE_WRITE32_MEMBER(unknown_int_req_w);
	DECLARE_READ32_MEMBER(undrfire_lightgun_r);
	DECLARE_WRITE32_MEMBER(rotate_control_w);
	DECLARE_WRITE32_MEMBER(motor_control_w);
	DECLARE_WRITE32_MEMBER(cbombers_cpua_ctrl_w);
	DECLARE_READ32_MEMBER(cbombers_adc_r);
	DECLARE_WRITE8_MEMBER(cbombers_adc_w);
	DECLARE_CUSTOM_INPUT_MEMBER(frame_counter_r);
	DECLARE_DRIVER_INIT(undrfire);
	DECLARE_DRIVER_INIT(cbombers);
	virtual void video_start() override;
	UINT32 screen_update_undrfire(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cbombers(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(undrfire_interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const int *primasks,int x_offs,int y_offs);
	void draw_sprites_cbombers(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,const int *primasks,int x_offs,int y_offs);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};

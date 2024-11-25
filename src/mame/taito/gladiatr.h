// license:BSD-3-Clause
// copyright-holders:Victor Trucco,Steve Ellenoff,Phil Stroffolino,Tatsuyuki Satoh,Tomasz Slanina,Nicola Salmoria,Vas Crabb

#include "machine/gen_latch.h"
#include "sound/msm5205.h"
#include "cpu/m6809/m6809.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z80/z80.h"
#include "emupal.h"
#include "tilemap.h"


class gladiatr_state_base : public driver_device
{
protected:
	gladiatr_state_base(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "sub")
		, m_audiocpu(*this, "audiocpu")
		, m_cctl(*this, "cctl")
		, m_ccpu(*this, "ccpu")
		, m_ucpu(*this, "ucpu")
		, m_csnd(*this, "csnd")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_msm(*this, "msm")
		, m_soundlatch(*this, "soundlatch")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_textram(*this, "textram")
		, m_spriteram(*this, "spriteram")
		, m_video_attributes(0)
		, m_fg_scrolly(0)
		, m_fg_tile_bank(0)
		, m_bg_tile_bank(0)
		, m_sprite_bank(0)
		, m_sprite_buffer(0)
		, m_fg_tilemap(nullptr)
		, m_bg_tilemap(nullptr)
	{
	}

	required_device<z80_device>             m_maincpu;
	required_device<z80_device>             m_subcpu;
	required_device<mc6809_device>          m_audiocpu;
	optional_device<upi41_cpu_device>       m_cctl;
	optional_device<upi41_cpu_device>       m_ccpu;
	optional_device<upi41_cpu_device>       m_ucpu;
	optional_device<upi41_cpu_device>       m_csnd;
	required_device<gfxdecode_device>       m_gfxdecode;
	required_device<palette_device>         m_palette;
	required_device<msm5205_device>         m_msm;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<u8>            m_videoram;
	required_shared_ptr<u8>            m_colorram;
	required_shared_ptr<u8>            m_textram;
	required_shared_ptr<u8>            m_spriteram;

	u8          m_video_attributes;
	u16         m_fg_scrolly;
	u32         m_fg_tile_bank;
	u32         m_bg_tile_bank;
	u32         m_sprite_bank;
	u8          m_sprite_buffer;

	tilemap_t   *m_fg_tilemap;
	tilemap_t   *m_bg_tilemap;

	void videoram_w(offs_t offset, u8 data);
	void colorram_w(offs_t offset, u8 data);
	void textram_w(offs_t offset, u8 data);
	void spritebuffer_w(int state);
	void adpcm_command_w(u8 data);
	u8 adpcm_command_r();
	void ym_irq(int state);

	TILE_GET_INFO_MEMBER(bg_get_tile_info);
	TILE_GET_INFO_MEMBER(fg_get_tile_info);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cpu2_map(address_map &map) ATTR_COLD;

};

class gladiatr_state : public gladiatr_state_base
{
public:
	gladiatr_state(const machine_config &mconfig, device_type type, const char *tag)
		: gladiatr_state_base(mconfig, type, tag)
		, m_mainbank(*this, "mainbank")
		, m_adpcmbank(*this, "adpcmbank")
		, m_dsw1(*this, "DSW1")
		, m_dsw2(*this, "DSW2")
		, m_in0(*this, "IN0")
		, m_in1(*this, "IN1")
		, m_in2(*this, "IN2")
		, m_coins(*this, "COINS")
		, m_tclk_val(false)
		, m_cctl_p1(0xff)
		, m_cctl_p2(0xff)
		, m_ucpu_p1(0xff)
		, m_csnd_p1(0xff)
		, m_fg_scrollx(0)
		, m_bg_scrollx(0)
		, m_bg_scrolly(0)
	{
	}

	DECLARE_INPUT_CHANGED_MEMBER(p1_s1);
	DECLARE_INPUT_CHANGED_MEMBER(p1_s2);
	DECLARE_INPUT_CHANGED_MEMBER(p2_s1);
	DECLARE_INPUT_CHANGED_MEMBER(p2_s2);

	void gladiatr(machine_config &config);
	void greatgur(machine_config &config);

	void init_gladiatr();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_memory_bank m_mainbank;
	required_memory_bank m_adpcmbank;
	required_ioport m_dsw1, m_dsw2;
	required_ioport m_in0, m_in1, m_in2;
	required_ioport m_coins;

	bool    m_tclk_val;
	u8      m_cctl_p1, m_cctl_p2;
	u8      m_ucpu_p1, m_csnd_p1;

	u16     m_fg_scrollx;
	u16     m_bg_scrollx;
	u16     m_bg_scrolly;

	void spritebank_w(int state);
	void gladiatr_video_registers_w(offs_t offset, u8 data);

	void gladiatr_irq_patch_w(u8 data);
	void gladiator_int_control_w(u8 data);
	void gladiator_adpcm_w(u8 data);

	void tclk_w(int state);
	u8 cctl_p1_r();
	u8 cctl_p2_r();
	void ccpu_p2_w(u8 data);
	int tclk_r();
	int ucpu_t1_r();
	u8 ucpu_p1_r();
	void ucpu_p1_w(u8 data);
	u8 ucpu_p2_r();
	int csnd_t1_r();
	u8 csnd_p1_r();
	void csnd_p1_w(u8 data);
	u8 csnd_p2_r();

	u32 screen_update_gladiatr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void gladiatr_cpu1_io(address_map &map) ATTR_COLD;
	void gladiatr_cpu1_map(address_map &map) ATTR_COLD;
	void gladiatr_cpu2_io(address_map &map) ATTR_COLD;
	void gladiatr_cpu3_map(address_map &map) ATTR_COLD;
};

class ppking_state : public gladiatr_state_base
{
public:
	ppking_state(const machine_config &mconfig, device_type type, const char *tag)
		: gladiatr_state_base(mconfig, type, tag)
		, m_nvram(*this, "nvram")
		, m_soundlatch2(*this, "soundlatch2")
	{
	}

	void init_ppking();

	void ppking(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<u8>    m_nvram;
	required_device<generic_latch_8_device> m_soundlatch2;

	struct
	{
		u8 rxd = 0U;
		u8 txd = 0U;
		u8 rst = 0U;
		u8 state = 0U;
		u8 packet_type = 0U;
	} m_mcu[2];

	bool mcu_parity_check();
	void mcu_input_check();

	u8 ppking_f1_r();
	void ppking_qx0_w(offs_t offset, u8 data);
	void ppking_qx1_w(offs_t offset, u8 data);
	void ppking_qx3_w(u8 data);
	u8 ppking_qx3_r(offs_t offset);
	u8 ppking_qx0_r(offs_t offset);
	u8 ppking_qx1_r(offs_t offset);
	u8 ppking_qxcomu_r(offs_t offset);
	void ppking_qxcomu_w(u8 data);
	void ppking_video_registers_w(offs_t offset, u8 data);
	void ppking_adpcm_w(u8 data);
	void cpu2_irq_ack_w(u8 data);

	u32 screen_update_ppking(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void ppking_cpu1_io(address_map &map) ATTR_COLD;
	void ppking_cpu1_map(address_map &map) ATTR_COLD;
	void ppking_cpu2_io(address_map &map) ATTR_COLD;
	void ppking_cpu3_map(address_map &map) ATTR_COLD;
};

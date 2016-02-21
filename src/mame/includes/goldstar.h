// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca, Vas Crabb
#include "emu.h"

#include "machine/ticket.h"


class goldstar_state : public driver_device
{
public:
	goldstar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_fg_vidram(*this, "fg_vidram"),
		m_fg_atrram(*this, "fg_atrram"),
		m_bg_vidram(*this, "bg_vidram"),
		m_bg_atrram(*this, "bg_atrram"),
		m_reel1_ram(*this, "reel1_ram"),
		m_reel2_ram(*this, "reel2_ram"),
		m_reel3_ram(*this, "reel3_ram"),
		m_reel1_scroll(*this, "reel1_scroll"),
		m_reel2_scroll(*this, "reel2_scroll"),
		m_reel3_scroll(*this, "reel3_scroll"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
	}

	DECLARE_WRITE8_MEMBER(protection_w);
	DECLARE_READ8_MEMBER(protection_r);
	DECLARE_WRITE8_MEMBER(p1_lamps_w);
	DECLARE_WRITE8_MEMBER(p2_lamps_w);
	DECLARE_WRITE8_MEMBER(ncb3_port81_w);
	DECLARE_WRITE8_MEMBER(cm_coincount_w);
	DECLARE_WRITE8_MEMBER(goldstar_fg_vidram_w);
	DECLARE_WRITE8_MEMBER(goldstar_fg_atrram_w);
	DECLARE_WRITE8_MEMBER(goldstar_reel1_ram_w);
	DECLARE_WRITE8_MEMBER(goldstar_reel2_ram_w);
	DECLARE_WRITE8_MEMBER(goldstar_reel3_ram_w);
	DECLARE_WRITE8_MEMBER(goldstar_fa00_w);
	DECLARE_WRITE8_MEMBER(ay8910_outputa_w);
	DECLARE_WRITE8_MEMBER(ay8910_outputb_w);
	DECLARE_DRIVER_INIT(goldstar);
	DECLARE_DRIVER_INIT(cmast91);
	DECLARE_DRIVER_INIT(wcherry);
	DECLARE_DRIVER_INIT(super9);
	DECLARE_VIDEO_START(goldstar);
	DECLARE_PALETTE_INIT(cm);
	DECLARE_VIDEO_START(cherrym);
	DECLARE_PALETTE_INIT(cmast91);
	DECLARE_PALETTE_INIT(lucky8);
	UINT32 screen_update_goldstar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cmast91(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	TILE_GET_INFO_MEMBER(get_goldstar_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_cherrym_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_goldstar_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_goldstar_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_goldstar_reel3_tile_info);

	int m_dataoffset;

	required_shared_ptr<UINT8> m_fg_vidram;
	required_shared_ptr<UINT8> m_fg_atrram;

	optional_shared_ptr<UINT8> m_bg_vidram;
	optional_shared_ptr<UINT8> m_bg_atrram;

	optional_shared_ptr<UINT8> m_reel1_ram;
	optional_shared_ptr<UINT8> m_reel2_ram;
	optional_shared_ptr<UINT8> m_reel3_ram;

	optional_shared_ptr<UINT8> m_reel1_scroll;
	optional_shared_ptr<UINT8> m_reel2_scroll;
	optional_shared_ptr<UINT8> m_reel3_scroll;

	tilemap_t *m_reel1_tilemap;
	tilemap_t *m_reel2_tilemap;
	tilemap_t *m_reel3_tilemap;

	int m_bgcolor;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	UINT8 m_cmaster_girl_num;
	UINT8 m_cmaster_girl_pal;
	UINT8 m_cm_enable_reg;
	UINT8 m_cm_girl_scroll;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


class cmaster_state : public goldstar_state
{
public:
	cmaster_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldstar_state(mconfig, type, tag)
	{
	}

	DECLARE_WRITE8_MEMBER(outport0_w);
	DECLARE_WRITE8_MEMBER(girl_scroll_w);
	DECLARE_WRITE8_MEMBER(background_col_w);

	DECLARE_DRIVER_INIT(cm);
	DECLARE_DRIVER_INIT(cmv4);
	DECLARE_DRIVER_INIT(tonypok);
	DECLARE_DRIVER_INIT(schery97);
	DECLARE_DRIVER_INIT(schery97a);
	DECLARE_DRIVER_INIT(skill98);
	DECLARE_DRIVER_INIT(po33);
	DECLARE_DRIVER_INIT(match133);
	DECLARE_DRIVER_INIT(nfb96_dk);
	DECLARE_DRIVER_INIT(nfb96_c2);
	DECLARE_DRIVER_INIT(nfb96_d);
	DECLARE_DRIVER_INIT(nfb96_c1);
	DECLARE_DRIVER_INIT(nfb96sea);
	DECLARE_DRIVER_INIT(fb2010);
	DECLARE_DRIVER_INIT(rp35);
	DECLARE_DRIVER_INIT(rp36);
	DECLARE_DRIVER_INIT(rp36c3);
	DECLARE_DRIVER_INIT(rp96sub);

	UINT32 screen_update_amcoe1a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// installed by various driver init handlers to get stuff to work
	READ8_MEMBER(fixedval09_r) { return 0x09; }
	READ8_MEMBER(fixedval38_r) { return 0x38; }
	READ8_MEMBER(fixedval48_r) { return 0x48; }
	READ8_MEMBER(fixedval58_r) { return 0x58; }
	READ8_MEMBER(fixedval68_r) { return 0x68; }
	READ8_MEMBER(fixedval74_r) { return 0x74; }
	READ8_MEMBER(fixedval80_r) { return 0x80; }
	READ8_MEMBER(fixedval82_r) { return 0x82; }
	READ8_MEMBER(fixedval84_r) { return 0x84; }
	READ8_MEMBER(fixedval90_r) { return 0x90; }
	READ8_MEMBER(fixedval96_r) { return 0x96; }
	READ8_MEMBER(fixedvala8_r) { return 0xa8; }
	READ8_MEMBER(fixedvalaa_r) { return 0xaa; }
	READ8_MEMBER(fixedvalb2_r) { return 0xb2; }
	READ8_MEMBER(fixedvalb4_r) { return 0xb4; }
	READ8_MEMBER(fixedvalbe_r) { return 0xbe; }
	READ8_MEMBER(fixedvalc7_r) { return 0xc7; }
	READ8_MEMBER(fixedvalea_r) { return 0xea; }
	READ8_MEMBER(fixedvale4_r) { return 0xe4; }
};


class wingco_state : public goldstar_state
{
public:
	wingco_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldstar_state(mconfig, type, tag)
	{
	}

	DECLARE_WRITE8_MEMBER(magodds_outb850_w);
	DECLARE_WRITE8_MEMBER(magodds_outb860_w);
	DECLARE_WRITE8_MEMBER(system_outputa_w);
	DECLARE_WRITE8_MEMBER(system_outputb_w);
	DECLARE_WRITE8_MEMBER(system_outputc_w);

	DECLARE_DRIVER_INIT(lucky8a);
	DECLARE_DRIVER_INIT(magoddsc);

	DECLARE_VIDEO_START(bingowng);
	DECLARE_VIDEO_START(magical);
	DECLARE_PALETTE_INIT(magodds);
	UINT32 screen_update_bingowng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_magical(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(masked_irq);

protected:
	TILE_GET_INFO_MEMBER(get_magical_fg_tile_info);

private:
	UINT8 m_nmi_enable;
	UINT8 m_vidreg;

	int m_tile_bank;
};


class cb3_state : public goldstar_state
{
public:
	cb3_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldstar_state(mconfig, type, tag)
	{
	}

	DECLARE_DRIVER_INIT(cb3);
	DECLARE_DRIVER_INIT(cb3e);
	DECLARE_DRIVER_INIT(cherrys);
	DECLARE_DRIVER_INIT(chrygld);
	DECLARE_DRIVER_INIT(chry10);

protected:
	void do_blockswaps(UINT8* ROM);
	void dump_to_file(UINT8* ROM);

	UINT8 cb3_decrypt(UINT8 cipherText, UINT16 address);
	UINT8 chry10_decrypt(UINT8 cipherText);
};


class sanghopm_state : public goldstar_state
{
public:
	sanghopm_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldstar_state(mconfig, type, tag),
		m_reel1_attrram(*this, "reel1_attrram"),
		m_reel2_attrram(*this, "reel2_attrram"),
		m_reel3_attrram(*this, "reel3_attrram")
	{
	}

	DECLARE_WRITE8_MEMBER(enable_w);
	DECLARE_WRITE8_MEMBER(coincount_w);

	DECLARE_WRITE8_MEMBER(fg_vidram_w);
	DECLARE_WRITE8_MEMBER(fg_atrram_w);
	DECLARE_WRITE8_MEMBER(bg_vidram_w);
	DECLARE_WRITE8_MEMBER(bg_atrram_w);
	DECLARE_WRITE8_MEMBER(reel1_attrram_w);
	DECLARE_WRITE8_MEMBER(reel2_attrram_w);
	DECLARE_WRITE8_MEMBER(reel3_attrram_w);

	DECLARE_VIDEO_START(sangho);
	UINT32 screen_update_sangho(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_reel3_tile_info);

private:
	required_shared_ptr<UINT8> m_reel1_attrram;
	required_shared_ptr<UINT8> m_reel2_attrram;
	required_shared_ptr<UINT8> m_reel3_attrram;

	UINT8 m_enable_reg;
};


class unkch_state : public goldstar_state
{
public:
	unkch_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldstar_state(mconfig, type, tag),
		m_reel1_attrram(*this, "reel1_attrram"),
		m_reel2_attrram(*this, "reel2_attrram"),
		m_reel3_attrram(*this, "reel3_attrram"),
		m_ticket_dispenser(*this, "tickets")
	{
	}

	DECLARE_WRITE8_MEMBER(coincount_w);
	DECLARE_WRITE8_MEMBER(unkcm_0x02_w);
	DECLARE_WRITE8_MEMBER(unkcm_0x03_w);

	DECLARE_WRITE8_MEMBER(reel1_attrram_w);
	DECLARE_WRITE8_MEMBER(reel2_attrram_w);
	DECLARE_WRITE8_MEMBER(reel3_attrram_w);

	DECLARE_DRIVER_INIT(unkch1);
	DECLARE_DRIVER_INIT(unkch3);
	DECLARE_DRIVER_INIT(unkch4);

	DECLARE_VIDEO_START(unkch);
	UINT32 screen_update_unkch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);

protected:
	TILE_GET_INFO_MEMBER(get_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_reel3_tile_info);

private:
	required_shared_ptr<UINT8> m_reel1_attrram;
	required_shared_ptr<UINT8> m_reel2_attrram;
	required_shared_ptr<UINT8> m_reel3_attrram;

	UINT8 m_vblank_irq_enable;
	UINT8 m_vidreg;

	optional_device<ticket_dispenser_device> m_ticket_dispenser;
};

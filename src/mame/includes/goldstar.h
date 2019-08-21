// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca, Vas Crabb
#ifndef MAME_INCLUDES_GOLDSTAR_H
#define MAME_INCLUDES_GOLDSTAR_H

#pragma once

#include "machine/ds2401.h"
#include "machine/i8255.h"
#include "machine/ticket.h"
#include "emupal.h"
#include "tilemap.h"


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
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi8255_%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

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
	void init_chryangl();
	void init_goldstar();
	void init_jkrmast();
	void init_cmast91();
	void init_wcherry();
	void init_super9();
	DECLARE_VIDEO_START(goldstar);
	void cm_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(cherrym);
	void cmast91_palette(palette_device &palette) const;
	void lucky8_palette(palette_device &palette) const;
	uint32_t screen_update_goldstar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cmast91(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ladylinr(machine_config &config);
	void wcherry(machine_config &config);
	void pkrmast(machine_config &config);
	void moonlght(machine_config &config);
	void kkotnoli(machine_config &config);
	void cmast91(machine_config &config);
	void super9(machine_config &config);
	void goldfrui(machine_config &config);
	void goldstar(machine_config &config);
	void goldstbl(machine_config &config);
	void bonusch_portmap(address_map &map);
	void feverch_portmap(address_map &map);
	void cm_map(address_map &map);
	void cmast91_portmap(address_map &map);
	void flaming7_map(address_map &map);
	void goldstar_map(address_map &map);
	void goldstar_readport(address_map &map);
	void kkotnoli_map(address_map &map);
	void ladylinr_map(address_map &map);
	void lucky8_map(address_map &map);
	void lucky8f_decrypted_opcodes_map(address_map &map);
	void mbstar_map(address_map &map);
	void megaline_portmap(address_map &map);
	void ncb3_readwriteport(address_map &map);
	void nfm_map(address_map &map);
	void pkrmast_portmap(address_map &map);
	void ramdac_map(address_map &map);
	void wcat3_map(address_map &map);
	void wcherry_map(address_map &map);
	void wcherry_readwriteport(address_map &map);

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	TILE_GET_INFO_MEMBER(get_goldstar_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_cherrym_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_goldstar_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_goldstar_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_goldstar_reel3_tile_info);

	int m_dataoffset;

	required_shared_ptr<uint8_t> m_fg_vidram;
	required_shared_ptr<uint8_t> m_fg_atrram;

	optional_shared_ptr<uint8_t> m_bg_vidram;
	optional_shared_ptr<uint8_t> m_bg_atrram;

	optional_shared_ptr<uint8_t> m_reel1_ram;
	optional_shared_ptr<uint8_t> m_reel2_ram;
	optional_shared_ptr<uint8_t> m_reel3_ram;

	optional_shared_ptr<uint8_t> m_reel1_scroll;
	optional_shared_ptr<uint8_t> m_reel2_scroll;
	optional_shared_ptr<uint8_t> m_reel3_scroll;

	optional_shared_ptr<uint8_t> m_decrypted_opcodes;

	tilemap_t *m_reel1_tilemap;
	tilemap_t *m_reel2_tilemap;
	tilemap_t *m_reel3_tilemap;

	int m_bgcolor;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	uint8_t m_cmaster_girl_num;
	uint8_t m_cmaster_girl_pal;
	uint8_t m_cm_enable_reg;
	uint8_t m_cm_girl_scroll;

	required_device<cpu_device> m_maincpu;
	optional_device_array<i8255_device, 3> m_ppi;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	output_finder<16> m_lamps;
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

	void init_cm();
	void init_cmv4();
	void init_tonypok();
	void init_schery97();
	void init_schery97a();
	void init_skill98();
	void init_po33();
	void init_match133();
	void init_nfb96_dk();
	void init_nfb96_c2();
	void init_nfb96_d();
	void init_nfb96_c1();
	void init_nfb96sea();
	void init_fb2010();
	void init_rp35();
	void init_rp36();
	void init_rp36c3();
	void init_rp96sub();
	void init_tcl();
	void init_super7();

	uint32_t screen_update_amcoe1a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void cm(machine_config &config);
	void cmasterc(machine_config &config);
	void amcoe1a(machine_config &config);
	void nfm(machine_config &config);
	void amcoe2(machine_config &config);
	void amcoe1(machine_config &config);
	void chryangl(machine_config &config);
	void amcoe1_portmap(address_map &map);
	void amcoe2_portmap(address_map &map);
	void cm_portmap(address_map &map);
	void chryangl_decrypted_opcodes_map(address_map &map);

protected:
	// installed by various driver init handlers to get stuff to work
	READ8_MEMBER(fixedval09_r) { return 0x09; }
	READ8_MEMBER(fixedval38_r) { return 0x38; }
	READ8_MEMBER(fixedval48_r) { return 0x48; }
	READ8_MEMBER(fixedval58_r) { return 0x58; }
	READ8_MEMBER(fixedval68_r) { return 0x68; }
	READ8_MEMBER(fixedval74_r) { return 0x74; }
	READ8_MEMBER(fixedval7d_r) { return 0x7d; }
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
		goldstar_state(mconfig, type, tag),
		m_fl7w4_id(*this, "fl7w4_id")
	{
	}

	DECLARE_WRITE8_MEMBER(magodds_outb850_w);
	DECLARE_WRITE8_MEMBER(magodds_outb860_w);
	DECLARE_WRITE8_MEMBER(fl7w4_outc802_w);
	DECLARE_WRITE8_MEMBER(system_outputa_w);
	DECLARE_WRITE8_MEMBER(system_outputb_w);
	DECLARE_WRITE8_MEMBER(system_outputc_w);

	void init_lucky8a();
	void init_lucky8f();
	void init_magoddsc();
	void init_flaming7();
	void init_flam7_tw();
	void init_luckylad();

	DECLARE_VIDEO_START(bingowng);
	DECLARE_VIDEO_START(magical);
	void magodds_palette(palette_device &palette) const;
	uint32_t screen_update_bingowng(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_magical(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_mbstar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(masked_irq);

	void bingowng(machine_config &config);
	void flaming7(machine_config &config);
	void lucky8(machine_config &config);
	void lucky8f(machine_config &config);
	void wcat3(machine_config &config);
	void magodds(machine_config &config);
	void flam7_w4(machine_config &config);
	void bingownga(machine_config &config);
	void mbstar(machine_config &config);
	void flam7_tw(machine_config &config);
	void magodds_map(address_map &map);

protected:
	TILE_GET_INFO_MEMBER(get_magical_fg_tile_info);

private:
	optional_device<ds2401_device> m_fl7w4_id;

	uint8_t m_nmi_enable;
	uint8_t m_vidreg;

	int m_tile_bank;
};


class cb3_state : public goldstar_state
{
public:
	cb3_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldstar_state(mconfig, type, tag)
	{
	}

	void init_cb3();
	void init_cb3e();
	void init_cherrys();
	void init_chrygld();
	void init_chry10();

	void cherrys(machine_config &config);
	void chryangla(machine_config &config);
	void chrygld(machine_config &config);
	void cb3c(machine_config &config);
	void cb3e(machine_config &config);
	void ncb3(machine_config &config);
	void cm97(machine_config &config);
	void ncb3_map(address_map &map);
	void chryangla_map(address_map &map);
	void chryangla_decrypted_opcodes_map(address_map &map);

protected:
	void do_blockswaps(uint8_t* ROM);
	void dump_to_file(uint8_t* ROM);

	uint8_t cb3_decrypt(uint8_t cipherText, uint16_t address);
	uint8_t chry10_decrypt(uint8_t cipherText);
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
	uint32_t screen_update_sangho(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void star100(machine_config &config);
	void star100_map(address_map &map);
	void star100_readport(address_map &map);
protected:
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_reel3_tile_info);

private:
	required_shared_ptr<uint8_t> m_reel1_attrram;
	required_shared_ptr<uint8_t> m_reel2_attrram;
	required_shared_ptr<uint8_t> m_reel3_attrram;

	uint8_t m_enable_reg;
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

	void init_unkch1();
	void init_unkch3();
	void init_unkch4();

	DECLARE_VIDEO_START(unkch);
	uint32_t screen_update_unkch(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER(vblank_irq);

	void megaline(machine_config &config);
	void unkch(machine_config &config);
	void bonusch(machine_config &config);
	void feverch(machine_config &config);
	void bonusch_map(address_map &map);
	void feverch_map(address_map &map);
	void megaline_map(address_map &map);
	void unkch_map(address_map &map);
	void unkch_portmap(address_map &map);
protected:
	TILE_GET_INFO_MEMBER(get_reel1_tile_info);
	TILE_GET_INFO_MEMBER(get_reel2_tile_info);
	TILE_GET_INFO_MEMBER(get_reel3_tile_info);

private:
	required_shared_ptr<uint8_t> m_reel1_attrram;
	required_shared_ptr<uint8_t> m_reel2_attrram;
	required_shared_ptr<uint8_t> m_reel3_attrram;

	uint8_t m_vblank_irq_enable;
	uint8_t m_vidreg;

	optional_device<ticket_dispenser_device> m_ticket_dispenser;
};

#endif // MAME_INCLUDES_GOLDSTAR_H

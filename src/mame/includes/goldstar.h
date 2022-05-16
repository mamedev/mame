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
		m_bgcolor(0),
		m_maincpu(*this, "maincpu"),
		m_ppi(*this, "ppi8255_%u", 0U),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void protection_w(uint8_t data);
	uint8_t protection_r();
	void p1_lamps_w(uint8_t data);
	void p2_lamps_w(uint8_t data);
	void ncb3_port81_w(uint8_t data);
	void cm_coincount_w(uint8_t data);
	void goldstar_fg_vidram_w(offs_t offset, uint8_t data);
	void goldstar_fg_atrram_w(offs_t offset, uint8_t data);
	void goldstar_reel1_ram_w(offs_t offset, uint8_t data);
	void goldstar_reel2_ram_w(offs_t offset, uint8_t data);
	void goldstar_reel3_ram_w(offs_t offset, uint8_t data);
	void goldstar_fa00_w(uint8_t data);
	void ay8910_outputa_w(uint8_t data);
	void ay8910_outputb_w(uint8_t data);
	void init_chryangl();
	void init_goldstar();
	void init_jkrmast();
	void init_pkrmast();
	void init_crazybonb();
	void init_cmast91();
	void init_wcherry();
	void init_super9();
	void init_ladylinrb();
	void init_ladylinrc();
	void init_ladylinrd();
	void init_ladylinre();
	DECLARE_VIDEO_START(goldstar);
	void cm_palette(palette_device &palette) const;
	DECLARE_VIDEO_START(cherrym);
	void cmast91_palette(palette_device &palette) const;
	void lucky8_palette(palette_device &palette) const;
	void nfm_palette(palette_device &palette) const;
	uint32_t screen_update_goldstar(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cmast91(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ladylinr(machine_config &config);
	void ladylinrb(machine_config &config);
	void wcherry(machine_config &config);
	void crazybon(machine_config &config);
	void crazybonb(machine_config &config);
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
	void crazybon_portmap(address_map &map);
	void flaming7_map(address_map &map);
	void goldstar_map(address_map &map);
	void goldstar_readport(address_map &map);
	void kkotnoli_map(address_map &map);
	void ladylinr_map(address_map &map);
	void lucky8_map(address_map &map);
	void common_decrypted_opcodes_map(address_map &map);
	void super972_decrypted_opcodes_map(address_map &map);
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

	int m_dataoffset = 0;

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

	tilemap_t *m_reel1_tilemap = nullptr;
	tilemap_t *m_reel2_tilemap = nullptr;
	tilemap_t *m_reel3_tilemap = nullptr;

	int m_bgcolor = 0;
	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_cmaster_girl_num = 0U;
	uint8_t m_cmaster_girl_pal = 0U;
	uint8_t m_cm_enable_reg = 0U;
	uint8_t m_cm_girl_scroll = 0U;

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

	void outport0_w(uint8_t data);
	void girl_scroll_w(uint8_t data);
	void background_col_w(uint8_t data);

	void init_cm();
	void init_cmv4();
	void init_tonypok();
	void init_schery97();
	void init_schery97a();
	void init_skill98();
	void init_po33();
	void init_match133();
	void init_nfb96_a();
	void init_nfb96_b();
	void init_nfb96_c1();
	void init_nfb96_c1_2();
	void init_nfb96_c2();
	void init_nfb96_d();
	void init_nfb96_dk();
	void init_nfb96_g();
	void init_nfb96sea();
	void init_fb2010();
	void init_rp35();
	void init_rp36();
	void init_rp36c3();
	void init_rp96sub();
	void init_tcl();
	void init_super7();
	void init_chthree();
	void init_wcat3a();

	uint32_t screen_update_amcoe1a(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void cm(machine_config &config);
	void cm97(machine_config &config);
	void cmasterc(machine_config &config);
	void amcoe1a(machine_config &config);
	void nfm(machine_config &config);
	void amcoe2(machine_config &config);
	void amcoe1(machine_config &config);
	void chryangl(machine_config &config);
	void ss2001(machine_config &config);
	void super7(machine_config &config);
	void amcoe1_portmap(address_map &map);
	void amcoe2_portmap(address_map &map);
	void cm_portmap(address_map &map);
	void cm97_portmap(address_map &map);
	void super7_portmap(address_map &map);
	void chryangl_decrypted_opcodes_map(address_map &map);
	void ss2001_portmap(address_map &map);

protected:
	// installed by various driver init handlers to get stuff to work
	template <uint8_t V> uint8_t fixedval_r() { return V; }
};


class wingco_state : public goldstar_state
{
public:
	wingco_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldstar_state(mconfig, type, tag),
		m_fl7w4_id(*this, "fl7w4_id")
	{
	}

	void magodds_outb850_w(uint8_t data);
	void magodds_outb860_w(uint8_t data);
	void fl7w4_outc802_w(uint8_t data);
	void system_outputa_w(uint8_t data);
	void system_outputb_w(uint8_t data);
	void system_outputc_w(uint8_t data);

	void init_lucky8a();
	void init_lucky8f();
	void init_lucky8l();
	void init_magoddsc();
	void init_flaming7();
	void init_flam7_tw();
	void init_luckylad();
	void init_nd8lines();
	void init_super972();
	void init_wcat3();

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
	void lucky8k(machine_config &config);
	void luckylad(machine_config &config);
	void nd8lines(machine_config &config);
	void super972(machine_config &config);
	void wcat3(machine_config &config);
	void magodds(machine_config &config);
	void flam7_w4(machine_config &config);
	void bingownga(machine_config &config);
	void mbstar(machine_config &config);
	void flam7_tw(machine_config &config);
	void magodds_map(address_map &map);

protected:
	TILE_GET_INFO_MEMBER(get_magical_fg_tile_info);
	virtual void machine_start() override { goldstar_state::machine_start(); m_tile_bank = 0; }

private:
	optional_device<ds2401_device> m_fl7w4_id;

	uint8_t m_nmi_enable = 0U;
	uint8_t m_vidreg = 0U;

	uint8_t m_tile_bank = 0U;

	void nd8lines_map(address_map &map);
};


class cb3_state : public goldstar_state
{
public:
	cb3_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldstar_state(mconfig, type, tag)
	{
	}

	void init_cb3();
	void init_cb3c();
	void init_cb3e();
	void init_cb3f();
	void init_cherrys();
	void init_chrygld();
	void init_chry10();

	void cherrys(machine_config &config);
	void chryangla(machine_config &config);
	void chrygld(machine_config &config);
	void cb3c(machine_config &config);
	void cb3e(machine_config &config);
	void ncb3(machine_config &config);
	void eldoradd(machine_config &config);
	void ncb3_map(address_map &map);
	void chryangla_map(address_map &map);
	void chryangla_decrypted_opcodes_map(address_map &map);

protected:
	void do_blockswaps(uint8_t *rom);
	void dump_to_file(uint8_t *rom);

	uint8_t cb3_decrypt(uint8_t cipherText, uint16_t address);
	uint8_t cb3f_decrypt(uint8_t cipherText, uint16_t address);
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

	void enable_w(uint8_t data);
	void coincount_w(uint8_t data);

	void fg_vidram_w(offs_t offset, uint8_t data);
	void fg_atrram_w(offs_t offset, uint8_t data);
	void bg_vidram_w(offs_t offset, uint8_t data);
	void bg_atrram_w(offs_t offset, uint8_t data);
	void reel1_attrram_w(offs_t offset, uint8_t data);
	void reel2_attrram_w(offs_t offset, uint8_t data);
	void reel3_attrram_w(offs_t offset, uint8_t data);

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

	uint8_t m_enable_reg = 0U;
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

	void coincount_w(uint8_t data);
	void unkcm_0x02_w(uint8_t data);
	void unkcm_0x03_w(uint8_t data);

	void reel1_attrram_w(offs_t offset, uint8_t data);
	void reel2_attrram_w(offs_t offset, uint8_t data);
	void reel3_attrram_w(offs_t offset, uint8_t data);

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

	uint8_t m_vblank_irq_enable = 0U;
	uint8_t m_vidreg = 0U;

	optional_device<ticket_dispenser_device> m_ticket_dispenser;
};

#endif // MAME_INCLUDES_GOLDSTAR_H

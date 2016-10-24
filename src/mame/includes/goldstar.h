// license:BSD-3-Clause
// copyright-holders:David Haywood, Roberto Fresca, Vas Crabb
#include "emu.h"

#include "machine/ds2401.h"
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

	void protection_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void p1_lamps_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void p2_lamps_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ncb3_port81_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cm_coincount_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void goldstar_fg_vidram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void goldstar_fg_atrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void goldstar_reel1_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void goldstar_reel2_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void goldstar_reel3_ram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void goldstar_fa00_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_outputa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ay8910_outputb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_goldstar();
	void init_cmast91();
	void init_wcherry();
	void init_super9();
	void video_start_goldstar();
	void palette_init_cm(palette_device &palette);
	void video_start_cherrym();
	void palette_init_cmast91(palette_device &palette);
	void palette_init_lucky8(palette_device &palette);
	uint32_t screen_update_goldstar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cmast91(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	void get_goldstar_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_cherrym_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_goldstar_reel1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_goldstar_reel2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_goldstar_reel3_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

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

	void outport0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void girl_scroll_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void background_col_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

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

	uint32_t screen_update_amcoe1a(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// installed by various driver init handlers to get stuff to work
	uint8_t fixedval09_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x09; }
	uint8_t fixedval38_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x38; }
	uint8_t fixedval48_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x48; }
	uint8_t fixedval58_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x58; }
	uint8_t fixedval68_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x68; }
	uint8_t fixedval74_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x74; }
	uint8_t fixedval80_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x80; }
	uint8_t fixedval82_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x82; }
	uint8_t fixedval84_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x84; }
	uint8_t fixedval90_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x90; }
	uint8_t fixedval96_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0x96; }
	uint8_t fixedvala8_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0xa8; }
	uint8_t fixedvalaa_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0xaa; }
	uint8_t fixedvalb2_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0xb2; }
	uint8_t fixedvalb4_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0xb4; }
	uint8_t fixedvalbe_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0xbe; }
	uint8_t fixedvalc7_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0xc7; }
	uint8_t fixedvalea_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0xea; }
	uint8_t fixedvale4_r(address_space &space, offs_t offset, uint8_t mem_mask) { return 0xe4; }
};


class wingco_state : public goldstar_state
{
public:
	wingco_state(const machine_config &mconfig, device_type type, const char *tag) :
		goldstar_state(mconfig, type, tag),
		m_fl7w4_id(*this, "fl7w4_id")
	{
	}

	void magodds_outb850_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void magodds_outb860_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fl7w4_outc802_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void system_outputa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void system_outputb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void system_outputc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_lucky8a();
	void init_magoddsc();
	void init_flaming7();
	void init_flam7_tw();

	void video_start_bingowng();
	void video_start_magical();
	void palette_init_magodds(palette_device &palette);
	uint32_t screen_update_bingowng(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_magical(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void masked_irq(device_t &device);

protected:
	void get_magical_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

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

	void enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void coincount_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void fg_vidram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fg_atrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_vidram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bg_atrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reel1_attrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reel2_attrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reel3_attrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void video_start_sangho();
	uint32_t screen_update_sangho(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	void get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_reel1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_reel2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_reel3_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

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

	void coincount_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void unkcm_0x02_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void unkcm_0x03_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void reel1_attrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reel2_attrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reel3_attrram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_unkch1();
	void init_unkch3();
	void init_unkch4();

	void video_start_unkch();
	uint32_t screen_update_unkch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void vblank_irq(device_t &device);

protected:
	void get_reel1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_reel2_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_reel3_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

private:
	required_shared_ptr<uint8_t> m_reel1_attrram;
	required_shared_ptr<uint8_t> m_reel2_attrram;
	required_shared_ptr<uint8_t> m_reel3_attrram;

	uint8_t m_vblank_irq_enable;
	uint8_t m_vidreg;

	optional_device<ticket_dispenser_device> m_ticket_dispenser;
};

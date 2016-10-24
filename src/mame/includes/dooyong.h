// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Vas Crabb
#include "video/bufsprite.h"

#define MCFG_DOOYONG_ROM_TILEMAP_ADD(tag, gfx, num, rom, offset) \
		MCFG_DEVICE_ADD(tag, DOOYONG_ROM_TILEMAP, 0) \
		dooyong_rom_tilemap_device::static_set_gfxdecode_tag(*device, "^" gfx); \
		dooyong_rom_tilemap_device::static_set_tilerom_tag(*device, "^" rom); \
		dooyong_rom_tilemap_device::static_set_gfxnum(*device, (num)); \
		dooyong_rom_tilemap_device::static_set_tilerom_offset(*device, (offset));
#define MCFG_DOOYONG_ROM_TILEMAP_TRANSPARENT_PEN(pen) \
		dooyong_rom_tilemap_device::static_set_transparent_pen(*device, (pen));
#define MCFG_DOOYONG_ROM_TILEMAP_PRIMELLA_CODE_BITS(bits) \
		dooyong_rom_tilemap_device::static_set_primella_code_bits(*device, (bits));

#define MCFG_RSHARK_ROM_TILEMAP_ADD(tag, gfx, num, rom, offset, rom2, offset2) \
		MCFG_DEVICE_ADD(tag, RSHARK_ROM_TILEMAP, 0) \
		dooyong_rom_tilemap_device::static_set_gfxdecode_tag(*device, "^" gfx); \
		dooyong_rom_tilemap_device::static_set_tilerom_tag(*device, "^" rom); \
		dooyong_rom_tilemap_device::static_set_gfxnum(*device, (num)); \
		dooyong_rom_tilemap_device::static_set_tilerom_offset(*device, (offset)); \
		dooyong_rom_tilemap_device::static_set_primella_code_bits(*device, 13); \
		rshark_rom_tilemap_device::static_set_colorrom_tag(*device, "^" rom2); \
		rshark_rom_tilemap_device::static_set_colorrom_offset(*device, (offset2));

#define MCFG_DOOYONG_RAM_TILEMAP_ADD(tag, gfx, num) \
		MCFG_DEVICE_ADD(tag, DOOYONG_RAM_TILEMAP, 0) \
		dooyong_rom_tilemap_device::static_set_gfxdecode_tag(*device, "^" gfx); \
		dooyong_rom_tilemap_device::static_set_gfxnum(*device, (num));


extern device_type const DOOYONG_ROM_TILEMAP;
extern device_type const RSHARK_ROM_TILEMAP;
extern device_type const DOOYONG_RAM_TILEMAP;


class dooyong_tilemap_device_base : public device_t
{
public:
	static void static_set_gfxdecode_tag(device_t &device, char const *tag);
	static void static_set_gfxnum(device_t &device, int gfxnum);

	void draw(screen_device &screen, bitmap_ind16 &dest, rectangle const &cliprect, uint32_t flags, uint8_t priority);

	void set_palette_bank(uint16_t bank);

protected:
	dooyong_tilemap_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *name,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			char const *shortname,
			char const *source);

	gfx_element const &gfx() const { return *m_gfxdecode->gfx(m_gfxnum); }

	required_device<gfxdecode_device> m_gfxdecode;
	int m_gfxnum;

	tilemap_t *m_tilemap;
	uint16_t m_palette_bank;
};

class dooyong_rom_tilemap_device : public dooyong_tilemap_device_base
{
public:
	dooyong_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	static void static_set_tilerom_tag(device_t &device, char const *tag);
	static void static_set_tilerom_offset(device_t &device, int offset);
	static void static_set_transparent_pen(device_t &device, unsigned pen);
	static void static_set_primella_code_bits(device_t &device, unsigned bits);

	void ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	dooyong_rom_tilemap_device(
			machine_config const &mconfig,
			device_type type,
			char const *name,
			char const *tag,
			device_t *owner,
			uint32_t clock,
			char const *shortname,
			char const *source);

	virtual void device_start() override;

	virtual void tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	tilemap_memory_index adjust_tile_index(tilemap_memory_index tile_index) const
	{ return tile_index + ((unsigned(m_registers[1]) * 256U / gfx().width()) * m_rows); }

	int m_rows;

private:
	required_region_ptr<uint16_t> m_tilerom;
	int m_tilerom_offset;
	unsigned m_transparent_pen;
	unsigned m_primella_code_mask;
	unsigned m_primella_color_mask;
	unsigned m_primella_color_shift;

	uint8_t m_registers[0x10];
};

class rshark_rom_tilemap_device : public dooyong_rom_tilemap_device
{
public:
	rshark_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	static void static_set_colorrom_tag(device_t &device, char const *tag);
	static void static_set_colorrom_offset(device_t &device, int offset);

protected:
	virtual void device_start() override;

	virtual void tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index) override;

private:
	required_region_ptr<uint8_t> m_colorrom;
	int m_colorrom_offset;
};

class dooyong_ram_tilemap_device : public dooyong_tilemap_device_base
{
public:
	dooyong_ram_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	uint16_t tileram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff) { return m_tileram[offset & ((64U * 32U) - 1)]; }
	void tileram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void set_scrolly(int value) { m_tilemap->set_scrolly(value); }

protected:
	virtual void device_start() override;

private:
	void tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	std::unique_ptr<uint16_t[]> m_tileram;
};


class dooyong_state : public driver_device
{
public:
	dooyong_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_bg(*this, "bg")
		, m_bg2(*this, "bg2")
		, m_fg(*this, "fg")
		, m_fg2(*this, "fg2")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<dooyong_rom_tilemap_device> m_bg;
	optional_device<dooyong_rom_tilemap_device> m_bg2;
	optional_device<dooyong_rom_tilemap_device> m_fg;
	optional_device<dooyong_rom_tilemap_device> m_fg2;
};

class dooyong_z80_state : public dooyong_state
{
public:
	dooyong_z80_state(const machine_config &mconfig, device_type type, const char *tag)
		: dooyong_state(mconfig, type, tag)
		, m_tx(*this, "tx")
		, m_spriteram(*this, "spriteram")
	{
	}

	enum
	{
		SPRITE_12BIT = 0x01,
		SPRITE_HEIGHT = 0x02,
		SPRITE_YSHIFT_BLUEHAWK = 0x04,
		SPRITE_YSHIFT_FLYTIGER = 0x08
	};

	void flip_screen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t lastday_tx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void lastday_tx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t bluehawk_tx_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bluehawk_tx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void primella_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t paletteram_flytiger_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void paletteram_flytiger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flytiger_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void get_tx_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, unsigned extensions = 0);
	uint32_t screen_update_bluehawk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_flytiger(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_primella(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void machine_start_cpu_z80();
	void video_start_bluehawk();
	void video_start_flytiger();
	void video_start_primella();

	std::unique_ptr<uint8_t[]> m_paletteram_flytiger;
	uint8_t m_sprites_disabled = 0;
	uint8_t m_flytiger_pri = 0;
	uint8_t m_tx_pri = 0;
	uint8_t m_palette_bank = 0;

	required_device<dooyong_ram_tilemap_device> m_tx;
	optional_device<buffered_spriteram8_device> m_spriteram;
};

class dooyong_z80_ym2203_state : public dooyong_z80_state
{
public:
	dooyong_z80_ym2203_state(const machine_config &mconfig, device_type type, const char *tag)
		: dooyong_z80_state(mconfig, type, tag)
	{
	}

	void lastday_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pollux_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irqhandler_2203_1(int state);
	void irqhandler_2203_2(int state);
	uint8_t unk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void machine_reset_sound_ym2203();
	uint32_t screen_update_lastday(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gulfstrm(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_pollux(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void video_start_lastday();
	void video_start_gulfstrm();
	void video_start_pollux();

	int m_interrupt_line_1 = 0;
	int m_interrupt_line_2 = 0;
};

class dooyong_68k_state : public dooyong_state
{
public:
	dooyong_68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: dooyong_state(mconfig, type, tag)
		, m_spriteram(*this, "spriteram")
	{
	}

	void ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void scanline(timer_device &timer, void *ptr, int32_t param);

protected:
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint16_t m_bg2_priority = 0;
	required_device<buffered_spriteram16_device> m_spriteram;
};

class rshark_state : public dooyong_68k_state
{
public:
	rshark_state(const machine_config &mconfig, device_type type, const char *tag)
		: dooyong_68k_state(mconfig, type, tag)
	{
	}

	uint32_t screen_update_rshark(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void video_start_rshark();
};

class popbingo_state : public dooyong_68k_state
{
public:
	popbingo_state(const machine_config &mconfig, device_type type, const char *tag)
		: dooyong_68k_state(mconfig, type, tag)
		, m_bg_bitmap()
		, m_bg2_bitmap()
		, m_screen(*this, "screen")
	{
	}

	uint32_t screen_update_popbingo(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void video_start_popbingo();

protected:
	bitmap_ind16 m_bg_bitmap;
	bitmap_ind16 m_bg2_bitmap;

	required_device<screen_device> m_screen;
};

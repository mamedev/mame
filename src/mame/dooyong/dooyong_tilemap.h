// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_DOOYONG_DOOYONG_TILEMAP_H
#define MAME_DOOYONG_DOOYONG_TILEMAP_H

#pragma once

#include "video/bufsprite.h"
#include "tilemap.h"

DECLARE_DEVICE_TYPE(DOOYONG_ROM_TILEMAP, dooyong_rom_tilemap_device)
DECLARE_DEVICE_TYPE(RSHARK_ROM_TILEMAP,  rshark_rom_tilemap_device)
DECLARE_DEVICE_TYPE(DOOYONG_RAM_TILEMAP, dooyong_ram_tilemap_device)


class dooyong_tilemap_device_base : public device_t
{
public:
	template <typename T> void set_gfxdecode_tag(T &&gfxdecode_tag) { m_gfxdecode.set_tag(std::forward<T>(gfxdecode_tag)); }
	void set_gfxnum(int gfxnum) { m_gfxnum = gfxnum; }

	void draw(screen_device &screen, bitmap_ind16 &dest, rectangle const &cliprect, u32 flags, u8 priority, u8 priority_mask = 0xff);

	void set_palette_bank(u16 bank);

protected:
	dooyong_tilemap_device_base(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);

	gfx_element const &gfx() const { return *m_gfxdecode->gfx(m_gfxnum); }

	required_device<gfxdecode_device> m_gfxdecode;
	int m_gfxnum;

	tilemap_t *m_tilemap;
	u16 m_palette_bank;
};

class dooyong_rom_tilemap_device : public dooyong_tilemap_device_base
{
public:
	template <typename T, typename U>
	dooyong_rom_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&gfxdecode_tag, int gfxnum, U &&tilerom_tag, int tilerom_offset, int tilerom_length)
		: dooyong_rom_tilemap_device(mconfig, tag, owner, 0)
	{
		set_gfxdecode_tag(std::forward<T>(gfxdecode_tag));
		set_gfxnum(gfxnum);
		set_tilerom_tag(std::forward<U>(tilerom_tag));
		set_tilerom_offset(tilerom_offset);
		set_tilerom_length(tilerom_length);
	}

	dooyong_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename U> void set_tilerom_tag(U &&tilerom_tag) { m_tilerom.set_tag(std::forward<U>(tilerom_tag)); }
	void set_tilerom_offset(int offset) { m_tilerom_offset = offset; }
	void set_tilerom_length(int length) { m_tilerom_length = length; }
	void set_transparent_pen(unsigned pen) { m_transparent_pen = pen; }

	typedef device_delegate<void (u16 attr, u32 &code, u32 &color)> dooyong_tmap_cb_delegate;
	template <typename... T> void set_tile_callback(T &&... args) { m_tmap_cb.set(std::forward<T>(args)...); }

	void ctrl_w(offs_t offset, u8 data);

protected:
	dooyong_rom_tilemap_device(
			machine_config const &mconfig,
			device_type type,
			char const *tag,
			device_t *owner,
			u32 clock);

	virtual void device_start() override ATTR_COLD;

	virtual TILE_GET_INFO_MEMBER(tile_info);

	tilemap_memory_index adjust_tile_index(tilemap_memory_index tile_index) const
	{ return tile_index + ((unsigned(m_registers[1]) * 256U / gfx().width()) * m_rows); }

	int m_rows;

private:
	required_region_ptr<u16> m_tilerom;
	dooyong_tmap_cb_delegate m_tmap_cb;
	int m_tilerom_offset;
	int m_tilerom_length;
	unsigned m_transparent_pen;

	u8 m_registers[0x10];
};

class rshark_rom_tilemap_device : public dooyong_rom_tilemap_device
{
public:
	template <typename T, typename U, typename V>
	rshark_rom_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&gfxdecode_tag, int gfxnum, U &&tilerom_tag, int tilerom_offset, int tilerom_length, V &&colorrom_tag, int colorrom_offset, int colorrom_length)
		: rshark_rom_tilemap_device(mconfig, tag, owner, 0)
	{
		set_gfxdecode_tag(std::forward<T>(gfxdecode_tag));
		set_gfxnum(gfxnum);
		set_tilerom_tag(std::forward<U>(tilerom_tag));
		set_tilerom_offset(tilerom_offset);
		set_tilerom_length(tilerom_length);
		set_colorrom_tag(std::forward<V>(colorrom_tag));
		set_colorrom_offset(colorrom_offset);
		set_colorrom_length(colorrom_length);
	}

	rshark_rom_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	template <typename V> void set_colorrom_tag(V &&colorrom_tag) { m_colorrom.set_tag(std::forward<V>(colorrom_tag)); }
	void set_colorrom_offset(int offset) { m_colorrom_offset = offset; }
	void set_colorrom_length(int length) { m_colorrom_length = length; }

protected:
	virtual void device_start() override ATTR_COLD;

	virtual TILE_GET_INFO_MEMBER(tile_info) override;

private:
	required_region_ptr<u8> m_colorrom;
	int m_colorrom_offset;
	int m_colorrom_length;
};

class dooyong_ram_tilemap_device : public dooyong_tilemap_device_base
{
public:
	template <typename T>
	dooyong_ram_tilemap_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&gfxdecode_tag, int gfxnum)
		: dooyong_ram_tilemap_device(mconfig, tag, owner, 0)
	{
		set_gfxdecode_tag(std::forward<T>(gfxdecode_tag));
		set_gfxnum(gfxnum);
	}

	dooyong_ram_tilemap_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	u16 tileram_r(offs_t offset) { return m_tileram[offset & ((64U * 32U) - 1)]; }
	void tileram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void set_scrolly(int value) { m_tilemap->set_scrolly(value); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	TILE_GET_INFO_MEMBER(tile_info);

	std::unique_ptr<u16[]> m_tileram;
};

#endif // MAME_DOOYONG_DOOYONG_TILEMAP_H

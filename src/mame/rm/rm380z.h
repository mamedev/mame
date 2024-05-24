// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona

/*

Research Machines RM 380Z

*/

#ifndef MAME_RM_RM380Z_H
#define MAME_RM_RM380Z_H

#pragma once

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "machine/keyboard.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"
#include "video/sn74s262.h"

#include "emupal.h"

//
//
//

#define RM380Z_MAINCPU_TAG      "maincpu"

//
//
//

// abstract base class
class rm380z_state : public driver_device
{
protected:
	rm380z_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, RM380Z_MAINCPU_TAG),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_messram(*this, RAM_TAG),
		m_fdc(*this, "wd1771"),
		m_floppy0(*this, "wd1771:0"),
		m_floppy1(*this, "wd1771:1")
	{
	}

	static inline constexpr int RM380Z_SCREENROWS = 24;
	static inline constexpr int RM380Z_SCREENCOLS = 40;

	void base_configure(machine_config &config);
	void fds_configure();

	void machine_reset() override;

	bool ports_enabled_high() const { return bool(m_port0 & 0x80); }
	bool ports_enabled_low() const { return !(m_port0 & 0x80); }

	virtual bool get_rowcol_from_offset(int &row, int &col, offs_t offset) const;

	virtual void port_write(offs_t offset, uint8_t data);
	virtual uint8_t port_read(offs_t offset);
	void port_write_1b00(offs_t offset, uint8_t data);
	uint8_t port_read_1b00(offs_t offset);

	virtual uint8_t videoram_read(offs_t offset) = 0;
	virtual void videoram_write(offs_t offset, uint8_t data) = 0;

	uint8_t rm380z_portlow_r();
	void rm380z_portlow_w(offs_t offset, uint8_t data);
	uint8_t rm380z_porthi_r();
	void rm380z_porthi_w(offs_t offset, uint8_t data);

	void disk_0_control(uint8_t data);

	void keyboard_put(u8 data);

	void config_memory_map();
	virtual void update_screen(bitmap_ind16 &bitmap) const = 0;
	uint32_t screen_update_rm380z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void rm380z_io(address_map &map);
	void rm380z_mem(address_map &map);

	uint8_t m_port0 = 0;
	uint8_t m_port0_kbd = 0;
	uint8_t m_port1 = 0;
	uint8_t m_fbfe = 0;

	required_device<cpu_device> m_maincpu;
	optional_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_device<ram_device> m_messram;
	optional_device<fd1771_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;
};

// COS 3.4 machine with cassette and VDU-40 display
class rm380z_state_cos34 : public rm380z_state
{
public:
	rm380z_state_cos34(const machine_config &mconfig, device_type type, const char *tag) :
		rm380z_state(mconfig, type, tag),
		m_rocg(*this, "sn74s262"),
		m_cassette(*this, "cassette")
	{
	}

	void rm380z34e(machine_config &config);
	void rm380z34d(machine_config &config) { rm380z34e(config); fds_configure(); }

protected:
	void machine_reset() override;

	void port_write(offs_t offset, uint8_t data) override;
	uint8_t port_read(offs_t offset) override;
	void update_screen(bitmap_ind16 &bitmap) const override;
	uint8_t videoram_read(offs_t offset) override;
	void videoram_write(offs_t offset, uint8_t data) override;

private:
	template <int ROWS, int COLS>
	class rm380z_vram
	{
	public:
		void set_char(int row, int col, uint8_t data) { m_chars[row][col] = data; }
		void reset() { memset(m_chars, 0x80, sizeof(m_chars)); }

		uint8_t get_char(int row, int col) const { return m_chars[row][col]; }

	private:
		uint8_t m_chars[ROWS][COLS];
	};

	void putChar_vdu40(int charnum, int x, int y, bitmap_ind16 &bitmap) const;

	rm380z_vram<RM380Z_SCREENROWS, RM380Z_SCREENCOLS> m_vram;

	required_device<sn74s262_device> m_rocg;
	required_device<cassette_image_device> m_cassette;
};

// COS 4.0 machine with VDU-80 display
class rm380z_state_cos40 : public rm380z_state
{
public:
	rm380z_state_cos40(const machine_config &mconfig, device_type type, const char *tag) :
		rm380z_state(mconfig, type, tag),
		m_chargen(*this, "chargen"),
		m_speaker(*this, "speaker")
	{
	}

	void rm380z(machine_config &config);
	void rm380zf(machine_config &config) { rm380z(config); fds_configure(); }

protected:
	template <int ROWS, int COLS>
	class rm380z_vram
	{
	public:
		void set_char(int row, int col, uint8_t data) { m_chars[get_row(row)][col] = data; }
		void set_attrib(int row, int col, uint8_t data) { m_attribs[get_row(row)][col] = data; }
		void set_scroll_register(uint8_t value) { m_scroll_reg = value; }
		void reset() { memset(m_attribs, 0, sizeof(m_attribs)); memset(m_chars, 0x80, sizeof(m_chars)); }

		uint8_t get_char(int row, int col) const { return m_chars[get_row(row)][col]; }
		uint8_t get_attrib(int row, int col) const { return m_attribs[get_row(row)][col]; }

	private:
		int get_row(int row) const { return (row + m_scroll_reg) % ROWS; }

		uint8_t m_chars[ROWS][COLS];
		uint8_t m_attribs[ROWS][COLS];
		uint8_t m_scroll_reg = 0;
	};

	static inline constexpr int RM380Z_SCREENCOLS = 80;
	static inline constexpr int RM380Z_VIDEOMODE_40COL = 0x01;
	static inline constexpr int RM380Z_VIDEOMODE_80COL = 0x02;

	void machine_reset() override;

	void port_write(offs_t offset, uint8_t data) override;
	uint8_t port_read(offs_t offset) override;
	void update_screen(bitmap_ind16 &bitmap) const override;
	uint8_t videoram_read(offs_t offset) override;
	void videoram_write(offs_t offset, uint8_t data) override;

	int m_videomode = RM380Z_VIDEOMODE_80COL;
	rm380z_vram<RM380Z_SCREENROWS, RM380Z_SCREENCOLS> m_vram;

	uint8_t m_fbfd = 0;

	required_region_ptr<u8> m_chargen;
	optional_device<speaker_sound_device> m_speaker;

private:
	void config_videomode();
	void putChar_vdu80(int charnum, int attribs, int x, int y, bitmap_ind16 &bitmap) const;
	bool get_rowcol_from_offset(int &row, int &col, offs_t offset) const override;

	uint8_t m_character_row = 0;
	uint8_t m_character = 0;

	uint8_t m_user_defined_chars[2048];
};

// COS 4.0 machine with VDU-80 display, HRG and colour
class rm380z_state_cos40_hrg : public rm380z_state_cos40
{
public:
	rm380z_state_cos40_hrg(const machine_config &mconfig, device_type type, const char *tag) :
		rm380z_state_cos40(mconfig, type, tag),
		m_io_display_type(*this, "display_type")
	{
	}

	void rm380zhrg(machine_config &config);
	void rm380zfhrg(machine_config &config) { rm380zhrg(config); fds_configure(); }

	DECLARE_INPUT_CHANGED_MEMBER(monitor_changed);

protected:
	void machine_reset() override;

	void port_write(offs_t offset, uint8_t data) override;
	uint8_t port_read(offs_t offset) override;
	void update_screen(bitmap_ind16 &bitmap) const override;
	uint8_t videoram_read(offs_t offset) override;
	void videoram_write(offs_t offset, uint8_t data) override;

private:
	enum class hrg_display_mode : uint8_t
	{
		none = 0,
		high = 1,
		medium_0 = 2,
		medium_1 = 3
	};

	static inline constexpr int RM380Z_HRG_RAM_SIZE = 16384; // 16k
	static inline constexpr int RM380Z_HRG_SCRATCHPAD_SIZE = 16;

	void palette_init(palette_device &palette);
	void change_hrg_scratchpad(int index, uint8_t value, uint8_t mask);
	void change_palette(int index, uint8_t value);
	int calculate_hrg_vram_index(offs_t offset) const;
	void draw_high_res_graphics(bitmap_ind16 &bitmap) const;
	void draw_medium_res_graphics(bitmap_ind16 &bitmap) const;

	uint8_t m_hrg_ram[RM380Z_HRG_RAM_SIZE];
	uint8_t m_hrg_scratchpad[RM380Z_HRG_SCRATCHPAD_SIZE];

	uint8_t m_hrg_port0 = 0;
	uint8_t m_hrg_port1 = 0;
	hrg_display_mode m_hrg_display_mode = hrg_display_mode::none;

	required_ioport m_io_display_type;
};

// partially implemented non working RM480z
class rm480z_state : public rm380z_state_cos40
{
public:
	rm480z_state(const machine_config &mconfig, device_type type, const char *tag) :
		rm380z_state_cos40(mconfig, type, tag)
	{
	}

	void rm480z(machine_config &config);
	void rm480za(machine_config &config) { rm480z(config); }

protected:
	void machine_reset() override;

	void update_screen(bitmap_ind16 &bitmap) const override;

private:
	void rm480z_io(address_map &map);
	void rm480z_mem(address_map &map);
};

#endif // MAME_RM_RM380Z_H

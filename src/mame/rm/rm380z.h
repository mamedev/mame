// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona

/*

Research Machines RM 380Z

*/

#ifndef MAME_INCLUDES_RM380Z_H
#define MAME_INCLUDES_RM380Z_H

#pragma once

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/ram.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "machine/keyboard.h"

//
//
//

#define RM380Z_MAINCPU_TAG      "maincpu"
#define RM380Z_PORTS_ENABLED_HIGH   ( m_port0 & 0x80 )
#define RM380Z_PORTS_ENABLED_LOW    ( ( m_port0 & 0x80 ) == 0x00 )

#define RM380Z_VIDEOMODE_40COL  0x01
#define RM380Z_VIDEOMODE_80COL  0x02

#define RM380Z_CHDIMX 5
#define RM380Z_CHDIMY 9
#define RM380Z_NCX 8
#define RM380Z_NCY 16
#define RM380Z_SCREENCOLS 80
#define RM380Z_SCREENROWS 24

#define RM380Z_VIDEORAM_SIZE 0x600
#define RM380Z_SCREENSIZE 0x1200


//
//
//


class rm380z_state : public driver_device
{
public:
	rm380z_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, RM380Z_MAINCPU_TAG),
		m_cassette(*this, "cassette"),
		m_messram(*this, RAM_TAG),
		m_fdc(*this, "wd1771"),
		m_floppy0(*this, "wd1771:0"),
		m_floppy1(*this, "wd1771:1")
	{
	}

	void rm480z(machine_config &config);
	void rm380z(machine_config &config);

	void init_rm380z();
	void init_rm380z34d();
	void init_rm380z34e();
	void init_rm480z();

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	void put_point(int charnum,int x,int y,int col);
	void init_graphic_chars();

	void putChar(int charnum,int attribs,int x,int y,bitmap_ind16 &bitmap,unsigned char* chsb,int vmode);
	void decode_videoram_char(int pos,uint8_t& chr,uint8_t& attrib);
	void scroll_videoram();
	void config_videomode();
	void check_scroll_register();

	int writenum = 0;

	uint8_t m_port0 = 0;
	uint8_t m_port0_mask = 0;
	uint8_t m_port0_kbd = 0;
	uint8_t m_port1 = 0;
	uint8_t m_fbfd = 0;
	uint8_t m_fbfe = 0;

	uint8_t m_graphic_chars[0x80][(RM380Z_CHDIMX+1)*(RM380Z_CHDIMY+1)];

	uint8_t   m_mainVideoram[RM380Z_VIDEORAM_SIZE];
	uint8_t   m_vramchars[RM380Z_SCREENSIZE];
	uint8_t   m_vramattribs[RM380Z_SCREENSIZE];
	uint8_t   m_vram[RM380Z_SCREENSIZE];

	int m_rasterlineCtr = 0;
	emu_timer* m_vblankTimer = nullptr;

	int m_old_fbfd = 0;
	int m_old_old_fbfd = 0;

	int m_videomode = 0;
	int m_old_videomode = 0;

	emu_timer *m_static_vblank_timer = nullptr;

	required_device<cpu_device> m_maincpu;
	optional_device<cassette_image_device> m_cassette;
	optional_device<ram_device> m_messram;
	optional_device<fd1771_device> m_fdc;
	optional_device<floppy_connector> m_floppy0;
	optional_device<floppy_connector> m_floppy1;

	void port_write(offs_t offset, uint8_t data);
	uint8_t port_read(offs_t offset);
	void port_write_1b00(offs_t offset, uint8_t data);
	uint8_t port_read_1b00(offs_t offset);

	uint8_t videoram_read(offs_t offset);
	void videoram_write(offs_t offset, uint8_t data);

	uint8_t rm380z_portlow_r();
	void rm380z_portlow_w(offs_t offset, uint8_t data);
	uint8_t rm380z_porthi_r();
	void rm380z_porthi_w(offs_t offset, uint8_t data);

	void disk_0_control(uint8_t data);

	void keyboard_put(u8 data);

	DECLARE_MACHINE_RESET(rm480z);

	void config_memory_map();
	void update_screen(bitmap_ind16 &bitmap);
	uint32_t screen_update_rm380z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rm480z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(static_vblank_timer);

	void rm380z_io(address_map &map);
	void rm380z_mem(address_map &map);
	void rm480z_io(address_map &map);
	void rm480z_mem(address_map &map);
};

#endif // MAME_INCLUDES_RM380Z_H

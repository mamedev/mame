// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona

/*

Research Machines RM 380Z

*/

#ifndef RM380Z_H_
#define RM380Z_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "imagedev/flopdrv.h"
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
private:

	void put_point(int charnum,int x,int y,int col);
	void init_graphic_chars();

	void putChar(int charnum,int attribs,int x,int y,bitmap_ind16 &bitmap,unsigned char* chsb,int vmode);
	void decode_videoram_char(int pos,UINT8& chr,UINT8& attrib);
	void scroll_videoram();
	void config_videomode();
	void check_scroll_register();

	int writenum;

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

public:

	UINT8 m_port0;
	UINT8 m_port0_kbd;
	UINT8 m_port1;
	UINT8 m_fbfd;
	UINT8 m_fbfe;

	UINT8 m_graphic_chars[0x80][(RM380Z_CHDIMX+1)*(RM380Z_CHDIMY+1)];

	UINT8   m_mainVideoram[RM380Z_VIDEORAM_SIZE];
	UINT8   m_vramchars[RM380Z_SCREENSIZE];
	UINT8   m_vramattribs[RM380Z_SCREENSIZE];
	UINT8   m_vram[RM380Z_SCREENSIZE];

	int m_rasterlineCtr;
	emu_timer* m_vblankTimer;

	int m_old_fbfd;
	int m_old_old_fbfd;

	int m_videomode;
	int m_old_videomode;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_messram;
	required_device<fd1771_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;

	rm380z_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, RM380Z_MAINCPU_TAG),
		m_messram(*this, RAM_TAG),
		m_fdc(*this, "wd1771"),
		m_floppy0(*this, "wd1771:0"),
		m_floppy1(*this, "wd1771:1")
	{
	}

	DECLARE_WRITE8_MEMBER( port_write );
	DECLARE_READ8_MEMBER( port_read );
	DECLARE_WRITE8_MEMBER( port_write_1b00 );
	DECLARE_READ8_MEMBER( port_read_1b00 );

	DECLARE_READ8_MEMBER( videoram_read );
	DECLARE_WRITE8_MEMBER( videoram_write );

	UINT8 hiram[0x1000];
	DECLARE_READ8_MEMBER( hiram_read );
	DECLARE_WRITE8_MEMBER( hiram_write );

	DECLARE_READ8_MEMBER( rm380z_portlow_r );
	DECLARE_WRITE8_MEMBER( rm380z_portlow_w );
	DECLARE_READ8_MEMBER( rm380z_porthi_r );
	DECLARE_WRITE8_MEMBER( rm380z_porthi_w );

	DECLARE_WRITE8_MEMBER(disk_0_control);

	DECLARE_WRITE8_MEMBER( keyboard_put );

	void config_memory_map();
	void update_screen(bitmap_ind16 &bitmap);
	UINT32 screen_update_rm380z(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(static_vblank_timer);
};


#endif

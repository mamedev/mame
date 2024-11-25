// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Sega Visual Memory Unit

        driver by Sandro Ronco

        TODO:
        - add more bios versions
        - serial

****************************************************************************/

#include "emu.h"
#include "cpu/lc8670/lc8670.h"
#include "imagedev/snapquik.h"
#include "machine/intelfsh.h"
#include "sound/spkrdev.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "svmu.lh"


namespace {

#define     PIXEL_SIZE          7
#define     PIXEL_DISTANCE      1

class svmu_state : public driver_device
{
public:
	svmu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_flash(*this, "flash")
		, m_speaker(*this, "speaker")
		, m_bios(*this, "bios")
		, m_battery(*this, "BATTERY")
		, m_file_icon(*this, "file_icon")
		, m_game_icon(*this, "game_icon")
		, m_clock_icon(*this, "clock_icon")
		, m_flash_icon(*this, "flash_icon")
	{ }

	void svmu(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	LC8670_LCD_UPDATE(svmu_lcd_update);
	void svmu_palette(palette_device &palette) const;
	void page_w(uint8_t data);
	uint8_t prog_r(offs_t offset);
	void prog_w(offs_t offset, uint8_t data);
	uint8_t p1_r();
	void p1_w(uint8_t data);
	uint8_t p7_r();
	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

	void svmu_io_mem(address_map &map) ATTR_COLD;
	void svmu_mem(address_map &map) ATTR_COLD;

	required_device<lc8670_cpu_device> m_maincpu;
	required_device<intelfsh8_device> m_flash;
	required_device<speaker_sound_device> m_speaker;
	required_region_ptr<uint8_t> m_bios;
	required_ioport m_battery;
	output_finder<> m_file_icon;
	output_finder<> m_game_icon;
	output_finder<> m_clock_icon;
	output_finder<> m_flash_icon;

	uint8_t       m_page;
};


void svmu_state::page_w(uint8_t data)
{
	m_page = data & 0x03;
}

uint8_t svmu_state::prog_r(offs_t offset)
{
	if (m_page == 1)
		return m_flash->read(offset);
	else if (m_page == 2)
		return m_flash->read(0x10000 + offset);
	else
		return m_bios[offset];
}

void svmu_state::prog_w(offs_t offset, uint8_t data)
{
	if (m_page == 1)
		m_flash->write(offset, data);
	else if (m_page == 2)
		m_flash->write(0x10000 + offset, data);
}

/*
    Port 1

    x--- ----   PWM output
    -x-- ----   BUZ
    --x- ----   SCK1
    ---x ----   SB1
    ---- x---   SO1
    ---- -x--   SCK0
    ---- --x-   SB0
    ---- ---x   SO0

*/

uint8_t svmu_state::p1_r()
{
	return 0;
}

void svmu_state::p1_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 7));
}


/*
    Port 7

    ---- x---   ID1
    ---- -x--   ID0
    ---- --x-   battery low voltage
    ---- ---x   5V detection
*/

uint8_t svmu_state::p7_r()
{
	return (m_battery->read()<<1);
}


void svmu_state::svmu_mem(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(svmu_state::prog_r), FUNC(svmu_state::prog_w));
}

void svmu_state::svmu_io_mem(address_map &map)
{
	map(LC8670_PORT1, LC8670_PORT1).rw(FUNC(svmu_state::p1_r), FUNC(svmu_state::p1_w));
	map(LC8670_PORT3, LC8670_PORT3).portr("P3");
	map(LC8670_PORT7, LC8670_PORT7).r(FUNC(svmu_state::p7_r));
}

/* Input ports */
static INPUT_PORTS_START( svmu )
	PORT_START( "P3" )
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_NAME("Up")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_NAME("Down")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_NAME("Left")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_NAME("Right")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Button A")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Button B")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("MODE")   PORT_CODE( KEYCODE_M )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("SLEEP")  PORT_CODE( KEYCODE_S )
	PORT_START("BATTERY")
	PORT_CONFNAME( 0x01, 0x01, "Battery" )
	PORT_CONFSETTING( 0x01, "Good" )
	PORT_CONFSETTING( 0x00, "Poor" )
INPUT_PORTS_END

void svmu_state::machine_start()
{
	m_file_icon.resolve();
	m_game_icon.resolve();
	m_clock_icon.resolve();
	m_flash_icon.resolve();
}

void svmu_state::machine_reset()
{
	m_page = 0;
}

void svmu_state::svmu_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

LC8670_LCD_UPDATE(svmu_state::svmu_lcd_update)
{
	if (lcd_enabled)
	{
		for (int y=0; y<32; y++)
			for (int x=0; x<6; x++)
			{
				int gfx = vram[y*6 + x];

				for (int b = 0; b < 8; b++)
					bitmap.plot_box((x*8 + b) * (PIXEL_SIZE + PIXEL_DISTANCE), y * (PIXEL_SIZE + PIXEL_DISTANCE), PIXEL_SIZE, PIXEL_SIZE, BIT(gfx,7-b));
			}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	m_file_icon  = lcd_enabled ? BIT(vram[0xc1],6) : 0;
	m_game_icon  = lcd_enabled ? BIT(vram[0xc2],4) : 0;
	m_clock_icon = lcd_enabled ? BIT(vram[0xc3],2) : 0;
	m_flash_icon = lcd_enabled ? BIT(vram[0xc4],0) : 0;

	return 0;
}


inline void vmufat_write_byte(uint8_t* flash, uint8_t block, offs_t offset, uint8_t data)
{
	flash[(block * 512) + offset] = data;
}

inline void vmufat_write_word(uint8_t* flash, uint8_t block, offs_t offset, uint16_t data)
{
	// 16-bit data are stored in little endian
	flash[(block * 512) + offset + 0] = data & 0xff;
	flash[(block * 512) + offset + 1] = (data>>8) & 0xff;
}

QUICKLOAD_LOAD_MEMBER(svmu_state::quickload_cb)
{
	uint32_t size = image.length();
	uint8_t *flash = m_flash->base();

	image.fread(flash, size);

	// verify if image is already a valid VMUFAT file system
	bool valid_vmufat = true;
	if (size == 0x20000)
	{
		for (int i=0; i<0x10; i++)
			if (flash[255 * 512 + i] != 0x55)
			{
				valid_vmufat = false;
				break;
			}
	}
	else
	{
		valid_vmufat = false;
	}

	if (!valid_vmufat)
	{
		// more info about the VMUFAT here: http://mc.pp.se/dc/vms/flashmem.html

		//-------------------------------- Formatting --------------------------------
		memset(flash + 241*512, 0, 15*512);                 // clears the last 15 blocks that contain file system information

		for (int i=0; i<0x10; i++)
			vmufat_write_byte(flash, 255, i, 0x55);         // first 16 bytes should be 0x55 to indicate a properly formatted card

		vmufat_write_byte(flash, 255, 0x10, 0x00);          // custom VMS colour (1 = use custom colours, 0 = standard colour)
		vmufat_write_byte(flash, 255, 0x11, 0x00);          // VMS colour blue component
		vmufat_write_byte(flash, 255, 0x12, 0x00);          // VMS colour green component
		vmufat_write_byte(flash, 255, 0x13, 0x00);          // VMS colour red component
		vmufat_write_byte(flash, 255, 0x14, 0x00);          // VMS colour alpha component
		vmufat_write_byte(flash, 255, 0x30, 0x19);          // Century (BCD)
		vmufat_write_byte(flash, 255, 0x31, 0x99);          // Year (BCD)
		vmufat_write_byte(flash, 255, 0x32, 0x01);          // Month (BCD)
		vmufat_write_byte(flash, 255, 0x33, 0x01);          // Day (BCD)
		vmufat_write_byte(flash, 255, 0x34, 0x00);          // Hour (BCD)
		vmufat_write_byte(flash, 255, 0x35, 0x00);          // Minute (BCD)
		vmufat_write_byte(flash, 255, 0x36, 0x00);          // Second (BCD)
		vmufat_write_byte(flash, 255, 0x37, 0x00);          // Day of week (0 = Monday, 6 = Sunday)
		vmufat_write_word(flash, 255, 0x44, 0x00ff);        // location of Root
		vmufat_write_word(flash, 255, 0x46, 0x00fe);        // location of FAT (254)
		vmufat_write_word(flash, 255, 0x48, 0x0001);        // size of FAT in blocks (1)
		vmufat_write_word(flash, 255, 0x4a, 0x00fd);        // location of Directory (253)
		vmufat_write_word(flash, 255, 0x4c, 0x000d);        // size of Directory in blocks (13)
		vmufat_write_word(flash, 255, 0x4e, 0x0000);        // icon shape for this VMS (0-123)
		vmufat_write_word(flash, 255, 0x50, 0x00c8);        // number of user blocks (200)

		for (int i=0; i<256; i++)
			vmufat_write_word(flash, 254, i<<1, 0xfffc);    // marks all blocks as unallocated

		for (int i=253; i>241; --i)
			vmufat_write_word(flash, 254, i<<1, i - 1);     // marsk all Directory blocks as allocate

		vmufat_write_word(flash, 254, 0x1e2, 0xfffa);       // marks last Directory block
		vmufat_write_word(flash, 254, 0x1fc, 0xfffa);       // marks FAT block as allocated
		vmufat_write_word(flash, 254, 0x1fe, 0xfffa);       // marks Root block as allocated

		//-------------------------------- Create the vms file --------------------------------
		int vms_blocks = (size / 512) + (size & 0x1ff ? 1 : 0); // number of blocks required for store the vms file

		for (int i=0; i<vms_blocks - 1; i++)
			vmufat_write_word(flash, 254, i<<1, i + 1);     // marks blocks where the file is allocated

		vmufat_write_word(flash, 254, (vms_blocks-1)<<1, 0xfffa);   // last block for this file

		vmufat_write_byte(flash, 253, 0x00, 0xcc);          // file type (0x00 = no file, 0x33 = data, 0xcc = game)
		vmufat_write_byte(flash, 253, 0x01, 0x00);          // copy protect (0x00 = no, 0xff = yes)
		vmufat_write_word(flash, 253, 0x02, 0x0000);        // location of first file block

		const char *vms_filename = image.basename_noext();
		for (int i=0; i<12; i++)
		{
			if (i < strlen(vms_filename))
				vmufat_write_byte(flash, 253, i + 4, vms_filename[i]);  // 12 bytes filename
			else
				vmufat_write_byte(flash, 253, i + 4, 0x20);             // space padded
		}

		vmufat_write_byte(flash, 253, 0x10, 0x19);          // Century (BCD)
		vmufat_write_byte(flash, 253, 0x11, 0x99);          // Year (BCD)
		vmufat_write_byte(flash, 253, 0x12, 0x01);          // Month (BCD)
		vmufat_write_byte(flash, 253, 0x13, 0x01);          // Day (BCD)
		vmufat_write_byte(flash, 253, 0x14, 0x00);          // Hour (BCD)
		vmufat_write_byte(flash, 253, 0x15, 0x00);          // Minute (BCD)
		vmufat_write_byte(flash, 253, 0x16, 0x00);          // Second (BCD)
		vmufat_write_byte(flash, 253, 0x17, 0x00);          // Day of week (0 = Monday, 6 = Sunday)
		vmufat_write_word(flash, 253, 0x18, vms_blocks);    // file size (in blocks)
		vmufat_write_word(flash, 253, 0x1a, 0x0001);        // offset of header (in blocks) from file start
	}

	return std::make_pair(std::error_condition(), std::string());
}


void svmu_state::svmu(machine_config &config)
{
	/* basic machine hardware */
	LC8670(config, m_maincpu, XTAL(32'768));
	m_maincpu->set_addrmap(AS_PROGRAM, &svmu_state::svmu_mem);
	m_maincpu->set_addrmap(AS_IO, &svmu_state::svmu_io_mem);

	/* specific LC8670 configurations */
	m_maincpu->set_clock_sources(XTAL(32'768), 600000, XTAL(6'000'000)); // tolerance range of the RC oscillator is 600kHz to 1200kHz
	m_maincpu->bank_cb().set(FUNC(svmu_state::page_w));
	m_maincpu->set_lcd_update_cb(FUNC(svmu_state::svmu_lcd_update));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	screen.set_size(48 * (PIXEL_SIZE + PIXEL_DISTANCE), 32 * (PIXEL_SIZE + PIXEL_DISTANCE));
	screen.set_visarea(0, 48*(PIXEL_SIZE + PIXEL_DISTANCE) - 1, 0, 32*(PIXEL_SIZE + PIXEL_DISTANCE) - 1);
	screen.set_screen_update("maincpu", FUNC(lc8670_cpu_device::screen_update));
	screen.set_palette("palette");

	config.set_default_layout(layout_svmu);
	PALETTE(config, "palette", FUNC(svmu_state::svmu_palette), 2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* devices */
	ATMEL_29C010(config, m_flash);

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "vms,bin"));
	quickload.set_load_callback(FUNC(svmu_state::quickload_cb));
	quickload.set_interface("svmu_quik");

	/* Software lists */
	SOFTWARE_LIST(config, "quik_list").set_original("svmu");
}


/* ROM definition */
ROM_START( svmu )
	// some ROMs come from the Sega Katana SDK and are scrambled, a simple way to restore them is to remove the first 4 bytes and xor the whole file for the xor key.

	ROM_REGION(0x10000, "bios", 0)
	ROM_DEFAULT_BIOS("en1005b")

	// Version 1.005,1999/04/28,315-6124-07,SEGA Visual Memory System BIOS Produced by Sue
	ROM_SYSTEM_BIOS(0, "en1005a", "VMS English BIOS (1.005 1999/04/28)")
	ROMX_LOAD("en1005-19990428-315-6124-07.bin", 0x0000, 0x10000, CRC(dfd77f4e) SHA1(4a7bfd1b8eb599d87883312df0bb48e0edd13034), ROM_BIOS(0)) // extracted with trojan

	// Version 1.005,1999/10/26,315-6208-05,SEGA Visual Memory System BIOS Produced by Sue
	ROM_SYSTEM_BIOS(1, "en1005b", "VMS English BIOS (1.005 1999/10/26)")
	ROMX_LOAD("en1005-19991026-315-6208-05.bin", 0x0000, 0x10000, CRC(c825003a) SHA1(6242320d705c156f8369969d6caa8c737f01e4f3), ROM_BIOS(1)) // extracted with trojan

	// Version 1.001,1998/05/28,315-6124-02,SEGA Visual Memory System BIOS Produced by Sue
	ROM_SYSTEM_BIOS(2, "jp1001", "VMS Japanese BIOS (1.001 1998/05/28)")
	ROMX_LOAD("jp1001-19980528-315-6124-02.bin", 0x0000, 0x10000, CRC(e6339f4a) SHA1(688b2e1ff8c60bde6e8b07a2d2695cdacc07bd0c), ROM_BIOS(2))

	// Version 1.002,1998/06/04,315-6124-03,SEGA Visual Memory System BIOS Produced by Sue
	ROM_SYSTEM_BIOS(3, "jp1002", "VMS Japanese BIOS (1.002 1998/06/04)")
	ROMX_LOAD("jp1002-19980604-315-6124-03.bin", 0x0000, 0x10000, CRC(6c020d48) SHA1(9ee7c87d7b033235e0b315a0b421e70deb547c7a), ROM_BIOS(3))

	// Version 1.004,1998/09/30,315-6208-01,SEGA Visual Memory System BIOS Produced by Sue
	ROM_SYSTEM_BIOS(4, "jp1004", "VMS Japanese BIOS (1.004, 1998/09/30)")
	ROMX_LOAD("jp1004-19980930-315-6208-01.bin", 0x0000, 0x10000, CRC(8e0f867a) SHA1(dc2fa2963138a1049a43f7f36439ad0a416ee8b4), ROM_BIOS(4)) // from Sega Katana SDK (original file: fbios.sbf, CRC: c7c77b3c, xor key: 0x37)

	// Version 1.005,1998/12/09,315-6124-05,SEGA Visual Memory System BIOS Produced by Sue
	ROM_SYSTEM_BIOS(5, "jp1005a", "VMS Japanese BIOS (1.005 1998/12/09)")
	ROMX_LOAD("jp1005-19981209-315-6124-05.bin", 0x0000, 0x10000, CRC(47623324) SHA1(fca1aceff8a2f8c6826f3a865f4d5ef88dfd9ed1), ROM_BIOS(5))

	// Version 1.005,1999/10/26,315-6208-04,SEGA Visual Memory System BIOS Produced by Sue
	ROM_SYSTEM_BIOS(6, "jp1005b", "VMS Japanese BIOS (1.005 1999/10/26)")
	ROMX_LOAD("jp1005-19991026-315-6208-04.bin", 0x0000, 0x10000, CRC(6cab02c2) SHA1(6cc2fbf4a67770988922117c300d006aa20899ac), ROM_BIOS(6)) // extracted with trojan

	// Version 1.004,1998/09/30,315-6208-01,SEGA Visual Memory System BIOS Produced by Sue
	ROM_SYSTEM_BIOS(7, "dev1004", "VMS Japanese Development BIOS (1.004 1998/09/30)") // automatically boot the first game found in the flash
	ROMX_LOAD( "jp1004-19980930-315-6208-01-dev.bin", 0x0000, 0x10000, CRC(395e25f2) SHA1(37dea034322b5b80b35b2de784298d32c71ba7a3), ROM_BIOS(7)) // from Sega Katana SDK (original file: qbios.sbf, CRC: eed5524c, xor key: 0x43)
ROM_END

} // anonymous namespace


/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE       INIT        COMPANY  FULLNAME              FLAGS */
COMP( 1998, svmu, 0,      0,      svmu,    svmu,  svmu_state, empty_init, "Sega",  "Visual Memory Unit", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

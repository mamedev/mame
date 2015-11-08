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
#include "sound/speaker.h"
#include "softlist.h"
#include "svmu.lh"

#define     PIXEL_SIZE          7
#define     PIXEL_DISTANCE      1

class svmu_state : public driver_device
{
public:
	svmu_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_flash(*this, "flash"),
			m_speaker(*this, "speaker"),
			m_bios(*this, "bios")
		{ }

	required_device<lc8670_cpu_device> m_maincpu;
	required_device<intelfsh8_device> m_flash;
	required_device<speaker_sound_device> m_speaker;
	required_region_ptr<UINT8> m_bios;

	DECLARE_PALETTE_INIT(svmu);
	virtual void machine_reset();

	DECLARE_WRITE8_MEMBER(page_w);
	DECLARE_READ8_MEMBER(prog_r);
	DECLARE_WRITE8_MEMBER(prog_w);
	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_READ8_MEMBER(p7_r);
	DECLARE_QUICKLOAD_LOAD_MEMBER( svmu );

private:
	UINT8       m_page;
};


WRITE8_MEMBER(svmu_state::page_w)
{
	m_page = data & 0x03;
}

READ8_MEMBER(svmu_state::prog_r)
{
	if (m_page == 1)
		return m_flash->read(offset);
	else if (m_page == 2)
		return m_flash->read(0x10000 + offset);
	else
		return m_bios[offset];
}

WRITE8_MEMBER(svmu_state::prog_w)
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

READ8_MEMBER(svmu_state::p1_r)
{
	return 0;
}

WRITE8_MEMBER(svmu_state::p1_w)
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

READ8_MEMBER(svmu_state::p7_r)
{
	return (ioport("BATTERY")->read()<<1);
}


static ADDRESS_MAP_START(svmu_mem, AS_PROGRAM, 8, svmu_state)
	AM_RANGE( 0x0000, 0xffff ) AM_READWRITE(prog_r, prog_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START(svmu_io_mem, AS_IO, 8, svmu_state)
	AM_RANGE( LC8670_PORT1, LC8670_PORT1 ) AM_READWRITE(p1_r, p1_w)
	AM_RANGE( LC8670_PORT3, LC8670_PORT3 ) AM_READ_PORT("P3")
	AM_RANGE( LC8670_PORT7, LC8670_PORT7 ) AM_READ(p7_r)
ADDRESS_MAP_END

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

void svmu_state::machine_reset()
{
	m_page = 0;
}

PALETTE_INIT_MEMBER(svmu_state, svmu)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

static LC8670_LCD_UPDATE( svmu_lcd_update )
{
	if (lcd_enabled)
	{
		for (int y=0; y<32; y++)
			for (int x=0; x<6; x++)
			{
				int gfx = vram[y*6 + x];

				for (int b=0; b<8; b++)
					bitmap.plot_box((x*8 + b) * (PIXEL_SIZE + PIXEL_DISTANCE), y * (PIXEL_SIZE + PIXEL_DISTANCE), PIXEL_SIZE, PIXEL_SIZE, BIT(gfx,7-b));
			}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}

	output_set_value("file_icon" , lcd_enabled ? BIT(vram[0xc1],6) : 0);
	output_set_value("game_icon" , lcd_enabled ? BIT(vram[0xc2],4) : 0);
	output_set_value("clock_icon", lcd_enabled ? BIT(vram[0xc3],2) : 0);
	output_set_value("flash_icon", lcd_enabled ? BIT(vram[0xc4],0) : 0);

	return 0;
}


inline void vmufat_write_byte(UINT8* flash, UINT8 block, offs_t offset, UINT8 data)
{
	flash[(block * 512) + offset] = data;
}

inline void vmufat_write_word(UINT8* flash, UINT8 block, offs_t offset, UINT16 data)
{
	// 16-bit data are stored in little endian
	flash[(block * 512) + offset + 0] = data & 0xff;
	flash[(block * 512) + offset + 1] = (data>>8) & 0xff;
}

QUICKLOAD_LOAD_MEMBER( svmu_state, svmu )
{
	UINT32 size = image.length();
	UINT8 *flash = (UINT8*)m_flash->space().get_read_ptr(0);

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

	return IMAGE_INIT_PASS;
}


static MACHINE_CONFIG_START( svmu, svmu_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", LC8670, XTAL_32_768kHz)
	MCFG_CPU_PROGRAM_MAP(svmu_mem)
	MCFG_CPU_IO_MAP(svmu_io_mem)

	/* specific LC8670 configurations */
	MCFG_LC8670_SET_CLOCK_SOURCES(XTAL_32_768kHz, 600000, XTAL_6MHz)    // tolerance range of the RC oscillator is 600kHz to 1200kHz
	MCFG_LC8670_BANKSWITCH_CB(WRITE8(svmu_state, page_w))
	MCFG_LC8670_LCD_UPDATE_CB(svmu_lcd_update)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) // not accurate
	MCFG_SCREEN_SIZE(48 * (PIXEL_SIZE + PIXEL_DISTANCE), 32 * (PIXEL_SIZE + PIXEL_DISTANCE))
	MCFG_SCREEN_VISIBLE_AREA(0, 48*(PIXEL_SIZE + PIXEL_DISTANCE) - 1, 0, 32*(PIXEL_SIZE + PIXEL_DISTANCE) - 1)
	MCFG_SCREEN_UPDATE_DEVICE("maincpu", lc8670_cpu_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_svmu)
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(svmu_state, svmu)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_ATMEL_29C010_ADD("flash")
	MCFG_QUICKLOAD_ADD("quickload", svmu_state, svmu, "vms,bin", 0)
	MCFG_QUICKLOAD_INTERFACE("svmu_quik")

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("quik_list", "svmu")
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( svmu )
	ROM_REGION( 0x10000, "bios", 0 )
	ROM_DEFAULT_BIOS("jp1004")
	// these ROMs come from the Sega Katana SDK and are scrambled, a simple way to restore it is to remove the first 4 bytes and xor the whole file for the xor key.
	ROM_SYSTEM_BIOS(0, "jp1001", "VMS Japanese BIOS (v1.001)")
	ROMX_LOAD("vmu1001.bin", 0x0000, 0x10000, CRC(e6339f4a) SHA1(688b2e1ff8c60bde6e8b07a2d2695cdacc07bd0c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "jp1002", "VMS Japanese BIOS (v1.002)")
	ROMX_LOAD("vmu1002.bin", 0x0000, 0x10000, CRC(6c020d48) SHA1(9ee7c87d7b033235e0b315a0b421e70deb547c7a), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "jp1004", "VMS Japanese BIOS (v1.004)")
	ROMX_LOAD("vmu1004.bin", 0x0000, 0x10000, CRC(8e0f867a) SHA1(dc2fa2963138a1049a43f7f36439ad0a416ee8b4), ROM_BIOS(3))    // from Sega Katana SDK (original file: fbios.sbf, CRC: c7c77b3c, xor key: 0x37)
	ROM_SYSTEM_BIOS(3, "jp1005", "VMS Japanese BIOS (v1.005)")
	ROMX_LOAD("vmu1005.bin", 0x0000, 0x10000, CRC(47623324) SHA1(fca1aceff8a2f8c6826f3a865f4d5ef88dfd9ed1), ROM_BIOS(4))
	ROM_SYSTEM_BIOS(4, "dev1004", "VMS Japanese Dev BIOS (v1.004)") // automatically boot the first game found in the flash
	ROMX_LOAD( "vmsdev1004.bin", 0x0000, 0x10000, CRC(395e25f2) SHA1(37dea034322b5b80b35b2de784298d32c71ba7a3), ROM_BIOS(5))    // from Sega Katana SDK (original file: qbios.sbf, CRC: eed5524c, xor key: 0x43)
ROM_END


/* Driver */

/*  YEAR  NAME  PARENT  COMPAT   MACHINE    INPUT   INIT    COMPANY   FULLNAME     FLAGS */
COMP( 1998, svmu,   0,  0,  svmu ,  svmu , driver_device,   0, "Sega",   "Visual Memory Unit",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND)

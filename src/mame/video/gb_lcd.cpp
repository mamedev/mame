// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  gb_lcd.c

  Video file to handle emulation of the Nintendo Game Boy.

  Original code                               Carsten Sorensen   1998
  Mess modifications, bug fixes and speedups  Hans de Goede      1998
  Bug fixes, SGB and GBC code                 Anthony Kruize     2002
  Improvements to match real hardware         Wilbert Pol        2006-2008

  Timing is not accurate enough:
  - Mode 3 takes 172 cycles (measuered with logic analyzer by costis)

***************************************************************************/

#include "emu.h"
//#include "cpu/lr35902/lr35902.h"
#include "video/gb_lcd.h"

/* Interrupts (copied from includes/gb.h)... */
#define VBL_INT               0       /* V-Blank    */
#define LCD_INT               1       /* LCD Status */
#define TIM_INT               2       /* Timer      */
#define SIO_INT               3       /* Serial I/O */
#define EXT_INT               4       /* Joypad     */



#define LCDCONT     m_vid_regs[0x00]  /* LCD control register                       */
#define LCDSTAT     m_vid_regs[0x01]  /* LCD status register                        */
#define SCROLLY     m_vid_regs[0x02]  /* Starting Y position of the background      */
#define SCROLLX     m_vid_regs[0x03]  /* Starting X position of the background      */
#define CURLINE     m_vid_regs[0x04]  /* Current screen line being scanned          */
#define CMPLINE     m_vid_regs[0x05]  /* Gen. int. when scan reaches this line      */
#define BGRDPAL     m_vid_regs[0x07]  /* Background palette                         */
#define SPR0PAL     m_vid_regs[0x08]  /* Sprite palette #0                          */
#define SPR1PAL     m_vid_regs[0x09]  /* Sprite palette #1                          */
#define WNDPOSY     m_vid_regs[0x0A]  /* Window Y position                          */
#define WNDPOSX     m_vid_regs[0x0B]  /* Window X position                          */
#define KEY1        m_vid_regs[0x0D]  /* Prepare speed switch                       */
#define HDMA1       m_vid_regs[0x11]  /* HDMA source high byte                      */
#define HDMA2       m_vid_regs[0x12]  /* HDMA source low byte                       */
#define HDMA3       m_vid_regs[0x13]  /* HDMA destination high byte                 */
#define HDMA4       m_vid_regs[0x14]  /* HDMA destination low byte                  */
#define HDMA5       m_vid_regs[0x15]  /* HDMA length/mode/start                     */
#define GBCBCPS     m_vid_regs[0x28]  /* Backgound palette spec                     */
#define GBCBCPD     m_vid_regs[0x29]  /* Backgound palette data                     */
#define GBCOCPS     m_vid_regs[0x2A]  /* Object palette spec                        */
#define GBCOCPD     m_vid_regs[0x2B]  /* Object palette data                        */

/* -- Super Game Boy specific -- */
#define SGB_BORDER_PAL_OFFSET   64  /* Border colours stored from pal 4-7   */
#define SGB_XOFFSET             48  /* GB screen starts at column 48        */
#define SGB_YOFFSET             40  /* GB screen starts at row 40           */


enum {
	UNLOCKED=0,
	LOCKED
};


enum {
	GB_LCD_STATE_LYXX_M3=1,
	GB_LCD_STATE_LYXX_PRE_M0,
	GB_LCD_STATE_LYXX_M0,
	GB_LCD_STATE_LYXX_M0_SCX3,
	GB_LCD_STATE_LYXX_M0_GBC_PAL,
	GB_LCD_STATE_LYXX_M0_PRE_INC,
	GB_LCD_STATE_LYXX_M0_INC,
	GB_LCD_STATE_LY00_M2,
	GB_LCD_STATE_LYXX_M2,
	GB_LCD_STATE_LY9X_M1,
	GB_LCD_STATE_LY9X_M1_INC,
	GB_LCD_STATE_LY00_M1,
	GB_LCD_STATE_LY00_M1_1,
	GB_LCD_STATE_LY00_M1_2,
	GB_LCD_STATE_LY00_M0
};


/* OAM contents on power up.

 The OAM area seems contain some kind of unit fingerprint. On each boot
 the data is almost always the same. Some random bits are flipped between
 different boots. It is currently unknown how much these fingerprints
 differ between different units.

 OAM fingerprints taken from Wilbert Pol's own unit.
 */

static const UINT8 dmg_oam_fingerprint[0x100] = {
	0xD8, 0xE6, 0xB3, 0x89, 0xEC, 0xDE, 0x11, 0x62, 0x0B, 0x7E, 0x48, 0x9E, 0xB9, 0x6E, 0x26, 0xC9,
	0x36, 0xF4, 0x7D, 0xE4, 0xD9, 0xCE, 0xFA, 0x5E, 0xA3, 0x77, 0x60, 0xFC, 0x1C, 0x64, 0x8B, 0xAC,
	0xB6, 0x74, 0x3F, 0x9A, 0x0E, 0xFE, 0xEA, 0xA9, 0x40, 0x3A, 0x7A, 0xB6, 0xF2, 0xED, 0xA8, 0x3E,
	0xAF, 0x2C, 0xD2, 0xF2, 0x01, 0xE0, 0x5B, 0x3A, 0x53, 0x6A, 0x1C, 0x6C, 0x20, 0xD9, 0x22, 0xB4,
	0x8C, 0x38, 0x71, 0x69, 0x3E, 0x93, 0xA3, 0x22, 0xCE, 0x76, 0x24, 0xE7, 0x1A, 0x14, 0x6B, 0xB1,
	0xF9, 0x3D, 0xBF, 0x3D, 0x74, 0x64, 0xCB, 0xF5, 0xDC, 0x9A, 0x53, 0xC6, 0x0E, 0x78, 0x34, 0xCB,
	0x42, 0xB3, 0xFF, 0x07, 0x73, 0xAE, 0x6C, 0xA2, 0x6F, 0x6A, 0xA4, 0x66, 0x0A, 0x8C, 0x40, 0xB3,
	0x9A, 0x3D, 0x39, 0x78, 0xAB, 0x29, 0xE7, 0xC5, 0x7A, 0xDD, 0x51, 0x95, 0x2B, 0xE4, 0x1B, 0xF6,
	0x31, 0x16, 0x34, 0xFE, 0x11, 0xF2, 0x5E, 0x11, 0xF3, 0x95, 0x66, 0xB9, 0x37, 0xC2, 0xAD, 0x6D,
	0x1D, 0xA7, 0x79, 0x06, 0xD7, 0xE5, 0x8F, 0xFA, 0x9C, 0x02, 0x0C, 0x31, 0x8B, 0x17, 0x2E, 0x31,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const UINT8 mgb_oam_fingerprint[0x100] = {
	0xB9, 0xE9, 0x0D, 0x69, 0xBB, 0x7F, 0x00, 0x80, 0xE9, 0x7B, 0x79, 0xA2, 0xFD, 0xCF, 0xD8, 0x0A,
	0x87, 0xEF, 0x44, 0x11, 0xFE, 0x37, 0x10, 0x21, 0xFA, 0xFF, 0x00, 0x17, 0xF6, 0x4F, 0x83, 0x03,
	0x3A, 0xF4, 0x00, 0x24, 0xBB, 0xAE, 0x05, 0x01, 0xFF, 0xF7, 0x12, 0x48, 0xA7, 0x5E, 0xF6, 0x28,
	0x5B, 0xFF, 0x2E, 0x10, 0xFF, 0xB9, 0x50, 0xC8, 0xAF, 0x77, 0x2C, 0x1A, 0x62, 0xD7, 0x81, 0xC2,
	0xFD, 0x5F, 0xA0, 0x94, 0xAF, 0xFF, 0x51, 0x20, 0x36, 0x76, 0x50, 0x0A, 0xFD, 0xF6, 0x20, 0x00,
	0xFE, 0xF7, 0xA0, 0x68, 0xFF, 0xFC, 0x29, 0x51, 0xA3, 0xFA, 0x06, 0xC4, 0x94, 0xFF, 0x39, 0x0A,
	0xFF, 0x6C, 0x20, 0x20, 0xF1, 0xAD, 0x0C, 0x81, 0x56, 0xFB, 0x03, 0x82, 0xFF, 0xFF, 0x08, 0x58,
	0x96, 0x7E, 0x01, 0x4D, 0xFF, 0xE4, 0x82, 0xE3, 0x3D, 0xBB, 0x54, 0x00, 0x3D, 0xF3, 0x04, 0x21,
	0xB7, 0x39, 0xCC, 0x10, 0xF9, 0x5B, 0x80, 0x50, 0x3F, 0x6A, 0x1C, 0x21, 0x1F, 0xFA, 0xA8, 0x52,
	0x5F, 0xB3, 0x44, 0xA1, 0x96, 0x1E, 0x00, 0x27, 0x63, 0x77, 0x30, 0x54, 0x37, 0x6F, 0x60, 0x22,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const UINT8 cgb_oam_fingerprint[0x100] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x74, 0xFF, 0x09, 0x00, 0x9D, 0x61, 0xA8, 0x28, 0x36, 0x1E, 0x58, 0xAA, 0x75, 0x74, 0xA1, 0x42,
	0x05, 0x96, 0x40, 0x09, 0x41, 0x02, 0x60, 0x00, 0x1F, 0x11, 0x22, 0xBC, 0x31, 0x52, 0x22, 0x54,
	0x22, 0xA9, 0xC4, 0x00, 0x1D, 0xAD, 0x80, 0x0C, 0x5D, 0xFA, 0x51, 0x92, 0x93, 0x98, 0xA4, 0x04,
	0x22, 0xA9, 0xC4, 0x00, 0x1D, 0xAD, 0x80, 0x0C, 0x5D, 0xFA, 0x51, 0x92, 0x93, 0x98, 0xA4, 0x04,
	0x22, 0xA9, 0xC4, 0x00, 0x1D, 0xAD, 0x80, 0x0C, 0x5D, 0xFA, 0x51, 0x92, 0x93, 0x98, 0xA4, 0x04,
	0x22, 0xA9, 0xC4, 0x00, 0x1D, 0xAD, 0x80, 0x0C, 0x5D, 0xFA, 0x51, 0x92, 0x93, 0x98, 0xA4, 0x04
};

/*
 For an AGS in CGB mode this data is: */
#if 0
static const UINT8 abs_oam_fingerprint[0x100] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
	0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB,
	0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,
	0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
	0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
#endif


const device_type GB_LCD_DMG = &device_creator<gb_lcd_device>;
const device_type GB_LCD_MGB = &device_creator<mgb_lcd_device>;
const device_type GB_LCD_SGB = &device_creator<sgb_lcd_device>;
const device_type GB_LCD_CGB = &device_creator<cgb_lcd_device>;



gb_lcd_device::gb_lcd_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source)
				: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
					device_video_interface(mconfig, *this),
					m_sgb_border_hack(0)
{
}

gb_lcd_device::gb_lcd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: device_t(mconfig, GB_LCD_DMG, "DMG LCD", tag, owner, clock, "dmg_lcd", __FILE__),
					device_video_interface(mconfig, *this),
					m_sgb_border_hack(0)
{
}

mgb_lcd_device::mgb_lcd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: gb_lcd_device(mconfig, GB_LCD_MGB, "MGB LCD", tag, owner, clock, "mgb_lcd", __FILE__)
{
}

sgb_lcd_device::sgb_lcd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: gb_lcd_device(mconfig, GB_LCD_SGB, "SGB LCD", tag, owner, clock, "sgb_lcd", __FILE__)
{
}

cgb_lcd_device::cgb_lcd_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
				: gb_lcd_device(mconfig, GB_LCD_CGB, "CGB LCD", tag, owner, clock, "cgb_lcd", __FILE__)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void gb_lcd_device::common_start()
{
	m_screen->register_screen_bitmap(m_bitmap);
	save_item(NAME(m_bitmap));
	m_oam = make_unique_clear<UINT8[]>(0x100);

	machine().save().register_postload(save_prepost_delegate(FUNC(gb_lcd_device::videoptr_restore), this));

	m_maincpu = machine().device<cpu_device>("maincpu");

	save_pointer(NAME(m_oam.get()), 0x100);
	save_item(NAME(m_window_lines_drawn));
	save_item(NAME(m_vid_regs));
	save_item(NAME(m_bg_zbuf));

	save_item(NAME(m_cgb_bpal));
	save_item(NAME(m_cgb_spal));

	save_item(NAME(m_gb_bpal));
	save_item(NAME(m_gb_spal0));
	save_item(NAME(m_gb_spal1));

	save_item(NAME(m_current_line));
	save_item(NAME(m_cmp_line));
	save_item(NAME(m_sprCount));
	save_item(NAME(m_sprite));
	save_item(NAME(m_previous_line));
	save_item(NAME(m_start_x));
	save_item(NAME(m_end_x));
	save_item(NAME(m_mode));
	save_item(NAME(m_state));
	save_item(NAME(m_lcd_irq_line));
	save_item(NAME(m_triggering_line_irq));
	save_item(NAME(m_line_irq));
	save_item(NAME(m_triggering_mode_irq));
	save_item(NAME(m_mode_irq));
	save_item(NAME(m_delayed_line_irq));
	save_item(NAME(m_sprite_cycles));
	save_item(NAME(m_scrollx_adjust));
	save_item(NAME(m_oam_locked));
	save_item(NAME(m_vram_locked));
	save_item(NAME(m_pal_locked));
	save_item(NAME(m_hdma_enabled));
	save_item(NAME(m_hdma_possible));
	save_item(NAME(m_gbc_mode));
	save_item(NAME(m_gb_tile_no_mod));
	save_item(NAME(m_vram_bank));

	save_item(NAME(m_gb_chrgen_offs));
	save_item(NAME(m_gb_bgdtab_offs));
	save_item(NAME(m_gb_wndtab_offs));
	save_item(NAME(m_gbc_chrgen_offs));
	save_item(NAME(m_gbc_bgdtab_offs));
	save_item(NAME(m_gbc_wndtab_offs));

	save_item(NAME(m_layer[0].enabled));
	save_item(NAME(m_layer[0].xindex));
	save_item(NAME(m_layer[0].xshift));
	save_item(NAME(m_layer[0].xstart));
	save_item(NAME(m_layer[0].xend));
	save_item(NAME(m_layer[0].bgline));
	save_item(NAME(m_layer[1].enabled));
	save_item(NAME(m_layer[1].xindex));
	save_item(NAME(m_layer[1].xshift));
	save_item(NAME(m_layer[1].xstart));
	save_item(NAME(m_layer[1].xend));
	save_item(NAME(m_layer[1].bgline));
}


void gb_lcd_device::videoptr_restore()
{
	m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
	m_layer[0].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
	m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
	m_layer[1].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
}

void cgb_lcd_device::videoptr_restore()
{
	m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
	m_layer[0].gbc_map = m_vram.get() + m_gbc_bgdtab_offs;
	m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
	m_layer[1].gbc_map = m_vram.get() + m_gbc_wndtab_offs;
}


void gb_lcd_device::device_start()
{
	common_start();
	m_lcd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(gb_lcd_device::lcd_timer_proc),this));

	m_vram = make_unique_clear<UINT8[]>(0x2000);
	save_pointer(NAME(m_vram.get()), 0x2000);

	memcpy(m_oam.get(), dmg_oam_fingerprint, 0x100);
}

void mgb_lcd_device::device_start()
{
	common_start();
	m_lcd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(mgb_lcd_device::lcd_timer_proc),this));

	m_vram = make_unique_clear<UINT8[]>(0x2000);
	save_pointer(NAME(m_vram.get()), 0x2000);

	memcpy(m_oam.get(), mgb_oam_fingerprint, 0x100);
}

void sgb_lcd_device::device_start()
{
	common_start();
	m_lcd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(sgb_lcd_device::lcd_timer_proc),this));

	m_vram = make_unique_clear<UINT8[]>(0x2000);
	save_pointer(NAME(m_vram.get()), 0x2000);

	m_sgb_tile_data = make_unique_clear<UINT8[]>(0x2000);
	save_pointer(NAME(m_sgb_tile_data.get()), 0x2000);

	memset(m_sgb_tile_map, 0, sizeof(m_sgb_tile_map));

	/* Some default colours for non-SGB games */
	m_sgb_pal[0] = 32767;
	m_sgb_pal[1] = 21140;
	m_sgb_pal[2] = 10570;
	m_sgb_pal[3] = 0;
	/* The rest of the colortable can be black */
	for (int i = 4; i < 8 * 16; i++)
		m_sgb_pal[i] = 0;

	save_item(NAME(m_sgb_atf_data));
	save_item(NAME(m_sgb_atf));
	save_item(NAME(m_sgb_pal_data));
	save_item(NAME(m_sgb_pal_map));
	save_item(NAME(m_sgb_pal));
	save_item(NAME(m_sgb_tile_map));
	save_item(NAME(m_sgb_window_mask));
}

void cgb_lcd_device::device_start()
{
	common_start();
	m_lcd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(cgb_lcd_device::lcd_timer_proc),this));

	m_vram = make_unique_clear<UINT8[]>(0x4000);
	save_pointer(NAME(m_vram.get()), 0x4000);

	memcpy(m_oam.get(), cgb_oam_fingerprint, 0x100);

	/* Background is initialised as white */
	for (int i = 0; i < 32; i++)
		m_cgb_bpal[i] = 32767;
	/* Sprites are supposed to be uninitialized, but we'll make them black */
	for (int i = 0; i < 32; i++)
		m_cgb_spal[i] = 0;
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void gb_lcd_device::common_reset()
{
	m_window_lines_drawn = 0;

	m_current_line = 0;
	m_cmp_line = 0;
	m_sprCount = 0;
	m_previous_line = 0;
	m_start_x = 0;
	m_end_x = 0;
	m_mode = 0;
	m_state = 0;
	m_lcd_irq_line = 0;
	m_triggering_line_irq = 0;
	m_line_irq = 0;
	m_triggering_mode_irq = 0;
	m_mode_irq = 0;
	m_delayed_line_irq = 0;
	m_sprite_cycles = 0;
	m_scrollx_adjust = 0;
	m_oam_locked = 0;
	m_vram_locked = 0;
	m_pal_locked = 0;
	m_gbc_mode = 0;
	m_gb_tile_no_mod = 0;
	m_vram_bank = 0;

	m_gb_chrgen_offs = 0;
	m_gb_bgdtab_offs = 0x1c00;
	m_gb_wndtab_offs = 0x1c00;

	memset(&m_vid_regs, 0, sizeof(m_vid_regs));
	memset(&m_bg_zbuf, 0, sizeof(m_bg_zbuf));
	memset(&m_cgb_bpal, 0, sizeof(m_cgb_bpal));
	memset(&m_cgb_spal, 0, sizeof(m_cgb_spal));
	memset(&m_sprite, 0, sizeof(m_sprite));
	memset(&m_layer[0], 0, sizeof(m_layer[0]));
	memset(&m_layer[1], 0, sizeof(m_layer[1]));

	// specific reg initialization
	m_vid_regs[0x06] = 0xff;

	for (int i = 0x0c; i < _NR_GB_VID_REGS; i++)
		m_vid_regs[i] = 0xff;

	LCDSTAT = 0x80;
	LCDCONT = 0x00;     /* Video hardware is turned off at boot time */
	m_current_line = CURLINE = CMPLINE = 0x00;
	SCROLLX = SCROLLY = 0x00;
	SPR0PAL = SPR1PAL = 0xFF;
	WNDPOSX = WNDPOSY = 0x00;

	// Initialize palette arrays
	for (int i = 0; i < 4; i++)
		m_gb_bpal[i] = m_gb_spal0[i] = m_gb_spal1[i] = i;

}


void gb_lcd_device::device_reset()
{
	common_reset();

	m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(456));
}

void sgb_lcd_device::device_reset()
{
	common_reset();

	memset(m_sgb_tile_data.get(), 0, 0x2000);

	m_sgb_window_mask = 0;

	memset(m_sgb_pal_map, 0, sizeof(m_sgb_pal_map));
	memset(m_sgb_atf_data, 0, sizeof(m_sgb_atf_data));
}

void cgb_lcd_device::device_reset()
{
	common_reset();

	m_gbc_chrgen_offs = 0x2000;
	m_gbc_bgdtab_offs = 0x3c00;
	m_gbc_wndtab_offs = 0x3c00;

	/* HDMA disabled */
	m_hdma_enabled = 0;
	m_hdma_possible = 0;

	m_gbc_mode = 1;
}



inline void gb_lcd_device::plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color)
{
	bitmap.pix16(y, x) = (UINT16)color;
}

/*
  Select which sprites should be drawn for the current scanline and return the
  number of sprites selected.
 */
void gb_lcd_device::select_sprites()
{
	int /*yindex,*/ line, height;
	UINT8 *oam = m_oam.get() + 39 * 4;

	m_sprCount = 0;

	/* If video hardware is enabled and sprites are enabled */
	if ((LCDCONT & 0x80) && (LCDCONT & 0x02))
	{
		/* Check for stretched sprites */
		if (LCDCONT & 0x04)
			height = 16;
		else
			height = 8;

		//yindex = m_current_line;
		line = m_current_line + 16;

		for (int i = 39; i >= 0; i--)
		{
			if (line >= oam[0] && line < (oam[0] + height) && oam[1] && oam[1] < 168)
			{
				/* We limit the sprite count to max 10 here;
				   proper games should not exceed this... */
				if (m_sprCount < 10)
				{
					m_sprite[m_sprCount] = i;
					m_sprCount++;
				}
			}
			oam -= 4;
		}
	}
}

void gb_lcd_device::update_sprites()
{
	bitmap_ind16 &bitmap = m_bitmap;
	UINT8 height, tilemask, line, *oam, *vram;
	int yindex;

	if (LCDCONT & 0x04)
	{
		height = 16;
		tilemask = 0xFE;
	}
	else
	{
		height = 8;
		tilemask = 0xFF;
	}

	yindex = m_current_line;
	line = m_current_line + 16;

	oam = m_oam.get() + 39 * 4;
	vram = m_vram.get();
	for (int i = 39; i >= 0; i--)
	{
		/* if sprite is on current line && x-coordinate && x-coordinate is < 168 */
		if (line >= oam[0] && line < (oam[0] + height) && oam[1] && oam[1] < 168)
		{
			UINT16 data;
			UINT8 bit, *spal;
			int xindex, adr;

			spal = (oam[3] & 0x10) ? m_gb_spal1 : m_gb_spal0;
			xindex = oam[1] - 8;
			if (oam[3] & 0x40)         /* flip y ? */
			{
				adr = (oam[2] & tilemask) * 16 + (height - 1 - line + oam[0]) * 2;
			}
			else
			{
				adr = (oam[2] & tilemask) * 16 + (line - oam[0]) * 2;
			}
			data = (vram[adr + 1] << 8) | vram[adr];

			switch (oam[3] & 0xA0)
			{
			case 0xA0:                 /* priority is set (behind bgnd & wnd, flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
					if (colour && !m_bg_zbuf[xindex] && xindex >= 0 && xindex < 160)
						plot_pixel(bitmap, xindex, yindex, spal[colour]);
					data >>= 1;
				}
				break;
			case 0x20:                 /* priority is not set (overlaps bgnd & wnd, flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
					if (colour && xindex >= 0 && xindex < 160)
						plot_pixel(bitmap, xindex, yindex, spal[colour]);
					data >>= 1;
				}
				break;
			case 0x80:                 /* priority is set (behind bgnd & wnd, don't flip x) */
				for (bit = 0; bit < 8 && xindex < 160; bit++, xindex++)
				{
					int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
					if (colour && !m_bg_zbuf[xindex] && xindex >= 0 && xindex < 160)
						plot_pixel(bitmap, xindex, yindex, spal[colour]);
					data <<= 1;
				}
				break;
			case 0x00:                 /* priority is not set (overlaps bgnd & wnd, don't flip x) */
				for (bit = 0; bit < 8 && xindex < 160; bit++, xindex++)
				{
					int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
					if (colour && xindex >= 0 && xindex < 160)
						plot_pixel(bitmap, xindex, yindex, spal[colour]);
					data <<= 1;
				}
				break;
			}
		}
		oam -= 4;
	}
}

void gb_lcd_device::update_scanline()
{
	bitmap_ind16 &bitmap = m_bitmap;

	g_profiler.start(PROFILER_VIDEO);

	/* Make sure we're in mode 3 */
	if ((LCDSTAT & 0x03) == 0x03)
	{
		/* Calculate number of pixels to render based on time still left on the timer */
		UINT32 cycles_to_go = m_maincpu->attotime_to_cycles(m_lcd_timer->remaining());
		int l = 0;

		if (m_start_x < 0)
		{
			/* Window is enabled if the hardware says so AND the current scanline is
			 * within the window AND the window X coordinate is <=166 */
			m_layer[1].enabled = ((LCDCONT & 0x20) && (m_current_line >= WNDPOSY) && (WNDPOSX <= 166)) ? 1 : 0;

			/* BG is enabled if the hardware says so AND (window_off OR (window_on
			* AND window's X position is >=7)) */
			m_layer[0].enabled = ((LCDCONT & 0x01) && ((!m_layer[1].enabled) || (m_layer[1].enabled && (WNDPOSX >= 7)))) ? 1 : 0;

			if (m_layer[0].enabled)
			{
				m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
				m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
				m_layer[0].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
				m_layer[0].xindex = SCROLLX >> 3;
				m_layer[0].xshift = SCROLLX & 7;
				m_layer[0].xstart = 0;
				m_layer[0].xend = 160;
			}

			if (m_layer[1].enabled)
			{
				int xpos = WNDPOSX - 7;             /* Window is offset by 7 pixels */
				if (xpos < 0)
					xpos = 0;

				m_layer[1].bgline = m_window_lines_drawn;
				m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
				m_layer[1].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
				m_layer[1].xindex = 0;
				m_layer[1].xshift = 0;
				m_layer[1].xstart = xpos;
				m_layer[1].xend = 160;
				m_layer[0].xend = xpos;
			}
			m_start_x = 0;
		}

		if (cycles_to_go < 160)
		{
			m_end_x = MIN(160 - cycles_to_go, 160);
			/* Draw empty pixels when the background is disabled */
			if (!(LCDCONT & 0x01))
			{
				rectangle r(m_start_x, m_end_x - 1, m_current_line, m_current_line);
				bitmap.fill(m_gb_bpal[0], r);
			}
			while (l < 2)
			{
				UINT8 xindex, *map, *tiles;
				UINT16 data;
				int i, tile_index;

				if (!m_layer[l].enabled)
				{
					l++;
					continue;
				}
				map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
				tiles = m_layer[l].bg_tiles + ((m_layer[l].bgline & 7) << 1);
				xindex = m_start_x;
				if (xindex < m_layer[l].xstart)
					xindex = m_layer[l].xstart;
				i = m_end_x;
				if (i > m_layer[l].xend)
					i = m_layer[l].xend;
				i = i - xindex;

				tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
				data = tiles[tile_index] | (tiles[tile_index+1] << 8);
				data <<= m_layer[l].xshift;

				while (i > 0)
				{
					while ((m_layer[l].xshift < 8) && i)
					{
						int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
						plot_pixel(bitmap, xindex, m_current_line, m_gb_bpal[colour]);
						m_bg_zbuf[xindex] = colour;
						xindex++;
						data <<= 1;
						m_layer[l].xshift++;
						i--;
					}
					if (m_layer[l].xshift == 8)
					{
						/* Take possible changes to SCROLLY into account */
						if (l == 0)
						{
							m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
							map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
							tiles = m_layer[l].bg_tiles + ((m_layer[l].bgline & 7) << 1);
						}

						m_layer[l].xindex = (m_layer[l].xindex + 1) & 31;
						m_layer[l].xshift = 0;
						tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
						data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
					}
				}
				l++;
			}
			if (m_end_x == 160 && LCDCONT & 0x02)
			{
				update_sprites();
			}
			m_start_x = m_end_x;
		}
	}
	else
	{
		if (!(LCDCONT & 0x80))
		{
			/* Draw an empty line when LCD is disabled */
			if (m_previous_line != m_current_line)
			{
				if (m_current_line < 144)
				{
					const rectangle &r = m_screen->visible_area();
					rectangle r1(r.min_x, r.max_x, m_current_line, m_current_line);
					bitmap.fill(0, r1);
				}
				m_previous_line = m_current_line;
			}
		}
	}

	g_profiler.stop();
}

/* --- Super Game Boy Specific --- */

void sgb_lcd_device::update_sprites()
{
	bitmap_ind16 &bitmap = m_bitmap;
	UINT8 height, tilemask, line, *oam, *vram, pal;
	INT16 yindex;

	if (LCDCONT & 0x04)
	{
		height = 16;
		tilemask = 0xFE;
	}
	else
	{
		height = 8;
		tilemask = 0xFF;
	}

	/* Offset to center of screen */
	yindex = m_current_line + SGB_YOFFSET;
	line = m_current_line + 16;

	oam = m_oam.get() + 39 * 4;
	vram = m_vram.get();
	for (int i = 39; i >= 0; i--)
	{
		/* if sprite is on current line && x-coordinate && x-coordinate is < 168 */
		if (line >= oam[0] && line < (oam[0] + height) && oam[1] && oam[1] < 168)
		{
			UINT16 data;
			UINT8 bit, *spal;
			INT16 xindex;
			int adr;

			spal = (oam[3] & 0x10) ? m_gb_spal1 : m_gb_spal0;
			xindex = oam[1] - 8;
			if (oam[3] & 0x40)         /* flip y ? */
			{
				adr = (oam[2] & tilemask) * 16 + (height - 1 - line + oam[0]) * 2;
			}
			else
			{
				adr = (oam[2] & tilemask) * 16 + (line - oam[0]) * 2;
			}
			data = (vram[adr + 1] << 8) | vram[adr];

			/* Find the palette to use */
			// If sprite started before the start of the line we may need to pick a different pal_map entry?
			pal = m_sgb_pal_map[(xindex < 0) ? 0 : (xindex >> 3)][((yindex - SGB_YOFFSET) >> 3)] << 2;

			/* Offset to center of screen */
			xindex += SGB_XOFFSET;

			switch (oam[3] & 0xA0)
			{
			case 0xA0:                 /* priority is set (behind bgnd & wnd, flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
					if ((xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160) && colour && !m_bg_zbuf[xindex - SGB_XOFFSET])
						plot_pixel(bitmap, xindex, yindex, m_sgb_pal[pal + spal[colour]]);
					data >>= 1;
				}
				break;
			case 0x20:                 /* priority is not set (overlaps bgnd & wnd, flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
					if ((xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160) && colour)
						plot_pixel(bitmap, xindex, yindex, m_sgb_pal[pal + spal[colour]]);
					data >>= 1;
				}
				break;
			case 0x80:                 /* priority is set (behind bgnd & wnd, don't flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
					if ((xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160) && colour && !m_bg_zbuf[xindex - SGB_XOFFSET])
						plot_pixel(bitmap, xindex, yindex, m_sgb_pal[pal + spal[colour]]);
					data <<= 1;
				}
				break;
			case 0x00:                 /* priority is not set (overlaps bgnd & wnd, don't flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
					if ((xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160) && colour)
						plot_pixel(bitmap, xindex, yindex, m_sgb_pal[pal + spal[colour]]);
					data <<= 1;
				}
				break;
			}
		}
		oam -= 4;
	}
}


void sgb_lcd_device::refresh_border()
{
	UINT16 data, data2;
	UINT8 *tiles, *tiles2;

	for (UINT16 yidx = 0; yidx < 224; yidx++)
	{
		UINT8 *map = m_sgb_tile_map + ((yidx >> 3) * 64);
		UINT16 xindex = 0;

		for (UINT16 xidx = 0; xidx < 64; xidx += 2)
		{
			if (map[xidx + 1] & 0x80) /* Vertical flip */
				tiles = m_sgb_tile_data.get() + ((7 - (yidx % 8)) << 1);
			else /* No vertical flip */
				tiles = m_sgb_tile_data.get() + ((yidx % 8) << 1);
			tiles2 = tiles + 16;

			UINT8 pal = (map[xidx + 1] & 0x1C) >> 2;
			if (pal == 0)
				pal = 1;
			pal <<= 4;

			if (m_sgb_border_hack)
			{ /* A few games do weird stuff */
				UINT8 tileno = map[xidx];
				if (tileno >= 128) tileno = ((64 + tileno) % 128) + 128;
				else tileno = (64 + tileno) % 128;
				data = tiles[tileno * 32] | (tiles[(tileno * 32) + 1] << 8);
				data2 = tiles2[tileno * 32] | (tiles2[(tileno * 32) + 1] << 8);
			}
			else
			{
				data = tiles[map[xidx] * 32] | (tiles[(map[xidx] * 32) + 1] << 8);
				data2 = tiles2[map[xidx] * 32] | (tiles2[(map[xidx] * 32) + 1] << 8);
			}

			for (int i = 0; i < 8; i++)
			{
				UINT8 colour;
				if ((map[xidx + 1] & 0x40))  /* Horizontal flip */
				{
					colour = ((data  & 0x0001) ? 1 : 0) | ((data  & 0x0100) ? 2 : 0) |
							((data2 & 0x0001) ? 4 : 0) | ((data2 & 0x0100) ? 8 : 0);
					data >>= 1;
					data2 >>= 1;
				}
				else    /* No horizontal flip */
				{
					colour = ((data  & 0x0080) ? 1 : 0) | ((data  & 0x8000) ? 2 : 0) |
							((data2 & 0x0080) ? 4 : 0) | ((data2 & 0x8000) ? 8 : 0);
					data <<= 1;
					data2 <<= 1;
				}
				/* A slight hack below so we don't draw over the GB screen.
				 * Drawing there is allowed, but due to the way we draw the
				 * scanline, it can obscure the screen even when it shouldn't.
				 */
				if (!((yidx >= SGB_YOFFSET && yidx < SGB_YOFFSET + 144) &&
					(xindex >= SGB_XOFFSET && xindex < SGB_XOFFSET + 160)))
				{
					plot_pixel(m_bitmap, xindex, yidx, m_sgb_pal[pal + colour]);
				}
				xindex++;
			}
		}
	}
}

void sgb_lcd_device::update_scanline()
{
	bitmap_ind16 &bitmap = m_bitmap;

	g_profiler.start(PROFILER_VIDEO);

	if ((LCDSTAT & 0x03) == 0x03)
	{
		/* Calcuate number of pixels to render based on time still left on the timer */
		UINT32 cycles_to_go = m_maincpu->attotime_to_cycles(m_lcd_timer->remaining());
		int l = 0;

		if (m_start_x < 0)
		{
			/* Window is enabled if the hardware says so AND the current scanline is
			 * within the window AND the window X coordinate is <=166 */
			m_layer[1].enabled = ((LCDCONT & 0x20) && m_current_line >= WNDPOSY && WNDPOSX <= 166) ? 1 : 0;

			/* BG is enabled if the hardware says so AND (window_off OR (window_on
			 * AND window's X position is >=7 )) */
			m_layer[0].enabled = ((LCDCONT & 0x01) && ((!m_layer[1].enabled) || (m_layer[1].enabled && WNDPOSX >= 7))) ? 1 : 0;

			if (m_layer[0].enabled)
			{
				m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
				m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
				m_layer[0].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
				m_layer[0].xindex = SCROLLX >> 3;
				m_layer[0].xshift = SCROLLX & 7;
				m_layer[0].xstart = 0;
				m_layer[0].xend = 160;
			}

			if (m_layer[1].enabled)
			{
				int xpos;

				/* Window X position is offset by 7 so we'll need to adjust */
				xpos = WNDPOSX - 7;
				if (xpos < 0)
					xpos = 0;

				m_layer[1].bgline = m_window_lines_drawn;
				m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
				m_layer[1].bg_tiles = m_vram.get() + m_gb_chrgen_offs;
				m_layer[1].xindex = 0;
				m_layer[1].xshift = 0;
				m_layer[1].xstart = xpos;
				m_layer[1].xend = 160;
				m_layer[0].xend = xpos;
			}
			m_start_x = 0;
		}

		if (cycles_to_go == 0)
		{
			/* Does this belong here? or should it be moved to the else block */
			/* Handle SGB mask */
			switch (m_sgb_window_mask)
			{
			case 1: /* Freeze screen */
				return;
			case 2: /* Blank screen (black) */
				{
					rectangle r(SGB_XOFFSET, SGB_XOFFSET + 160-1, SGB_YOFFSET, SGB_YOFFSET + 144 - 1);
					bitmap.fill(0, r);
				}
				return;
			case 3: /* Blank screen (white - or should it be color 0?) */
				{
					rectangle r(SGB_XOFFSET, SGB_XOFFSET + 160 - 1, SGB_YOFFSET, SGB_YOFFSET + 144 - 1);
					bitmap.fill(32767, r);
				}
				return;
			}

			/* Draw the "border" if we're on the first line */
			if (m_current_line == 0)
			{
				refresh_border();
			}
		}
		if (cycles_to_go < 160)
		{
			m_end_x = MIN(160 - cycles_to_go,160);

			/* if background or screen disabled clear line */
			if (!(LCDCONT & 0x01))
			{
				rectangle r(SGB_XOFFSET, SGB_XOFFSET + 160 - 1, m_current_line + SGB_YOFFSET, m_current_line + SGB_YOFFSET);
				bitmap.fill(0, r);
			}
			while (l < 2)
			{
				UINT8   xindex, sgb_palette, *map, *tiles;
				UINT16  data;
				int i, tile_index;

				if (!m_layer[l].enabled)
				{
					l++;
					continue;
				}
				map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
				tiles = m_layer[l].bg_tiles + ((m_layer[l].bgline & 7) << 1);
				xindex = m_start_x;
				if (xindex < m_layer[l].xstart)
					xindex = m_layer[l].xstart;
				i = m_end_x;
				if (i > m_layer[l].xend)
					i = m_layer[l].xend;
				i = i - xindex;

				tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
				data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
				data <<= m_layer[l].xshift;

				while (i > 0)
				{
					/* Figure out which palette we're using */
					assert(((m_end_x - i) >> 3) >= 0 && ((m_end_x - i) >> 3) < ARRAY_LENGTH(m_sgb_pal_map));
					sgb_palette = m_sgb_pal_map[(m_end_x - i) >> 3][m_current_line >> 3] << 2;

					while ((m_layer[l].xshift < 8) && i)
					{
						int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
						plot_pixel(bitmap, xindex + SGB_XOFFSET, m_current_line + SGB_YOFFSET, m_sgb_pal[sgb_palette + m_gb_bpal[colour]]);
						m_bg_zbuf[xindex] = colour;
						xindex++;
						data <<= 1;
						m_layer[l].xshift++;
						i--;
					}
					if (m_layer[l].xshift == 8)
					{
						/* Take possible changes to SCROLLY into account */
						if (l == 0)
						{
							m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
							map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
							tiles = m_layer[l].bg_tiles + ((m_layer[l].bgline & 7) << 1);
						}

						m_layer[l].xindex = (m_layer[l].xindex + 1) & 31;
						m_layer[l].xshift = 0;
						tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
						data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
					}
				}
				l++;
			}
			if ((m_end_x == 160) && (LCDCONT & 0x02))
			{
				update_sprites();
			}
			m_start_x = m_end_x;
		}
	}
	else
	{
		if (!(LCDCONT * 0x80))
		{
			/* if screen disabled clear line */
			if (m_previous_line != m_current_line)
			{
				/* Also refresh border here??? */
				if (m_current_line < 144)
				{
					rectangle r(SGB_XOFFSET, SGB_XOFFSET + 160 - 1, m_current_line + SGB_YOFFSET, m_current_line + SGB_YOFFSET);
					bitmap.fill(0, r);
				}
				m_previous_line = m_current_line;
			}
		}
	}

	g_profiler.stop();
}

/* --- Game Boy Color Specific --- */

void cgb_lcd_device::update_sprites()
{
	bitmap_ind16 &bitmap = m_bitmap;
	UINT8 height, tilemask, line, *oam;
	int xindex, yindex;

	if (LCDCONT & 0x04)
	{
		height = 16;
		tilemask = 0xFE;
	}
	else
	{
		height = 8;
		tilemask = 0xFF;
	}

	yindex = m_current_line;
	line = m_current_line + 16;

	oam = m_oam.get() + 39 * 4;
	for (int i = 39; i >= 0; i--)
	{
		/* if sprite is on current line && x-coordinate && x-coordinate is < 168 */
		if (line >= oam[0] && line < (oam[0] + height) && oam[1] && oam[1] < 168)
		{
			UINT16 data;
			UINT8 bit, pal;

			/* Handle mono mode for GB games */
			if (!m_gbc_mode)
				pal = (oam[3] & 0x10) ? 4 : 0;
			else
				pal = ((oam[3] & 0x7) * 4);

			xindex = oam[1] - 8;
			if (oam[3] & 0x40)         /* flip y ? */
			{
				data = *((UINT16 *) &m_vram[((oam[3] & 0x8)<<10) + (oam[2] & tilemask) * 16 + (height - 1 - line + oam[0]) * 2]);
			}
			else
			{
				data = *((UINT16 *) &m_vram[((oam[3] & 0x8)<<10) + (oam[2] & tilemask) * 16 + (line - oam[0]) * 2]);
			}

			data = LITTLE_ENDIANIZE_INT16(data);

			switch (oam[3] & 0xA0)
			{
			case 0xA0:                 /* priority is set (behind bgnd & wnd, flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
					if (colour && !m_bg_zbuf[xindex] && xindex >= 0 && xindex < 160)
					{
						if (!m_gbc_mode)
							colour = pal ? m_gb_spal1[colour] : m_gb_spal0[colour];
						plot_pixel(bitmap, xindex, yindex, m_cgb_spal[pal + colour]);
					}
					data >>= 1;
				}
				break;
			case 0x20:                 /* priority is not set (overlaps bgnd & wnd, flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					int colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
					if ((m_bg_zbuf[xindex] & 0x80) && (m_bg_zbuf[xindex] & 0x7f) && (LCDCONT & 0x1))
						colour = 0;
					if (colour && xindex >= 0 && xindex < 160)
					{
						if (!m_gbc_mode)
							colour = pal ? m_gb_spal1[colour] : m_gb_spal0[colour];
						plot_pixel(bitmap, xindex, yindex, m_cgb_spal[pal + colour]);
					}
					data >>= 1;
				}
				break;
			case 0x80:                 /* priority is set (behind bgnd & wnd, don't flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
					if (colour && !m_bg_zbuf[xindex] && xindex >= 0 && xindex < 160)
					{
						if (!m_gbc_mode)
							colour = pal ? m_gb_spal1[colour] : m_gb_spal0[colour];
						plot_pixel(bitmap, xindex, yindex, m_cgb_spal[pal + colour]);
					}
					data <<= 1;
				}
				break;
			case 0x00:                 /* priority is not set (overlaps bgnd & wnd, don't flip x) */
				for (bit = 0; bit < 8; bit++, xindex++)
				{
					if (xindex >= 0 && xindex < 160)
					{
						int colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
						if ((m_bg_zbuf[xindex] & 0x80) && (m_bg_zbuf[xindex] & 0x7f) && (LCDCONT & 0x1))
							colour = 0;
						if (colour)
						{
							if (!m_gbc_mode)
								colour = pal ? m_gb_spal1[colour] : m_gb_spal0[colour];
							plot_pixel(bitmap, xindex, yindex, m_cgb_spal[pal + colour]);
						}
					}
					data <<= 1;
				}
				break;
			}
		}
		oam -= 4;
	}
}

void cgb_lcd_device::update_scanline()
{
	bitmap_ind16 &bitmap = m_bitmap;

	g_profiler.start(PROFILER_VIDEO);

	if ((LCDSTAT & 0x03) == 0x03)
	{
		/* Calcuate number of pixels to render based on time still left on the timer */
		UINT32 cycles_to_go = m_maincpu->attotime_to_cycles(m_lcd_timer->remaining());
		int l = 0;

		if (m_start_x < 0)
		{
			/* Window is enabled if the hardware says so AND the current scanline is
			 * within the window AND the window X coordinate is <=166 */
			m_layer[1].enabled = ((LCDCONT & 0x20) && (m_current_line >= WNDPOSY) && (WNDPOSX <= 166)) ? 1 : 0;

			/* BG is enabled if the hardware says so AND (window_off OR (window_on
			 * AND window's X position is >=7 )) */
			m_layer[0].enabled = ((LCDCONT & 0x01) && ((!m_layer[1].enabled) || (m_layer[1].enabled && (WNDPOSX >= 7)))) ? 1 : 0;

			if (m_layer[0].enabled)
			{
				m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
				m_layer[0].bg_map = m_vram.get() + m_gb_bgdtab_offs;
				m_layer[0].gbc_map = m_vram.get() + m_gbc_bgdtab_offs;
				m_layer[0].xindex = SCROLLX >> 3;
				m_layer[0].xshift = SCROLLX & 7;
				m_layer[0].xstart = 0;
				m_layer[0].xend = 160;
			}

			if (m_layer[1].enabled)
			{
				int xpos;

				/* Window X position is offset by 7 so we'll need to adust */
				xpos = WNDPOSX - 7;
				if (xpos < 0)
					xpos = 0;

				m_layer[1].bgline = m_window_lines_drawn;
				m_layer[1].bg_map = m_vram.get() + m_gb_wndtab_offs;
				m_layer[1].gbc_map = m_vram.get() + m_gbc_wndtab_offs;
				m_layer[1].xindex = 0;
				m_layer[1].xshift = 0;
				m_layer[1].xstart = xpos;
				m_layer[1].xend = 160;
				m_layer[0].xend = xpos;
			}
			m_start_x = 0;
		}

		if (cycles_to_go < 160)
		{
			m_end_x = MIN(160 - cycles_to_go, 160);
			/* Draw empty line when the background is disabled */
			if (!(LCDCONT & 0x01))
			{
				rectangle r(m_start_x, m_end_x - 1, m_current_line, m_current_line);
				bitmap.fill((!m_gbc_mode) ? 0 : 32767, r);
			}
			while (l < 2)
			{
				UINT8   xindex, *map, *tiles, *gbcmap;
				UINT16  data;
				int i, tile_index;

				if (!m_layer[l].enabled)
				{
					l++;
					continue;
				}
				map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
				gbcmap = m_layer[l].gbc_map + ((m_layer[l].bgline << 2) & 0x3E0);
				tiles = (gbcmap[m_layer[l].xindex] & 0x08) ? (m_vram.get() + m_gbc_chrgen_offs) : (m_vram.get() + m_gb_chrgen_offs);

				/* Check for vertical flip */
				if (gbcmap[m_layer[l].xindex] & 0x40)
				{
					tiles += ((7 - (m_layer[l].bgline & 0x07)) << 1);
				}
				else
				{
					tiles += ((m_layer[l].bgline & 0x07) << 1);
				}
				xindex = m_start_x;
				if (xindex < m_layer[l].xstart)
					xindex = m_layer[l].xstart;
				i = m_end_x;
				if (i > m_layer[l].xend)
					i = m_layer[l].xend;
				i = i - xindex;

				tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
				data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
				/* Check for horinzontal flip */
				if (gbcmap[m_layer[l].xindex] & 0x20)
				{
					data >>= m_layer[l].xshift;
				}
				else
				{
					data <<= m_layer[l].xshift;
				}

				while (i > 0)
				{
					while ((m_layer[l].xshift < 8) && i)
					{
						int colour;
						/* Check for horinzontal flip */
						if (gbcmap[m_layer[l].xindex] & 0x20)
						{
							colour = ((data & 0x0100) ? 2 : 0) | ((data & 0x0001) ? 1 : 0);
							data >>= 1;
						}
						else
						{
							colour = ((data & 0x8000) ? 2 : 0) | ((data & 0x0080) ? 1 : 0);
							data <<= 1;
						}
						plot_pixel(bitmap, xindex, m_current_line, m_cgb_bpal[(!m_gbc_mode) ? m_gb_bpal[colour] : (((gbcmap[m_layer[l].xindex] & 0x07) * 4) + colour)]);
						m_bg_zbuf[xindex] = colour + (gbcmap[m_layer[l].xindex] & 0x80);
						xindex++;
						m_layer[l].xshift++;
						i--;
					}
					if (m_layer[l].xshift == 8)
					{
						/* Take possible changes to SCROLLY into account */
						if (l == 0)
						{
							m_layer[0].bgline = (SCROLLY + m_current_line) & 0xFF;
							map = m_layer[l].bg_map + ((m_layer[l].bgline << 2) & 0x3E0);
							gbcmap = m_layer[l].gbc_map + ((m_layer[l].bgline << 2) & 0x3E0);
						}

						m_layer[l].xindex = (m_layer[l].xindex + 1) & 31;
						m_layer[l].xshift = 0;
						tiles = (gbcmap[m_layer[l].xindex] & 0x08) ? (m_vram.get() + m_gbc_chrgen_offs) : (m_vram.get() + m_gb_chrgen_offs);

						/* Check for vertical flip */
						if (gbcmap[m_layer[l].xindex] & 0x40)
						{
							tiles += ((7 - (m_layer[l].bgline & 0x07)) << 1);
						}
						else
						{
							tiles += ((m_layer[l].bgline & 0x07) << 1);
						}
						tile_index = (map[m_layer[l].xindex] ^ m_gb_tile_no_mod) * 16;
						data = tiles[tile_index] | (tiles[tile_index + 1] << 8);
					}
				}
				l++;
			}
			if (m_end_x == 160 && (LCDCONT & 0x02))
			{
				update_sprites();
			}
			m_start_x = m_end_x;
		}
	}
	else
	{
		if (!(LCDCONT & 0x80))
		{
			/* Draw an empty line when LCD is disabled */
			if (m_previous_line != m_current_line)
			{
				if (m_current_line < 144)
				{
					const rectangle &r1 = m_screen->visible_area();
					rectangle r(r1.min_x, r1.max_x, m_current_line, m_current_line);
					bitmap.fill((!m_gbc_mode) ? 0 : 32767 , r);
				}
				m_previous_line = m_current_line;
			}
		}
	}

	g_profiler.stop();
}


TIMER_CALLBACK_MEMBER(gb_lcd_device::video_init_vbl)
{
	m_maincpu->set_input_line(VBL_INT, ASSERT_LINE);
}

UINT32 gb_lcd_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}


void gb_lcd_device::increment_scanline()
{
	m_current_line = (m_current_line + 1) % 154;
	if (LCDCONT & 0x80)
	{
		CURLINE = m_current_line;
	}
	if (m_current_line == 0)
	{
		m_window_lines_drawn = 0;
	}
}

TIMER_CALLBACK_MEMBER(gb_lcd_device::lcd_timer_proc)
{
	static const int gb_sprite_cycles[] = { 0, 8, 20, 32, 44, 52, 64, 76, 88, 96, 108 };

	m_state = param;

	if (LCDCONT & 0x80)
	{
		switch (m_state)
		{
		case GB_LCD_STATE_LYXX_PRE_M0:  /* Just before switching to mode 0 */
			m_mode = 0;
			if (LCDSTAT & 0x08)
			{
				if (!m_mode_irq)
				{
					if (!m_line_irq && !m_delayed_line_irq)
					{
						m_mode_irq = 1;
						m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
					}
				}
				else
				{
					m_mode_irq = 0;
				}
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LYXX_M0);
			break;
		case GB_LCD_STATE_LYXX_M0:      /* Switch to mode 0 */
			/* update current scanline */
			update_scanline();
			/* Increment the number of window lines drawn if enabled */
			if (m_layer[1].enabled)
			{
				m_window_lines_drawn++;
			}
			m_previous_line = m_current_line;
			/* Set Mode 0 lcdstate */
			m_mode = 0;
			LCDSTAT &= 0xFC;
			m_oam_locked = UNLOCKED;
			m_vram_locked = UNLOCKED;
			/*
			    There seems to a kind of feature in the Game Boy hardware when the lowest bits of the
			    SCROLLX register equals 3 or 7, then the delayed M0 irq is triggered 4 cycles later
			    than usual.
			    The SGB probably has the same bug.
			*/
			if ((SCROLLX & 0x03) == 0x03)
			{
				m_scrollx_adjust += 4;
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LYXX_M0_SCX3);
				break;
			}
		case GB_LCD_STATE_LYXX_M0_SCX3:
			/* Generate lcd interrupt if requested */
			if (!m_mode_irq && (LCDSTAT & 0x08) &&
					((!m_line_irq && m_delayed_line_irq) || !(LCDSTAT & 0x40)))
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(196 - m_scrollx_adjust - m_sprite_cycles), GB_LCD_STATE_LYXX_M0_PRE_INC);
			break;
		case GB_LCD_STATE_LYXX_M0_PRE_INC:  /* Just before incrementing the line counter go to mode 2 internally */
			if (CURLINE < 143)
			{
				m_mode = 2;
				m_triggering_mode_irq = (LCDSTAT & 0x20) ? 1 : 0;
				if (m_triggering_mode_irq)
				{
					if (!m_mode_irq)
					{
						if (!m_line_irq && !m_delayed_line_irq)
						{
							m_mode_irq = 1;
							m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
						}
					}
					else
					{
						m_mode_irq = 0;
					}
				}
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LYXX_M0_INC);
			break;
		case GB_LCD_STATE_LYXX_M0_INC:  /* Increment LY, stay in M0 for 4 more cycles */
			increment_scanline();
			m_delayed_line_irq = m_line_irq;
			m_triggering_line_irq = ((CMPLINE == CURLINE) && (LCDSTAT & 0x40)) ? 1 : 0;
			m_line_irq = 0;
			if (!m_mode_irq && !m_delayed_line_irq && m_triggering_line_irq && !m_triggering_mode_irq)
			{
				m_line_irq = m_triggering_line_irq;
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			/* Reset LY==LYC STAT bit */
			LCDSTAT &= 0xFB;
			/* Check if we're going into VBlank next */
			if (CURLINE == 144)
			{
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY9X_M1);
			}
			else
			{
				/* Internally switch to mode 2 */
				m_mode = 2;
				/* Generate lcd interrupt if requested */
				if (!m_mode_irq && m_triggering_mode_irq &&
						((!m_triggering_line_irq && !m_delayed_line_irq) || !(LCDSTAT & 0x40)))
				{
					m_mode_irq = 1;
					m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
				}
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LYXX_M2);
			}
			break;
		case GB_LCD_STATE_LY00_M2:      /* Switch to mode 2 on line #0 */
			/* Set Mode 2 lcdstate */
			m_mode = 2;
			LCDSTAT = (LCDSTAT & 0xFC) | 0x02;
			m_oam_locked = LOCKED;
			/* Generate lcd interrupt if requested */
			if ((LCDSTAT & 0x20) && !m_line_irq)
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			/* Check for regular compensation of x-scroll register */
			m_scrollx_adjust = (SCROLLX & 0x04) ? 4 : 0;
			/* Mode 2 lasts approximately 80 clock cycles */
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(80), GB_LCD_STATE_LYXX_M3);
			break;
		case GB_LCD_STATE_LYXX_M2:      /* Switch to mode 2 */
			/* Update STAT register to the correct state */
			LCDSTAT = (LCDSTAT & 0xFC) | 0x02;
			m_oam_locked = LOCKED;
			/* Generate lcd interrupt if requested */
			if ((m_delayed_line_irq && m_triggering_line_irq && !(LCDSTAT & 0x20)) ||
					(!m_mode_irq && !m_line_irq && !m_delayed_line_irq && m_triggering_mode_irq))
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			m_line_irq = m_triggering_line_irq;
			m_triggering_mode_irq = 0;
			/* Check if LY==LYC STAT bit should be set */
			if (CURLINE == CMPLINE)
			{
				LCDSTAT |= 0x04;
			}
			/* Check for regular compensation of x-scroll register */
			m_scrollx_adjust = (SCROLLX & 0x04) ? 4 : 0;
			/* Mode 2 last for approximately 80 clock cycles */
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(80), GB_LCD_STATE_LYXX_M3);
			break;
		case GB_LCD_STATE_LYXX_M3:      /* Switch to mode 3 */
			select_sprites();
			m_sprite_cycles = gb_sprite_cycles[m_sprCount];
			/* Set Mode 3 lcdstate */
			m_mode = 3;
			LCDSTAT = (LCDSTAT & 0xFC) | 0x03;
			m_vram_locked = LOCKED;
			/* Check for compensations of x-scroll register */
			/* Mode 3 lasts for approximately 172+cycles needed to handle sprites clock cycles */
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(168 + m_scrollx_adjust + m_sprite_cycles), GB_LCD_STATE_LYXX_PRE_M0);
			m_start_x = -1;
			break;
		case GB_LCD_STATE_LY9X_M1:      /* Switch to or stay in mode 1 */
			if (CURLINE == 144)
			{
				/* Trigger VBlank interrupt */
				m_maincpu->set_input_line(VBL_INT, ASSERT_LINE);
				/* Set VBlank lcdstate */
				m_mode = 1;
				LCDSTAT = (LCDSTAT & 0xFC) | 0x01;
				/* Trigger LCD interrupt if requested */
				if (LCDSTAT & 0x10)
				{
					m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
				}
			}
			/* Check if LY==LYC STAT bit should be set */
			if (CURLINE == CMPLINE)
			{
				LCDSTAT |= 0x04;
			}
			if (m_delayed_line_irq && m_triggering_line_irq)
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(452), GB_LCD_STATE_LY9X_M1_INC);
			break;
		case GB_LCD_STATE_LY9X_M1_INC:      /* Increment scanline counter */
			increment_scanline();
			m_delayed_line_irq = m_line_irq;
			m_triggering_line_irq = ((CMPLINE == CURLINE) && (LCDSTAT & 0x40)) ? 1 : 0;
			m_line_irq = 0;
			if (!m_delayed_line_irq && m_triggering_line_irq)
			{
				m_line_irq = m_triggering_line_irq;
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			/* Reset LY==LYC STAT bit */
			LCDSTAT &= 0xFB;
			if (m_current_line == 153)
			{
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY00_M1);
			}
			else
			{
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY9X_M1);
			}
			break;
		case GB_LCD_STATE_LY00_M1:      /* we stay in VBlank but current line counter should already be incremented */
			/* Check LY=LYC for line #153 */
			if (m_delayed_line_irq)
			{
				if (m_triggering_line_irq)
				{
					m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
				}
			}
			m_delayed_line_irq = m_delayed_line_irq | m_line_irq;
			if (CURLINE == CMPLINE)
			{
				LCDSTAT |= 0x04;
			}
			increment_scanline();
			m_triggering_line_irq = ((CMPLINE == CURLINE) && (LCDSTAT & 0x40)) ? 1 : 0;
			m_line_irq = 0;
			LCDSTAT &= 0xFB;
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4/*8*/), GB_LCD_STATE_LY00_M1_1);
			break;
		case GB_LCD_STATE_LY00_M1_1:
			if (!m_delayed_line_irq && m_triggering_line_irq)
			{
				m_line_irq = m_triggering_line_irq;
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY00_M1_2);
			break;
		case GB_LCD_STATE_LY00_M1_2:    /* Rest of line #0 during VBlank */
			if (m_delayed_line_irq && m_triggering_line_irq)
			{
				m_line_irq = m_triggering_line_irq;
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			if (CURLINE == CMPLINE)
			{
				LCDSTAT |= 0x04;
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(444), GB_LCD_STATE_LY00_M0);
			break;
		case GB_LCD_STATE_LY00_M0:      /* The STAT register seems to go to 0 for about 4 cycles */
			/* Set Mode 0 lcdstat */
			m_mode = 0;
			LCDSTAT = (LCDSTAT & 0xFC);
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY00_M2);
			break;
		}
	}
	else
	{
		increment_scanline();
		if (m_current_line < 144)
		{
			update_scanline();
		}
		m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(456));
	}
}


// CGB specific code

void cgb_lcd_device::hdma_trans(UINT16 length)
{
	UINT16 src, dst;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	src = ((UINT16)HDMA1 << 8) | (HDMA2 & 0xF0);
	dst = ((UINT16)(HDMA3 & 0x1F) << 8) | (HDMA4 & 0xF0);
	dst |= 0x8000;
	while (length > 0)
	{
		space.write_byte(dst++, space.read_byte(src++));
		length--;
	}
	HDMA1 = src >> 8;
	HDMA2 = src & 0xF0;
	HDMA3 = 0x1f & (dst >> 8);
	HDMA4 = dst & 0xF0;
	HDMA5--;
	if ((HDMA5 & 0x7f) == 0x7f)
	{
		HDMA5 = 0xff;
		m_hdma_enabled = 0;
	}
}


TIMER_CALLBACK_MEMBER(cgb_lcd_device::lcd_timer_proc)
{
	static const int cgb_sprite_cycles[] = { 0, 8, 20, 32, 44, 52, 64, 76, 88, 96, 108 };

	m_state = param;

	if (LCDCONT & 0x80)
	{
		switch (m_state)
		{
		case GB_LCD_STATE_LYXX_PRE_M0:  /* Just before switching to mode 0 */
			m_mode = 0;
			if (LCDSTAT & 0x08)
			{
				if (!m_mode_irq)
				{
					if (!m_line_irq && !m_delayed_line_irq)
					{
						m_mode_irq = 1;
						m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
					}
				}
				else
				{
					m_mode_irq = 0;
				}
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LYXX_M0);
			break;
		case GB_LCD_STATE_LYXX_M0:      /* Switch to mode 0 */
			/* update current scanline */
			update_scanline();
			/* Increment the number of window lines drawn if enabled */
			if (m_layer[1].enabled)
			{
				m_window_lines_drawn++;
			}
			m_previous_line = m_current_line;
			/* Set Mode 0 lcdstate */
			m_mode = 0;
			LCDSTAT &= 0xFC;
			m_oam_locked = UNLOCKED;
			m_vram_locked = UNLOCKED;
			/*
			    There seems to a kind of feature in the Game Boy hardware when the lowest bits of the
			    SCROLLX register equals 3 or 7, then the delayed M0 irq is triggered 4 cycles later
			    than usual.
			    The SGB probably has the same bug.
			*/
			m_triggering_mode_irq = (LCDSTAT & 0x08) ? 1 : 0;
			if ((SCROLLX & 0x03) == 0x03)
			{
				m_scrollx_adjust += 4;
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LYXX_M0_SCX3);
				break;
			}
		case GB_LCD_STATE_LYXX_M0_SCX3:
			/* Generate lcd interrupt if requested */
			if (!m_mode_irq && m_triggering_mode_irq &&
					((!m_line_irq && m_delayed_line_irq) || !(LCDSTAT & 0x40)))
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
				m_triggering_mode_irq = 0;
			}
			if ((SCROLLX & 0x03) == 0x03)
			{
				m_pal_locked = UNLOCKED;
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LYXX_M0_GBC_PAL);
			break;
		case GB_LCD_STATE_LYXX_M0_GBC_PAL:
			m_pal_locked = UNLOCKED;
			/* Check for HBLANK DMA */
			if (m_hdma_enabled)
			{
				hdma_trans(0x10);
//              cpunum_set_reg(0, LR35902_DMA_CYCLES, 36);
			}
			else
			{
				m_hdma_possible = 1;
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(192 - m_scrollx_adjust - m_sprite_cycles), GB_LCD_STATE_LYXX_M0_PRE_INC);
			break;
		case GB_LCD_STATE_LYXX_M0_PRE_INC:  /* Just before incrementing the line counter go to mode 2 internally */
			m_cmp_line = CMPLINE;
			if (CURLINE < 143)
			{
				m_mode = 2;
				if (LCDSTAT & 0x20)
				{
					if (!m_mode_irq)
					{
						if (!m_line_irq && !m_delayed_line_irq)
						{
							m_mode_irq = 1;
							m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
						}
					}
					else
					{
						m_mode_irq = 0;
					}
				}
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LYXX_M0_INC);
			break;
		case GB_LCD_STATE_LYXX_M0_INC:  /* Increment LY, stay in M0 for 4 more cycles */
			increment_scanline();
			m_delayed_line_irq = m_line_irq;
			m_triggering_line_irq = ((m_cmp_line == CURLINE) && (LCDSTAT & 0x40)) ? 1 : 0;
			m_line_irq = 0;
			if (!m_mode_irq && !m_delayed_line_irq && m_triggering_line_irq && !(LCDSTAT & 0x20))
			{
				m_line_irq = m_triggering_line_irq;
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			m_hdma_possible = 0;
			/* Check if we're going into VBlank next */
			if (CURLINE == 144)
			{
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY9X_M1);
			}
			else
			{
				/* Internally switch to mode 2 */
				m_mode = 2;
				/* Generate lcd interrupt if requested */
				if (!m_mode_irq && (LCDSTAT & 0x20) &&
						((!m_triggering_line_irq && !m_delayed_line_irq) || !(LCDSTAT & 0x40)))
				{
					m_mode_irq = 1;
					m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
				}
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LYXX_M2);
			}
			break;
		case GB_LCD_STATE_LY00_M2:      /* Switch to mode 2 on line #0 */
			/* Set Mode 2 lcdstate */
			m_mode = 2;
			LCDSTAT = (LCDSTAT & 0xFC) | 0x02;
			m_oam_locked = LOCKED;
			/* Generate lcd interrupt if requested */
			if ((LCDSTAT & 0x20) && !m_line_irq)
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			/* Check for regular compensation of x-scroll register */
			m_scrollx_adjust = (SCROLLX & 0x04) ? 4 : 0;
			/* Mode 2 lasts approximately 80 clock cycles */
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(80), GB_LCD_STATE_LYXX_M3);
			break;
		case GB_LCD_STATE_LYXX_M2:      /* Switch to mode 2 */
			/* Update STAT register to the correct state */
			LCDSTAT = (LCDSTAT & 0xFC) | 0x02;
			m_oam_locked = LOCKED;
			/* Generate lcd interrupt if requested */
			if ((m_delayed_line_irq && m_triggering_line_irq && !(LCDSTAT & 0x20)) ||
					(!m_mode_irq && !m_line_irq && !m_delayed_line_irq && (LCDSTAT & 0x20)))
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			m_line_irq = m_triggering_line_irq;
			/* Check if LY==LYC STAT bit should be set */
			if (CURLINE == CMPLINE)
			{
				LCDSTAT |= 0x04;
			}
			else
			{
				LCDSTAT &= ~0x04;
			}
			/* Check for regular compensation of x-scroll register */
			m_scrollx_adjust = (SCROLLX & 0x04) ? 4 : 0;
			/* Mode 2 last for approximately 80 clock cycles */
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(80), GB_LCD_STATE_LYXX_M3);
			break;
		case GB_LCD_STATE_LYXX_M3:      /* Switch to mode 3 */
			select_sprites();
			m_sprite_cycles = cgb_sprite_cycles[m_sprCount];
			/* Set Mode 3 lcdstate */
			m_mode = 3;
			LCDSTAT = (LCDSTAT & 0xFC) | 0x03;
			m_vram_locked = LOCKED;
			m_pal_locked = LOCKED;
			/* Check for compensations of x-scroll register */
			/* Mode 3 lasts for approximately 172+cycles needed to handle sprites clock cycles */
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(168 + m_scrollx_adjust + m_sprite_cycles), GB_LCD_STATE_LYXX_PRE_M0);
			m_start_x = -1;
			break;
		case GB_LCD_STATE_LY9X_M1:      /* Switch to or stay in mode 1 */
			if (CURLINE == 144)
			{
				/* Trigger VBlank interrupt */
				m_maincpu->set_input_line(VBL_INT, ASSERT_LINE);
				/* Set VBlank lcdstate */
				m_mode = 1;
				LCDSTAT = (LCDSTAT & 0xFC) | 0x01;
				/* Trigger LCD interrupt if requested */
				if (LCDSTAT & 0x10)
				{
					m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
				}
			}
			/* Check if LY==LYC STAT bit should be set */
			if (CURLINE == CMPLINE)
			{
				LCDSTAT |= 0x04;
			}
			else
			{
				LCDSTAT &= ~0x04;
			}
			if (m_delayed_line_irq && m_triggering_line_irq)
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(452), GB_LCD_STATE_LY9X_M1_INC);
			break;
		case GB_LCD_STATE_LY9X_M1_INC:      /* Increment scanline counter */
			increment_scanline();
			m_delayed_line_irq = m_line_irq;
			m_triggering_line_irq = ((CMPLINE == CURLINE) && (LCDSTAT & 0x40)) ? 1 : 0;
			m_line_irq = 0;
			if (!m_delayed_line_irq && m_triggering_line_irq)
			{
				m_line_irq = m_triggering_line_irq;
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			if (m_current_line == 153)
			{
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY00_M1);
			}
			else
			{
				m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY9X_M1);
			}
			break;
		case GB_LCD_STATE_LY00_M1:      /* we stay in VBlank but current line counter should already be incremented */
			/* Check LY=LYC for line #153 */
			if (m_delayed_line_irq)
			{
				if (m_triggering_line_irq)
				{
					m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
				}
			}
			m_delayed_line_irq = m_delayed_line_irq | m_line_irq;
			if (CURLINE == CMPLINE)
			{
				LCDSTAT |= 0x04;
			}
			else
			{
				LCDSTAT &= ~0x04;
			}
			increment_scanline();
			m_triggering_line_irq = ((CMPLINE == CURLINE) && (LCDSTAT & 0x40)) ? 1 : 0;
			m_line_irq = 0;
			LCDSTAT &= 0xFB;
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY00_M1_1);
			break;
		case GB_LCD_STATE_LY00_M1_1:
			if (!m_delayed_line_irq && m_triggering_line_irq)
			{
				m_line_irq = m_triggering_line_irq;
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY00_M1_2);
			break;
		case GB_LCD_STATE_LY00_M1_2:    /* Rest of line #0 during VBlank */
			if (m_delayed_line_irq && m_triggering_line_irq)
			{
				m_line_irq = m_triggering_line_irq;
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			if (CURLINE == CMPLINE)
			{
				LCDSTAT |= 0x04;
			}
			else
			{
				LCDSTAT &= ~0x04;
			}
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(444), GB_LCD_STATE_LY00_M0);
			break;
		case GB_LCD_STATE_LY00_M0:      /* The STAT register seems to go to 0 for about 4 cycles */
			/* Set Mode 0 lcdstat */
			m_mode = 0;
			m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(4), GB_LCD_STATE_LY00_M2);
			break;
		}
	}
	else
	{
		increment_scanline();
		if (m_current_line < 144)
		{
			update_scanline();
		}
		m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(456));
	}
}


void gb_lcd_device::lcd_switch_on()
{
	m_current_line = 0;
	m_previous_line = 153;
	m_window_lines_drawn = 0;
	m_line_irq = 0;
	m_delayed_line_irq = 0;
	m_mode = 0;
	m_oam_locked = LOCKED;   /* TODO: Investigate whether this OAM locking is correct. */
	/* Check for LY=LYC coincidence */
	if (CURLINE == CMPLINE)
	{
		LCDSTAT |= 0x04;
		/* Generate lcd interrupt if requested */
		if (LCDSTAT & 0x40)
		{
			m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
		}
	}
	m_state = GB_LCD_STATE_LY00_M2;
	m_lcd_timer->adjust(m_maincpu->cycles_to_attotime(80), GB_LCD_STATE_LYXX_M3);
}




READ8_MEMBER(gb_lcd_device::vram_r)
{
	return (m_vram_locked == LOCKED) ? 0xff : m_vram[offset + (m_vram_bank * 0x2000)];
}

WRITE8_MEMBER(gb_lcd_device::vram_w)
{
	if (m_vram_locked == LOCKED)
		return;

	m_vram[offset + (m_vram_bank * 0x2000)] = data;
}

READ8_MEMBER(gb_lcd_device::oam_r)
{
	return (m_oam_locked == LOCKED) ? 0xff : m_oam[offset];
}

WRITE8_MEMBER(gb_lcd_device::oam_w)
{
	if (m_oam_locked == LOCKED || offset >= 0xa0)
		return;

	m_oam[offset] = data;
}



READ8_MEMBER(gb_lcd_device::video_r)
{
	return m_vid_regs[offset];
}

WRITE8_MEMBER(gb_lcd_device::video_w)
{
	switch (offset)
	{
	case 0x00:                      /* LCDC - LCD Control */
		m_gb_chrgen_offs = (data & 0x10) ? 0x0000 : 0x0800;
		m_gb_tile_no_mod = (data & 0x10) ? 0x00 : 0x80;
		m_gb_bgdtab_offs = (data & 0x08) ? 0x1c00 : 0x1800;
		m_gb_wndtab_offs = (data & 0x40) ? 0x1c00 : 0x1800;
		/* if LCD controller is switched off, set STAT and LY to 00 */
		if (!(data & 0x80))
		{
			LCDSTAT &= ~0x03;
			CURLINE = 0;
			m_oam_locked = UNLOCKED;
			m_vram_locked = UNLOCKED;
		}
		/* If LCD is being switched on */
		if (!(LCDCONT & 0x80) && (data & 0x80))
		{
			lcd_switch_on();
		}
		break;
	case 0x01:                      /* STAT - LCD Status */
		data = 0x80 | (data & 0x78) | (LCDSTAT & 0x07);
		/*
		   Check for the STAT bug:
		   Writing to STAT when the LCD controller is active causes a STAT
		   interrupt to be triggered.
		 */
		if (LCDCONT & 0x80)
		{
			/* Triggers seen so far:
			   - 0x40 -> 0x00 - trigger
			   - 0x00 -> 0x08 - trigger
			   - 0x08 -> 0x00 - don't trigger
			   - 0x00 -> 0x20 (mode 3) - trigger
			   - 0x00 -> 0x60 (mode 2) - don't trigger
			   - 0x20 -> 0x60 (mode 3) - trigger
			   - 0x20 -> 0x40 (mode 3) - trigger
			   - 0x40 -> 0x20 (mode 2) - don't trigger
			   - 0x40 -> 0x08 (mode 0) - don't trigger
			   - 0x00 -> 0x40 - trigger only if LY==LYC
			   - 0x20 -> 0x00/0x08/0x10/0x20/0x40 (mode 2, after m2int) - don't trigger
			   - 0x20 -> 0x00/0x08/0x10/0x20/0x40 (mode 3, after m2int) - don't trigger
			*/
			if (!m_mode_irq && ((m_mode == 1) ||
				((LCDSTAT & 0x40) && !(data & 0x68)) ||
				(!(LCDSTAT & 0x40) && (data & 0x40) && (LCDSTAT & 0x04)) ||
				(!(LCDSTAT & 0x48) && (data & 0x08)) ||
				((LCDSTAT & 0x60) == 0x00 && (data & 0x60) == 0x20) ||
				((LCDSTAT & 0x60) == 0x20 && (data & 0x40))
				))
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			/*
			   - 0x20 -> 0x08/0x18/0x28/0x48 (mode 0, after m2int) - trigger
			   - 0x20 -> 0x00/0x10/0x20/0x40 (mode 0, after m2int) - trigger (stat bug)
			   - 0x00 -> 0xXX (mode 0) - trigger stat bug
			*/
			if (m_mode_irq && m_mode == 0)
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
		}
		break;
	case 0x04:                      /* LY - LCD Y-coordinate */
		return;
	case 0x05:                      /* LYC */
		if (CMPLINE != data)
		{
			if (CURLINE == data)
			{
				if (m_state != GB_LCD_STATE_LYXX_M0_INC && m_state != GB_LCD_STATE_LY9X_M1_INC)
				{
					LCDSTAT |= 0x04;
					/* Generate lcd interrupt if requested */
					if (LCDSTAT & 0x40)
					{
						m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
					}
				}
			}
			else
			{
				LCDSTAT &= 0xFB;
				m_triggering_line_irq = 0;
			}
		}
		break;
	case 0x06:                      /* DMA - DMA Transfer and Start Address */
		{
			UINT8 *P = m_oam.get();
			offset = (UINT16) data << 8;
			for (data = 0; data < 0xA0; data++)
				*P++ = space.read_byte(offset++);
		}
		return;
	case 0x07:                      /* BGP - Background Palette */
		update_scanline();
		m_gb_bpal[0] = data & 0x3;
		m_gb_bpal[1] = (data & 0xC) >> 2;
		m_gb_bpal[2] = (data & 0x30) >> 4;
		m_gb_bpal[3] = (data & 0xC0) >> 6;
		break;
	case 0x08:                      /* OBP0 - Object Palette 0 */
//      update_scanline();
		m_gb_spal0[0] = data & 0x3;
		m_gb_spal0[1] = (data & 0xC) >> 2;
		m_gb_spal0[2] = (data & 0x30) >> 4;
		m_gb_spal0[3] = (data & 0xC0) >> 6;
		break;
	case 0x09:                      /* OBP1 - Object Palette 1 */
//      update_scanline();
		m_gb_spal1[0] = data & 0x3;
		m_gb_spal1[1] = (data & 0xC) >> 2;
		m_gb_spal1[2] = (data & 0x30) >> 4;
		m_gb_spal1[3] = (data & 0xC0) >> 6;
		break;
	case 0x02:                      /* SCY - Scroll Y */
	case 0x03:                      /* SCX - Scroll X */
		update_scanline();
		break;
	case 0x0A:                      /* WY - Window Y position */
	case 0x0B:                      /* WX - Window X position */
		break;
	default:                        /* Unknown register, no change */
		return;
	}
	m_vid_regs[offset] = data;
}

READ8_MEMBER(cgb_lcd_device::video_r)
{
	switch (offset)
	{
	case 0x11:  /* FF51 */
	case 0x12:  /* FF52 */
	case 0x13:  /* FF53 */
	case 0x14:  /* FF54 */
		return 0xFF;
	case 0x29:  /* FF69 */
	case 0x2B:  /* FF6B */
		if (m_pal_locked == LOCKED)
		{
			return 0xFF;
		}
		break;
	}
	return m_vid_regs[offset];
}

WRITE8_MEMBER(cgb_lcd_device::video_w)
{
	switch (offset)
	{
	case 0x00:      /* LCDC - LCD Control */
		m_gb_chrgen_offs = (data & 0x10) ? 0x0000 : 0x0800;
		m_gbc_chrgen_offs = (data & 0x10) ? 0x2000 : 0x2800;
		m_gb_tile_no_mod = (data & 0x10) ? 0x00 : 0x80;
		m_gb_bgdtab_offs = (data & 0x08) ? 0x1c00 : 0x1800;
		m_gbc_bgdtab_offs = (data & 0x08) ? 0x3c00 : 0x3800;
		m_gb_wndtab_offs = (data & 0x40) ? 0x1c00 : 0x1800;
		m_gbc_wndtab_offs = (data & 0x40) ? 0x3c00 : 0x3800;
		/* if LCD controller is switched off, set STAT to 00 */
		if (!(data & 0x80))
		{
			LCDSTAT &= ~0x03;
			CURLINE = 0;
			m_oam_locked = UNLOCKED;
			m_vram_locked = UNLOCKED;
			m_pal_locked = UNLOCKED;
		}
		/* If LCD is being switched on */
		if (!(LCDCONT & 0x80) && (data & 0x80))
		{
			lcd_switch_on();
		}
		break;
	case 0x01:      /* STAT - LCD Status */
		data = 0x80 | (data & 0x78) | (LCDSTAT & 0x07);
		if (LCDCONT & 0x80)
		{
			/*
			   - 0x20 -> 0x08/0x18/0x28/0x48 (mode 0, after m2int) - trigger
			*/
			if (m_mode_irq && m_mode == 0 && (LCDSTAT & 0x28) == 0x20 && (data & 0x08))
			{
				m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
			}
			/* Check if line irqs are being disabled */
			if (!(data & 0x40))
			{
				m_delayed_line_irq = 0;
			}
			/* Check if line irqs are being enabled */
			if (!(LCDSTAT & 0x40) && (data & 0x40))
			{
				if (CMPLINE == CURLINE)
				{
					m_line_irq = 1;
					m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
				}
			}
		}
		break;
	case 0x05:                      /* LYC */
		if (CMPLINE != data)
		{
			if ((m_state != GB_LCD_STATE_LYXX_M0_PRE_INC && CURLINE == data) ||
					(m_state == GB_LCD_STATE_LYXX_M0_INC && m_triggering_line_irq))
			{
				LCDSTAT |= 0x04;
				/* Generate lcd interrupt if requested */
				if (LCDSTAT & 0x40)
				{
					m_maincpu->set_input_line(LCD_INT, ASSERT_LINE);
				}
			}
			else
			{
				LCDSTAT &= 0xFB;
				m_triggering_line_irq = 0;
				m_cmp_line = data;
			}
		}
		break;
	case 0x07:      /* BGP - GB background palette */
		update_scanline();
		m_gb_bpal[0] = data & 0x3;
		m_gb_bpal[1] = (data & 0xC) >> 2;
		m_gb_bpal[2] = (data & 0x30) >> 4;
		m_gb_bpal[3] = (data & 0xC0) >> 6;
		break;
	case 0x08:      /* OBP0 - GB Object 0 palette */
		m_gb_spal0[0] = data & 0x3;
		m_gb_spal0[1] = (data & 0xC) >> 2;
		m_gb_spal0[2] = (data & 0x30) >> 4;
		m_gb_spal0[3] = (data & 0xC0) >> 6;
		break;
	case 0x09:      /* OBP1 - GB Object 1 palette */
		m_gb_spal1[0] = data & 0x3;
		m_gb_spal1[1] = (data & 0xC) >> 2;
		m_gb_spal1[2] = (data & 0x30) >> 4;
		m_gb_spal1[3] = (data & 0xC0) >> 6;
		break;
	case 0x0c:      /* Undocumented register involved in selecting gb/gbc mode */
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		break;
	case 0x0F:      /* VBK - VRAM bank select */
		m_vram_bank = data & 0x01;
		data |= 0xFE;
		break;
	case 0x11:      /* HDMA1 - HBL General DMA - Source High */
		break;
	case 0x12:      /* HDMA2 - HBL General DMA - Source Low */
		data &= 0xF0;
		break;
	case 0x13:      /* HDMA3 - HBL General DMA - Destination High */
		data &= 0x1F;
		break;
	case 0x14:      /* HDMA4 - HBL General DMA - Destination Low */
		data &= 0xF0;
		break;
	case 0x15:      /* HDMA5 - HBL General DMA - Mode, Length */
		if (!(data & 0x80))
		{
			if (m_hdma_enabled)
			{
				m_hdma_enabled = 0;
				data = HDMA5 & 0x80;
			}
			else
			{
				/* General DMA */
				hdma_trans(((data & 0x7F) + 1) * 0x10);
//              cpunum_set_reg(0, LR35902_DMA_CYCLES, 4 + (((data & 0x7F) + 1) * 32));
				data = 0xff;
			}
		}
		else
		{
			/* H-Blank DMA */
			m_hdma_enabled = 1;
			data &= 0x7f;
			m_vid_regs[offset] = data;
			/* Check if HDMA should be immediately performed */
			if (m_hdma_possible)
			{
				hdma_trans(0x10);
//              cpunum_set_reg(0, LR35902_DMA_CYCLES, 36);
				m_hdma_possible = 0;
			}
		}
		break;
	case 0x28:      /* BCPS - Background palette specification */
		GBCBCPS = data;
		if (data & 0x01)
			GBCBCPD = m_cgb_bpal[(data >> 1) & 0x1F] >> 8;
		else
			GBCBCPD = m_cgb_bpal[(data >> 1) & 0x1F] & 0xFF;
		break;
	case 0x29:      /* BCPD - background palette data */
		if (m_pal_locked == LOCKED)
		{
			return;
		}
		GBCBCPD = data;
		if (GBCBCPS & 0x01)
			m_cgb_bpal[(GBCBCPS >> 1) & 0x1F] = ((data << 8) | (m_cgb_bpal[(GBCBCPS >> 1) & 0x1F] & 0xFF)) & 0x7FFF;
		else
			m_cgb_bpal[(GBCBCPS >> 1) & 0x1F] = ((m_cgb_bpal[(GBCBCPS >> 1) & 0x1F] & 0xFF00) | data) & 0x7FFF;
		if (GBCBCPS & 0x80)
		{
			GBCBCPS++;
			GBCBCPS &= 0xBF;
		}
		break;
	case 0x2A:      /* OCPS - Object palette specification */
		GBCOCPS = data;
		if (data & 0x01)
			GBCOCPD = m_cgb_spal[(data >> 1) & 0x1F] >> 8;
		else
			GBCOCPD = m_cgb_spal[(data >> 1) & 0x1F] & 0xFF;
		break;
	case 0x2B:      /* OCPD - Object palette data */
		if (m_pal_locked == LOCKED)
		{
			return;
		}
		GBCOCPD = data;
		if (GBCOCPS & 0x01)
			m_cgb_spal[(GBCOCPS >> 1) & 0x1F] = ((data << 8) | (m_cgb_spal[(GBCOCPS >> 1) & 0x1F] & 0xFF)) & 0x7FFF;
		else
			m_cgb_spal[(GBCOCPS >> 1) & 0x1F] = ((m_cgb_spal[(GBCOCPS >> 1) & 0x1F] & 0xFF00) | data) & 0x7FFF;
		if (GBCOCPS & 0x80)
		{
			GBCOCPS++;
			GBCOCPS &= 0xBF;
		}
		break;
	/* Undocumented registers */
	case 0x2C:
		/* bit 0 can be read/written */
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		data = 0xFE | (data & 0x01);
		if (data & 0x01)
		{
			m_gbc_mode = 0;
		}
		break;
	case 0x32:
	case 0x33:
	case 0x34:
		/* whole byte can be read/written */
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		break;
	case 0x35:
		/* bit 4-6 can be read/written */
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		data = 0x8F | (data & 0x70);
		break;
	case 0x36:
	case 0x37:
		logerror("Write to undocumented register: %X = %X\n", offset, data);
		return;
	default:
		/* we didn't handle the write, so pass it to the GB handler */
		gb_lcd_device::video_w(space, offset, data);
		return;
	}

	m_vid_regs[offset] = data;
}

// Super Game Boy

void sgb_lcd_device::sgb_io_write_pal(int offs, UINT8 *data)
{
	switch (offs)
	{
		case 0x00:  /* PAL01 */
			m_sgb_pal[0 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[0 * 4 + 1] = data[3] | (data[4] << 8);
			m_sgb_pal[0 * 4 + 2] = data[5] | (data[6] << 8);
			m_sgb_pal[0 * 4 + 3] = data[7] | (data[8] << 8);
			m_sgb_pal[1 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[1 * 4 + 1] = data[9] | (data[10] << 8);
			m_sgb_pal[1 * 4 + 2] = data[11] | (data[12] << 8);
			m_sgb_pal[1 * 4 + 3] = data[13] | (data[14] << 8);
			break;
		case 0x01:  /* PAL23 */
			m_sgb_pal[2 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[2 * 4 + 1] = data[3] | (data[4] << 8);
			m_sgb_pal[2 * 4 + 2] = data[5] | (data[6] << 8);
			m_sgb_pal[2 * 4 + 3] = data[7] | (data[8] << 8);
			m_sgb_pal[3 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[3 * 4 + 1] = data[9] | (data[10] << 8);
			m_sgb_pal[3 * 4 + 2] = data[11] | (data[12] << 8);
			m_sgb_pal[3 * 4 + 3] = data[13] | (data[14] << 8);
			break;
		case 0x02:  /* PAL03 */
			m_sgb_pal[0 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[0 * 4 + 1] = data[3] | (data[4] << 8);
			m_sgb_pal[0 * 4 + 2] = data[5] | (data[6] << 8);
			m_sgb_pal[0 * 4 + 3] = data[7] | (data[8] << 8);
			m_sgb_pal[3 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[3 * 4 + 1] = data[9] | (data[10] << 8);
			m_sgb_pal[3 * 4 + 2] = data[11] | (data[12] << 8);
			m_sgb_pal[3 * 4 + 3] = data[13] | (data[14] << 8);
			break;
		case 0x03:  /* PAL12 */
			m_sgb_pal[1 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[1 * 4 + 1] = data[3] | (data[4] << 8);
			m_sgb_pal[1 * 4 + 2] = data[5] | (data[6] << 8);
			m_sgb_pal[1 * 4 + 3] = data[7] | (data[8] << 8);
			m_sgb_pal[2 * 4 + 0] = data[1] | (data[2] << 8);
			m_sgb_pal[2 * 4 + 1] = data[9] | (data[10] << 8);
			m_sgb_pal[2 * 4 + 2] = data[11] | (data[12] << 8);
			m_sgb_pal[2 * 4 + 3] = data[13] | (data[14] << 8);
			break;
		case 0x04:  /* ATTR_BLK */
		{
			UINT8 I, J, K, o;
			for( K = 0; K < data[1]; K++ )
			{
				o = K * 6;
				if( data[o + 2] & 0x1 )
				{
					for( I = data[ o + 4]; I <= data[o + 6]; I++ )
					{
						for( J = data[o + 5]; J <= data[o + 7]; J++ )
						{
							m_sgb_pal_map[I][J] = data[o + 3] & 0x3;
						}
					}
				}
			}
		}
			break;
		case 0x05:  /* ATTR_LIN */
		{
			UINT8 J, K;
			if( data[1] > 15 )
				data[1] = 15;
			for( K = 0; K < data[1]; K++ )
			{
				if( data[K + 1] & 0x80 )
				{
					for( J = 0; J < 20; J++ )
					{
						m_sgb_pal_map[J][data[K + 1] & 0x1f] = (data[K + 1] & 0x60) >> 5;
					}
				}
				else
				{
					for( J = 0; J < 18; J++ )
					{
						m_sgb_pal_map[data[K + 1] & 0x1f][J] = (data[K + 1] & 0x60) >> 5;
					}
				}
			}
		}
			break;
		case 0x06:  /* ATTR_DIV */
		{
			UINT8 I, J;
			if( data[1] & 0x40 ) /* Vertical */
			{
				for( I = 0; I < data[2]; I++ )
				{
					for( J = 0; J < 20; J++ )
					{
						m_sgb_pal_map[J][I] = (data[1] & 0xC) >> 2;
					}
				}
				for( J = 0; J < 20; J++ )
				{
					m_sgb_pal_map[J][data[2]] = (data[1] & 0x30) >> 4;
				}
				for( I = data[2] + 1; I < 18; I++ )
				{
					for( J = 0; J < 20; J++ )
					{
						m_sgb_pal_map[J][I] = data[1] & 0x3;
					}
				}
			}
			else /* Horizontal */
			{
				for( I = 0; I < data[2]; I++ )
				{
					for( J = 0; J < 18; J++ )
					{
						m_sgb_pal_map[I][J] = (data[1] & 0xC) >> 2;
					}
				}
				for( J = 0; J < 18; J++ )
				{
					m_sgb_pal_map[data[2]][J] = (data[1] & 0x30) >> 4;
				}
				for( I = data[2] + 1; I < 20; I++ )
				{
					for( J = 0; J < 18; J++ )
					{
						m_sgb_pal_map[I][J] = data[1] & 0x3;
					}
				}
			}
		}
			break;
		case 0x07:  /* ATTR_CHR */
		{
			UINT16 I, sets;
			UINT8 x, y;
			sets = (data[3] | (data[4] << 8) );
			if( sets > 360 )
				sets = 360;
			sets >>= 2;
			sets += 6;
			x = data[1];
			y = data[2];
			if( data[5] ) /* Vertical */
			{
				for( I = 6; I < sets; I++ )
				{
					m_sgb_pal_map[x][y++] = (data[I] & 0xC0) >> 6;
					if( y > 17 )
					{
						y = 0;
						x++;
						if( x > 19 )
							x = 0;
					}

					m_sgb_pal_map[x][y++] = (data[I] & 0x30) >> 4;
					if( y > 17 )
					{
						y = 0;
						x++;
						if( x > 19 )
							x = 0;
					}

					m_sgb_pal_map[x][y++] = (data[I] & 0xC) >> 2;
					if( y > 17 )
					{
						y = 0;
						x++;
						if( x > 19 )
							x = 0;
					}

					m_sgb_pal_map[x][y++] = data[I] & 0x3;
					if( y > 17 )
					{
						y = 0;
						x++;
						if( x > 19 )
							x = 0;
					}
				}
			}
			else /* horizontal */
			{
				for( I = 6; I < sets; I++ )
				{
					m_sgb_pal_map[x++][y] = (data[I] & 0xC0) >> 6;
					if( x > 19 )
					{
						x = 0;
						y++;
						if( y > 17 )
							y = 0;
					}

					m_sgb_pal_map[x++][y] = (data[I] & 0x30) >> 4;
					if( x > 19 )
					{
						x = 0;
						y++;
						if( y > 17 )
							y = 0;
					}

					m_sgb_pal_map[x++][y] = (data[I] & 0xC) >> 2;
					if( x > 19 )
					{
						x = 0;
						y++;
						if( y > 17 )
							y = 0;
					}

					m_sgb_pal_map[x++][y] = data[I] & 0x3;
					if( x > 19 )
					{
						x = 0;
						y++;
						if( y > 17 )
							y = 0;
					}
				}
			}
		}
			break;
		case 0x08:  /* SOUND */
			/* This command enables internal sound effects */
			/* Not Implemented */
			break;
		case 0x09:  /* SOU_TRN */
			/* This command sends data to the SNES sound processor.
			 We'll need to emulate that for this to be used */
			/* Not Implemented */
			break;
		case 0x0A:  /* PAL_SET */
		{
			UINT16 index_;

			/* Palette 0 */
			index_ = (UINT16)(data[1] | (data[2] << 8)) * 4;
			m_sgb_pal[0] = m_sgb_pal_data[index_];
			m_sgb_pal[1] = m_sgb_pal_data[index_ + 1];
			m_sgb_pal[2] = m_sgb_pal_data[index_ + 2];
			m_sgb_pal[3] = m_sgb_pal_data[index_ + 3];
			/* Palette 1 */
			index_ = (UINT16)(data[3] | (data[4] << 8)) * 4;
			m_sgb_pal[4] = m_sgb_pal_data[index_];
			m_sgb_pal[5] = m_sgb_pal_data[index_ + 1];
			m_sgb_pal[6] = m_sgb_pal_data[index_ + 2];
			m_sgb_pal[7] = m_sgb_pal_data[index_ + 3];
			/* Palette 2 */
			index_ = (UINT16)(data[5] | (data[6] << 8)) * 4;
			m_sgb_pal[8] = m_sgb_pal_data[index_];
			m_sgb_pal[9] = m_sgb_pal_data[index_ + 1];
			m_sgb_pal[10] = m_sgb_pal_data[index_ + 2];
			m_sgb_pal[11] = m_sgb_pal_data[index_ + 3];
			/* Palette 3 */
			index_ = (UINT16)(data[7] | (data[8] << 8)) * 4;
			m_sgb_pal[12] = m_sgb_pal_data[index_];
			m_sgb_pal[13] = m_sgb_pal_data[index_ + 1];
			m_sgb_pal[14] = m_sgb_pal_data[index_ + 2];
			m_sgb_pal[15] = m_sgb_pal_data[index_ + 3];
			/* Attribute File */
			if (data[9] & 0x40)
				m_sgb_window_mask = 0;
			m_sgb_atf = (data[9] & 0x3f) * (18 * 5);
			if (data[9] & 0x80)
			{
				for (int j = 0; j < 18; j++ )
				{
					for (int i = 0; i < 5; i++ )
					{
						m_sgb_pal_map[i * 4][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0xC0) >> 6;
						m_sgb_pal_map[(i * 4) + 1][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0x30) >> 4;
						m_sgb_pal_map[(i * 4) + 2][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0xC) >> 2;
						m_sgb_pal_map[(i * 4) + 3][j] = m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0x3;
					}
				}
			}
		}
			break;
		case 0x0B:  /* PAL_TRN */
		{
			UINT16 col;

			for (int i = 0; i < 2048; i++ )
			{
				col = (m_vram[0x0800 + (i * 2) + 1] << 8) | m_vram[0x0800 + (i * 2)];
				m_sgb_pal_data[i] = col;
			}
		}
			break;
		case 0x0C:  /* ATRC_EN */
			/* Not Implemented */
			break;
		case 0x0D:  /* TEST_EN */
			/* Not Implemented */
			break;
		case 0x0E:  /* ICON_EN */
			/* Not Implemented */
			break;
		case 0x0F:  /* DATA_SND */
			/* Not Implemented */
			break;
		case 0x10:  /* DATA_TRN */
			/* Not Implemented */
			break;
		case 0x12:  /* JUMP */
			/* Not Implemented */
			break;
		case 0x13:  /* CHR_TRN */
			if (data[1] & 0x1)
				memcpy(m_sgb_tile_data.get() + 4096, m_vram.get() + 0x0800, 4096);
			else
				memcpy(m_sgb_tile_data.get(), m_vram.get() + 0x0800, 4096);
			break;
		case 0x14:  /* PCT_TRN */
		{
			UINT16 col;
			if (m_sgb_border_hack)
			{
				memcpy(m_sgb_tile_map, m_vram.get() + 0x1000, 2048);
				for (int i = 0; i < 64; i++)
				{
					col = (m_vram[0x0800 + (i * 2) + 1 ] << 8) | m_vram[0x0800 + (i * 2)];
					m_sgb_pal[SGB_BORDER_PAL_OFFSET + i] = col;
				}
			}
			else /* Do things normally */
			{
				memcpy(m_sgb_tile_map, m_vram.get() + 0x0800, 2048);
				for (int i = 0; i < 64; i++)
				{
					col = (m_vram[0x1000 + (i * 2) + 1] << 8) | m_vram[0x1000 + (i * 2)];
					m_sgb_pal[SGB_BORDER_PAL_OFFSET + i] = col;
				}
			}
		}
			break;
		case 0x15:  /* ATTR_TRN */
			memcpy(m_sgb_atf_data, m_vram.get() + 0x0800, 4050);
			break;
		case 0x16:  /* ATTR_SET */
		{
			/* Attribute File */
			if (data[1] & 0x40)
				m_sgb_window_mask = 0;
			m_sgb_atf = (data[1] & 0x3f) * (18 * 5);
			for (int j = 0; j < 18; j++)
			{
				for (int i = 0; i < 5; i++)
				{
					m_sgb_pal_map[i * 4][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0xC0) >> 6;
					m_sgb_pal_map[(i * 4) + 1][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0x30) >> 4;
					m_sgb_pal_map[(i * 4) + 2][j] = (m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0xC) >> 2;
					m_sgb_pal_map[(i * 4) + 3][j] = m_sgb_atf_data[(j * 5) + m_sgb_atf + i] & 0x3;
				}
			}
		}
			break;
		case 0x17:  /* MASK_EN */
			m_sgb_window_mask = data[1];
			break;
		case 0x18:  /* OBJ_TRN */
			/* Not Implemnted */
			break;
		case 0x19:  /* ? */
			/* Called by: dkl,dkl2,dkl3,zeldadx
			 But I don't know what it is for. */
			/* Not Implemented */
			break;
		case 0x1E:  /* Used by bootrom to transfer the gb cart header */
			break;
		case 0x1F:  /* Used by bootrom to transfer the gb cart header */
			break;
		default:
			logerror( "SGB: Unknown Command 0x%02x!\n", data[0] >> 3 );
	}

}

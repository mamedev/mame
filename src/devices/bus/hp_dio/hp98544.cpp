// license:BSD-3-Clause
// copyright-holders:R. Belmont, Sven Schnelle
/***************************************************************************

  HP98544 high-resolution monochrome board

  VRAM at 0x200000, ROM and registers at 0x560000

***************************************************************************/

#include "emu.h"
#include "hp98544.h"
#include "screen.h"

#define HP98544_SCREEN_NAME   "98544_screen"
#define HP98544_ROM_REGION    "98544_rom"

#define VRAM_SIZE   (0x100000)

ROM_START( hp98544 )
	ROM_REGION( 0x2000, HP98544_ROM_REGION, 0 )
	ROM_LOAD( "98544_1818-1999.bin", 0x000000, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(HPDIO_98544, dio16_98544_device, "dio98544", "HP98544 high-res monochrome DIO video card")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(dio16_98544_device::device_add_mconfig)
	MCFG_SCREEN_ADD(HP98544_SCREEN_NAME, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, dio16_98544_device, screen_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
MACHINE_CONFIG_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *dio16_98544_device::device_rom_region() const
{
	return ROM_NAME( hp98544 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  dio16_98544_device - constructor
//-------------------------------------------------

dio16_98544_device::dio16_98544_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	dio16_98544_device(mconfig, HPDIO_98544, tag, owner, clock)
{
}

dio16_98544_device::dio16_98544_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_dio16_card_interface(mconfig, *this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dio16_98544_device::device_start()
{
	// set_nubus_device makes m_slot valid
	set_dio_device();

	m_rom = device().machine().root_device().memregion(this->subtag(HP98544_ROM_REGION).c_str())->base();

	m_vram.resize(VRAM_SIZE);
	m_dio->install_memory(0x200000, 0x2fffff, read16_delegate(FUNC(dio16_98544_device::vram_r), this),
							write16_delegate(FUNC(dio16_98544_device::vram_w), this));
	m_dio->install_memory(0x560000, 0x563fff, read16_delegate(FUNC(dio16_98544_device::rom_r), this),
							write16_delegate(FUNC(dio16_98544_device::rom_w), this));
	m_dio->install_memory(0x564000, 0x567fff, read16_delegate(FUNC(dio16_98544_device::ctrl_r), this),
							write16_delegate(FUNC(dio16_98544_device::ctrl_w), this));
	m_cursor_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dio16_98544_device::cursor_callback),this));
	m_cursor_timer->adjust(attotime::from_hz(3));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void dio16_98544_device::device_reset()
{
	memset(&m_vram[0], 0, VRAM_SIZE);

	m_palette[1] = rgb_t(255, 255, 255);
	m_palette[0] = rgb_t(0, 0, 0);
	m_pixel_replacement_rule = TOPCAT_REPLACE_RULE_SRC;
}

TIMER_CALLBACK_MEMBER(dio16_98544_device::cursor_callback)
{
        m_cursor_timer->adjust(attotime::from_hz(5));
	m_cursor_state ^= true;

	if (m_cursor_ctrl & 0x02) {
		for(int i = 0; i < m_cursor_width; i++) {
			m_vram[(m_cursor_y_pos * 512) + (m_cursor_x_pos + i)/2] = m_cursor_state ? 0xffff : 0;
			m_vram[((m_cursor_y_pos-1) * 512) + (m_cursor_x_pos + i)/2] = m_cursor_state ? 0xffff : 0;
			m_vram[((m_cursor_y_pos-2) * 512) + (m_cursor_x_pos + i)/2] = m_cursor_state ? 0xffff : 0;
		}
	}
}

void dio16_98544_device::update_cursor(int x, int y, uint8_t ctrl, uint8_t width)
{
	for(int i = 0; i < m_cursor_width; i++) {
		m_vram[(m_cursor_y_pos * 512) + (m_cursor_x_pos + i)/2] = 0;
		m_vram[((m_cursor_y_pos-1) * 512) + (m_cursor_x_pos + i)/2] = 0;
		m_vram[((m_cursor_y_pos-2) * 512) + (m_cursor_x_pos + i)/2] = 0;
	}
	m_cursor_x_pos = x;
	m_cursor_y_pos = y;
	m_cursor_ctrl = ctrl;
	m_cursor_width = width;
}

READ16_MEMBER(dio16_98544_device::vram_r)
{
	return m_vram[offset];
}

WRITE16_MEMBER(dio16_98544_device::vram_w)
{
//	execute_rule(data, (replacement_rule_t)m_pixel_replacement_rule, &data);
	COMBINE_DATA(&m_vram[offset]);
}

READ16_MEMBER(dio16_98544_device::rom_r)
{
	return 0xff00 | m_rom[offset];
}

// the video chip registers live here, so these writes are valid
WRITE16_MEMBER(dio16_98544_device::rom_w)
{
}

void dio16_98544_device::execute_rule(uint16_t src, replacement_rule_t rule, uint16_t *dst)
{
	switch(rule & 0x0f) {
	case TOPCAT_REPLACE_RULE_CLEAR:
		*dst = 0;
		break;
	case TOPCAT_REPLACE_RULE_SRC_AND_DST:
		*dst &= src;
		break;
	case TOPCAT_REPLACE_RULE_SRC_AND_NOT_DST:
		*dst = ~(*dst) & src;
		break;
	case TOPCAT_REPLACE_RULE_SRC:
		*dst = src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_AND_DST:
		*dst &= ~src;
		break;
	case TOPCAT_REPLACE_RULE_NOP:
		break;
	case TOPCAT_REPLACE_RULE_SRC_XOR_DST:
		*dst ^= src;
		break;
	case TOPCAT_REPLACE_RULE_SRC_OR_DST:
		*dst |= src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_AND_NOT_DST:
		*dst = ~(*dst) & ~src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_XOR_DST:
		*dst ^= ~src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_DST:
		*dst ^= 0xffff;
		break;
	case TOPCAT_REPLACE_RULE_SRC_OR_NOT_DST:
		*dst = src | ~(*dst);
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC:
		*dst = ~src;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_OR_DST:
		*dst = ~src | *dst;
		break;
	case TOPCAT_REPLACE_RULE_NOT_SRC_OR_NOT_DST:
		*dst = ~src | ~(*dst);
		break;
	case TOPCAT_REPLACE_RULE_SET:
		*dst = 0xffff;
		break;

	}
}

void dio16_98544_device::window_move(void)
{
	for(int line = 0; line < m_block_mover_pixel_height; line++) {
		for(int column = 0; column < m_block_mover_pixel_width; column++) {
			uint16_t sdata = m_vram[((m_source_y_pixel + line) * 1024 + (m_source_x_pixel + column))/2];
			uint16_t *ddata = &m_vram[((m_dst_y_pixel + line) * 1024 + (m_dst_x_pixel + column))/2];
			execute_rule(sdata, (replacement_rule_t)((m_move_replacement_rule >> 4) & 0x0f), ddata);
			execute_rule(sdata, (replacement_rule_t)(m_move_replacement_rule & 0x0f), ddata);
		}
	}
}

WRITE16_MEMBER(dio16_98544_device::ctrl_w)
{
	if (mem_mask == 0xff00)
		data >>= 8;

	if (mem_mask == 0x00ff) {
		logerror("%s: write ignored\n", __FUNCTION__);
		return;
	}

	switch(offset) {
	case TOPCAT_REG_VBLANK:
		m_vblank = data & 0xff;
		break;
	case TOPCAT_REG_WMOVE_ACTIVE:
		break;
	case TOPCAT_REG_VERT_RETRACE_INTRQ:
		m_vert_retrace_intrq = data;
		break;
	case TOPCAT_REG_WMOVE_INTRQ:
		m_wmove_intrq = data;
		break;
	case TOPCAT_REG_DISPLAY_PLANE_ENABLE:
		m_display_enable_planes = data;
		break;
	case TOPCAT_REG_DISPLAY_WRITE_ENABLE_PLANE:
		m_write_enable_plane = data;
		break;
	case TOPCAT_REG_DISPLAY_READ_ENABLE_PLANE:
		m_read_enable_plane = data;
		break;
	case TOPCAT_REG_FB_WRITE_ENABLE:
		m_fb_write_enable = data;
		break;
	case TOPCAT_REG_START_WMOVE:
		window_move();
		break;
	case TOPCAT_REG_ENABLE_BLINK_PLANES:
		logerror("ENABLE_BLINK_PLANES: %04x\n", data);
		m_enable_blink_planes = data;
		break;
	case TOPCAT_REG_ENABLE_ALT_FRAME:
		logerror("ENABLE_ALT_PLANE: %04x\n", data);
		m_enable_alt_frame = data;
		break;
	case TOPCAT_REG_PIXEL_REPLACE_RULE:
		logerror("PIXEL RR: data %04X mask %04X\n", data, mem_mask);
		m_pixel_replacement_rule = data;
		break;
	case TOPCAT_REG_MOVE_REPLACE_RULE:
		logerror("MOVE RR: data %04X mask %04X\n", data, mem_mask);
		m_move_replacement_rule = data;
		break;
	case TOPCAT_REG_SOURCE_X_PIXEL:
		m_source_x_pixel = data;
		break;
	case TOPCAT_REG_SOURCE_Y_PIXEL:
		m_source_y_pixel = data;
		break;
	case TOPCAT_REG_DST_X_PIXEL:
		m_dst_x_pixel = data;
		break;
	case TOPCAT_REG_DST_Y_PIXEL:
		m_dst_y_pixel = data;
		break;
	case TOPCAT_REG_BLOCK_MOVER_PIXEL_WIDTH:
		m_block_mover_pixel_width = data;
		break;
	case TOPCAT_REG_BLOCK_MOVER_PIXEL_HEIGHT:
		m_block_mover_pixel_height = data;
		break;
	case TOPCAT_REG_CURSOR_CNTL:
		update_cursor(m_cursor_x_pos, m_cursor_y_pos, data, m_cursor_width);
		break;
	case TOPCAT_REG_CURSOR_X_POS:
		update_cursor(data, m_cursor_y_pos, m_cursor_ctrl, m_cursor_width);
		break;
	case TOPCAT_REG_CURSOR_Y_POS:
		update_cursor(m_cursor_x_pos, data, m_cursor_ctrl, m_cursor_width);
		break;
	case TOPCAT_REG_CURSOR_WIDTH:
		update_cursor(m_cursor_x_pos, m_cursor_y_pos, m_cursor_ctrl, data);
		break;
	default:
		logerror("unknown register: %02X = %04x\n", offset, data, mem_mask);
		break;
	}
}

READ16_MEMBER(dio16_98544_device::ctrl_r)
{
	uint16_t ret = 0xffff;

	switch(offset) {
	case TOPCAT_REG_VBLANK:
		ret = m_vblank;
		break;
	case TOPCAT_REG_WMOVE_ACTIVE:
		ret = m_wmove_active;
		break;
	case TOPCAT_REG_VERT_RETRACE_INTRQ:
		ret = m_vert_retrace_intrq;
		break;
	case TOPCAT_REG_WMOVE_INTRQ:
		ret = m_wmove_intrq;
		break;
	case TOPCAT_REG_DISPLAY_PLANE_ENABLE:
		ret = m_display_enable_planes;
		break;
	case TOPCAT_REG_DISPLAY_WRITE_ENABLE_PLANE:
		ret = m_write_enable_plane;
		break;
	case TOPCAT_REG_DISPLAY_READ_ENABLE_PLANE:
		ret = m_read_enable_plane;
		break;
	case TOPCAT_REG_FB_WRITE_ENABLE:
		ret = m_fb_write_enable;
		break;
	case TOPCAT_REG_START_WMOVE:
		ret = 0;
		break;
	case TOPCAT_REG_ENABLE_BLINK_PLANES:
		ret = m_enable_blink_planes;
		break;
	case TOPCAT_REG_ENABLE_ALT_FRAME:
		ret = m_enable_alt_frame;
		break;
	case TOPCAT_REG_CURSOR_CNTL:
		ret = m_cursor_ctrl;
		break;
	case TOPCAT_REG_PIXEL_REPLACE_RULE:
		ret = m_pixel_replacement_rule;
		break;
	case TOPCAT_REG_MOVE_REPLACE_RULE:
		ret = m_move_replacement_rule;
		break;
	case TOPCAT_REG_SOURCE_X_PIXEL:
		ret = m_source_x_pixel;
		break;
	case TOPCAT_REG_SOURCE_Y_PIXEL:
		ret = m_source_y_pixel;
		break;
	case TOPCAT_REG_DST_X_PIXEL:
		ret = m_dst_x_pixel;
		break;
	case TOPCAT_REG_DST_Y_PIXEL:
		ret = m_dst_y_pixel;
		break;
	case TOPCAT_REG_BLOCK_MOVER_PIXEL_WIDTH:
		ret = m_block_mover_pixel_width;
		break;
	case TOPCAT_REG_BLOCK_MOVER_PIXEL_HEIGHT:
		ret = m_block_mover_pixel_height;
		break;
	default:
		logerror("unknown register read %02x\n", offset);
		break;
	}
	return ret;
}
uint32_t dio16_98544_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint32_t pixels;

	for (y = 0; y < 768; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1024/2; x++)
		{
			pixels = m_vram[(y * 512) + x];

			*scanline++ = m_palette[(pixels>>8) & 1];
			*scanline++ = m_palette[(pixels & 1)];
		}
	}

	return 0;
}

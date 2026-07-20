// license:BSD-3-Clause
// copyright-holders:David Haywood
/*****************************************************************************

    GPL16218B (not GPL16228B, even if that would make more sense)
    GPL16238B
    GPL16248VB
    GPL16258VB

	die is marked GCM420 on gormiti (assumed to be GPL16218B)

	generalplus_gpl162xx_lcdtype.cpp appears to also use a GPL162xxB series
	chip, just ignoring the video output, it does however use a custom OTP
	which might mean it's a slight variarion on the above which documentation
	indicates only have the OTP provided by GeneralPlus

	On the surface the B chips may appear to be cost-reduced versions of the
	equivalent A chips (less RAM, smaller internal ROM, no support for MP3 etc.)
	but they change the video in a number of noteworthy ways, including adding
	YUV tilemap/sprite support, extended sprites from main RAM, byteswap unit
	and more.

**********************************************************************/

#include "emu.h"
#include "generalplus_gpl162xx_b_soc.h"

#define LOG_GPL162XX_B       (1U << 1)

#define VERBOSE             (LOG_GPL162XX_B)
#include "logmacro.h"


generalplus_gpl162xx_b_base::generalplus_gpl162xx_b_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal) :
	generalplus_gpl162xx_base_device(mconfig, type, tag, owner, clock, internal)
{
}

generalplus_gpl16218b_device::generalplus_gpl16218b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	generalplus_gpl162xx_b_base(mconfig, GPL16218B, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpl16218b_device::gpl16218b_map), this))
{
}

generalplus_gpl16218b_device::generalplus_gpl16218b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal) :
	generalplus_gpl162xx_b_base(mconfig, type, tag, owner, clock, internal)
{
}


generalplus_gpl16238b_device::generalplus_gpl16238b_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	generalplus_gpl16218b_device(mconfig, GPL16238B, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpl16238b_device::gpl16238b_map), this))
{
}

generalplus_gpl16238b_device::generalplus_gpl16238b_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal) :
	generalplus_gpl16218b_device(mconfig, type, tag, owner, clock, internal)
{
}


generalplus_gpl16248vb_device::generalplus_gpl16248vb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	generalplus_gpl16238b_device(mconfig, GPL16248VB, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpl16248vb_device::gpl16248vb_map), this))
{
}

generalplus_gpl16248vb_device::generalplus_gpl16248vb_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor internal) :
	generalplus_gpl16238b_device(mconfig, type, tag, owner, clock, internal)
{
}

generalplus_gpl16258vb_device::generalplus_gpl16258vb_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	generalplus_gpl16248vb_device(mconfig, GPL16258VB, tag, owner, clock, address_map_constructor(FUNC(generalplus_gpl16258vb_device::gpl16258vb_map), this))
{
}

void generalplus_gpl162xx_b_base::device_reset()
{
	generalplus_gpl162xx_base_device::device_reset();
	m_esp_ctrl = 0x0000;
	m_byteswap = 0x0000;
}

void generalplus_gpl162xx_b_base::device_start()
{
	generalplus_gpl162xx_base_device::device_start();
	save_item(NAME(m_esp_ctrl));
	save_item(NAME(m_byteswap));
}

void generalplus_gpl162xx_b_base::esp_ctrl_w(u16 data)
{
	LOGMASKED(LOG_GPL162XX_B, "%s:generalplus_gpl162xx_b_base::esp_ctrl_w %04x\n", machine().describe_context(), data);
	m_esp_ctrl = data;
}

u16 generalplus_gpl162xx_b_base::esp_ctrl_r()
{
	LOGMASKED(LOG_GPL162XX_B, "%s:generalplus_gpl162xx_b_base::esp_ctrl_r\n", machine().describe_context());
	return m_esp_ctrl;
}

void generalplus_gpl162xx_b_base::gpl162xx_b_extended_sprite_map(address_map &map)
{
	map(0x0070cd, 0x0070cd).rw(FUNC(generalplus_gpl162xx_b_base::esp_ctrl_r), FUNC(generalplus_gpl162xx_b_base::esp_ctrl_w));
}

// Byteswap device (might be the same as GPL951xx)

void generalplus_gpl162xx_b_base::byteswap_w(u16 data)
{
	LOGMASKED(LOG_GPL162XX_B, "%s:generalplus_gpl162xx_b_base::byteswap_w %04x\n", machine().describe_context(), data);
	m_byteswap = data;
}

u16 generalplus_gpl162xx_b_base::byteswap_r()
{
	LOGMASKED(LOG_GPL162XX_B, "%s:generalplus_gpl162xx_b_base::byteswap_r\n", machine().describe_context());
	return ((m_byteswap & 0xff00) >> 8) | ((m_byteswap & 0x00ff) << 8);
}

void generalplus_gpl162xx_b_base::gpl162xx_b_byteswap_map(address_map &map)
{
	map(0x007af0, 0x007af0).rw(FUNC(generalplus_gpl162xx_b_base::byteswap_r), FUNC(generalplus_gpl162xx_b_base::byteswap_w));
	// 7af1 - Nibble Swap
	// 7af2 - TwoBit Swap
	// 7af3 - Reverse Bits
}


void generalplus_gpl16218b_device::gpl16218b_map(address_map &map)
{
	map(0x000000, 0x002fff).ram().share("mainram"); // 12K * 16
	base_internal_map(map);
	//nand_peripheral_map(map); // no NAND support here
	spi_peripheral_map(map); // but does have SPI support
	gpl162xx_b_byteswap_map(map);
	no_internal_rom(map);
	cs_main_view_area(map);
}

void generalplus_gpl16238b_device::gpl16238b_map(address_map &map)
{
	map(0x000000, 0x002fff).ram().share("mainram"); // 12K * 16
	base_internal_map(map);
	nand_peripheral_map(map);
	spi_peripheral_map(map);
	gpl162xx_b_byteswap_map(map);
	internal_rom_4kword(map);
	cs_main_view_area(map);
}

void generalplus_gpl16248vb_device::gpl16248vb_map(address_map &map)
{
	map(0x000000, 0x002fff).ram().share("mainram"); // 12K * 16
	base_internal_map(map);
	nand_peripheral_map(map);
	spi_peripheral_map(map);
	gpl162xx_b_extended_sprite_map(map);
	gpl162xx_b_byteswap_map(map);
	internal_rom_4kword(map);
	cs_main_view_area(map);
}

void generalplus_gpl16258vb_device::gpl16258vb_map(address_map &map)
{
	map(0x000000, 0x002fff).ram().share("mainram"); // 12K * 16
	base_internal_map(map);
	nand_peripheral_map(map);
	spi_peripheral_map(map);
	gpl162xx_b_extended_sprite_map(map);
	gpl162xx_b_byteswap_map(map);
	internal_rom_4kword(map);
	cs_main_view_area(map);
}


void generalplus_gpl16218b_device::device_add_mconfig(machine_config &config)
{
	generalplus_gpl162xx_b_base::device_add_mconfig(config);
	m_spg_video->set_has_gpl162xx_b_features();
	// 16 channel sound (like SPG2xx?
	// supports SPI but not NAND
}

void generalplus_gpl16238b_device::device_add_mconfig(machine_config &config)
{
	generalplus_gpl16218b_device::device_add_mconfig(config);
	// 16 channel sound (like SPG2xx?)
	// adds support for NAND device
}

void generalplus_gpl16248vb_device::device_add_mconfig(machine_config &config)
{
	generalplus_gpl16238b_device::device_add_mconfig(config);
	m_spg_video->set_has_vga_modes();
	m_spg_video->set_has_gpl162xx_b_extended_sprites();
	// 32 channel sound
}

void generalplus_gpl16258vb_device::device_add_mconfig(machine_config &config)
{
	generalplus_gpl16248vb_device::device_add_mconfig(config);
	m_spg_video->set_has_3d_sprite_modes();
	// 32 channel sound
}


ROM_START( gpl1658vb )
	ROM_REGION16_BE( 0x2000, "internal", 0 )
	ROM_LOAD16_WORD_SWAP("gpl16258vb_bootrom.bin", 0x00000, 0x2000, NO_DUMP )
ROM_END

const tiny_rom_entry *generalplus_gpl16258vb_device::device_rom_region() const
{
	return ROM_NAME( gpl1658vb );
}

ROM_START( gpl1648vb )
	ROM_REGION16_BE( 0x2000, "internal", 0 )
	ROM_LOAD16_WORD_SWAP("gpl16248vb_bootrom.bin", 0x00000, 0x2000, NO_DUMP )
ROM_END

const tiny_rom_entry *generalplus_gpl16248vb_device::device_rom_region() const
{
	return ROM_NAME( gpl1648vb );
}

ROM_START( gpl1638b )
	ROM_REGION16_BE( 0x2000, "internal", 0 )
	ROM_LOAD16_WORD_SWAP("gpl16238b_bootrom.bin", 0x00000, 0x2000, NO_DUMP )
ROM_END

const tiny_rom_entry *generalplus_gpl16238b_device::device_rom_region() const
{
	return ROM_NAME( gpl1638b );
}

// it isn't clear if the GPL162318B has a standard internal ROM

DEFINE_DEVICE_TYPE(GPL16218B, generalplus_gpl16218b_device, "gpl16218b", "GeneralPlus GPL16218B / GPAC500B System-on-a-Chip")
DEFINE_DEVICE_TYPE(GPL16238B, generalplus_gpl16238b_device, "gpl16238b", "GeneralPlus GPL16238B System-on-a-Chip")
DEFINE_DEVICE_TYPE(GPL16248VB, generalplus_gpl16248vb_device, "gpl16248vb", "GeneralPlus GPL16248VB System-on-a-Chip")
DEFINE_DEVICE_TYPE(GPL16258VB, generalplus_gpl16258vb_device, "gpl16258vb", "GeneralPlus GPL16258VB System-on-a-Chip") // aka GPAC800B (not used by JAKKS?)

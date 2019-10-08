// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

Seibu Custom "CRT Controller" emulation

written by Angelo Salese

used by several Seibu games:
Raiden later rev (probably the first game to use it)
*Sengoku Mahjong
*Good e Jong
*Tottemo de Jong
*Blood Bros.
*Sky Smasher
*D-Con
*SD Gundam Psycho Salamander no Kyoui
(all games in legionna.cpp)
(all games in raiden2.cpp)
(all games in seibuspi.cpp)

TODO:
- Most registers are still a mystery;
- Get the proper Seibu chip ID number.
  Kold found that a Raiden alt set has irq request pin from a chip named SEI0160, which might be our man.

preliminary memory map:
(screen 0 -> Background)
(screen 1 -> Foreground)
(screen 2 -> Midground)
(screen 3 -> Text layer)
[0x00]: Total number of visible cells?
[0x02]: Single cell H size +1?
[0x1a]: --x- Layer Dynamic Size?
        ---x Flip Screen
[0x1c]: Layer Enable
---x ---- sprite enable
---- x--- tilemap screen 3 enable
---- -x-- tilemap screen 1 enable
---- --x- tilemap screen 2 enable
---- ---x tilemap screen 0 enable
[0x20]: Tilemap Screen 0 scroll X
[0x22]: Tilemap Screen 0 scroll Y
[0x24]: Tilemap Screen 2 scroll X
[0x26]: Tilemap Screen 2 scroll Y
[0x28]: Tilemap Screen 1 scroll X
[0x2a]: Tilemap Screen 1 scroll Y
[0x2c]: Tilemap Screen 0 base scroll X
[0x2e]: Tilemap Screen 0 base scroll Y
[0x30]: Tilemap Screen 2 base scroll X
[0x32]: Tilemap Screen 2 base scroll Y
[0x34]: Tilemap Screen 1 base scroll X
[0x36]: Tilemap Screen 1 base scroll Y
[0x38]: Tilemap Screen 3 base scroll X
[0x3a]: Tilemap Screen 3 base scroll Y
[0x3e]: OBJ Y base
[0x40]: Semaphore for 0x4e register

In later games using the SEI251, SEI252 and RISE sprite chips, registers
0x40 through 0x4e appear to be either nonexistent or overridden. The aberrant
mapping of these registers in some games is not unusual for Seibu customs,
but it should be noted that SD Gundam Psycho Salamander relegates the
initialization of these registers to a separate routine.

===========================================================================================

List of default vregs (title screen):

*Sengoku Mahjong:
8000:  000F 0013 009F 00BF 00FA 000F 00FA 00FF (320 x 240 -> 16 - 256 V res)
8010:  007D 0006 0000 0002 0000 0000 0000 0000
8020:  0000 0000 0004 0000 0000 0000 0040 01FF
8030:  003E 01FF 003F 01FF 0040 0001 0034 0035
8040:  0000 A8A8 0003 1C37 0001 0000 0000 0000
0x1ff - 0x9f = 352
0x1ff - 0xbf = 320
0x1ff - 0xfa = 261
0x1ff - 0xfa = 261
0x1ff - 0xff = 256

*Tottemo E Jong
8000:  000F 000F 009F 00BF 00FA 000F 00FA 00FF (256 x 224 -> 16 - 240 V res)
8010:  0076 0006 0000 0002 0000 0002 0006 0000
8020:  0000 0000 0000 0000 0000 0000 01C0 01FF
8030:  003E 01FF 003F 01FF 00C0 01FF 0034 003F
8040:  0000 A8A8 0003 1830 0001 0000 0000 0000

*Good E Jong
8040:  000F 000F 009F 00BF 00FA 000F 00FA 00FF (256 x 224 -> 16 - 240 v res)
8050:  0076 0006 0000 0002 0000 0002 0006 0000
8060:  0000 00FA 0000 0000 0000 0000 01C0 01FF
8070:  003E 01FF 003F 01FF 00C0 01FF 0034 003F
8000:  0000 A8A8 0003 1830 0001 0000 0000 0000

*Raiden (256 x 224 -> 16 - 240 V res)
0D040:  000F 000F 00B2 00D7 00FB 000F 00FC 00FF
0D050:  0076 0006 0000 0002 0000 0002 0000 0000
0D060:  0000 0000 0000 0000 0000 0000 01D8 0000
0D070:  01DA 0000 003F 01FF 00C0 0000 0034 0025
0D080:  0000 A8A8 0003 1830 0001 0000 0000 0000
0x1ff - 0xb2 = 333
0x1ff - 0xd7 = 296
0x1ff - 0xfb = 260
0x1ff - 0xfc = 259
0x1ff - 0xff = 256

*Blood Bros. (sets 1 and 2, init routine at $620)
0C0000:  000F 000F 00B2 00D7 00F9 000F 00F9 00FF
0C0010:  0076 0006 0001 0002 0000 0000 0004 ****
0C0020:  **** **** **** **** **** **** 01D8 01FF
0C0030:  01DA 01FF **** **** 01D8 01FF 0034 003F
0C0040:  0000 A8A8 0005 1830 0009 **** **** ****

*Blood Bros. (set 3, init routine at $580)
0C0000:  000F 000F 00B7 00BF 00FA 000F 00FA 00FF
0C0010:  0076 0006 0001 0002 0000 0000 0004 ****
0C0020:  **** **** **** **** **** **** 01C0 01FF
0C0030:  01C2 01FF **** **** 00C0 01FF 0034 003F
0C0040:  0000 A8A8 0004 1830 0009 **** **** ****

*Sky Smasher (init routine at $1f5d0)
0C0000:  000F 000F 00B2 00D7 00F9 000F 00F9 00FF
0C0020:  0076 0006 0000 0002 0000 0000 0004 ****
0C0010:  **** **** **** **** **** **** 01D9 01FF
0C0030:  01DB 01FF **** **** 01D8 01FF 0034 003F
0C0040:  0000 A8A8 0005 1830 0009 **** **** ****

*Sky Smasher (unused init routine at $1f4ee)
0C0000:  000F 000F 00AF 00BF 00F8 000F 00F8 00FF
0C0020:  0076 0006 0000 0002 0000 0000 0004 ****
0C0010:  **** **** **** **** **** **** 01C0 01FF
0C0030:  01C2 01FF **** **** 00C0 01FF 0034 003F
0C0040:  0000 A8A8 0006 1830 0009 **** **** ****

*SD Gundam Psycho Salamander no Kyoui (init routines at $4b56 and $13fc; 320 x 224 -> 0 - 224 v res)
0C0000:  000F 0013 009F 00BF 00FA 000F 00FA 00FF
0C0010:  0076 0006 0000 0002 0000 0000 0000 0000
0C0020:  0000 0000 0000 0000 0000 0000 0040 01FF
0C0030:  0040 01FF 0040 01FF 0040 01FF 0034 003F
0C0040:  **** A8A8 0003 1C37 0009 **** **** ****

*D-Con (320 x 224 -> 0 - 224 v res)
0C0000:  000F 0013 009F 00BF 00FA 000F 00FA 00FF
0C0010:  0076 0006 0000 0002 0000 0000 0000 0000
0C0020:  0000 0000 0000 0000 0000 0000 FFC0 FFEF
0C0030:  FFC2 FFEF FFC1 FFEF FFC0 FFEF 0034 003F
0C0040:  0000 A8A8 0013 1C37 0009 0000 0000 0000

*SD Gundam Sangokushi Rainbow Tairiku Senki (320 x 224; Incorrect resolution between service mode and actual in-game value)
(320 x 224 at service mode = 320 x 256 actually)
(320 x 256 at service mode = 320 x 240 actually)
(320 x 240 at service mode = 320 x 224 actually)
(normal)
100600:  000F 0013 00A7 00C7 00FA 000F 00FA 00FF
100610:  0076 0006 0000 0002 0000 0000 0017 0000
100620:  0004 0000 0000 0000 0000 0000 01D8 01FF
100630:  01DA 01FF 01D9 01FF 01D8 01FF 0034 003F
100640:  0000 A8A8 0004 1C37 0008 0000 0000 FFFF
(flipped)
100600:  000F 0013 00A7 00C7 00FA 000F 00FA 00FF
100610:  0076 0006 0000 0002 0000 0001 0017 0000
100620:  0005 0000 0000 0000 0000 0000 0187 0300
100630:  0185 0300 0186 0300 0185 0300 0034 003F
100640:  0000 A8A8 00FB 1C37 0018 0000 013F FFFF

*Seibu Cup Soccer (320 x 240 normal -> 0 - 240 v res, @ service mode; Undefined registers only)
100600:  000F 0013 00A7 00C7 00E0 000F 00E1 00E9
100610:  0076 01FE 0000 0002 0000 0000 **** 0000
100620:  **** **** **** **** **** **** **** ****
100630:  **** **** **** **** **** **** 0034 003F
100640:  0000 A8A8 0016 1C37 0009 0000 0000 FFFF
(320 x 240, flipped)
100600:  000F 0013 00A7 00C7 00E0 000F 00E1 00E9
100610:  0076 01FE 0000 0002 0000 0001 **** 0000
100620:  **** **** **** **** **** **** **** ****
100630:  **** **** **** **** **** **** 0034 003F
100640:  0000 A8A8 00E9 1C37 0019 0000 013F FFFF
(320 x 224, normal)
100600:  000F 0013 00A7 00C7 00FA 000F 00FA 00FF
100610:  0076 0006 0000 0002 0000 0000 **** 0000
100620:  **** **** **** **** **** **** **** ****
100630:  **** **** **** **** **** **** 0034 003F
100640:  0000 A8A8 0004 1C37 0009 0000 0000 FFFF
(320 x 224, flipped)
100600:  000F 0013 00A7 00C7 00FA 000F 00FA 00FF
100610:  0076 0006 0000 0002 0000 0001 **** 0000
100620:  **** **** **** **** **** **** **** ****
100630:  **** **** **** **** **** **** 0034 003F
100640:  0000 A8A8 010B 1C37 0019 0000 013F FFFF
(320 x 256, normal)
100600:  000F 0013 00A7 00C7 00E0 000F 00E7 00F3
100610:  007E 01FE 0000 0002 0000 0000 **** 0000
100620:  **** **** **** **** **** **** **** ****
100630:  **** **** **** **** **** **** 0034 003F
100640:  0000 A8A8 001E 1C37 0009 0000 0000 FFFF
(320 x 256, flipped)
100600:  000F 0013 00A7 00C7 00E0 000F 00E7 00F3
100610:  007E 01FE 0000 0002 0000 0001 **** 0000
100620:  **** **** **** **** **** **** **** ****
100630:  **** **** **** **** **** **** 0034 003F
100640:  0000 A8A8 00E1 1C37 0019 0000 013F FFFF

*Legionnaire (attract mode, that definitely runs with an horizontal res of 256)
100600:  000F 000F 00B0 00D7 00FA 000F 00FA 00FF
100610:  0076 0006 0000 0002 0000 0000 0000 0000
100620:  0000 0000 0000 0000 0000 0000 01D8 01FF
100630:  01DA 01FF 01D9 01FF 01D8 01FF 0034 003F
100640:  0000 A8A8 0004 1830 0009 0000 0000 FFFF
*Godzilla, 320 x 224 (455 x 258 hvtotal)
100600:  000f 0013 00a0 00d7 00fa 000f 00fa 00ff

*Seibu SPI is a newer CRTC?
00000400: 013F007F - hblank start(-1), hblank length(-1)
00000404: 03DA03B5 - related to above, not sure how
00000408: 00FC002A - vblank start(-1), vblank length(-1)
0000040C: 03F303EB - related to above, not sure how
00000410: 01FE0076
00000414: related to decryption
00000418: fg layer bank, rowscroll enable, ...
0000041C-0000043F: same as other chips (layer enable, scrollregs, base)

***************************************************************************/

#include "emu.h"
#include "video/seibu_crtc.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(SEIBU_CRTC, seibu_crtc_device, "seibu_crtc", "Seibu CRT Controller")

void seibu_crtc_device::seibu_crtc_vregs(address_map &map)
{
	map(0x0000, 0x004f).ram(); // debug
	map(0x0014, 0x0015).w(FUNC(seibu_crtc_device::decrypt_key_w));
	map(0x001a, 0x001b).rw(FUNC(seibu_crtc_device::reg_1a_r), FUNC(seibu_crtc_device::reg_1a_w));
	map(0x001c, 0x001d).w(FUNC(seibu_crtc_device::layer_en_w));
	map(0x0020, 0x002b).w(FUNC(seibu_crtc_device::layer_scroll_w));
	map(0x002c, 0x003b).w(FUNC(seibu_crtc_device::layer_scroll_base_w));
}

WRITE16_MEMBER(seibu_crtc_device::decrypt_key_w)
{
	if (!m_decrypt_key_cb.isnull())
		m_decrypt_key_cb(0, data, mem_mask);
}

WRITE16_MEMBER( seibu_crtc_device::layer_en_w)
{
	if (!m_layer_en_cb.isnull())
		m_layer_en_cb(0,data,mem_mask);
}

WRITE16_MEMBER( seibu_crtc_device::layer_scroll_w)
{
	if (!m_layer_scroll_cb.isnull())
		m_layer_scroll_cb(offset,data,mem_mask);
}

READ16_MEMBER(seibu_crtc_device::reg_1a_r)
{
	// SPI needs read/write access to this
	return m_reg_1a;
}

WRITE16_MEMBER( seibu_crtc_device::reg_1a_w)
{
	COMBINE_DATA(&m_reg_1a);
	if (!m_reg_1a_cb.isnull())
		m_reg_1a_cb(offset,data,mem_mask);
}

WRITE16_MEMBER( seibu_crtc_device::layer_scroll_base_w)
{
	if (!m_layer_scroll_base_cb.isnull())
		m_layer_scroll_base_cb(offset,data,mem_mask);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  seibu_crtc_device - constructor
//-------------------------------------------------

seibu_crtc_device::seibu_crtc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SEIBU_CRTC, tag, owner, clock),
		device_memory_interface(mconfig, *this),
		device_video_interface(mconfig, *this),
		m_decrypt_key_cb(*this),
		m_layer_en_cb(*this),
		m_layer_scroll_cb(*this),
		m_reg_1a_cb(*this),
		m_layer_scroll_base_cb(*this),
		m_space_config("vregs", ENDIANNESS_LITTLE, 16, 7, 0, address_map_constructor(FUNC(seibu_crtc_device::seibu_crtc_vregs), this))
{
}


//-------------------------------------------------
//  device_validity_check - perform validity checks
//  on this device
//-------------------------------------------------

void seibu_crtc_device::device_validity_check(validity_checker &valid) const
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void seibu_crtc_device::device_start()
{
	m_decrypt_key_cb.resolve();
	m_layer_en_cb.resolve();
	m_layer_scroll_cb.resolve();
	m_reg_1a_cb.resolve();
	m_layer_scroll_base_cb.resolve();

	save_item(NAME(m_reg_1a));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void seibu_crtc_device::device_reset()
{
	m_reg_1a = 0;
}

//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector seibu_crtc_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}



//**************************************************************************
//  INLINE HELPERS
//**************************************************************************

//-------------------------------------------------
//  read_word - read a word at the given address
//-------------------------------------------------

inline uint16_t seibu_crtc_device::read_word(offs_t address)
{
	return space().read_word(address << 1);
}

//-------------------------------------------------
//  write_word - write a word at the given address
//-------------------------------------------------

inline void seibu_crtc_device::write_word(offs_t address, uint16_t data)
{
	space().write_word(address << 1, data);
}

//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

READ16_MEMBER( seibu_crtc_device::read )
{
	return read_word(offset);
}

WRITE16_MEMBER( seibu_crtc_device::write )
{
	write_word(offset,data);
}

/* Sky Smasher / Raiden DX swaps registers [0x10] with [0x20] */
READ16_MEMBER( seibu_crtc_device::read_alt )
{
	return read_word(bitswap<16>(offset,15,14,13,12,11,10,9,8,7,6,5,3,4,2,1,0));
}

WRITE16_MEMBER( seibu_crtc_device::write_alt )
{
	write_word(bitswap<16>(offset,15,14,13,12,11,10,9,8,7,6,5,3,4,2,1,0),data);
}

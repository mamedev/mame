/*
    SMS code from HazeMD
     - used by Syetem E
     - Megatech / Megaplay

    this contains common code to support those systems

    (not currently very good / accurate, should be rewritten)

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/sn76496.h"
#include "machine/mc8123.h"
#include "machine/segacrpt.h"
#include "includes/segamsys.h"


//static UINT8* sms_rom;
UINT8* sms_mainram;
UINT8* smsgg_backupram = 0;
static TIMER_CALLBACK( sms_scanline_timer_callback );
static struct sms_vdp *vdp2;
static struct sms_vdp *vdp1;

static struct sms_vdp *md_sms_vdp;

/* All Accesses to VRAM go through here for safety */
#define SMS_VDP_VRAM(address) chip->vram[(address)&0x3fff]

#ifdef UNUSED_FUNCTION
static ADDRESS_MAP_START( sms_map, AS_PROGRAM, 8 )
//  AM_RANGE(0x0000 , 0xbfff) AM_ROM
//  AM_RANGE(0xc000 , 0xdfff) AM_RAM AM_MIRROR(0x2000)
ADDRESS_MAP_END
#endif


ADDRESS_MAP_START( sms_io_map, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( sms_common )
	PORT_START("PAD1")		/* Joypad 1 (2 button) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 B") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 C") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PAD2")		/* Joypad 2 (2 button) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 B") // a
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 C") // b
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

INPUT_PORTS_START( sms )
	PORT_INCLUDE( sms_common )

	PORT_START("PAUSE")		/* Buttons on SMS Console */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Pause Button") PORT_IMPULSE(1)
INPUT_PORTS_END

INPUT_PORTS_START( gamegear )
	PORT_INCLUDE( sms_common )

	PORT_START("GGSTART")		/* Extra GameGear button */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Start Button")
INPUT_PORTS_END

/* Precalculated tables for H/V counters.  Note the position the counter 'jumps' is marked with with
   an empty comment */
static const UINT8 hc_256[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,    0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,    0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,    0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,    0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,    0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,    0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,    0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,    0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,    0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,    0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,    0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,/**/0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
    0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3,    0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
    0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,    0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
    0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3,    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3,    0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3,    0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3,    0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};


static const UINT8 vc_ntsc_192[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,/**/0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4,    0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,    0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_ntsc_224[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,/**/0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4,    0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_ntsc_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05
};



static const UINT8 vc_pal_192[] =
{
    0x00, 0x01, 0x02,    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12,    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22,    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32,    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42,    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52,    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62,    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72,    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82,    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92,    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2,    0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2,    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2,    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2,    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2,    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2,/**/0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6,
    0xc7, 0xc8, 0xc9,    0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9,    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9,    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};


static const UINT8 vc_pal_224[] =
{
    0x00, 0x01, 0x02,    0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12,    0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22,    0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32,    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42,    0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52,    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62,    0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72,    0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82,    0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92,    0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2,    0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2,    0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2,    0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2,    0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2,    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2,    0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02,/**/0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9,    0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9,    0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9,    0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const UINT8 vc_pal_240[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,    0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,    0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a,    0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a,    0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a,    0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a,    0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,    0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a,    0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a,    0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a,    0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,    0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba,    0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca,    0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,    0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,    0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa,    0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,/**/0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
    0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1,    0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
    0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1,    0xf2, 0xf3, 0xf4, 0xf5, 0xf6,
    0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

static const struct
{
	UINT8 sms2_name[40];
	int sms2_valid;
	int sms2_height;
	int sms2_tilemap_height;
	const UINT8* sms_vcounter_table;
	const UINT8* sms_hcounter_table;

} sms_mode_table[] =
{
	/* NTSC Modes */
	{ "Graphic 1 (NTSC)",         0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Text (NTSC)",              0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Graphic 2 (NTSC)",         0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 1+2 (NTSC)" ,         0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Multicolor (NTSC)",        0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 1+3 (NTSC)",          0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 2+3 (NTSC)",          0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 1+2+3 (NTSC)",        0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 224, vc_ntsc_192, hc_256 },
	{ "Invalid Text (NTSC)",      0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (224-line) (NTSC)", 1, 224, 256, vc_ntsc_224, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 224, vc_ntsc_192, hc_256 },
	{ "Invalid Text (NTSC)",      0, 192, 224, vc_ntsc_192, hc_256 },
	{ "Mode 4 (240-line) (NTSC)", 1, 240, 256, vc_ntsc_240, hc_256 },
	{ "Mode 4 (NTSC)",            1, 192, 244, vc_ntsc_192, hc_256 },

	/* Pal Modes (different Vcounter) */
	{ "Graphic 1 (PAL)",         0, 192, 224, vc_pal_192, hc_256 },
	{ "Text (PAL)",              0, 192, 224, vc_pal_192, hc_256 },
	{ "Graphic 2 (PAL)",         0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 1+2 (PAL)" ,         0, 192, 224, vc_pal_192, hc_256 },
	{ "Multicolor (PAL)",        0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 1+3 (PAL)",          0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 2+3 (PAL)",          0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 1+2+3 (PAL)",        0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 224, vc_pal_192, hc_256 },
	{ "Invalid Text (PAL)",      0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (224-line) (PAL)", 1, 224, 256, vc_pal_224, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 224, vc_pal_192, hc_256 },
	{ "Invalid Text (PAL)",      0, 192, 224, vc_pal_192, hc_256 },
	{ "Mode 4 (240-line) (PAL)", 1, 240, 256, vc_pal_240, hc_256 },
	{ "Mode 4 (PAL)",            1, 192, 244, vc_pal_192, hc_256 }
};

enum
{
	SMS_VDP = 0,  // SMS1 VDP
	SMS2_VDP = 1, // SMS2 VDP, or Game Gear VDP running in SMS2 Mode
	GG_VDP = 2,   // Game Gear VDP running in Game Gear Mode
	GEN_VDP = 3   // Genesis VDP running in SMS2 Mode
};

static int sms_vdp_null_irq_callback(running_machine &machine, int status)
{
	return -1;
}

static int sms_vdp_cpu0_irq_callback(running_machine &machine, int status)
{
	if (status == 1)
		cputag_set_input_line(machine, "maincpu", 0, HOLD_LINE);
	else
		cputag_set_input_line(machine, "maincpu", 0, CLEAR_LINE);

	return 0;
}

static int sms_vdp_cpu1_irq_callback(running_machine &machine, int status)
{
	if (status == 1)
		cputag_set_input_line(machine, "genesis_snd_z80", 0, HOLD_LINE);
	else
		cputag_set_input_line(machine, "genesis_snd_z80", 0, CLEAR_LINE);

	return 0;
}


static int sms_vdp_cpu2_irq_callback(running_machine &machine, int status)
{
	if (status == 1)
		cputag_set_input_line(machine, "mtbios", 0, HOLD_LINE);
	else
		cputag_set_input_line(machine, "mtbios", 0, CLEAR_LINE);

	return 0;
}




struct sms_vdp
{
	UINT8 chip_id;

	UINT8  cmd_pend;
	UINT8  cmd_part1;
	UINT8  cmd_part2;
	UINT16 addr_reg;
	UINT8  cmd_reg;
	UINT8  regs[0x10];
	UINT8  readbuf;
	UINT8* vram;
	UINT8* cram;
	UINT8  writemode;
	bitmap_t* r_bitmap;
	UINT8* tile_renderline;
	UINT8* sprite_renderline;

	UINT8 sprite_collision;
	UINT8 sprite_overflow;

	UINT8  yscroll;
	UINT8  hint_counter;

	UINT8 frame_irq_pending;
	UINT8 line_irq_pending;

	UINT8 vdp_type;

	UINT8 gg_cram_latch; // gamegear specific.

	/* below are MAME specific, to make things easier */
	UINT8 screen_mode;
	UINT8 is_pal;
	int sms_scanline_counter;
	int sms_total_scanlines;
	int sms_framerate;
	emu_timer* sms_scanline_timer;
	UINT16* cram_mamecolours; // for use on RGB_DIRECT screen
	int	 (*set_irq)(running_machine &machine, int state);

};



static void *start_vdp(running_machine &machine, int type)
{
	struct sms_vdp *chip;

	chip = auto_alloc_clear(machine, struct sms_vdp);

	chip->vdp_type = type;

	chip->set_irq = sms_vdp_null_irq_callback;

	chip->cmd_pend = 0;
	chip->cmd_part1 = 0;
	chip->cmd_part2 = 0;
	chip->addr_reg = 0;
	chip->cmd_reg = 0;

	chip->regs[0x0] = 0x06; // mode 4
	chip->regs[0x1] = 0x18; // mode 4
	chip->regs[0x2] = 0;
	chip->regs[0x3] = 0;
	chip->regs[0x4] = 0;
	chip->regs[0x5] = 0;
	chip->regs[0x6] = 0;
	chip->regs[0x7] = 0;
	chip->regs[0x8] = 0;
	chip->regs[0x9] = 0;
	chip->regs[0xa] = 0;
	/* b-f don't matter */
	chip->readbuf = 0;
	chip->vram = auto_alloc_array_clear(machine, UINT8, 0x4000);

	//printf("%d\n", (*chip->set_irq)(machine, 200));

	if (chip->vdp_type==GG_VDP)
	{
		chip->cram = auto_alloc_array_clear(machine, UINT8, 0x0040);
		chip->cram_mamecolours = auto_alloc_array_clear(machine, UINT16, 0x0080/2);
		chip->gg_cram_latch = 0;
	}
	else
	{
		chip->cram = auto_alloc_array_clear(machine, UINT8, 0x0020);
		chip->cram_mamecolours = auto_alloc_array(machine, UINT16, 0x0040/2);
	}

	chip->tile_renderline = auto_alloc_array(machine, UINT8, 256+8);
	memset(chip->tile_renderline,0x00,256+8);

	chip->sprite_renderline = auto_alloc_array(machine, UINT8, 256+32);
	memset(chip->sprite_renderline,0x00,256+32);

	chip->writemode = 0;
	chip->r_bitmap = auto_bitmap_alloc(machine, 256, 256, BITMAP_FORMAT_RGB15);

	chip->sms_scanline_timer = machine.scheduler().timer_alloc(FUNC(sms_scanline_timer_callback), chip);

	return chip;
}

/* stop timer and clear ram.. used on megatech when we switch between genesis and sms mode */
void segae_md_sms_stop_scanline_timer(void)
{
	md_sms_vdp->sms_scanline_timer->adjust(attotime::never);
	memset(md_sms_vdp->vram,0x00,0x4000);
}


#ifdef UNUSED_FUNCTION
static READ8_HANDLER( z80_unmapped_r )
{
	printf("unmapped z80 read %04x\n",offset);
	return 0;
}

static WRITE8_HANDLER( z80_unmapped_w )
{
	printf("unmapped z80 write %04x\n",offset);
}
#endif

static UINT8 vcounter_r(struct sms_vdp *chip)
{
//  return vc_pal_224[sms_scanline_counter%(sizeof vc_pal_224)];
	UINT8 retvalue;
	int scanline = chip->sms_scanline_counter%chip->sms_total_scanlines;

	retvalue = sms_mode_table[chip->screen_mode].sms_vcounter_table[scanline];

	return retvalue;
	//printf("retvalue %d\n";
}


static UINT8 vdp_data_r(struct sms_vdp *chip)
{
	UINT8 retdata = chip->readbuf;
	chip->readbuf = SMS_VDP_VRAM(chip->addr_reg);
	chip->addr_reg++; chip->addr_reg&=0x3fff;
	return retdata;
}

static void vdp_data_w(address_space *space, UINT8 data, struct sms_vdp* chip)
{
	/* data writes clear the pending flag */
	chip->cmd_pend = 0;

	if (chip->writemode==0)
	{ /* Write to VRAM */
		SMS_VDP_VRAM(chip->addr_reg)=data;
		chip->addr_reg++; chip->addr_reg&=0x3fff;
		chip->readbuf = data; // quirk of the VDP
	}
	else if (chip->writemode==1)
	{
		if (chip->vdp_type==GG_VDP)
		{
			if (!(chip->addr_reg&1))
			{ /* Even address, value latched */
				chip->gg_cram_latch = data;
			}
			else
			{
				chip->cram[(chip->addr_reg&0x3e)+1]=data;
				chip->cram[(chip->addr_reg&0x3e)+0]=chip->gg_cram_latch;

				/* Set Colour */
				{
					UINT16 palword;
					UINT8 r,g,b;

					palword = ((chip->cram[(chip->addr_reg&0x3e)+1])<<8)|(chip->cram[(chip->addr_reg&0x3e)+0]);

					//printf("addr %04x palword %04x\n", chip->addr_reg&0x3f, palword);

					r = (palword & 0x000f)>>0;
					g = (palword & 0x00f0)>>4;
					b = (palword & 0x0f00)>>8;
					palette_set_color_rgb(space->machine(),(chip->addr_reg&0x3e)/2, pal4bit(r), pal4bit(g), pal4bit(b));
					chip->cram_mamecolours[(chip->addr_reg&0x3e)/2]=(b<<1)|(g<<6)|(r<<11);
				}
			}
		}
		else
		{
			chip->cram[chip->addr_reg&0x1f]=data;

			/* Set Colour */
			{
				UINT8 r,g,b;
				r = (data & 0x03)>>0;
				g = (data & 0x0c)>>2;
				b = (data & 0x30)>>4;
				palette_set_color_rgb(space->machine(),chip->addr_reg&0x1f, pal2bit(r), pal2bit(g), pal2bit(b));
				chip->cram_mamecolours[chip->addr_reg&0x1f]=(b<<3)|(g<<8)|(r<<13);
			}

		}

		chip->addr_reg++; chip->addr_reg&=0x3fff;
		chip->readbuf = data; // quirk of the VDP

	}

}

static UINT8 vdp_ctrl_r(address_space *space, struct sms_vdp *chip)
{
	UINT8 retvalue;

	retvalue = (chip->frame_irq_pending<<7) |
		       (chip->sprite_overflow<<6) |
	           (chip->sprite_collision<<5);

	chip->cmd_pend = 0;
	chip->frame_irq_pending = 0;
	chip->line_irq_pending = 0;
	chip->sprite_collision = 0;
	chip->sprite_overflow = 0;

	(chip->set_irq)(space->machine(), 0); // clear IRQ;


	return retvalue;
}

/* check me */
static void vdp_update_code_addr_regs(struct sms_vdp *chip)
{
	chip->addr_reg = ((chip->cmd_part2&0x3f)<<8) | chip->cmd_part1;
	chip->cmd_reg = (chip->cmd_part2&0xc0)>>6;
}

static void vdp_set_register(running_machine &machine, struct sms_vdp *chip)
{
	UINT8 reg = chip->cmd_part2&0x0f;
	chip->regs[reg] = chip->cmd_part1;

	//if(reg==0) printf("setting reg 0 to %02x\n",chip->cmd_part1);

	//if (reg>0xa) printf("Invalid register write to register %01x\n",reg);

	if(reg==1)
	{
		if ((chip->regs[0x1]&0x20) && chip->frame_irq_pending)
		{
			(chip->set_irq)(machine, 1); // set IRQ;
		}
		else
		{
			(chip->set_irq)(machine, 0); // clear IRQ;
		}
	}

	if(reg==0)
	{
		if ((chip->regs[0x0]&0x10) && chip->line_irq_pending)
		{
			(chip->set_irq)(machine, 1); // set IRQ;
		}
		else
		{
			(chip->set_irq)(machine, 0); // clear IRQ;
		}
	}


//  printf("VDP: setting register %01x to %02x\n",reg, chip->cmd_part1);
}

static void vdp_ctrl_w(address_space *space, UINT8 data, struct sms_vdp *chip)
{
	if (chip->cmd_pend)
	{ /* Part 2 of a command word write */
		chip->cmd_pend = 0;
		chip->cmd_part2 = data;
		vdp_update_code_addr_regs(chip);

		switch (chip->cmd_reg)
		{
			case 0x0: /* VRAM read mode */
				chip->readbuf = SMS_VDP_VRAM(chip->addr_reg);
				chip->addr_reg++; chip->addr_reg&=0x3fff;
				chip->writemode = 0;
				break;

			case 0x1: /* VRAM write mode */
				chip->writemode = 0;
				break;

			case 0x2: /* REG setting */
				vdp_set_register(space->machine(), chip);
				chip->writemode = 0;
				break;

			case 0x3: /* CRAM write mode */
				chip->writemode = 1;
				break;
		}
	}
	else
	{ /* Part 1 of a command word write */
		chip->cmd_pend = 1;
		chip->cmd_part1 = data;
		vdp_update_code_addr_regs(chip);
	}
}

/* for the Genesis */

READ8_HANDLER( md_sms_vdp_vcounter_r )
{
	return vcounter_r(md_sms_vdp);
}

READ8_HANDLER( md_sms_vdp_data_r )
{
	return vdp_data_r(md_sms_vdp);
}

WRITE8_HANDLER( md_sms_vdp_data_w )
{
	vdp_data_w(space, data, md_sms_vdp);
}

READ8_HANDLER( md_sms_vdp_ctrl_r )
{
	return vdp_ctrl_r(space, md_sms_vdp);
}

WRITE8_HANDLER( md_sms_vdp_ctrl_w )
{
	vdp_ctrl_w(space, data, md_sms_vdp);
}


/* Read / Write Handlers - call other functions */

READ8_HANDLER( sms_vcounter_r )
{
	return vcounter_r(vdp1);
}

READ8_HANDLER( sms_vdp_data_r )
{
	return vdp_data_r(vdp1);
}

WRITE8_HANDLER( sms_vdp_data_w )
{
	vdp_data_w(space, data, vdp1);
}

READ8_HANDLER( sms_vdp_ctrl_r )
{
	return vdp_ctrl_r(space, vdp1);
}

WRITE8_HANDLER( sms_vdp_ctrl_w )
{
	vdp_ctrl_w(space, data, vdp1);
}

static void draw_tile_line(int drawxpos, int tileline, UINT16 tiledata, UINT8* linebuf, struct sms_vdp* chip)
{
	int xx;
	UINT32 gfxdata;
	UINT16 gfx_base = (tiledata & 0x01ff)<<5;
	UINT8  flipx = (tiledata & 0x0200)>>9;
	UINT8  flipy = (tiledata & 0x0400)>>10;
	UINT8  pal   = (tiledata & 0x0800)>>11;
	UINT8  pri   = (tiledata & 0x1000)>>12;

	if (flipy)
	{
		gfx_base+=(7-tileline)*4;
	}
	else
	{
		gfx_base+=tileline*4;
	}

	gfxdata = (SMS_VDP_VRAM(gfx_base)<<24)|(SMS_VDP_VRAM(gfx_base+1)<<16)|(SMS_VDP_VRAM(gfx_base+2)<<8)|(SMS_VDP_VRAM(gfx_base+3)<<0);

	for (xx=0;xx<8;xx++)
	{
		UINT8 pixel;

		if (flipx)
		{
			pixel = (( (gfxdata>>(0+xx)  ) &0x00000001)<<3)|
		            (( (gfxdata>>(8+xx)  ) &0x00000001)<<2)|
		            (( (gfxdata>>(16+xx) ) &0x00000001)<<1)|
		            (( (gfxdata>>(24+xx) ) &0x00000001)<<0);
		}
		else
		{
			pixel = (( (gfxdata>>(7-xx)  ) &0x00000001)<<3)|
			        (( (gfxdata>>(15-xx) ) &0x00000001)<<2)|
			        (( (gfxdata>>(23-xx) ) &0x00000001)<<1)|
			        (( (gfxdata>>(31-xx) ) &0x00000001)<<0);
		}

		pixel += pal*0x10;

		if (!pri) linebuf[drawxpos+xx] = pixel;
		else
		{
			if (pixel&0xf)
				linebuf[drawxpos+xx] = pixel|0x80;
			else
				linebuf[drawxpos+xx] = pixel;

		}
	}
}

static void sms_render_spriteline(int scanline, struct sms_vdp* chip)
{
	int spritenum;
	int height = 8;
	int width = 8;
	int max_sprites = 8;
	int visible_line = 0;

	UINT16 table_base = (chip->regs[0x5]&0x7e) << 7;
	UINT8 pattern_bit = (chip->regs[0x6]&0x04) >> 2; // high bit of the tile # (because spriteram can only contain an 8-bit tile #)


	memset(chip->sprite_renderline, 0, 256+32);

	for (spritenum = 0;spritenum<64;spritenum++)
	{
		int xpos,ypos,num;
		/*
        00: yyyyyyyyyyyyyyyy
        10: yyyyyyyyyyyyyyyy
        20: yyyyyyyyyyyyyyyy
        30: yyyyyyyyyyyyyyyy
        40: ????????????????
        50: ????????????????
        60: ????????????????
        70: ????????????????
        80: xnxnxnxnxnxnxnxn
        90: xnxnxnxnxnxnxnxn
        A0: xnxnxnxnxnxnxnxn
        B0: xnxnxnxnxnxnxnxn
        C0: xnxnxnxnxnxnxnxn
        D0: xnxnxnxnxnxnxnxn
        E0: xnxnxnxnxnxnxnxn
        F0: xnxnxnxnxnxnxnxn
        */

		ypos = SMS_VDP_VRAM(table_base+spritenum);
		xpos = SMS_VDP_VRAM(table_base+0x80+spritenum*2+0);
		num  = SMS_VDP_VRAM(table_base+0x80+spritenum*2+1)|(pattern_bit<<8);

		if (chip->regs[0x1]&0x2)
		{
			num &=0x1fe;
			height=16;
		}
		else height = 8;


		xpos+=16; // allow room either side for clipping (avoids xdrawpos of -8 if bit below is set)

		if (chip->regs[0x0]&0x08) xpos-=8;

		if ((sms_mode_table[chip->screen_mode].sms2_height)==192)
		{
			if (ypos == 0xd0)
				return;
		}

		ypos++;

		num <<= 5;
		//num+=((scanline-ypos)&0x7)*4;

		visible_line = 0;

		if (ypos<=scanline && ypos+height>scanline)
		{
			visible_line = 1;
			num+=((scanline-ypos)&(height-1))*4;

		}
		else if (ypos+height>0x100)
		{
			if (scanline< ypos+height-0x100)
			{
				visible_line = 1;
				num+=((scanline-ypos)&(height-1))*4;

			}
		}

		if (visible_line)
		{
			int xx;
			UINT32 gfxdata;

			gfxdata = (SMS_VDP_VRAM(num&0x3fff)<<24)|(SMS_VDP_VRAM((num+1)&0x3fff)<<16)|(SMS_VDP_VRAM((num+2)&0x3fff)<<8)|(SMS_VDP_VRAM((num+3)&0x3fff)<<0);


			for (xx=0;xx<8;xx++)
			{
				UINT8 pixel = (( (gfxdata>>(0+xx)  ) &0x00000001)<<3)|
				              (( (gfxdata>>(8+xx)  ) &0x00000001)<<2)|
				              (( (gfxdata>>(16+xx) ) &0x00000001)<<1)|
				              (( (gfxdata>>(24+xx) ) &0x00000001)<<0);

				if (pixel)
				{
					if (!chip->sprite_renderline[xpos+((width-1)-xx)])
					{
						chip->sprite_renderline[xpos+((width-1)-xx)] = pixel;
					}
					else
					{
						chip->sprite_collision = 1;
					}
				}
			}

			max_sprites--;

			if (max_sprites==0)
			{
				chip->sprite_overflow = 1;
				return;
			}

		}
	}
}

static void sms_render_tileline(int scanline, struct sms_vdp* chip)
{
	int column = 0;
	int count = 32;
	int drawxpos;

	UINT8 xscroll = chip->regs[0x8];
	UINT8 yscroll = chip->yscroll;
	UINT16 table_base = (chip->regs[0x2]&0x0e)<<10;
	UINT16 our_base;
	UINT8  our_line = (scanline+yscroll) & 0x7;

	/* In 224 and 240 line modes the table base is different */
	if ((sms_mode_table[chip->screen_mode].sms2_height)!=192)
	{
		table_base &=0x3700; table_base|=0x700;
	}

	if ((chip->regs[0x0]&0x40) && scanline < 16)
	{
		xscroll = 0;
	}

//  xscroll = 0;

//  table_base -= 0x0100;

	our_base = table_base+(((scanline+yscroll)%sms_mode_table[chip->screen_mode].sms2_tilemap_height)>>3)*64;// + (yscroll>>3)*32;

	our_base &=0x3fff;

	memset(chip->tile_renderline, (chip->regs[0x7]&0x0f)+0x10, 256+8);

	drawxpos = xscroll&0x7;
	column = 32-(xscroll>>3);

	do
	{
		UINT16 tiledata = (SMS_VDP_VRAM((our_base+(column&0x1f)*2+1)&0x3fff) << 8) |
		                  (SMS_VDP_VRAM((our_base+(column&0x1f)*2+0)&0x3fff) << 0);

//      UINT16 pattern = ((column+((scanline>>3)*32)) & 0x01ff)<<5;

		draw_tile_line(drawxpos, our_line, tiledata, chip->tile_renderline, chip);

		drawxpos+=8;
		column++;
		column&=0x1f;
		count--;
	} while (count);

}

static void sms_copy_to_renderbuffer(int scanline, struct sms_vdp* chip)
{
	int x;
	UINT16* lineptr = BITMAP_ADDR16(chip->r_bitmap, scanline, 0);

	for (x=0;x<256;x++)
	{
		UINT8 dat = chip->tile_renderline[x];
		UINT8 col;


		col = (chip->regs[0x7]&0x0f)+0x10;
		lineptr[x] = chip->cram_mamecolours[col];


		if ((x<8 && (chip->regs[0x0]&0x20)) || !(chip->regs[0x1]&0x40))
			continue;


		if (!(dat & 0x80))
		{
			lineptr[x] = chip->cram_mamecolours[dat&0x1f];
			if ((dat&0xf)==0x0) lineptr[x]|=0x8000;

		}

		if (chip->sprite_renderline[x+16])
		{
			lineptr[x] =  chip->cram_mamecolours[chip->sprite_renderline[x+16]+0x10];
		}

		if (dat & 0x80)
		{
			lineptr[x] = chip->cram_mamecolours[dat&0x1f];
			if ((dat&0xf)==0x0) lineptr[x]|=0x8000;
		}

	}

}

static void sms_draw_scanline(int scanline, struct sms_vdp* chip)
{

	if (scanline>=0 && scanline<sms_mode_table[chip->screen_mode].sms2_height)
	{
		sms_render_spriteline(scanline, chip);
		sms_render_tileline(scanline, chip);
		sms_copy_to_renderbuffer(scanline, chip);

	}
}


static TIMER_CALLBACK( sms_scanline_timer_callback )
{
	/* This function is called at the very start of every scanline starting at the very
       top-left of the screen.  The first scanline is scanline 0 (we set scanline to -1 in
       VIDEO_EOF) */

	/* Compensate for some rounding errors

       When the counter reaches 314 (or whatever the max lines is) we should have reached the
       end of the frame, however due to rounding errors in the timer calculation we're not quite
       there.  Let's assume we are still in the previous scanline for now.

       The position to get the H position also has to compensate for a few errors
    */
//  printf("num %d\n",num );
	struct sms_vdp *chip = (struct sms_vdp *)ptr;

	if (chip->sms_scanline_counter<(chip->sms_total_scanlines-1))
	{
		chip->sms_scanline_counter++;
		chip->sms_scanline_timer->adjust(attotime::from_hz(chip->sms_framerate * chip->sms_total_scanlines));

		if (chip->sms_scanline_counter>sms_mode_table[chip->screen_mode].sms2_height)
		{
			chip->hint_counter=chip->regs[0xa];
		}

		if (chip->sms_scanline_counter==0)
		{
			chip->hint_counter=chip->regs[0xa];
		}

		if (chip->sms_scanline_counter<=192)
		{
			chip->hint_counter--;

			if (chip->hint_counter==0xff)
			{
				//if (chip->chip_id==2) printf("irq triggerd on scanline %d %d\n", vdp1->sms_scanline_counter, vdp2->sms_scanline_counter);

				chip->line_irq_pending = 1;
				chip->hint_counter=chip->regs[0xa];
				if (chip->regs[0x0]&0x10)
				{
					(chip->set_irq)(machine, 1); // set IRQ;
				}
				else
				{
					(chip->set_irq)(machine, 0); // clear IRQ;
				}
			}

		}


		sms_draw_scanline(chip->sms_scanline_counter, chip);

		//if(sms_scanline_counter==0) chip->sprite_collision = 0;

		if (chip->sms_scanline_counter==sms_mode_table[chip->screen_mode].sms2_height+1 )
		{
			chip->frame_irq_pending = 1;
			if (chip->regs[0x1]&0x20)
			{
				(chip->set_irq)(machine, 1); // set IRQ;
			}
			else
			{
				(chip->set_irq)(machine, 0); // clear IRQ;
			}
		}
	}
	else
	{	/* if we're called passed the total number of scanlines then assume we're still on the last one.
           this can happen due to rounding errors */
		chip->sms_scanline_counter = chip->sms_total_scanlines-1;
	}
}

#ifdef UNUSED_FUNCTION
static void show_tiles(struct sms_vdp* chip)
{
	int x,y,xx,yy;
	UINT16 count = 0;

	for (y=0;y<16;y++)
	{
		for (x=0;x<32;x++)
		{
			for (yy=0;yy<8;yy++)
			{
				int drawypos = y*8+yy;
				UINT16* lineptr = BITMAP_ADDR16(chip->r_bitmap, drawypos, 0);

				UINT32 gfxdata = (SMS_VDP_VRAM(count)<<24)|(SMS_VDP_VRAM(count+1)<<16)|(SMS_VDP_VRAM(count+2)<<8)|(SMS_VDP_VRAM(count+3)<<0);

				for (xx=0;xx<8;xx++)
				{
					int drawxpos = x*8+xx;

					UINT8 pixel = (( (gfxdata>>(0+xx)  ) &0x00000001)<<3)|
					              (( (gfxdata>>(8+xx)  ) &0x00000001)<<2)|
					              (( (gfxdata>>(16+xx) ) &0x00000001)<<1)|
					              (( (gfxdata>>(24+xx) ) &0x00000001)<<0);

					lineptr[drawxpos] = chip->cram_mamecolours[pixel+16];

				}


				count+=4;count&=0x3fff;
			}
		}
	}
}
#endif

/*
 Register $00 - Mode Control No. 1

 D7 - 1= Disable vertical scrolling for columns 24-31
 D6 - 1= Disable horizontal scrolling for rows 0-1
 D5 - 1= Mask column 0 with overscan color from register #7
 D4 - (IE1) 1= Line interrupt enable
 D3 - (EC) 1= Shift sprites left by 8 pixels
 D2 - (M4) 1= Use Mode 4, 0= Use TMS9918 modes (selected with M1, M2, M3)
 D1 - (M2) Must be 1 for M1/M3 to change screen height in Mode 4.
      Otherwise has no effect.
 D0 - 1= No sync, display is monochrome, 0= Normal display

 Bits 0 and 5 have no effect on the GameGear in either mode, while bit 6
 has no effect in GG mode but works normally in SMS mode.
 */

 /*
  Register $01 - Mode Control No. 2

  D7 - No effect
  D6 - (BLK) 1= Display visible, 0= display blanked.
  D5 - (IE0) 1= Frame interrupt enable.
  D4 - (M1) Selects 224-line screen for Mode 4 if M2=1, else has no effect.
  D3 - (M3) Selects 240-line screen for Mode 4 if M2=1, else has no effect.
  D2 - No effect
  D1 - Sprites are 1=16x16,0=8x8 (TMS9918), Sprites are 1=8x16,0=8x8 (Mode 4)
  D0 - Sprite pixels are doubled in size.

 Even though some games set bit 7, it does nothing.
 */

static void end_of_frame(running_machine &machine, struct sms_vdp *chip)
{
	UINT8 m1 = (chip->regs[0x1]&0x10)>>4;
	UINT8 m2 = (chip->regs[0x0]&0x02)>>1;
	UINT8 m3 = (chip->regs[0x1]&0x08)>>3;
	UINT8 m4 = (chip->regs[0x0]&0x04)>>2;
	UINT8 m5 = chip->is_pal;
	chip->screen_mode = m1|(m2<<1)|(m3<<2)|(m4<<3)|(m5<<4);

	if (chip->vdp_type!=GG_VDP) /* In GG mode the Game Gear resolution is fixed */
	{
		rectangle visarea;

		visarea.min_x = 0;
		visarea.max_x = 256-1;
		visarea.min_y = 0;
		visarea.max_y = sms_mode_table[chip->screen_mode].sms2_height-1;

		if (chip->chip_id==3) machine.primary_screen->configure(256, 256, visarea, HZ_TO_ATTOSECONDS(chip->sms_framerate));

	}
	else /* 160x144 */
	{
		rectangle visarea;
		visarea.min_x = (256-160)/2;
		visarea.max_x = (256-160)/2+160-1;
		visarea.min_y = (192-144)/2;
		visarea.max_y = (192-144)/2+144-1;

		machine.primary_screen->configure(256, 256, visarea, HZ_TO_ATTOSECONDS(chip->sms_framerate));
	}



//  printf("Mode: %s is ok\n", sms_mode_table[chip->screen_mode].sms2_name);

	chip->sms_scanline_counter = -1;
	chip->yscroll = chip->regs[0x9]; // this can't change mid-frame
	chip->sms_scanline_timer->adjust(attotime::zero);
}


SCREEN_EOF(sms)
{
	end_of_frame(machine, md_sms_vdp);

	// the SMS has a 'RESET' button on the machine, it generates an NMI
	if (input_port_read_safe(machine,"PAUSE",0x00))
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_NMI, PULSE_LINE);

}


VIDEO_START(sms)
{

}


static MACHINE_RESET(sms)
{
	md_sms_vdp->sms_scanline_timer->adjust(attotime::zero);
}



UINT8* vdp2_vram_bank0;
UINT8* vdp2_vram_bank1;

UINT8* vdp1_vram_bank0;
UINT8* vdp1_vram_bank1;

void segae_set_vram_banks(UINT8 data)
{
	if (data&0x80)
	{
		vdp1->vram = vdp1_vram_bank1;
	}
	else
	{
		vdp1->vram = vdp1_vram_bank0;
	}

	if (data&0x40)
	{
		vdp2->vram = vdp2_vram_bank1;
	}
	else
	{
		vdp2->vram = vdp2_vram_bank0;
	}


}

MACHINE_RESET(systeme)
{
	vdp1->sms_scanline_timer->adjust(attotime::zero);
	vdp2->sms_scanline_timer->adjust(attotime::zero);
}

MACHINE_RESET(megatech_md_sms)
{
	md_sms_vdp->sms_scanline_timer->adjust(attotime::zero);
}

MACHINE_RESET(megatech_bios)
{
	vdp1->sms_scanline_timer->adjust(attotime::zero);
}

SCREEN_EOF(systeme)
{
	end_of_frame(machine, vdp1);
	end_of_frame(machine, vdp2);
}


SCREEN_EOF(megatech_md_sms)
{
	end_of_frame(machine, md_sms_vdp);
}

SCREEN_EOF(megatech_bios)
{
	end_of_frame(machine, vdp1);
}

SCREEN_UPDATE(megatech_md_sms)
{
	int x,y;

	for (y=0;y<224;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(md_sms_vdp->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			lineptr[x]=srcptr[x]&0x7fff;
		}
	}

	return 0;
}


SCREEN_UPDATE(megatech_bios)
{
	int x,y;

	for (y=0;y<224;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(vdp1->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			lineptr[x]=srcptr[x]&0x7fff;
		}
	}

	return 0;
}

SCREEN_UPDATE(megaplay_bios)
{
	int x,y;

	for (y=0;y<224;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y+16, 32);
		UINT16* srcptr =  BITMAP_ADDR16(vdp1->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			UINT16 src = srcptr[x]&0x7fff;

			if (src)
				lineptr[x]=srcptr[x]&0x7fff;
		}
	}

	return 0;
}

SCREEN_UPDATE(systeme)
{
//  show_tiles();
	int x,y;

	for (y=0;y<192;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(vdp1->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			lineptr[x]=srcptr[x]&0x7fff;
		}

	}

	for (y=0;y<192;y++)
	{
		UINT16* lineptr = BITMAP_ADDR16(bitmap, y, 0);
		UINT16* srcptr =  BITMAP_ADDR16(vdp2->r_bitmap, y, 0);

		for (x=0;x<256;x++)
		{
			if(!(srcptr[x]&0x8000)) lineptr[x]=srcptr[x]&0x7fff;
		}
	}


	return 0;
}




READ8_HANDLER( sms_vdp_2_data_r )
{
	return vdp_data_r(vdp2);
}

WRITE8_HANDLER( sms_vdp_2_data_w )
{
	vdp_data_w(space, data, vdp2);
}

READ8_HANDLER( sms_vdp_2_ctrl_r )
{
	return vdp_ctrl_r(space, vdp2);
}

WRITE8_HANDLER( sms_vdp_2_ctrl_w )
{
	vdp_ctrl_w(space, data, vdp2);
}


void init_for_megadrive(running_machine &machine)
{
	md_sms_vdp = (struct sms_vdp *)start_vdp(machine, GEN_VDP);
	md_sms_vdp->set_irq = sms_vdp_cpu1_irq_callback;
	md_sms_vdp->is_pal = 0;
	md_sms_vdp->sms_total_scanlines = 262;
	md_sms_vdp->sms_framerate = 60;
	md_sms_vdp->chip_id = 3;
}



DRIVER_INIT( megatech_bios )
{
	vdp1 = (struct sms_vdp *)start_vdp(machine, SMS2_VDP);
	vdp1->set_irq = sms_vdp_cpu2_irq_callback;
	vdp1->is_pal = 0;
	vdp1->sms_total_scanlines = 262;
	vdp1->sms_framerate = 60;
	vdp1->chip_id = 1;

	vdp1_vram_bank0 = vdp1->vram;
	vdp1_vram_bank1 = auto_alloc_array(machine, UINT8, 0x4000);

	smsgg_backupram = 0;
}

DRIVER_INIT( smscm )
{
	megatech_set_genz80_as_sms_standard_map(machine, "maincpu", MAPPER_CODEMASTERS);

	md_sms_vdp = (struct sms_vdp *)start_vdp(machine, SMS2_VDP);
	md_sms_vdp->set_irq = sms_vdp_cpu0_irq_callback;
	md_sms_vdp->is_pal = 1;
	md_sms_vdp->sms_total_scanlines = 313;
	md_sms_vdp->sms_framerate = 50;
	md_sms_vdp->chip_id = 3;

	vdp1_vram_bank0 = md_sms_vdp->vram;
	vdp1_vram_bank1 = auto_alloc_array(machine, UINT8, 0x4000);

	smsgg_backupram = 0;
}

DRIVER_INIT( smspal )
{
	megatech_set_genz80_as_sms_standard_map(machine, "maincpu", MAPPER_STANDARD);

	md_sms_vdp = (struct sms_vdp *)start_vdp(machine, SMS2_VDP);
	md_sms_vdp->set_irq = sms_vdp_cpu0_irq_callback;
	md_sms_vdp->is_pal = 1;
	md_sms_vdp->sms_total_scanlines = 313;
	md_sms_vdp->sms_framerate = 50;
	md_sms_vdp->chip_id = 3;

	vdp1_vram_bank0 = md_sms_vdp->vram;
	vdp1_vram_bank1 = auto_alloc_array(machine, UINT8, 0x4000);

	smsgg_backupram = 0;
}

DRIVER_INIT( sms )
{
	megatech_set_genz80_as_sms_standard_map(machine, "maincpu", MAPPER_STANDARD);

	md_sms_vdp = (struct sms_vdp *)start_vdp(machine, SMS2_VDP);
	md_sms_vdp->set_irq = sms_vdp_cpu0_irq_callback;
	md_sms_vdp->is_pal = 0;
	md_sms_vdp->sms_total_scanlines = 262;
	md_sms_vdp->sms_framerate = 60;
	md_sms_vdp->chip_id = 3;

	vdp1_vram_bank0 = md_sms_vdp->vram;
	vdp1_vram_bank1 = auto_alloc_array(machine, UINT8, 0x4000);

	smsgg_backupram = 0;
}

static UINT8 ioport_gg00_r(running_machine& machine)
{
	UINT8 GG_START_BUTTON = input_port_read_safe(machine,"GGSTART",0x00);

	return (GG_START_BUTTON << 7) |
		   (0               << 6) |
		   (0               << 5) |
		   (0               << 4) |
		   (0               << 3) |
		   (0               << 2) |
		   (0               << 1) |
		   (0               << 0);
}


READ8_HANDLER( sms_ioport_gg00_r )
{
	return ioport_gg00_r(space->machine());
}


void init_extra_gg_ports(running_machine& machine, const char* tag)
{
	address_space *io = machine.device(tag)->memory().space(AS_IO);
	io->install_legacy_read_handler     (0x00, 0x00, FUNC(sms_ioport_gg00_r));
}

DRIVER_INIT( smsgg )
{
	megatech_set_genz80_as_sms_standard_map(machine, "maincpu", MAPPER_STANDARD);
	init_extra_gg_ports(machine, "maincpu");

	md_sms_vdp = (struct sms_vdp *)start_vdp(machine, GG_VDP);
	md_sms_vdp->set_irq = sms_vdp_cpu0_irq_callback;
	md_sms_vdp->is_pal = 0;
	md_sms_vdp->sms_total_scanlines = 262;
	md_sms_vdp->sms_framerate = 60;
	md_sms_vdp->chip_id = 3;

	smsgg_backupram = 0;
}


DRIVER_INIT( hazemd_segasyse )
{
	vdp1 = (struct sms_vdp *)start_vdp(machine, SMS2_VDP);
//  vdp1->set_irq = sms_vdp_cpu0_irq_callback;
	vdp1->is_pal = 0;
	vdp1->sms_total_scanlines = 262;
	vdp1->sms_framerate = 60;
	vdp1->chip_id = 1;

	vdp1_vram_bank0 = vdp1->vram;
	vdp1_vram_bank1 = auto_alloc_array(machine, UINT8, 0x4000);


	vdp2 = (struct sms_vdp *)start_vdp(machine, SMS2_VDP);
	vdp2->set_irq = sms_vdp_cpu0_irq_callback;
	vdp2->is_pal = 0;
	vdp2->sms_total_scanlines = 262;
	vdp2->sms_framerate = 60;
	vdp2->chip_id = 2;

	vdp2_vram_bank0 = vdp2->vram;
	vdp2_vram_bank1 = auto_alloc_array(machine, UINT8, 0x4000);
}

/* Functions to set up the Memory Map

 -- these are needed because on a Genesis the Sound Z80 becomes the SMS Z80 in comptibility mode
    so we need to be able to dynamically change the mapping

*/

static READ8_HANDLER( z80_unmapped_port_r )
{
//  printf("unmapped z80 port read %04x\n",offset);
	return 0;
}

static WRITE8_HANDLER( z80_unmapped_port_w )
{
//  printf("unmapped z80 port write %04x\n",offset);
}

static READ8_HANDLER( z80_unmapped_r )
{
	printf("unmapped z80 read %04x\n",offset);
	return 0;
}

static WRITE8_HANDLER( z80_unmapped_w )
{
	printf("unmapped z80 write %04x\n",offset);
}

static UINT8* sms_rom;


/* the SMS inputs should be more complex, like the megadrive ones */
READ8_HANDLER (megatech_sms_ioport_dc_r)
{
	running_machine &machine = space->machine();
	/* 2009-05 FP: would it be worth to give separate inputs to SMS? SMS has only 2 keys A,B (which are B,C on megadrive) */
	/* bit 4: TL-A; bit 5: TR-A */
	return (input_port_read(machine, "PAD1") & 0x3f) | ((input_port_read(machine, "PAD2") & 0x03) << 6);
}

READ8_HANDLER (megatech_sms_ioport_dd_r)
{
	running_machine &machine = space->machine();
	/* 2009-05 FP: would it be worth to give separate inputs to SMS? SMS has only 2 keys A,B (which are B,C on megadrive) */
	/* bit 2: TL-B; bit 3: TR-B; bit 4: RESET; bit 5: unused; bit 6: TH-A; bit 7: TH-B*/
	return ((input_port_read(machine, "PAD2") & 0x3c) >> 2) | 0x10;
}


READ8_HANDLER( smsgg_backupram_r )
{
	return smsgg_backupram[offset];
}

WRITE8_HANDLER( smsgg_backupram_w )
{
	smsgg_backupram[offset] = data;
}


static WRITE8_HANDLER( mt_sms_standard_rom_bank_w )
{
	int bank = data&0x1f;
	//logerror("bank w %02x %02x\n", offset, data);

	sms_mainram[0x1ffc+offset] = data;
	switch (offset)
	{
		case 0:
			logerror("bank w %02x %02x\n", offset, data);
			if ((data & 0x08) && smsgg_backupram)
			{
				space->install_legacy_readwrite_handler(0x8000, 0x9fff, FUNC(smsgg_backupram_r), FUNC(smsgg_backupram_w));
			}
			else
			{
				space->install_rom(0x0000, 0xbfff, sms_rom);
				space->unmap_write(0x0000, 0xbfff);
			}

			//printf("bank ram??\n");
			break;
		case 1:
			memcpy(sms_rom+0x0000, space->machine().region("maincpu")->base()+bank*0x4000, 0x4000);
			break;
		case 2:
			memcpy(sms_rom+0x4000, space->machine().region("maincpu")->base()+bank*0x4000, 0x4000);
			break;
		case 3:
			memcpy(sms_rom+0x8000, space->machine().region("maincpu")->base()+bank*0x4000, 0x4000);
			break;

	}
}

static WRITE8_HANDLER( codemasters_rom_bank_0000_w )
{
	int bank = data&0x1f;
	memcpy(sms_rom+0x0000, space->machine().region("maincpu")->base()+bank*0x4000, 0x4000);
}

static WRITE8_HANDLER( codemasters_rom_bank_4000_w )
{
	int bank = data&0x1f;
	memcpy(sms_rom+0x4000, space->machine().region("maincpu")->base()+bank*0x4000, 0x4000);
}

static WRITE8_HANDLER( codemasters_rom_bank_8000_w )
{
	int bank = data&0x1f;
	memcpy(sms_rom+0x8000, space->machine().region("maincpu")->base()+bank*0x4000, 0x4000);
}


static void megatech_set_genz80_as_sms_standard_ports(running_machine &machine, const char* tag)
{
	/* INIT THE PORTS *********************************************************************************************/

	address_space *io = machine.device(tag)->memory().space(AS_IO);
	device_t *sn = machine.device("snsnd");

	io->install_legacy_readwrite_handler(0x0000, 0xffff, FUNC(z80_unmapped_port_r), FUNC(z80_unmapped_port_w));

	io->install_legacy_read_handler      (0x7e, 0x7e, FUNC(md_sms_vdp_vcounter_r));
	io->install_legacy_write_handler(*sn, 0x7e, 0x7f, FUNC(sn76496_w));
	io->install_legacy_readwrite_handler (0xbe, 0xbe, FUNC(md_sms_vdp_data_r), FUNC(md_sms_vdp_data_w));
	io->install_legacy_readwrite_handler (0xbf, 0xbf, FUNC(md_sms_vdp_ctrl_r), FUNC(md_sms_vdp_ctrl_w));

	io->install_legacy_read_handler      (0x10, 0x10, FUNC(megatech_sms_ioport_dd_r)); // super tetris

	io->install_legacy_read_handler      (0xdc, 0xdc, FUNC(megatech_sms_ioport_dc_r));
	io->install_legacy_read_handler      (0xdd, 0xdd, FUNC(megatech_sms_ioport_dd_r));
	io->install_legacy_read_handler      (0xde, 0xde, FUNC(megatech_sms_ioport_dd_r));
	io->install_legacy_read_handler      (0xdf, 0xdf, FUNC(megatech_sms_ioport_dd_r)); // adams family
}

void megatech_set_genz80_as_sms_standard_map(running_machine &machine, const char* tag, int mapper)
{
	/* INIT THE MEMMAP / BANKING *********************************************************************************/

	/* catch any addresses that don't get mapped */
	machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_readwrite_handler(0x0000, 0xffff, FUNC(z80_unmapped_r), FUNC(z80_unmapped_w));

	/* main ram area */
	sms_mainram = (UINT8 *)machine.device(tag)->memory().space(AS_PROGRAM)->install_ram(0xc000, 0xdfff, 0, 0x2000);
	memset(sms_mainram,0x00,0x2000);

	megatech_set_genz80_as_sms_standard_ports(machine,  tag);

	/* fixed rom bank area */
	sms_rom = (UINT8 *)machine.device(tag)->memory().space(AS_PROGRAM)->install_rom(0x0000, 0xbfff, NULL);

	memcpy(sms_rom, machine.region("maincpu")->base(), 0xc000);

	if (mapper == MAPPER_STANDARD )
	{


		machine.device(tag)->memory().space(AS_PROGRAM)->install_legacy_write_handler(0xfffc, 0xffff, FUNC(mt_sms_standard_rom_bank_w));

	}
	else if (mapper == MAPPER_CODEMASTERS )
	{
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x0000, 0x0000, FUNC(codemasters_rom_bank_0000_w));
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x4000, 0x4000, FUNC(codemasters_rom_bank_4000_w));
		machine.device("maincpu")->memory().space(AS_PROGRAM)->install_legacy_write_handler(0x8000, 0x8000, FUNC(codemasters_rom_bank_8000_w));
	}
//  smsgg_backupram = NULL;
}

static NVRAM_HANDLER( sms )
{
	if (smsgg_backupram!=NULL)
	{
		if (read_or_write)
			file->write(smsgg_backupram, 0x2000);
		else
		{
			if (file)
			{
				file->read(smsgg_backupram, 0x2000);
			}
		}
	}
}

MACHINE_CONFIG_START( sms, driver_device )
	MCFG_CPU_ADD("maincpu", Z80, 3579540)
	//MCFG_CPU_PROGRAM_MAP(sms_map)
	MCFG_CPU_IO_MAP(sms_io_map)

	/* IRQ handled via the timers */
	MCFG_MACHINE_RESET(sms)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_RGB15)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 223)
//  MCFG_SCREEN_VISIBLE_AREA(0, 255, 0, 191)
	MCFG_SCREEN_UPDATE(megatech_md_sms) /* Copies a bitmap */
	MCFG_SCREEN_EOF(sms) /* Used to Sync the timing */

	MCFG_PALETTE_LENGTH(0x200)

	MCFG_VIDEO_START(sms)

	MCFG_NVRAM_HANDLER( sms )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("snsnd", SN76496, 3579540)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END



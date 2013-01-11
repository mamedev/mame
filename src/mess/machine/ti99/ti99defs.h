/****************************************************************************

    Common definitions for TI family
    Should be included in each component.

    Michael Zapf

*****************************************************************************/

#ifndef __TI99DEFS__
#define __TI99DEFS__

// TI-99/4(A)
#define region_grom     "cons_grom"
#define region_grom_cart    "cart_grom"
#define TMS9901_TAG     "tms9901"
#define TIBOARD_TAG     "ti_board"
#define DATAMUX_TAG     "datamux_16_8"
#define VIDEO_SYSTEM_TAG "video"
#define SCREEN_TAG      "screen"
#define TISOUNDCHIP_TAG "soundchip"
#define TISOUND_TAG     "tisound"
#define GROMPORT_TAG    "gromport"
#define GROM0_TAG       "console_grom_0"
#define GROM1_TAG       "console_grom_1"
#define GROM2_TAG       "console_grom_2"
#define PERIBOX_TAG     "peb"
#define MECMOUSE_TAG    "mecmouse"
#define HANDSET_TAG     "handset"
#define JOYPORT_TAG     "joyport"
#define VDP_TAG         "vdp"

#define GROMFREQ 10700000.0/24

// TI-99/8
#define SRAM_TAG        "sram8"
#define DRAM_TAG        "dram8"
#define MAPPER_TAG      "mapper"
#define SPEECH_TAG      "speech"

// Geneve
#define GKEYBOARD_TAG   "gkeyboard"
#define GMAPPER_TAG     "gmapper"
#define GMOUSE_TAG      "gmouse"
#define GCLOCK_TAG      "mm58274c"

#define READ16Z_MEMBER(name)            void name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset,  ATTR_UNUSED UINT16 *value, ATTR_UNUSED UINT16 mem_mask)
#define DECLARE_READ16Z_MEMBER(name)    void name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT16 *value, ATTR_UNUSED UINT16 mem_mask = 0xffff)
#define READ8Z_MEMBER(name)             void name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 *value, ATTR_UNUSED UINT8 mem_mask)
#define DECLARE_READ8Z_MEMBER(name)     void name(ATTR_UNUSED address_space &space, ATTR_UNUSED offs_t offset, ATTR_UNUSED UINT8 *value, ATTR_UNUSED UINT8 mem_mask = 0xff)

#define GENMOD 0x01

/*
    Base class for all devices that have a special read method. This read
    methods differs from the standard READ8/16 by allowing for a "high-impedance"
    state "Z" which means that the bus lines are not changed. That way, we can
    built busses with parallel devices as in the real machine, and only the
    active device changes the bus lines.
*/
class bus8z_device : public device_t
{
public:
	bus8z_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, type, name, tag, owner, clock) { }
	virtual DECLARE_READ8Z_MEMBER(readz) =0;
	virtual DECLARE_WRITE8_MEMBER(write) =0;
};

class bus16z_device : device_t
{
public:
	virtual DECLARE_READ16Z_MEMBER(read16z) =0;
	virtual DECLARE_WRITE16_MEMBER(write16) =0;
};

/****************************************************************************
    Constants
****************************************************************************/

enum
{
	GM_TURBO = 1,
	GM_TIM = 2
};

enum
{
	GENEVE_098 = 0,
	GENEVE_100 = 1
};

#endif

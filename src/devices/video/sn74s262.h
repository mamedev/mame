// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Texas Instruments SN74S262N Row Output Character Generator emulation

**********************************************************************
                            _____   _____
                CHAR E   1 |*    \_/     | 20  Vcc
                CHAR F   2 |             | 19  CHAR D
                CHAR G   3 |             | 18  CHAR C
                 ROW A   4 |             | 17  CHAR B
                 ROW B   5 |  SN74S262N  | 16  CHAR A
                 ROW C   6 |  SN74S263N  | 15  _Me1
                 ROW D   7 |             | 14  _Me2
                    Y1   8 |             | 13  Y5
                    Y2   9 |             | 12  Y4
                   GND  10 |_____________| 11  Y3

**********************************************************************/

#ifndef MAME_VIDEO_SN74S262_H
#define MAME_VIDEO_SN74S262_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sn74s262_device

class sn74s262_device : public device_t,
						public device_gfx_interface
{
public:
	// construction/destruction
	sn74s262_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	u8 read(u8 character, u8 row);

protected:
	sn74s262_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	static const gfx_layout charlayout;
	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	required_region_ptr<uint8_t> m_char_rom;
};


// ======================> sn74s263_device

class sn74s263_device : public sn74s262_device
{
public:
	// construction/destruction
	sn74s263_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(SN74S262, sn74s262_device) // English
DECLARE_DEVICE_TYPE(SN74S263, sn74s263_device) // Swedish/Finnish

#endif // MAME_VIDEO_SN74S262_H

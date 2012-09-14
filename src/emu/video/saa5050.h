/**********************************************************************

    Mullard SAA5050 Teletext Character Generator emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 28  DE
                   _SI   2 |             | 27  PO
                 _DATA   3 |             | 26  LOSE
                    D1   4 |   SAA5050   | 25  BLAN
                    D2   5 |   SAA5051   | 24  R
                    D3   6 |   SAA5052   | 23  G
                    D4   7 |   SAA5053   | 22  B
                    D5   8 |   SAA5054   | 21  Y
                    D6   9 |   SAA5055   | 20  F1
                    D7  10 |   SAA5056   | 19  TR6
                  DLIM  11 |   SAA5057   | 18  Vdd
                  _GLR  12 |             | 17  N/C
                   DEW  13 |             | 16  _TLC
                   CRS  14 |_____________| 15  _BCS

**********************************************************************/

#pragma once

#ifndef __SAA5050__
#define __SAA5050__

#include "emu.h"



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SAA5050_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, SAA5050, _clock) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_SAA5052_ADD(_tag, _clock, _config) \
	MCFG_DEVICE_ADD(_tag, SAA5052, _clock) \
	MCFG_DEVICE_CONFIG(_config)


#define SAA5050_INTERFACE(name) \
	const saa5050_interface (name) =



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saa5050_interface

struct saa5050_interface
{
	devcb_read8 m_in_d_cb;

	int m_cols;
	int m_rows;
	int m_size;
};


// ======================> saa5050_device

class saa5050_device :	public device_t,
                        public saa5050_interface
{
public:
    // construction/destruction
    saa5050_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant);
    saa5050_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

	DECLARE_WRITE_LINE_MEMBER( dew_w );
	DECLARE_WRITE_LINE_MEMBER( lose_w );
	void write(UINT8 data);
	DECLARE_WRITE_LINE_MEMBER( f1_w );
	DECLARE_WRITE_LINE_MEMBER( tr6_w );
	int get_rgb();

	// NOTE: the following are provided for convenience only, SAA5050 is not a display controller
	// this emulates the common setup where bit 7 of data inverts the display, and the
	// bottom half of a double height row gets the same character data as the top half 
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	enum
	{
		TYPE_5050,
		TYPE_5052
	};

    // device-level overrides
	virtual void device_config_complete();
    virtual void device_start();

private:
	void process_control_character(UINT8 data);
	void get_character_data(UINT8 data);

	devcb_resolved_read8	m_in_d_func;

	const UINT8 *m_char_rom;
	UINT8 m_code;
	UINT8 m_last_code;
	UINT8 m_char_data;
	int m_bit;
	rgb_t m_color;
	int m_ra;
	int m_bg;
	int m_fg;
	bool m_graphics;
	bool m_separated;
	bool m_conceal;
	bool m_flash;
	bool m_boxed;
	int m_double_height;
	bool m_double_height_top_row;
	bool m_double_height_bottom_row;
	bool m_hold;
	int m_frame_count;

	int m_variant;
};


// ======================> saa5052_device

class saa5052_device :  public saa5050_device
{
public:
    // construction/destruction
    saa5052_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type SAA5050;
extern const device_type SAA5052;



#endif

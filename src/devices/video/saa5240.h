// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*********************************************************************

    Philips SAA5240 European Controlled Teletext Circuit

**********************************************************************

    Pinning:                ____  ____
                           |    \/    |
                   Vdd   1 |          | 40  A10
                   A11   2 |          | 39  A9
                   A12   3 |          | 38  A8
                   nOE   4 |          | 37  A7
                   nWE   5 |          | 36  A6
                   TTD   6 |          | 35  A5
                   TTC   7 |          | 34  A4
                   HDK   8 |          | 33  A3
                    F6   9 |          | 32  A2
                   VCS  10 | SAA5240  | 31  A1
                  SAND  11 |          | 30  A0
             nTCS/nSCS  12 |          | 29  D7
                     R  13 |          | 28  D6
                     G  14 |          | 27  D5
                     B  15 |          | 26  D4
                  nCOR  16 |          | 25  D3
                  BLAN  17 |          | 24  D2
                     Y  18 |          | 23  D1
                   SCL  19 |          | 22  D0
                   SDA  20 |          | 21  Vss
                           |__________|

*********************************************************************/

#ifndef MAME_VIDEO_SAA5240_H
#define MAME_VIDEO_SAA5240_H

#pragma once



//**************************************************************************
//  CONSTANTS
//**************************************************************************

#define SAA5240_SLAVE_ADDRESS ( 0x22 )


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> saa5240_device

class saa5240_device :
	public device_t,
	public device_memory_interface
{
public:
	// construction/destruction
	saa5240_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write_scl(int state);
	void write_sda(int state);
	int read_sda();

	void vcs_w(int state);
	void f6_w(int state);
	int get_rgb();

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	saa5240_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual space_config_vector memory_space_config() const override;

private:
	required_region_ptr<uint16_t> m_char_rom;

	const address_space_config m_space_config;
	address_space *m_videoram;

	// internal state
	uint8_t m_register[12];
	int m_slave_address;
	int m_i2c_scl;
	int m_i2c_sdaw;
	int m_i2c_sdar;
	int m_i2c_state;
	int m_i2c_bits;
	int m_i2c_shift;
	int m_i2c_devsel;
	int m_i2c_address;

	void increment_active_data();
	void update_active_data();

	enum { STATE_IDLE, STATE_DEVSEL, STATE_ADDRESS, STATE_DATAIN, STATE_DATAOUT };

	void saa5240_vram(address_map &map) ATTR_COLD;

	enum
	{
		NUL = 0,
		ALPHA_RED,
		ALPHA_GREEN,
		ALPHA_YELLOW,
		ALPHA_BLUE,
		ALPHA_MAGENTA,
		ALPHA_CYAN,
		ALPHA_WHITE,
		FLASH,
		STEADY,
		END_BOX,
		START_BOX,
		NORMAL_HEIGHT,
		DOUBLE_HEIGHT,
		S0,
		S1,
		DLE,
		GRAPHICS_RED,
		GRAPHICS_GREEN,
		GRAPHICS_YELLOW,
		GRAPHICS_BLUE,
		GRAPHICS_MAGENTA,
		GRAPHICS_CYAN,
		GRAPHICS_WHITE,
		CONCEAL_DISPLAY,
		CONTIGUOUS_GFX,
		SEPARATED_GFX,
		ESC,
		BLACK_BACKGROUND,
		NEW_BACKGROUND,
		HOLD_GRAPHICS,
		RELEASE_GRAPHICS
	};

	void process_control_character(uint8_t data);
	void set_next_chartype();
	uint16_t get_gfx_data(uint8_t data, offs_t row, bool separated);
	uint16_t get_rom_data(uint8_t data, offs_t row);
	void get_character_data(uint8_t data);

	uint8_t m_held_char;
	uint8_t m_next_chartype;
	uint8_t m_curr_chartype;
	uint8_t m_held_chartype;
	uint16_t m_char_data;
	int m_bit;
	rgb_t m_color;
	int m_crs;
	int m_ra;
	int m_bg;
	int m_fg;
	int m_prev_col;
	bool m_graphics;
	bool m_separated;
	bool m_flash;
	bool m_boxed;
	bool m_double_height;
	bool m_double_height_old;
	bool m_double_height_bottom_row;
	bool m_double_height_prev_row;
	bool m_hold_char;
	bool m_hold_clear;
	bool m_hold_off;
	int m_frame_count;

	// internal helpers
	int calc_parity(uint8_t data);
};


// ======================> saa5240a_device

class saa5240a_device :  public saa5240_device
{
public:
	// construction/destruction
	saa5240a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> saa5240b_device

class saa5240b_device :  public saa5240_device
{
public:
	// construction/destruction
	saa5240b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};


// ======================> saa5243e_device

class saa5243e_device :  public saa5240_device
{
public:
	// construction/destruction
	saa5243e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};



// device type definition
DECLARE_DEVICE_TYPE(SAA5240A, saa5240a_device) // English, German, Swedish
DECLARE_DEVICE_TYPE(SAA5240B, saa5240b_device) // Italian, German, French
DECLARE_DEVICE_TYPE(SAA5243E, saa5243e_device) // West European
//DECLARE_DEVICE_TYPE(SAA5243H, saa5243h_device) // East European


#endif // MAME_VIDEO_SAA5240_H

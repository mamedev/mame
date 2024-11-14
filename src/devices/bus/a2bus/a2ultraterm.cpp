// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2ultraterm.c

    Implementation of the Videx UltraTerm 80/132/160-column card

    Notes:

    C0nX: C0n0 is 6845 register address,
          C0n1 is 6845 register data.
          C0n2 is control 1: b7 = 0 to read RAM at cc00, 1 for ROM (writes always to RAM)
                             b6 = 0 for Apple II video, 1 for 6845
                             b5 = 0 for 17.430 MHz 6845 clock, 1 for 28.7595 MHz 6845 clock
                             b4 = 0 for 512 byte RAM block addressing (VideoTerm emulation), 1 for 256-byte RAM page addressing
                             b3-b0 = page select
          C0n3 is control 2: b7 = 0 for attributes software controllable, 1 for DIP switches control attributes
                             b5 = 0 for normal video if bit 7 set, 1 for inverse if bit 7 set
                             b4 = 0 for lowlight if bit 7 set, 1 for highlight if bit 7 set
                             b2 = 0 for high-density character set, 1 for low-density character set
                             b1 = same as b5
                             b0 = same as b4

    C800-CBFF: ROM page 1
    CC00-CFEF: VRAM window or ROM page 2

*********************************************************************/

#include "emu.h"
#include "a2ultraterm.h"

#include "video/mc6845.h"

#include "screen.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define ULTRATERM_ROM_REGION  "uterm_rom"
#define ULTRATERM_GFX_REGION  "uterm_gfx"
#define ULTRATERM_SCREEN_NAME "uterm_screen"
#define ULTRATERM_MC6845_NAME "mc6845_uterm"

#define CLOCK_LOW   17430000
#define CLOCK_HIGH  28759500

#define CT1_MEMSEL  (0x80)  // 0 for read RAM at cc00, 1 for read ROM
#define CT1_VIDSEL  (0x40)  // 0 for Apple video passthrough, 1 for 6845 video
#define CT1_CLKSEL  (0x20)  // 0 for Videoterm clock, 1 for faster clock
#define CT1_VTEMU   (0x10)  // Videoterm emulation mode if 0
#define CT1_PAGEMASK (0x0f)

#define CT2_USEDIPS (0x80)  // 0 to use the rest of ctrl2's bits, 1 to use DIPs
#define CT2_INVBIT7H (0x20)
#define CT2_HLBIT7H (0x10)
#define CT2_HIDENSITY (0x04)
#define CT2_INVBIT7L (0x02)
#define CT2_HLBIT7L (0x01)


const rgb_t ultraterm_palette[4] =
{
	rgb_t(0x00,0x00,0x00),
	rgb_t(0x55,0x55,0x55),
	rgb_t(0xaa,0xaa,0xaa),
	rgb_t(0xff,0xff,0xff)
};


ROM_START( a2ultraterm )
	ROM_REGION(0x1000, ULTRATERM_ROM_REGION, 0)
	ROM_LOAD( "frm_b537.bin", 0x000000, 0x001000, CRC(1e85a93e) SHA1(b4acd1775c08ae43996ab4edf6d8e28f4736346b) )

	ROM_REGION(0x1000, ULTRATERM_GFX_REGION, 0)
	ROM_LOAD( "chs_7859.bin", 0x000000, 0x001000, CRC(ebe8f333) SHA1(3517fa9e7a39573f1cb159b3161d6939dec199ba) )

	ROM_REGION(0x400, "pal", 0)
	ROM_LOAD( "ult_2a313.jed", 0x000000, 0x000305, CRC(dcd51dea) SHA1(0ad0c5e802e48495da27f7bd26ee3ab1c92d74dd) )
ROM_END

ROM_START( a2ultratermenh )
	ROM_REGION(0x1000, ULTRATERM_ROM_REGION, 0)
	ROM_LOAD( "frm_b5c9.bin", 0x000000, 0x001000, CRC(b71e05e0) SHA1(092e3eda4644d4f465809864a7f023ac7d1d1542) )

	ROM_REGION(0x1000, ULTRATERM_GFX_REGION, 0)
	ROM_LOAD( "chs_5604.bin", 0x000000, 0x001000, CRC(3fb4e90a) SHA1(94ff75199232a9b613585c22f88470f73fb7dd09) )

	ROM_REGION(0x400, "pal", 0)
	ROM_LOAD( "ult_251c.jed", 0x000000, 0x000305, CRC(12fabb0d) SHA1(d4a36837cb98bb65f7ddef7455eb5a7f8e648a82) )
ROM_END

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_videx160_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	// construction/destruction
	a2bus_videx160_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

	uint8_t m_ram[256*16];
	int m_framecnt;
	uint8_t m_ctrl1, m_ctrl2;

	required_device<mc6845_device> m_crtc;
	required_region_ptr<uint8_t> m_rom, m_chrrom;

private:
	void vsync_changed(int state);
	MC6845_UPDATE_ROW(crtc_update_row);

	int m_rambank;
};

class a2bus_ultraterm_device : public a2bus_videx160_device
{
public:
	a2bus_ultraterm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class a2bus_ultratermenh_device : public a2bus_videx160_device
{
public:
	a2bus_ultratermenh_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_videx160_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, ULTRATERM_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_raw(CLOCK_LOW, 882, 0, 720, 370, 0, 350);
	screen.set_screen_update(ULTRATERM_MC6845_NAME, FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, CLOCK_LOW/9);
	m_crtc->set_screen(ULTRATERM_SCREEN_NAME);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(a2bus_videx160_device::crtc_update_row));
	m_crtc->out_vsync_callback().set(FUNC(a2bus_videx160_device::vsync_changed));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_ultraterm_device::device_rom_region() const
{
	return ROM_NAME( a2ultraterm );
}

const tiny_rom_entry *a2bus_ultratermenh_device::device_rom_region() const
{
	return ROM_NAME( a2ultratermenh );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_videx160_device::a2bus_videx160_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_framecnt(0), m_ctrl1(0), m_ctrl2(0),
	m_crtc(*this, ULTRATERM_MC6845_NAME),
	m_rom(*this, ULTRATERM_ROM_REGION),
	m_chrrom(*this, ULTRATERM_GFX_REGION),
	m_rambank(0)
{
}

a2bus_ultraterm_device::a2bus_ultraterm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_videx160_device(mconfig, A2BUS_ULTRATERM, tag, owner, clock)
{
}

a2bus_ultratermenh_device::a2bus_ultratermenh_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_videx160_device(mconfig, A2BUS_ULTRATERMENH, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_videx160_device::device_start()
{
	memset(m_ram, 0, 256*16);

	save_item(NAME(m_ram));
	save_item(NAME(m_framecnt));
	save_item(NAME(m_rambank));
	save_item(NAME(m_ctrl1));
	save_item(NAME(m_ctrl2));
}

void a2bus_videx160_device::device_reset()
{
	m_rambank = 0;
	m_framecnt = 0;
}


/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_videx160_device::read_c0nx(uint8_t offset)
{
//    printf("%s Read c0n%x\n", machine().describe_context().c_str(), offset);

	if (!(m_ctrl1 & CT1_VTEMU))
	{
		m_rambank = ((offset>>2) & 3) * 512;
	}

	switch (offset)
	{
		case 1:
			return m_crtc->register_r();   // status_r?

		case 2:
			return m_ctrl1;

		case 3:
			return m_ctrl2;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_videx160_device::write_c0nx(uint8_t offset, uint8_t data)
{
//    printf("%s Write %02x to c0n%x\n", machine().describe_context().c_str(), data, offset);

	switch (offset)
	{
		case 0:
			m_crtc->address_w(data);
			break;

		case 1:
			m_crtc->register_w(data);
			break;

		case 2:
			m_ctrl1 = data;
//          printf("%02x to ctrl1\n", data);

			// if disabling Videoterm emulation, change RAM banking
			if (data & CT1_VTEMU)
			{
				m_rambank = (data & CT1_PAGEMASK) * 256;
			}
			break;

		case 3:
			m_ctrl2 = data;
//          printf("%02x to ctrl2\n", data);
			break;
	}

	if (!(m_ctrl1 & CT1_VTEMU))
	{
		m_rambank = ((offset >> 2) & 3) * 512;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_videx160_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset+(slotno() * 0x100)];
}

/*-------------------------------------------------
    write_cnxx - called for writes to this card's cnxx space
    the firmware writes here to switch in our $C800 a lot
-------------------------------------------------*/
void a2bus_videx160_device::write_cnxx(uint8_t offset, uint8_t data)
{
}

/*-------------------------------------------------
    read_c800 - called for reads from this card's c800 space
-------------------------------------------------*/

uint8_t a2bus_videx160_device::read_c800(uint16_t offset)
{
	// ROM at c800-cbff
	// bankswitched RAM at cc00-cdff
	if (offset < 0x400)
	{
//        printf("Read VRAM at %x = %02x\n", offset+m_rambank, m_ram[offset + m_rambank]);
		return m_rom[offset + 0x800];
	}
	else
	{
		if (m_ctrl1 & CT1_MEMSEL)   // read ROM?
		{
			return m_rom[offset + 0x800];
		}

		return m_ram[(offset - 0x400) + m_rambank];
	}
}

/*-------------------------------------------------
    write_c800 - called for writes to this card's c800 space
-------------------------------------------------*/
void a2bus_videx160_device::write_c800(uint16_t offset, uint8_t data)
{
	if (offset >= 0x400)
	{
//        printf("%02x to VRAM at %x\n", data, offset-0x400+m_rambank);
		m_ram[(offset-0x400) + m_rambank] = data;
	}
}

MC6845_UPDATE_ROW( a2bus_videx160_device::crtc_update_row )
{
	uint32_t  *p = &bitmap.pix(y);
	uint16_t  chr_base = ra;

	for ( int i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ma + i );
		uint8_t chr = m_ram[ offset ];
		uint8_t data = m_chrrom[ chr_base + (chr * 16) ];
		uint8_t fg = 2;
		uint8_t bg = 0;
		uint8_t tmp;

		// apply attributes
		if (!(m_ctrl2 & CT2_USEDIPS))
		{
			// this set and bit 7 in char, highlight
			if ((m_ctrl2 & CT2_HLBIT7H) && (chr & 0x80))
			{
				fg = 3;
				bg = 0;
			}

			// this set and NOT bit 7 in char, highlight
			if ((m_ctrl2 & CT2_HLBIT7L) && (!(chr & 0x80)))
			{
				fg = 3;
				bg = 0;
			}

			// this clear and bit 7 in char, lowlight
			if (!(m_ctrl2 & CT2_HLBIT7H) && (chr & 0x80))
			{
				fg = 1;
				bg = 0;
			}

			// this clear and NOT bit 7 in char, lowlight
			if (!(m_ctrl2 & CT2_HLBIT7L) && (!(chr & 0x80)))
			{
				fg = 1;
				bg = 0;
			}

			// invert last so invert + hilight/invert + lowlight are possible
			// invert if char bit 7 is set
			if ((m_ctrl2 & CT2_INVBIT7H) && (chr & 0x80))
			{
				tmp = fg;
				fg = bg;
				bg = tmp;
			}

			// invert if char bit 7 is clear
			if ((m_ctrl2 & CT2_INVBIT7L) && (!(chr & 0x80)))
			{
				tmp = fg;
				fg = bg;
				bg = tmp;
			}
		}

		if ( i == cursor_x )
		{
			if ( m_framecnt & 0x08 )
			{
				data = 0xFF;
			}
		}

		*p = ultraterm_palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = ultraterm_palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

void a2bus_videx160_device::vsync_changed(int state)
{
	if ( state )
	{
		m_framecnt++;
	}
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ULTRATERM,    device_a2bus_card_interface, a2bus_ultraterm_device,    "a2ulttrm", "Videx UltraTerm (original)")
DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ULTRATERMENH, device_a2bus_card_interface, a2bus_ultratermenh_device, "a2ultrme", "Videx UltraTerm (enhanced //e)")

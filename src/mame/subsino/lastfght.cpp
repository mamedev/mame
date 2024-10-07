// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*********************************************************************************************************************

Last Fighting  (C)2000 Subsino

driver by Luca Elia

PCB Layout
----------

|------------------------------------------------------|
|TDA1519A                                              |
|     VOL                                              |
|                HM86171                       ULN2003 |
|   LM324                                              |
|           S-1                                ULN2003 |
|                                                      |
|                                    |-----|   ULN2003 |
|                        |-----|     |U1   |           |
|              V100.U7   |U6   |     |     |           |
|J                       |     |     |-----|           |
|A                       |-----|                       |
|M                                                     |
|M                                                     |
|A          KM428C256      32MHz     CXK58257          |
|     |----ROM BOARD------|                            |
|     |                   |          V106.U16          |
|     |          U19      |                         SW1|
|     |       |-------|   |          |-----|           |
|     |       |SUBSINO|   |          |H8   |           |
|     |       |9623EX |   | |-----|  |3044 |           |
|     |       |008    |   | |EPM  |  |-----|           |
|     |       |-------|   | |7032 |                    |
|     |     CN2           | |-----|             3V_BATT|
|-----|-------------------|----------------------------|

Notes:
      H8/3044 - Subsino re-badged Hitachi H8/3044 HD6433044A22F Microcontroller (QFP100)
                The H8/3044 is a H8/3002 with 24bit address bus and has 32k mask ROM and 2k RAM, clock input is 16.000MHz [32/2]
                MD0,MD1 & MD2 are configured to MODE 6 16M-Byte Expanded Mode with the on-chip 32k mask ROM enabled.
      EPM7032 - Altera EPM7032LC44-15T CPLD (PLCC44)
     CXK58257 - Sony CXK58257 32k x8 SRAM (SOP28)
    KM428C256 - Samsung Semiconductor KM428C256 256k x8 Dual Port DRAM (SOJ40)
     ULKN2003 - Toshiba ULN2003 High Voltage High Current Darlington Transistor Array comprising 7 NPN Darlington pairs (DIP16)
      HM86171 - Hualon Microelectronics HMC HM86171 VGA 256 colour RAMDAC (DIP28)
      3V_BATT - 3 Volt Coin Battery. This is tied to the CXK58257 SRAM. It appears to be used as an EEPROM, as the game
                has on-board settings in test mode and there's no DIPs and no EEPROM.
          S-1 - ?? Probably some kind of audio OP AMP or DAC? (DIP8)
     TDA1519A - Philips TDA1519A 22W BTL/Dual 11W Audio Power Amplifier IC (SIL9)
          CN2 - 70 pin connector for connection of ROM board
          SW1 - Push Button Test Switch
        HSync - 15.75kHz
        VSync - 60Hz
    ROM BOARD - Small Daughterboard containing positions for 8x 16MBit SOP44 mask ROMs. Only positions 1-4 are populated.
   Custom ICs -
                U19     - SUBSINO 9623EX008 (QFP208)
                H8/3044 - SUBSINO SS9689 6433044A22F, rebadged Hitachi H8/3044 MCU (QFP100)
                U1      - SUBSINO SS9802 9933 (QFP100)
                U6      - SUBSINO SS9804 0001 (QFP100)
         ROMs -
                V106.U16 - MX27C4000 4MBit DIP32 EPROM; Main Program
                V100.U7  - ST M27C801 8MBit DIP32 EPROM; Audio Samples?

    TODO:
     - blitter timing is guessed, definitely expect non-instant transfers otherwise game is too fast

    The EEPROM protection method is the same as in the subsino2.cpp games.

*********************************************************************************************************************/

#include "emu.h"
#include "subsino_io.h"
#include "cpu/h8/h83048.h"
#include "machine/ds2430a.h"
#include "machine/nvram.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"

#define DEBUG_GFX 0


namespace {

class lastfght_state : public driver_device
{
public:
	lastfght_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette")
	{ }

	void lastfght(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory */
	void hi_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void x_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void yw_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void h_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sx_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sy_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sr_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void blit_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void dest_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint8_t c00000_r();
	uint8_t c00002_r();
	void c00007_w(uint8_t data);
	uint16_t sound_r();
	void sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void lastfght_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;

	/* video-related */
	bitmap_ind16 m_bitmap[2];
	int m_dest = 0;
	int m_hi = 0;
	int m_sx = 0;
	int m_sx1 = 0;
	int m_dsx = 0;
	int m_sy = 0;
	int m_sy1 = 0;
	int m_dsy = 0;
	int m_sp = 0;
	int m_sr = 0;
	int m_x = 0;
	int m_y = 0;
	int m_w = 0;
	int m_h = 0;
#if DEBUG_GFX
	unsigned m_base = 0;
	int m_view_roms = 0;
#endif

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<ds2430a_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	bool m_blitter_busy = false;
	emu_timer *m_blitter_end_timer = nullptr;
	TIMER_CALLBACK_MEMBER(blitter_end_cb);
};


/***************************************************************************
                                Video Hardware
***************************************************************************/

void lastfght_state::video_start()
{
	for (int i = 0; i < 2; i++)
		m_screen->register_screen_bitmap(m_bitmap[i]);

	save_item(NAME(m_bitmap[0]));
	save_item(NAME(m_bitmap[1]));

	m_blitter_end_timer = timer_alloc(FUNC(lastfght_state::blitter_end_cb), this);
}


uint32_t lastfght_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
#if DEBUG_GFX
	// gfx roms viewer (toggle with enter, use pgup/down to browse)
	uint8_t const *const gfxdata = memregion("gfx1")->base();

	if (machine().input().code_pressed_once(KEYCODE_ENTER)) m_view_roms ^= 1;
	if (m_view_roms)
	{
		if (machine().input().code_pressed_once(KEYCODE_PGDN))  m_base += 512 * 256;
		if (machine().input().code_pressed_once(KEYCODE_PGUP))  m_base -= 512 * 256;
		m_base %= memregion("gfx1")->bytes();

		int count = m_base;

		bitmap.fill(m_palette->black_pen(), cliprect );
		for (int y = 0 ; y < 256; y++)
		{
			for (int x = 0; x < 512; x++)
			{
				uint8_t data = (((count & 0xf) == 0) && ((count & 0x1e00) == 0)) ? m_palette->white_pen() : gfxdata[count];   // white grid or data
				bitmap.pix(y, x) = data;
				count++;
			}
		}
		popmessage("%x", m_base);
		return 0;
	}
#endif

	copybitmap(bitmap, m_bitmap[m_dest ^ 1], 0, 0, 0, 0, cliprect);

	return 0;
}

//  Blitter (supports zooming)

// high byte of a 16 bit register
void lastfght_state::hi_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		logerror("%06x: 600000.b = %02x\n", m_maincpu->pc(), data >> 8);
	if (ACCESSING_BITS_0_7)
	{
		m_hi = data << 8;
		//logerror("%06x: hi  = %02x\n", m_maincpu->pc(), data);
	}
}

// screen x
void lastfght_state::x_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		logerror("%06x: 800008.b = %02x\n", m_maincpu->pc(), data >> 8);
	if (ACCESSING_BITS_0_7)
	{
		m_x = m_hi | data;
		//logerror("%06x: x   = %02x\n", m_maincpu->pc(),data);
	}
}

// screen y, screen width - 1
void lastfght_state::yw_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_y = m_hi | (data >> 8);
		//logerror("%06x: y   = %02x\n", m_maincpu->pc(), data >> 8);
	}
	if (ACCESSING_BITS_0_7)
	{
		m_w = m_hi | data;
		//logerror("%06x: w   = %02x\n", m_maincpu->pc(), data);
	}
}

// screen height - 1
void lastfght_state::h_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_h = m_hi | (data >> 8);
		//logerror("%06x: h   = %02x\n", m_maincpu->pc(), data >> 8);
	}
	if (ACCESSING_BITS_0_7)
		logerror("%06x: 80000d.b = %02x\n", m_maincpu->pc(), data);
}

// source delta x << 6, source x << 6
void lastfght_state::sx_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_dsx = m_hi | (data >> 8);
		//logerror("%06x: dsx = %02x\n", m_maincpu->pc(), data >> 8);
	}
	if (ACCESSING_BITS_0_7)
	{
		m_sx = m_hi | data;
		//logerror("%06x: sx  = %02x\n", m_maincpu->pc(), data);
	}
}

// source y << 6, source y1 << 6
void lastfght_state::sy_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_sy = m_hi | (data >> 8);
		//logerror("%06x: sy  = %02x\n", m_maincpu->pc(), data >> 8);
	}
	if (ACCESSING_BITS_0_7)
	{
		m_sy1 = m_hi | data;
		//logerror("%06x: sy1 = %02x\n", m_maincpu->pc(), data);
	}
}

// source rom (0x200000 bytes), source page (512x256 bytes)
void lastfght_state::sr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_sp = (m_hi >> 8) >> 4;
		//logerror("%06x: sp  = %02x\n", m_maincpu->pc(), data >> 8);
	}
	if (ACCESSING_BITS_0_7)
	{
		m_sr = data;
		//logerror("%06x: sr  = %02x\n", m_maincpu->pc(), data);
	}
}

// source x1 << 6, source delta y << 6
void lastfght_state::sd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		m_sx1 = m_hi | (data >> 8);
		//logerror("%06x: sx1 = %02x\n", m_maincpu->pc(), data >> 8);
	}
	if (ACCESSING_BITS_0_7)
	{
		m_dsy = m_hi | data;
		//logerror("%06x: dsy = %02x\n", m_maincpu->pc(), data);
	}
}

// start blit
void lastfght_state::blit_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
	{
		uint8_t *gfxdata = memregion( "gfx1" )->base();
		bitmap_ind16 &dest = m_bitmap[m_dest];

#if 0
		logerror("%06x: blit x %03x, y %03x, w %03x, h %03x, sx %03x.%02x, sx1 %03x.%02x, dsx %03x.%02x, sy %03x.%02x, sy1 %03x.%02x, dsy %03x.%02x, sp %02x, sr %02x, data %02x\n", m_maincpu->pc(),
				m_x, m_y, m_w + 1, m_h + 1,
				m_sx >> 6, m_sx & 0x3f, m_sx1 >> 6, m_dsx & 0x3f, m_sx1 >> 6, m_sx1 & 0x3f,
				m_sy >> 6, m_sy & 0x3f, m_sy1 >> 6, m_dsy & 0x3f, m_sy1 >> 6, m_sy1 & 0x3f,
				m_sp, m_sr,
				data >> 8);
#endif

		for (int y = 0; y <= m_h; y++)
		{
			for (int x = 0; x <= m_w; x++)
			{
				int addr = (((m_sx + m_sx1 + m_dsx * x) >> 6) & 0x1ff) +
							(((m_sy + m_sy1 + m_dsy * y) >> 6) & 0xff) * 0x200 +
							m_sp * 0x200 * 0x100 + m_sr * 0x200000;

				data = gfxdata[addr];

				if (data && (m_x + x >= 0) && (m_x + x < 512) && (m_y + y >= 0) && (m_y + y < 256))
					dest.pix(m_y + y, m_x + x) = data;
			}
		}
		m_blitter_busy = true;
		// num pixels x2 seems to match a reasonable timer countdown during gameplay.
		// notice that the other two bits (bit 6 and bit 5 in $c00007) are all
		// busy checks, implying multiple stall checks (drawing? vblank?).
		m_blitter_end_timer->adjust(m_maincpu->cycles_to_attotime(m_w * m_h * 2));
	}
	if (ACCESSING_BITS_0_7)
		logerror("%06x: 600007.b = %02x\n", m_maincpu->pc(), data);
}

TIMER_CALLBACK_MEMBER(lastfght_state::blitter_end_cb)
{
	m_blitter_busy = false;
}

// toggle framebuffer
void lastfght_state::dest_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_dest ^= 1;
}

uint8_t lastfght_state::c00000_r()
{
	// bit 7 = blitter busy
	// bit 6 = blitter?
	return 0x40 | m_blitter_busy << 7;
}

uint8_t lastfght_state::c00002_r()
{
	// mask 0x1c: from sound?
	return (machine().rand() & 0x1c) | 0x03;
}

void lastfght_state::c00007_w(uint8_t data)
{
	m_eeprom->data_w(!BIT(data, 6));
}

uint16_t lastfght_state::sound_r()
{
	// low byte:
	// bit 3
	return 8;
}

void lastfght_state::sound_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_8_15)
		logerror("%06x: sound_w msb = %02x\n", m_maincpu->pc(), data >> 8);
	if (ACCESSING_BITS_0_7)
		logerror("%06x: sound_w lsb = %02x\n", m_maincpu->pc(), data);
}

/***************************************************************************
                                Memory Maps
***************************************************************************/

void lastfght_state::lastfght_map(address_map &map)
{
	map.global_mask(0xffffff);

	map(0x000000, 0x007fff).rom();
	map(0x080000, 0x0fffff).rom();

	map(0x200000, 0x20ffff).ram().share("nvram"); // battery

	map(0x600000, 0x600001).w(FUNC(lastfght_state::hi_w));
	map(0x600002, 0x600003).rw(FUNC(lastfght_state::sound_r), FUNC(lastfght_state::sound_w));
	map(0x600006, 0x600007).w(FUNC(lastfght_state::blit_w));
	map(0x600009, 0x600009).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x600008, 0x600008).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x60000a, 0x60000a).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x800000, 0x800001).w(FUNC(lastfght_state::sx_w));
	map(0x800002, 0x800003).w(FUNC(lastfght_state::sd_w));
	map(0x800004, 0x800005).w(FUNC(lastfght_state::sy_w));
	map(0x800006, 0x800007).w(FUNC(lastfght_state::sr_w));
	map(0x800008, 0x800009).w(FUNC(lastfght_state::x_w));
	map(0x80000a, 0x80000b).w(FUNC(lastfght_state::yw_w));
	map(0x80000c, 0x80000d).w(FUNC(lastfght_state::h_w));

	map(0x800014, 0x800015).w(FUNC(lastfght_state::dest_w));

	map(0xc00000, 0xc0001f).rw("io", FUNC(ss9802_device::read), FUNC(ss9802_device::write));
}

void lastfght_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( lastfght )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1       )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER          ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN        )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1          )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2          )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1        )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2        )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1         )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1        ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2        ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2         )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN        )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2)

	PORT_START("PROT")
	PORT_BIT( 0x005f, IP_ACTIVE_HIGH, IPT_UNUSED        ) // outputs
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN        ) // blitter?
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_CUSTOM        ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", ds2430a_device, data_r)
INPUT_PORTS_END


/***************************************************************************
                                Machine Drivers
***************************************************************************/

void lastfght_state::machine_start()
{
	save_item(NAME(m_dest));
	save_item(NAME(m_hi));
	save_item(NAME(m_sx));
	save_item(NAME(m_sx1));
	save_item(NAME(m_dsx));
	save_item(NAME(m_sy));
	save_item(NAME(m_sy1));
	save_item(NAME(m_dsy));
	save_item(NAME(m_sp));
	save_item(NAME(m_sr));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_w));
	save_item(NAME(m_h));
}

void lastfght_state::machine_reset()
{
	m_dest = 0;
	m_hi = 0;
	m_sx = 0;
	m_sx1 = 0;
	m_dsx = 0;
	m_sy = 0;
	m_sy1 = 0;
	m_dsy = 0;
	m_sp = 0;
	m_sr = 0;
	m_x = 0;
	m_y = 0;
	m_w = 0;
	m_h = 0;
}

void lastfght_state::lastfght(machine_config &config)
{
	/* basic machine hardware */
	H83044(config, m_maincpu, 32000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &lastfght_state::lastfght_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	ss9802_device &io(SS9802(config, "io"));
	io.in_port_callback<0>().set(FUNC(lastfght_state::c00000_r));
	io.in_port_callback<2>().set(FUNC(lastfght_state::c00002_r));
	io.in_port_callback<3>().set_ioport("IN0");
	io.in_port_callback<4>().set_ioport("IN1");
	io.in_port_callback<5>().set_ioport("IN2");
	io.in_port_callback<6>().set_ioport("IN3");
	io.in_port_callback<7>().set_ioport("PROT");
	io.out_port_callback<7>().set(FUNC(lastfght_state::c00007_w));

	DS2430A(config, m_eeprom).set_timing_scale(0.32);

	/* video hardware */
	PALETTE(config, m_palette).set_entries(256);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette)); // HMC HM86171 VGA 256 colour RAMDAC
	ramdac.set_addrmap(0, &lastfght_state::ramdac_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 512-1, 0, 256-16-1);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(lastfght_state::screen_update));
	m_screen->set_palette(m_palette);
	m_screen->screen_vblank().set_inputline(m_maincpu, 0);
}


/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( lastfght )
	ROM_REGION( 0x100000, "maincpu", 0 ) // H8/3044 program
	ROM_LOAD( "ss9689_6433044a22f.u12", 0x000000, 0x008000, CRC(ece09075) SHA1(a8bc3aa44f30a6f919f4151c6093fb52e5da2f40) )
	ROM_LOAD( "v106.u16",               0x080000, 0x080000, CRC(7aec89f4) SHA1(7cff00844ad82a0f8d19b1bd07ba3a2bced69d66) )

	ROM_REGION( 0x800000, "gfx1", 0 ) // Blitter data
	ROM_LOAD( "1.b1", 0x000000, 0x200000, CRC(6c438136) SHA1(138934e948bbd6bd80f354f037badedef6cd8cb1) )
	ROM_LOAD( "2.b2", 0x200000, 0x200000, CRC(9710bcff) SHA1(0291385489a065ed895c99ae7197fdeac0a0e2a0) )
	ROM_LOAD( "3.b3", 0x400000, 0x200000, CRC(4236c79a) SHA1(94f093d12c096d38d1e7278796f6d58e4ba14e2e) )
	ROM_LOAD( "4.b4", 0x600000, 0x200000, CRC(68153b0f) SHA1(46ddf37d5885f411e0e6de9c7e8969ba3a00f17f) )

	ROM_REGION( 0x100000, "samples", 0 ) // Samples
	ROM_LOAD( "v100.u7", 0x000000, 0x100000, CRC(c134378c) SHA1(999c75f3a7890421cfd904a926ca377ee43a6825) )

	ROM_REGION( 0x28, "eeprom", 0 )
	ROM_LOAD( "ds2430a.q3", 0x00, 0x28, CRC(af461d83) SHA1(bb8d25e9bb60e00e460e4b7e1855c735becaaa6d) )
ROM_END

} // anonymous namespace


GAME( 2000, lastfght, 0, lastfght, lastfght, lastfght_state, empty_init, ROT0, "Subsino", "Last Fighting", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_TIMING | MACHINE_SUPPORTS_SAVE )

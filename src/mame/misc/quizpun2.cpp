// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

Quiz Punch II (C)1989 Space Computer

Driver by Luca Elia

- It uses a COP MCU for protection, that supplies
  the address to jump to (same as mosaic.cpp) and handles the EEPROM

PCB Layout
----------

|---------------------------------------------|
|U1    U26        6116                   32MHz|
|                             U120 U119       |
|U2                                  U118 U117|
|      U27        6116                        |
|                                             |
|                                             |
|      U28        6116        Z80             |
|                         U2A     93C46   U111|
|                                      6264   |
|      U29                    COP         8MHz|
|  U20            6116                        |
|                                6116         |
|  U21 U30                                    |
|                 6116                        |
|  U22  6116                           DSW1(8)|
|VOL    YM2203 Z80                            |
|    YM3014     MAHJONG28                     |
|---------------------------------------------|
Notes:
      COP - DIP40 identified as COP402 MCU (+5V on pin 17, GND on pin 22).
          pins 4,3,2 of 93C46 tied to COP pins 23,24,25
      All clocks unknown, PCB not working
      Possibly Z80's @ 4MHz and YM2203 @ 2MHz
      PCB marked 'Ducksan Trading Co. Ltd. Made In Korea'

***************************************************************************/
/***************************************************************************

Quiz Punch (C)1988 Space Computer
Ducksan 1989

88-01-14-0775
|--------------------------------------------------|
|VOL  MC1455                        02.U2   01.U1  |
|UPC1241                                           |
|LM358  05.U22 04.U21  03.U20                      |
|YM3014B  6116   10.U30 09.U29 08.U28 07.U27 06.U26|
|YM2203  Z80A                                      |
|                                                  |
|J              6116   6116   6116   6116   6116   |
|A                                                 |
|M                                                 |
|M                       |------------|            |
|A                       |            |            |
|                        |  EPOXY     |            |
|                        |  MODULE    |            |
|                        |            |            |
|    6116       6116     |            |            |
|                        |------------|    14.U120 |
|                               GM76C88    13.U119 |
|                                          12.U118 |
|                                          11.U117 |
|   DSW(8)                   8MHz 15.U111     32MHz|
|--------------------------------------------------|
Notes:
       Z80A - clock 4.000MHz (8/2)
     YM2203 - clock 4.000MHz (8/2)
     VSync - 59.3148Hz
     HSync - 15.2526kHz

     Epoxy Module contains:
      Z80B (input clock 4.000MHz)
      68705P5 MCU
      HY 93C46 EEPROM (upside down)
      4 logic chips

***************************************************************************/

#include "emu.h"

#include "cpu/cop400/cop400.h"
#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"

#include "machine/eepromser.h"
#include "machine/gen_latch.h"

#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

// configurable logging
#define LOG_SCROLL     (1U << 1)
#define LOG_MCU        (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_SCROLL | LOG_MCU)

#include "logmacro.h"

#define LOGSCROLL(...)     LOGMASKED(LOG_SCROLL,     __VA_ARGS__)
#define LOGMCU(...) LOGMASKED(LOG_MCU, __VA_ARGS__)


namespace {

class quizpun2_state : public driver_device
{
public:
	quizpun2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_eeprom(*this, "eeprom"),
		m_fg_ram(*this, "fg_ram"),
		m_bg_ram(*this, "bg_ram"),
		m_mainbank(*this, "mainbank")
	{ }

	void quizpun2_base(machine_config &config);
	void quizpun(machine_config &config);
	void quizpun2(machine_config &config);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_shared_ptr<uint8_t> m_fg_ram;
	required_shared_ptr<uint8_t> m_bg_ram;
	required_memory_bank m_mainbank;

	tilemap_t *m_bg_tmap;
	tilemap_t *m_fg_tmap;
	uint8_t m_scroll;

	uint8_t m_mcu_data_port;
	uint8_t m_mcu_control_port;
	bool m_mcu_pending;
	bool m_mcu_written;
	bool m_mcu_repeat;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void bg_ram_w(offs_t offset, uint8_t data);
	void fg_ram_w(offs_t offset, uint8_t data);
	void scroll_w(uint8_t data);
	void rombank_w(uint8_t data);
	void irq_ack(uint8_t data);
	void soundlatch_w(uint8_t data);
	uint8_t protection_r();
	void protection_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// quizpun2
	void cop_d_w(uint8_t data);
	void cop_g_w(uint8_t data);
	uint8_t cop_l_r();
	void cop_l_w(uint8_t data);
	uint8_t cop_in_r();

	// quizpun
	uint8_t quizpun_68705_port_a_r();
	void quizpun_68705_port_a_w(uint8_t data);

	uint8_t quizpun_68705_port_b_r();
	void quizpun_68705_port_b_w(uint8_t data);

	uint8_t quizpun_68705_port_c_r();
	void quizpun_68705_port_c_w(uint8_t data);

	void quizpun2_cop_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

/***************************************************************************
                                Video Hardware
***************************************************************************/

TILE_GET_INFO_MEMBER(quizpun2_state::get_bg_tile_info)
{
	uint16_t code = m_bg_ram[tile_index * 2] + m_bg_ram[tile_index * 2 + 1] * 256;
	tileinfo.set(0, code, code >> 12, TILE_FLIPXY((code & 0x800) >> 11));
}

TILE_GET_INFO_MEMBER(quizpun2_state::get_fg_tile_info)
{
	uint16_t code  = m_fg_ram[tile_index * 4]; // + m_fg_ram[tile_index * 4 + 1] * 256
	uint8_t  color = m_fg_ram[tile_index * 4 + 2];
	tileinfo.set(1, code, color / 2, 0);
}

void quizpun2_state::bg_ram_w(offs_t offset, uint8_t data)
{
	m_bg_ram[offset] = data;
	m_bg_tmap->mark_tile_dirty(offset / 2);
}

void quizpun2_state::fg_ram_w(offs_t offset, uint8_t data)
{
	m_fg_ram[offset] = data;
	m_fg_tmap->mark_tile_dirty(offset / 4);
}

void quizpun2_state::scroll_w(uint8_t data)
{
	m_scroll = data;
}

void quizpun2_state::video_start()
{
	m_bg_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(quizpun2_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x40);
	m_fg_tmap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(quizpun2_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 0x20, 0x40);

	m_bg_tmap->set_transparent_pen(0);
	m_fg_tmap->set_transparent_pen(0);

	save_item(NAME(m_scroll));
}

uint32_t quizpun2_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int layers_ctrl = -1;

#ifdef MAME_DEBUG
	if (machine().input().code_pressed(KEYCODE_Z))
	{
		int msk = 0;
		if (machine().input().code_pressed(KEYCODE_Q))  msk |= 1;
		if (machine().input().code_pressed(KEYCODE_W))  msk |= 2;
		if (msk != 0) layers_ctrl &= msk;
	}
#endif

	int bg_scroll = (m_scroll & 0x3) >> 0;
	int fg_scroll = (m_scroll & 0xc) >> 2;

	m_bg_tmap->set_scrolly(0, bg_scroll * 0x100);
	m_fg_tmap->set_scrolly(0, fg_scroll * 0x100);

	if (layers_ctrl & 1)    m_bg_tmap->draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	else                    bitmap.fill(m_palette->black_pen(), cliprect);

	if (layers_ctrl & 2)    m_fg_tmap->draw(screen, bitmap, cliprect, 0, 0);

	LOGSCROLL("BG: %x FG: %x", bg_scroll, fg_scroll);

	return 0;
}

/***************************************************************************
                         Quizpun2 Protection Simulation

    ROM checksum:   write 0x80 | (0x00-0x7f), write 0, read 2 bytes
    Read address:   write 0x80 | param1 & 0x07f (0x00), write param2 & 0x7f, read 2 bytes
    Read EEPROM:    write 0x20 | (0x00-0x0f), write 0, read 8 bytes
    Write EEPROM:   write 0x00 | (0x00-0x0f), write 0, write 8 bytes

***************************************************************************/

void quizpun2_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();
	m_mainbank->configure_entries(0, 0x20, &ROM[0x10000], 0x2000);

	save_item(NAME(m_mcu_data_port));
	save_item(NAME(m_mcu_control_port));
	save_item(NAME(m_mcu_pending));
	save_item(NAME(m_mcu_repeat));
	save_item(NAME(m_mcu_written));
}

void quizpun2_state::machine_reset()
{
	m_mainbank->set_entry(0);

	// quizpun2 service mode needs this to fix a race condition since the MCU takes a while to start up
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);

	m_mcu_data_port = m_mcu_control_port = 0;
	m_mcu_pending = m_mcu_written = m_mcu_repeat = false;
}

void quizpun2_state::cop_d_w(uint8_t data)
{
	m_eeprom->cs_write(BIT(data, 0));
	// bit 1 used to control second EEPROM???
}

void quizpun2_state::cop_g_w(uint8_t data)
{
	if (BIT(m_mcu_control_port, 0) && !BIT(data, 0))
	{
		m_mcu_pending = false;
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}

	m_mcu_control_port = data;
}

uint8_t quizpun2_state::cop_l_r()
{
	return m_mcu_data_port;
}

void quizpun2_state::cop_l_w(uint8_t data)
{
	if (m_mcu_repeat)
		m_mcu_data_port = data;
}

uint8_t quizpun2_state::cop_in_r()
{
	return 8 | (m_mcu_written ? 4 : 2) | (m_mcu_pending ? 0 : 1);
}

/***************************************************************************
                            Memory Maps - Main CPU
***************************************************************************/

void quizpun2_state::rombank_w(uint8_t data)
{
	m_mainbank->set_entry(data & 0x1f);
}

void quizpun2_state::irq_ack(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
}

void quizpun2_state::soundlatch_w(uint8_t data)
{
	m_soundlatch->write(data ^ 0x80);
	m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void quizpun2_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).bankr(m_mainbank);

	map(0xa000, 0xbfff).ram().w(FUNC(quizpun2_state::fg_ram_w)).share(m_fg_ram);
	map(0xc000, 0xcfff).ram().w(FUNC(quizpun2_state::bg_ram_w)).share(m_bg_ram);

	map(0xd000, 0xd3ff).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
	map(0xe000, 0xffff).ram();
}

void quizpun2_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).w(FUNC(quizpun2_state::irq_ack));
	map(0x50, 0x50).w(FUNC(quizpun2_state::soundlatch_w));
	map(0x60, 0x60).w(FUNC(quizpun2_state::rombank_w));
	map(0x70, 0x70).w(FUNC(quizpun2_state::scroll_w));
	map(0x80, 0x80).portr("DSW");
	map(0x90, 0x90).portr("IN0");
	map(0xa0, 0xa0).portr("IN1");
	map(0xe0, 0xe0).rw(FUNC(quizpun2_state::protection_r), FUNC(quizpun2_state::protection_w));
}

void quizpun2_state::quizpun2_cop_map(address_map &map)
{
	map(0x000, 0x3ff).rom().region("cop", 0);
}

// quizpun

uint8_t quizpun2_state::protection_r()
{
	LOGMCU("%s: port A read %02x\n", machine().describe_context(), m_mcu_data_port);

	/*
	   Upon reading this port the main CPU is stalled until the MCU provides the value to read
	   and explicitly un-stalls the z80. Is this possible under the current MAME architecture?

	   ** ghastly hack **

	   The first read stalls the main CPU and triggers the MCU, it returns an incorrect value.
	   It also decrements the main CPU PC back to the start of the read instruction.
	   When the MCU un-stalls the Z80, the read happens again, returning the correct MCU-provided value this time
	*/
	if (m_mcu_repeat)
	{
		m_mcu_repeat = false;
	}
	else
	{
		m_mcu_pending = true;
		m_mcu_written = false;
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		m_maincpu->yield();

		m_maincpu->set_state_int(Z80_PC, m_maincpu->state_int(Z80_PC) - 2);
		m_mcu_repeat = true;
	}

	return m_mcu_data_port;
}

void quizpun2_state::protection_w(uint8_t data)
{
	LOGMCU("%s: port A write %02x\n", machine().describe_context(), data);
	m_mcu_data_port = data;
	m_mcu_pending = true;
	m_mcu_written = true;
	m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_maincpu->yield();
}

/***************************************************************************
                            Memory Maps - MCU
***************************************************************************/

// Port A - I/O with main CPU (data)

uint8_t quizpun2_state::quizpun_68705_port_a_r()
{
	LOGMCU("%s: port A read %02x\n", machine().describe_context(), m_mcu_data_port);
	return m_mcu_data_port;
}

void quizpun2_state::quizpun_68705_port_a_w(uint8_t data)
{
	LOGMCU("%s: port A write %02x\n", machine().describe_context(), data);
	m_mcu_data_port = data;
}

// Port B - I/O with main CPU (status)

uint8_t quizpun2_state::quizpun_68705_port_b_r()
{
	// bit 3: 0 = pending
	// bit 1: 0 = main CPU has written
	// bit 0: 0 = main CPU is reading

	uint8_t const ret =
			0xf4 |
			( m_mcu_pending                        ? 0 : (1 << 3)) |
			((m_mcu_pending &&  m_mcu_written) ? 0 : (1 << 1)) |
			((m_mcu_pending && !m_mcu_written) ? 0 : (1 << 0));

	LOGMCU("%s: port B read %02x\n", machine().describe_context(), ret);
	return ret;
}

void quizpun2_state::quizpun_68705_port_b_w(uint8_t data)
{
	LOGMCU("%s: port B write %02x\n", machine().describe_context(), data);

	// bit 2: 0->1 run main CPU

	if (!BIT(m_mcu_control_port, 2) && BIT(data, 2))
	{
		m_mcu_pending = false;
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}
	m_mcu_control_port = data;
}

// Port C - EEPROM

uint8_t quizpun2_state::quizpun_68705_port_c_r()
{
	uint8_t const ret = 0xf7 | (m_eeprom->do_read() ? 0x08 : 0x00);
	LOGMCU("%s: port C read %02x\n", machine().describe_context(), ret);
	return ret;
}

void quizpun2_state::quizpun_68705_port_c_w(uint8_t data)
{
	// latch the bit
	m_eeprom->di_write(BIT(data, 2));

	// reset line asserted: reset.
	m_eeprom->cs_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);

	// clock line asserted: write latch or select next bit to read
	m_eeprom->clk_write(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);

	LOGMCU("%s: port C write %02x\n", machine().describe_context(), data);
}

/***************************************************************************
                            Memory Maps - Sound CPU
***************************************************************************/

void quizpun2_state::sound_map(address_map &map)
{
	map(0x0000, 0xf7ff).rom();
	map(0xf800, 0xffff).ram();
}

void quizpun2_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).nopw();  // IRQ end
	map(0x20, 0x20).nopw();  // NMI end
	map(0x40, 0x40).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x60, 0x61).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
}

/***************************************************************************
                                Input Ports
***************************************************************************/

static INPUT_PORTS_START( quizpun2 )
	PORT_START("DSW")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x30, "Play Time" )
	PORT_DIPSETTING(    0x30, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x10, "8" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x00, "5" )

	PORT_START("IN0") // port $90
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("IN1") // port $a0
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1   )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2  )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

/***************************************************************************
                                Graphics Layout
***************************************************************************/

static const gfx_layout layout_16x16x1 =
{
	16, 16,
	RGN_FRAC(1, 1),
	1,
	{ 0 },
	{ STEP8(7*1,-1),STEP8(15*1,-1) },
	{ STEP16(0,16*1) },
	16*16*1
};

static GFXDECODE_START( gfx_quizpun2 )
	GFXDECODE_ENTRY( "bg",  0, gfx_16x16x4_packed_lsb,     0, 256/16 )
	GFXDECODE_ENTRY( "fg",  0, layout_16x16x1        , 0x100, 256/2  )
	GFXDECODE_ENTRY( "fg2", 0, layout_16x16x1,         0x100, 256/2  )
GFXDECODE_END

/***************************************************************************
                                Machine Drivers
***************************************************************************/

void quizpun2_state::quizpun2_base(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(8'000'000) / 2); // 4 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &quizpun2_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &quizpun2_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(quizpun2_state::irq0_line_hold));

	Z80(config, m_audiocpu, XTAL(8'000'000) / 2); // 4 MHz
	m_audiocpu->set_addrmap(AS_PROGRAM, &quizpun2_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &quizpun2_state::sound_io_map);
	m_audiocpu->set_vblank_int("screen", FUNC(quizpun2_state::irq0_line_hold));
	// NMI generated by main CPU

	EEPROM_93C46_16BIT(config, "eeprom");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(384, 256);
	screen.set_visarea(0, 384-1, 0, 256-1);
	screen.set_screen_update(FUNC(quizpun2_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_quizpun2);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 0x200);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ym2203_device &ymsnd(YM2203(config, "ymsnd", XTAL(8'000'000) / 2)); // 4 MHz
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void quizpun2_state::quizpun2(machine_config &config)
{
	quizpun2_base(config);

	cop402_cpu_device &cop(COP402(config, "cop", XTAL(8'000'000) / 2));
	cop.set_addrmap(AS_PROGRAM, &quizpun2_state::quizpun2_cop_map);
	cop.set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false);
	cop.write_d().set(FUNC(quizpun2_state::cop_d_w));
	cop.write_g().set(FUNC(quizpun2_state::cop_g_w));
	cop.read_l().set(FUNC(quizpun2_state::cop_l_r));
	cop.write_l().set(FUNC(quizpun2_state::cop_l_w));
	cop.read_in().set(FUNC(quizpun2_state::cop_in_r));
	cop.read_si().set("eeprom", FUNC(eeprom_serial_93cxx_device::do_read));
	cop.write_so().set("eeprom", FUNC(eeprom_serial_93cxx_device::di_write));
	cop.write_sk().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write));
}

void quizpun2_state::quizpun(machine_config &config)
{
	quizpun2_base(config);

	m68705p5_device &mcu(M68705P5(config, "mcu", XTAL(4'000'000))); // xtal is 4MHz, divided by 4 internally
	mcu.porta_r().set(FUNC(quizpun2_state::quizpun_68705_port_a_r));
	mcu.portb_r().set(FUNC(quizpun2_state::quizpun_68705_port_b_r));
	mcu.portc_r().set(FUNC(quizpun2_state::quizpun_68705_port_c_r));
	mcu.porta_w().set(FUNC(quizpun2_state::quizpun_68705_port_a_w));
	mcu.portb_w().set(FUNC(quizpun2_state::quizpun_68705_port_b_w));
	mcu.portc_w().set(FUNC(quizpun2_state::quizpun_68705_port_c_w));
}

/***************************************************************************
                                ROMs Loading
***************************************************************************/

ROM_START( quizpun2 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "u111", 0x00000, 0x08000, CRC(14bdaffc) SHA1(7fb5988ea565d7cbe3c8e2cdb9402d3cf81507d7) )
	ROM_LOAD( "u117", 0x10000, 0x10000, CRC(e9d1d05e) SHA1(c24104e023d12db8c9199d3e18750414aa511e40) )
	ROM_LOAD( "u118", 0x20000, 0x10000, CRC(1f232707) SHA1(3f5f44611f25c556521333f15daf3e2128cc1538) BAD_DUMP ) // fails ROM check
	ROM_LOAD( "u119", 0x30000, 0x10000, CRC(c73b82f7) SHA1(d5c683440e9db46dd5859b519b3f32da80352626) )
	ROM_LOAD( "u120", 0x40000, 0x10000, CRC(700648b8) SHA1(dfa824166dfe7361d7c2ab0d8aa1ada882916cb9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "u22", 0x00000, 0x10000, CRC(f40768b5) SHA1(4410f71850357ec1d10a3a114bb540966e72781b) )

	ROM_REGION( 0x2000, "cop", 0 )
	ROM_LOAD( "u2a", 0x0000, 0x2000, CRC(13afc2bd) SHA1(0d9c8813525dfc7a844e72d2cf84261db3d10a23) ) // 111xxxxxxxxxx = 0xFF

	ROM_REGION( 0x40000, "bg", 0 )    // 16x16x8
	ROM_LOAD( "u21", 0x00000, 0x10000, CRC(8ac86759) SHA1(2eac9ceee4462ce905aa08ff4f5a6215e0b6672f) )
	ROM_LOAD( "u20", 0x10000, 0x10000, CRC(67640a46) SHA1(5b33850afbb89db9ce9044a578423bfe3a55420d) )
	ROM_LOAD( "u29", 0x20000, 0x10000, CRC(cd8ff05b) SHA1(25e5be914fe49ff96a3c04de0c0e266a79068930) )
	ROM_LOAD( "u30", 0x30000, 0x10000, CRC(8612b443) SHA1(1033a378b21023eca471f43309d49461494b5ea1) )

	ROM_REGION( 0x6000, "fg", 0 ) // 16x16x1
	ROM_LOAD( "u26", 0x1000, 0x1000, CRC(151de8af) SHA1(2159ab030043e69d63cc9fbbc772f5bae8ab3f9d) )
	ROM_CONTINUE(    0x0000, 0x1000 )
	ROM_LOAD( "u27", 0x3000, 0x1000, CRC(2afdafea) SHA1(4c116a1e8a91f2e309646063139763b837e24bc7) )
	ROM_CONTINUE(    0x2000, 0x1000 )
	ROM_LOAD( "u28", 0x5000, 0x1000, CRC(c8bd85ad) SHA1(e7f0882f669edea1bb4634c263872f63da6a3290) ) // 1ST HALF = xx00
	ROM_CONTINUE(    0x4000, 0x1000 )

	ROM_REGION( 0x20000, "fg2", 0 )    // 16x16x1
	ROM_LOAD( "u1", 0x00000, 0x10000, CRC(58506040) SHA1(9d8bed2585e8f188a20270fccd9cfbdb91e48599) )
	ROM_LOAD( "u2", 0x10000, 0x10000, CRC(9294a19c) SHA1(cd7109262e5f68b946c84aa390108bcc47ee1300) )
ROM_END

ROM_START( quizpun )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "15.u111", 0x00000, 0x08000, CRC(0ffe42d9) SHA1(f91e499800923d185a5d3514fc4c50e5c86378bf) )
	ROM_LOAD( "11.u117", 0x10000, 0x10000, CRC(13541476) SHA1(5e81e4143fbc8fa68c2c7d54792a432e97964d7f) )
	ROM_LOAD( "12.u118", 0x20000, 0x10000, CRC(678b57c1) SHA1(83869e5b6fe528c0b072f7d97338febc31db9f8b) )
	ROM_LOAD( "13.u119", 0x30000, 0x10000, CRC(9c0ee0de) SHA1(14b148f3ca951a5a9010b4d253e3ba7d35708403) )
	ROM_LOAD( "14.u120", 0x40000, 0x10000, CRC(21c11262) SHA1(e50678fafdf775a49ef96f8837b124824a2d1ca2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "05.u22", 0x00000, 0x10000, CRC(515f337e) SHA1(21b2cca95b5da934fd8139892c2ee2c623d51a4e) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "68705p5.bin", 0x000, 0x800, CRC(2e52bc67) SHA1(13ad4aee88c53c75c7cc1f31a149ba0234447f42) ) // in epoxy block

	ROM_REGION( 0x40000, "bg", 0 )    // 16x16x8
	ROM_LOAD( "04.u21", 0x00000, 0x10000, CRC(fa8d64f4) SHA1(71badabf8f34f246dec83323a1cddbe74deb91bd) )
	ROM_LOAD( "03.u20", 0x10000, 0x10000, CRC(8dda8167) SHA1(42838cf6866fb1d59c5bb3b477053aac448e7760) )
	ROM_LOAD( "09.u29", 0x20000, 0x10000, CRC(b9f28569) SHA1(1395cd226d314ee57385eed25f28b68607bfda53) )
	ROM_LOAD( "10.u30", 0x30000, 0x10000, CRC(db5762c0) SHA1(606dc4a3e6b8034f063f11dcf0a2b1db59838f4c) )

	ROM_REGION( 0xc000, "fg", 0 ) // 16x16x1
	ROM_LOAD( "06.u26", 0x1000, 0x1000, CRC(6d071b6d) SHA1(19565c8d768eeecd4119677915cc06f3ea18a47a) )
	ROM_CONTINUE(       0x0000, 0x1000 )
	ROM_LOAD( "07.u27", 0x3000, 0x1000, CRC(0f8b516e) SHA1(8bfabfd0bd28a1c7ddd01586fe9757b241feb59b) ) // FIXED BITS (xxxxxxx00xxxxxxx), BADADDR --xxxxxxxxxxxxx
	ROM_CONTINUE(       0x2000, 0x1000 )
	ROM_CONTINUE(       0x6000, 0x6000 ) // ??
	ROM_LOAD( "08.u28", 0x5000, 0x1000, CRC(51c0c5cb) SHA1(0c7bfc9b6b3ce0cdd5c0e36df2b4d90f9cff7fae) ) // FIXED BITS (0xxxxxx000000000), 111xxxxxxxxx1 = 0x00
	ROM_CONTINUE(       0x4000, 0x1000 )

	ROM_REGION( 0x20000, "fg2", 0 )    // 16x16x1
	ROM_LOAD( "01.u1", 0x00000, 0x10000, CRC(58506040) SHA1(9d8bed2585e8f188a20270fccd9cfbdb91e48599) )
	ROM_LOAD( "02.u2", 0x10000, 0x10000, CRC(9294a19c) SHA1(cd7109262e5f68b946c84aa390108bcc47ee1300) )
ROM_END

} // anonymous namespace


GAME( 1988, quizpun,  0, quizpun,  quizpun2, quizpun2_state, empty_init, ROT270, "Space Computer", "Quiz Punch",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1989, quizpun2, 0, quizpun2, quizpun2, quizpun2_state, empty_init, ROT270, "Space Computer", "Quiz Punch II", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

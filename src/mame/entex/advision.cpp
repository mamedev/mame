// license:BSD-3-Clause
// copyright-holders:Dan Boris
/*************************************************************************

Driver for the Entex Adventure Vision

TODO:
- convert to discrete sound
- screen pincushion distortion

**************************************************************************/

#include "emu.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/cop400/cop400.h"
#include "cpu/mcs48/mcs48.h"
#include "sound/dac.h"
#include "emupal.h"

namespace {

#define I8048_TAG   "i8048"
#define COP411_TAG  "cop411"

class advision_state : public driver_device
{
public:
	advision_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, I8048_TAG)
		, m_soundcpu(*this, COP411_TAG)
		, m_dac(*this, "dac")
		, m_cart(*this, "cartslot")
		, m_bank1(*this, "bank1")
		, m_joy(*this, "joystick")
		, m_palette(*this, "palette")
	{ }

	required_device<i8048_device> m_maincpu;
	required_device<cop411_cpu_device> m_soundcpu;
	required_device<dac_byte_interface> m_dac;
	required_device<generic_slot_device> m_cart;
	required_memory_bank m_bank1;
	required_ioport m_joy;
	required_device<palette_device> m_palette;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void update_dac();
	void vh_write(int data);
	void vh_update(int x);

	uint8_t rom_r(offs_t offset);
	uint8_t ext_ram_r(offs_t offset);
	void ext_ram_w(offs_t offset, uint8_t data);
	uint8_t controller_r();
	void bankswitch_w(uint8_t data);
	void av_control_w(uint8_t data);
	DECLARE_READ_LINE_MEMBER( vsync_r );

	TIMER_CALLBACK_MEMBER( sound_cmd_sync );
	uint8_t sound_cmd_r();
	void sound_g_w(uint8_t data);
	void sound_d_w(uint8_t data);

	memory_region *m_cart_rom = nullptr;

	int m_ea_bank = 0;

	/* external RAM state */
	std::vector<uint8_t> m_ext_ram;
	int m_rambank = 0;

	/* video state */
	int m_frame_count = 0;
	int m_frame_start = 0;
	int m_video_enable = 0;
	int m_video_bank = 0;
	int m_video_hpos = 0;
	uint8_t m_led_latch[8] = {};
	std::unique_ptr<uint8_t []> m_display;

	/* sound state */
	int m_sound_cmd = 0;
	int m_sound_d = 0;
	int m_sound_g = 0;
	void advision_palette(palette_device &palette) const;
	void advision(machine_config &config);
	void io_map(address_map &map);
	void program_map(address_map &map);
};

/* Memory Maps */

uint8_t advision_state::rom_r(offs_t offset)
{
	offset += 0x400;
	return m_cart->read_rom(offset & 0xfff);
}

void advision_state::program_map(address_map &map)
{
	map(0x0000, 0x03ff).bankr("bank1");
	map(0x0400, 0x0fff).r(FUNC(advision_state::rom_r));
}

void advision_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(advision_state::ext_ram_r), FUNC(advision_state::ext_ram_w));
}

/* Input Ports */

static INPUT_PORTS_START( advision )
	PORT_START("joystick")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON4 )       PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 )       PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )       PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON1 )       PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_8WAY
INPUT_PORTS_END

/*
    8048 Ports:

    P1  Bit 0..1  - RAM bank select
        Bit 3..7  - Keypad input

    P2  Bit 0..3  - A8-A11
        Bit 4..7  - Sound control/Video write address

    T1  Mirror sync pulse
*/

/* Machine Initialization */

void advision_state::machine_start()
{
	std::string region_tag;
	m_cart_rom = memregion(region_tag.assign(m_cart->tag()).append(GENERIC_ROM_REGION_TAG).c_str());

	/* configure EA banking */
	m_bank1->configure_entry(0, memregion(I8048_TAG)->base());
	if (m_cart_rom)
		m_bank1->configure_entry(1, m_cart_rom->base());
	m_maincpu->space(AS_PROGRAM).install_readwrite_bank(0x0000, 0x03ff, m_bank1);
	m_bank1->set_entry(0);

	m_sound_d = 0;
	m_sound_g = 0;

	m_video_hpos = 0;
	m_display = std::make_unique<uint8_t []>(8 * 8 * 256);
	std::fill_n(m_display.get(), 8 * 8 * 256, 0);

	/* allocate external RAM */
	m_ext_ram.resize(0x400);
	save_item(NAME(m_ext_ram));

	save_item(NAME(m_ea_bank));
	save_item(NAME(m_rambank));
	save_item(NAME(m_frame_count));
	save_item(NAME(m_frame_start));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_video_bank));
	save_item(NAME(m_led_latch));
	save_item(NAME(m_sound_cmd));
	save_item(NAME(m_sound_d));
	save_item(NAME(m_sound_g));
	save_pointer(NAME(m_display), 8 * 8 * 256);
	save_item(NAME(m_video_hpos));
}

void advision_state::machine_reset()
{
	m_ea_bank = 1;
	m_rambank = 0x300;
	m_frame_start = 0;
	m_video_enable = 0;
	m_sound_cmd = 0;

	/* enable internal ROM */
	m_maincpu->set_input_line(MCS48_INPUT_EA, CLEAR_LINE);
	if (m_cart_rom)
		m_bank1->set_entry(m_ea_bank);

	/* reset sound CPU */
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

/* Bank Switching */

void advision_state::bankswitch_w(uint8_t data)
{
	m_ea_bank = BIT(data, 2);
	m_rambank = (data & 0x03) << 8;

	m_maincpu->set_input_line(MCS48_INPUT_EA, m_ea_bank ? ASSERT_LINE : CLEAR_LINE);
	if (m_cart_rom)
		m_bank1->set_entry(m_ea_bank);
}

/* External RAM */

uint8_t advision_state::ext_ram_r(offs_t offset)
{
	uint8_t data = m_ext_ram[m_rambank + offset];

	/* the video hardware interprets reads as writes */
	vh_write(data);

	if (m_video_bank == 0x06)
	{
		m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
	}

	return data;
}

void advision_state::ext_ram_w(offs_t offset, uint8_t data)
{
	m_ext_ram[m_rambank + offset] = data;
}

/* Sound */

TIMER_CALLBACK_MEMBER( advision_state::sound_cmd_sync )
{
	m_sound_cmd = param;
}

uint8_t advision_state::sound_cmd_r()
{
	return m_sound_cmd;
}

void advision_state::update_dac()
{
	int translate[] = { 2, 0, 0, 1 };
	m_dac->write(translate[(m_sound_g << 1) | m_sound_d]);
}

void advision_state::sound_g_w(uint8_t data)
{
	m_sound_g = data & 0x01;

	update_dac();
}

void advision_state::sound_d_w(uint8_t data)
{
	m_sound_d = data & 0x01;

	update_dac();
}

/* Video */

void advision_state::av_control_w(uint8_t data)
{
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(advision_state::sound_cmd_sync), this), data >> 4);

	if ((m_video_enable == 0x00) && (data & 0x10))
	{
		vh_update(m_video_hpos);

		m_video_hpos++;

		if (m_video_hpos > 255)
		{
			m_video_hpos = 0;
			logerror("HPOS OVERFLOW\n");
		}
	}

	m_video_enable = data & 0x10;
	m_video_bank = (data & 0xe0) >> 5;
}

READ_LINE_MEMBER( advision_state::vsync_r )
{
	if (m_frame_start)
	{
		m_frame_start = 0;

		return 0;
	}
	else
	{
		return 1;
	}
}

/* Input */

uint8_t advision_state::controller_r()
{
	// Get joystick switches
	uint8_t in = m_joy->read();
	uint8_t data = in | 0x0f;

	// Get buttons
	if (in & 0x02) data = data & 0xf7; /* Button 3 */
	if (in & 0x08) data = data & 0xcf; /* Button 1 */
	if (in & 0x04) data = data & 0xaf; /* Button 2 */
	if (in & 0x01) data = data & 0x6f; /* Button 4 */

	data &= 0xf8;
	data |= (m_ea_bank << 2);
	data |= (m_rambank >> 8);
	return data;
}


/***************************************************************************

  Initialise the palette.

***************************************************************************/

void advision_state::advision_palette(palette_device &palette) const
{
	// 8 shades of RED
	for (int i = 0; i < 8; i++)
		m_palette->set_pen_color(i, pal3bit(i), 0x00, 0x00);
}

/***************************************************************************

  Update the display data.

***************************************************************************/

void advision_state::vh_write(int data)
{
	if (m_video_bank >= 1 && m_video_bank <=5)
		m_led_latch[m_video_bank] = data;
}

void advision_state::vh_update(int x)
{
	uint8_t *dst = &m_display[x];

	for (int y = 0; y < 8; y++)
	{
		uint8_t data = m_led_latch[7 - y];

		for (int i = 0; i < 8; i++)
		{
			if (!BIT(data, 7 - i))
				dst[i * 256] = 8;
		}

		m_led_latch[7 - y] = 0xff;
		dst += 8 * 256;
	}
}


/***************************************************************************

  Refresh the video screen

***************************************************************************/

uint32_t advision_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if ((m_frame_count++ % 4) == 0)
	{
		m_frame_start = 1;
		m_video_hpos = 0;
	}

	for (int x = 0; x < 150; x++)
	{
		uint8_t *led = &m_display[x];

		for (int y = 0; y < 128; y+=2)
		{
			if (*led > 0)
				bitmap.pix(30 + y, 85 + x) = --(*led);
			else
				bitmap.pix(30 + y, 85 + x) = 0;

			led += 256;
		}
	}
	return 0;
}

/* Machine Driver */

void advision_state::advision(machine_config &config)
{
	/* basic machine hardware */
	I8048(config, m_maincpu, XTAL(11'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &advision_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &advision_state::io_map);
	m_maincpu->p1_in_cb().set(FUNC(advision_state::controller_r));
	m_maincpu->p1_out_cb().set(FUNC(advision_state::bankswitch_w));
	m_maincpu->p2_out_cb().set(FUNC(advision_state::av_control_w));
	m_maincpu->t1_in_cb().set(FUNC(advision_state::vsync_r));

	COP411(config, m_soundcpu, 52631*4); // COP411L-KCN/N, R11=82k, C8=56pF
	m_soundcpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_RAM_POWER_SUPPLY, false);
	m_soundcpu->read_l().set(FUNC(advision_state::sound_cmd_r));
	m_soundcpu->write_g().set(FUNC(advision_state::sound_g_w));
	m_soundcpu->write_d().set(FUNC(advision_state::sound_d_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(4*15);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update(FUNC(advision_state::screen_update));
	screen.set_size(320, 200);
	screen.set_visarea(84, 235, 60, 142);
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(advision_state::advision_palette), 8);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_BINARY_WEIGHTED(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "advision_cart");

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("advision");
}

/* ROMs */

ROM_START( advision )
	ROM_REGION( 0x1000, I8048_TAG, ROMREGION_ERASE00 )
	ROM_LOAD( "b225__ins8048-11kdp_n.u5", 0x000, 0x400, CRC(279e33d1) SHA1(bf7b0663e9125c9bfb950232eab627d9dbda8460) ) // "<natsemi logo> /B225 \\ INS8048-11KDP/N"

	ROM_REGION( 0x200, COP411_TAG, 0 )
	ROM_LOAD( "b8223__cop411l-kcn_n.u8", 0x000, 0x200, CRC(81e95975) SHA1(8b6f8c30dd3e9d8e43f1ea20fba2361b383790eb) ) // "<natsemi logo> /B8223 \\ COP411L-KCN/N"
ROM_END

} // Anonymous namespace

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME            FLAGS */
CONS( 1982, advision, 0,      0,      advision, advision, advision_state, empty_init, "Entex", "Adventure Vision", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )

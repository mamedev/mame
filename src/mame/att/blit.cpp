// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    AT&T Blit -- monochrome, raster graphics (800x1024), serial terminal.

    https://code.9front.org/hg/plan9front/file/f4fa0b9d0397/sys/src/games/blit/mmap
        memory map

    https://pbs.twimg.com/media/C_uxVGgUwAAImk6.jpg:orig
        board photo

    Onboard hardware (common to all revisions) is
    - 24K ROM
    - 256K RAM (includes frame buffer)
    - 2 serial ports, each driven by 6850 ACIA

    To do:
    - keyboard
    - mouse
    - make 'mux' window system work -- "68ld: load protocol failed"
    - sound?
    - what do dip switches do?

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/ram.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


#define M68K_TAG "maincpu"
#define ACIA0_TAG "acia0"
#define ACIA1_TAG "acia1"
#define RS232_H_TAG "host"
#define RS232_K_TAG "keyboard"


#define LOG_PIA       (1U << 1)
#define LOG_DEBUG     (1U << 2)

//#define VERBOSE (LOG_DEBUG)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGPIA(...) LOGMASKED(LOG_PIA, __VA_ARGS__)
#define LOGDBG(...) LOGMASKED(LOG_DEBUG, __VA_ARGS__)


namespace {

class blit_state : public driver_device
{
public:
	blit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, M68K_TAG)
		, m_ram(*this, RAM_TAG)
		, m_acia0(*this, ACIA0_TAG)
		, m_acia1(*this, ACIA1_TAG)
		, m_screen(*this, "screen")
		, m_misccr(*this, "misccr")
		, m_p_ram(*this, "p_ram")
		, m_sysrom(*this, M68K_TAG)
	{ }

	void system_clock_write(int state);

	void start_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void vblank_ack(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void blit(machine_config &config);
	void blit_mem(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<acia6850_device> m_acia0;
	required_device<acia6850_device> m_acia1;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint16_t> m_misccr;
	required_shared_ptr<uint16_t> m_p_ram;
	required_region_ptr<uint16_t> m_sysrom;

	memory_passthrough_handler m_rom_shadow_tap;
	int m_videostart = 0;
};

void blit_state::blit_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x03ffff).ram().share("p_ram");
	map(0x040000, 0x045fff).rom().region(M68K_TAG, 0);
	// octal 0000, 0002 - 16-bit
//  map(0x060000, 0x060003).r(FUNC(blit_state::mouse_xy));
	// octal 0011, 0013 - 8-bit - host
	map(0x060008, 0x060009).rw(m_acia0, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0x06000a, 0x06000b).rw(m_acia0, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));
	// octal 0021 - 8-bit
//  map(0x060010, 0x060011).r(FUNC(blit_state::mouse_buttons)).mask(0x00ff);
	// octal 0025 - mirror
//  map(0x060014, 0x060015).r(FUNC(blit_state::mouse_buttons)).mask(0x00ff);
	// octal 0027 - ???
	// octal 0030 - 16-bit
	map(0x060018, 0x060019).w(FUNC(blit_state::start_write));
	// octal 0040 - 16-bit
	map(0x060020, 0x060029).ram().share("misccr");
	// octal 0050 - 16-bit
//  map(0x060028, 0x060029).w(FUNC(blit_state::init_write));
	// octal 0060, 0062 - 8-bit - keyboard
	map(0x060030, 0x060031).rw(m_acia1, FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w));
	map(0x060032, 0x060033).rw(m_acia1, FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w));
	// octal 0070 - 16-bit
	map(0x060038, 0x060039).w(FUNC(blit_state::vblank_ack));
}

static INPUT_PORTS_START(blit)
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( host_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
	DEVICE_INPUT_DEFAULTS( "FLOW_CONTROL", 0x01, 0x01 )
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START( kbd_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_19200 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_2 )
	DEVICE_INPUT_DEFAULTS( "FLOW_CONTROL", 0x01, 0x01 )
DEVICE_INPUT_DEFAULTS_END


void blit_state::start_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("START <- %04X (addr %06X)\n", data, data << 2);
	m_videostart = data << 1;
}

void blit_state::system_clock_write(int state)
{
	m_acia0->write_txc(state);
	m_acia0->write_rxc(state);
	//
	m_acia1->write_txc(state);
	m_acia1->write_rxc(state);
}

void blit_state::vblank_ack(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_maincpu->set_input_line(M68K_IRQ_1, CLEAR_LINE);
}


uint32_t blit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int fg, bg;

	if (BIT(*m_misccr, 0))
	{
		fg = 0; bg = 1;
	}
	else
	{
		fg = 1; bg = 0;
	}

	for (int y = 0; y < 1024; y++)
	{
		uint16_t *p = &bitmap.pix(y);

		for (int x = 0; x < 800 / 16; x += 1)
		{
			uint16_t gfx;

			gfx = m_p_ram[m_videostart + x + y * 50];

			for (int z = 15; z >= 0; z--)
			{
				*p++ = BIT(gfx, z) ? fg : bg;
			}
		}
	}
	return 0;
}


void blit_state::machine_start()
{
}

void blit_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x000000, 0x000007, m_sysrom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0x040000, 0x045fff,
			"rom_shadow_r",
			[this] (offs_t offset, u16 &data, u16 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall ram over the rom shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x000000, 0x000007, m_p_ram);
				}
			},
			&m_rom_shadow_tap);

	*m_misccr = 0;
}

void blit_state::blit(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(33'000'000) / 4); // maybe XTAL(32'760'000)
	m_maincpu->set_addrmap(AS_PROGRAM, &blit_state::blit_mem);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD, rgb_t::white());
	m_screen->set_refresh_hz(60);
	m_screen->set_size(800, 1024);
	m_screen->set_visarea(0, 800-1, 0, 1024-1);
	m_screen->set_screen_update(FUNC(blit_state::screen_update));
	m_screen->screen_vblank().set_inputline(M68K_TAG, M68K_IRQ_1);
	m_screen->set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	CLOCK(config, "system_clock", 19200 * 16).signal_handler().set(FUNC(blit_state::system_clock_write));

	ACIA6850(config, m_acia0, 0);
	m_acia0->txd_handler().set(RS232_H_TAG, FUNC(rs232_port_device::write_txd));
	m_acia0->rts_handler().set(RS232_H_TAG, FUNC(rs232_port_device::write_rts));
	m_acia0->irq_handler().set_inputline(M68K_TAG, M68K_IRQ_5);

	rs232_port_device &rs232h(RS232_PORT(config, RS232_H_TAG, default_rs232_devices, "null_modem"));
	rs232h.rxd_handler().set(m_acia0, FUNC(acia6850_device::write_rxd));
	rs232h.dcd_handler().set(m_acia0, FUNC(acia6850_device::write_dcd));
	rs232h.cts_handler().set(m_acia0, FUNC(acia6850_device::write_cts));
	rs232h.set_option_device_input_defaults("null_modem", DEVICE_INPUT_DEFAULTS_NAME(host_rs232_defaults));

	ACIA6850(config, m_acia1, 0);
	m_acia1->txd_handler().set(RS232_K_TAG, FUNC(rs232_port_device::write_txd));
	m_acia1->rts_handler().set(RS232_K_TAG, FUNC(rs232_port_device::write_rts));
	m_acia1->irq_handler().set_inputline(M68K_TAG, M68K_IRQ_2);

	rs232_port_device &rs232k(RS232_PORT(config, RS232_K_TAG, default_rs232_devices, "keyboard"));
	rs232k.rxd_handler().set(m_acia1, FUNC(acia6850_device::write_rxd));
	rs232k.dcd_handler().set(m_acia1, FUNC(acia6850_device::write_dcd));
	rs232k.cts_handler().set(m_acia1, FUNC(acia6850_device::write_cts));
	rs232k.set_option_device_input_defaults("keyboard", DEVICE_INPUT_DEFAULTS_NAME(kbd_rs232_defaults));

	RAM(config, RAM_TAG).set_default_size("256K");
}

ROM_START( blit )
	ROM_REGION16_BE(0x6000, M68K_TAG, 0)
	ROM_DEFAULT_BIOS("blit")

	ROM_SYSTEM_BIOS(0, "blit", "blit ROM")
	ROMX_LOAD("rom0.bin", 0x0001, 0x1000, CRC(e1cea320) SHA1(ba71ccd794b3bb87af58e2ff3056fc6b6e03a167), ROM_BIOS(0)|ROM_SKIP(1))
	ROMX_LOAD("rom1.bin", 0x0000, 0x1000, CRC(b717fa64) SHA1(45fbc6029031cfa6f16c258c131916f56f7d8d38), ROM_BIOS(0)|ROM_SKIP(1))
	ROMX_LOAD("rom2.bin", 0x2001, 0x1000, CRC(8a80bf57) SHA1(0b21ef8657a03418f5333772312b163d9a8e4ba0), ROM_BIOS(0)|ROM_SKIP(1))
	ROMX_LOAD("rom3.bin", 0x2000, 0x1000, CRC(846b742c) SHA1(7d27970df6e0304f3b026fd350acffa5fd921a96), ROM_BIOS(0)|ROM_SKIP(1))
	ROMX_LOAD("rom4.bin", 0x4001, 0x08bf, CRC(2af93f8b) SHA1(1168cb341bfe2a5bc283281f6afe42837adb60f1), ROM_BIOS(0)|ROM_SKIP(1))
	ROMX_LOAD("rom5.bin", 0x4000, 0x08bf, CRC(d87f121f) SHA1(6e776ac29554b8a8bb332168c155bcc502c927b5), ROM_BIOS(0)|ROM_SKIP(1))
ROM_END

} // anonymous namespace

/* Driver */
//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS        INIT           COMPANY  FULLNAME     FLAGS
COMP( 1981, blit,     0,      0,      blit,     blit,     blit_state,  empty_init,    "AT&T",  "Blit",      MACHINE_IS_SKELETON )

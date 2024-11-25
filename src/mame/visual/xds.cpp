// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Visual Technology X Display Station (XDS19P)

    Hardware:
    - MC68HC000P12F
    - 2x 27C010L-15 EPROM on cartridge (+ 4 empty sockets)
    - 32x KM44C256AP-8 / MCM514256AP80 / MT4C4256-8 (4M DRAM)
    - 4x HM53461P-10 (128k VRAM)
    - DS1225Y NVRAM
    - AM7990PC LANCE
    - MC68B45 CRTC
    - SCN68681C1N40
    - MC68B50P
    - 16.667 MHz XTAL, 100 MHz XTAL
    - XTAL X1 3.6864 MHz (assumed, unreadable), XTAL X2 (unreadable)
    - 4 position DIP switch
    - Buzzer

    TODO:
    - Verify and hook up irq mask
    - Source and timing of clock interrupt
    - What's irq1 connected to?
    - Remaining DUART output/input port bits
    - Real DIP switch meaning
    - Buzzer
    - Better screen rendering using the MC6845?
    - Configurable RAM
    - Mouse port loopback test

    Notes:
    - PCB marked "VISUAL 55-0106-000 REV A  WL3-94V1"
    - The diagnostic serial loopback test passes if you attach
      the "dec_loopback" slot device
    - "romboot" is not possible because the sockets aren't populated. You
      need to boot the system over the network instead.

***************************************************************************/

#include "emu.h"

#include "xds_kbd.h"

#include "bus/rs232/hlemouse.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/am79c90.h"
#include "machine/clock.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "video/mc6845.h"

#include "screen.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class xds_state : public driver_device
{
public:
	xds_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_crtc(*this, "crtc"),
		m_lance(*this, "lance"),
		m_duart(*this, "duart"),
		m_serialport(*this, "serialport"),
		m_mouseport(*this, "mouseport"),
		m_acia(*this, "acia"),
		m_vram(*this, "vram"),
		m_nvram(*this, "nvram", 0x2000, ENDIANNESS_BIG),
		m_prom(*this, "prom"),
		m_sw1(*this, "sw1")
	{ }

	void xds19p(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m68000_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<mc6845_device> m_crtc;
	required_device<am7990_device> m_lance;
	required_device<mc68681_device> m_duart;
	required_device<rs232_port_device> m_serialport;
	required_device<rs232_port_device> m_mouseport;
	required_device<acia6850_device> m_acia;
	required_shared_ptr<uint16_t> m_vram;
	memory_share_creator<uint8_t> m_nvram;
	required_memory_region m_prom;
	required_ioport m_sw1;

	uint8_t m_irq_mask = 0x00;

	void mem_map(address_map &map) ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint16_t lance_dma_r(offs_t offset, uint16_t mem_mask);
	void lance_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	uint8_t dip_prom_r(offs_t offset);
	void output_w(uint8_t data);
	uint8_t nvram_r(offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);
	uint8_t irq_mask_r();
	void irq_mask_w(uint8_t data);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void xds_state::mem_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0x200000, 0x2001ff).r(FUNC(xds_state::dip_prom_r)).umask16(0x00ff);
	map(0x400000, 0x400003).rw(m_lance, FUNC(am7990_device::regs_r), FUNC(am7990_device::regs_w));
	map(0x500000, 0x500003).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff);
	map(0x600000, 0x60001f).rw(m_duart, FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0x700000, 0x700001).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w)).umask16(0x00ff);
	map(0x700002, 0x700003).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w)).umask16(0x00ff);
	map(0x800000, 0xbfffff).ram(); // fixed 4M for now
	map(0xc00000, 0xc00003).rw(FUNC(xds_state::irq_mask_r), FUNC(xds_state::irq_mask_w)).umask16(0xff00);
	map(0xd00000, 0xd1ffff).ram().share("vram");
	map(0xe00000, 0xe03fff).ram().rw(FUNC(xds_state::nvram_r), FUNC(xds_state::nvram_w)).umask16(0xff00);
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( xds )
	PORT_START("sw1")
	PORT_DIPNAME( 0xf, 0x5, "Video Mode?")
	PORT_DIPSETTING(   0x0, "0" )
	PORT_DIPSETTING(   0x1, "1 - 1152x904 @72" )
	PORT_DIPSETTING(   0x2, "2 - 1024x864 @72" )
	PORT_DIPSETTING(   0x3, "3" )
	PORT_DIPSETTING(   0x4, "4" )
	PORT_DIPSETTING(   0x5, "5 - 1152x904 @72" ) // this (and 1) seems to be correct for this model
	PORT_DIPSETTING(   0x6, "6 - 1152x904 @66" )
	PORT_DIPSETTING(   0x7, "7 - 1024x800 @88" )
	PORT_DIPSETTING(   0x8, "8" )
	PORT_DIPSETTING(   0x9, "9" )
	PORT_DIPSETTING(   0xa, "a" )
	PORT_DIPSETTING(   0xb, "b" )
	PORT_DIPSETTING(   0xc, "c" )
	PORT_DIPSETTING(   0xd, "d" )
	PORT_DIPSETTING(   0xe, "e" )
	PORT_DIPSETTING(   0xf, "f" )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint32_t xds_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (unsigned y = 0; y < 904; y++)
	{
		uint32_t *line = &bitmap.pix(y);

		for (unsigned x = 0; x < 1152 / 16; x++)
		{
			uint16_t data = m_vram[y * (1152 / 16) + x];

			for (unsigned b = 0; b < 16; b++)
				*line++ = BIT(data, 15 - b) ? rgb_t::white() : rgb_t::black();
		}
	}

	return 0;
}


//**************************************************************************
//  NETWORK
//**************************************************************************

uint16_t xds_state::lance_dma_r(offs_t offset, uint16_t mem_mask)
{
	return m_maincpu->space(AS_PROGRAM).read_word(offset, mem_mask);
}

void xds_state::lance_dma_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_maincpu->space(AS_PROGRAM).write_word(offset, data, mem_mask);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t xds_state::dip_prom_r(offs_t offset)
{
	uint8_t data = 0;

	data |= (m_sw1->read() & 0xf) << 4;
	data |= (m_prom->as_u8(offset) & 0xf);

	return data;
}

void xds_state::output_w(uint8_t data)
{
	// 76------  unknown
	// --5-----  serial port spds
	// ---4----  unknown
	// ----3---  toggles at a rate of 32 hz
	// -----2--  unknown
	// ------1-  serial rts
	// -------0  serial dtr

	// maybe, but gives ~80 instead of ~100 in the clocktest diagnostic
	if (BIT(data, 3))
		m_maincpu->pulse_input_line(2, attotime::from_usec(1));

	m_serialport->write_dtr(BIT(~data, 0));
	m_serialport->write_rts(BIT(data, 1));
	m_serialport->write_spds(BIT(~data, 5));
}

uint8_t xds_state::nvram_r(offs_t offset)
{
	return m_nvram[offset];
}

void xds_state::nvram_w(offs_t offset, uint8_t data)
{
	m_nvram[offset] = data;
}

uint8_t xds_state::irq_mask_r()
{
	return m_irq_mask;
}

void xds_state::irq_mask_w(uint8_t data)
{
	logerror("irq_mask_w: %02x\n", data);
	m_irq_mask = data;
}

void xds_state::machine_start()
{
	// register for save states
	save_item(NAME(m_irq_mask));
}

void xds_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static void mouse_devices(device_slot_interface &device)
{
	device.option_add("msystems_mouse", MSYSTEMS_HLE_SERIAL_MOUSE);
}

void xds_state::xds19p(machine_config &config)
{
	M68000(config, m_maincpu, 16.667_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &xds_state::mem_map);

	NVRAM(config, "nvram");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(100_MHz_XTAL, 1472, 0, 1152, 941, 0, 904);
	m_screen->set_screen_update(FUNC(xds_state::screen_update));

	MC6845(config, m_crtc, 100_MHz_XTAL / 64);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(64);

	AM7990(config, m_lance, 0);
	m_lance->intr_out().set_inputline(m_maincpu, 5).invert();
	m_lance->dma_in().set(FUNC(xds_state::lance_dma_r));
	m_lance->dma_out().set(FUNC(xds_state::lance_dma_w));

	MC68681(config, m_duart, 3.6864_MHz_XTAL); // xtal unreadable
	m_duart->irq_cb().set_inputline(m_maincpu, 4);
	m_duart->outport_cb().set(FUNC(xds_state::output_w));
	m_duart->a_tx_cb().set(m_mouseport, FUNC(rs232_port_device::write_txd));
	m_duart->b_tx_cb().set(m_serialport, FUNC(rs232_port_device::write_txd));

	RS232_PORT(config, m_mouseport, mouse_devices, "msystems_mouse");
	m_mouseport->rxd_handler().set(m_duart, FUNC(mc68681_device::rx_a_w));
	m_mouseport->dsr_handler().set(m_duart, FUNC(mc68681_device::ip4_w));

	RS232_PORT(config, m_serialport, default_rs232_devices, nullptr);
	m_serialport->rxd_handler().set(m_duart, FUNC(mc68681_device::rx_b_w));
	m_serialport->dsr_handler().set(m_duart, FUNC(mc68681_device::ip0_w));
	m_serialport->cts_handler().set(m_duart, FUNC(mc68681_device::ip1_w));
	m_serialport->dcd_handler().set(m_duart, FUNC(mc68681_device::ip2_w));
	m_serialport->si_handler().set(m_duart, FUNC(mc68681_device::ip5_w));

	ACIA6850(config, m_acia, 0);
	m_acia->irq_handler().set_inputline(m_maincpu, 3);
	m_acia->txd_handler().set("kbd", FUNC(xds_kbd_hle_device::rx_w));

	// probably clocked externally instead
	clock_device &acia_clock(CLOCK(config, "acia_clock", 1200 * 16));
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_txc));

	// keyboard
	xds_kbd_hle_device &kbd(XDS_KBD_HLE(config, "kbd"));
	kbd.tx_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	kbd.cts_handler().set(m_acia, FUNC(acia6850_device::write_cts));
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( xds19p )
	ROM_REGION16_BE(0x40000, "maincpu", 0)
	ROM_LOAD16_BYTE("boot_30.e6", 0x00000, 0x20000, CRC(32449da2) SHA1(e1bc41987fb9c7da5b7fed5d6714c3451be4438c))
	ROM_LOAD16_BYTE("boot_30.e15", 0x00001, 0x20000, CRC(a577087f) SHA1(2fe9d4d49fd6e8ecd230f8097f7004238afbb626))

	ROM_REGION(0x100, "prom", 0)
	ROM_LOAD("1837.e06d", 0x000, 0x100, CRC(b5d9f883) SHA1(28303155c515ac2638a06123fbf7e55f89dd2af9))

	// not a real rom, this file needs to be provided by a tftp server to the system to boot
	ROM_REGION(1045320, "tftp", 0)
	ROM_LOAD("x15-3.1l", 0, 1045320, CRC(5de8fdf1) SHA1(4203ffbba775a8c4a1b827621d116ba00839f91a))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY              FULLNAME   FLAGS
COMP( 1990, xds19p, 0,      0,      xds19p,  xds,   xds_state, empty_init, "Visual Technology", "XDS-19P", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

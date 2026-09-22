// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************


    Philips CD-i consoles and games
    -------------------------------

    Preliminary MAME driver by Ryan Holtz
    Help provided by CD-i Fan


*******************************************************************************

STATUS:

  CD-i:
- The SLAVE MCU cannot be low-level emulated until there is proper /DTACK
  support in the 68k core. A2/A1 and D7..D0 are hooked up to Port C bits 1/0
  and Port A respectively, the Read/Write signal is sent to Port D bit 7, and
  /DTACK is received from Port B bit 6. The MCU therefore has the capability to
  pull /DTACK high on a data read in order to tell the 68k to hold off until
  data is ready.

- There is currently a lack of documentation on any of the chips used for
  audio in any of the CD-i models. The CDIC, which was used on Mono-I boards,
  is partially emulated thanks to information provided by CD-i Fan, the author
  of CD-i Emu. Desired documentation includes:
  * GSX38KG307CE46, "ATTEX"
  * Philips IMS66490, "CDIC" ADPCM decoder
  * PC85010 DSP

TODO:

- Screen clocks are a hack right now; they should be exactly CLOCK_A/2. However, the
  MCD-212 documentation states in both tables and timing diagrams that vertical retrace
  has an additional half-line even in non-interlaced mode, which cannot be represented
  in the current screen-timing framework. The input clock has been adjusted downward
  to factor out this half-line, resulting in the expected 50Hz exactly in PAL mode.

- Proper abstraction of the 68070's internal devices (UART, DMA, Timers, etc.)

- Mono-I: Full emulation of the CDIC, as well as the SERVO and SLAVE MCUs

- Mono-II: SERVO and SLAVE I/O device hookup
- Mono-II: DSP56k hookup

*******************************************************************************/

#include "emu.h"
#include "cdi.h"

#include "cpu/m6805/m6805.h"
#include "imagedev/cdromimg.h"
#include "machine/timekpr.h"
#include "sound/cdda.h"

#include "emupal.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"

#include "cdrom.h"

#include "cdipcb.h"

#include "cdi.lh"

// TODO: NTSC system clock is 30.2098 MHz; additional 4.9152 MHz XTAL provided for UART
#define CLOCK_A 30_MHz_XTAL

#define LOG_DVC             (1U << 1)
#define LOG_QUIZARD_READS   (1U << 2)
#define LOG_QUIZARD_WRITES  (1U << 3)
#define LOG_QUIZARD_OTHER   (1U << 4)
#define LOG_UART            (1U << 5)

#define VERBOSE         (0)
#include "logmacro.h"

// What can be plugged into the serial connector on the back.
static void cdi_serial_devices(device_slot_interface &device)
{
	device.option_add("cdipcb", CDI_SERVICE_PCB);
}

/*************************
*      Memory maps       *
*************************/
void cdi_state::cdi_common_mem(address_map &map)
{
	map(0x000000, 0xffffff).rw(FUNC(cdi_state::bus_error_r), FUNC(cdi_state::bus_error_w));
	map(0x000000, 0x07ffff).rw(FUNC(cdi_state::plane_r<0>), FUNC(cdi_state::plane_w<0>)).share("plane0");
	map(0x200000, 0x27ffff).rw(FUNC(cdi_state::plane_r<1>), FUNC(cdi_state::plane_w<1>)).share("plane1");
	map(0x320000, 0x323fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00);    /* nvram (only low bytes used) */
	map(0x400000, 0x47ffff).r(FUNC(cdi_state::main_rom_r));
	map(0x4fffe0, 0x4fffff).m(m_mcd212, FUNC(mcd212_device::map));
}

void cdi_state::cdimono1_mem(address_map &map)
{
	cdi_common_mem(map);
	map(0x300000, 0x303bff).rw(m_cdic, FUNC(cdicdic_device::ram_r), FUNC(cdicdic_device::ram_w));

	map(0x303c00, 0x303fff).rw(m_cdic, FUNC(cdicdic_device::regs_r), FUNC(cdicdic_device::regs_w));
	map(0x310000, 0x317fff).rw(m_slave_hle, FUNC(cdislave_hle_device::slave_r), FUNC(cdislave_hle_device::slave_w));
	map(0x318000, 0x31ffff).noprw();

	map(0x500000, 0x57ffff).ram();
	map(0xd00000, 0xdfffff).ram(); // DVC RAM block 1
	map(0xe00000, 0xe7ffff).rw(FUNC(cdi_state::dvc_r), FUNC(cdi_state::dvc_w));
	map(0xe80000, 0xefffff).ram(); // DVC RAM block 2
}

void cdi_state::cdimono2_mem(address_map &map)
{
	cdi_common_mem(map);
}

void cdi_state::cdi910_mem(address_map &map)
{
	map(0x000000, 0x07ffff).ram().share("plane0");
	map(0x180000, 0x1fffff).rom().region("maincpu", 0); // boot vectors point here
	map(0x200000, 0x27ffff).ram().share("plane1");

	map(0x320000, 0x323fff).rw("mk48t08", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0xff00);    /* nvram (only low bytes used) */
	map(0x4fffe0, 0x4fffff).m(m_mcd212, FUNC(mcd212_device::map));
	map(0x500000, 0xffffff).noprw();
}


/*************************
*      Input ports       *
*************************/

static INPUT_PORTS_START( cdi )
	PORT_START("MOUSEX")
	PORT_BIT(0xffff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(2)

	PORT_START("MOUSEY")
	PORT_BIT(0xffff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(2)

	PORT_START("MOUSEBTN")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Button 1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Button 2")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Button 3")
	PORT_BIT(0xf8, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("SERVICE")
	PORT_CONFNAME( 0x01, 0x00, "Service mode" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "Test plug (service shell)" )
INPUT_PORTS_END

static INPUT_PORTS_START( cdimono2 )
INPUT_PORTS_END

static INPUT_PORTS_START( quizard )
	PORT_START("P0")
	PORT_DIPNAME( 0x07, 0x05, "Settings" )
	PORT_DIPSETTING(    0x00, "1 Coin, 0 Bonus Limit, 0 Bonus Number" )
	PORT_DIPSETTING(    0x01, "2 Coins, 0 Bonus Limit, 0 Bonus Number" )
	PORT_DIPSETTING(    0x02, "1 Coin, 2 Bonus Limit, 1 Bonus Number" )
	PORT_DIPSETTING(    0x03, "1 Coin, 3 Bonus Limit, 1 Bonus Number" )
	PORT_DIPSETTING(    0x04, "1 Coin, 5 Bonus Limit, 1 Bonus Number" )
	PORT_DIPSETTING(    0x05, "1 Coin, 5 Bonus Limit, 2 Bonus Number" )
	PORT_DIPSETTING(    0x06, "1 Coin, 10 Bonus Limit, 2 Bonus Number" )
	PORT_DIPSETTING(    0x07, "2 Coins, 4 Bonus Limit, 1 Bonus Number" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xc8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Player 1 A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Player 1 B")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Player 1 C")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Player 2 A")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Player 2 B")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Player 2 C")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


/***************************
*  Machine Initialization  *
***************************/

void cdi_state::machine_reset()
{
	uint16_t *src = &m_main_rom[0];
	uint16_t *dst = &m_plane_ram[0][0];
	memcpy(dst, src, 0x8);
}

void quizard_state::machine_start()
{
	save_item(NAME(m_boot_press));

	m_boot_timer = timer_alloc(FUNC(quizard_state::boot_press_tick), this);
}

void quizard_state::machine_reset()
{
	cdi_state::machine_reset();

	m_boot_press = false;
	m_boot_timer->adjust(attotime::from_seconds(22), 1);
	m_mcu_rxd = 1;
}


/***************************
*  Wait-State Handling     *
***************************/

template<int Channel>
uint16_t cdi_state::plane_r(offs_t offset, uint16_t mem_mask)
{
	m_maincpu->eat_cycles(m_mcd212->ram_dtack_cycle_count<Channel>());
	return m_plane_ram[Channel][offset];
}

template<int Channel>
void cdi_state::plane_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_maincpu->eat_cycles(m_mcd212->ram_dtack_cycle_count<Channel>());
	COMBINE_DATA(&m_plane_ram[Channel][offset]);
}

uint16_t cdi_state::main_rom_r(offs_t offset)
{
	m_maincpu->eat_cycles(m_mcd212->rom_dtack_cycle_count());
	return m_main_rom[offset];
}


/**********************
*  BERR Handling      *
**********************/

uint16_t cdi_state::bus_error_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(offset*2, true, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
	return 0xff;
}

void cdi_state::bus_error_w(offs_t offset, uint16_t data)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(offset*2, false, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
}


/**********************
*  Quizard Protection *
**********************/

TIMER_CALLBACK_MEMBER(quizard_state::boot_press_tick)
{
	m_boot_press = (bool)param;
	if (m_boot_press)
		m_boot_timer->adjust(attotime::from_msec(250), 0);
}

uint8_t quizard_state::mcu_button_press()
{
	return (uint8_t)m_boot_press;
}

void quizard_state::mcu_rtsn_from_cpu(int state)
{
	LOGMASKED(LOG_UART, "MCU receiving RTSN from CPU: %d\n", state);
}

void quizard_state::mcu_rxd_from_cpu(int state)
{
	m_mcu_rxd = state;
}

uint8_t quizard_state::mcu_p0_r()
{
	const uint8_t data = m_inputs[0]->read();
	LOGMASKED(LOG_QUIZARD_READS, "%s: MCU Port 0 Read (%02x)\n", machine().describe_context(), data);
	return data;
}

uint8_t quizard_state::mcu_p1_r()
{
	uint8_t data = m_inputs[1]->read();
	if (BIT(~m_inputs[0]->read(), 4))
		data &= ~(1 << 4);
	LOGMASKED(LOG_QUIZARD_READS, "%s: MCU Port 1 Read (%02x)\n", machine().describe_context(), data);
	return data;
}

uint8_t quizard_state::mcu_p2_r()
{
	const uint8_t data = m_inputs[2]->read();
	LOGMASKED(LOG_QUIZARD_READS, "%s: MCU Port 2 Read (%02x)\n", machine().describe_context(), data);
	return data;
}

uint8_t quizard_state::mcu_p3_r()
{
	const uint8_t data = m_mcu_rxd ? 0x7f : 0x7e;
	LOGMASKED(LOG_QUIZARD_READS, "%s: MCU Port 3 Read (%02x)\n", machine().describe_context(), data);
	return data;
}

void quizard_state::mcu_p0_w(uint8_t data)
{
	LOGMASKED(LOG_QUIZARD_WRITES, "%s: MCU Port 0 Write (%02x)\n", machine().describe_context(), data);
}

void quizard_state::mcu_p1_w(uint8_t data)
{
	LOGMASKED(LOG_QUIZARD_WRITES, "%s: MCU Port 1 Write (%02x)\n", machine().describe_context(), data);
}

void quizard_state::mcu_p2_w(uint8_t data)
{
	LOGMASKED(LOG_QUIZARD_WRITES, "%s: MCU Port 2 Write (%02x)\n", machine().describe_context(), data);
}

void quizard_state::mcu_p3_w(uint8_t data)
{
	LOGMASKED(LOG_QUIZARD_WRITES, "%s: MCU Port 3 Write (%02x)\n", machine().describe_context(), data);
	m_maincpu->rx_w(BIT(data, 1));
	m_maincpu->uart_ctsn(BIT(data, 6));
}

/*************************
*     DVC cartridge      *
*************************/

uint16_t cdi_state::dvc_r(offs_t offset, uint16_t mem_mask)
{
	LOGMASKED(LOG_DVC, "%s: dvc_r: %08x = 0000 & %04x\n", machine().describe_context(), 0xe80000 + (offset << 1), mem_mask);
	return 0;
}

void cdi_state::dvc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_DVC, "%s: dvc_w: %08x = %04x & %04x\n", machine().describe_context(), 0xe80000 + (offset << 1), data, mem_mask);
}

/*************************
*       LCD screen       *
*************************/


uint32_t cdi_state::screen_update_cdimono1_lcd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint8_t lcd_state[16];
	std::copy_n(m_slave_hle->get_lcd_state(), 16, lcd_state);

	cdi220_lcd::draw(bitmap, cliprect, lcd_state);
	return 0;
}

/*************************
*    Machine Drivers     *
*************************/

// CD-i Mono-I system base
void cdi_state::cdimono1_base(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_A);
	m_maincpu->set_addrmap(AS_PROGRAM, &cdi_state::cdimono1_mem);


	// The serial connector on the back, carrying the 68070's UART.
	RS232_PORT(config, m_serial_port, cdi_serial_devices, nullptr);
	m_maincpu->out_txd_cb().set(m_serial_port, FUNC(rs232_port_device::write_txd));
	m_maincpu->uart_rtsn_callback().set(m_serial_port, FUNC(rs232_port_device::write_rts));
	m_serial_port->rxd_handler().set(m_maincpu, FUNC(scc68070_device::rx_w));
	m_serial_port->cts_handler().set(m_maincpu, FUNC(scc68070_device::uart_ctsn));

	m_maincpu->iack4_callback().set(m_cdic, FUNC(cdicdic_device::intack_r));

	MCD212(config, m_mcd212, CLOCK_A, m_plane_ram[0], m_plane_ram[1]);
	m_mcd212->set_screen("screen");
	m_mcd212->int_callback().set(m_maincpu, FUNC(scc68070_device::int1_w));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(14976000*2, 960, 0, 768, 312*2, 32*2, 312*2); // x2 for interlace
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
	screen.set_screen_update(m_mcd212, FUNC(mcd212_device::screen_update));

	SCREEN(config, m_lcd);
	m_lcd->set_refresh_hz(50);
	m_lcd->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_lcd->set_size(cdi220_lcd::WIDTH, cdi220_lcd::HEIGHT);
	m_lcd->set_visarea_full();
	m_lcd->set_screen_update(FUNC(cdi_state::screen_update_cdimono1_lcd));

	PALETTE(config, "palette").set_entries(0x100);

	config.set_default_layout(layout_cdi);

	// IMS66490 CDIC input clocks are 22.5792 MHz and 19.3536 MHz
	// DSP input clock is 7.5264 MHz
	CDI_CDIC(config, m_cdic, 45.1584_MHz_XTAL / 2);
	m_cdic->set_clock2(45.1584_MHz_XTAL * 3 / 7); // generated by PLL circuit incorporating 19.3575 MHz XTAL
	m_cdic->intreq_callback().set(m_maincpu, FUNC(scc68070_device::in4_w));

	CDI_SLAVE_HLE(config, m_slave_hle);
	m_slave_hle->int_callback().set(m_maincpu, FUNC(scc68070_device::in2_w));
	m_slave_hle->atten_callback().set(m_cdic, FUNC(cdicdic_device::atten_w));

	CDROM(config, m_cdrom);
	m_cdrom->set_interface("cdrom");

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	DMADAC(config, m_dmadac[0]);
	m_dmadac[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);

	DMADAC(config, m_dmadac[1]);
	m_dmadac[1]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	MK48T08(config, "mk48t08");
}

// CD-i model 220 (Mono-II, NTSC)
void cdi_state::cdimono2(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_A);
	m_maincpu->set_addrmap(AS_PROGRAM, &cdi_state::cdimono2_mem);

	MCD212(config, m_mcd212, CLOCK_A, m_plane_ram[0], m_plane_ram[1]);
	m_mcd212->set_screen("screen");
	m_mcd212->int_callback().set(m_maincpu, FUNC(scc68070_device::int1_w));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(14976000*2, 960, 0, 768, 312*2, 32*2, 312*2); // x2 for interlace
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
	screen.set_screen_update(m_mcd212, FUNC(mcd212_device::screen_update));

	SCREEN(config, m_lcd);
	m_lcd->set_refresh_hz(60);
	m_lcd->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_lcd->set_size(cdi220_lcd::WIDTH, cdi220_lcd::HEIGHT);
	m_lcd->set_visarea_full();
	m_lcd->set_screen_update(FUNC(cdi_state::screen_update_cdimono1_lcd));

	PALETTE(config, "palette").set_entries(0x100);

	config.set_default_layout(layout_cdi);

	M68HC05C8(config, m_servo, 4_MHz_XTAL);
	M68HC05C8(config, m_slave, 4_MHz_XTAL);

	CDROM(config, m_cdrom).set_interface("cdrom");
	SOFTWARE_LIST(config, "cd_list").set_original("cdi").set_filter("!DVC");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	DMADAC(config, m_dmadac[0]);
	m_dmadac[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);

	DMADAC(config, m_dmadac[1]);
	m_dmadac[1]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	MK48T08(config, "mk48t08");
}

void cdi_state::cdi910(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_A);
	m_maincpu->set_addrmap(AS_PROGRAM, &cdi_state::cdi910_mem);

	MCD212(config, m_mcd212, CLOCK_A, m_plane_ram[0], m_plane_ram[1]);
	m_mcd212->set_screen("screen");
	m_mcd212->int_callback().set(m_maincpu, FUNC(scc68070_device::int1_w));

	screen_device &screen(SCREEN(config, "screen"));
	screen.set_raw(14976000*2, 960, 0, 768, 312*2, 32*2, 312*2); // x2 for interlace
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
	screen.set_screen_update(m_mcd212, FUNC(mcd212_device::screen_update));

	SCREEN(config, m_lcd);
	m_lcd->set_refresh_hz(60);
	m_lcd->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_lcd->set_size(cdi220_lcd::WIDTH, cdi220_lcd::HEIGHT);
	m_lcd->set_visarea_full();
	m_lcd->set_screen_update(FUNC(cdi_state::screen_update_cdimono1_lcd));

	PALETTE(config, "palette").set_entries(0x100);

	config.set_default_layout(layout_cdi);

	M68HC05C8(config, m_servo, 4_MHz_XTAL);
	M68HC05C8(config, m_slave, 4_MHz_XTAL);

	CDROM(config, "cdrom").set_interface("cdrom");
	SOFTWARE_LIST(config, "cd_list").set_original("cdi").set_filter("!DVC");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	DMADAC(config, m_dmadac[0]);
	m_dmadac[0]->add_route(ALL_OUTPUTS, "speaker", 1.0, 0);

	DMADAC(config, m_dmadac[1]);
	m_dmadac[1]->add_route(ALL_OUTPUTS, "speaker", 1.0, 1);

	MK48T08(config, "mk48t08");
}

// CD-i Mono-I, with CD-ROM image device (MESS) and Software List (MESS)
void cdi_state::cdimono1(machine_config &config)
{
	cdimono1_base(config);

	m_slave_hle->read_mousex().set_ioport("MOUSEX");
	m_slave_hle->read_mousey().set_ioport("MOUSEY");
	m_slave_hle->read_mousebtn().set_ioport("MOUSEBTN");
	m_slave_hle->testplug_callback().set_ioport("SERVICE").bit(0);

	SOFTWARE_LIST(config, "cd_list").set_original("cdi").set_filter("!DVC");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
}

void quizard_state::quizard(machine_config &config)
{
	cdimono1_base(config);
	m_cdrom->add_region("cdrom");

	m_maincpu->set_addrmap(AS_PROGRAM, &quizard_state::cdimono1_mem);
	m_maincpu->uart_rtsn_callback().set(FUNC(quizard_state::mcu_rtsn_from_cpu));
	m_maincpu->out_txd_cb().set(FUNC(quizard_state::mcu_rxd_from_cpu));

	I8751(config, m_mcu, 11.0592_MHz_XTAL);
	m_mcu->port_in_cb<0>().set(FUNC(quizard_state::mcu_p0_r));
	m_mcu->port_in_cb<1>().set(FUNC(quizard_state::mcu_p1_r));
	m_mcu->port_in_cb<2>().set(FUNC(quizard_state::mcu_p2_r));
	m_mcu->port_in_cb<3>().set(FUNC(quizard_state::mcu_p3_r));
	m_mcu->port_out_cb<0>().set(FUNC(quizard_state::mcu_p0_w));
	m_mcu->port_out_cb<1>().set(FUNC(quizard_state::mcu_p1_w));
	m_mcu->port_out_cb<2>().set(FUNC(quizard_state::mcu_p2_w));
	m_mcu->port_out_cb<3>().set(FUNC(quizard_state::mcu_p3_w));

	m_slave_hle->read_mousebtn().set(FUNC(quizard_state::mcu_button_press));
}

/*************************
*        Rom Load        *
*************************/

ROM_START( cdimono1 )
	ROM_REGION(0x80000, "maincpu", 0) // these roms need byteswapping
	ROM_SYSTEM_BIOS( 0, "mcdi200", "Magnavox CD-i 200" )
	ROMX_LOAD( "cdi200.rom", 0x000000, 0x80000, CRC(40c4e6b9) SHA1(d961de803c89b3d1902d656ceb9ce7c02dccb40a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pcdi220", "Philips CD-i 220 F2" )
	ROMX_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "pcdi220_alt", "Philips CD-i 220?" ) // doesn't boot
	ROMX_LOAD( "cdi220.rom", 0x000000, 0x80000, CRC(584c0af8) SHA1(5d757ab46b8c8fc36361555d978d7af768342d47), ROM_BIOS(2) )

	// The two MCU dumps below are taken from the cdi910. We still need dumps from a Mono-I board in case the revisions are different.
	ROM_REGION(0x2000, "servo", 0)
	ROM_LOAD( "zx405037p__cdi_servo_2.1__b43t__llek9215.mc68hc705c8a_withtestrom.7201", 0x0000, 0x2000, CRC(7a3af407) SHA1(fdf8d78d6a0df4a56b5b963d72eabd39fcec163f) BAD_DUMP )

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zx405042p__cdi_slave_2.0__b43t__zzmk9213.mc68hc705c8a_withtestrom.7206", 0x0000, 0x2000, CRC(688cda63) SHA1(56d0acd7caad51c7de703247cd6d842b36173079) BAD_DUMP )
ROM_END

ROM_START( cdi910 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "cdi910", "CD-I 910-17P Mini-MMC" )
	ROMX_LOAD( "philips__cd-i_2.1__mb834200b-15__26b_aa__9224_z01.tc574200.7211", 0x000000, 0x80000, CRC(4ae3bee3) SHA1(9729b4ee3ce0c17172d062339c47b1ab822b222b), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE )
	ROM_SYSTEM_BIOS( 1, "cdi910_alt", "alt" )
	ROMX_LOAD( "cdi910.rom", 0x000000, 0x80000, CRC(2f3048d2) SHA1(11c4c3e602060518b52e77156345fa01f619e793), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE )

	ROM_REGION(0x2000, "servo", 0)
	ROM_LOAD( "zx405037p__cdi_servo_2.1__b43t__llek9215.mc68hc705c8a_withtestrom.7201", 0x0000, 0x2000, CRC(7a3af407) SHA1(fdf8d78d6a0df4a56b5b963d72eabd39fcec163f) )

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zx405042p__cdi_slave_2.0__b43t__zzmk9213.mc68hc705c8a_withtestrom.7206", 0x0000, 0x2000, CRC(688cda63) SHA1(56d0acd7caad51c7de703247cd6d842b36173079) )

	ROM_REGION(0x2000, "pals", 0)
	ROM_LOAD( "ti_portugal_206xf__tibpal20l8-15cnt__m7205n.7205.bin",      0x0000, 0x144, CRC(dd167e0d) SHA1(2ba82a4619d7a0f19e62e02a2841afd4d45d56ba) )
	ROM_LOAD( "ti_portugal_774_206xf__tibpal16l8-10cn_m7204n.7204.bin",    0x0000, 0x104, CRC(04e6bd37) SHA1(153d1a977291bedb7420484a9f889325dbd3628e) )
ROM_END

ROM_START( cdimono2 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "pcdi220", "Philips CD-i 220 F3")
	ROMX_LOAD( "philips__cdi-220_ph3_r1.2__mb834200b-15__02f_aa__9402_z04.tc574200-le._1.7211", 0x000000, 0x80000, CRC(17d723e7) SHA1(6c317a82e35d60ca5e7a74fc99f665055693169d), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE )
	ROM_SYSTEM_BIOS(1, "pcdi210", "Philips CD-i 210 F2")
	ROMX_LOAD( "philips__cd-i_4.1_r1.1__mb834200b-15__10e_aa__9336_z01.7211", 0x000000, 0x80000, CRC(8453553f) SHA1(5ee4dc3e7eb4c3867ac9d04f1614908906af19fb), ROM_BIOS(1) | ROM_GROUPWORD | ROM_REVERSE )

	ROM_REGION(0x2000, "servo", 0)
	ROM_LOAD( "zc405351p__servo_cdi_4.1__0d67p__lluk9404.mc68hc705c8a.7490", 0x0000, 0x2000, CRC(2bc8e4e9) SHA1(8cd052b532fc052d6b0077261c12f800e8655bb1) )

	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zc405352p__slave_cdi_4.1__0d67p__lltr9403.mc68hc705c8a.7206", 0x0000, 0x2000, CRC(5b19da07) SHA1(cf02d84977050c71e87a38f1249e83c43a93949b) )
ROM_END

ROM_START( cdi490a )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "cdi490", "CD-i 490" )
	ROMX_LOAD( "cdi490a.rom", 0x000000, 0x80000, CRC(e2f200f6) SHA1(c9bf3c4c7e4fe5cbec3fe3fc993c77a4522ca547), ROM_BIOS(0) | ROM_GROUPWORD | ROM_REVERSE  )

	ROM_REGION(0x60000, "mpegs", 0) // keep these somewhere
	ROM_LOAD( "impega.rom", 0x00000, 0x40000, CRC(84d6f6aa) SHA1(02526482a0851ea2a7b582d8afaa8ef14a8bd914) ) // 1ST AND 2ND HALF IDENTICAL
	// Philips CD-i - DVC card 22ER9141
	ROM_LOAD16_BYTE( "fmv ffd9 p7308 r4.1 vmpeg.bin", 0x40000, 0x10000, CRC(30ba9273) SHA1(d8adca0627b356ced6131b9458ac1175e43e6548) )
	ROM_LOAD16_BYTE( "fmv 4ba9 p7307 r4.1 vmpeg.bin", 0x40001, 0x10000, CRC(623edb1f) SHA1(4c6b11e28ad4c2f5c2e439f7910a783e0a79d1a9) )
ROM_END

ROM_START( gpi1200 )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_LOAD16_WORD_SWAP( "gpi-1200k-1313.bin", 0x000000, 0x80000, CRC(dbd41615) SHA1(83929617a5c01551ee961aeb685295fcc0810f54) )
ROM_END

ROM_START( cdibios ) // for the quizard sets
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "mcdi200", "Magnavox CD-i 200" )
	ROMX_LOAD( "cdi200.rom", 0x000000, 0x80000, CRC(40c4e6b9) SHA1(d961de803c89b3d1902d656ceb9ce7c02dccb40a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "pcdi220", "Philips CD-i 220 F2" )
	ROMX_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e), ROM_BIOS(1) )

	// The MCU dump below is taken from the cdi910. We still need a dump from a Mono-I board SLAVE MCU in case the revisions are different.
	ROM_REGION(0x2000, "slave", 0)
	ROM_LOAD( "zx405042p__cdi_slave_2.0__b43t__zzmk9213.mc68hc705c8a_withtestrom.7206", 0x0000, 0x2000, CRC(688cda63) SHA1(56d0acd7caad51c7de703247cd6d842b36173079) BAD_DUMP )
ROM_END

/*  Quizard notes

    The MCU controls the protection sequence, which in turn controls the game display language.
    Each Quizard game (1,2,3,4) requires its own MCU, you can upgrade between revisions by changing
    just the CD, but not between games as a new MCU is required.

    MCU Notes:
    i8751 MCU dumps confirmed good on original hardware
    Italian language MCU for Quizard 1 is dumped
    German language MCUs for Quizard 1 through 4 are dumped
    Czech language MCU for Quizard 4 is dumped
    Alt. German language MCU for Quizard 2 is known to exist (DE 122 D3, not dumped)

*/


#define QUIZARD_BIOS_ROM \
	ROM_REGION(0x80000, "maincpu", 0) \
	ROM_LOAD( "cdi220b.rom", 0x000000, 0x80000, CRC(279683ca) SHA1(53360a1f21ddac952e95306ced64186a3fc0b93e) )

//********************************************************
//                     Quizard (1)
//********************************************************

#define QUIZARD1_CHD_10 \
	DISK_REGION( "cdrom" ) \
	DISK_IMAGE_READONLY( "quizard10", 0, SHA1(5715db50f0d5ffe06f47c0943f4bf0481ab6048e) ) // Dumped via BurnAtOnce 0.99.5, CHDMAN 0.163, TS-L633R drive

// CD-ROM printed 01/95
#define QUIZARD1_CHD_12 \
	DISK_REGION( "cdrom" ) \
	DISK_IMAGE_READONLY( "quizard12", 0, BAD_DUMP SHA1(6e41683b96b74e903040842aeb18437ad7813c82) )

#define QUIZARD1_CHD_17 \
	DISK_REGION( "cdrom" ) \
	DISK_IMAGE_READONLY( "quizard17", 0, BAD_DUMP SHA1(4bd698f076505b4e17be978481bce027eb47123b) )

#define QUIZARD1_CHD_18 \
	DISK_REGION( "cdrom" ) \
	DISK_IMAGE_READONLY( "quizard18", 0, BAD_DUMP SHA1(ede873b22957f2a707bbd3039e962ef2ca5aedbd) )

// MCU Type: Intel D8751H MCU
#define QUIZARD1_MCU \
	ROM_REGION(0x1000, "mcu", 0) /* Intel D8751H MCU */ \
	ROM_SYSTEM_BIOS( 0, "de021f", "German, DE 11 D3 (0x021F)" ) \
	ROMX_LOAD( "de_11_d3.bin", 0x0000, 0x1000, CRC(95f45b6b) SHA1(51b34956539b1e2cf0306f243a970750f1e18d01), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "de019c", "German alt. (0x019C)" ) \
	ROMX_LOAD( "quizard_mcu_de_019c.bin", 0x0000, 0x1000, CRC(c7b48c71) SHA1(7fd950f110c4616a040339d7c72b115be8c8552e) BAD_DUMP, ROM_BIOS(1) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 2, "en00c5", "English (0x00C5)" ) \
	ROMX_LOAD( "quizard_mcu_en_00c5.bin", 0x0000, 0x1000, CRC(ded29c61) SHA1(b190d5401c3bf8a01e45b151dc78eaf3e4900e5e) BAD_DUMP, ROM_BIOS(2) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 3, "it0162", "Italian (0x0162)" ) \
	ROMX_LOAD( "quizard_mcu_it_0162.bin", 0x0000, 0x1000, CRC(fb560b5a) SHA1(1ed9c892e8c7fcd0c25d9248fec467b7b1798f33) BAD_DUMP, ROM_BIOS(3) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 4, "fr0114", "French (0x0114)" ) \
	ROMX_LOAD( "quizard_mcu_fr_0114.bin", 0x0000, 0x1000, CRC(237ddd99) SHA1(f14d54fde957a29eea01ef7d97e2bcdf83237824) BAD_DUMP, ROM_BIOS(4) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 5, "es02f6", "Spanish (0x02F6)" ) \
	ROMX_LOAD( "quizard_mcu_es_02f6.bin", 0x0000, 0x1000, CRC(2335fbdf) SHA1(a9a3a0eb5df5cb65029d1b0faf7a30c3f988f729) BAD_DUMP, ROM_BIOS(5) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 6, "gr01e8", "Greek #1 (0x01E8)" ) \
	ROMX_LOAD( "quizard_mcu_gr_01e8.bin", 0x0000, 0x1000, CRC(db265c13) SHA1(fa3323a0574b7d705aa05512af6448f70aed03dc) BAD_DUMP, ROM_BIOS(6) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 7, "gr0126", "Greek #2 (0x0126)" ) \
	ROMX_LOAD( "quizard_mcu_gr_0126.bin", 0x0000, 0x1000, CRC(8932a3ae) SHA1(a7db610e481e3a1556ddae03bb9c0667381b237c) BAD_DUMP, ROM_BIOS(7) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 8, "gr0293", "Greek #3 (0x0293)" ) \
	ROMX_LOAD( "quizard_mcu_gr_0293.bin", 0x0000, 0x1000, CRC(233e0180) SHA1(7ffd22afcf92ae65ca5ea7c5a5aa4d4ac5b9c3a9) BAD_DUMP, ROM_BIOS(8) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 9, "hr0343", "Croatian (0x0343)" ) \
	ROMX_LOAD( "quizard_mcu_hr_0343.bin", 0x0000, 0x1000, CRC(51754008) SHA1(c5e5c732579bfdf7170f03d990d4173585aa5eed) BAD_DUMP, ROM_BIOS(9) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 10, "sl00fd", "Slovenian (0x00FD)" ) \
	ROMX_LOAD( "quizard_mcu_sl_00fd.bin", 0x0000, 0x1000, CRC(1422f2f1) SHA1(a1332c103c1cbdf0db9194c8fe75ff09560d3db7) BAD_DUMP, ROM_BIOS(10) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 11, "cz00a9", "Czech (0x00A9)" ) \
	ROMX_LOAD( "quizard_mcu_cz_00a9.bin", 0x0000, 0x1000, CRC(f53b7048) SHA1(7f89ee667652ef71cb3a960247a1d416a3b0ca8c) BAD_DUMP, ROM_BIOS(11) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 12, "ar0001", "Arabic (0x0001)" ) \
	ROMX_LOAD( "quizard_mcu_ar_0001.bin", 0x0000, 0x1000, CRC(ec79737b) SHA1(802b7fc3ca7ca4d8afb1c769a92d7841cd41be3b) BAD_DUMP, ROM_BIOS(12) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 13, "ae02de", "American English (0x02DE)" ) \
	ROMX_LOAD( "quizard_mcu_ae_02de.bin", 0x0000, 0x1000, CRC(7ab8bf02) SHA1(6c11b8771ebd39cfb8dbec8593733f81b6b9c161) BAD_DUMP, ROM_BIOS(13) ) /* AI-modified, needs proper dump */

#define QUIZARD1_MCU_IT \
	ROM_REGION(0x1000, "mcu", 0) \
	ROM_LOAD( "it_11_i2.bin", 0x0000, 0x1000, CRC(e00dc02c) SHA1(e4ef1ea47c242879a99c9d54cfc008ae99a651cb) ) // Italian

ROM_START( quizard )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_18
	QUIZARD1_MCU
ROM_END

ROM_START( quizard_17 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_17
	QUIZARD1_MCU
ROM_END

ROM_START( quizard_12 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_12
	QUIZARD1_MCU
ROM_END

ROM_START( quizard_10 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_10
	QUIZARD1_MCU
ROM_END

ROM_START( quizardi )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_18
	QUIZARD1_MCU_IT
ROM_END

ROM_START( quizardi_17 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_17
	QUIZARD1_MCU_IT
ROM_END

ROM_START( quizardi_12 )
	QUIZARD_BIOS_ROM
	QUIZARD1_CHD_12
	QUIZARD1_MCU_IT
ROM_END

//********************************************************
//                     Quizard 2
//********************************************************

#define QUIZARD2_MCU \
	ROM_REGION(0x1000, "mcu", 0) /* Intel D8751H MCU */ \
	ROM_SYSTEM_BIOS( 0, "de02c7", "German, DN 122 D3 (0x02C7)" ) \
	ROMX_LOAD( "dn_122_d3.bin", 0x0000, 0x1000, CRC(d48063ea) SHA1(b512fa5e53f296a180340e09b53613dd1c0d38bc), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "de0188", "German alt. (0x0188)" ) \
	ROMX_LOAD( "quizard2_mcu_de1_0188.bin", 0x0000, 0x1000, CRC(5020df9a) SHA1(199c3f5dbe6abb1fa1e9834e4c8a51125713caaf) BAD_DUMP, ROM_BIOS(1) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 2, "it02b1", "Italian (0x02B1)" ) \
	ROMX_LOAD( "quizard2_mcu_it_02b1.bin", 0x0000, 0x1000, CRC(9026f8ed) SHA1(3ab343bb11a9065451d5a5dd912ac18d55364901) BAD_DUMP, ROM_BIOS(2) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 3, "cz0083", "Czech (0x0083)" ) \
	ROMX_LOAD( "quizard2_mcu_cz_0083.bin", 0x0000, 0x1000, CRC(10488cef) SHA1(2dadfea1334d79a8ed4a6b4e4a4a612526344313) BAD_DUMP, ROM_BIOS(3) ) /* AI-modified, needs proper dump */

#define QUIZARD2_22_MCU \
	ROM_REGION(0x1000, "mcu", 0) /* Intel D8751H MCU */ \
	ROM_SYSTEM_BIOS( 0, "de02c7", "German, DN 122 D3 (0x02C7)" ) \
	ROMX_LOAD( "dn_122_d3.bin", 0x0000, 0x1000, CRC(d48063ea) SHA1(b512fa5e53f296a180340e09b53613dd1c0d38bc), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "en0343", "English (0x0343)" ) \
	ROMX_LOAD( "quizard2_2_mcu_en_0343.bin", 0x0000, 0x1000, CRC(59466774) SHA1(eb7cb6af175ba11d3e34060c69219220d9a0d73c) BAD_DUMP, ROM_BIOS(1) ) /* AI-modified, needs proper dump */

ROM_START( quizard2 ) /* CD-ROM printed ??/?? */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard23", 0, BAD_DUMP SHA1(cd909d9a54275d6f2d36e03e83eea996e781b4d3) )

	QUIZARD2_MCU
ROM_END

ROM_START( quizard2_22 )
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard22", 0, BAD_DUMP SHA1(03c8fdcf27ead6e221691111e8c679b551099543) )

	QUIZARD2_22_MCU
ROM_END


//********************************************************
//                     Quizard 3
//********************************************************

#define QUIZARD3_MCU \
	ROM_REGION(0x1000, "mcu", 0) /* Intel D8751H MCU */ \
	ROM_SYSTEM_BIOS( 0, "de00ae", "German, DE 132 D3 (0x00AE)" ) \
	ROMX_LOAD( "de_132_d3.bin", 0x0000, 0x1000, CRC(8858251e) SHA1(2c1005a74bb6f0c2918dff4ab6326528eea48e1f), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "de026d", "German alt. #1 (0x026D)" ) \
	ROMX_LOAD( "quizard3_4_mcu_de_2_026d.bin", 0x0000, 0x1000, CRC(100bfe36) SHA1(710ae406e3de153edcc9f196da05d2dd080fabfb) BAD_DUMP, ROM_BIOS(1) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 2, "de0181", "German alt. #2 (0x0181)" ) \
	ROMX_LOAD( "quizard3_4_mcu_de_3_0181.bin", 0x0000, 0x1000, CRC(b4efe412) SHA1(0cccfdaf70d1694aeda639d7a094da669d3384e8) BAD_DUMP, ROM_BIOS(2) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 3, "en0001", "English (0x0001)" ) \
	ROMX_LOAD( "quizard3_4_mcu_en_0001.bin", 0x0000, 0x1000, CRC(3798f723) SHA1(4ec6ca3399a6a7b0898083535edd752cac764e1e) BAD_DUMP, ROM_BIOS(3) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 4, "fr0002", "French (0x0002)" ) \
	ROMX_LOAD( "quizard3_4_mcu_fr_0002.bin", 0x0000, 0x1000, CRC(95221971) SHA1(cd9808fd8cdddb8597b998d39213f90cd0ca2bf0) BAD_DUMP, ROM_BIOS(4) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 5, "es0003", "Spanish (0x0003)" ) \
	ROMX_LOAD( "quizard3_4_mcu_es_0003.bin", 0x0000, 0x1000, CRC(4264be80) SHA1(943f2ad0d39faff3ab2bb89baf1b1559f865698d) BAD_DUMP, ROM_BIOS(5) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 6, "it01c3", "Italian (0x01C3)" ) \
	ROMX_LOAD( "quizard3_4_mcu_it_01c3.bin", 0x0000, 0x1000, CRC(a512abd9) SHA1(8c6b321a8fd36f6e34d69806bede23ee9f2c0e09) BAD_DUMP, ROM_BIOS(6) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 7, "gr00ef", "Greek #1 (0x00EF)" ) \
	ROMX_LOAD( "quizard3_4_mcu_gr_1_00ef.bin", 0x0000, 0x1000, CRC(3b1f8487) SHA1(68f175b37fc4a3a71e1c7258b64161789b3cced3) BAD_DUMP, ROM_BIOS(7) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 8, "gr0335", "Greek #2 (0x0335)" ) \
	ROMX_LOAD( "quizard3_4_mcu_gr_2_0335.bin", 0x0000, 0x1000, CRC(2aff86e8) SHA1(f6b7ef0f0fac0a2b9076c3107519abc0906661d5) BAD_DUMP, ROM_BIOS(8) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 9, "cz02c0", "Czech (0x02C0)" ) \
	ROMX_LOAD( "quizard3_4_mcu_cz_02c0.bin", 0x0000, 0x1000, CRC(da3765a8) SHA1(1942ce822a4937ebc625e3a1d91c650d0d5279c0) BAD_DUMP, ROM_BIOS(9) ) /* AI-modified, needs proper dump */

ROM_START( quizard3 ) /* CD-ROM printed ??/?? */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard34", 0, BAD_DUMP SHA1(37ad49b72b5175afbb87141d57bc8604347fe032) )

	QUIZARD3_MCU
ROM_END

ROM_START( quizard3a ) /* CD-ROM printed ??/?? */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard34", 0, BAD_DUMP SHA1(37ad49b72b5175afbb87141d57bc8604347fe032) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "de_132_a1.bin", 0x0000, 0x1000, CRC(313ac673) SHA1(cb0ee7e9a6eaa5f4d000f5ea99b7ee4c440b31d1) ) // German language - earlier version of MCU code
ROM_END

ROM_START( quizard3_32 )
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard32", 0, BAD_DUMP SHA1(31e9fa2169aa44d799c37170b238134ab738e1a1) )

	QUIZARD3_MCU
ROM_END


//********************************************************
//                     Quizard 4
//********************************************************

// these are also valid for quizard4_41
#define QUIZARD4_42_MCU \
	ROM_REGION(0x1000, "mcu", 0) /* Intel D8751H MCU */ \
	ROM_SYSTEM_BIOS( 0, "de142d3", "DE 142 D3 - G_SCREEN, German questions, English UI, erotic on (code 01, 0x004D)" ) \
	ROMX_LOAD( "de_142_d3.bin", 0x0000, 0x1000, CRC(77be0b40) SHA1(113b5c239480a2259f55e411ba8fb3972e6d4301), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "en_on2", "G_SCREEN, German questions, English UI, erotic on (code 03, 0x011F)" ) \
	ROMX_LOAD( "quizard4.2_mcu_code03_011f.bin", 0x0000, 0x1000, CRC(81fc7b0b) SHA1(6e4e47716befb7db9c1c1eb3244412f14a359682) BAD_DUMP, ROM_BIOS(1) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 2, "en_on3", "G_SCREEN, German questions, English UI, erotic on (code 05, 0x03CC)" ) \
	ROMX_LOAD( "quizard4.2_mcu_code05_03cc.bin", 0x0000, 0x1000, CRC(b5008f8b) SHA1(0e90c23e677b2ae91a6be8a3f0548cd2dabda67f) BAD_DUMP, ROM_BIOS(2) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 3, "en_on4", "G_SCREEN, German questions, English UI, erotic on (code 07, 0x00DE)" ) \
	ROMX_LOAD( "quizard4.2_mcu_code07_00de.bin", 0x0000, 0x1000, CRC(a98d4c59) SHA1(16c646347d0da86bc6e87014c8a7fb86749afb5c) BAD_DUMP, ROM_BIOS(3) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 4, "cz_on", "C_SCREEN, Czech questions and UI, erotic on (code 09, 0x0277)" ) \
	ROMX_LOAD( "quizard4.2_mcu_code09_0277.bin", 0x0000, 0x1000, CRC(cb53afd4) SHA1(6ee895475d6d2ebeb86af6017912be8ee38af323) BAD_DUMP, ROM_BIOS(4) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 5, "cz_off", "CO_SCREEN, Czech questions and UI, erotic off (code 10, 0x0139)" ) \
	ROMX_LOAD( "quizard4.2_mcu_code10_0139.bin", 0x0000, 0x1000, CRC(9cc85f3c) SHA1(d91223dd120c0460b8a33311138528feeb12c8c9) BAD_DUMP, ROM_BIOS(5) ) /* AI-modified, needs proper dump */

/*
    The following are theoretically all good MCU codes, but the CD doesn't support the erotic off path for the German questions.

    ROM_SYSTEM_BIOS( 6, "alt1", "A_SCREEN, erotic off path (code 02, 0x0195) - black screen on the dumped disc" )
    ROMX_LOAD( "quizard4.2_mcu_code02_0195.bin", 0x0000, 0x1000, CRC(67c5dcc9) SHA1(7bc12ae26c17a1e7f03432ecfdbaa607189a5094) BAD_DUMP, ROM_BIOS(6) ) // AI-modified, needs proper dump
    ROM_SYSTEM_BIOS( 7, "alt2", "A_SCREEN, erotic off path (code 04, 0x0234) - black screen on the dumped disc" )
    ROMX_LOAD( "quizard4.2_mcu_code04_0234.bin", 0x0000, 0x1000, CRC(4787aeaa) SHA1(30591fd6da9c45f613e2b2611234a8caf1cb6895) BAD_DUMP, ROM_BIOS(7) ) // AI-modified, needs proper dump
    ROM_SYSTEM_BIOS( 8, "alt3", "A_SCREEN, erotic off path (code 06, 0x017B) - black screen on the dumped disc" )
    ROMX_LOAD( "quizard4.2_mcu_code06_017b.bin", 0x0000, 0x1000, CRC(61d7f867) SHA1(dd981c3dcaa329ee9c619515744b2d3e5fa64bd1) BAD_DUMP, ROM_BIOS(8) ) // AI-modified, needs proper dump
    ROM_SYSTEM_BIOS( 9, "alt4", "A_SCREEN, erotic off path (code 08, 0x0177) - black screen on the dumped disc" )
    ROMX_LOAD( "quizard4.2_mcu_code08_0177.bin", 0x0000, 0x1000, CRC(45375b18) SHA1(f3b09d65aab174652b16399154e4f01a450bfe23) BAD_DUMP, ROM_BIOS(9) ) // AI-modified, needs proper dump
*/


#define QUIZARD4_40_MCU \
	ROM_REGION(0x1000, "mcu", 0) /* Intel D8751H MCU */ \
	ROM_SYSTEM_BIOS( 0, "de142d3", "DE 142 D3 - English UI, German questions, erotic on (code 01, 0x004D)" ) \
	ROMX_LOAD( "de_142_d3.bin", 0x0000, 0x1000, CRC(77be0b40) SHA1(113b5c239480a2259f55e411ba8fb3972e6d4301), ROM_BIOS(0) ) \
	ROM_SYSTEM_BIOS( 1, "en_off1", "English UI, German questions, erotic off (code 02, 0x0195)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code02_0195.bin", 0x0000, 0x1000, CRC(67c5dcc9) SHA1(7bc12ae26c17a1e7f03432ecfdbaa607189a5094) BAD_DUMP, ROM_BIOS(1) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 2, "en_on2", "English UI, German questions, erotic on (code 03, 0x011F)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code03_011f.bin", 0x0000, 0x1000, CRC(81fc7b0b) SHA1(6e4e47716befb7db9c1c1eb3244412f14a359682) BAD_DUMP, ROM_BIOS(2) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 3, "en_off2", "English UI, German questions, erotic off (code 04, 0x0234)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code04_0234.bin", 0x0000, 0x1000, CRC(4787aeaa) SHA1(30591fd6da9c45f613e2b2611234a8caf1cb6895) BAD_DUMP, ROM_BIOS(3) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 4, "en_on3", "English UI, German questions, erotic on (code 05, 0x03CC)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code05_03cc.bin", 0x0000, 0x1000, CRC(b5008f8b) SHA1(0e90c23e677b2ae91a6be8a3f0548cd2dabda67f) BAD_DUMP, ROM_BIOS(4) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 5, "en_off3", "English UI, German questions, erotic off (code 06, 0x017B)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code06_017b.bin", 0x0000, 0x1000, CRC(61d7f867) SHA1(dd981c3dcaa329ee9c619515744b2d3e5fa64bd1) BAD_DUMP, ROM_BIOS(5) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 6, "en_on4", "English UI, German questions, erotic on (code 07, 0x00DE)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code07_00de.bin", 0x0000, 0x1000, CRC(a98d4c59) SHA1(16c646347d0da86bc6e87014c8a7fb86749afb5c) BAD_DUMP, ROM_BIOS(6) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 7, "en_off4", "English UI, German questions, erotic off (code 08, 0x0177)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code08_0177.bin", 0x0000, 0x1000, CRC(45375b18) SHA1(f3b09d65aab174652b16399154e4f01a450bfe23) BAD_DUMP, ROM_BIOS(7) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 8, "en_on5", "English UI, German questions, erotic on (code 09, 0x0315)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code09_0315.bin", 0x0000, 0x1000, CRC(ae935263) SHA1(67b83a37d3b4ab700a1487409920ad2a7e36db2c) BAD_DUMP, ROM_BIOS(8) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 9, "it_on", "Italian UI, German questions, erotic on (code 10, 0x0180)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code10_0180.bin", 0x0000, 0x1000, CRC(7b2f9f6d) SHA1(a0ad7035d16ac2f790aed30751763de0fba5ff25) BAD_DUMP, ROM_BIOS(9) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 10, "fr_on", "French UI, German questions, erotic on (code 11, 0x0388)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code11_0388.bin", 0x0000, 0x1000, CRC(b7d7fa4f) SHA1(3474c2150995fa8c7c1e3ddc2ac99b66ebf2fe8d) BAD_DUMP, ROM_BIOS(10) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 11, "es_on", "Spanish UI, German questions, erotic on (code 12, 0x00C2)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code12_00c2.bin", 0x0000, 0x1000, CRC(fc139472) SHA1(35858407d41451000177aeaeeccc20d3c015b768) BAD_DUMP, ROM_BIOS(11) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 12, "gr_on1", "Greek UI, German questions, erotic on (code 13, 0x007B)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code13_007b.bin", 0x0000, 0x1000, CRC(1bf45423) SHA1(8f144fbe05a67deb8ee45dcdfc79d56c448aff42) BAD_DUMP, ROM_BIOS(12) ) /* AI-modified, needs proper dump */ \
	ROM_SYSTEM_BIOS( 13, "gr_on2", "Greek UI, German questions, erotic on (code 14, 0x02BA)" ) \
	ROMX_LOAD( "quizard4.0_mcu_code14_02ba.bin", 0x0000, 0x1000, CRC(bde197bd) SHA1(dc379f9fe708f2f518768e5600897358cdf42fe4) BAD_DUMP, ROM_BIOS(13) ) /* AI-modified, needs proper dump */

ROM_START( quizard4 ) /* CD-ROM printed 09/98 */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r42", 0, BAD_DUMP SHA1(a5d5c8950b4650b8753f9119dc7f1ccaa2aa5442) )

	QUIZARD4_42_MCU
ROM_END

ROM_START( quizard4cz ) /* CD-ROM printed 09/98 */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r42", 0, BAD_DUMP SHA1(a5d5c8950b4650b8753f9119dc7f1ccaa2aa5442) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "ts142_cz1.bin", 0x0000, 0x1000, CRC(fdc1f457) SHA1(5169c4d2ea4073a854c3f619205161386c9af8af) ) // Czech language - works with all Quizard 4 versions
ROM_END

ROM_START( quizard4_41 )
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r41", 0, BAD_DUMP SHA1(2c0484c6545aac8e00b318328c6edce6f5dde43d) )

	QUIZARD4_42_MCU
ROM_END

ROM_START( quizard4_40 ) /* CD-ROM printed 07/97 */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizard4r40", 0, BAD_DUMP SHA1(288cc37a994e4f1cbd47aa8c92342879c6fc0b87) )

	QUIZARD4_40_MCU
ROM_END

ROM_START( quizardff ) /* CD-ROM printed 01/96 */
	QUIZARD_BIOS_ROM

	DISK_REGION( "cdrom" )
	DISK_IMAGE_READONLY( "quizardff", 0, SHA1(ac533040379c1350066e778e3a86d1beb11c6f71) )

	ROM_REGION(0x1000, "mcu", 0) // Intel D8751H MCU
	ROM_LOAD( "quizard_french_special_mcu_027f.bin", 0x0000, 0x1000, CRC(4818aa47) SHA1(d6bd62717a2381e925d00f7c802d101bcc04123f) BAD_DUMP ) // AI-modified, needs proper dump
ROM_END


/*************************
*      Game driver(s)    *
*************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS      INIT        COMPANY       FULLNAME */
// BIOS / System
CONS( 1991, cdimono1, 0,      0,      cdimono1, cdi,      cdi_state, empty_init, "Philips",    "CD-i (Mono-I) (PAL)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
CONS( 1991, cdimono2, 0,      0,      cdimono2, cdimono2, cdi_state, empty_init, "Philips",    "CD-i (Mono-II) (NTSC)",   MACHINE_NOT_WORKING )
CONS( 1991, cdi910,   0,      0,      cdi910,   cdimono2, cdi_state, empty_init, "Philips",    "CD-i 910-17P Mini-MMC (PAL)",   MACHINE_NOT_WORKING )
CONS( 1991, cdi490a,  0,      0,      cdimono1, cdi,      cdi_state, empty_init, "Philips",    "CD-i 490",   MACHINE_NOT_WORKING )
CONS( 1995, gpi1200,  0,      0,      cdimono1, cdi,      cdi_state, empty_init, "Goldstar",   "GPi 1200",   MACHINE_NOT_WORKING )

// The Quizard games are retail CD-i units in a cabinet, with an additional JAMMA adapter and dongle for protection, hence being clones of the system.
/*    YEAR  NAME         PARENT    MACHINE        INPUT     DEVICE          INIT         MONITOR     COMPANY         FULLNAME */
GAME( 1995, cdibios,     0,        cdimono1,      quizard,  cdi_state,     empty_init,  ROT0,     "Philips",  "CD-i (Mono-I) (PAL) BIOS", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_GRAPHICS | MACHINE_IS_BIOS_ROOT )

GAME( 1995, quizard,     cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.8, German, i8751 DE 11 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard_17,  quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.7, German, i8751 DE 11 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard_12,  quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.2, German, i8751 DE 11 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard_10,  quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.0, German, i8751 DE 11 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizardi,    quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.8, Italian, i8751 IT 11 I2)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizardi_17, quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.7, Italian, i8751 IT 11 I2)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizardi_12, quizard,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard (v1.2, Italian, i8751 IT 11 I2)", MACHINE_IMPERFECT_SOUND )

GAME( 1995, quizard2,    cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 2 (v2.3, German, i8751 DN 122 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard2_22, quizard2, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 2 (v2.2, German, i8751 DN 122 D3)", MACHINE_IMPERFECT_SOUND )

// Quizard 3 and 4 will hang after starting a game (CDIC issues?)
GAME( 1995, quizard3,    cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 3 (v3.4, German, i8751 DE 132 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, quizard3a,   quizard3, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 3 (v3.4, German, i8751 DE 132 A1)", MACHINE_IMPERFECT_SOUND )
GAME( 1996, quizard3_32, quizard3, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 3 (v3.2, German, i8751 DE 132 D3)", MACHINE_IMPERFECT_SOUND )

GAME( 1998, quizard4,    cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 4 Rainbow (v4.2, German, i8751 DE 142 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, quizard4cz,  quizard4, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 4 Rainbow (v4.2, Czech, i8751 TS142 CZ1)", MACHINE_IMPERFECT_SOUND )
GAME( 1998, quizard4_41, quizard4, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 4 Rainbow (v4.1, German, i8751 DE 142 D3)", MACHINE_IMPERFECT_SOUND )
GAME( 1997, quizard4_40, quizard4, quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard 4 Rainbow (v4.0, German, i8751 DE 142 D3)", MACHINE_IMPERFECT_SOUND )

GAME( 1996, quizardff,   cdibios,  quizard,       quizard,  quizard_state, empty_init,  ROT0, "TAB Austria",  "Quizard Fun and Fascination (French Edition V1 - 01/96)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )

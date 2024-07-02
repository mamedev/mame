// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, James Wallace
/***************************************************************************

    Nova Laserdisc Games/Atari Games COPS
    (hardware developed by Nova Productions Limited, should we move it from Atari?)
    Preliminary driver by Mariusz Wojcieszek, James Wallace

    Cops uses a Sony CD-ROM in addition to the regular setup, purely to play
    Bad Boys by Inner Circle, so there is musical accompaniment to areas
    where the laserdisc audio is muted.

    The different games here have subtly different control PCBs. COPS has an Atari
    part number (58-12B), while Revelations simply refers to a Lasermax control PCB
    (Lasermax being the consumer name for the LDP series).
	COPS, Street Viper and all other Nova Productions laserdisc games run on forms of this
	hardware.

    NOTES: To init NVRAM on Revelations at first boot, turn the refill key (R) and press any button.
    A and B adjust date/time (clock is not Y2K compliant), C selects.
    Once the clock is running, press Collect or Continue.
    Let it boot up to REFILL mode, and then turn the refill key off.

    TODO: There are probably more ROMs for Revelations and related, the disc
    contains full data for a non-payout US release of the game called 'Vision Quest'.
    However, the Vision Quest Laserdisc for the USA is slightly different, with 
    Revelations specific data seemingly replaced with black level.
	We have a disc image for Vision Quest in full, but no ROM

	BTANB for Revelations:
	Game options cannot be adjusted, any attempt to do so resets the machine (seen on real hardware)
	Volume control cycles in the 'Stereo' test, but cannot be tested.


    This should be similar hardware for Street Viper if we get a dump.
***************************************************************************/


#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/ldp1450hle.h"
#include "machine/r65c52.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "sound/sn76496.h"
#include "machine/watchdog.h"

#include "machine/bacta_datalogger.h"
#include "machine/meters.h"

#include "speaker.h"

#include "cops.lh"
#include "revlatns.lh"

#define LOG_CDROM   (1U << 1)

#define VERBOSE (LOG_CDROM)
#include "logmacro.h"


namespace {

#define MAIN_CLOCK 4_MHz_XTAL
#define DACIA_CLOCK 3.6864_MHz_XTAL

class cops_state : public driver_device
{
public:
	cops_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_sn(*this, "snsnd")
		, m_ld(*this, "laserdisc")
		, m_dacia(*this, "dacia")
		, m_watchdog(*this, "watchdog")
		, m_meters(*this, "meters")
		, m_switches(*this, "SW%u", 0U)
		, m_steer(*this, "STEER")
		, m_digits(*this, "digit%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
		, m_irq(0)
		, m_acia1_irq(0)
		, m_acia2_irq(0)
	{ }

	void revlatns(machine_config &config);
	void base(machine_config &config);
	void cops(machine_config &config);
	void cops_map(address_map &map);
	void revlatns_map(address_map &map);

	void init_cops();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_nvram;
	required_device<sn76489_device> m_sn;
	required_device<sony_ldp1450hle_device> m_ld;
   	required_device<r65c52_device> m_dacia;
    required_device<watchdog_timer_device> m_watchdog;
	optional_device<meters_device> m_meters;

	required_ioport_array<3> m_switches;
	optional_ioport m_steer;
	output_finder<16> m_digits;
	output_finder<23> m_lamps;
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io1_cops_w(offs_t offset, uint8_t data);
	void io1_w(offs_t offset, uint8_t data);
	uint8_t io1_cops_r(offs_t offset);
	uint8_t io1_r(offs_t offset);
	void io2_w(offs_t offset, uint8_t data);
	uint8_t io2_r(offs_t offset);
	void acia1_irq(int state);
	void acia2_irq(int state);
	void dacia_irq();
	void via1_irq(int state);
	void via2_irq(int state);
	void via1_a_w(uint8_t data);
	void via1_a_revlatns_w(uint8_t data);
	void via1_b_w(uint8_t data);
	void via1_cb1_w(uint8_t data);
	void cdrom_data_w(uint8_t data);
	void cdrom_ctrl_w(uint8_t data);
	uint8_t cdrom_data_r();
	int m_irq, m_acia1_irq, m_acia2_irq;

	uint8_t m_lcd_addr_l, m_lcd_addr_h;
	uint8_t m_lcd_data_l, m_lcd_data_h;

	uint8_t m_cdrom_ctrl;
	uint8_t m_cdrom_data;

	uint8_t m_sn_data;
	uint8_t m_sn_cb1;
};

void cops_state::video_start()
{
}

uint32_t cops_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/*************************************
 *
 * Sony CDU33A-02 CDROM
 *
 *************************************/

void cops_state::cdrom_data_w(uint8_t data)
{
	const char *regs[4] = { "CMD", "PARAM", "WRITE", "CTRL" };
	m_cdrom_data = bitswap<8>(data,0,1,2,3,4,5,6,7);
	uint8_t reg = ((m_cdrom_ctrl & 4) >> 1) | ((m_cdrom_ctrl & 8) >> 3);
	LOGMASKED(LOG_CDROM, "%s:cdrom_data_w(reg = %s, data = %02x)\n", machine().describe_context(), regs[reg & 0x03], m_cdrom_data);
}

void cops_state::cdrom_ctrl_w(uint8_t data)
{
	LOGMASKED(LOG_CDROM, "%s:cdrom_ctrl_w(%02x)\n", machine().describe_context(), data);
	m_cdrom_ctrl = data;
}

uint8_t cops_state::cdrom_data_r()
{
	const char *regs[4] = { "STATUS", "RESULT", "READ", "FIFOST" };
	uint8_t reg = ((m_cdrom_ctrl & 4) >> 1) | ((m_cdrom_ctrl & 8) >> 3);
	LOGMASKED(LOG_CDROM, "%s:cdrom_data_r(reg = %s)\n", machine().describe_context(), regs[reg & 0x03]);
	return machine().rand()&0xff;
}

/*************************************
 *
 * 6552 DACIA - IRQs are inverted
 *
 *************************************/

void cops_state::acia1_irq(int state)
{
	m_acia1_irq=!state;
	dacia_irq();
}

void cops_state::acia2_irq(int state)
{
	m_acia2_irq=!state;
	dacia_irq();
}

void cops_state::dacia_irq()
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, m_acia1_irq | m_acia2_irq? ASSERT_LINE:CLEAR_LINE);
}

/*************************************
 *
 * I/O
 *
 *************************************/

uint8_t cops_state::io1_cops_r(offs_t offset)
{
	switch( offset & 0x0f )
	{
		case 0x07: /* WOP7 - watchdog*/
			return 1;
		case 0x08:  /* SW0 */
			return m_switches[0]->read();
		case 0x09:  /* SW1 */
			return m_switches[1]->read();
		case 0x0a:  /* SW2 */
			return m_switches[2]->read();
		default:
			logerror("Unknown io1_r, offset = %03x\n", offset);
			return 0;
	}
}

uint8_t cops_state::io1_r(offs_t offset)
{
	switch( offset & 0x0f )
	{
		case 0x01:  /* SW0 */
			return m_switches[0]->read();
		case 0x07: /* WOP7 - watchdog*/
			return 1;
		case 0x08:  /* SW0 */
			return m_switches[0]->read();
		case 0x09:  /* SW1 */
			return m_switches[1]->read();
		case 0x0a:  /* SW2 */
			return m_switches[2]->read();
		default:
			logerror("Unknown io1_r, offset = %03x\n", offset);
			return 0;
	}
}

void cops_state::io1_cops_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
		case 0x00: /* WOP0 Alpha display*/
			m_lcd_addr_l = data;
			break;
		case 0x01: /* WOP1 Alpha display*/
			m_lcd_addr_h = data;
			{
				// update display
				constexpr uint16_t addrs_table[] = {
						0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0002, 0x0001, 0x0080,
						0x1000, 0x0800, 0x0400, 0x2000, 0x4000, 0x0200, 0x0100, 0x8000 };
				const uint16_t addr = m_lcd_addr_l | (m_lcd_addr_h << 8);
				for (int i = 0; i < 16; i++)
				{
					if (addr == addrs_table[i])
					{
						const uint16_t display_data = m_lcd_data_l | (m_lcd_data_h << 8);
						m_digits[i] = bitswap<16>(display_data, 4, 5, 12, 1, 0, 11, 10, 6, 7, 2, 9, 3, 15, 8, 14, 13);
						break;
					}
				}
			}
			break;
		case 0x02: /* WOP2 Alpha display*/
			m_lcd_data_l = data;
			break;
		case 0x03: /* WOP3 Alpha display*/
			m_lcd_data_h = data;
			break;
		case 0x04: /* WOP4 */
			for (int i = 3; i >= 0; i--)
				m_lamps[i + 0x4] = BIT(data, i + 4); //Offroad right 1 - 4
			for (int i = 3; i >= 0; i--)
				m_lamps[i] = BIT(data, i); // Offroad left 1 - 4
			break;
		case 0x05: /* WOP5 */
			m_lamps[0xe] = BIT(data, 7); // Damage
			m_lamps[0xd] = BIT(data, 6); // Stop
			m_lamps[0xc] = BIT(data, 5); // Gun active right
			m_lamps[0x9] = BIT(data, 4); // Vest 2
			m_lamps[0xa] = BIT(data, 2); // Vest 3
			m_lamps[0xb] = BIT(data, 1); // Gun active left
			m_lamps[0x8] = BIT(data, 0); // Vest 1
			break;
		case 0x06: /* WOP6 */
			logerror("WOP6: data = %02x\n", data);
			break;
		case 0x07: /* WOP7 - watchdog*/
			m_watchdog->reset_w(data);
			break;
		default:
			logerror("Unknown io1_w, offset = %03x, data = %02x\n", offset, data);
			break;
	}
}


void cops_state::io1_w(offs_t offset, uint8_t data)
{
	switch (offset & 0x0f)
	{
		case 0x00: /* WOP0 Alpha display*/
			m_lcd_addr_l = data;
			break;
		case 0x01: /* WOP1 Alpha display*/
			m_lcd_addr_h = data;
			{
				// update display
				constexpr uint16_t addrs_table[] = {
						0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0002, 0x0001, 0x0080,
						0x1000, 0x0800, 0x0400, 0x2000, 0x4000, 0x0200, 0x0100, 0x8000 };
				const uint16_t addr = m_lcd_addr_l | (m_lcd_addr_h << 8);
				for (int i = 0; i < 16; i++)
				{
					if (addr == addrs_table[i])
					{
						const uint16_t display_data = m_lcd_data_l | (m_lcd_data_h << 8);
						m_digits[i] = bitswap<16>(display_data, 4, 5, 12, 1, 0, 11, 10, 6, 7, 2, 9, 3, 15, 8, 14, 13);
						break;
					}
				}
			}
			break;
		case 0x02: /* WOP2 Alpha display*/
			m_lcd_data_l = data;
			break;
		case 0x03: /* WOP3 Alpha display*/
			m_lcd_data_h = data;
			break;
		case 0x04: /* WOP4 */
			/*
			0 20p
			1 40p
			2 £1
			3 £2
			4 £5
			5 £10
			6 £20
			7 Win lamp
			*/

			for (int i = 0; i < 8; i++)
				m_lamps[i] = BIT(data, i);

			break;
		case 0x05: /* WOP5 */

			m_lamps[0x8] = BIT(data, 0); // A
			m_lamps[0x9] = BIT(data, 1); // B
			m_lamps[0xa] = BIT(data, 2); // Collect

			m_meters->update(0, BIT(data, 3));

			m_lamps[0xb] = BIT(data, 4); // C
			m_lamps[0xc] = BIT(data, 5); // Continue

			m_meters->update(1, BIT(data, 6));

			m_lamps[0xd] = BIT(data, 7); // *

			/*
			0 A lamp
			1 B lamp
			2 Collect lamp
			3 Cash in Meter
			4 C lamp
			5 Continue lamp
			6 Cash out meter
			7 '*' lamp
			*/

			// logerror("WOP5 (lamps): data = %02x\n", data);
			break;
		case 0x06: /* WOP6 - Not connected in Revelations, at least*/
			logerror("WOP6: data = %02x\n", data);
			break;
		case 0x07: /* WOP7 - watchdog*/
			m_watchdog->reset_w(data);
			break;
		default:
			logerror("Unknown io1_w, offset = %03x, data = %02x\n", offset, data);
			break;
	}
}

uint8_t cops_state::io2_r(offs_t offset)
{
	switch( offset & 0x0f )
	{
		case 0x03:
			return m_steer->read();
		default:
			logerror("Unknown io2_r, offset = %02x\n", offset);
			return 0;
	}
}

void cops_state::io2_w(offs_t offset, uint8_t data)
{
	switch( offset & 0x0f )
	{
		case 0x02:
			m_lamps[0xf] = BIT(data, 0); // Flash red
			m_lamps[0x10] = BIT(data, 7); // Flash blue
			// Any other I/O here?
			break;
		case 0x04:
			for (int i = 5; i >= 0; i--)
				m_lamps[0x11 + i] = BIT(data, i); // bullet
			break;
		default:
			break;
	}
}

/*************************************
 *
 *  VIA 1 (U18)
 *   PA0-2 Steer
 *   PA3   Shake motor?
 *   PA4-6 Fade?
 *   PA7   STK (system rom banking A13)
 *   PB0-7 SN76489 data bus
 *   CA1-2 n.c.
 *   CB1   /WE SN76489
 *   IRQ   IRQ
 *
 *************************************/


void cops_state::via1_irq(int state)
{
	if ( state == ASSERT_LINE )
	{
		m_irq |= 1;
	}
	else
	{
		m_irq &= ~1;
	}
	m_maincpu->set_input_line(M6502_IRQ_LINE, m_irq ? ASSERT_LINE : CLEAR_LINE);
}

void cops_state::via1_a_w(uint8_t data)
{
}

void cops_state::via1_b_w(uint8_t data)
{
	m_sn_data = bitswap<8>(data,0,1,2,3,4,5,6,7);
	if (m_sn_cb1)
	{
		m_sn->write(m_sn_data);
	}
}

void cops_state::via1_cb1_w(uint8_t data)
{
	m_sn_cb1 = data;
}

/*************************************
 *
 *  VIA 2 (U27)
 *   PA0-7 GUN
 *   PB0-7 GUN
 *   IRQ   IRQ
 *
 *************************************/

void cops_state::via2_irq(int state)
{
	if ( state == ASSERT_LINE )
	{
		m_irq |= 2;
	}
	else
	{
		m_irq &= ~2;
	}
	m_maincpu->set_input_line(M6502_IRQ_LINE, m_irq ? ASSERT_LINE : CLEAR_LINE);
}


/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void cops_state::cops_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share(m_nvram);
	map(0x2000, 0x9fff).rom().region("program", 0);
	map(0xa000, 0xafff).rw(FUNC(cops_state::io1_cops_r), FUNC(cops_state::io1_cops_w));
	map(0xb000, 0xb00f).m("via6522_1", FUNC(via6522_device::map));  /* VIA 1 */
	map(0xb800, 0xb80f).m("via6522_2", FUNC(via6522_device::map));  /* VIA 2 */
	map(0xc000, 0xcfff).rw(FUNC(cops_state::io2_r), FUNC(cops_state::io2_w));
	map(0xd000, 0xd007).m(m_dacia, FUNC(r65c52_device::map));
	map(0xd800, 0xd80f).m("via6522_3", FUNC(via6522_device::map));  /* VIA 3 */
	map(0xe000, 0xffff).bankr("sysbank1");
}

void cops_state::revlatns_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share(m_nvram);
	map(0x2000, 0x9fff).rom().region("program", 0);
	map(0xa000, 0xafff).rw(FUNC(cops_state::io1_r), FUNC(cops_state::io1_w));
	map(0xb000, 0xb00f).m("via6522_1", FUNC(via6522_device::map));  /* VIA 1 */
	map(0xc000, 0xc00f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	map(0xd000, 0xd007).m(m_dacia, FUNC(r65c52_device::map));
	map(0xe000, 0xffff).bankr("sysbank1");
}

static INPUT_PORTS_START( cops )
	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Switch A") PORT_CODE(KEYCODE_A) PORT_IMPULSE(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Switch C") PORT_CODE(KEYCODE_C) PORT_IMPULSE(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("PROGRAM") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Switch B") PORT_CODE(KEYCODE_B) PORT_IMPULSE(1)

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) // N.C.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Gas pedal
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("20P LEVEL") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x38, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("100P LEVEL") PORT_CODE(KEYCODE_W)

	PORT_START("SW2") //GUN?
	PORT_BIT( 0x7f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) //Left floating, games will fail to boot if this is low

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)
INPUT_PORTS_END

static INPUT_PORTS_START( revlatns )
	PORT_START("SW0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("A")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("C")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("COLLECT")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN5 ) //COIN5
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_IMPULSE(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 ) // COIN6
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Continue")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("B")

	PORT_START("SW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Refill Key") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_INTERLOCK) PORT_NAME("Cashbox Door") PORT_CODE(KEYCODE_Q) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )  // 20p level
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("*")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_INTERLOCK) PORT_NAME("Back Door") PORT_CODE(KEYCODE_W) PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) // £1 level

	PORT_START("SW2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("50p")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("20p")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("10p")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_NAME("100p")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_CUSTOM )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) //Left floating, games will fail to boot if this is low
INPUT_PORTS_END

void cops_state::machine_start()
{
	m_digits.resolve();
	m_lamps.resolve();
}

void cops_state::machine_reset()
{
	m_irq = 0;
	m_lcd_addr_l = m_lcd_addr_h = 0;
	m_lcd_data_l = m_lcd_data_h = 0;
	}


void cops_state::init_cops()
{
	//The hardware is designed and programmed to use multiple system ROM banks, but for some reason it's hardwired to bank 2.
	//For documentation's sake, here's the init
	uint8_t *rom = memregion("system")->base();
	membank("sysbank1")->configure_entries(0, 4, &rom[0x0000], 0x2000);
	membank("sysbank1")->set_entry(2);
}

void cops_state::base(machine_config &config)
{
	M6502(config, m_maincpu, MAIN_CLOCK/2/2); // fed through two dividers

	SONY_LDP1450HLE(config, m_ld, 0);
	m_ld->set_screen("screen");
	m_ld->set_overlay(256, 256, FUNC(cops_state::screen_update));
	m_ld->add_route(0, "lspeaker", 0.50);
	m_ld->add_route(1, "rspeaker", 0.50);
	m_ld->set_baud(9600);
	m_ld->add_ntsc_screen(config, "screen");
	m_ld->serial_tx().set("dacia", FUNC(r65c52_device::write_rxd1));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SPEAKER(config, "lspeaker").front_left();

	SPEAKER(config, "mspeaker").front_center();

	SPEAKER(config, "rspeaker").front_right();

	R65C52(config, m_dacia, DACIA_CLOCK);
	m_dacia->txd1_handler().set("laserdisc", FUNC(sony_ldp1450hle_device::rx_w));
	m_dacia->irq1_handler().set(FUNC(cops_state::acia1_irq));
	m_dacia->irq2_handler().set(FUNC(cops_state::acia2_irq));

	SN76489(config, m_sn, MAIN_CLOCK/2);
	m_sn->add_route(ALL_OUTPUTS, "mspeaker", 0.30);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_msec(1600));
}

void cops_state::cops(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &cops_state::cops_map);

	/* via */
	via6522_device &via1(MOS6522(config, "via6522_1", MAIN_CLOCK/4));
	via1.irq_handler().set(FUNC(cops_state::via1_irq));
	via1.writepa_handler().set(FUNC(cops_state::via1_a_w));
	via1.writepb_handler().set(FUNC(cops_state::via1_b_w));
	via1.cb1_handler().set(FUNC(cops_state::via1_cb1_w));

	via6522_device &via2(MOS6522(config, "via6522_2", MAIN_CLOCK/4));
	via2.irq_handler().set(FUNC(cops_state::via2_irq));

	via6522_device &via3(MOS6522(config, "via6522_3", MAIN_CLOCK/4));
	via3.readpa_handler().set(FUNC(cops_state::cdrom_data_r));
	via3.writepa_handler().set(FUNC(cops_state::cdrom_data_w));
	via3.writepb_handler().set(FUNC(cops_state::cdrom_ctrl_w));
}

void cops_state::revlatns(machine_config &config)
{
	base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &cops_state::revlatns_map);


	/* via */
	via6522_device &via1(MOS6522(config, "via6522_1", MAIN_CLOCK/4));
	via1.irq_handler().set(FUNC(cops_state::via1_irq));
	via1.writepb_handler().set(FUNC(cops_state::via1_b_w));
	via1.cb1_handler().set(FUNC(cops_state::via1_cb1_w));

	METERS(config, m_meters, 0).set_number(2);

	bacta_datalogger_device &bacta(BACTA_DATALOGGER(config, "bacta", 0));

	m_dacia->txd1_handler().set("laserdisc", FUNC(sony_ldp1450hle_device::rx_w));
	m_dacia->txd2_handler().set("bacta", FUNC(bacta_datalogger_device::write_txd));

	bacta.rxd_handler().set("dacia", FUNC(r65c52_device::write_rxd2));

	MSM6242(config, "rtc", 32.768_kHz_XTAL);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cops )
	ROM_REGION( 0x8000, "program", 0 )
	ROM_LOAD( "cops_prg.dat", 0x0000, 0x8000, CRC(a5c02366) SHA1(b135d72fcfe737a113c984b0b8dd78428f248414) )

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "cops_sys.dat", 0x0000, 0x8000, CRC(0060e5d0) SHA1(b8c9f6fde6a315e33fa7946e5d3bb4ea2fbe76a8) )

	DISK_REGION( "audiocd" )
	DISK_IMAGE_READONLY( "copscd", 0, NO_DUMP )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "copsld", 0, NO_DUMP )
ROM_END

ROM_START( copsuk )
	ROM_REGION( 0x8000, "program", 0 )
	ROM_LOAD( "cops1b_uk.bin", 0x0000, 0x8000, CRC(f095ee95) SHA1(86bb517331d81ae3a8f3b87df67c321013c6aae4) )

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "cops_sys.dat", 0x0000, 0x8000, CRC(0060e5d0) SHA1(b8c9f6fde6a315e33fa7946e5d3bb4ea2fbe76a8) )

	DISK_REGION( "audiocd" )
	DISK_IMAGE_READONLY( "copscd", 0, NO_DUMP )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "copsld", 0, NO_DUMP )
ROM_END

ROM_START( revlatns )
	ROM_REGION( 0x8000, "program", 0 )
	ROM_LOAD( "revelations_prog.u31", 0x0000, 0x8000, CRC(5ab41ac3) SHA1(0f7027551da17011576cf077e2f199729bb10482) )

	ROM_REGION( 0x8000, "system", 0 )
	ROM_LOAD( "revelations_sys.u17", 0x0000, 0x8000, CRC(43e5e3ec) SHA1(fa44b102b5aa7ad2421c575abdc67f1c29f23bc1) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "nova dp1-3a", 0, BAD_DUMP SHA1(f69c6a3def1e1eec0a58862c487e47d4da12b25e))  //one disc, no correction, old method
ROM_END

} // Anonymous namespace


GAMEL( 1994, cops,     0,    cops,     cops,     cops_state, init_cops, ROT0, "Atari Games",                           "Cops (USA)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_cops )
GAMEL( 1994, copsuk,   cops, cops,     cops,     cops_state, init_cops, ROT0, "Nova Productions Ltd./ Deith Leisure",  "Cops (UK)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND, layout_cops )
GAMEL( 1991, revlatns, 0,    revlatns, revlatns, cops_state, init_cops, ROT0, "Nova Productions Ltd.",                 "Revelations", MACHINE_SUPPORTS_SAVE, layout_revlatns ) 

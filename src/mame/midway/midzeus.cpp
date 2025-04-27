// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Driver for Midway Zeus games

    driver by Aaron Giles

    Games supported:
        * Invasion - The Abductors
        * Mortal Kombat 4
        * Cruis'n Exotica
        * The Grid

    Known bugs:
        * not done yet

According to a Midway service bulletin
As of 2/12/2001 the latest software levels:

Game Title       Level  Released
----------------------------------
Cruis'n Exotica  v2.4   08/23/2000
Invasion         v5.0   12/14/1999
The Grid         v1.2   10/18/2000

**************************************************************************/

#include "emu.h"

#include "midzeus.h"

#include "dcs.h"

#include "cpu/tms32031/tms32031.h"
#include "cpu/adsp2100/adsp2100.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "machine/ibm21s850.h"
#include "machine/nvram.h"
#include "machine/tsb12lv01a.h"
#include "video/zeus2.h"

#include "speaker.h"

#include "crusnexo.lh"

static constexpr int BEAM_DY = 3;
static constexpr int BEAM_DX = 3;
static constexpr int BEAM_XOFFS = 40; // table in the code indicates an offset of 20 with a beam height of 7

#define LOG_FIREWIRE    (1U << 1)
#define LOG_DISK        (1U << 2)
#define LOG_DISK_JR     (1U << 3)
#define LOG_TMS32032    (1U << 4)
#define LOG_INPUT       (1U << 5)
#define LOG_CMOS        (1U << 6)
#define LOG_UNKNOWN     (1U << 7)

#define VERBOSE (LOG_FIREWIRE)
#include "logmacro.h"

/*************************************************************************
Driver for Midway Zeus2 games
**************************************************************************/

class midzeus2_state : public midzeus_state
{
protected:
	midzeus2_state(const machine_config &mconfig, device_type type, const char *tag)
		: midzeus_state(mconfig, type, tag)
		, m_zeus(*this, "zeus2")
		, m_m48t35(*this, "m48t35")
		, m_fw_link(*this, "fw_link")
		, m_fw_phy(*this, "fw_phy")
		, m_io_keypad(*this, "KEYPAD")
	{ }

	virtual void machine_start() override
	{
		midzeus_state::machine_start();

		m_mainbank->configure_entries(0, 3, memregion("bankeddata")->base(), 0x400000*4);
		m_mainbank->set_entry(0);

		save_item(NAME(m_disk_asic));
		save_item(NAME(m_fw_int_enable));
		save_item(NAME(m_fw_int));
	}

	virtual void machine_reset() override
	{
		midzeus_state::machine_reset();

		memset(m_disk_asic, 0x0, 0x10 * 4);
		m_fw_int_enable = 0;
		m_fw_int = 0;
	}

	virtual void video_start() override {}

	uint32_t disk_asic_r(offs_t offset);
	void disk_asic_w(offs_t offset, uint32_t data);

	void firewire_irq(int state);
	void zeus_irq(int state);

	uint32_t zpram_r(offs_t offset);
	void zpram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t timekeeper_r(offs_t offset);
	void timekeeper_w(offs_t offset, uint32_t data);

	void update_firewire_irq();

	void zeus2_map(address_map &map) ATTR_COLD;
	void midzeus2(machine_config &config);

	uint32_t    m_disk_asic[0x10]{};
	int         m_fw_int_enable = 0;
	int         m_fw_int = 0;

	required_device<zeus2_device> m_zeus;
	required_device<timekeeper_device> m_m48t35;
	required_device<tsb12lv01a_device> m_fw_link;
	required_device<ibm21s851_device> m_fw_phy;

	required_ioport m_io_keypad;
};


class crusnexo_state : public midzeus2_state
{
public:
	crusnexo_state(const machine_config &mconfig, device_type type, const char *tag)
		: midzeus2_state(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_lamps(*this, "lamp%u", 0U)
		, m_io_analog(*this, "ANALOG%u", 0U)
	{ }

	void crusnexo(machine_config &config);

	ioport_value keypad_r();

protected:
	virtual void machine_start() override
	{
		midzeus2_state::machine_start();

		m_digits.resolve();
		m_leds.resolve();
		m_lamps.resolve();

		save_item(NAME(m_keypad_select));
		save_item(NAME(m_crusnexo_leds_select));
	}

private:
	uint32_t crusnexo_leds_r(offs_t offset);
	void crusnexo_leds_w(offs_t offset, uint32_t data);
	void keypad_select_w(offs_t offset, uint32_t data);
	uint32_t analog_r(offs_t offset);
	void analog_w(uint32_t data);

	void crusnexo_map(address_map &map) ATTR_COLD;

	uint8_t     m_keypad_select = 0;
	uint8_t     m_crusnexo_leds_select = 0;

	output_finder<7> m_digits;
	output_finder<32> m_leds;
	output_finder<8> m_lamps;
	required_ioport_array<4> m_io_analog;
};

class thegrid_state : public midzeus2_state
{
public:
	thegrid_state(const machine_config &mconfig, device_type type, const char *tag)
		: midzeus2_state(mconfig, type, tag)
		, m_io_49way_x(*this, "49WAYX")
		, m_io_49way_y(*this, "49WAYY")
		, m_io_trackx(*this, "TRACKX")
		, m_io_tracky(*this, "TRACKY")
	{ }

	void thegrid(machine_config &config);

	ioport_value custom_49way_r();

private:
	uint32_t trackball_r(offs_t offset);
	uint32_t grid_keypad_r(offs_t offset);

	void thegrid_map(address_map &map) ATTR_COLD;

	required_ioport m_io_49way_x;
	required_ioport m_io_49way_y;
	required_ioport m_io_trackx;
	required_ioport m_io_tracky;
};




/*************************************
 *
 *  Machine init
 *
 *************************************/

void midzeus_state::machine_start()
{
	m_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate());
	m_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate());

	m_display_irq_off_timer = timer_alloc(FUNC(midzeus_state::display_irq_off), this);

	save_item(NAME(m_disk_asic_jr));
	save_item(NAME(m_cmos_protected));
}

void invasnab_state::machine_start()
{
	midzeus_state::machine_start();

	m_gun_timer[0] = timer_alloc(FUNC(invasnab_state::invasn_gun_callback), this);
	m_gun_timer[1] = timer_alloc(FUNC(invasnab_state::invasn_gun_callback), this);

	save_item(NAME(m_gun_control));
	save_item(NAME(m_gun_irq_state));
	save_item(NAME(m_gun_x));
	save_item(NAME(m_gun_y));
}


void midzeus_state::machine_reset()
{
	memcpy(m_ram_base, memregion("maindata")->base(), 0x40000*4);
	*m_ram_base <<= 1;
	m_maincpu->reset();

	m_cmos_protected = true;
	memset(m_disk_asic_jr, 0x0, 0x10 * 4);
	m_disk_asic_jr[6] = 0xa0; // Rev3 Athens
}



/*************************************
 *
 *  Display interrupt generation
 *
 *************************************/

TIMER_CALLBACK_MEMBER(midzeus_state::display_irq_off)
{
	m_maincpu->set_input_line(TMS3203X_IRQ0, CLEAR_LINE);
}

INTERRUPT_GEN_MEMBER(midzeus_state::display_irq)
{
	m_maincpu->set_input_line(TMS3203X_IRQ0, ASSERT_LINE);
	m_display_irq_off_timer->adjust(attotime::from_hz(30000000));
}

void midzeus2_state::zeus_irq(int state)
{
	m_maincpu->set_input_line(TMS3203X_IRQ2, ASSERT_LINE);
}


/*************************************
 *
 *  CMOS access (Zeus only)
 *
 *************************************/

void midzeus_state::cmos_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_disk_asic_jr[2] && !m_cmos_protected)
		COMBINE_DATA(&m_nvram[offset]);
	else
		LOGMASKED(LOG_CMOS, "%06X:cmos_w with disk_asic_jr[2] = %d, cmos_protected = %d\n", m_maincpu->pc(), m_disk_asic_jr[2], m_cmos_protected);
	m_cmos_protected = true;
}


uint32_t midzeus_state::cmos_r(offs_t offset)
{
	return m_nvram[offset] | 0xffffff00;
}


void midzeus_state::cmos_protect_w(uint32_t data)
{
	m_cmos_protected = false;
}



/*************************************
 *
 *  Timekeeper and ZPRAM access
 *  (Zeus 2 only)
 *
 *************************************/

uint32_t midzeus2_state::timekeeper_r(offs_t offset)
{
	return m_m48t35->read(offset) | 0xffffff00;
}

void midzeus2_state::timekeeper_w(offs_t offset, uint32_t data)
{
	if (m_disk_asic_jr[2] && !m_cmos_protected)
		m_m48t35->write(offset, data);
	else
		LOGMASKED(LOG_CMOS, "%s:timekeeper_w with disk_asic_jr[2] = %d, cmos_protected = %d\n", machine().describe_context(), m_disk_asic_jr[2], m_cmos_protected);
	m_cmos_protected = true;
}


uint32_t midzeus2_state::zpram_r(offs_t offset)
{
	return m_nvram[offset] | 0xffffff00;
}


void midzeus2_state::zpram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (m_disk_asic_jr[2])
		COMBINE_DATA(&m_nvram[offset]);
	else
		LOGMASKED(LOG_CMOS, "%06X:zpram_w with disk_asic_jr[2] = %d\n", m_maincpu->pc(), m_disk_asic_jr[2]);
}



/*************************************
*
*  Disk ASIC registers
*
*************************************/

uint32_t midzeus2_state::disk_asic_r(offs_t offset)
{
	uint32_t retVal = m_disk_asic[offset];
	switch (offset)
	{
		// Sys Control
		// Bit 0: zeus reset
		// Bit 3: io reset
		// Bit 15:8 Version
		case 0:
			retVal = (retVal & 0xffff00ff) | 0x3300;
			LOGMASKED(LOG_DISK, "%s: Disk ASIC System Control Read: %08x\n", machine().describe_context(), retVal);
			break;
		// Interrupt Status / Control
		// 0x004: IFIFO Interrupt
		// 0x200: Firewire
		case 1:
			retVal = (m_fw_int << 9);
			LOGMASKED(LOG_DISK, "%s: Disk ASIC Interrupt Status/Ctrl Read: %08x\n", machine().describe_context(), retVal);
			break;
		// Test
		case 2:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC Test(?) Read: %08x\n", machine().describe_context(), retVal);
			break;
		// Wait State Config
		case 3:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC Wait State Config Read: %08x\n", machine().describe_context(), retVal);
			break;
		// IDE Config
		case 4:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC IDE Config Read: %08x\n", machine().describe_context(), retVal);
			break;
		// PSRAM Config
		case 5:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC PSRAM Config Read: %08x\n", machine().describe_context(), retVal);
			break;
		// Unknown
		default:
			LOGMASKED(LOG_DISK | LOG_UNKNOWN, "%s: Disk ASIC Unknown Read: %08x\n", machine().describe_context(), offset);
			break;
	}
	return retVal;
}


void midzeus2_state::disk_asic_w(offs_t offset, uint32_t data)
{
	m_disk_asic[offset] = data;

	switch (offset)
	{
		// Sys Control
		// Bit 0: zeus reset
		// Bit 3: io reset
		case 0:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC System Control Write: %08x\n", machine().describe_context(), data);
			break;
		// Interrupt Status / Control
		// 0x004: IFIFO Interrupt
		// 0x200: Enable Firewire interrupt
		case 1:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC Interrupt Status/Ctrl Write: %08x\n", machine().describe_context(), data);
			m_fw_int_enable = BIT(data, 9);
			update_firewire_irq();
			break;
		// Test
		case 2:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC Test(?) Write: %08x\n", machine().describe_context(), data);
			break;
		// Wait State Config
		case 3:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC Wait State Config Write: %08x\n", machine().describe_context(), data);
			break;
		// IDE Config
		case 4:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC IDE Config Write: %08x\n", machine().describe_context(), data);
			break;
		// PSRAM Config
		case 5:
			LOGMASKED(LOG_DISK, "%s: Disk ASIC PSRAM Config Write: %08x\n", machine().describe_context(), data);
			break;
		// Unknown
		default:
			LOGMASKED(LOG_DISK | LOG_UNKNOWN, "%s: Disk ASIC Unknown Write: %08x = %08x\n", machine().describe_context(), offset, data);
			break;
	}
}

/*************************************
 *
 *  Disk ASIC JR registers
 *
 *************************************/

uint32_t midzeus_state::disk_asic_jr_r(offs_t offset)
{
	uint32_t retVal = m_disk_asic_jr[offset];
	switch (offset)
	{
		// miscellaneous hw wait states
		case 1:
			break;

		// CMOS/ZPRAM write enable; only low bit is used
		case 2:
			//return m_disk_asic_jr[offset] | ~1;
			break;

		// reset status; bit 0 is watchdog reset; mk4/invasn/thegrid read at startup; invasn freaks if it is 1 at startup
		case 3:
			//return m_disk_asic_jr[offset] | ~1;
			break;

		// ROM bank selection on Zeus 2; two bits are used
		case 5:
			//return m_disk_asic_jr[offset] | ~3;
			break;

		// disk asic jr id; crusnexo reads at startup: if (val & 0xf0) == 0xa0 it affects
		// how the Zeus is used (reg 0x5d is set to 0x54580006)
		// thegrid does the same, writing either 0xD4580006 or 0xC4180006 depending
		// this is the value reported as DISK JR ASIC version in thegrid startup test
		// Set in reset
		// a0 = Rev3 Athens
		// 90 = Rev2 Athens
		case 6:
			break;

		// unknown purpose
		default:
			if (!machine().side_effects_disabled())
				LOGMASKED(LOG_DISK_JR, "%06X:disk_asic_jr_r(%X)\n", m_maincpu->pc(), offset);
			break;
	}
	return retVal;
}


void midzeus_state::disk_asic_jr_w(offs_t offset, uint32_t data)
{
	//uint32_t oldval = m_disk_asic_jr[offset];
	m_disk_asic_jr[offset] = data;

	switch (offset)
	{
		// disk asic jr led; crusnexo toggles this between 0 and 1 every 20 frames; thegrid writes 1
		case 0:
			if (data != 0 && data != 1)
				LOGMASKED(LOG_DISK_JR, "%06X:disk_asic_jr_w(%X) = %X (unexpected)\n", m_maincpu->pc(), offset, data);
			break;

		// miscellaneous hardware wait states; mk4/invasn write 1 here at initialization; crusnexo/thegrid write 3
		case 1:
			if (data != 1 && data != 3)
				LOGMASKED(LOG_DISK_JR, "%06X:disk_asic_jr_w(%X) = %X (unexpected)\n", m_maincpu->pc(), offset, data);
			break;

		// CMOS/ZPRAM write enable; only low bit is used
		case 2:
			break;

		// reset status; bit 0 is watchdog reset; mk4/invasn/thegrid read at startup; invasn freaks if it is 1 at startup
		case 3:
			break;

		// unknown purpose; invasn writes 2 here at startup
		case 4:
			if (data != 2)
				LOGMASKED(LOG_DISK_JR, "%06X:disk_asic_jr_w(%X) = %X (unexpected)\n", m_maincpu->pc(), offset, data);
			break;

		// ROM bank selection on Zeus 2
		case 5:
			if (m_mainbank)
				m_mainbank->set_entry(m_disk_asic_jr[offset] & 3);
			break;

		// zeus2 ws; 0=zeus access 1 wait state, 2=unlock ROMs; crusnexo/thegrid write 1 at startup
		case 7:
			break;

		// romsize; crusnexo writes 4 at startup; thegrid writes 6
		case 8:
			if (data != 4 && data != 6)
				LOGMASKED(LOG_DISK_JR, "%06X:disk_asic_jr_w(%X) = %X (unexpected)\n", m_maincpu->pc(), offset, data);
			break;

		// trackball reset; thegrid writes 1 at startup
		case 9:
			if (data != 1)
				LOGMASKED(LOG_DISK_JR, "%06X:disk_asic_jr_w(%X) = %X (unexpected)\n", m_maincpu->pc(), offset, data);
			break;
		// unknown purpose
		default:
			//if (oldval ^ data)
				LOGMASKED(LOG_DISK_JR, "%06X:disk_asic_jr_w(%X) = %X\n", m_maincpu->pc(), offset, data);
			break;

	}
}



/*************************************
 *
 *  7-segment LED controls
 *
 *************************************/

uint32_t crusnexo_state::crusnexo_leds_r(offs_t offset)
{
	// reads appear to just be for synchronization
	return ~0;
}


void crusnexo_state::crusnexo_leds_w(offs_t offset, uint32_t data)
{
	switch (offset)
	{
		case 0: // unknown purpose
			break;

		case 1: // controls lamps
			for (int bit = 0; bit < 8; bit++)
				m_lamps[bit] = BIT(data, bit);
			break;

		case 2: // sets state of selected LEDs

			// selection bits 4-6 select the 3 7-segment LEDs
			for (int bit = 4; bit < 7; bit++)
				if ((m_crusnexo_leds_select & (1 << bit)) == 0)
					m_digits[bit] = ~data & 0xff;

			// selection bits 0-2 select the tachometer LEDs
			for (int bit = 0; bit < 3; bit++)
				if ((m_crusnexo_leds_select & (1 << bit)) == 0)
					for (int led = 0; led < 8; led++)
						m_leds[bit * 8 + led] = BIT(~data, led);
			break;

		case 3: // selects which set of LEDs we are addressing
			m_crusnexo_leds_select = data;
			break;
	}
}



/*************************************
 *
 *  Firewire/IEEE 1394 access (Zeus 2 only)
 *
 *  Hardware: TSB12LV01A link layer controller and IBM IBM21S851 physical layer (PHY) transceiver.
 *
 *************************************/

void midzeus2_state::firewire_irq(int state)
{
	m_fw_int = state;
	update_firewire_irq();
}

void midzeus2_state::update_firewire_irq()
{
	LOGMASKED(LOG_FIREWIRE, "%ssserting FW IRQ\n", (m_fw_int_enable && m_fw_int) ? "A" : "Dea");
	m_maincpu->set_input_line(TMS3203X_IRQ3, (m_fw_int_enable && m_fw_int) ? ASSERT_LINE : CLEAR_LINE);
}

/*************************************
 *
 *  TMS32031 I/O accesses
 *
 *************************************/

uint32_t midzeus_state::tms32032_control_r(offs_t offset)
{
	// watch for accesses to the timers
	if (offset == 0x24 || offset == 0x34)
	{
		// timer is clocked at 100ns
		int const which = (offset >> 4) & 1;
		int32_t const result = (m_timer[which]->elapsed() * 10000000).as_double();
		return result;
	}

	// log anything else except the memory control register
	if (!machine().side_effects_disabled())
	{
		if (offset != 0x64)
			LOGMASKED(LOG_TMS32032, "%06X:tms32032_control_r(%02X)\n", m_maincpu->pc(), offset);
	}

	return m_tms32032_control[offset];
}


void midzeus_state::tms32032_control_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tms32032_control[offset]);

	// ignore changes to the memory control register
	if (offset == 0x64)
		;

	// watch for accesses to the timers
	else if (offset == 0x20 || offset == 0x30)
	{
		int const which = (offset >> 4) & 1;
		if (data & 0x40)
			m_timer[which]->adjust(attotime::never);
	}
	else
		LOGMASKED(LOG_TMS32032, "%06X:tms32032_control_w(%02X) = %08X\n", m_maincpu->pc(), offset, data);
}



/*************************************
 *
 *  49-way joystick handling
 *
 *************************************/

ioport_value thegrid_state::custom_49way_r()
{
	static const uint8_t translate49[7] = { 0x8, 0xc, 0xe, 0xf, 0x3, 0x1, 0x0 };
	return (translate49[m_io_49way_y->read() >> 4] << 4) | translate49[m_io_49way_x->read() >> 4];
}


void crusnexo_state::keypad_select_w(offs_t offset, uint32_t data)
{
	if (offset == 1)
		m_keypad_select = data;
}


ioport_value crusnexo_state::keypad_r()
{
	uint32_t bits = m_io_keypad->read();
	uint8_t select = m_keypad_select;
	while ((select & 1) != 0)
	{
		select >>= 1;
		bits >>= 4;
	}
	return bits;
}

uint32_t thegrid_state::grid_keypad_r(offs_t offset)
{
	uint32_t const bits = (m_io_keypad->read() >> ((offset >> 1) << 2)) & 0xf;
	return bits;
}

uint32_t thegrid_state::trackball_r(offs_t offset)
{
	if (offset == 0)
		return m_io_tracky->read();
	else
		return m_io_trackx->read();
}



/*************************************
 *
 *  Analog input handling
 *
 *************************************/

uint32_t crusnexo_state::analog_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (offset < 8 || offset > 11)
			LOGMASKED(LOG_INPUT, "%06X:analog_r(%X)\n", m_maincpu->pc(), offset);
	}
	return m_io_analog[offset & 3]->read();
}


void crusnexo_state::analog_w(uint32_t data)
{
	// 16 writes to the location before a read
}



/*************************************
 *
 *  Lightgun handling
 *
 *************************************/

void invasnab_state::update_gun_irq()
{
	// low 2 bits of gun_control seem to enable IRQs
	if (m_gun_irq_state & m_gun_control & 0x03)
		m_maincpu->set_input_line(TMS3203X_IRQ3, ASSERT_LINE);
	else
		m_maincpu->set_input_line(TMS3203X_IRQ3, CLEAR_LINE);
}


TIMER_CALLBACK_MEMBER(invasnab_state::invasn_gun_callback)
{
	int const player = param;
	int beamy = m_screen->vpos();

	// set the appropriate IRQ in the internal gun control and update
	m_gun_irq_state |= 0x01 << player;
	update_gun_irq();

	// generate another interrupt on the next scanline while we are within the BEAM_DY
	beamy++;
	if (beamy <= m_screen->visible_area().max_y && beamy <= m_gun_y[player] + BEAM_DY)
		m_gun_timer[player]->adjust(m_screen->time_until_pos(beamy, std::max(0, m_gun_x[player] - BEAM_DX)), player);
}


void invasnab_state::invasn_gun_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t const old_control = m_gun_control;

	COMBINE_DATA(&m_gun_control);

	// bits 0-1 enable IRQs (?)
	// bits 2-3 reset IRQ states
	m_gun_irq_state &= ~((m_gun_control >> 2) & 3);
	update_gun_irq();

	for (int player = 0; player < 2; player++)
	{
		uint8_t const pmask = 0x04 << player;
		if (((old_control ^ m_gun_control) & pmask) != 0 && (m_gun_control & pmask) == 0)
		{
			const rectangle &visarea = m_screen->visible_area();
			m_gun_x[player] = m_io_gun_x[player]->read() * visarea.width() / 255 + visarea.min_x + BEAM_XOFFS;
			m_gun_y[player] = m_io_gun_y[player]->read() * visarea.height() / 255 + visarea.min_y;
			m_gun_timer[player]->adjust(m_screen->time_until_pos(std::max(0, m_gun_y[player] - BEAM_DY), std::max(0, m_gun_x[player] - BEAM_DX)), player);
		}
	}
}


uint32_t invasnab_state::invasn_gun_r()
{
	int const beamx = m_screen->hpos();
	int const beamy = m_screen->vpos();
	uint32_t result = 0xffff;

	for (int player = 0; player < 2; player++)
	{
		int const diffx = beamx - m_gun_x[player];
		int const diffy = beamy - m_gun_y[player];
		if (diffx >= -BEAM_DX && diffx <= BEAM_DX && diffy >= -BEAM_DY && diffy <= BEAM_DY)
			result ^= 0x1000 << player;
	}
	return result;
}



/*************************************
 *
 *  Memory maps
 *
 *************************************/

void midzeus_state::zeus_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x03ffff).ram().share(m_ram_base);
	map(0x400000, 0x41ffff).ram();
	map(0x808000, 0x80807f).rw(FUNC(midzeus_state::tms32032_control_r), FUNC(midzeus_state::tms32032_control_w)).share(m_tms32032_control);
	map(0x880000, 0x8803ff).rw(FUNC(midzeus_state::zeus_r), FUNC(midzeus_state::zeus_w)).share(m_zeusbase);
	map(0x8d0000, 0x8d0009).rw(FUNC(midzeus_state::disk_asic_jr_r), FUNC(midzeus_state::disk_asic_jr_w));
	map(0x990000, 0x99000f).rw(m_ioasic, FUNC(midway_ioasic_device::read), FUNC(midway_ioasic_device::write));
	map(0x9e0000, 0x9e0000).nopw();        // watchdog?
	map(0x9f0000, 0x9f7fff).rw(FUNC(midzeus_state::cmos_r), FUNC(midzeus_state::cmos_w)).share(m_nvram);
	map(0x9f8000, 0x9f8000).w(FUNC(midzeus_state::cmos_protect_w));
	map(0xa00000, 0xffffff).rom().region("maindata", 0);
}

void invasnab_state::invasnab_map(address_map &map)
{
	zeus_map(map);
	map(0x9c0000, 0x9c0000).rw(FUNC(invasnab_state::invasn_gun_r), FUNC(invasnab_state::invasn_gun_w));
}


void midzeus2_state::zeus2_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x03ffff).ram().share(m_ram_base);
	map(0x400000, 0x43ffff).ram();
	map(0x808000, 0x80807f).rw(FUNC(midzeus2_state::tms32032_control_r), FUNC(midzeus2_state::tms32032_control_w)).share(m_tms32032_control);
	map(0x880000, 0x88007f).rw(m_zeus, FUNC(zeus2_device::zeus2_r), FUNC(zeus2_device::zeus2_w));
	map(0x8a0000, 0x8a00cf).rw(m_fw_link, FUNC(tsb12lv01a_device::read), FUNC(tsb12lv01a_device::write));
	//map(0x8a0000, 0x8a00cf).rw(FUNC(midzeus2_state::firewire_r), FUNC(midzeus2_state::firewire_w)).share("firewire");
	map(0x8d0000, 0x8d0009).rw(FUNC(midzeus2_state::disk_asic_jr_r), FUNC(midzeus2_state::disk_asic_jr_w));
	map(0x900000, 0x91ffff).rw(FUNC(midzeus2_state::zpram_r), FUNC(midzeus2_state::zpram_w)).share(m_nvram).mirror(0x020000);
	map(0x990000, 0x99000f).rw(m_ioasic, FUNC(midway_ioasic_device::read), FUNC(midway_ioasic_device::write));
	map(0x9d0000, 0x9d000f).rw(FUNC(midzeus2_state::disk_asic_r), FUNC(midzeus2_state::disk_asic_w));
	map(0x9e0000, 0x9e0000).nopw();        // watchdog?
	map(0x9f0000, 0x9f7fff).rw(FUNC(midzeus2_state::timekeeper_r), FUNC(midzeus2_state::timekeeper_w));
	map(0x9f8000, 0x9f8000).w(FUNC(midzeus2_state::cmos_protect_w));
	map(0xa00000, 0xbfffff).rom().region("maindata", 0);
	map(0xc00000, 0xffffff).bankr(m_mainbank);
}

void crusnexo_state::crusnexo_map(address_map &map)
{
	zeus2_map(map);
	map(0x8d0009, 0x8d000a).w(FUNC(crusnexo_state::keypad_select_w));
	map(0x9b0004, 0x9b0007).rw(FUNC(crusnexo_state::crusnexo_leds_r), FUNC(crusnexo_state::crusnexo_leds_w));
	map(0x9c0000, 0x9c000f).rw(FUNC(crusnexo_state::analog_r), FUNC(crusnexo_state::analog_w));
}

void thegrid_state::thegrid_map(address_map &map)
{
	zeus2_map(map);
	map(0x8c0000, 0x8c0001).r(FUNC(thegrid_state::trackball_r));
	map(0x9b0000, 0x9b0004).r(FUNC(thegrid_state::grid_keypad_r));
}

/*

    mk4:

        writes to 9D0000: 00000009, FFFFFFFF
        reads from 9D0000
        writes to 9D0001: 00000000
        writes to 9D0003: 00000374
        writes to 9D0005: 00000000

    crusnexo:

        reads from 8A0000

        writes to 9D0000: 00000000, 00000008, 00000009, FFFFFFFF
        reads from 9D0000
        writes to 9D0001: 00000000, 00000004, 00000204
        writes to 9D0003: 00000374
            -- hard coded to $374 at startup
        writes to 9D0004: 0000000F
            -- hard coded to $F at startup

        writes to 9E0008: 00000000

        writes to 9E8000: 00810081

    thegrid:

        writes to 9D0000: 00000008, 00000009, 0000008D
        writes to 9D0001: 00000000, 00000004, 00000204
        writes to 9D0003: 00000354
        reads from 9D0003
        writes to 9D0004: FFFFFFFF
        writes to 9D0005: 00000000

        writes to 9E8000: 00810081
*/


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( mk4 )
	PORT_START("DIPS")      // DS1
	PORT_DIPNAME( 0x0001, 0x0001, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x003e, 0x003e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x003e, "USA-1" )
	PORT_DIPSETTING(      0x003c, "USA-2" )
	PORT_DIPSETTING(      0x003a, "USA-3" )
	PORT_DIPSETTING(      0x0038, "USA-4" )
	PORT_DIPSETTING(      0x0034, "USA-9" )
	PORT_DIPSETTING(      0x0032, "USA-11" )
	PORT_DIPSETTING(      0x0036, "USA-ECA" )
	PORT_DIPSETTING(      0x002e, "German-1" )
	PORT_DIPSETTING(      0x002c, "German-2" )
	PORT_DIPSETTING(      0x002a, "German-3" )
	PORT_DIPSETTING(      0x0028, "German-4" )
	PORT_DIPSETTING(      0x0026, "German-ECA" )
	PORT_DIPSETTING(      0x001e, "French-1" )
	PORT_DIPSETTING(      0x001c, "French-2" )
	PORT_DIPSETTING(      0x001a, "French-3" )
	PORT_DIPSETTING(      0x0018, "French-4" )
	PORT_DIPSETTING(      0x0016, "French-ECA" )
	PORT_DIPSETTING(      0x0030, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  // Manual lists this dip as Unused
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Test Switch" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Fatalities" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Blood" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) // Manual states that switches 3-7 are Unused
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )    // Bill

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 High Punch")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Block")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 High Kick")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 High Punch")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Block")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 High Kick")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Low Punch")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Low Kick")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Run")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 Low Punch")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 Low Kick")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Run")
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( invasn )
	PORT_START("DIPS")      // DS1
	PORT_DIPNAME( 0x0001, 0x0001, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x003e, 0x003e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x003e, "USA-1" )
	PORT_DIPSETTING(      0x003c, "USA-2" )
	PORT_DIPSETTING(      0x003a, "USA-3" )
	PORT_DIPSETTING(      0x0038, "USA-4" )
	PORT_DIPSETTING(      0x0034, "USA-9" )
	PORT_DIPSETTING(      0x0032, "USA-10" )
	PORT_DIPSETTING(      0x0036, "USA-ECA" )
	PORT_DIPSETTING(      0x002e, "German-1" )
	PORT_DIPSETTING(      0x002c, "German-2" )
	PORT_DIPSETTING(      0x002a, "German-3" )
	PORT_DIPSETTING(      0x0028, "German-4" )
	PORT_DIPSETTING(      0x0024, "German-5" )
	PORT_DIPSETTING(      0x0026, "German-ECA" )
	PORT_DIPSETTING(      0x001e, "French-1" )
	PORT_DIPSETTING(      0x001c, "French-2" )
	PORT_DIPSETTING(      0x001a, "French-3" )
	PORT_DIPSETTING(      0x0018, "French-4" )
	PORT_DIPSETTING(      0x0014, "French-11" )
	PORT_DIPSETTING(      0x0012, "French-12" )
	PORT_DIPSETTING(      0x0016, "French-ECA" )
	PORT_DIPSETTING(      0x0030, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Flip Y" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Test Switch" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Mirrored Display" )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Show Blood" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )    // Bill

	PORT_START("IN1")
	PORT_BIT( 0x000f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00e0, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0f00, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xe000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("GUNX1")     // fake analog X
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10)

	PORT_START("GUNY1")     // fake analog Y
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10)

	PORT_START("GUNX2")     // fake analog X
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUNY2")     // fake analog Y
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( crusnexo )
	PORT_START("DIPS")      // DS1
	PORT_DIPNAME( 0x001f, 0x001f, "Country Code" )
	PORT_DIPSETTING(      0x001f, DEF_STR( USA ) )
	PORT_DIPSETTING(      0x001e, "Germany" )
	PORT_DIPSETTING(      0x001d, "France" )
	PORT_DIPSETTING(      0x001c, "Canada" )
	PORT_DIPSETTING(      0x001b, "Switzerland" )
	PORT_DIPSETTING(      0x001a, "Italy" )
	PORT_DIPSETTING(      0x0019, "UK" )
	PORT_DIPSETTING(      0x0018, "Spain" )
	PORT_DIPSETTING(      0x0017, "Australia" )
	PORT_DIPSETTING(      0x0016, DEF_STR( Japan ) )
	PORT_DIPSETTING(      0x0015, DEF_STR( Taiwan ) )
	PORT_DIPSETTING(      0x0014, "Austria" )
	PORT_DIPSETTING(      0x0013, "Belgium" )
	PORT_DIPSETTING(      0x000f, "Sweden" )
	PORT_DIPSETTING(      0x000e, "Finland" )
	PORT_DIPSETTING(      0x000d, "Netherlands" )
	PORT_DIPSETTING(      0x000c, "Norway" )
	PORT_DIPSETTING(      0x000b, "Denmark" )
	PORT_DIPSETTING(      0x000a, "Hungary" )
	PORT_DIPSETTING(      0x0008, "General" )
	PORT_DIPNAME( 0x0060, 0x0060, "Coin Mode" )
	PORT_DIPSETTING(      0x0060, "Mode 1" ) // USA1/GER1/FRA1/SPN1/AUSTRIA1/GEN1/CAN1/SWI1/ITL1/JPN1/TWN1/BLGN1/NTHRLND1/FNLD1/NRWY1/DNMK1/HUN1
	PORT_DIPSETTING(      0x0040, "Mode 2" ) // USA3/GER1/FRA1/SPN1/AUSTRIA1/GEN3/CAN2/SWI2/ITL2/JPN2/TWN2/BLGN2/NTHRLND2
	PORT_DIPSETTING(      0x0020, "Mode 3" ) // USA7/GER1/FRA1/SPN1/AUSTRIA1/GEN5/CAN3/SWI3/ITL3/JPN3/TWN3/BLGN3
	PORT_DIPSETTING(      0x0000, "Mode 4" ) // USA8/GER1/FRA1/SPN1/AUSTRIA1/GEN7
	PORT_DIPNAME( 0x0080, 0x0080, "Test Switch" )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Game Type" ) // Manual states "*DIP 1, Switch 1 MUST be set
	PORT_DIPSETTING(      0x0100, "Dedicated" ) //   to OFF position for proper operation"
	PORT_DIPSETTING(      0x0000, "Kit" )
	PORT_DIPNAME( 0x0200, 0x0200, "Seat Motion" )   // For dedicated Sit Down models with Motion Seat
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(      0x0400, "Stand Up" )
	PORT_DIPSETTING(      0x0000, "Sit Down" )
	PORT_DIPNAME( 0x0800, 0x0800, "Wheel Invert" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, "ROM Configuration" ) // Manual lists this dip as Unused
	PORT_DIPSETTING(      0x1000, "32M ROM Normal" )
	PORT_DIPSETTING(      0x0000, "16M ROM Split Active" )
	PORT_DIPNAME( 0x2000, 0x2000, "Link" )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Linking I.D.")
	PORT_DIPSETTING(      0xc000, "Master #1" )
	PORT_DIPSETTING(      0x8000, "Slave #2" )
	PORT_DIPSETTING(      0x4000, "Slave #3" )
	PORT_DIPSETTING(      0x0000, "Slave #4" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )

	PORT_START("IN1")   // Listed "names" are via the manual's "JAMMA" pinout sheet"
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )                          // Not Used
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Radio")       // Radio Switch
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )                          // Not Used
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )                          // Not Used
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("View 1")      // View 1
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("View 2")      // View 2
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("View 3")      // View 3
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("View 4")     // View 4
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("1st Gear")    // Gear 1
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("2nd Gear")    // Gear 2
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("3rd Gear")    // Gear 3
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("4th Gear")    // Gear 4
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )                          // Not Used
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )                          // Not Used
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )                          // Not Used
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(crusnexo_state::keypad_r))
	PORT_BIT( 0xfff8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEYPAD")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)   // keypad 3
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)   // keypad 1
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)   // keypad 2
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)   // keypad 6
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)   // keypad 4
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)   // keypad 5
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)   // keypad 9
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)   // keypad 7
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)   // keypad 8
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad #") PORT_CODE(KEYCODE_PLUS_PAD)    // keypad #
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_MINUS_PAD)   // keypad *
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)   // keypad 0

	PORT_START("ANALOG3")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x10,0xf0) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ANALOG2")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ANALOG1")
	PORT_BIT( 0xff, 0x00, IPT_PEDAL2 ) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("ANALOG0")
	PORT_BIT( 0xff, 0x00, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( thegrid )
	PORT_START("DIPS")      // DS1
	PORT_DIPNAME( 0x0100, 0x0100, "Show Blood" )
	PORT_DIPSETTING(      0x0100, "Show Blood" )
	PORT_DIPSETTING(      0x0000, "Do not show blood" )
	PORT_DIPUNUSED( 0xfe00, 0xfe00)
	PORT_DIPNAME( 0x0001, 0x0001, "Coinage Source" )
	PORT_DIPSETTING(      0x0001, "Dipswitch" )
	PORT_DIPSETTING(      0x0000, "CMOS" )
	PORT_DIPNAME( 0x003e, 0x003e, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x003e, "USA-1" )
	PORT_DIPSETTING(      0x0038, "USA-2" )
	PORT_DIPSETTING(      0x003c, "USA-10" )
	PORT_DIPSETTING(      0x003a, "USA-14" )
	PORT_DIPSETTING(      0x0036, "USA-DC1" )
	PORT_DIPSETTING(      0x0030, "USA-DC2" )
	PORT_DIPSETTING(      0x0032, "USA-DC4" )
	PORT_DIPSETTING(      0x0034, "USA-DC5" )
	PORT_DIPSETTING(      0x002e, "French-ECA1" )
	PORT_DIPSETTING(      0x002c, "French-ECA2" )
	PORT_DIPSETTING(      0x002a, "French-ECA3" )
	PORT_DIPSETTING(      0x0028, "French-ECA4" )
	PORT_DIPSETTING(      0x0026, "French-ECA5" )
	PORT_DIPSETTING(      0x0024, "French-ECA6" )
	PORT_DIPSETTING(      0x0022, "French-ECA7" )
	PORT_DIPSETTING(      0x0020, "French-ECA8" )
	PORT_DIPSETTING(      0x001e, "German-1" )
	PORT_DIPSETTING(      0x001c, "German-2" )
	PORT_DIPSETTING(      0x001a, "German-3" )
	PORT_DIPSETTING(      0x0018, "German-4" )
	PORT_DIPSETTING(      0x0016, "German-5" )
	PORT_DIPSETTING(      0x0014, "German-ECA1" )
	PORT_DIPSETTING(      0x0012, "German-ECA2" )
	PORT_DIPSETTING(      0x0010, "German-ECA3" )
	PORT_DIPSETTING(      0x0008, "UK-4" )
	PORT_DIPSETTING(      0x0006, "UK-5" )
	PORT_DIPSETTING(      0x000e, "UK-1 ECA" )
	PORT_DIPSETTING(      0x000c, "UK-2 ECA" )
	PORT_DIPSETTING(      0x000a, "UK-3 ECA" )
	PORT_DIPSETTING(      0x0004, "UK-6 ECA" )
	PORT_DIPSETTING(      0x0002, "UK-7 ECA" )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  // Manual states switches 7 & 8 are Unused
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Game Mode" )
	PORT_DIPSETTING(      0x0080, "Normal" )
	PORT_DIPSETTING(      0x0000, "Test" )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_TILT ) // Slam Switch
	PORT_SERVICE_NO_TOGGLE( 0x0010, IP_ACTIVE_LOW )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x6000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BILL1 )    // Bill

	PORT_START("IN1")   // Listed "names" are via the manual's "JAMMA" pinout sheet"
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(1) PORT_8WAY // Not Used
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(1) PORT_8WAY // Not Used
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(1) PORT_8WAY // Not Used
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY // Not Used
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // Trigger
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // Fire
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // Action
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_PLAYER(2) PORT_8WAY // No Connection
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_PLAYER(2) PORT_8WAY // No Connection
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_PLAYER(2) PORT_8WAY // No Connection
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY // No Connection
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // No Connection
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) // No Connection
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // No Connection
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(thegrid_state::custom_49way_r))
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("49WAYX")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_X ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("49WAYY")
	PORT_BIT( 0xff, 0x38, IPT_AD_STICK_Y ) PORT_MINMAX(0x00,0x6f) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("KEYPAD")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 1") PORT_CODE(KEYCODE_1_PAD)     // keypad 1
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 4") PORT_CODE(KEYCODE_4_PAD)     // keypad 4
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 7") PORT_CODE(KEYCODE_7_PAD)     // keypad 7
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad *") PORT_CODE(KEYCODE_ASTERISK)  // keypad *
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 2") PORT_CODE(KEYCODE_2_PAD)     // keypad 2
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 5") PORT_CODE(KEYCODE_5_PAD)     // keypad 5
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 8") PORT_CODE(KEYCODE_8_PAD)     // keypad 8
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 0") PORT_CODE(KEYCODE_0_PAD)     // keypad 0
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 3") PORT_CODE(KEYCODE_3_PAD)     // keypad 3
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 6") PORT_CODE(KEYCODE_6_PAD)     // keypad 6
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad 9") PORT_CODE(KEYCODE_9_PAD)     // keypad 9
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Keypad #") PORT_CODE(KEYCODE_PLUS_PAD)  // keypad #

	PORT_START("TRACKX")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_X) PORT_SENSITIVITY(1) PORT_KEYDELTA(1) PORT_PLAYER(1)

	PORT_START("TRACKY")
	PORT_BIT(0xff, 0x00, IPT_TRACKBALL_Y) PORT_SENSITIVITY(1) PORT_KEYDELTA(1) PORT_REVERSE PORT_PLAYER(1)

INPUT_PORTS_END



/*************************************
 *
 *  Machine drivers
 *
 *************************************/

static constexpr XTAL CPU_CLOCK = XTAL(60'000'000);

void midzeus_state::midzeus(machine_config &config)
{
	// basic machine hardware
	TMS32032(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &midzeus_state::zeus_map);
	m_maincpu->set_vblank_int("screen", FUNC(midzeus_state::display_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// video hardware
	PALETTE(config, m_palette, palette_device::RGB_555);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(MIDZEUS_VIDEO_CLOCK / 8, 529, 0, 400, 278, 0, 256);
	m_screen->set_screen_update(FUNC(midzeus_state::screen_update));
	m_screen->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	dcs2_audio_2104_device &dcs(DCS2_AUDIO_2104(config, "dcs", 0));
	dcs.set_maincpu_tag(m_maincpu);
	dcs.add_route(0, "speaker", 1.0, 1);
	dcs.add_route(1, "speaker", 1.0, 0);

	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag("dcs");
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_STANDARD);
	m_ioasic->set_yearoffs(94);
}

void midzeus_state::mk4(machine_config &config)
{
	midzeus(config);
	m_ioasic->set_upper(461/* or 474 */);
	m_ioasic->set_shuffle_default(1);
}

void invasnab_state::invasn(machine_config &config)
{
	midzeus(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &invasnab_state::invasnab_map);

	PIC16C57(config, "pic", 8000000);  // ?
	m_ioasic->set_upper(468/* or 488 */);
}

void midzeus2_state::midzeus2(machine_config &config)
{
	// basic machine hardware
	TMS32032(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &midzeus2_state::zeus2_map);
	m_maincpu->set_vblank_int("screen", FUNC(midzeus2_state::display_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(ZEUS2_VIDEO_CLOCK / 4, 666, 0, 512, 438, 0, 400);
	m_screen->set_screen_update(m_zeus, FUNC(zeus2_device::screen_update));

	ZEUS2(config, m_zeus, ZEUS2_VIDEO_CLOCK);
	m_zeus->irq_callback().set(FUNC(midzeus2_state::zeus_irq));

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	dcs2_audio_2104_device &dcs(DCS2_AUDIO_2104(config, "dcs", 0));
	dcs.set_maincpu_tag(m_maincpu);
	dcs.add_route(0, "speaker", 1.0, 1);
	dcs.add_route(1, "speaker", 1.0, 0);

	M48T35(config, m_m48t35, 0);

	// I/O hardware
	MIDWAY_IOASIC(config, m_ioasic, 0);
	m_ioasic->in_port_cb<0>().set_ioport("DIPS");
	m_ioasic->in_port_cb<1>().set_ioport("SYSTEM");
	m_ioasic->in_port_cb<2>().set_ioport("IN1");
	m_ioasic->in_port_cb<3>().set_ioport("IN2");
	m_ioasic->set_dcs_tag("dcs");
	m_ioasic->set_shuffle(midway_ioasic_device::SHUFFLE_STANDARD);
	m_ioasic->set_yearoffs(99);
	m_ioasic->set_upper(474);

	IBM21S851(config, m_fw_phy, 0);
	m_fw_phy->reset_cb().set(m_fw_link, FUNC(tsb12lv01a_device::phy_reset_w));

	TSB12LV01A(config, m_fw_link, 0);
	m_fw_link->int_cb().set(FUNC(midzeus2_state::firewire_irq));
	m_fw_link->phy_read().set(m_fw_phy, FUNC(ibm21s851_device::read));
	m_fw_link->phy_write().set(m_fw_phy, FUNC(ibm21s851_device::write));
}

void crusnexo_state::crusnexo(machine_config &config)
{
	midzeus2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &crusnexo_state::crusnexo_map);

	m_ioasic->set_upper(472/* or 476,477,478,110 */);
}

void thegrid_state::thegrid(machine_config &config)
{
	midzeus2(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &thegrid_state::thegrid_map);

	PIC16C57(config, "pic", 8000000).set_disable();  // unverified clock, not hooked up
	m_ioasic->set_upper(474/* or 491 */);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mk4 )
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "mk4_l2.u2", 0x000000, 0x100000, CRC(f9d410b4) SHA1(49bcacf83430ed26c08789b2f3ed9f946c3a0e5e) ) // Labeled as v2.0, ROM type M27C800
	ROM_LOAD16_BYTE( "mk4_l2.u3", 0x400000, 0x200000, CRC(8fbcf0ac) SHA1(c53704e72cfcba800c7af3a03267041f1e29a784) ) // Labeled as v2.0, ROM type M27C160
	ROM_LOAD16_BYTE( "mk4_l1.u4", 0x800000, 0x200000, CRC(dee91696) SHA1(00a182a36a414744cd014fcfc53c2e1a66ab5189) ) // Labeled as v1.0, ROM type M27C160
	ROM_LOAD16_BYTE( "mk4_l1.u5", 0xc00000, 0x200000, CRC(44d072be) SHA1(8a636c2801d799dfb84e69607ade76d2b49cf09f) ) // Labeled as v1.0, ROM type M27C160

	// 461 Mortal K. 4 25" U76
	ROM_REGION( 0x2000, "pic", 0 ) // PIC16C57
	ROM_LOAD( "461_mortal_k_4_25_u76.u76", 0x0000, 0x2000, CRC(d4432af9) SHA1(44a4b114f9b2075fdc611c011123a37b99458752) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x1800000, "maindata", 0 )
	ROM_LOAD32_WORD( "mk4_l3.u10", 0x0000000, 0x200000, CRC(84efe5a9) SHA1(e2a9bf6fab971691017371a87ab87b1bf66f96d0) ) // ROMs U10 & U11 were labeled as v3.0
	ROM_LOAD32_WORD( "mk4_l3.u11", 0x0000002, 0x200000, CRC(0c026ccb) SHA1(7531fe81ff8d8dd9ec3cd915acaf14cbe6bdc90a) )
	ROM_LOAD32_WORD( "mk4_l2.u12", 0x0400000, 0x200000, CRC(7816c07f) SHA1(da94b4391e671f915c61b5eb9bece4acb3382e31) ) // ROMs U12 through U17 were all labeled as v2.0
	ROM_LOAD32_WORD( "mk4_l2.u13", 0x0400002, 0x200000, CRC(b3c237cd) SHA1(9e71e60cc92c17524f85f36543c174ca138104cd) )
	ROM_LOAD32_WORD( "mk4_l2.u14", 0x0800000, 0x200000, CRC(fd33eb1a) SHA1(59d9d2e5251679d19cab031f51731c85f429ba18) ) // It is possible that in late production, these
	ROM_LOAD32_WORD( "mk4_l2.u15", 0x0800002, 0x200000, CRC(b907518f) SHA1(cfb56538746895bdca779957fec6a872019b23c3) ) // ROM were also labeled as v3.0, but the labels
	ROM_LOAD32_WORD( "mk4_l2.u16", 0x0c00000, 0x200000, CRC(24371d57) SHA1(c90134b17c23a182d391d1679bf457d251e641f7) ) // with v2.0 have been verified on several boards
	ROM_LOAD32_WORD( "mk4_l2.u17", 0x0c00002, 0x200000, CRC(3a1a082c) SHA1(5f8e8ce760d8ebadd1240ef08f1382a37cf11d0b) ) // Some PCBs may have all mask ROMs instead.
ROM_END

ROM_START( mk4a )
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "mk4_l2.u2", 0x000000, 0x100000, CRC(f9d410b4) SHA1(49bcacf83430ed26c08789b2f3ed9f946c3a0e5e) ) // Labeled as v2.0, ROM type M27C800
	ROM_LOAD16_BYTE( "mk4_l2.u3", 0x400000, 0x200000, CRC(8fbcf0ac) SHA1(c53704e72cfcba800c7af3a03267041f1e29a784) ) // Labeled as v2.0, ROM type M27C160
	ROM_LOAD16_BYTE( "mk4_l1.u4", 0x800000, 0x200000, CRC(dee91696) SHA1(00a182a36a414744cd014fcfc53c2e1a66ab5189) ) // Labeled as v1.0, ROM type M27C160
	ROM_LOAD16_BYTE( "mk4_l1.u5", 0xc00000, 0x200000, CRC(44d072be) SHA1(8a636c2801d799dfb84e69607ade76d2b49cf09f) ) // Labeled as v1.0, ROM type M27C160

	// 461 Mortal K. 4 25" U76
	ROM_REGION( 0x2000, "pic", 0 ) // PIC16C57
	ROM_LOAD( "461_mortal_k_4_25_u76.u76", 0x0000, 0x2000, CRC(d4432af9) SHA1(44a4b114f9b2075fdc611c011123a37b99458752) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x1800000, "maindata", 0 )
	ROM_LOAD32_WORD( "mk4_l2.1.u10", 0x0000000, 0x200000, CRC(42d0f1c9) SHA1(5ac0ded8bf6e756319be2691e3b555eac079ebdc) ) // ROMs U10 & U11 were labeled as v2.1
	ROM_LOAD32_WORD( "mk4_l2.1.u11", 0x0000002, 0x200000, CRC(6e21b243) SHA1(6d4768a5972db05c1409e0d16e79df9eff8918a0) )
	ROM_LOAD32_WORD( "mk4_l2.u12",   0x0400000, 0x200000, CRC(7816c07f) SHA1(da94b4391e671f915c61b5eb9bece4acb3382e31) ) // ROMs U12 through U17 were all labeled as v2.0
	ROM_LOAD32_WORD( "mk4_l2.u13",   0x0400002, 0x200000, CRC(b3c237cd) SHA1(9e71e60cc92c17524f85f36543c174ca138104cd) )
	ROM_LOAD32_WORD( "mk4_l2.u14",   0x0800000, 0x200000, CRC(fd33eb1a) SHA1(59d9d2e5251679d19cab031f51731c85f429ba18) )
	ROM_LOAD32_WORD( "mk4_l2.u15",   0x0800002, 0x200000, CRC(b907518f) SHA1(cfb56538746895bdca779957fec6a872019b23c3) )
	ROM_LOAD32_WORD( "mk4_l2.u16",   0x0c00000, 0x200000, CRC(24371d57) SHA1(c90134b17c23a182d391d1679bf457d251e641f7) )
	ROM_LOAD32_WORD( "mk4_l2.u17",   0x0c00002, 0x200000, CRC(3a1a082c) SHA1(5f8e8ce760d8ebadd1240ef08f1382a37cf11d0b) )
ROM_END

ROM_START( mk4b )
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "mk4_l1.u2", 0x000000, 0x200000, CRC(daac8ab5) SHA1(b93aa205868212077a9b6ac8e93205e1ebf8c05e) ) // All sound ROMs were labeled as v1.0 & are M27C160 type
	ROM_LOAD16_BYTE( "mk4_l1.u3", 0x400000, 0x200000, CRC(cb59413e) SHA1(f7e5c589a8f6a2e7dceee4881594e7403be4d4ad) )
	ROM_LOAD16_BYTE( "mk4_l1.u4", 0x800000, 0x200000, CRC(dee91696) SHA1(00a182a36a414744cd014fcfc53c2e1a66ab5189) )
	ROM_LOAD16_BYTE( "mk4_l1.u5", 0xc00000, 0x200000, CRC(44d072be) SHA1(8a636c2801d799dfb84e69607ade76d2b49cf09f) )

	// 461 Mortal K. 4 25" U76
	ROM_REGION( 0x2000, "pic", 0 ) // PIC16C57
	ROM_LOAD( "461_mortal_k_4_25_u76.u76", 0x0000, 0x2000, CRC(d4432af9) SHA1(44a4b114f9b2075fdc611c011123a37b99458752) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x1800000, "maindata", 0 )
	ROM_LOAD32_WORD( "mk4_l1.u10", 0x0000000, 0x200000, CRC(6fcc86dd) SHA1(b3b2b463daf51450fbcd5d2922ac1b091bd91c4a) ) // All ROMs were labeled as v1.0
	ROM_LOAD32_WORD( "mk4_l1.u11", 0x0000002, 0x200000, CRC(04895940) SHA1(55d368905f5986587c4e3da236401fdd5e2c269c) )
	ROM_LOAD32_WORD( "mk4_l1.u12", 0x0400000, 0x200000, CRC(323ddc5c) SHA1(4303c109c68a7cc15ff6fe91b6d34383b6066351) )
	ROM_LOAD32_WORD( "mk4_l1.u13", 0x0400002, 0x200000, CRC(0b95bdf0) SHA1(a25d48b33a861b5e52736720c7a79291fa837f78) )
	ROM_LOAD32_WORD( "mk4_l1.u14", 0x0800000, 0x200000, CRC(cb6816ef) SHA1(9c828c188d297aee0f211acc283035289e80b5a8) )
	ROM_LOAD32_WORD( "mk4_l1.u15", 0x0800002, 0x200000, CRC(cde47df7) SHA1(63383d983c03703b2f3f1973ce2a7553654836d4) )
	// No U16 or U17 ROMs present in this version
ROM_END

ROM_START( invasnab ) // Version 5.0 Program ROMs, v4.0 Graphics ROMs, v2.0 Sound ROMs
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "invasion2.u2", 0x000000, 0x200000, CRC(59d2e1d6) SHA1(994a4311ac4841d4341449c0c7480952b6f3855d) ) // These four sound ROMs were labeled as v2.0
	ROM_LOAD16_BYTE( "invasion2.u3", 0x400000, 0x200000, CRC(86b956ae) SHA1(f7fd4601a2ce3e7e9b67e7d77908bfa206ee7e62) )
	ROM_LOAD16_BYTE( "invasion2.u4", 0x800000, 0x200000, CRC(5ef1fab5) SHA1(987afa0672fa89b18cf20d28644848a9e5ee9b17) )
	ROM_LOAD16_BYTE( "invasion2.u5", 0xc00000, 0x200000, CRC(e42805c9) SHA1(e5b71eb1852809a649ac43a82168b3bdaf4b1526) )

	ROM_REGION( 0x2000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "pic16c57.u76", 0x00000, 0x2000, BAD_DUMP CRC(f62729c9) SHA1(9642c53dd7eceeb7eb178497d367691c44abc5c5) ) // is this even a valid dump?

	ROM_REGION32_LE( 0x1800000, "maindata", 0 )
	ROM_LOAD32_WORD( "invasion5.u10", 0x0000000, 0x200000, CRC(8c7785d9) SHA1(701602314cd4eba4215c47ea0ae75fd4eddad43b) ) // ROMs U10 & U11 were labeled as v5.0
	ROM_LOAD32_WORD( "invasion5.u11", 0x0000002, 0x200000, CRC(8ceb1f32) SHA1(82d01f25cba25d77b11c347632e8b72776e12984) )
	ROM_LOAD32_WORD( "invasion4.u12", 0x0400000, 0x200000, CRC(ce1eb06a) SHA1(ff17690a0cbca6dcccccde70e2c5812ae03db5bb) ) // ROMs U12 through U19 were all labeled as v4.0
	ROM_LOAD32_WORD( "invasion4.u13", 0x0400002, 0x200000, CRC(33fc6707) SHA1(11a39ad980ec320547319eca6ffa5aef3ab8b010) )
	ROM_LOAD32_WORD( "invasion4.u14", 0x0800000, 0x200000, CRC(760682a1) SHA1(ff91210225d4aa750115c6219d4c35c9521a3f0b) )
	ROM_LOAD32_WORD( "invasion4.u15", 0x0800002, 0x200000, CRC(90467d7a) SHA1(a143a3d3605e5626852e75937160ba6bcd891608) )
	ROM_LOAD32_WORD( "invasion4.u16", 0x0c00000, 0x200000, CRC(3ef1b28d) SHA1(6f9a071b8830194fea84daa62aadabae86977c5f) )
	ROM_LOAD32_WORD( "invasion4.u17", 0x0c00002, 0x200000, CRC(97aa677a) SHA1(4d21cc59e0ffd4985f89c97c71d605c3b404a8a3) )
	ROM_LOAD32_WORD( "invasion4.u18", 0x1000000, 0x200000, CRC(6930c656) SHA1(28054ff9a6c6f5764a371f8defe4c1f5730618f3) )
	ROM_LOAD32_WORD( "invasion4.u19", 0x1000002, 0x200000, CRC(89fa6ee5) SHA1(572565e1308142b0b062aa72315c68e928f2419c) )
ROM_END

ROM_START( invasnab4 ) // Version 4.0 Program ROMs & Graphics ROMs, v2.0 Sound ROMs
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "invasion2.u2", 0x000000, 0x200000, CRC(59d2e1d6) SHA1(994a4311ac4841d4341449c0c7480952b6f3855d) ) // These four sound ROMs were labeled as v2.0
	ROM_LOAD16_BYTE( "invasion2.u3", 0x400000, 0x200000, CRC(86b956ae) SHA1(f7fd4601a2ce3e7e9b67e7d77908bfa206ee7e62) )
	ROM_LOAD16_BYTE( "invasion2.u4", 0x800000, 0x200000, CRC(5ef1fab5) SHA1(987afa0672fa89b18cf20d28644848a9e5ee9b17) )
	ROM_LOAD16_BYTE( "invasion2.u5", 0xc00000, 0x200000, CRC(e42805c9) SHA1(e5b71eb1852809a649ac43a82168b3bdaf4b1526) )

	ROM_REGION( 0x2000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "pic16c57.u76", 0x00000, 0x2000, BAD_DUMP CRC(f62729c9) SHA1(9642c53dd7eceeb7eb178497d367691c44abc5c5) ) // is this even a valid dump?

	ROM_REGION32_LE( 0x1800000, "maindata", 0 )
	ROM_LOAD32_WORD( "invasion4.u10", 0x0000000, 0x200000, CRC(b3ce958b) SHA1(ed51c167d85bc5f6155b8046ec056a4f4ad5cf9d) ) // These ROM were all labeled as v4.0
	ROM_LOAD32_WORD( "invasion4.u11", 0x0000002, 0x200000, CRC(0bd09359) SHA1(f40886bd2e5f5fbf506580e5baa2f733be200852) )
	ROM_LOAD32_WORD( "invasion4.u12", 0x0400000, 0x200000, CRC(ce1eb06a) SHA1(ff17690a0cbca6dcccccde70e2c5812ae03db5bb) )
	ROM_LOAD32_WORD( "invasion4.u13", 0x0400002, 0x200000, CRC(33fc6707) SHA1(11a39ad980ec320547319eca6ffa5aef3ab8b010) )
	ROM_LOAD32_WORD( "invasion4.u14", 0x0800000, 0x200000, CRC(760682a1) SHA1(ff91210225d4aa750115c6219d4c35c9521a3f0b) )
	ROM_LOAD32_WORD( "invasion4.u15", 0x0800002, 0x200000, CRC(90467d7a) SHA1(a143a3d3605e5626852e75937160ba6bcd891608) )
	ROM_LOAD32_WORD( "invasion4.u16", 0x0c00000, 0x200000, CRC(3ef1b28d) SHA1(6f9a071b8830194fea84daa62aadabae86977c5f) )
	ROM_LOAD32_WORD( "invasion4.u17", 0x0c00002, 0x200000, CRC(97aa677a) SHA1(4d21cc59e0ffd4985f89c97c71d605c3b404a8a3) )
	ROM_LOAD32_WORD( "invasion4.u18", 0x1000000, 0x200000, CRC(6930c656) SHA1(28054ff9a6c6f5764a371f8defe4c1f5730618f3) )
	ROM_LOAD32_WORD( "invasion4.u19", 0x1000002, 0x200000, CRC(89fa6ee5) SHA1(572565e1308142b0b062aa72315c68e928f2419c) )
ROM_END

ROM_START( invasnab3 ) // Version 3.0 Program ROMs & v2.0 Graphics ROMs, v2.0 Sound ROMs
	ROM_REGION16_LE( 0x1000000, "dcs", ROMREGION_ERASEFF )  // sound data
	ROM_LOAD16_BYTE( "invasion2.u2", 0x000000, 0x200000, CRC(59d2e1d6) SHA1(994a4311ac4841d4341449c0c7480952b6f3855d) ) // These four sound ROMs were labeled as v2.0 Dated 6/24/99
	ROM_LOAD16_BYTE( "invasion2.u3", 0x400000, 0x200000, CRC(86b956ae) SHA1(f7fd4601a2ce3e7e9b67e7d77908bfa206ee7e62) )
	ROM_LOAD16_BYTE( "invasion2.u4", 0x800000, 0x200000, CRC(5ef1fab5) SHA1(987afa0672fa89b18cf20d28644848a9e5ee9b17) )
	ROM_LOAD16_BYTE( "invasion2.u5", 0xc00000, 0x200000, CRC(e42805c9) SHA1(e5b71eb1852809a649ac43a82168b3bdaf4b1526) )

	ROM_REGION( 0x2000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "pic16c57.u76", 0x00000, 0x2000, BAD_DUMP CRC(f62729c9) SHA1(9642c53dd7eceeb7eb178497d367691c44abc5c5) ) // is this even a valid dump?

	ROM_REGION32_LE( 0x1800000, "maindata", 0 )
	ROM_LOAD32_WORD( "invasion3.u10", 0x0000000, 0x200000, CRC(8404830e) SHA1(808fea45fb09fb7bf60f9f1e195a51d39e9966f5) ) // ROMs U10 through U13 were labeled as v3.0 Dated 8/30
	ROM_LOAD32_WORD( "invasion3.u11", 0x0000002, 0x200000, CRC(cb893a37) SHA1(c0b8283d9b6b2b1a5fed7f542a8964ed875182b1) )
	ROM_LOAD32_WORD( "invasion3.u12", 0x0400000, 0x200000, CRC(79bfa881) SHA1(7c68a2f236223506f24a38d21836d132f2e10ac3) )
	ROM_LOAD32_WORD( "invasion3.u13", 0x0400002, 0x200000, CRC(7a371a4a) SHA1(e87e7a9d6417057c2fc3dae2bd8c45b673420b08) )
	ROM_LOAD32_WORD( "invasion2.u14", 0x0800000, 0x200000, CRC(b51ecabd) SHA1(435888ec9886dcd22a2f24a199789d10889550b3) ) // ROMs U14 through U19 were labeled as v2.0 Dated 6/24/99
	ROM_LOAD32_WORD( "invasion2.u15", 0x0800002, 0x200000, CRC(584b0596) SHA1(cab8222977ecc8a689f1f3f7ebc38ff7bec6a43f) )
	ROM_LOAD32_WORD( "invasion2.u16", 0x0c00000, 0x200000, CRC(5d422855) SHA1(ceafd60b020b03de051765b9e9dd0d01285a0335) )
	ROM_LOAD32_WORD( "invasion2.u17", 0x0c00002, 0x200000, CRC(c6555769) SHA1(a97361001311dd6fb28f79df422e6e8c27ea2495) )
	ROM_LOAD32_WORD( "invasion2.u18", 0x1000000, 0x200000, CRC(dbc9548e) SHA1(6deac9fde26144a61bd63833a197801d159f0c9a) )
	ROM_LOAD32_WORD( "invasion2.u19", 0x1000002, 0x200000, CRC(4b05a2a9) SHA1(3d47c22a809f5883e4795a9153161d3e29c64662) )
ROM_END

/*

Cruis'N Exotica

While many PCB use mask ROMs for data ROMs, when EPROMs are used the common label format is:

---------------------------
|    CRUIS'N EXOTICA      |
)      U22 REV.1.0        |
| C 1999 Midway Games Inc |
---------------------------

An early PCB was found with the following labels:

--------------------------------
| U  CRUIS'N EXOTICA           |
\ 2      5350-40072-17         |
/ 2  C 1999  DD74 1/10/2000    |
|    MIDWAY AMUSMENT GAMES LLC |
--------------------------------

Each label reads:

 U2  CRUIS'N EXOTICA  C       5350-40072-13  6500           - missing info due to no label
 U3  CRUIS'N EXOTICA  C 2000  5350-40072-14  8CD4  1/25/00
 U4  CRUIS'N EXOTICA  C 1999  5350-40072-15  94CD  10-7-99
U25  CRUIS'N EXOTICA  C 1999  5350-40072-20  44FB  1/10/2000
U24  CRUIS'N EXOTICA  C 1999  5350-40072-19  DD74  1/10/2000
U23  CRUIS'N EXOTICA  C 1999  5350-40072-18  8E69  1/10/2000
U22  CRUIS'N EXOTICA  C 1999  5350-40072-17  84FC  1/10/2000
U19  CRUIS'N EXOTICA  C 2000  5350-40072-8  E4F0  1/30/00
U18  CRUIS'N EXOTICA  C 2000  5350-40072-7  CDEC  1/30/00
U17  CRUIS'N EXOTICA  C 1999  5350-40072-6  ADC6  12-20-99
U16  CRUIS'N EXOTICA  C 1999  5350-40072-5  83B2  12-20-99
U15  CRUIS'N EXOTICA  C 1999  5350-40072-4  8E69  12-20-99
U14  CRUIS'N EXOTICA  C 1999  5350-40072-3  84FC  12-20-99

NOTE: Boards can also be found using "half sized" U18 & U19 ROMs with the remaining data found in U20 & U21,
      during the boot up, the PCB ROM test shows U18 & U19 checked while U20 & U21 are shown unpopulated and
      are skipped. However, PCBs with genuine labels have been found in this configuration.

*/

ROM_START( crusnexo )
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "cruisn_exotica_u2_rev.1.0.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) ) // labeled CRUIS'N EXOTICA  U2 REV.1.0
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "cruisn_exotica_u3_rev.1.0.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) ) // labeled CRUIS'N EXOTICA  U3 REV.1.0
	ROM_LOAD( "cruisn_exotica_u4_rev.1.0.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) ) // labeled CRUIS'N EXOTICA  U4 REV.1.0
	// U5 unpopulated

	ROM_REGION( 0x1000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "472_cruisn_exot_27.u53", 0x0000, 0x1000, CRC(7ff41d76) SHA1(13d23e634dc8d20fbee11a9c39923b7e54984672) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u10_rev_2.4.u10", 0x0000000, 0x200000, CRC(5e702f7c) SHA1(98c76fb46b304d4d21656d0505d5e5e99c8335bf) ) // Version 2.4  Wed Aug 23, 2000  17:26:53
	ROM_LOAD32_WORD( "cruisn_exotica_u11_rev_2.4.u11", 0x0000002, 0x200000, CRC(5ecb2cbc) SHA1(57283167e48ca96579d0712d9fec23a36fa2b496) )
	ROM_LOAD32_WORD( "cruisn_exotica_u12_rev.1.0.u12", 0x0400000, 0x200000, CRC(21f122b2) SHA1(5473401ec954bf9ab66a8283bd08d17c7960cd29) ) // These 2 ROMs might be labeled as a different version,
	ROM_LOAD32_WORD( "cruisn_exotica_u13_rev.1.0.u13", 0x0400002, 0x200000, CRC(cf9d3609) SHA1(6376891f478185d26370466bef92f0c5304d58d3) ) // but the data doesn't change. Verified for v1.3 & v1.6

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u14_rev.1.0.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "cruisn_exotica_u15_rev.1.0.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "cruisn_exotica_u16_rev.1.0.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u17_rev.1.0.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "cruisn_exotica_u22_rev.1.0.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "cruisn_exotica_u23_rev.1.0.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "cruisn_exotica_u24_rev.1.0.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "cruisn_exotica_u25_rev.1.0.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "cruisn_exotica_u18_rev.1.0.u18", 0x2000000, 0x400000, CRC(06efb00e) SHA1(fe4968220c854ee65a78323a44787cee394234a3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u19_rev.1.0.u19", 0x2000002, 0x400000, CRC(2e75bf61) SHA1(196c24814b873dc0e500bb2187ec54e4cae6a139) )
	// U20 & U21 unpopulated
ROM_END

ROM_START( crusnexoa )
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "cruisn_exotica_u2_rev.1.0.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) ) // labeled CRUIS'N EXOTICA  U2 REV.1.0
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "cruisn_exotica_u3_rev.1.0.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) ) // labeled CRUIS'N EXOTICA  U3 REV.1.0
	ROM_LOAD( "cruisn_exotica_u4_rev.1.0.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) ) // labeled CRUIS'N EXOTICA  U4 REV.1.0
	// U5 unpopulated

	ROM_REGION( 0x1000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "472_cruisn_exot_27.u53", 0x0000, 0x1000, CRC(7ff41d76) SHA1(13d23e634dc8d20fbee11a9c39923b7e54984672) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u10_rev_2.0.u10", 0x0000000, 0x200000, CRC(43d80f54) SHA1(25683d835f3ed3dee99da33280ae6e21865801e4) ) // Version 2.0  Fri Apr 07, 2000  17:55:07
	ROM_LOAD32_WORD( "cruisn_exotica_u11_rev_2.0.u11", 0x0000002, 0x200000, CRC(dba26b69) SHA1(4900ac3fe67664a543dcd66e41793874f6cdc07f) )
	ROM_LOAD32_WORD( "cruisn_exotica_u12_rev.1.0.u12", 0x0400000, 0x200000, CRC(21f122b2) SHA1(5473401ec954bf9ab66a8283bd08d17c7960cd29) ) // These 2 ROMs might be labeled as a different version,
	ROM_LOAD32_WORD( "cruisn_exotica_u13_rev.1.0.u13", 0x0400002, 0x200000, CRC(cf9d3609) SHA1(6376891f478185d26370466bef92f0c5304d58d3) ) // but the data doesn't change. Verified for v1.3 & v1.6

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u14_rev.1.0.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "cruisn_exotica_u15_rev.1.0.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "cruisn_exotica_u16_rev.1.0.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u17_rev.1.0.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "cruisn_exotica_u22_rev.1.0.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "cruisn_exotica_u23_rev.1.0.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "cruisn_exotica_u24_rev.1.0.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "cruisn_exotica_u25_rev.1.0.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "cruisn_exotica_u18_rev.1.0.u18", 0x2000000, 0x400000, CRC(06efb00e) SHA1(fe4968220c854ee65a78323a44787cee394234a3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u19_rev.1.0.u19", 0x2000002, 0x400000, CRC(2e75bf61) SHA1(196c24814b873dc0e500bb2187ec54e4cae6a139) )
	// U20 & U21 unpopulated
ROM_END

ROM_START( crusnexoaa ) // known alternate ROM configuration - The half size U18 & U19 and use of U20 & U21 can appear with any program ROM revision
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "cruisn_exotica_u2_rev.1.0.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) ) // labeled CRUIS'N EXOTICA  U2 REV.1.0
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "cruisn_exotica_u3_rev.1.0.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) ) // labeled CRUIS'N EXOTICA  U3 REV.1.0
	ROM_LOAD( "cruisn_exotica_u4_rev.1.0.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) ) // labeled CRUIS'N EXOTICA  U4 REV.1.0
	// U5 unpopulated

	ROM_REGION( 0x1000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "472_cruisn_exot_27.u53", 0x0000, 0x1000, CRC(7ff41d76) SHA1(13d23e634dc8d20fbee11a9c39923b7e54984672) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u10_rev_2.0.u10", 0x0000000, 0x200000, CRC(43d80f54) SHA1(25683d835f3ed3dee99da33280ae6e21865801e4) ) // Version 2.0  Fri Apr 07, 2000  17:55:07
	ROM_LOAD32_WORD( "cruisn_exotica_u11_rev_2.0.u11", 0x0000002, 0x200000, CRC(dba26b69) SHA1(4900ac3fe67664a543dcd66e41793874f6cdc07f) )
	ROM_LOAD32_WORD( "cruisn_exotica_u12_rev.1.0.u12", 0x0400000, 0x200000, CRC(21f122b2) SHA1(5473401ec954bf9ab66a8283bd08d17c7960cd29) ) // These 2 ROMs might be labeled as a different version,
	ROM_LOAD32_WORD( "cruisn_exotica_u13_rev.1.0.u13", 0x0400002, 0x200000, CRC(cf9d3609) SHA1(6376891f478185d26370466bef92f0c5304d58d3) ) // but the data doesn't change. Verified for v1.3 & v1.6

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u14_rev.1.0.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "cruisn_exotica_u15_rev.1.0.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "cruisn_exotica_u16_rev.1.0.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u17_rev.1.0.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "cruisn_exotica_u22_rev.1.0.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "cruisn_exotica_u23_rev.1.0.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "cruisn_exotica_u24_rev.1.0.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "cruisn_exotica_u25_rev.1.0.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "cruisn_exotica_u18_rev.1.0.u18", 0x2000000, 0x200000, CRC(60cf5caa) SHA1(629870a305802d632bd2681131d1ffc0086280d2) ) // sldh - 1st half of U18 compared to other sets
	ROM_LOAD32_WORD( "cruisn_exotica_u19_rev.1.0.u19", 0x2000002, 0x200000, CRC(6b919a18) SHA1(20e40e195554146ed1d3fad54f7280823ae89d4b) ) // sldh - 1st half of U19 compared to other sets
	ROM_LOAD32_WORD( "cruisn_exotica_u20_rev.1.0.u20", 0x2400000, 0x200000, CRC(4855b68b) SHA1(1f6e557590b2621d0d5c782b95577f1be5cbc51d) ) // 2nd half of U18 compared to other sets
	ROM_LOAD32_WORD( "cruisn_exotica_u21_rev.1.0.u21", 0x2400002, 0x200000, CRC(0011b9d6) SHA1(231d768c964a16b905857b0814d758fe93c2eefb) ) // 2nd half of U19 compared to other sets
ROM_END

ROM_START( crusnexob )
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "cruisn_exotica_u2_rev.1.0.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) ) // labeled CRUIS'N EXOTICA  U2 REV.1.0
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "cruisn_exotica_u3_rev.1.0.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) ) // labeled CRUIS'N EXOTICA  U3 REV.1.0
	ROM_LOAD( "cruisn_exotica_u4_rev.1.0.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) ) // labeled CRUIS'N EXOTICA  U4 REV.1.0
	// U5 unpopulated

	ROM_REGION( 0x1000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "472_cruisn_exot_27.u53", 0x0000, 0x1000, CRC(7ff41d76) SHA1(13d23e634dc8d20fbee11a9c39923b7e54984672) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u10_rev_1.6.u10", 0x0000000, 0x200000, CRC(65450140) SHA1(cad41a2cad48426de01feb78d3f71f768e3fc872) ) // Version 1.6  Tue Feb 22, 2000  10:25:01
	ROM_LOAD32_WORD( "cruisn_exotica_u11_rev_1.6.u11", 0x0000002, 0x200000, CRC(e994891f) SHA1(bb088729b665864c7f3b79b97c3c86f9c8f68770) )
	ROM_LOAD32_WORD( "cruisn_exotica_u12_rev.1.0.u12", 0x0400000, 0x200000, CRC(21f122b2) SHA1(5473401ec954bf9ab66a8283bd08d17c7960cd29) ) // These 2 ROMs might be labeled as a different version,
	ROM_LOAD32_WORD( "cruisn_exotica_u13_rev.1.0.u13", 0x0400002, 0x200000, CRC(cf9d3609) SHA1(6376891f478185d26370466bef92f0c5304d58d3) ) // but the data doesn't change. Verified for v1.3 & v1.6

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u14_rev.1.0.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "cruisn_exotica_u15_rev.1.0.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "cruisn_exotica_u16_rev.1.0.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u17_rev.1.0.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "cruisn_exotica_u22_rev.1.0.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "cruisn_exotica_u23_rev.1.0.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "cruisn_exotica_u24_rev.1.0.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "cruisn_exotica_u25_rev.1.0.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "cruisn_exotica_u18_rev.1.0.u18", 0x2000000, 0x400000, CRC(06efb00e) SHA1(fe4968220c854ee65a78323a44787cee394234a3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u19_rev.1.0.u19", 0x2000002, 0x400000, CRC(2e75bf61) SHA1(196c24814b873dc0e500bb2187ec54e4cae6a139) )
	// U20 & U21 unpopulated
ROM_END

ROM_START( crusnexoc )
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "cruisn_exotica_u2_rev.1.0.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) ) // labeled CRUIS'N EXOTICA  U2 REV.1.0
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "cruisn_exotica_u3_rev.1.0.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) ) // labeled CRUIS'N EXOTICA  U3 REV.1.0
	ROM_LOAD( "cruisn_exotica_u4_rev.1.0.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) ) // labeled CRUIS'N EXOTICA  U4 REV.1.0
	// U5 unpopulated

	ROM_REGION( 0x1000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "472_cruisn_exot_27.u53", 0x0000, 0x1000, CRC(7ff41d76) SHA1(13d23e634dc8d20fbee11a9c39923b7e54984672) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u10_rev_1.3.u10", 0x0000000, 0x200000, CRC(ab7f1b5e) SHA1(c0c561e8cb15fd97465278b4b3b15acb27380c5d) ) // Version 1.3  Fri Feb 11, 2000  16:19:13
	ROM_LOAD32_WORD( "cruisn_exotica_u11_rev_1.3.u11", 0x0000002, 0x200000, CRC(62d3c966) SHA1(9a485892295984a292501424d2c78caafac99a75) )
	ROM_LOAD32_WORD( "cruisn_exotica_u12_rev.1.0.u12", 0x0400000, 0x200000, CRC(21f122b2) SHA1(5473401ec954bf9ab66a8283bd08d17c7960cd29) ) // These 2 ROMs might be labeled as a different version,
	ROM_LOAD32_WORD( "cruisn_exotica_u13_rev.1.0.u13", 0x0400002, 0x200000, CRC(cf9d3609) SHA1(6376891f478185d26370466bef92f0c5304d58d3) ) // but the data doesn't change. Verified for v1.3 & v1.6

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u14_rev.1.0.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "cruisn_exotica_u15_rev.1.0.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "cruisn_exotica_u16_rev.1.0.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u17_rev.1.0.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "cruisn_exotica_u22_rev.1.0.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "cruisn_exotica_u23_rev.1.0.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "cruisn_exotica_u24_rev.1.0.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "cruisn_exotica_u25_rev.1.0.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "cruisn_exotica_u18_rev.1.0.u18", 0x2000000, 0x400000, CRC(06efb00e) SHA1(fe4968220c854ee65a78323a44787cee394234a3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u19_rev.1.0.u19", 0x2000002, 0x400000, CRC(2e75bf61) SHA1(196c24814b873dc0e500bb2187ec54e4cae6a139) )
	// U20 & U21 unpopulated
ROM_END

ROM_START( crusnexod )
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "cruisn_exotica_u2_rev.1.0.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) ) // labeled CRUIS'N EXOTICA  U2 REV.1.0
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "cruisn_exotica_u3_rev.1.0.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) ) // labeled CRUIS'N EXOTICA  U3 REV.1.0
	ROM_LOAD( "cruisn_exotica_u4_rev.1.0.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) ) // labeled CRUIS'N EXOTICA  U4 REV.1.0
	// U5 unpopulated

	ROM_REGION( 0x1000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "472_cruisn_exot_27.u53", 0x0000, 0x1000, CRC(7ff41d76) SHA1(13d23e634dc8d20fbee11a9c39923b7e54984672) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u10_rev.1.0.u10", 0x0000000, 0x200000, CRC(305fe2c1) SHA1(5d12163da0ae6db7d8d1f64f79c767a3c7df29a0) ) // Version 1.0  Tue Feb 08, 2000  13:22:04
	ROM_LOAD32_WORD( "cruisn_exotica_u11_rev.1.0.u11", 0x0000002, 0x200000, CRC(50b241ff) SHA1(b8a353d9420009c4e521bb088575d704a7f386b3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u12_rev.1.0.u12", 0x0400000, 0x200000, CRC(21f122b2) SHA1(5473401ec954bf9ab66a8283bd08d17c7960cd29) )
	ROM_LOAD32_WORD( "cruisn_exotica_u13_rev.1.0.u13", 0x0400002, 0x200000, CRC(cf9d3609) SHA1(6376891f478185d26370466bef92f0c5304d58d3) )

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u14_rev.1.0.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "cruisn_exotica_u15_rev.1.0.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "cruisn_exotica_u16_rev.1.0.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u17_rev.1.0.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "cruisn_exotica_u22_rev.1.0.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "cruisn_exotica_u23_rev.1.0.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "cruisn_exotica_u24_rev.1.0.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "cruisn_exotica_u25_rev.1.0.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "cruisn_exotica_u18_rev.1.0.u18", 0x2000000, 0x400000, CRC(06efb00e) SHA1(fe4968220c854ee65a78323a44787cee394234a3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u19_rev.1.0.u19", 0x2000002, 0x400000, CRC(2e75bf61) SHA1(196c24814b873dc0e500bb2187ec54e4cae6a139) )
	// U20 & U21 unpopulated
ROM_END

ROM_START( crusnexoe )
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "cruisn_exotica_u2_rev.1.0.u2", 0x000000, 0x200000, CRC(d2d54acf) SHA1(2b4d6fda30af807228bb281335939dfb6df9b530) )
	ROM_RELOAD(             0x200000, 0x200000 )
	ROM_LOAD( "cruisn_exotica_u3_rev.1.0.u3", 0x400000, 0x400000, CRC(28a3a13d) SHA1(8d7d641b883df089adefdd144229afef79db9e8a) )
	ROM_LOAD( "cruisn_exotica_u4_rev.1.0.u4", 0x800000, 0x400000, CRC(213f7fd8) SHA1(8528d524a62bc41a8e3b39f0dbeeba33c862ee27) )
	// U5 unpopulated

	ROM_REGION( 0x1000, "pic", 0 ) // PIC16c57 Code
	ROM_LOAD( "472_cruisn_exot_27.u53", 0x0000, 0x1000, CRC(7ff41d76) SHA1(13d23e634dc8d20fbee11a9c39923b7e54984672) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "cruisin_exotica_u10_98bf_1-31-00_rev_1.0.u10", 0x0000000, 0x200000, CRC(0206492b) SHA1(d2275bf0c02a14db8d2af20f888a5b765cfc33e1) )
	ROM_LOAD32_WORD( "cruisin_exotica_u11_db5a_1-31-00_rev_1.0.u11", 0x0000002, 0x200000, CRC(f0546f97) SHA1(442a6544f40207de628ef15b63f3c976ef297cfb) )
	ROM_LOAD32_WORD( "cruisin_exotica_u12_8e33_1-31-00_rev_1.0.u12", 0x0400000, 0x200000, CRC(f129c6ba) SHA1(c4b6c39146312026c5207eb3b10400fa9a7df687) )
	ROM_LOAD32_WORD( "cruisin_exotica_u13_2c9c_1-31-00_rev_1.0.u13", 0x0400002, 0x200000, CRC(b45c081f) SHA1(924236393cea569191966b2f61957bc37a21d334) )

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "cruisn_exotica_u14_rev.1.0.u14", 0x0000000, 0x400000, CRC(84452fc2) SHA1(06d87263f83ef079e6c5fb9de620e0135040c858) )
	ROM_LOAD32_WORD( "cruisn_exotica_u15_rev.1.0.u15", 0x0000002, 0x400000, CRC(b6aaebdb) SHA1(6ede6ea123be6a88d1ff38e90f059c9d1f822d6d) )
	ROM_LOAD32_WORD( "cruisn_exotica_u16_rev.1.0.u16", 0x0800000, 0x400000, CRC(aac6d2a5) SHA1(6c336520269d593b46b82414d9352a3f16955cc3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u17_rev.1.0.u17", 0x0800002, 0x400000, CRC(71cf5404) SHA1(a6eed1a66fb4f4ddd749e4272a2cdb8e3e354029) )
	ROM_LOAD32_WORD( "cruisn_exotica_u22_rev.1.0.u22", 0x1000000, 0x400000, CRC(ad6dcda7) SHA1(5c9291753e1659f9adbe7e59fa2d0e030efae5bc) )
	ROM_LOAD32_WORD( "cruisn_exotica_u23_rev.1.0.u23", 0x1000002, 0x400000, CRC(1f103a68) SHA1(3b3acc63a461677cd424e75e7211fa6f063a37ef) )
	ROM_LOAD32_WORD( "cruisn_exotica_u24_rev.1.0.u24", 0x1800000, 0x400000, CRC(6312feef) SHA1(4113e4e5d39c99e8131d41a57c973df475b67d18) )
	ROM_LOAD32_WORD( "cruisn_exotica_u25_rev.1.0.u25", 0x1800002, 0x400000, CRC(b8277b16) SHA1(1355e87affd78e195906aedc9aed9e230374e2bf) )
	ROM_LOAD32_WORD( "cruisn_exotica_u18_rev.1.0.u18", 0x2000000, 0x400000, CRC(06efb00e) SHA1(fe4968220c854ee65a78323a44787cee394234a3) )
	ROM_LOAD32_WORD( "cruisn_exotica_u19_rev.1.0.u19", 0x2000002, 0x400000, CRC(2e75bf61) SHA1(196c24814b873dc0e500bb2187ec54e4cae6a139) )
	// U20 & U21 unpopulated
ROM_END

ROM_START( thegrid ) // Version 1.2 Program ROMs
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "the_grid.u2", 0x000000, 0x400000, CRC(e6a39ee9) SHA1(4ddc62f5d278ea9791205098fa5f018ab1e698b4) )
	ROM_LOAD( "the_grid.u3", 0x400000, 0x400000, CRC(40be7585) SHA1(e481081edffa07945412a6eab17b4d3e7b42cfd3) )
	ROM_LOAD( "the_grid.u4", 0x800000, 0x400000, CRC(7a15c203) SHA1(a0a49dd08bba92402640ed2d1fb4fee112c4ab5f) )

	ROM_REGION( 0x2000, "pic", 0 ) // PIC16C57
	ROM_LOAD( "pic16c57.u76", 0x0000, 0x1fff, CRC(8234d466) SHA1(5737e355d3262cd0b13191cdf9b49dd74f69dd15) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "thegrid-12.u10", 0x0000000, 0x100000, CRC(eb6c2d54) SHA1(ddd32757a9be011988b7add3c091e93292a0867c) )
	ROM_LOAD32_WORD( "thegrid-12.u11", 0x0000002, 0x100000, CRC(b9b5f92b) SHA1(36e16f109af9a5172869344f09b337b67e0b3e11) )
	ROM_LOAD32_WORD( "thegrid-12.u12", 0x0200000, 0x100000, CRC(2810c207) SHA1(d244eaf85473ed49442a906d437af1a9f91a2f9d) )
	ROM_LOAD32_WORD( "thegrid-12.u13", 0x0200002, 0x100000, CRC(8b721848) SHA1(d82f39045437ada2061587176e24f558a5e203fe) )

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "the_grid.u18",   0x0000000, 0x400000, CRC(3a3460be) SHA1(e719dae8a2e54584cb6a074ed42e35e3debef2f6) )
	ROM_LOAD32_WORD( "the_grid.u19",   0x0000002, 0x400000, CRC(af262d5b) SHA1(3eb3980fa81a360a70aa74e793b2bc3028f68cf2) )
	ROM_LOAD32_WORD( "the_grid.u20",   0x0800000, 0x400000, CRC(e6ad1917) SHA1(acab25e1251fd07b374badebe79f6ec1772b3589) )
	ROM_LOAD32_WORD( "the_grid.u21",   0x0800002, 0x400000, CRC(48c03f8e) SHA1(50790bdae9f2234ffb4914c2c5c16374e3508b47) )
	ROM_LOAD32_WORD( "the_grid.u22",   0x1000000, 0x400000, CRC(84c3a8b6) SHA1(de0dcf9daf7ada7a6952b9e29a29571b2aa9d0b2) )
	ROM_LOAD32_WORD( "the_grid.u23",   0x1000002, 0x400000, CRC(f48ef409) SHA1(79d74b4fe38b06a02ae0351d13d7f0a7ed0f0c87) )
ROM_END


ROM_START( thegrida ) // Version 1.1 Program ROMs
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "the_grid.u2", 0x000000, 0x400000, CRC(e6a39ee9) SHA1(4ddc62f5d278ea9791205098fa5f018ab1e698b4) )
	ROM_LOAD( "the_grid.u3", 0x400000, 0x400000, CRC(40be7585) SHA1(e481081edffa07945412a6eab17b4d3e7b42cfd3) )
	ROM_LOAD( "the_grid.u4", 0x800000, 0x400000, CRC(7a15c203) SHA1(a0a49dd08bba92402640ed2d1fb4fee112c4ab5f) )

	ROM_REGION( 0x2000, "pic", 0 ) // PIC16C57
	ROM_LOAD( "pic16c57.u76", 0x0000, 0x1fff, CRC(8234d466) SHA1(5737e355d3262cd0b13191cdf9b49dd74f69dd15) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "thegrid-11.u10", 0x0000000, 0x100000, CRC(87ea0e9e) SHA1(618de2ca87b7a3e0225d1f7e65f8fc1356de1421) )
	ROM_LOAD32_WORD( "thegrid-11.u11", 0x0000002, 0x100000, CRC(73d84b1a) SHA1(8dcfcab5ff64f46f8486e6439a10d91ad26fd48a) )
	ROM_LOAD32_WORD( "thegrid-11.u12", 0x0200000, 0x100000, CRC(78d16ca1) SHA1(7b893ec8af2f44d8bc293861fd8622d68d41ccbe) )
	ROM_LOAD32_WORD( "thegrid-11.u13", 0x0200002, 0x100000, CRC(8e00b400) SHA1(96581c5da62afc19e6d69b2352b3166665cb9918) )

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "the_grid.u18",   0x0000000, 0x400000, CRC(3a3460be) SHA1(e719dae8a2e54584cb6a074ed42e35e3debef2f6) )
	ROM_LOAD32_WORD( "the_grid.u19",   0x0000002, 0x400000, CRC(af262d5b) SHA1(3eb3980fa81a360a70aa74e793b2bc3028f68cf2) )
	ROM_LOAD32_WORD( "the_grid.u20",   0x0800000, 0x400000, CRC(e6ad1917) SHA1(acab25e1251fd07b374badebe79f6ec1772b3589) )
	ROM_LOAD32_WORD( "the_grid.u21",   0x0800002, 0x400000, CRC(48c03f8e) SHA1(50790bdae9f2234ffb4914c2c5c16374e3508b47) )
	ROM_LOAD32_WORD( "the_grid.u22",   0x1000000, 0x400000, CRC(84c3a8b6) SHA1(de0dcf9daf7ada7a6952b9e29a29571b2aa9d0b2) )
	ROM_LOAD32_WORD( "the_grid.u23",   0x1000002, 0x400000, CRC(f48ef409) SHA1(79d74b4fe38b06a02ae0351d13d7f0a7ed0f0c87) )
ROM_END

ROM_START( thegridb ) // Version 1.01 Program ROMs
	ROM_REGION16_LE( 0xc00000, "dcs", ROMREGION_ERASEFF )   // sound data
	ROM_LOAD( "the_grid.u2", 0x000000, 0x400000, CRC(e6a39ee9) SHA1(4ddc62f5d278ea9791205098fa5f018ab1e698b4) )
	ROM_LOAD( "the_grid.u3", 0x400000, 0x400000, CRC(40be7585) SHA1(e481081edffa07945412a6eab17b4d3e7b42cfd3) )
	ROM_LOAD( "the_grid.u4", 0x800000, 0x400000, CRC(7a15c203) SHA1(a0a49dd08bba92402640ed2d1fb4fee112c4ab5f) )

	ROM_REGION( 0x2000, "pic", 0 ) // PIC16C57
	ROM_LOAD( "pic16c57.u76", 0x0000, 0x1fff, CRC(8234d466) SHA1(5737e355d3262cd0b13191cdf9b49dd74f69dd15) ) // decapped but not hooked up

	ROM_REGION32_LE( 0x0800000, "maindata", 0 )
	ROM_LOAD32_WORD( "mpg_the_grid_1-17-00_ver1.01_54e6.u10", 0x0000000, 0x100000, CRC(cd0bf7c3) SHA1(8b490955381c078443e048dadd78fa931754bd0f) )
	ROM_LOAD32_WORD( "mpg_the_grid_1-17-00_ver1.01_568d.u11", 0x0000002, 0x100000, CRC(ffea0d0a) SHA1(f0fe36b9f2fe890957a0dcc05bb091a78357cced) )
	ROM_LOAD32_WORD( "mpg_the_grid_1-17-00_ver1.01_a117.u12", 0x0200000, 0x100000, CRC(ad54ad55) SHA1(2c7175bed85c75070357c83009527229e4943fe0) )
	ROM_LOAD32_WORD( "mpg_the_grid_1-17-00_ver1.01_5694.u13", 0x0200002, 0x100000, CRC(976a3ab8) SHA1(6e521525208358f270a4961cad408ed598a25c88) )

	ROM_REGION32_LE( 0x3000000, "bankeddata", 0 )
	ROM_LOAD32_WORD( "the_grid.u18",   0x0000000, 0x400000, CRC(3a3460be) SHA1(e719dae8a2e54584cb6a074ed42e35e3debef2f6) )
	ROM_LOAD32_WORD( "the_grid.u19",   0x0000002, 0x400000, CRC(af262d5b) SHA1(3eb3980fa81a360a70aa74e793b2bc3028f68cf2) )
	ROM_LOAD32_WORD( "the_grid.u20",   0x0800000, 0x400000, CRC(e6ad1917) SHA1(acab25e1251fd07b374badebe79f6ec1772b3589) )
	ROM_LOAD32_WORD( "the_grid.u21",   0x0800002, 0x400000, CRC(48c03f8e) SHA1(50790bdae9f2234ffb4914c2c5c16374e3508b47) )
	ROM_LOAD32_WORD( "the_grid.u22",   0x1000000, 0x400000, CRC(84c3a8b6) SHA1(de0dcf9daf7ada7a6952b9e29a29571b2aa9d0b2) )
	ROM_LOAD32_WORD( "the_grid.u23",   0x1000002, 0x400000, CRC(f48ef409) SHA1(79d74b4fe38b06a02ae0351d13d7f0a7ed0f0c87) )
ROM_END



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME(  1997, mk4,        0,        mk4,      mk4,      midzeus_state,  empty_init, ROT0, "Midway", "Mortal Kombat 4 (version 3.0)",                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME(  1997, mk4a,       mk4,      mk4,      mk4,      midzeus_state,  empty_init, ROT0, "Midway", "Mortal Kombat 4 (version 2.1)",                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME(  1997, mk4b,       mk4,      mk4,      mk4,      midzeus_state,  empty_init, ROT0, "Midway", "Mortal Kombat 4 (version 1.0)",                       MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME(  1999, invasnab,   0,        invasn,   invasn,   invasnab_state, empty_init, ROT0, "Midway", "Invasion - The Abductors (version 5.0)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME(  1999, invasnab4,  invasnab, invasn,   invasn,   invasnab_state, empty_init, ROT0, "Midway", "Invasion - The Abductors (version 4.0)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME(  1999, invasnab3,  invasnab, invasn,   invasn,   invasnab_state, empty_init, ROT0, "Midway", "Invasion - The Abductors (version 3.0)",              MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAMEL( 1999, crusnexo,   0,        crusnexo, crusnexo, crusnexo_state, empty_init, ROT0, "Midway", "Cruis'n Exotica (version 2.4)",                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_crusnexo )
GAMEL( 1999, crusnexoa,  crusnexo, crusnexo, crusnexo, crusnexo_state, empty_init, ROT0, "Midway", "Cruis'n Exotica (version 2.0)",                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_crusnexo )
GAMEL( 1999, crusnexoaa, crusnexo, crusnexo, crusnexo, crusnexo_state, empty_init, ROT0, "Midway", "Cruis'n Exotica (version 2.0, alternate ROM format)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_crusnexo )
GAMEL( 1999, crusnexob,  crusnexo, crusnexo, crusnexo, crusnexo_state, empty_init, ROT0, "Midway", "Cruis'n Exotica (version 1.6)",                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_crusnexo )
GAMEL( 1999, crusnexoc,  crusnexo, crusnexo, crusnexo, crusnexo_state, empty_init, ROT0, "Midway", "Cruis'n Exotica (version 1.3)",                       MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_crusnexo )
GAMEL( 1999, crusnexod,  crusnexo, crusnexo, crusnexo, crusnexo_state, empty_init, ROT0, "Midway", "Cruis'n Exotica (version 1.0, build 8764)",           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_crusnexo ) //  8-Feb-2000
GAMEL( 1999, crusnexoe,  crusnexo, crusnexo, crusnexo, crusnexo_state, empty_init, ROT0, "Midway", "Cruis'n Exotica (version 1.0, build 8643)",           MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE, layout_crusnexo ) // 31-Jan-2000
GAME(  2000, thegrid,    0,        thegrid,  thegrid,  thegrid_state,  empty_init, ROT0, "Midway", "The Grid (version 1.2)",                              MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 10/16/00
GAME(  2000, thegrida,   thegrid,  thegrid,  thegrid,  thegrid_state,  empty_init, ROT0, "Midway", "The Grid (version 1.1)",                              MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 07/26/00
GAME(  2000, thegridb,   thegrid,  thegrid,  thegrid,  thegrid_state,  empty_init, ROT0, "Midway", "The Grid (version 1.01)",                             MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // 07/17/00

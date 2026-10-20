// license:BSD-3-Clause
// copyright-holders:Dirk Best, Tomás García-Merás (ClawGrip)
/*
    Azkoyen "Design" tobacco vending machines (D6, D8, D10, D12, D14, D21...)

    * design6: pesetas firmware, only has tables for a six channel machine.
    * designe: euro firmware dated 05-10-06, for machines with the adult
      access remote kit (43521600-5, shown at power on), with 16 model
      presets selected in the CONFIGURACION menu.

    Hardware:
    * Intel P8051, 27C256 EPROM, NEC D446C-2 2K SRAM (battery backed)
    * OKI M62X428 RTC
    * Rockwell 10937P-50 A8201-17 VFD controller, 16 characters
    * Azkoyen L66S coin selector with PIC16C76/PIC16F76 (undumped)

    The I/O map and signal meanings come from analysing both firmware
    dumps, which use the same map.  "(assumption)" marks interpretations
    not verified on hardware and "(simulated)" marks machine mechanics
    modelled with made-up timings; the rest is firmware behaviour.
    Quoted messages are the firmware errors tied to an input or output.

    designe initialises the UART but never uses it.  The operator manual
    available (D14/D21, pesetas, 7/94, adapted for euro) does not fully
    match the dumps.  The L66S service manual (13.02.01) says the selector
    identifies up to 32 coin types, and that the selector and sorter
    assembly connects to the machine with 4 coin outputs, a general inhibit
    input and drivers for three sorter coils; there is no accept input.

    Usage
    -----
    The service switch (CS) selects NORMAL (selling) or CONTROL mode.  In
    control mode the display first repeats any problem found while
    selling; then each press of Coin Return moves to the next menu and the
    keypad A-D inside the machine works inside it.  Long menu names scroll
    by.  The designe selections are numbered as in its D21 preset.

    Setting up a new machine, which shows FUERA SERVICIO:
    1. Set the service switch to CONTROL.  The display shows DESPROGRAMADA
       (designe: also DESCONFIGURADA).
    2. designe only, choose the model: press Coin Return (CONFIGURACION),
       then D (it shows D6), B five times (D8, D10 RODE, D10/D12, D14,
       D21) and D four times (ORDEN EJECUTADA).  For the D6, D8 and
       D10/D12 models also set the Hopper Exit Sensors setting to Active
       High.
    3. Press Coin Return until PROGRAMACION PRECIOS scrolls by and PULSE
       CANAL shows.
    4. Press a selection.  The display shows CANAL, its number and its
       price.  Press A once to set the price to 0, then add to it with A
       (1), B (10), C (100) and D (1000); designe counts in cents (0.01,
       0.10, 1.00 and 10.00 euro).  For 150 pesetas: A, C, B five times.
       For 1.30 euro: A, C, B three times.  Pressing the selection again
       starts it over.  Do this for every selection.
    5. designe only, so coins are accepted without the adult remote: press
       Coin Return until ACCESO ADULTO, then D, B (MANDO ADULTO ON), A
       (OFF) and D four times.
    6. Set the service switch to NORMAL.  The display shows VERIFICANDO,
       then *** AZKOYEN *** and the time: the machine is ready.

    Selling: insert coins and press a selection; the product falls and the
    change is paid.  Coin Return gives the coins back.  designe shows SOLO
    ADULTOS until the adult access remote is pressed.  The hopper empty
    and full switches are off on a working machine.

    The menus, in the order Coin Return shows them.  Once the machine is
    set up, control mode starts at the first one.
    - DESCARGA DEVOLVEDORES: A, B or C empties a hopper, counting the
      coins; D stops.
    - PROGRAMACION PRECIOS: see step 4.
    - PRODUCTO VENDIDO POR CANAL: a selection shows its sales, A clears.
    - VENTA TOTAL: money taken.
    - BORRADO TOTAL PRODUCTO VENDIDO: A clears all the sales.
    - HORAS / MIN: the clock.
    - PROGRAM. MENSAJE: the message shown while idle.
    - TEST VENTA: A switches it on; then, back in normal mode, every
      selection vends without money.  Switch it off again here.
    - designe: ACCESO ADULTO (step 5), CONFIGURACION (step 2), MONEDAS
      CAMBIO and INHIBICION.
    A fault (AVERIA ..., VACIO DEVOL.) shows when entering control mode and
    is cleared by leaving it.

    TODO:
    - Coin selector timings, meaning of coin code 0xe
    - Function of latch 0 bit 7 and P3.3, dump optional voice synthesizer
      kit, watchdog
    - Verify the assumptions on real hardware
*/

#include "emu.h"

#include "cpu/mcs51/i8051.h"
#include "machine/74259.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "video/roc10937.h"

#include "design6.lh"
#include "designe.lh"

#define LOG_COINSEL (1U << 1)
#define LOG_VOICE   (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_COINSEL | LOG_VOICE)
#include "logmacro.h"

#define LOGCOINSEL(...) LOGMASKED(LOG_COINSEL, __VA_ARGS__)
#define LOGVOICE(...)   LOGMASKED(LOG_VOICE, __VA_ARGS__)


namespace {

class design_state : public driver_device
{
public:
	design_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_hopper(*this, "hopper%u", 1U)
		, m_xdata_view(*this, "xdata")
		, m_io_keys(*this, "KEY%u", 0U)
		, m_io_flaps(*this, "FLAPS")
		, m_out_extractor(*this, "extractor%u", 1U)
		, m_out_flap(*this, "flap")
	{
	}

	void design6(machine_config &config) ATTR_COLD;
	void designe(machine_config &config) ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(remote_pressed);

	ioport_value coin_code_r() { return (m_coinsel_code & 0x0f) | (BIT(m_coinsel_code, 4) << 5); }
	int recovery_position_r() { return m_recovery_pos || m_remote_pulse; }
	ioport_value flaps_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	enum coinsel_state : u8
	{
		COINSEL_IDLE,
		COINSEL_VALIDATE,
		COINSEL_TRAVEL,
		COINSEL_CONFIRM
	};

	required_device<i8051_device> m_maincpu;
	required_device_array<hopper_device, 3> m_hopper;
	memory_view m_xdata_view;
	required_ioport_array<4> m_io_keys;
	required_ioport m_io_flaps;
	output_finder<24> m_out_extractor;
	output_finder<> m_out_flap;

	emu_timer *m_coinsel_timer = nullptr;
	emu_timer *m_recovery_timer = nullptr;
	emu_timer *m_flap_timer = nullptr;
	emu_timer *m_remote_timer = nullptr;

	u8 m_port1 = 0xff;
	u8 m_coinsel_state = COINSEL_IDLE;
	u8 m_coinsel_coin = 0;
	u8 m_coinsel_code = 0;
	bool m_coinsel_enable = false;
	bool m_recovery_motor = false;
	bool m_recovery_pos = false;
	u32 m_extractors = 0;
	bool m_flap_raised = false;
	bool m_remote_pulse = false;

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	void port1_w(u8 data);
	u8 keys_r();
	void voice_w(u8 data);

	void enable_w(int state);
	void recovery_motor_w(int state);
	template <unsigned N> void extractors_w(u8 data);

	TIMER_CALLBACK_MEMBER(coinsel_update);
	TIMER_CALLBACK_MEMBER(recovery_update);
	TIMER_CALLBACK_MEMBER(flap_update);
	TIMER_CALLBACK_MEMBER(remote_release);
};



/**************************************************************************
    Address maps
**************************************************************************/

void design_state::program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}

void design_state::data_map(address_map &map)
{
	map(0x0000, 0xffff).view(m_xdata_view);

	// P1.5 = 0: I/O, P2 is always 0
	m_xdata_view[0](0x0000, 0x0000).portr("IN0");
	m_xdata_view[0](0x0000, 0x0007).w("outlatch0", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0010, 0x0010).portr("IN1");
	m_xdata_view[0](0x0010, 0x0017).w("outlatch1", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0020, 0x0020).r(FUNC(design_state::keys_r));
	m_xdata_view[0](0x0020, 0x0027).w("outlatch2", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0030, 0x0030).portr("IN3");
	m_xdata_view[0](0x0030, 0x0037).w("outlatch3", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0040, 0x0047).w("outlatch4", FUNC(cd4099_device::write_d0));
	m_xdata_view[0](0x0060, 0x006f).rw("rtc", FUNC(msm6242_device::read), FUNC(msm6242_device::write));
	m_xdata_view[0](0x0070, 0x0070).w(FUNC(design_state::voice_w));

	// P1.5 = 1: SRAM, decoding above A10 unknown
	m_xdata_view[1](0x0000, 0x07ff).mirror(0xf800).ram().share("nvram");
}



/**************************************************************************
    Input ports
**************************************************************************/

// the firmware stops routing coins to a hopper while its "full" input is low (meaning assumed)
static INPUT_PORTS_START( design )
	PORT_START("IN0")
	PORT_BIT(0x2f, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(design_state::coin_code_r))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED) // designe expects 0 to clear a coin selector fault
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_SERVICE) PORT_TOGGLE PORT_NAME("Service/Control Switch (CS)")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 2 Full")

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper3", FUNC(hopper_device::line_r))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper2", FUNC(hopper_device::line_r))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper1", FUNC(hopper_device::line_r))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 3 Empty") // "VACIO DEVOL."
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 2 Empty")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 1 Empty")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(design_state::recovery_position_r))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 1 Full")

	PORT_START("IN3")
	PORT_BIT(0x07, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_CUSTOM_MEMBER(FUNC(design_state::flaps_r)) // "AVERIA TRAMP."
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Hopper 3 Full")
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	// with P3.3 low a selection only shows the price, even with credit; function unknown
	PORT_START("P3")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_OTHER) PORT_TOGGLE PORT_NAME("Vend Inhibit (P3.3)")
	PORT_BIT(0xf7, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("FLAPS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Product Flap 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Product Flap 2")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Product Flap 3")

	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_B) PORT_NAME("Coin Return")
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY1")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY2")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("KEY3")
	PORT_BIT(0x0f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Z) PORT_NAME("Keypad A")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_X) PORT_NAME("Keypad B")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_C) PORT_NAME("Keypad C")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_V) PORT_NAME("Keypad D")
INPUT_PORTS_END

static INPUT_PORTS_START( design6 )
	PORT_INCLUDE(design)

	// only keys 1-6 are mapped to selections
	PORT_MODIFY("KEY0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Q) PORT_NAME("Selection 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_W) PORT_NAME("Selection 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_E) PORT_NAME("Selection 3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_R) PORT_NAME("Selection 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_T) PORT_NAME("Selection 5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Y) PORT_NAME("Selection 6")

	// codes 2/5 are presumably the old 5/25 peseta coins and 8/9 the new ones (assumption)
	PORT_START("COINS")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("5 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 8)
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("10 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 3)
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("25 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 9)
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("50 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 6)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_COIN5) PORT_NAME("100 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 7)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_COIN6) PORT_NAME("200 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 4)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_COIN7) PORT_NAME("500 Pesetas") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 1)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_COIN8) PORT_NAME("5 Pesetas (alt)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 2)
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_COIN9) PORT_NAME("25 Pesetas (alt)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 5)
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_COIN10) PORT_NAME("50 Pesetas (alt)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 10)
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_COIN11) PORT_NAME("200 Pesetas (alt)") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 11)
INPUT_PORTS_END

static INPUT_PORTS_START( designe )
	PORT_INCLUDE(design)

	// named as in the D21 preset, other presets map the keys differently
	PORT_MODIFY("KEY0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Q) PORT_NAME("Selection 1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_W) PORT_NAME("Selection 2")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_E) PORT_NAME("Selection 3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_R) PORT_NAME("Selection 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_T) PORT_NAME("Selection 5")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_Y) PORT_NAME("Selection 6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_U) PORT_NAME("Selection 7")

	PORT_MODIFY("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_I) PORT_NAME("Selection 8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_O) PORT_NAME("Selection 9")

	PORT_MODIFY("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_A) PORT_NAME("Selection 10")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_S) PORT_NAME("Selection 11")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_D) PORT_NAME("Selection 12")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F) PORT_NAME("Selection 13")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_G) PORT_NAME("Selection 14")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_H) PORT_NAME("Selection 15")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_J) PORT_NAME("Selection 16")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_K) PORT_NAME("Selection 17")

	PORT_MODIFY("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_L) PORT_NAME("Selection 18")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_COLON) PORT_NAME("Selection 19")

	PORT_START("REMOTE")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_M) PORT_NAME("Adult Access Remote") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::remote_pressed), 0)

	PORT_START("COINS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1) PORT_NAME("0.05 Euro") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 0x13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_COIN2) PORT_NAME("0.10 Euro") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 0x14)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN3) PORT_NAME("0.20 Euro") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 0x15)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_COIN4) PORT_NAME("0.50 Euro") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 0x16)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_COIN5) PORT_NAME("1 Euro") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 0x17)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN6) PORT_NAME("2 Euro") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 0x18)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_COIN7) PORT_NAME("Token") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(design_state::coin_inserted), 0x0c)

	// the firmware takes the hopper exit sensor polarity from the model preset
	PORT_START("CONF")
	PORT_CONFNAME(0x01, 0x00, "Hopper Exit Sensors")
	PORT_CONFSETTING(   0x01, "Active High (D6, D8, D10/D12 presets)")
	PORT_CONFSETTING(   0x00, "Active Low (other presets)")

	PORT_MODIFY("IN1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper3", FUNC(hopper_device::line_r)) PORT_CONDITION("CONF", 0x01, EQUALS, 0x01)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper2", FUNC(hopper_device::line_r)) PORT_CONDITION("CONF", 0x01, EQUALS, 0x01)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper1", FUNC(hopper_device::line_r)) PORT_CONDITION("CONF", 0x01, EQUALS, 0x01)
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper3", FUNC(hopper_device::line_r)) PORT_CONDITION("CONF", 0x01, EQUALS, 0x00)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper2", FUNC(hopper_device::line_r)) PORT_CONDITION("CONF", 0x01, EQUALS, 0x00)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_CUSTOM) PORT_READ_LINE_DEVICE_MEMBER("hopper1", FUNC(hopper_device::line_r)) PORT_CONDITION("CONF", 0x01, EQUALS, 0x00)
INPUT_PORTS_END



/**************************************************************************
    Machine emulation
**************************************************************************/

void design_state::port1_w(u8 data)
{
	// bit 6 is low during SRAM writes (write enable, assumption)
	// bit 7 is pulsed all the time (watchdog, assumption)
	m_port1 = data;
	m_xdata_view.select(BIT(data, 5));
}

u8 design_state::keys_r()
{
	u8 data = 0;

	for (unsigned i = 0; i < 4; i++)
		if (BIT(m_port1, i))
			data |= m_io_keys[i]->read();

	return data;
}

// 0 = no change available, 1 = sold out, 2 = sale completed; probably
// the optional voice synthesizer kit (assumption)
void design_state::voice_w(u8 data)
{
	LOGVOICE("%s: message %u\n", machine().describe_context(), data);
}

ioport_value design_state::flaps_r()
{
	return m_io_flaps->read() & (m_flap_raised ? 0 : 7);
}

void design_state::enable_w(int state)
{
	m_coinsel_enable = state;
}

template <unsigned N>
void design_state::extractors_w(u8 data)
{
	const u32 old = m_extractors;
	m_extractors = (m_extractors & ~(0xffU << (N * 8))) | (u32(data) << (N * 8));

	for (unsigned i = 0; i < 8; i++)
		m_out_extractor[N * 8 + i] = BIT(data, i);

	// (simulated) a product lifts the flaps a while after a motor starts
	if (!old && m_extractors)
		m_flap_timer->adjust(attotime::from_msec(500));
	else if (old && !m_extractors && !m_flap_raised)
		m_flap_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(design_state::flap_update)
{
	m_flap_raised = !m_flap_raised;
	m_out_flap = m_flap_raised;

	if (m_flap_raised)
		m_flap_timer->adjust(attotime::from_msec(200));
	else if (m_extractors)
		m_flap_timer->adjust(attotime::from_msec(500));
}

void design_state::recovery_motor_w(int state)
{
	if (bool(state) == m_recovery_motor)
		return;

	m_recovery_motor = state;

	// (simulated) position switch
	if (m_recovery_motor)
		m_recovery_timer->adjust(attotime::from_msec(m_recovery_pos ? 400 : 150));
	else
		m_recovery_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER(design_state::recovery_update)
{
	m_recovery_pos = !m_recovery_pos;
	m_recovery_timer->adjust(attotime::from_msec(m_recovery_pos ? 400 : 150));
}

/*
    designe: while idle, a high pulse of about 10-45 timer ticks on IN1
    bit 6 enables sales in the "MANDO" adult access modes (the default);
    longer levels are the recovery motor position.  The remote receiver
    is presumably wired here (assumption).
*/

INPUT_CHANGED_MEMBER(design_state::remote_pressed)
{
	if (newval && !m_remote_pulse)
	{
		m_remote_pulse = true;
		m_remote_timer->adjust(attotime::from_msec(30));
	}
}

TIMER_CALLBACK_MEMBER(design_state::remote_release)
{
	m_remote_pulse = false;
}


/*
    Coin selector (simulated)

    The firmware gives credit only after reading the same code twice, with
    code 0 in between.  Code 0 is idle, 0xf is ignored and 0xe is awaited
    while the escrow outputs are on.  Pulse lengths and delays are made up.
    Coins are rejected while latch 0 bit 3 is low (assumption).
*/

INPUT_CHANGED_MEMBER(design_state::coin_inserted)
{
	if (!newval || (m_coinsel_state != COINSEL_IDLE))
		return;

	if (!m_coinsel_enable)
	{
		LOGCOINSEL("coin code %02x rejected, selector inhibited\n", param);
		return;
	}

	LOGCOINSEL("coin inserted, code %02x\n", param);

	m_coinsel_coin = param;
	m_coinsel_state = COINSEL_VALIDATE;
	m_coinsel_code = m_coinsel_coin;
	m_coinsel_timer->adjust(attotime::from_msec(50));
}

TIMER_CALLBACK_MEMBER(design_state::coinsel_update)
{
	switch (m_coinsel_state)
	{
	case COINSEL_VALIDATE:
		m_coinsel_code = 0;
		m_coinsel_state = COINSEL_TRAVEL;
		m_coinsel_timer->adjust(attotime::from_msec(100));
		break;

	case COINSEL_TRAVEL:
		m_coinsel_code = m_coinsel_coin;
		m_coinsel_state = COINSEL_CONFIRM;
		m_coinsel_timer->adjust(attotime::from_msec(50));
		break;

	case COINSEL_CONFIRM:
		m_coinsel_code = 0;
		m_coinsel_state = COINSEL_IDLE;
		break;

	default:
		m_coinsel_code = 0;
		m_coinsel_state = COINSEL_IDLE;
		break;
	}
}


void design_state::machine_start()
{
	m_coinsel_timer = timer_alloc(FUNC(design_state::coinsel_update), this);
	m_recovery_timer = timer_alloc(FUNC(design_state::recovery_update), this);
	m_flap_timer = timer_alloc(FUNC(design_state::flap_update), this);
	m_remote_timer = timer_alloc(FUNC(design_state::remote_release), this);

	save_item(NAME(m_port1));
	save_item(NAME(m_coinsel_state));
	save_item(NAME(m_coinsel_coin));
	save_item(NAME(m_coinsel_code));
	save_item(NAME(m_coinsel_enable));
	save_item(NAME(m_recovery_motor));
	save_item(NAME(m_recovery_pos));
	save_item(NAME(m_extractors));
	save_item(NAME(m_flap_raised));
	save_item(NAME(m_remote_pulse));
}

void design_state::machine_reset()
{
	m_port1 = 0xff;
	m_xdata_view.select(1);

	m_coinsel_state = COINSEL_IDLE;
	m_coinsel_code = 0;
	m_coinsel_timer->adjust(attotime::never);
}



/**************************************************************************
    Machine configuration
**************************************************************************/

void design_state::design6(machine_config &config)
{
	I8051(config, m_maincpu, 6'000'000); // Unknown XTAL frequency
	m_maincpu->set_addrmap(AS_PROGRAM, &design_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &design_state::data_map);
	m_maincpu->port_out_cb<1>().set(FUNC(design_state::port1_w));
	m_maincpu->port_in_cb<3>().set_ioport("P3");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// latch types as on the Azkoyen T series boards (assumption)
	cd4099_device &outlatch0(CD4099(config, "outlatch0"));
	outlatch0.q_out_cb<0>().set(m_hopper[0], FUNC(hopper_device::motor_w));
	outlatch0.q_out_cb<0>().append_output("hopper_motor1");
	outlatch0.q_out_cb<1>().set(m_hopper[1], FUNC(hopper_device::motor_w));
	outlatch0.q_out_cb<1>().append_output("hopper_motor2");
	outlatch0.q_out_cb<2>().set(m_hopper[2], FUNC(hopper_device::motor_w));
	outlatch0.q_out_cb<2>().append_output("hopper_motor3");
	outlatch0.q_out_cb<3>().set(FUNC(design_state::enable_w)); // on unless out of service; selector general inhibit (assumption)
	outlatch0.q_out_cb<3>().append_output("enable");
	outlatch0.q_out_cb<4>().set_output("escrow_return"); // on during coin return; escrow (assumption)
	outlatch0.q_out_cb<5>().set_output("escrow_collect"); // on after a sale; escrow (assumption)
	outlatch0.q_out_cb<6>().set(FUNC(design_state::recovery_motor_w)); // "AVERIA RECUP."
	outlatch0.q_out_cb<6>().append_output("recovery_motor");
	outlatch0.q_out_cb<7>().set_output("coin_valid"); // pulsed when a valid coin code is read; not a selector input according to the L66S manual

	cd4099_device &outlatch1(CD4099(config, "outlatch1"));
	outlatch1.q_out_cb<0>().set_output("sorter_coil1"); // sorter coils, order relative to the manual's numbering unknown
	outlatch1.q_out_cb<1>().set_output("sorter_coil2");
	outlatch1.q_out_cb<2>().set_output("sorter_coil3");
	outlatch1.q_out_cb<5>().set("vfd", FUNC(roc10937_device::data));
	outlatch1.q_out_cb<6>().set("vfd", FUNC(roc10937_device::sclk));
	outlatch1.q_out_cb<7>().set("vfd", FUNC(roc10937_device::por));

	// ROM tables map each product channel to one of these outputs
	CD4099(config, "outlatch2").parallel_out_cb().set(FUNC(design_state::extractors_w<0>));
	CD4099(config, "outlatch3").parallel_out_cb().set(FUNC(design_state::extractors_w<1>));
	CD4099(config, "outlatch4").parallel_out_cb().set(FUNC(design_state::extractors_w<2>));

	// (simulated) coin hoppers
	for (auto &hopper : m_hopper)
		HOPPER(config, hopper, attotime::from_msec(100));

	ROC10937(config, "vfd");

	MSM6242(config, "rtc", 32.768_kHz_XTAL);

	config.set_default_layout(layout_design6);
}

void design_state::designe(machine_config &config)
{
	design6(config);

	config.set_default_layout(layout_designe);
}



/**************************************************************************
    ROM definitions
**************************************************************************/

ROM_START( design6 )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x8000, CRC(d3823da8) SHA1(bc1661727643c63e2ed94841f2e7c0a305333a10))

	ROM_REGION16_LE(0x4010, "coinsel", ROMREGION_ERASEFF)
	ROM_LOAD("pic16x76_l56s-l66s.bin", 0x0000, 0x4010, NO_DUMP)
ROM_END

ROM_START( designe )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("designe.bin", 0x0000, 0x8000, CRC(693d40bd) SHA1(9596bbf9c367bc919393923460da15563d9447ca))

	ROM_REGION16_LE(0x4010, "coinsel", ROMREGION_ERASEFF)
	ROM_LOAD("pic16x76_l56s-l66s.bin", 0x0000, 0x4010, NO_DUMP)
ROM_END


} // anonymous namespace



/**************************************************************************
    System drivers
**************************************************************************/

//    YEAR   NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS         INIT        COMPANY    FULLNAME                                FLAGS
SYST( 1995?, design6, 0,      0,      design6, design6, design_state, empty_init, "Azkoyen", "Design D6 (pesetas)",                  MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
SYST( 2006,  designe, 0,      0,      designe, designe, design_state, empty_init, "Azkoyen", "Design (euro, with adult remote kit)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // Adult remote kit is P/N 43521600-5

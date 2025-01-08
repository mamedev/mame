// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2echoii.c

    Implementation of the Street Electronics Echo II and EchoIIb speech card
    Ready logic traced by Lord Nightmare and Tony Diaz

    Notes from Tony Diaz:
        Capacitor values:
        C1,C2 - .47nF  C3,C4 - 16v 10uF   C5 - 16v 100uF   C7,C7,C8 .1uF

    Pictures:
    Original EchoII card, S/N 789 with ?original? TMS5200: http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/Speech/Street%20Echo%20II/Photos/SEC%20-%20Echo%20II%20rev.A%20-%20Front.jpg
    Original EchoII card, S/N 103, with a much later 1987 non-original TSP5220C installed: https://upload.wikimedia.org/wikipedia/en/e/ee/Echo2Card.jpg
    Later EchoII, S/N 1793, with original TMS5220, no VSM socket footprints, and with the 'FREQ' potentiometer: http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/Speech/Street%20Echo%20II/Photos/Echo%20II%20-%20Front.jpg
    EchoIIb, similar to later EchoII but with soldered-down original TMS5220C, no 'FREQ' potentiometer and with the 74LS92 clock divider: http://mirrors.apple2.org.za/Apple%20II%20Documentation%20Project/Interface%20Cards/Speech/Street%20Echo%20II/Photos/Echo%20IIb%20-%20Front.jpg

    TODO: separate cards for the EchoII(both the freq adjustable and the older non-adjustable version, and maybe a tms5200 version?), EchoIIb (tms5220c)
    TODO: echo+ is in a2mockingboard.cpp and really should be a sub-device inherited from here, to reduce duplicated code.
    TODO: find a hi-res picture of an echo+ card
*********************************************************************/

#include "emu.h"
#include "a2echoii.h"
#include "sound/tms5220.h"
#include "speaker.h"

#define LOG_READYQ (1U << 1)
#define LOG_READ   (1U << 2)
#define LOG_WRITE  (1U << 3)

//#define VERBOSE (LOG_READYQ | LOG_READ | LOG_WRITE)
#include "logmacro.h"


namespace {

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define TMS_TAG         "tms5220"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_echoii_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_echoii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<tms5220_device> m_tms;

protected:
	a2bus_echoii_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override;

private:
	//void tms_irq_callback(int state);
	void tms_readyq_callback(int state);
	uint8_t m_writelatch_data; // 74ls373 latch
	bool m_readlatch_flag; // 74c74 1st half
	bool m_writelatch_flag; // 74c74 2nd half
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_echoii_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "echoii").front_center();
	tms5220_device &tms(TMS5220(config, TMS_TAG, 640000));
	// Note the Echo II card has an R/C circuit (and sometimes a 'FREQ' potentiometer) to control the tms5220[c]'s clock frequency; 640khz is
	//   the nominal '8khz' value according to the TMS5220 datasheet.
	// The EchoIIb card however has a 74LS92 which divides the apple2's Q3 ((14.318/7)MHz asymmetrical) clock by 6 to produce a 681.809khz/2
	//   clock, which doesn't actually make sense, since the tms5220, unless it has a mask option (mentioned on the datasheet) to use a ceramic
	//   resonator instead of an r/c circuit, needs a clock at twice that speed. Could it be that the EchoIIb uses tsp5220C chips with a special
	//   mask option?
	// Some Old EchoII cards shipped with TMS5200(really?), some with TMS5220. Many (most?) were retrofitted with a TMS5220 or TMS5220C later.
	// The later VSM-socket-less EchoII shipped with a TMS5220.
	// The EchoIIb and later cards shipped with a TMS5220C
	//tms.irq_cb().set(FUNC(a2bus_echoii_device::tms_irq_callback));
	tms.ready_cb().set(FUNC(a2bus_echoii_device::tms_readyq_callback));
	tms.add_route(ALL_OUTPUTS, "echoii", 1.0);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_echoii_device::a2bus_echoii_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_tms(*this, TMS_TAG)
{
}

a2bus_echoii_device::a2bus_echoii_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_echoii_device(mconfig, A2BUS_ECHOII, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_echoii_device::device_start()
{
	m_writelatch_data = 0xff;
	m_readlatch_flag = true; // /RESET presets this latch
	m_writelatch_flag = true; // not initialized but we need to set it somewhere.
	save_item(NAME(m_writelatch_data));
	save_item(NAME(m_readlatch_flag));
	save_item(NAME(m_writelatch_flag));
}

void a2bus_echoii_device::device_reset()
{
	m_readlatch_flag = true; // /RESET presets this latch
	m_tms->rsq_w(m_readlatch_flag); // update the rsq pin
}

/*
void a2bus_echoii_device::tms_irq_callback(int state)
{
    update_irq_to_maincpu();
}
*/

void a2bus_echoii_device::tms_readyq_callback(int state)
{
	if (state == ASSERT_LINE)
	{
		LOGMASKED(LOG_READYQ,"ReadyQ callback called with state of %d! NOT READY\n", state);
		// the rising edge of /READY doesn't really do anything.
	}
	else
	{
		LOGMASKED(LOG_READYQ,"ReadyQ callback called with state of %d! READY\n", state);
		m_writelatch_flag = true;
		m_tms->wsq_w(m_writelatch_flag);
	}
}

uint8_t a2bus_echoii_device::read_c0nx(uint8_t offset)
{
	// offset is completely ignored on the echoii (but not so on the echo+), so the same register maps to the entire space.
	uint8_t retval = 0xff; // pull-up resistor pack on the tms5220 bus

	// upon the falling edge of /DEVREAD, the active part of the read...
	if (m_readlatch_flag == false) // /RS was low, so we need to return a value from the tms5220
	{
		retval = 0x1f | m_tms->status_r();
		LOGMASKED(LOG_READ,"Returning status of speech chip, which is %02x\n", retval);
	}
	else
		LOGMASKED(LOG_READ,"chip status read on odd cycle, returning pull-up value of %02x\n", retval);

	// upon the rising edge of /DEVREAD, i.e. after the read has finished (so no updating retval after this)
	m_readlatch_flag = (!m_readlatch_flag); // latch inverts itself upon each read...
	m_tms->rsq_w(m_readlatch_flag); // update the /RS pin
	return retval;
}

void a2bus_echoii_device::write_c0nx(uint8_t offset, uint8_t data)
{
	// offset is completely ignored on the echoii (but not so on the echo+), so the same register maps to the entire space.
	if (m_writelatch_flag == false)
		logerror("Data in echoii latch (%02x) was clobbered with new write (%02x) before being read by speech chip!\n", m_writelatch_data, data);
	else
		LOGMASKED(LOG_WRITE,"Data written to latch of %02x\n", data);

	m_writelatch_data = data;

	m_writelatch_flag = false; // /DEVWRITE clears the latch on its falling edge
	m_tms->wsq_w(m_writelatch_flag);
	m_tms->data_w(m_writelatch_data);
}

bool a2bus_echoii_device::take_c800()
{
	return false;
}

} // anonymous namespace


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_ECHOII, device_a2bus_card_interface, a2bus_echoii_device, "a2echoii", "Street Electronics Echo II")

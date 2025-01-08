// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    Dragon Alpha

    The Dragon Alpha was a prototype in development when Dragon Data went bust,
    it is basically an enhanced Dragon 64, with built in modem, disk system, and
    graphical boot rom.

    It has the following extra hardware :-
    A third 6821 PIA mapped between FF24 and FF27
        An AY-8912, connected to the PIA.

    Port A of the PIA is connected as follows :-

        b0  BDIR of AY8912
        b1  BC1 of AY8912
        b2  Rom select, High= boot rom, low=BASIC rom
        b3..7 not used.

    Port B
        b0..7 connected to D0..7 of the AY8912.

    CB1 DRQ of WD2797.

    /irqa
    /irqb   both connected to 6809 FIRQ.


    The analog outputs of the AY-8912 are connected to the standard sound multiplexer.
    The AY8912 output port is used as follows :-

        b0..b3  /DS0../DS3 for the drive interface (through an inverter first).
        b4      /motor for the drive interface (through an inverter first).
        b5..b7  not used as far as I can tell.

    A 6850 for the modem.

    A WD2797, used as an internal disk interface, this is however connected in a slightly strange
    way that I am yet to completely determine.
    19/10/2004, WD2797 is mapped between FF2C and FF2F, however the order of the registers is
    reversed so the command Register is at the highest address instead of the lowest. The Data
    request pin is connected to CB1(pin 18) of PIA2, to cause an firq, the INTRQ, is connected via
    an inverter to the 6809's NMI.

    All these are as yet un-emulated.

    29-Oct-2004, AY-8912 is now emulated.
    30-Oct-2004, Internal disk interface now emulated, Normal DragonDos rom replaced with a re-assembled
                version, that talks to the alpha hardware (verified on a clone of the real machine).

Dragon Alpha code added 21-Oct-2004,
            Phill Harvey-Smith (afra@aurigae.demon.co.uk)

            Added AY-8912 and FDC code 30-Oct-2004.

Fixed Dragon Alpha NMI enable/disable, following circuit traces on a real machine.
    P.Harvey-Smith, 11-Aug-2005.

Re-implemented Alpha NMI enable/disable, using direct PIA reads, rather than
keeping track of it in a variable in the driver.
    P.Harvey-Smith, 25-Sep-2006.

***************************************************************************/

#include "emu.h"
#include "dragon.h"

#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "sound/ay8910.h"

#include "softlist_dev.h"

#include "formats/dmk_dsk.h"
#include "formats/sdf_dsk.h"
#include "formats/vdk_dsk.h"


namespace {

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

/* devices */
#define WD2797_TAG                  "wd2797"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dragon_alpha_state : public dragon64_state
{
public:
	dragon_alpha_state(const machine_config &mconfig, device_type type, const char *tag) :
		dragon64_state(mconfig, type, tag),
		m_pia_2(*this, "pia2"),
		m_ay8912(*this, "ay8912"),
		m_fdc(*this, WD2797_TAG),
		m_floppy(*this, WD2797_TAG ":%u", 0U),
		m_nmis(*this, "nmis")
	{
	}

	void dgnalpha(machine_config &config);

private:
	static void dragon_formats(format_registration &fr);

	/* pia2 */
	void pia2_pa_w(uint8_t data);

	/* psg */
	uint8_t psg_porta_read();
	void psg_porta_write(uint8_t data);

	/* fdc */
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	void dgnalpha_io1(address_map &map) ATTR_COLD;

	required_device<pia6821_device> m_pia_2;
	required_device<ay8912_device> m_ay8912;
	required_device<wd2797_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<input_merger_device> m_nmis;

	/* modem */
	uint8_t modem_r(offs_t offset);
	void modem_w(offs_t offset, uint8_t data);
};



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void dragon_alpha_state::dgnalpha_io1(address_map &map)
{
	// $FF20-$FF3F
	map(0x00, 0x03).mirror(0x10).r(m_pia_1, FUNC(pia6821_device::read)).w(FUNC(coco12_state::ff20_write));
	map(0x04, 0x07).mirror(0x10).rw(m_pia_2, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x08, 0x0b).mirror(0x10).rw(FUNC(dragon_alpha_state::modem_r), FUNC(dragon_alpha_state::modem_w));
	map(0x0c, 0x0c).mirror(0x10).rw(m_fdc, FUNC(wd2797_device::data_r), FUNC(wd2797_device::data_w));
	map(0x0d, 0x0d).mirror(0x10).rw(m_fdc, FUNC(wd2797_device::sector_r), FUNC(wd2797_device::sector_w));
	map(0x0e, 0x0e).mirror(0x10).rw(m_fdc, FUNC(wd2797_device::track_r), FUNC(wd2797_device::track_w));
	map(0x0f, 0x0f).mirror(0x10).rw(m_fdc, FUNC(wd2797_device::status_r), FUNC(wd2797_device::cmd_w));
}


/***************************************************************************
  MODEM
***************************************************************************/

//-------------------------------------------------
//  modem_r
//-------------------------------------------------

uint8_t dragon_alpha_state::modem_r(offs_t offset)
{
	return 0xFF;
}



//-------------------------------------------------
//  modem_w
//-------------------------------------------------

void dragon_alpha_state::modem_w(offs_t offset, uint8_t data)
{
}



/***************************************************************************
  PIA2 ($FF24-$FF28) on Dragon Alpha/Professional

    PIA2 PA0        bcdir to AY-8912
    PIA2 PA1        bc0 to AY-8912
    PIA2 PA2        Rom switch, 0=basic rom, 1=boot rom.
    PIA2 PA3-PA7    Unknown/unused ?
    PIA2 PB0-PB7    connected to D0..7 of the AY8912.
    CB1             DRQ from WD2797 disk controller.
***************************************************************************/

//-------------------------------------------------
//  pia2_pa_w
//-------------------------------------------------

void dragon_alpha_state::pia2_pa_w(uint8_t data)
{
	uint8_t ddr = ~m_pia_2->port_b_z_mask();

	/* If bit 2 of the pia2 ddra is 1 then this pin is an output so use it */
	/* to control the paging of the boot and basic roms */
	/* Otherwise it set as an input, with an internal pull-up so it should */
	/* always be high (enabling boot rom) */
	/* PIA FIXME if (pia_get_ddr_a(2) & 0x04) */
	if(ddr & 0x04)
	{
		page_rom(data & 0x04 ? true : false);   /* bit 2 controls boot or basic rom */
	}

	/* Bits 0 and 1 for pia2 port a control the BCDIR and BC1 lines of the */
	/* AY-8912 */
	switch (data & 0x03)
	{
		case 0x00:      /* Inactive, do nothing */
			break;
		case 0x01:      /* Write to selected port */
			m_ay8912->data_w(m_pia_2->b_output());
			break;
		case 0x02:      /* Read from selected port */
			m_pia_2->portb_w(m_ay8912->data_r());
			break;
		case 0x03:      /* Select port to write to */
			m_ay8912->address_w(m_pia_2->b_output());
			break;
	}
}



/***************************************************************************
  AY8912
***************************************************************************/

//-------------------------------------------------
//  psg_porta_read
//-------------------------------------------------

uint8_t dragon_alpha_state::psg_porta_read()
{
	return 0;
}



//-------------------------------------------------
//  psg_porta_read
//-------------------------------------------------

void dragon_alpha_state::psg_porta_write(uint8_t data)
{
	/* Bits 0..3 are the drive select lines for the internal floppy interface */
	/* Bit 4 is the motor on, in the real hardware these are inverted on their way to the drive */
	/* Bits 5,6,7 are connected to /DDEN, ENP and 5/8 on the WD2797 */

	floppy_image_device *floppy = nullptr;

	for (int n = 0; n < 4; n++)
		if (BIT(data, n))
			floppy = m_floppy[n]->get_device();

	m_fdc->set_floppy(floppy);

	// todo: turning the motor on with bit 4 isn't giving the drive enough
	// time to spin up, how does it work in hardware?
	for (auto &f : m_floppy)
		if (f->get_device()) f->get_device()->mon_w(0);

	m_fdc->dden_w(BIT(data, 5));
}

/***************************************************************************
  FDC
***************************************************************************/

//-------------------------------------------------
//  fdc_intrq_w - The NMI line on the Alpha is gated
//  through IC16 (early PLD), and is gated by pia2 CA2
//-------------------------------------------------

void dragon_alpha_state::fdc_intrq_w(int state)
{
	if (state)
	{
		if (m_pia_2->ca2_output_z())
			m_nmis->in_w<1>(1);
	}
	else
	{
		m_nmis->in_w<1>(0);
	}
}



//-------------------------------------------------
//  fdc_drq_w - The DRQ line goes through pia2 CB1,
//  in exactly the same way as DRQ from DragonDos
//  does for pia1 CB1
//-------------------------------------------------

void dragon_alpha_state::fdc_drq_w(int state)
{
	m_pia_2->cb1_w(state ? ASSERT_LINE : CLEAR_LINE);
}


void dragon_alpha_state::dragon_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_VDK_FORMAT);
	fr.add(FLOPPY_DMK_FORMAT);
	fr.add(FLOPPY_SDF_FORMAT);
}

static void dragon_alpha_floppies(device_slot_interface &device)
{
	device.option_add("dd", FLOPPY_35_DD);
}


void dragon_alpha_state::dgnalpha(machine_config &config)
{
	dragon_base(config);

	// internal ram
	RAM(config, RAM_TAG).set_default_size("64K");

	sam().set_addrmap(1, &dragon_alpha_state::d64_rom0);
	sam().set_addrmap(2, &dragon_alpha_state::d64_rom1);
	sam().set_addrmap(4, &dragon_alpha_state::d64_io0);
	sam().set_addrmap(5, &dragon_alpha_state::dgnalpha_io1);

	// input merger
	INPUT_MERGER_ANY_HIGH(config, m_nmis).output_handler().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// cartridge
	COCOCART_SLOT(config, m_cococart, DERIVED_CLOCK(1, 1), &dragon_alpha_state::dragon_cart, nullptr);
	m_cococart->cart_callback().set([this] (int state) { cart_w(state != 0); }); // lambda because name is overloaded
	m_cococart->nmi_callback().set(m_nmis, FUNC(input_merger_device::in_w<0>));
	m_cococart->halt_callback().set_inputline(m_maincpu, INPUT_LINE_HALT);

	// acia
	mos6551_device &acia(MOS6551(config, "acia", 0));
	acia.set_xtal(1.8432_MHz_XTAL);

	// floppy
	WD2797(config, m_fdc, 4_MHz_XTAL/4);
	m_fdc->intrq_wr_callback().set(FUNC(dragon_alpha_state::fdc_intrq_w));
	m_fdc->drq_wr_callback().set(FUNC(dragon_alpha_state::fdc_drq_w));

	FLOPPY_CONNECTOR(config, WD2797_TAG ":0", dragon_alpha_floppies, "dd", dragon_alpha_state::dragon_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, WD2797_TAG ":1", dragon_alpha_floppies, "dd", dragon_alpha_state::dragon_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, WD2797_TAG ":2", dragon_alpha_floppies, nullptr, dragon_alpha_state::dragon_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, WD2797_TAG ":3", dragon_alpha_floppies, nullptr, dragon_alpha_state::dragon_formats).enable_sound(true);

	// sound hardware
	AY8912(config, m_ay8912, 4_MHz_XTAL/4);
	m_ay8912->port_a_read_callback().set(FUNC(dragon_alpha_state::psg_porta_read));
	m_ay8912->port_a_write_callback().set(FUNC(dragon_alpha_state::psg_porta_write));
	m_ay8912->add_route(ALL_OUTPUTS, "speaker", 0.75);

	// pia 2
	PIA6821(config, m_pia_2);
	m_pia_2->writepa_handler().set(FUNC(dragon_alpha_state::pia2_pa_w));
	m_pia_2->irqa_handler().set(m_firqs, FUNC(input_merger_device::in_w<2>));
	m_pia_2->irqb_handler().set(m_firqs, FUNC(input_merger_device::in_w<3>));

	// software lists
	SOFTWARE_LIST(config, "dgnalpha_flop_list").set_original("dgnalpha_flop");
	SOFTWARE_LIST(config, "dragon_flex_list").set_original("dragon_flex");
	SOFTWARE_LIST(config, "dragon_os9_list").set_original("dragon_os9");
}

ROM_START(dgnalpha)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_DEFAULT_BIOS("boot10")
	ROM_SYSTEM_BIOS(0, "boot10", "Boot v1.0")
	ROMX_LOAD("alpha_bt_10.rom", 0x2000,  0x2000, CRC(c3dab585) SHA1(4a5851aa66eb426e9bb0bba196f1e02d48156068), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "boot04", "Boot v0.4")
	ROMX_LOAD("alpha_bt_04.rom", 0x2000,  0x2000, CRC(d6172b56) SHA1(69ea376dbc7418f69e9e809b448d22a4de012344), ROM_BIOS(1))
	ROM_LOAD("alpha_ba.rom",    0x8000,  0x4000, CRC(84f68bf9) SHA1(1983b4fb398e3dd9668d424c666c5a0b3f1e2b69))
ROM_END

} // anonymous namespace


COMP( 1984, dgnalpha,   dragon32, 0,      dgnalpha,   dragon,     dragon_alpha_state, empty_init, "Dragon Data Ltd",              "Dragon Professional (Alpha)",    0 )

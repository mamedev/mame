// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    ACT Apricot Winchester Controller

    Version Rev 9

***************************************************************************/

#include "emu.h"
#include "winchester.h"
#include "imagedev/harddriv.h"
#include "machine/74259.h"

//#define LOG_GENERAL (1U <<  0)
#define LOG_REGS    (1U <<  1)
#define LOG_DATA    (1U <<  2)

//#define VERBOSE  (LOG_REGS)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGREGS(...)  LOGMASKED(LOG_REGS,  __VA_ARGS__)
#define LOGDATA(...)  LOGMASKED(LOG_DATA,  __VA_ARGS__)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(APRICOT_WINCHESTER, apricot_winchester_device, "apricot_winchester", "Apricot Winchester Controller Board")

//-------------------------------------------------
//  regs - controller register i/o map
//-------------------------------------------------

void apricot_winchester_device::regs(address_map &map)
{
	map(0x0e0, 0x0ef).rw(m_hdc, FUNC(wd1010_device::read), FUNC(wd1010_device::write)).umask16(0x00ff);
	map(0x1e0, 0x1e0).r(FUNC(apricot_winchester_device::int_r));
	map(0x1e0, 0x1ef).w("latch", FUNC(ls259_device::write_d0)).umask16(0x00ff);
	map(0x1f0, 0x1f0).rw(FUNC(apricot_winchester_device::data_r), FUNC(apricot_winchester_device::data_w)).umask16(0x00ff);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void apricot_winchester_device::device_add_mconfig(machine_config &config)
{
	WD1010(config, m_hdc, 5000000);
	m_hdc->out_intrq_callback().set(FUNC(apricot_winchester_device::hdc_intrq_w));
	m_hdc->in_data_callback().set(FUNC(apricot_winchester_device::hdc_data_r));
	m_hdc->out_data_callback().set(FUNC(apricot_winchester_device::hdc_data_w));

	ls259_device &latch(LS259(config, "latch")); // IC6
	latch.q_out_cb<0>().set(FUNC(apricot_winchester_device::head_w<0>));
	latch.q_out_cb<1>().set(FUNC(apricot_winchester_device::head_w<1>));
	latch.q_out_cb<2>().set(FUNC(apricot_winchester_device::head_w<2>));
	latch.q_out_cb<3>().set(FUNC(apricot_winchester_device::drive_w<0>));
	latch.q_out_cb<4>().set(FUNC(apricot_winchester_device::xferd_w)); // transferred
	latch.q_out_cb<5>().set(FUNC(apricot_winchester_device::hbcr_w)); // host buffer clear register
	latch.q_out_cb<6>().set(FUNC(apricot_winchester_device::clksel_w)); // buffer chip read/write clock select (host or wdc)
	latch.q_out_cb<7>().set(FUNC(apricot_winchester_device::drive_w<1>));

	HARDDISK(config, "hdc:0", 0);
	HARDDISK(config, "hdc:1", 0);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  apricot_winchester_device - constructor
//-------------------------------------------------

apricot_winchester_device::apricot_winchester_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, APRICOT_WINCHESTER, tag, owner, clock),
	device_apricot_expansion_card_interface(mconfig, *this),
	m_hdc(*this, "hdc"),
	m_ram_ptr(0),
	m_int(0),
	m_drive(0),
	m_head(0),
	m_hbcr(0),
	m_clksel(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void apricot_winchester_device::device_start()
{
	// allocate 8-bit buffer ram (8k)
	m_ram = std::make_unique<uint8_t[]>(0x2000);

	// register for save states
	save_pointer(NAME(m_ram), 0x2000);
	save_item(NAME(m_ram_ptr));
	save_item(NAME(m_int));
	save_item(NAME(m_drive));
	save_item(NAME(m_head));
	save_item(NAME(m_hbcr));
	save_item(NAME(m_clksel));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void apricot_winchester_device::device_reset()
{
	m_bus->install_io_device(0x000, 0xfff, *this, &apricot_winchester_device::regs);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER( apricot_winchester_device::hdc_intrq_w )
{
	LOGREGS("hdc_intrq_w: %d\n", state);

	m_int = state;
	m_bus->int2_w(state);
}

uint8_t apricot_winchester_device::hdc_data_r()
{
	uint8_t data = 0xff;

	if (m_clksel == 1)
	{
		LOGDATA("hdc_data_r[%04x]\n", m_ram_ptr);

		data = m_ram[m_ram_ptr];
		if (!machine().side_effects_disabled() && (m_ram_ptr < 0x1fff))
			m_ram_ptr++;
	}

	return data;
}

void apricot_winchester_device::hdc_data_w(uint8_t data)
{
	if (m_clksel == 1)
	{
		LOGDATA("hdc_data_w[%04x] = %02x\n", m_ram_ptr, data);

		m_ram[m_ram_ptr] = data;
		if (m_ram_ptr < 0x1fff)
			m_ram_ptr++;
	}
}

uint8_t apricot_winchester_device::int_r()
{
	return m_int;
}

template<int N>
WRITE_LINE_MEMBER( apricot_winchester_device::head_w )
{
	m_head = (m_head & ~(1 << N)) | (state << N);
	LOGREGS("Select head: %d\n", m_head);
}

template<int N>
WRITE_LINE_MEMBER( apricot_winchester_device::drive_w )
{
	m_drive = (m_drive & ~(1 << N)) | (state << N);
	LOGREGS("Select drive: %d\n", m_drive);

	// forward drive status to the hdc
	harddisk_image_device *drive = nullptr;

	switch (m_drive)
	{
	case 0:
		drive = nullptr;
		break;
	case 1:
		drive = m_hdc->subdevice<harddisk_image_device>("0");
		break;
	case 2:
		drive = m_hdc->subdevice<harddisk_image_device>("1");
		break;
	case 3:
		// both (invalid?)
		drive = nullptr;
		break;
	}

	m_hdc->drdy_w(drive != nullptr && drive->exists());
}

WRITE_LINE_MEMBER( apricot_winchester_device::xferd_w )
{
	LOGREGS("xferd_w: %02x\n", state);

	m_hdc->brdy_w(state);
}

WRITE_LINE_MEMBER( apricot_winchester_device::hbcr_w )
{
	LOGREGS("hbcr_w: %02x\n", state);

	// reset ram pointer on high->low transition
	if (m_hbcr == 1 && !state)
		m_ram_ptr = 0;

	m_hbcr = state;
}

WRITE_LINE_MEMBER( apricot_winchester_device::clksel_w )
{
	LOGREGS("clksel_w: %02x\n", state);

	m_clksel = state;
}

uint8_t apricot_winchester_device::data_r()
{
	uint8_t data = 0xff;

	if (m_clksel == 0)
	{
		LOGDATA("data_r[%04x]\n", m_ram_ptr);

		data = m_ram[m_ram_ptr];

		// wrap or stop at end?
		if (m_ram_ptr < 0x1fff)
			m_ram_ptr++;
	}

	return data;
}

void apricot_winchester_device::data_w(uint8_t data)
{
	if (m_clksel == 0)
	{
		LOGDATA("data_w[%04x] = %02x\n", m_ram_ptr, data);

		m_ram[m_ram_ptr] = data;

		// wrap or stop at end?
		if (m_ram_ptr < 0x1fff)
			m_ram_ptr++;
	}
}

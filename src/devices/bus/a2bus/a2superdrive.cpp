// license:BSD-3-Clause
// copyright-holders:Kelvin Sherlock
/*********************************************************************

    a2superdrive.cpp

    Apple II 3.5" Disk Controller Card (Apple, 1991)
    aka SuperDrive Card, aka NuMustang (development codename)

    32k ram
    32k eprom
    15.6672Mhz crystal
    SWIM 1 Chip (1987)
    65c02 processor (~2mhz)

    uc:

    $0000-$7fff is ram
    $8000-$ffff is rom
    $0a00-$0aff is i/o

    apple 2 bus:

    $c0n0-$c0nf - memory latch. selects a 1k window for $c800-$cbff
    $cn00-$cnff - uc ram ($7b00-$7bff)
    $c800-$cbff - uc ram ($0000-$3cff, based on memory latch)
    $cc00-$cfff - uc ram ($7c00-$7fff)

    spamming Control-D while booting will invoke the built-in
    diagnostics.  An alternative entry is Cx0DG from the monitor.

*********************************************************************/


#include "emu.h"
#include "a2superdrive.h"
#include "cpu/m6502/w65c02s.h"
#include "machine/applefdintf.h"
#include "machine/swim1.h"
#include "imagedev/floppy.h"

#define C16M    (15.6672_MHz_XTAL)


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_superdrive_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_superdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_superdrive_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

private:

	void m65c02_mem(address_map &map) ATTR_COLD;

	void m65c02_w(offs_t offset, uint8_t value);
	uint8_t m65c02_r(offs_t offset);

	void phases_w(uint8_t phases);
	void sel35_w(int sel35);
	void devsel_w(uint8_t devsel);
	void hdsel_w(int hdsel);


	required_device<cpu_device> m_65c02;
	required_region_ptr<u8> m_rom;
	required_shared_ptr<u8> m_ram;

	required_device<applefdintf_device> m_fdc;

	uint8_t m_bank_select;
	uint8_t m_side;
};


#define SUPERDRIVE_ROM_REGION  "superdrive_rom"


ROM_START( superdrive )
	ROM_REGION(0x8000, SUPERDRIVE_ROM_REGION, 0)
	ROM_LOAD( "341-0438-a.bin", 0x0000, 0x08000, CRC(c73ff25b) SHA1(440c3f84176c7b9f542da0b6ddf4fb13ec326c46) )
ROM_END


const tiny_rom_entry *a2bus_superdrive_device::device_rom_region() const
{
	return ROM_NAME( superdrive );
}

void a2bus_superdrive_device::m65c02_mem(address_map &map)
{
	map(0x0000, 0x7fff).ram().share(m_ram);
	map(0x0a00, 0x0aff).rw(FUNC(a2bus_superdrive_device::m65c02_r), FUNC(a2bus_superdrive_device::m65c02_w));
	map(0x8000, 0xffff).rom().region(SUPERDRIVE_ROM_REGION, 0x0000);
}


void a2bus_superdrive_device::device_add_mconfig(machine_config &config)
{

	W65C02S(config, m_65c02,  DERIVED_CLOCK(2, 7));  /* ~ 2.046 MHz */
	m_65c02->set_addrmap(AS_PROGRAM, &a2bus_superdrive_device::m65c02_mem);

	SWIM1(config, m_fdc, C16M);

	applefdintf_device::add_35_hd(config, "fdc:0");
	applefdintf_device::add_35_hd(config, "fdc:1");

	m_fdc->devsel_cb().set(FUNC(a2bus_superdrive_device::devsel_w));
	m_fdc->hdsel_cb().set(FUNC(a2bus_superdrive_device::hdsel_w));
	m_fdc->phases_cb().set(FUNC(a2bus_superdrive_device::phases_w));
	m_fdc->sel35_cb().set(FUNC(a2bus_superdrive_device::sel35_w));
}



a2bus_superdrive_device::a2bus_superdrive_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_65c02(*this, "superdrive_65c02"),
	m_rom(*this, SUPERDRIVE_ROM_REGION),
	m_ram(*this, "superdrive_ram"),
	m_fdc(*this, "fdc"),
	m_bank_select(0),
	m_side(0)
{ }

a2bus_superdrive_device::a2bus_superdrive_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	a2bus_superdrive_device(mconfig, A2BUS_SUPERDRIVE, tag, owner, clock)
{ }

void a2bus_superdrive_device::device_start()
{
	save_item(NAME(m_bank_select));
	save_item(NAME(m_side));
}


void a2bus_superdrive_device::device_reset()
{
	m_bank_select = 0;
	m_side = 0;
}


// overrides of standard a2bus slot functions
uint8_t a2bus_superdrive_device::read_c0nx(uint8_t offset)
{
	if (machine().side_effects_disabled()) return 0;

	m_bank_select = offset & 0x0f;
	return 0;
}

void a2bus_superdrive_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_bank_select = offset & 0x0f;
}

uint8_t a2bus_superdrive_device::read_cnxx(uint8_t offset)
{
	return m_ram[0x7b00 + offset];
}

void a2bus_superdrive_device::write_cnxx(uint8_t offset, uint8_t data)
{
	//m_ram[0x7b00 + offset] = data;
}

/*
 * $c800 - $cbff is uc RAM, controlled via c0nx
 * $cc00 - $cfff is uc RAM, hardcoded to $7c00 - $7fff
 */
uint8_t a2bus_superdrive_device::read_c800(uint16_t offset)
{
	unsigned address;

	if (offset < 0x400)
		address = (m_bank_select << 10) + offset;
	else
		address = 0x7c00 + offset - 0x400;

	return m_ram[address];
}

void a2bus_superdrive_device::write_c800(uint16_t offset, uint8_t data)
{
	unsigned address;

	if (offset < 0x400)
		address = (m_bank_select << 10) + offset;
	else
		address = 0x7c00 + offset - 0x400;

	m_ram[address] = data;
}


/* uc 65c02 i/o at $0a00 */
void a2bus_superdrive_device::m65c02_w(offs_t offset, uint8_t value)
{
	// $00-$0f = swim registers
	// $40 = head sel low
	// $41 = head sel high
	// $80 = diagnostic led on
	// $81 = diagnostic led off

	floppy_image_device *floppy = nullptr;

	if (offset < 16)
	{
		m_fdc->write(offset, value);
		return;
	}

	switch (offset)
	{
		case 0x40:
			m_side = 0;
			floppy = m_fdc->get_floppy();
			if (floppy) floppy->ss_w(m_side);
			break;

		case 0x41:
			m_side = 1;
			floppy = m_fdc->get_floppy();
			if (floppy) floppy->ss_w(m_side);
			break;

		case 0x80:
			logerror("LED on\n");
			break;

		case 0x81:
			logerror("LED off\n");
			break;

		default:
			logerror("write($0a%02x,%02x)\n", offset, value);
			break;
	}
}

uint8_t a2bus_superdrive_device::m65c02_r(offs_t offset)
{
	floppy_image_device *floppy = nullptr;

	if (offset < 16)
		return m_fdc->read(offset);

	if (machine().side_effects_disabled()) return 0;

	switch (offset)
	{
		case 0x40:
			m_side = 0;
			floppy = m_fdc->get_floppy();
			if (floppy) floppy->ss_w(m_side);
			break;

		case 0x41:
			m_side = 1;
			floppy = m_fdc->get_floppy();
			if (floppy) floppy->ss_w(m_side);
			break;

		case 0x80:
			logerror("LED on\n");
			break;

		case 0x81:
			logerror("LED off\n");
			break;

		default:
			logerror("read($0a%02x)\n", offset);
			break;
	}
	return 0;
}


void a2bus_superdrive_device::devsel_w(uint8_t devsel)
{
	floppy_image_device *floppy = nullptr;

	switch (devsel)
	{
		case 1:
			floppy = m_fdc->subdevice<floppy_connector>("0")->get_device();
			break;
		case 2:
			floppy = m_fdc->subdevice<floppy_connector>("1")->get_device();
			break;
	}

	m_fdc->set_floppy(floppy);
}

void a2bus_superdrive_device::phases_w(uint8_t phases)
{
	floppy_image_device *floppy = m_fdc->get_floppy();
	if (floppy)
		floppy->seek_phase_w(phases);
}


void a2bus_superdrive_device::sel35_w(int sel35)
{
}

void a2bus_superdrive_device::hdsel_w(int hdsel)
{
	/* Q3/HDSEL pin (ISM MODE register bit 5) is used to control the clock speed */
	/* MFM runs at 15.6672, GCR at 15.6672/2 */

	m_fdc->set_clock_scale( hdsel ? 1.0 : 0.5);
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_SUPERDRIVE, device_a2bus_card_interface, a2bus_superdrive_device, "a2superdrive", "Apple II 3.5\" Disk Controller Card")


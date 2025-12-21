// license:BSD-3-Clause
// copyright-holders:Golden Child, R. Belmont
/*********************************************************************

    a2ieee488.cpp
    Apple II IEEE-488 Interface Card

    Notes: The ROM will write values to CSWL (charout) and KSWL (keyin) for
    the input and output vectors.

    C138  lda #$03                                            A9 03
    C13A  sta <KSWL                                           85 38
    C13C  lda #$06                                            A9 06
    C13E  sta <CSWL                                           85 36
    C140  lda #$80                                            A9 80

    Upon executing PR#1, CSWL will have the value C100.

    Once this executes after PR#1 and a command is issued, the CSWL has vector of C106.
    However, the KSWL vector is FD03 instead of C103.

    0036  06 C1 03 FD

    Using both PR#1 and IN#1 in tandem does the following:

    Upon executing PR#1, CSWL will have the value C100.
    Upon executing IN#1, KSWL will have the value C100.
    After the IN#1 and a command being issued KSWL will have the proper value C103.

    So it needs both PR#x and IN#x in tandem for this to work properly, before any commands
    are issued.

    Some examples in the manual show using only PR#x but the apple will
    start a loop of generating junk @ characters.  This is because KSWL+1
    does not contain the slot ROM high address of Cx.

    Most examples in the manual show tandem PR#1 and IN#1 usage along with PR#0 and IN#0.

    Resetting the apple, a quick test will work (tandem PR#1 : IN#1 and PR#0 : IN#0)

    100 PR#1 : IN#1
    110 PRINT "SC1"
    120 PRINT "LF1"
    130 PRINT "DV1"
    140 PR#0 : IN#0

    The following will work, because it doesn't try to input anything, and
    the last IN#0 fixes the KSWL vector. (tandem PR#0 : IN#0 on exit)

    100 PR#1
    110 PRINT "SC1"
    120 PRINT "LF1"
    130 PRINT "DV1"
    140 PR#0 : IN#0

    This will generate junk characters on exit once it gets to the command line,
    needing a reset to fix the KSWL vector.

    100 PR#1
    110 PRINT "SC1"
    120 PRINT "LF1"
    130 PRINT "DV1"
    140 PR#0

    This will generate junk characters on exit once it gets to the INPUT A$,
    needing a reset to fix the KSWL vector.

    100 PR#1
    110 PRINT "SC1"
    140 PR#0
    145 INPUT A$

    The following will not trigger the runaway input as no commands have
    been issued to the card:

    100 PR#1
    140 PR#0
    145 INPUT A$

*********************************************************************/

#include "emu.h"
#include "a2ieee488.h"

#include "bus/ieee488/ieee488.h"
#include "machine/tms9914.h"

namespace {

class a2bus_ieee488_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_ieee488_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD {}
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;

	virtual u8 read_cnxx(u8 offset) override { return m_rom[offset]; };
	virtual uint8_t read_c800(uint16_t offset) override { return m_rom[offset]; }
	virtual bool take_c800() const override { return true; }

	virtual void reset_from_bus() override;

private:
	required_device<ieee488_device> m_ieee;
	required_device<tms9914_device> m_tms9914;
	required_region_ptr<u8> m_rom;
};

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_ieee488_device::device_add_mconfig(machine_config &config)
{
	TMS9914(config, m_tms9914, 1021800);
	// TMS9914 IRQ line is not hooked up
	m_tms9914->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_tms9914->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_tms9914->eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_tms9914->dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_tms9914->nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_tms9914->ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_tms9914->ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_tms9914->srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	m_tms9914->atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_tms9914->ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));

	IEEE488(config, m_ieee);
	m_ieee->eoi_callback().set(m_tms9914, FUNC(tms9914_device::eoi_w));
	m_ieee->dav_callback().set(m_tms9914, FUNC(tms9914_device::dav_w));
	m_ieee->nrfd_callback().set(m_tms9914, FUNC(tms9914_device::nrfd_w));
	m_ieee->ndac_callback().set(m_tms9914, FUNC(tms9914_device::ndac_w));
	m_ieee->ifc_callback().set(m_tms9914, FUNC(tms9914_device::ifc_w));
	m_ieee->srq_callback().set(m_tms9914, FUNC(tms9914_device::srq_w));
	m_ieee->atn_callback().set(m_tms9914, FUNC(tms9914_device::atn_w));
	m_ieee->ren_callback().set(m_tms9914, FUNC(tms9914_device::ren_w));
	IEEE488_SLOT(config, "ieee_dev", 0, hp_ieee488_devices, nullptr);

}

a2bus_ieee488_device::a2bus_ieee488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_ieee488_device(mconfig, A2BUS_IEEE488, tag, owner, clock)
{
}

a2bus_ieee488_device::a2bus_ieee488_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_ieee(*this, IEEE488_TAG),
	m_tms9914(*this, "tms9914"),
	m_rom(*this, "rom")
{
}

uint8_t a2bus_ieee488_device::read_c0nx(uint8_t offset)
{
	uint8_t data = 0xff;
	if (!machine().side_effects_disabled())
	{
		data = m_tms9914->read(offset & 0x07);
	}
	return data;
}

void a2bus_ieee488_device::write_c0nx(uint8_t offset, uint8_t data)
{
	m_tms9914->write(offset & 0x07, data);
}

void a2bus_ieee488_device::reset_from_bus()
{
	m_tms9914->reset();
}

ROM_START(a2ieee488)
	ROM_REGION(0x0800, "rom", 0)
	ROM_LOAD( "341-0059-a.bin", 0x0000, 0x0800, CRC(57a3ece1) SHA1(d5cd452bdb7f9ab5c1d77806ffea4b63da77f714) )
ROM_END

const tiny_rom_entry *a2bus_ieee488_device::device_rom_region() const
{
	return ROM_NAME(a2ieee488);
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_IEEE488, device_a2bus_card_interface, a2bus_ieee488_device, "a2ieee488", "Apple II IEEE-488 Interface")

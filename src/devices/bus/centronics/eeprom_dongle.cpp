// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    EEPROM based dongle

    TODO:
    - Identify manufacturer
    - Does it have more features?

***************************************************************************/

#include "emu.h"
#include "eeprom_dongle.h"

#include "machine/eepromser.h"

class eeprom_dongle_device : public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	eeprom_dongle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void input_data0(int state) override;
	virtual void input_data1(int state) override;
	virtual void input_data3(int state) override;

private:
	required_device<eeprom_serial_93c46_16bit_device> m_eeprom;

	void eeprom_do_w(int state);
};

eeprom_dongle_device::eeprom_dongle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, EEPROM_DONGLE, tag, owner, clock),
	device_centronics_peripheral_interface( mconfig, *this ),
	m_eeprom(*this, "eeprom")
{
}

void eeprom_dongle_device::input_data0(int state)
{
	m_eeprom->clk_write(state);
}

void eeprom_dongle_device::input_data1(int state)
{
	m_eeprom->di_write(state);
}

void eeprom_dongle_device::input_data3(int state)
{
	m_eeprom->cs_write(state);
}

void eeprom_dongle_device::eeprom_do_w(int state)
{
	output_ack(state ? 0 : 1); // inverted
}

void eeprom_dongle_device::device_start()
{
}

void eeprom_dongle_device::device_add_mconfig(machine_config &config)
{
	EEPROM_93C46_16BIT(config, m_eeprom);
	m_eeprom->do_callback().set(FUNC(eeprom_dongle_device::eeprom_do_w));
}

ROM_START( eeprom_dongle )
	ROM_REGION16_LE( 0x80, "eeprom", 0 )
	// hand-crafted. use serial BCPP-M962-PPH2-MMMM-VSMM-MM with this
	ROM_LOAD( "theos.bin", 0x00, 0x80, BAD_DUMP CRC(04b98d38) SHA1(fa6ff7ee7c25060981eaa0ec3fbd2102bf810612) )
ROM_END

const tiny_rom_entry *eeprom_dongle_device::device_rom_region() const
{
	return ROM_NAME( eeprom_dongle );
}

DEFINE_DEVICE_TYPE_PRIVATE(EEPROM_DONGLE, device_centronics_peripheral_interface, eeprom_dongle_device, "eeprom_dongle", "EEPROM Dongle")

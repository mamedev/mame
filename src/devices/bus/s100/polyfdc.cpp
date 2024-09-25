// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    PolyMorphic Systems Disk Controller

    This board controls up to three Shugart SA400 Minifloppy drives using
    only generic serial and parallel interface chips and TTL (and an
    onboard 4 MHz XTAL).

****************************************************************************/

#include "emu.h"
#include "polyfdc.h"

#include "machine/i8255.h"
#include "machine/mc6852.h"

class poly_fdc_device : public device_t, public device_s100_card_interface
{
public:
	// construction/destruction
	poly_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::DISK; }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_s100_card_interface overrides
	virtual u8 s100_sinp_r(offs_t offset) override;
	virtual void s100_sout_w(offs_t offset, u8 data) override;

private:
	void pa_w(u8 data);
	u8 pb_r();
	void pc_w(u8 data);

	// object finders
	required_device<mc6852_device> m_usrt;
	required_device<i8255_device> m_pio;
};

DEFINE_DEVICE_TYPE_PRIVATE(S100_POLY_FDC, device_s100_card_interface, poly_fdc_device, "polyfdc", "PolyMorphic Systems Disk Controller")

poly_fdc_device::poly_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, S100_POLY_FDC, tag, owner, clock)
	, device_s100_card_interface(mconfig, *this)
	, m_usrt(*this, "usrt")
	, m_pio(*this, "pio")
{
}

void poly_fdc_device::device_start()
{
}

u8 poly_fdc_device::s100_sinp_r(offs_t offset)
{
	if ((offset & 0x00f0) == 0x0020)
	{
		if (BIT(offset, 3))
			return m_pio->read(offset & 3);
		else
			return m_usrt->read(offset & 1);
	}

	return 0xff;
}

void poly_fdc_device::s100_sout_w(offs_t offset, u8 data)
{
	if ((offset & 0x00f0) == 0x20)
	{
		if (BIT(offset, 3))
			m_pio->write(offset & 3, data);
		else
			m_usrt->write(offset & 1, data);
	}
}

void poly_fdc_device::pa_w(u8 data)
{
	// PA0, PA1 are decoded onto drive select lines
	// DS1- (pin 10) = ~(~PA1 & PA0)
	// DS2- (pin 12) = ~(PA1 & ~PA0)
	// DS3- (pin 14) = ~(PA1 & PA0)
}

u8 poly_fdc_device::pb_r()
{
	// PB0 = SC0+ (from index/sector counting circuit)
	// PB1 = SC1+
	// PB2 = SC2+
	// PB3 = SC3+
	// PB4 = TR0+ (inverted from SA400 pin 26)
	// PB5 = WPRT+ (inverted from SA400 pin 28)
	// PB6 = PB7 = GND
	return 0;
}

void poly_fdc_device::pc_w(u8 data)
{
	// PC0 = INT+ (buffered onto VI5-)
	// PC2 = SCTR-
	// PC3 = MTRON+ (inverted onto SA400 pin 16)
	// PC4 = WCMD+
	// PC5 = RCMD+
	// PC6 = DS+ (inverted onto SA400 pin 18)
	// PC7 = STEP+ (inverted onto SA400 pin 20)
	m_bus->vi5_w(BIT(data, 0) ? ASSERT_LINE : CLEAR_LINE);
}

void poly_fdc_device::device_add_mconfig(machine_config &config)
{
	MC6852(config, m_usrt, 0); // E generated from PDBIN+ and PWR-

	I8255(config, m_pio);
	m_pio->out_pa_callback().set(FUNC(poly_fdc_device::pa_w));
	m_pio->in_pb_callback().set(FUNC(poly_fdc_device::pb_r));
	m_pio->out_pc_callback().set(FUNC(poly_fdc_device::pc_w));
	m_pio->tri_pc_callback().set_constant(0xfe);
}

// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    PowerBook Duo Floppy Dock
    This contains a discrete SWIM2 chip which allows connecting a standard
    external floppy drive with a PowerBook connector.  No ROM is necessary.

    By R. Belmont

***************************************************************************/

#include "emu.h"
#include "floppydock.h"

#include "machine/applefdintf.h"
#include "machine/swim2.h"

namespace {

class floppydock_device : public device_t, public device_pwrbkduo_card_interface
{
public:
	// construction/destruction
	floppydock_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	floppydock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	required_device<swim2_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;

	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	u16 swim_r(offs_t offset, u16 mem_mask);
	void swim_w(offs_t offset, u16 data, u16 mem_mask);
	void phases_w(uint8_t phases);
	void devsel_w(uint8_t devsel);
	void hdsel_w(uint8_t hdsel);

	floppy_image_device *m_cur_floppy;
	int m_hdsel;
};

void floppydock_device::device_add_mconfig(machine_config &config)
{
	SWIM2(config, m_fdc, 15.6672_MHz_XTAL);
	m_fdc->devsel_cb().set(FUNC(floppydock_device::devsel_w));
	m_fdc->phases_cb().set(FUNC(floppydock_device::phases_w));
	m_fdc->hdsel_cb().set(FUNC(floppydock_device::hdsel_w));

	applefdintf_device::add_35_hd(config, m_floppy[0]);
	applefdintf_device::add_35_nc(config, m_floppy[1]);
}

floppydock_device::floppydock_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	floppydock_device(mconfig, DUODOCK_FLOPPYDOCK, tag, owner, clock)
{
}

floppydock_device::floppydock_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_pwrbkduo_card_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy(*this, "fdc:%d", 0U)
{
}

void floppydock_device::device_start()
{
	pwrbkduo().install_device(0x50f16000, 0x50f17fff, emu::rw_delegate(*this, FUNC(floppydock_device::swim_r)), emu::rw_delegate(*this, FUNC(floppydock_device::swim_w)));
}

uint16_t floppydock_device::swim_r(offs_t offset, u16 mem_mask)
{
	if (!machine().side_effects_disabled())
	{
//      pwrbkduo().maincpu().adjust_icount(-5);
	}

	u16 result = m_fdc->read((offset >> 8) & 0xf);
	return result << 8;
}
void floppydock_device::swim_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		m_fdc->write((offset >> 8) & 0xf, data & 0xff);
	else
		m_fdc->write((offset >> 8) & 0xf, data >> 8);

//  pwrbkduo().maincpu().adjust_icount(-5);
}

void floppydock_device::phases_w(uint8_t phases)
{
	if (m_cur_floppy)
		m_cur_floppy->seek_phase_w(phases);
}

void floppydock_device::devsel_w(uint8_t devsel)
{
	if (devsel == 1)
		m_cur_floppy = m_floppy[0]->get_device();
	else if (devsel == 2)
		m_cur_floppy = m_floppy[1]->get_device();
	else
		m_cur_floppy = nullptr;

	m_fdc->set_floppy(m_cur_floppy);
	if (m_cur_floppy)
		m_cur_floppy->ss_w(m_hdsel);
}

void floppydock_device::hdsel_w(uint8_t hdsel)
{
	if (m_cur_floppy)
	{
		m_cur_floppy->ss_w(hdsel);
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(DUODOCK_FLOPPYDOCK, device_nubus_card_interface, floppydock_device, "floppydock", "Floppy Dock")


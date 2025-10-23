// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    AY mod (1.667 MHz clock).

    Variants of the mod used 1.5 or 1.75 MHz clock.

    Reads are passed through to external slot, usually occupied by
    joystick interface.

***************************************************************************/

#include "emu.h"
#include "ay.h"

#include "sound/ay8910.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_ay_device

class bk_ay_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_ay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD {};

	virtual uint16_t io_r() override;
	virtual void io_w(uint16_t data, bool word) override;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<ym2149_device> m_ay;
	required_device<bk_parallel_slot_device> m_up;
};


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_ay_device - constructor
//-------------------------------------------------

bk_ay_device::bk_ay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_AY, tag, owner, clock)
	, device_bk_parallel_interface(mconfig, *this)
	, m_ay(*this, "ay")
	, m_up(*this, "up")
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bk_ay_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2149(config, m_ay, 1667000);
	m_ay->add_route(0, "rspeaker", 0.5);
	m_ay->add_route(2, "rspeaker", 0.5);
	m_ay->add_route(1, "lspeaker", 0.5);
	m_ay->add_route(2, "lspeaker", 0.5);

	BK_PARALLEL_SLOT(config, m_up, DERIVED_CLOCK(1, 1), bk_parallel_devices, nullptr);
}

uint16_t bk_ay_device::io_r()
{
	return m_up->read() ^ 0xffff;
}

void bk_ay_device::io_w(uint16_t data, bool word)
{
	if (word)
		m_ay->address_w(data);
	else
		m_ay->data_w(data);
	m_up->write(0, data ^ 0xffff, word);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_AY, device_qbus_card_interface, bk_ay_device, "bk_ay", "BK Single AY Interface")

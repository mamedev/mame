// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Mono Covox interface with passthrough for loopback cart
    (used by "In Your Space" demo.)

***************************************************************************/

#include "emu.h"
#include "covox.h"

#include "sound/dac.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_covox_device

class bk_covox_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_covox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD {};

	virtual uint16_t io_r() override;
	virtual void io_w(uint16_t data, bool word) override;

private:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<dac_byte_interface> m_dac;
	required_device<bk_parallel_slot_device> m_up;
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_covox_device - constructor
//-------------------------------------------------

bk_covox_device::bk_covox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_COVOX, tag, owner, clock)
	, device_bk_parallel_interface(mconfig, *this)
	, m_dac(*this, "dac")
	, m_up(*this, "up")
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bk_covox_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "covox").front_center();
	DAC_8BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "covox", 0.5); // unknown DAC

	BK_PARALLEL_SLOT(config, m_up, DERIVED_CLOCK(1, 1), bk_parallel_devices, nullptr);
}

uint16_t bk_covox_device::io_r()
{
	return m_up->read() ^ 0xffff;
}

void bk_covox_device::io_w(uint16_t data, bool word)
{
	m_dac->write(data);
	m_up->write(0, data ^ 0xffff, word);
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_COVOX, device_qbus_card_interface, bk_covox_device, "bk_covox", "BK Mono Covox Interface")

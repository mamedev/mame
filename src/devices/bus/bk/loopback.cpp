// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Loopback cart for ROM test routines

***************************************************************************/

#include "emu.h"
#include "loopback.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bk_loopback_device

class bk_loopback_device : public device_t, public device_bk_parallel_interface
{
public:
	// construction/destruction
	bk_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD { m_data = 0; };

	virtual uint16_t io_r() override;
	virtual void io_w(uint16_t data, bool word) override;

private:
	uint16_t m_data;
};

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bk_loopback_device - constructor
//-------------------------------------------------

bk_loopback_device::bk_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BK_LOOPBACK, tag, owner, clock)
	, device_bk_parallel_interface(mconfig, *this)
{
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

void bk_loopback_device::device_start()
{
	save_item(NAME(m_data));
}

uint16_t bk_loopback_device::io_r()
{
	return m_data;
}

void bk_loopback_device::io_w(uint16_t data, bool word)
{
	m_data = data;
}

} // anonymous namespace


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(BK_LOOPBACK, device_bk_parallel_interface, bk_loopback_device, "bk_loopback", "BK Loopback")

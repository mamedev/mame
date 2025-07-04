// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub

#include "emu.h"
#include "cards.h"

namespace bus::spectrum::ay_slot {

namespace {

class ay8912_device : public spectrum_ay_device
{
public:
	ay8912_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: spectrum_ay_device(mconfig, AY_SLOT_AY8912, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

void ay8912_device::device_add_mconfig(machine_config &config)
{
	AY8912(config, m_ay0, clock());
	spectrum_ay_device::device_add_mconfig(config);
}


class turbosound_device : public spectrum_ay_device
{
public:
	turbosound_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: spectrum_ay_device(mconfig, AY_SLOT_TURBOSOUND, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

void turbosound_device::device_add_mconfig(machine_config &config)
{
	YM2149(config, m_ay0, clock());
	YM2149(config, m_ay1, clock());
	spectrum_ay_device::device_add_mconfig(config);
}


class ym2149_device : public spectrum_ay_device
{
public:
	ym2149_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: spectrum_ay_device(mconfig, AY_SLOT_YM2149, tag, owner, clock)
	{ }

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

void ym2149_device::device_add_mconfig(machine_config &config)
{
	YM2149(config, m_ay0, clock());
	spectrum_ay_device::device_add_mconfig(config);
}

} // anonymous namespace

} // namespace bus::spectrum::ay_slot

DEFINE_DEVICE_TYPE_PRIVATE(AY_SLOT_AY8912,     device_ay_slot_interface, bus::spectrum::ay_slot::ay8912_device,     "ay_ay8912",     "AY8912")
DEFINE_DEVICE_TYPE_PRIVATE(AY_SLOT_TURBOSOUND, device_ay_slot_interface, bus::spectrum::ay_slot::turbosound_device, "ay_turbosound", "TURBOSOUND")
DEFINE_DEVICE_TYPE_PRIVATE(AY_SLOT_YM2149,     device_ay_slot_interface, bus::spectrum::ay_slot::ym2149_device,     "ay_ym2149",     "YM2149")

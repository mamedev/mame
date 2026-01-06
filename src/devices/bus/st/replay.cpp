// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// Microdeal ST Replay

// A 8-bit mono DAC and a 8-bit mono ADC on a cartridge, with a vague
// lowpass filter.

#include "emu.h"
#include "replay.h"

#include "sound/adc.h"
#include "sound/dac.h"
#include "speaker.h"

namespace {

class st_replay_device : public device_t, public device_stcart_interface
{
public:
	st_replay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~st_replay_device();

	virtual void map(address_space_installer &space) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<zn449_device> m_adc;
	required_device<zn429e_device> m_dac;
	required_device<microphone_device> m_mic;
	required_device<speaker_device> m_speaker;

	void cartmap(address_map &map) ATTR_COLD;

	u16 dac_w(offs_t data);
};

st_replay_device::st_replay_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ST_REPLAY, tag, owner, clock),
	device_stcart_interface(mconfig, *this),
	m_adc(*this, "adc"),
	m_dac(*this, "dac"),
	m_mic(*this, "mic"),
	m_speaker(*this, "speaker")
{
}

st_replay_device::~st_replay_device()
{
}

u16 st_replay_device::dac_w(offs_t data)
{
	if (!machine().side_effects_disabled())
		m_dac->write(data);
	return 0xffff;
}

void st_replay_device::map(address_space_installer &space)
{
	space.install_device(0xfa0000, 0xfbffff, *this, &st_replay_device::cartmap);
}

void st_replay_device::cartmap(address_map &map)
{
	map(0x00000, 0x001ff).r(FUNC(st_replay_device::dac_w)).mirror(0xfe00);
	map(0x10001, 0x10001).r(m_adc, FUNC(zn449_device::read)).mirror(0xfffe);
}

void st_replay_device::device_add_mconfig(machine_config &config)
{
	MICROPHONE(config, m_mic, 1).front_center();
	m_mic->add_route(0, m_adc, 1.0, 0);

	ZN449(config, m_adc);

	// ZN429D the schematics say, not sure if any significant difference
	ZN429E(config, m_dac);
	m_dac->add_route(0, m_speaker, 1.0, 0);

	SPEAKER(config, m_speaker, 1).front_center();
}

void st_replay_device::device_start()
{
}

void st_replay_device::device_reset()
{
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(ST_REPLAY, device_stcart_interface, st_replay_device, "st_replay", "Microdeal ST Replay")

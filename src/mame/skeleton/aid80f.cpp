// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    AID-80F was Mostek's development system for the Z80 and 3870 families.
    It was later advertised as the development system for the MATRIX-80
    (MDX) STD bus microcomputer.

    Boards available for this modular system included:
    * SDB-80E/OEM-80E CPU Module
    * FLP-80E Floppy Controller
    * CRT-80E Video Terminal Controller
    * PPG-8/16 2708/2758/2716 PROM Programmer
    * AIM-80 Z80 Application Interface Module
      — later replaced by AIM-Z80AE (4 MHz) and AIM-Z80BE (6 MHz)
    * AIM-72 3870 Application Interface Module
      — later replaced by AIM-7XE

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80sio.h"


namespace {

class aid80f_state : public driver_device
{
public:
	aid80f_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, RAM_TAG)
		, m_monitor(*this, "monitor")
	{
	}

	void aid80f(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	u8 ram_r(offs_t offset);
	void ram_w(offs_t offset, u8 data);
	u8 monitor_r(offs_t offset);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<z80_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_region_ptr<u8> m_monitor;

	bool m_ram_enabled = false;
};

void aid80f_state::machine_start()
{
	save_item(NAME(m_ram_enabled));
}

void aid80f_state::machine_reset()
{
	m_ram_enabled = false;
}

u8 aid80f_state::ram_r(offs_t offset)
{
	if (m_ram_enabled)
		return m_ram->read(offset);
	else
		return m_monitor[offset & 0xfff];
}

void aid80f_state::ram_w(offs_t offset, u8 data)
{
	if (m_ram_enabled)
		m_ram->write(offset, data);
}

u8 aid80f_state::monitor_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		m_ram_enabled = true;
	return m_monitor[offset];
}

void aid80f_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(aid80f_state::ram_r), FUNC(aid80f_state::ram_w));
	map(0xe000, 0xefff).r(FUNC(aid80f_state::monitor_r));
}

void aid80f_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xd8, 0xdb).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0xdc, 0xdf).rw("pio", FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0xe4, 0xe7).rw("fdc", FUNC(fd1771_device::read), FUNC(fd1771_device::write));
}

static INPUT_PORTS_START(aid80f)
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "pio" },
	{ "sio" },
	{ nullptr }
};

void aid80f_state::aid80f(machine_config &config)
{
	Z80(config, m_maincpu, 4.9152_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &aid80f_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &aid80f_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	RAM(config, m_ram).set_default_size("64K");

	Z80CTC(config, "ctc", 4.9152_MHz_XTAL / 2);
	Z80PIO(config, "pio", 4.9152_MHz_XTAL / 2);
	Z80SIO(config, "sio", 4.9152_MHz_XTAL / 2);

	FD1771(config, "fdc", 4_MHz_XTAL / 4);
}

ROM_START(aid80f)
	ROM_REGION(0x1000, "monitor", 0)
	ROM_LOAD("ddt1.u48", 0x0000, 0x0400, CRC(c9e5dc59) SHA1(3143bca7472900e7255ec0e39ca4121d1cb74c5f))
	ROM_LOAD("ddt2.u49", 0x0400, 0x0400, CRC(dd37f628) SHA1(6da130a3678fd84070dd4ae3487c5212db5604ef))
	ROM_LOAD("ddt3.u50", 0x0800, 0x0400, CRC(646d5fd2) SHA1(b9d0a4b3658835c5d724d709ca3cd70d69474caa))
	ROM_LOAD("ddt4.u51", 0x0c00, 0x0400, CRC(c78e34c2) SHA1(5f3a4631d0b806a077b817f566ebbddd77ad7ba5))
	// U52 socket is empty
ROM_END

} // anonymous namespace


COMP(1978, aid80f, 0, 0, aid80f, aid80f, aid80f_state, empty_init, "Mostek", "AID-80F Development System", MACHINE_IS_SKELETON)

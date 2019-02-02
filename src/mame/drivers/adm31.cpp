// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for ADM-31 terminal.

    The ADM-31 and ADM-42 Data Display Terminals were Lear Siegler, Inc.'s
    first microprocessor-based video terminals, introduced in 1978 as the
    respective successors to their earlier ADM-1A and ADM-2 "smart"
    terminals. The original ADM-31 model was apparently rebranded in 1980
    as the ADM-31 Intermediate Terminal, and the ADM-32 was released a few
    months later.

    While the ADM-31 and ADM-32 only support 2 pages of display memory, the
    ADM-42 could be upgraded to 8. Enhancements over the ADM-31 offered by
    both the ADM-42 and ADM-32 include a status line, a larger monitor and
    a detachable keyboard. Several other expansion options were offered for
    the ADM-42, including synchronous serial and parallel printer ports.

****************************************************************************/

#include "emu.h"
//#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6800.h"
#include "machine/com8116.h"
#include "machine/input_merger.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "video/tms9927.h"
#include "screen.h"

class adm31_state : public driver_device
{
public:
	adm31_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_acia(*this, "acia%u", 1U)
		, m_vtac(*this, "vtac")
		, m_chargen(*this, "chargen")
	{
	}

	void adm31(machine_config &mconfig);

protected:
	virtual void machine_start() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device_array<acia6850_device, 2> m_acia;
	required_device<crt5027_device> m_vtac;
	required_region_ptr<u8> m_chargen;
};


void adm31_state::machine_start()
{
}


u32 adm31_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void adm31_state::mem_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();
	map(0x7000, 0x7000).nopw();
	map(0x7800, 0x7803).rw("pia", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0x7900, 0x7900).portr("7900");
	map(0x7a00, 0x7a01).rw(m_acia[0], FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x7c00, 0x7c01).rw(m_acia[1], FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x7d00, 0x7d00).portr("7D00");
	map(0x7e00, 0x7e00).portr("7E00");
	map(0x7f00, 0x7f0f).rw(m_vtac, FUNC(crt5027_device::read), FUNC(crt5027_device::write));
	map(0x8000, 0x8fff).ram();
	map(0xe000, 0xffff).rom().region("program", 0);
}


static INPUT_PORTS_START(adm31)
	PORT_START("7900")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("7D00")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNKNOWN)

	PORT_START("7E00")
	PORT_DIPNAME(0x02, 0x02, "Refresh Rate")
	PORT_DIPSETTING(0x00, "50 Hz")
	PORT_DIPSETTING(0x02, "60 Hz")
	PORT_BIT(0xfd, IP_ACTIVE_LOW, IPT_UNKNOWN)
INPUT_PORTS_END

void adm31_state::adm31(machine_config &config)
{
	M6800(config, m_maincpu, 19.584_MHz_XTAL / 20);
	m_maincpu->set_addrmap(AS_PROGRAM, &adm31_state::mem_map);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline(m_maincpu, M6800_IRQ_LINE);

	PIA6821(config, "pia");

	ACIA6850(config, m_acia[0]);
	m_acia[0]->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia[1]);
	m_acia[0]->irq_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));

	com8116_device &brg(COM8116(config, "brg", 5.0688_MHz_XTAL));
	brg.fr_handler().set(m_acia[0], FUNC(acia6850_device::write_rxc));
	brg.fr_handler().append(m_acia[0], FUNC(acia6850_device::write_txc));
	brg.ft_handler().set(m_acia[1], FUNC(acia6850_device::write_rxc));
	brg.ft_handler().append(m_acia[1], FUNC(acia6850_device::write_txc));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(19.584_MHz_XTAL, 1020, 0, 800, 320, 0, 300);
	screen.set_screen_update(FUNC(adm31_state::screen_update));

	CRT5027(config, m_vtac, 19.584_MHz_XTAL / 10);
	m_vtac->set_screen("screen");
	m_vtac->set_char_width(10);
}


ROM_START(adm31)
	ROM_REGION(0x2000, "program", 0)
	ROM_LOAD("adm-31-u62.bin", 0x0000, 0x0800, CRC(57e557a5) SHA1(cb3021ab570c279cbaa673b5de8fa1ca9eb48188))
	ROM_LOAD("adm-31-u63.bin", 0x0800, 0x0800, CRC(1268a59c) SHA1(f0cd8562e0d5faebf84d8decaa848ff28f2ac637))
	ROM_LOAD("adm-31-u64.bin", 0x1000, 0x0800, CRC(8939fa00) SHA1(00f6a8a49e51a9501cd9d1e2aae366fb070a5a1d))
	ROM_LOAD("adm-31-u65.bin", 0x1800, 0x0800, CRC(53e4e2f1) SHA1(bf30241815c790de3354e1acfe84e760c889cbb1))

	ROM_REGION(0x0800, "chargen", 0)
	ROM_LOAD("chargen.bin", 0x0000, 0x0800, NO_DUMP)
ROM_END


COMP(1978, adm31, 0, 0, adm31, adm31, adm31_state, empty_init, "Lear Siegler", "ADM-31 Data Display Terminal", MACHINE_IS_SKELETON)

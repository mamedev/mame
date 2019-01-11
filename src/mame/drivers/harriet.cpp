// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Harriet (c) 1990 Quantel

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/hd63450.h"
//#include "machine/imsc012.h"
#include "machine/mc68681.h"
#include "machine/mc68901.h"
#include "machine/nvram.h"
#include "machine/timekpr.h"
//#include "machine/wd33c93.h"

class harriet_state : public driver_device
{
public:
	harriet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void harriet(machine_config &config);
private:
	DECLARE_READ8_MEMBER(zpram_r);
	DECLARE_WRITE8_MEMBER(zpram_w);
	DECLARE_READ8_MEMBER(unk_status_r);

	void harriet_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	std::unique_ptr<u8[]> m_zpram_data;
};

READ8_MEMBER(harriet_state::zpram_r)
{
	return m_zpram_data[offset];
}

WRITE8_MEMBER(harriet_state::zpram_w)
{
	m_zpram_data[offset] = data;
}

READ8_MEMBER(harriet_state::unk_status_r)
{
	return 0x81;
}

void harriet_state::harriet_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("monitor", 0);
	map(0x040000, 0x040fff).rw(FUNC(harriet_state::zpram_r), FUNC(harriet_state::zpram_w)).umask16(0xff00);
	map(0x040000, 0x040fff).rw("timekpr", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0x00ff);
	map(0x7f0000, 0x7fffff).ram();
	map(0xf10000, 0xf1001f).rw("duart", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xf20000, 0xf2002f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xf30000, 0xf30fff).rw("dmac", FUNC(hd63450_device::read), FUNC(hd63450_device::write));
	map(0xf4003f, 0xf4003f).r(FUNC(harriet_state::unk_status_r));
	//map(0xf60000, 0xf6000f).rw("c012", FUNC(imsc012_device::read), FUNC(imsc012_device::write));
}

static INPUT_PORTS_START( harriet )
INPUT_PORTS_END

void harriet_state::machine_start()
{
	m_zpram_data = std::make_unique<u8[]>(0x800);
	subdevice<nvram_device>("zpram")->set_base(m_zpram_data.get(), 0x800);
	save_pointer(NAME(m_zpram_data), 0x800);
}

void harriet_state::machine_reset()
{
}


void harriet_state::harriet(machine_config &config)
{
	M68010(config, m_maincpu, 40_MHz_XTAL / 4); // MC68010FN10
	m_maincpu->set_addrmap(AS_PROGRAM, &harriet_state::harriet_map);

	MC68681(config, "duart", 3.6864_MHz_XTAL);

	mc68901_device &mfp(MC68901(config, "mfp", 40_MHz_XTAL / 16));
	mfp.set_timer_clock(2.4576_MHz_XTAL);
	mfp.set_rx_clock(9600);
	mfp.set_tx_clock(9600);
	mfp.out_so_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	//mfp.out_tco_cb().set("mfp", FUNC(mc68901_device::rc_w));
	//mfp.out_tdo_cb().set("mfp", FUNC(mc68901_device::tc_w));

	HD63450(config, "dmac", 40_MHz_XTAL / 4, "maincpu"); // MC68450R10 (or HD68450Y-10)

	M48T02(config, "timekpr", 0);
	NVRAM(config, "zpram", nvram_device::DEFAULT_ALL_0); // MK48Z02

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, "terminal"));
	rs232.rxd_handler().set("mfp", FUNC(mc68901_device::write_rx));
	rs232.rxd_handler().append("mfp", FUNC(mc68901_device::tbi_w));

	//WD33C93(config, "wdca", 40_MHz_XTAL / 4);
	//WD33C93(config, "wdcb", 40_MHz_XTAL / 4);
	//IMSC012(config, "c012", 40_MHz_XTAL / 8); // INMOS IMSC012-P20S link adaptor
}


ROM_START( harriet )
	ROM_REGION16_BE(0x8000, "monitor", 0)
	ROM_LOAD16_BYTE("harriet 36-74c.tfb v5.01 lobyte 533f.bin", 0x0001, 0x4000, CRC(f07fff76) SHA1(8288f7eaa8f4155e0e4746635f63ca2cc3da25d1))
	ROM_LOAD16_BYTE("harriet 36-74c.tdb v5.01 hibyte 2a0c.bin", 0x0000, 0x4000, CRC(a61f441d) SHA1(76af6eddd5c042f1b2eef590eb822379944b9b28))
ROM_END

COMP( 1990, harriet, 0, 0, harriet, harriet, harriet_state, empty_init, "Quantel", "Harriet", MACHINE_IS_SKELETON )

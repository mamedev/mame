// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Harriet (c) 1990 Quantel

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
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
protected:
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
	map(0x040000, 0x040fff).rw(this, FUNC(harriet_state::zpram_r), FUNC(harriet_state::zpram_w)).umask16(0xff00);
	map(0x040000, 0x040fff).rw("timekpr", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write)).umask16(0x00ff);
	map(0x7f0000, 0x7fffff).ram();
	map(0xf10000, 0xf1001f).rw("duart", FUNC(mc68681_device::read), FUNC(mc68681_device::write)).umask16(0x00ff);
	map(0xf20000, 0xf2002f).rw("mfp", FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xf4003f, 0xf4003f).r(this, FUNC(harriet_state::unk_status_r));
}

static INPUT_PORTS_START( harriet )
INPUT_PORTS_END

void harriet_state::machine_start()
{
	m_zpram_data = std::make_unique<u8[]>(0x800);
	subdevice<nvram_device>("zpram")->set_base(m_zpram_data.get(), 0x800);
	save_pointer(NAME(m_zpram_data.get()), 0x800);
}

void harriet_state::machine_reset()
{
}


MACHINE_CONFIG_START(harriet_state::harriet)
	MCFG_DEVICE_ADD("maincpu", M68010, 40_MHz_XTAL / 4) // MC68010FN10
	MCFG_DEVICE_PROGRAM_MAP(harriet_map)

	//MCFG_DEVICE_ADD("dmac", MC68450, DMAC_CLOCK)

	MCFG_DEVICE_ADD("duart", MC68681, 3.6864_MHz_XTAL)

	MCFG_DEVICE_ADD("mfp", MC68901, 0)
	MCFG_MC68901_TIMER_CLOCK(2.4576_MHz_XTAL)
	MCFG_MC68901_RX_CLOCK(9600)
	MCFG_MC68901_TX_CLOCK(9600)
	MCFG_MC68901_OUT_SO_CB(WRITELINE("rs232", rs232_port_device, write_txd))

	MCFG_M48T02_ADD("timekpr")
	MCFG_NVRAM_ADD_0FILL("zpram") // MK48Z02

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("mfp", mc68901_device, write_rx))

	//MCFG_DEVICE_ADD("wdc1", WD33C93, WD_CLOCK)
	//MCFG_DEVICE_ADD("wdc2", WD33C93, WD_CLOCK)
MACHINE_CONFIG_END


ROM_START( harriet )
	ROM_REGION16_BE(0x8000, "monitor", 0)
	ROM_LOAD16_BYTE("harriet 36-74c.tfb v5.01 lobyte 533f.bin", 0x0001, 0x4000, CRC(f07fff76) SHA1(8288f7eaa8f4155e0e4746635f63ca2cc3da25d1))
	ROM_LOAD16_BYTE("harriet 36-74c.tdb v5.01 hibyte 2a0c.bin", 0x0000, 0x4000, CRC(a61f441d) SHA1(76af6eddd5c042f1b2eef590eb822379944b9b28))
ROM_END

COMP( 1990, harriet, 0, 0, harriet, harriet, harriet_state, empty_init, "Quantel", "Harriet", MACHINE_IS_SKELETON )

// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Curt Coder
/*

Omnibyte MSBC-1 Multibus Single Board Computer

PCB Layout
----------

REV B

|-----------------------------------------------------------|
|       CN1     CN2     CN3     CN4         SW1     CN5     |
|                                                           |
|           3.6864MHz   3.6864MHz   ROM1                    |
|                                   ROM0                    |
|       SIO         SIO     SW3 SW2                         |
|                               PIT                         |
|               12.5MHz                 41256 41256 41256   |
|                                       41256 41256 41256   |
|   8MHz                        CPU           41245 41256   |
|                                                           |
|                                               8419        |
|                           SW4                             |
|-|-------------CN6----------------|----|-------CN7-------|-|
  |--------------------------------|    |-----------------|

Notes:
    Relevant IC's shown.

    CPU     - Motorola MC68000R12
    SIO     - Mostek MK68564N-5A Serial Input/Output
    PIT     - Motorola MC68230L10 Parallel Interface/Timer
    8419    - National Semiconductor DP8419N DRAM Controller
    41256   - Fujitsu MB81256-12 256Kx1 RAM
    ROM0    - AMD AM27128ADC 16Kx8 ROM "47-2818-36G1"
    ROM1    - AMD AM27128ADC 16Kx8 ROM "47-2818-36G2"
    SW1     - push button
    SW2     - rotary hex DIP
    SW3     - rotary hex DIP
    SW4     - DIP8
    CN1     -
    CN2     -
    CN3     -
    CN4     -
    CN5     -
    CN6     -
    CN7     -

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"
#include "machine/z80sio.h"


namespace {

#define MC68000R12_TAG  "u50"
#define MK68564_0_TAG   "u14"
#define MK68564_1_TAG   "u15"
#define MC68230L10_TAG  "u16"

class msbc1_state : public driver_device
{
public:
	msbc1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, MC68000R12_TAG)
	{ }

	void msbc1(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<cpu_device> m_maincpu;
};

void msbc1_state::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x5fffff).ram();
	map(0xf80000, 0xf87fff).rom().region(MC68000R12_TAG, 0);
	map(0xfffa00, 0xfffa3f).rw("sio1", FUNC(mk68564_device::read), FUNC(mk68564_device::write)).umask16(0x00ff);
	map(0xfffc00, 0xfffc3f).rw("sio2", FUNC(mk68564_device::read), FUNC(mk68564_device::write)).umask16(0x00ff);
	map(0xfffe00, 0xfffe3f).rw("pit", FUNC(pit68230_device::read), FUNC(pit68230_device::write)).umask16(0x00ff);
}

/* Input ports */
static INPUT_PORTS_START( msbc1 )
INPUT_PORTS_END

void msbc1_state::machine_reset()
{
	void *ram = m_maincpu->space(AS_PROGRAM).get_write_ptr(0);
	uint8_t *rom = memregion(MC68000R12_TAG)->base();

	memcpy(ram, rom, 8);
}

void msbc1_state::msbc1(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12.5_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &msbc1_state::mem_map);

	PIT68230(config, "pit", 8_MHz_XTAL);

	MK68564(config, "sio1", 8_MHz_XTAL / 2).set_xtal(3.6864_MHz_XTAL);
	MK68564(config, "sio2", 8_MHz_XTAL / 2).set_xtal(3.6864_MHz_XTAL);
}

/* ROM definition */
ROM_START( msbc1 )
	ROM_REGION16_BE( 0x8000, MC68000R12_TAG, 0 )
	ROM_LOAD16_BYTE( "47-2818-36g1.u18", 0x0000, 0x4000, CRC(14f25d47) SHA1(964bc49c1dd9e9680c0d3d89ff3794c5044bea62) )
	ROM_LOAD16_BYTE( "47-2818-36g2.u19", 0x0001, 0x4000, CRC(4814b9e1) SHA1(d96cf75084c6588cb33513830c6beeeffc2de853) )

	ROM_REGION( 0x100, "plds", 0 )
	ROM_LOAD( "p21.1.u22", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p22.0.u37", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p23.0.u38", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p24.0.u43", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p25.0.u44", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p26.0.u45", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p27.0.u46", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p28.1.u49", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p29.0.u58", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p30.0.u60", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p31.0.u61", 0x000, 0x100, NO_DUMP )
	ROM_LOAD( "p32.0.u76", 0x000, 0x100, NO_DUMP )
ROM_END

} // anonymous namespace


/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY     FULLNAME  FLAGS
COMP( 1985, msbc1, 0,      0,      msbc1,   msbc1, msbc1_state, empty_init, "Omnibyte", "MSBC-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )

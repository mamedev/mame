// license:MAME
// copyright-holders:Robbbert
/*************************************************************************************

  PINBALL
  Micropin : Pentacup
  First version used a 6800, but a later revision used a 8085A.

**************************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "cpu/i8085/i8085.h"
#include "machine/6821pia.h"
#include "micropin.lh"

class micropin_state : public genpin_class
{
public:
	micropin_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_v1cpu(*this, "v1cpu")
		, m_v2cpu(*this, "v2cpu")
		, m_pia51(*this, "pia51")
	{ }

	DECLARE_READ8_MEMBER(pia51_r);
	DECLARE_WRITE8_MEMBER(pia51_w);
	DECLARE_WRITE8_MEMBER(sol_w);
	DECLARE_READ8_MEMBER(sw_r);
	DECLARE_WRITE8_MEMBER(sw_w);
	DECLARE_WRITE8_MEMBER(lamp_w);
	DECLARE_WRITE8_MEMBER(p50a_w);
	DECLARE_WRITE8_MEMBER(p50b_w);
	DECLARE_DRIVER_INIT(micropin);
private:
	UINT8 m_row;
	virtual void machine_reset();
	optional_device<m6800_cpu_device> m_v1cpu;
	optional_device<i8085a_cpu_device> m_v2cpu;
	optional_device<pia6821_device> m_pia51;
};


static ADDRESS_MAP_START( micropin_map, AS_PROGRAM, 8, micropin_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff) // A10,11,15 not used
	AM_RANGE(0x0000, 0x01ff) AM_RAM AM_SHARE("nvram") // 4x 6561 RAM
	AM_RANGE(0x4000, 0x4005) AM_READWRITE(sw_r,sw_w)
	AM_RANGE(0x5000, 0x5003) AM_DEVREADWRITE("pia50", pia6821_device, read, write)
	AM_RANGE(0x5100, 0x5103) AM_READWRITE(pia51_r,pia51_w)
	AM_RANGE(0x5200, 0x5200) AM_WRITE(sol_w);
	AM_RANGE(0x5202, 0x5202) AM_WRITE(lamp_w);
	AM_RANGE(0x5203, 0x5203) AM_WRITENOP
	AM_RANGE(0x6400, 0x7fff) AM_ROM AM_REGION("v1cpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pentacup2_map, AS_PROGRAM, 8, micropin_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x23ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pentacup2_io, AS_IO, 8, micropin_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00, 0x0e) AM_WRITE
	//AM_RANGE(0x0f, 0x0f) AM_WRITE
	//AM_WRITE(0x00, 0x05) AM_READ
ADDRESS_MAP_END

static INPUT_PORTS_START( micropin )
INPUT_PORTS_END

READ8_MEMBER( micropin_state::pia51_r )
{
	return m_pia51->read(space, offset) ^ 0xff;
}

WRITE8_MEMBER( micropin_state::pia51_w )
{
	m_pia51->write(space, offset, data ^ 0xff);
}

WRITE8_MEMBER( micropin_state::lamp_w )
{
	m_row = data & 15;
	// lamps
}

WRITE8_MEMBER( micropin_state::sol_w )
{
}

READ8_MEMBER( micropin_state::sw_r )
{
	return 0xff;
}

WRITE8_MEMBER( micropin_state::sw_w )
{
}

WRITE8_MEMBER( micropin_state::p50a_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
	output_set_digit_value(m_row, patterns[data&15]);
	output_set_digit_value(m_row+20, patterns[data>>4]);
}

WRITE8_MEMBER( micropin_state::p50b_w )
{
	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0 }; // 7448
	output_set_digit_value(m_row+40, patterns[data&15]);
	output_set_digit_value(m_row+60, patterns[data>>4]);
}

void micropin_state::machine_reset()
{
	m_row = 0;
}

DRIVER_INIT_MEMBER( micropin_state, micropin )
{
}

static MACHINE_CONFIG_START( micropin, micropin_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("v1cpu", M6800, XTAL_2MHz / 2)
	MCFG_CPU_PROGRAM_MAP(micropin_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(micropin_state, irq0_line_hold,  500)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_micropin)

	/* Devices */
	MCFG_DEVICE_ADD("pia50", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(micropin_state, p50a_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(micropin_state, p50a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(micropin_state, p50b_r))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(micropin_state, p50b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(micropin_state, p50ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(micropin_state, p50cb2_w))

	MCFG_DEVICE_ADD("pia51", PIA6821, 0)
	//MCFG_PIA_READPA_HANDLER(READ8(micropin_state, p51a_r))
	//MCFG_PIA_WRITEPA_HANDLER(WRITE8(micropin_state, p51a_w))
	//MCFG_PIA_READPB_HANDLER(READ8(micropin_state, p51b_r))
	//MCFG_PIA_WRITEPB_HANDLER(WRITE8(micropin_state, p51b_w))
	//MCFG_PIA_CA2_HANDLER(WRITELINE(micropin_state, p51ca2_w))
	//MCFG_PIA_CB2_HANDLER(WRITELINE(micropin_state, p51cb2_w))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pentacup2, micropin_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("v2cpu", I8085A, 2000000)
	MCFG_CPU_PROGRAM_MAP(pentacup2_map)
	MCFG_CPU_IO_MAP(pentacup2_io)

	//MCFG_NVRAM_ADD_0FILL("nvram")

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Pentacup
/-------------------------------------------------------------------*/
ROM_START(pentacup)
	ROM_REGION(0x1c00, "v1cpu", 0)
	ROM_LOAD("ic2.bin", 0x0000, 0x0400, CRC(fa468a0f) SHA1(e9c8028bcd5b87d24f4588516536767a869c38ff))
	ROM_LOAD("ic3.bin", 0x0400, 0x0400, CRC(7bfdaec8) SHA1(f2037c0e2d4acf0477351ecafc9f0826e9d64d76))
	ROM_LOAD("ic4.bin", 0x0800, 0x0400, CRC(5e0fcb1f) SHA1(e529539c6eb1e174a799ad6abfce9e31870ff8af))
	ROM_LOAD("ic5.bin", 0x0c00, 0x0400, CRC(a26c6e0b) SHA1(21c4c306fbc2da52887e309b1c83a1ea69501c1f))
	ROM_LOAD("ic6.bin", 0x1000, 0x0400, CRC(4715ac34) SHA1(b6d8c20c487db8d7275e36f5793666cc591a6691))
	ROM_LOAD("ic7.bin", 0x1400, 0x0400, CRC(c58d13c0) SHA1(014958bc69ff326392a5a7782703af0980e6e170))
	ROM_LOAD("ic8.bin", 0x1800, 0x0400, CRC(9f67bc65) SHA1(504008d4c7c23a14fdf247c9e6fc00e95d907d7b))
ROM_END

ROM_START(pentacup2)
	ROM_REGION(0x2000, "v2cpu", 0)
	ROM_LOAD("micro_1.bin", 0x0000, 0x0800, CRC(4d6dc218) SHA1(745c553f3a42124f925ca8f2e52fd08d05999594))
	ROM_LOAD("micro_2.bin", 0x0800, 0x0800, CRC(33cd226d) SHA1(d1dff8445a0f35da09d560a16038c969845ff21f))
	ROM_LOAD("micro_3.bin", 0x1000, 0x0800, CRC(997bde74) SHA1(c3ea33f7afbdc7f2a22798a13ec323d7c6628dd4))
	ROM_LOAD("micro_4.bin", 0x1800, 0x0800, CRC(a804e7d6) SHA1(f414d6a5308266744645849940c00cd422e920d2))
	// 2 undumped proms DMA-01, DMA-02
ROM_END


GAME(1978,  pentacup,  0,         micropin,   micropin, micropin_state,  micropin,  ROT0, "Micropin", "Pentacup (rev. 1)",     GAME_IS_SKELETON_MECHANICAL)
GAME(1980,  pentacup2, pentacup,  pentacup2,  micropin, micropin_state,  micropin,  ROT0, "Micropin", "Pentacup (rev. 2)",     GAME_IS_SKELETON_MECHANICAL)

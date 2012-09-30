/**************************************************************************

    Pinball
    Wico's only game : Af-tor

    Schematic and PinMAME used as references.
    Code for the interrupts/timers was derived from PinMAME.


***************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6809/m6809.h"
#include "machine/nvram.h"
#include "sound/sn76496.h"
//#include "wico.lh"


class wico_state : public driver_device
{
public:
	wico_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_cpu2(*this, "maincpu"),
	m_shared_ram(*this, "sharedram"),
	m_samples(*this, "samples")
	{ }

	DECLARE_WRITE8_MEMBER(zcres_w);
	TIMER_DEVICE_CALLBACK_MEMBER(irq_housekeeping);
	TIMER_DEVICE_CALLBACK_MEMBER(firq_housekeeping);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_cpu2;
	required_shared_ptr<UINT8> m_shared_ram;
	required_device<samples_device> m_samples;

	// driver_device overrides
	virtual void machine_reset();
private:
	UINT8 m_zcen;
	UINT8 m_gten;
	UINT8 m_firqtimer;
public:
	DECLARE_DRIVER_INIT(wico);
};

// housekeeping cpu
static ADDRESS_MAP_START( wico_map, AS_PROGRAM, 8, wico_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("sharedram")
	AM_RANGE(0x1fe0, 0x1fe0) AM_NOP //AM_WRITE(muxld_w)
	//AM_RANGE(0x1fe1, 0x1fe1) AM_WRITE(store_w)
	//AM_RANGE(0x1fe2, 0x1fe2) AM_WRITE(muxen_w)
	//AM_RANGE(0x1fe3, 0x1fe3) AM_WRITE(csols_w)
	//AM_RANGE(0x1fe4, 0x1fe4) AM_WRITE(msols_w)
	//AM_RANGE(0x1fe5, 0x1fe5) AM_DEVWRITE("sn76494", sn76494_new_device, write)
	AM_RANGE(0x1fe6, 0x1fe6) AM_NOP //WRITE(wdogcl_w)
	AM_RANGE(0x1fe7, 0x1fe7) AM_WRITE(zcres_w)
	//AM_RANGE(0x1fe8, 0x1fe8) AM_WRITE(dled0_w)
	//AM_RANGE(0x1fe9, 0x1fe9) AM_WRITE(dled1_w)
	//AM_RANGE(0x1fea, 0x1fea) AM_WRITE(gentmrcl_w)
	//AM_RANGE(0x1feb, 0x1feb) AM_READ(lampst_r)
	//AM_RANGE(0x1fec, 0x1fec) AM_READ(sast_r)
	//AM_RANGE(0x1fed, 0x1fed) AM_READ(solst1_r)
	//AM_RANGE(0x1fee, 0x1fee) AM_READ(solst0_r)
	//AM_RANGE(0x1fef, 0x1fef) AM_READ(switch_r)
	AM_RANGE(0xf000, 0xffff) AM_ROM
ADDRESS_MAP_END

// command cpu
static ADDRESS_MAP_START( wico_sub_map, AS_PROGRAM, 8, wico_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("sharedram") // 2128  2k RAM
	//AM_RANGE(0x1fe0, 0x1fe0) AM_WRITE(muxld_w) // to display module
	//AM_RANGE(0x1fe1, 0x1fe1) AM_WRITE(store_w) // enable save to nvram
	//AM_RANGE(0x1fe2, 0x1fe2) AM_WRITE(muxen_w) // digit to display on diagnostic LED; d0=L will disable main displays
	//AM_RANGE(0x1fe3, 0x1fe3) AM_WRITE(csols_w) // solenoid column
	//AM_RANGE(0x1fe4, 0x1fe4) AM_WRITE(msols_w) // solenoid row
	//AM_RANGE(0x1fe5, 0x1fe5) AM_DEVWRITE("sn76494", sn76494_new_device, write)
	//AM_RANGE(0x1fe6, 0x1fe6) AM_WRITE(wdogcl_w) // watchdog clear
	AM_RANGE(0x1fe7, 0x1fe7) AM_WRITE(zcres_w) // enable IRQ on maincpu
	//AM_RANGE(0x1fe8, 0x1fe8) AM_WRITE(dled0_w) // turn off diagnostic LED
	//AM_RANGE(0x1fe9, 0x1fe9) AM_WRITE(dled1_w) // turn on diagnotic LED
	//AM_RANGE(0x1fea, 0x1fea) AM_WRITE(gentmrcl_w) // enable IRQ on cpu2
	//AM_RANGE(0x1feb, 0x1feb) AM_READ(lampst_r) // lamps?
	//AM_RANGE(0x1fec, 0x1fec) AM_READ(sast_r) // a pwron pulse to d0 L->H
	//AM_RANGE(0x1fed, 0x1fed) AM_READ(solst1_r) // switches
	//AM_RANGE(0x1fee, 0x1fee) AM_READ(solst0_r) // switches
	//AM_RANGE(0x1fef, 0x1fef) AM_READ(switch_r) // switches
	AM_RANGE(0x4000, 0x40ff) AM_RAM AM_SHARE("nvram") // X2212 4bit x 256 NVRAM, stores only when store_w is active
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xe000, 0xffff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( wico )
INPUT_PORTS_END

void wico_state::machine_reset()
{
	m_zcen = 0;
	m_gten = 0;
}

DRIVER_INIT_MEMBER(wico_state,wico)
{
}

WRITE8_MEMBER( wico_state::zcres_w )
{
	m_zcen = 1;
}


TIMER_DEVICE_CALLBACK_MEMBER( wico_state::irq_housekeeping )
{
	if (m_zcen)
		generic_pulse_irq_line(m_maincpu, M6809_IRQ_LINE,1);
}

TIMER_DEVICE_CALLBACK_MEMBER( wico_state::firq_housekeeping )
{
	if (!m_gten)
		generic_pulse_irq_line(m_maincpu, M6809_FIRQ_LINE,1);

	// Gen. timer irq of command CPU kicks in every 4 interrupts of this timer
	m_firqtimer++;
	if (m_firqtimer > 3)
	{
		generic_pulse_irq_line(m_cpu2, M6809_IRQ_LINE,1);
		m_firqtimer = 0;
	}
}


//-------------------------------------------------
//  sn76496_config psg_intf
//-------------------------------------------------

static const sn76496_config psg_intf =
{
	DEVCB_NULL
};

static MACHINE_CONFIG_START( wico, wico_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 10000000 / 8)
	MCFG_CPU_PROGRAM_MAP(wico_map)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", wico_state, irq_housekeeping, attotime::from_hz(120)) // zero crossing
	MCFG_TIMER_DRIVER_ADD_PERIODIC("firq", wico_state, firq_housekeeping, attotime::from_hz(750)) // time generator
	MCFG_CPU_ADD("cpu2", M6809, 10000000 / 8)
	MCFG_CPU_PROGRAM_MAP(wico_sub_map)
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	//MCFG_DEFAULT_LAYOUT(layout_wico)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
	MCFG_SOUND_ADD("sn76494", SN76494, 10000000 / 8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_CONFIG(psg_intf)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Af-Tor (1984)
/-------------------------------------------------------------------*/
ROM_START(aftor)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u25.bin", 0xf000, 0x1000, CRC(d66e95ff) SHA1(f7e8c51f1b37e7ef560406f1968c12a2043646c5))

	ROM_REGION(0x10000, "cpu2", 0)
	ROM_LOAD("u52.bin", 0x8000, 0x2000, CRC(8035b446) SHA1(3ec59015e259c315bf09f4e2046f9d98e2d7a732))
	ROM_LOAD("u48.bin", 0xe000, 0x2000, CRC(b4406563) SHA1(6d1a9086eb1f6f947eae3a92ccf7a9b7375d85d3))
ROM_END

/*-------------------------------------------------------------------
/ Big Top  (1977)
/-------------------------------------------------------------------*/

GAME(1984,  aftor,  0,  wico,  wico, wico_state,  wico,  ROT0,  "Wico", "Af-Tor", GAME_IS_SKELETON_MECHANICAL)

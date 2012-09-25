/******************************************************************************************
    Pinball
    Valley Spectra IV

    Rotating game, like Midway's "Rotation VIII".

    Schematic and PinMAME used as references.


*******************************************************************************************/


#include "machine/genpin.h"
#include "cpu/m6502/m6502.h"
#include "machine/6532riot.h"
#include "machine/nvram.h"
//#include "spectra.lh"


class spectra_state : public driver_device
{
public:
	spectra_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_samples(*this, "samples")
	{ }

	DECLARE_READ8_MEMBER(porta_r);
	DECLARE_READ8_MEMBER(portb_r);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;

	// driver_device overrides
	virtual void machine_reset();
public:
	DECLARE_DRIVER_INIT(spectra);
};


static ADDRESS_MAP_START( spectra_map, AS_PROGRAM, 8, spectra_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xfff)
	AM_RANGE(0x0000, 0x00ff) AM_RAM AM_SHARE("nvram") // battery backed, 2x 5101L
	AM_RANGE(0x0100, 0x017f) AM_RAM // RIOT RAM
	AM_RANGE(0x0180, 0x019f) AM_DEVREADWRITE_LEGACY("riot", riot6532_r, riot6532_w)
	AM_RANGE(0x0400, 0x0fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( spectra )
INPUT_PORTS_END

void spectra_state::machine_reset()
{
}

DRIVER_INIT_MEMBER(spectra_state,spectra)
{
}

READ8_MEMBER( spectra_state::porta_r )
{printf("ReadA ");
	return 0;
}

READ8_MEMBER( spectra_state::portb_r )
{printf("ReadB ");
	return 0;
}

WRITE8_MEMBER( spectra_state::porta_w )
{printf("A=%X ",data);
}

WRITE8_MEMBER( spectra_state::portb_w )
{printf("B=%X ",data);
}


static const riot6532_interface riot6532_intf =
{
	DEVCB_DRIVER_MEMBER(spectra_state, porta_r),	// port a in
	DEVCB_DRIVER_MEMBER(spectra_state, portb_r),	// port b in
	DEVCB_DRIVER_MEMBER(spectra_state, porta_w),	// port a out
	DEVCB_DRIVER_MEMBER(spectra_state, portb_w),	// port b in
	DEVCB_CPU_INPUT_LINE("maincpu", M6502_IRQ_LINE)	// interrupt
};

static MACHINE_CONFIG_START( spectra, spectra_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 3579545/4)  // actually a 6503
	MCFG_CPU_PROGRAM_MAP(spectra_map)
	MCFG_RIOT6532_ADD("riot", 3579545/4, riot6532_intf) // R6532
	MCFG_NVRAM_ADD_0FILL("nvram")

	/* Video */
	//MCFG_DEFAULT_LAYOUT(layout_spectra)

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )
MACHINE_CONFIG_END

/*--------------------------------
/ Spectra IV
/-------------------------------*/
ROM_START(spectra)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("spect_u5.dat", 0x0400, 0x0400, CRC(49e0759f) SHA1(c3badc90ff834cbc92d8c519780069310c2b1507))
	// pinmame has a different u4 rom: CRC(b58f1205) SHA1(9578fd89485f3f560789cb0f24c7116e4bc1d0da)
	ROM_LOAD("spect_u4.dat", 0x0800, 0x0400, BAD_DUMP CRC(e6519689) SHA1(06ef3d349ea27a072889b7c379f258d29b7217be))
	ROM_LOAD("spect_u3.dat", 0x0c00, 0x0400, CRC(9ca7510f) SHA1(a87849f16903836158063d593bb4a2e90c7473c8))
ROM_END


GAME(1979,  spectra,  0,  spectra,  spectra, spectra_state,  spectra,  ROT0,  "Valley",    "Spectra IV",     GAME_IS_SKELETON_MECHANICAL)

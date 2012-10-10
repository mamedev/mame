/***********************************************************************************

    Pinball
    Atari Generation/System 2

    Manuals and PinMAME used as references (couldn't find full schematics).


************************************************************************************/

#include "machine/genpin.h"
#include "cpu/m6800/m6800.h"
#include "machine/nvram.h"
#include "atari_s2.lh"


class atari_s2_state : public driver_device
{
public:
	atari_s2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_samples(*this, "samples")
	{ }

	DECLARE_WRITE8_HANDLER(intack_w);


	TIMER_DEVICE_CALLBACK_MEMBER(irq);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<samples_device> m_samples;

	// driver_device overrides
	virtual void machine_reset();
private:
	UINT8 m_t_c;
};


static ADDRESS_MAP_START( atari_s2_map, AS_PROGRAM, 8, atari_s2_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x00ff) AM_MIRROR(0x0700) AM_RAM
	AM_RANGE(0x0800, 0x08ff) AM_MIRROR(0x0700) AM_RAM AM_SHARE("nvram") // battery backed
	//AM_RANGE(0x1000, 0x1007) AM_MIRROR(0x07F8) AM_READ(sw_r)
	//AM_RANGE(0x1800, 0x1800) AM_MIRROR(0x071F) AM_WRITE(sound0_w)
	//AM_RANGE(0x1820, 0x1820) AM_MIRROR(0x071F) AM_WRITE(sound1_w)
	//AM_RANGE(0x1840, 0x1847) AM_MIRROR(0x0718) AM_WRITE(disp_w)
	//AM_RANGE(0x1860, 0x1867) AM_MIRROR(0x0718) AM_WRITE(lamp_w)
	//AM_RANGE(0x1880, 0x1880) AM_MIRROR(0x071F) AM_WRITE(sol0_w)
	//AM_RANGE(0x18a0, 0x18a7) AM_MIRROR(0x0718) AM_WRITE(sol1_w)
	AM_RANGE(0x18c0, 0x18c0) AM_MIRROR(0x071F) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x18e0, 0x18e0) AM_MIRROR(0x071F) AM_WRITE(intack_w)
	//AM_RANGE(0x2000, 0x2003) AM_MIRROR(0x07FC) AM_READWRITE(dip_r,dip_w)
	AM_RANGE(0x2800, 0x3fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( atari_s2 )
INPUT_PORTS_END

WRITE8_MEMBER( atari_s2_state::intack_w )
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( atari_s2_state::irq )
{
//	static const UINT8 patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // 4511
//	m_bit6++;
	if (m_t_c > 0x40)
		m_maincpu->set_input_line(M6800_IRQ_LINE, HOLD_LINE);
	else
		m_t_c++;

//	m_out_offs++;
//	m_out_offs &= 0x1f;
//	if ((m_out_offs & 3) == 3)
//	{
//		// Player number
//		char wordnum[8];
//		sprintf(wordnum,"text%d",m_out_offs>>2);
//		output_set_value(wordnum, !BIT(patterns[m_p_ram[m_out_offs]&15], 6)); // uses 'g' segment
//	}
//	else
//	{
//		// Digits
//		output_set_digit_value(m_out_offs << 1, patterns[m_p_ram[m_out_offs]>>4]);
//		output_set_digit_value((m_out_offs << 1)+1, patterns[m_p_ram[m_out_offs]&15]);
//	}
}

void atari_s2_state::machine_reset()
{
}

static MACHINE_CONFIG_START( atari_s2, atari_s2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_4MHz / 4)
	MCFG_CPU_PROGRAM_MAP(atari_s2_map)
	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq", atari_s2_state, irq, attotime::from_hz(XTAL_4MHz / 8192))

	/* Sound */
	MCFG_FRAGMENT_ADD( genpin_audio )

	/* Video */
	MCFG_DEFAULT_LAYOUT(layout_atari_s2)
MACHINE_CONFIG_END

/*-------------------------------------------------------------------
/ Superman (03/1979)
/-------------------------------------------------------------------*/
ROM_START(supermap)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("supmn_k.rom", 0x2800, 0x0800, CRC(a28091c2) SHA1(9f5e47db408da96a31cb2f3be0fa9fb1e79f8d85))
	ROM_LOAD("atari_m.rom", 0x3000, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_LOAD("atari_j.rom", 0x3800, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))

	ROM_REGION(0x1000, "sound1", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END

/*-------------------------------------------------------------------
/ Hercules (05/1979)
/-------------------------------------------------------------------*/
ROM_START(hercules)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("herc_k.rom",  0x2800, 0x0800, CRC(65e099b1) SHA1(83a06bc82e0f8f4c0655886c6a9962bb28d00c5e))
	ROM_LOAD("atari_m.rom", 0x3000, 0x0800, CRC(1bb6b72c) SHA1(dd24ed54de275aadf8dc0810a6af3ac97aea4026))
	ROM_LOAD("atari_j.rom", 0x3800, 0x0800, CRC(26521779) SHA1(2cf1c66441aee99b9d01859d495c12025b5ef094))

	ROM_REGION(0x1000, "sound1", 0)
	ROM_LOAD("82s130.bin", 0x0000, 0x0200, CRC(da1f77b4) SHA1(b21fdc1c6f196c320ec5404013d672c35f95890b))
ROM_END


GAME( 1979, supermap,  0,  atari_s2,  atari_s2, driver_device, 0,  ROT0, "Atari", "Superman (Pinball)", GAME_IS_SKELETON_MECHANICAL)
GAME( 1979, hercules,  0,  atari_s2,  atari_s2, driver_device, 0,  ROT0, "Atari", "Hercules", GAME_IS_SKELETON_MECHANICAL)

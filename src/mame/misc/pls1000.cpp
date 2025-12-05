// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Brainchild PLS-1000 "Personal Learning System"

TODO:
- keeps looping on INT_TIMER2 events in DragonBall core
(shouldn't take them? may require a first cart boot that populates the stack?):
':maincpu' (0062B4): tcmp_w<1>: TCMP2 = 028f
':maincpu' (0062B8): tstat_r: TSTAT2: 0000
':maincpu' (0062BE): tctl_w<1>: TCTL2 = 0039
TCTL_TEN, TCTL_CLKSOURCE_32KHZ4, TCTL_OM, TCTL_IRQEN

- Requires cart dumps to go further;

===================================================================================================

MC68328PV16V at U1
Atmel AT27C512R ROM at U2
HY62256ALJ-10 CMOS at U3
power switch labeled SW1
two knobs at console bottom sides, VR1/VR2

**************************************************************************************************/

#include "emu.h"

#include "cpu/m68000/m68000.h"
#include "machine/mc68328.h"
#include "video/mc68328lcd.h"

//#include "speaker.h"
#include "screen.h"

namespace {

class pls1000_state : public driver_device
{
public:
	pls1000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdctrl(*this, "lcdctrl")
		, m_screen(*this, "screen")
	{ }

	void pls1000(machine_config &config);

protected:
	void main_map(address_map &map) ATTR_COLD;

private:
	required_device<mc68328_base_device> m_maincpu;
	required_device<mc68328_lcd_device> m_lcdctrl;
	required_device<screen_device> m_screen;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

};

u32 pls1000_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_lcdctrl->video_update(bitmap, cliprect);
	return 0;
}

void pls1000_state::main_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("bios", 0);
	map(0x200000, 0x207fff).ram();
//	map(0x800000, 0x8.....) cart space (checks 0x5a05 header there)
}

static INPUT_PORTS_START( pls1000 )
INPUT_PORTS_END

void pls1000_state::pls1000(machine_config &config)
{
	// TODO: unverified clock
	MC68328(config, m_maincpu, 32768*506); // MC68328PV16VA
	m_maincpu->set_addrmap(AS_PROGRAM, &pls1000_state::main_map);

	// TODO: I/O ports

	m_maincpu->out_flm().set(m_lcdctrl, FUNC(mc68328_lcd_device::flm_w));
	m_maincpu->out_llp().set(m_lcdctrl, FUNC(mc68328_lcd_device::llp_w));
	m_maincpu->out_lsclk().set(m_lcdctrl, FUNC(mc68328_lcd_device::lsclk_w));
	m_maincpu->out_ld().set(m_lcdctrl, FUNC(mc68328_lcd_device::ld_w));
	m_maincpu->set_lcd_info_changed(m_lcdctrl, FUNC(mc68328_lcd_device::lcd_info_changed));

	// TODO: check vertical
	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(240, 160);
	m_screen->set_visarea(0, 240 - 1, 0, 160 - 1);
	m_screen->set_screen_update(FUNC(pls1000_state::screen_update));

	MC68328_LCD(config, m_lcdctrl, 0);

//	SPEAKER(config, "speaker").front_center();
//	DAC_1BIT(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



ROM_START( pls1000 )
	ROM_REGION16_BE( 0x208000, "bios", 0 )
	ROM_LOAD16_WORD( "at27c512r.u2", 0x000000, 0x10000, CRC(77f2beed) SHA1(7118108bd491434934910df072842d46a9ea6223) )
ROM_END


} // anonymous namespace

CONS( 1998, pls1000,  0, 0,  pls1000,  pls1000, pls1000_state, empty_init, "Brainchild", "PLS-1000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )


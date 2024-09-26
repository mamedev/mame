// license:BSD-3-Clause
// copyright-holders:David Viens
/************************************************************************
 *  Unisonic Champion 2711 (late 1977 based on part dates)
 *
 *  Driver from plgDavid (David Viens)
 *
 *  Thanks to Sylvain De Chantal (Sly D.C.) for the 2 test units,
 *  carts and FAQ: http://www.ccjvq.com/slydc/index/faq/2711
 *
 *  Thanks to Paul Robson for the GIC font rom.
 *  (http://worstconsole.blogspot.ca/2012/12/the-worstconsoleever.html)
 *  Note a spare dead GIC has been given to Lord Nightmare and should be sent for decap!
 *
 *  The Unisonic Champion is the only known GI "Gimini Mid-Range 8950 Programmable Game Set"
 *  to ever reach the market, and only in limited quantities (aprox 500 units ever built)
 *
 *  Architecture:
 *  Master IC : AY-3-8800-1 Graphics Interface (A.K.A. GIC, 40 pin)
 *  Slave  IC : CP1610 CPU (40 pin, same as in the Intellivision)
 *  EXEC ROM  : 9501-01009 (40 pin) at 0x0800 (factory mapped)
 *
 *  The GIC generates the CPU Clock, the video signals and the audio.
 *  The CPU does NOT access the GIC directly.
 *  One way CPU->GIC 'communication' takes place through 256 bytes of shared RAM
 *  (using two 4x256 TMS4043NL-2 (2112-1) Static Rams at U3 and U4)
 *
 *  In this design the GIC only allows the CPU to use the BUS (and shared RAM)
 *  a fraction of the frame time. (4.33ms for each 16.69ms, or 26% of the time)
 *  (the real ratio of clocks is 7752/29868 )
 *
 *  Boot: When the GIC let go of !RESET_OUT the EXEC Rom pushes 0x800 onto
 *  the bus for the CPU to fetch and place in R7 to start execution.
 *  This first CPU slice only last 3ms, then the GIC sets the CPU's BUSRQ low,
 *  stalling it for 12.36ms, then sets it high for 4.33ms etc...
 *  59.95 times a second - NTSC
 *
 *  TODO: Should we add an explicit Reset button like the controller has?
 *
 ************************************************************************/

#include "emu.h"

#include "cpu/cp1610/cp1610.h"
#include "gic.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class unichamp_state : public driver_device
{
public:
	unichamp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gic(*this, "gic"),
		m_cart(*this, "cartslot"),
		m_ctrls(*this, "CTRLS")
	{ }

	void unichamp(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cp1610_cpu_device> m_maincpu;
	required_device<gic_device> m_gic;
	required_device<generic_slot_device> m_cart;

	required_ioport m_ctrls;

	uint8_t m_ram[256];

	void unichamp_palette(palette_device &palette) const;

	uint8_t bext_r(offs_t offset);

	uint8_t gicram_r(offs_t offset);
	void gicram_w(offs_t offset, uint8_t data);

	uint16_t trapl_r(offs_t offset);
	void trapl_w(offs_t offset, uint16_t data);

	uint16_t read_ff();

	uint16_t iab_r();

	uint32_t screen_update_unichamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void unichamp_mem(address_map &map) ATTR_COLD;
};

void unichamp_state::unichamp_palette(palette_device &palette) const
{
	/*
	palette.set_pen_color(GIC_BLACK, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(GIC_RED,   rgb_t(0xae, 0x49, 0x41));//(from box shot)
	palette.set_pen_color(GIC_GREEN, rgb_t(0x62, 0x95, 0x88));//(from box shot)
	palette.set_pen_color(GIC_WHITE, rgb_t(0xff, 0xff, 0xff));
	*/

	//using from intv.c instead as suggested by RB
	palette.set_pen_color(GIC_BLACK, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(GIC_RED,   rgb_t(0xff, 0x3d, 0x10));
	//palette.set_pen_color(GIC_GREEN, rgb_t(0x38, 0x6b, 0x3f)); //intv's DARK GREEN
	palette.set_pen_color(GIC_GREEN, rgb_t(0x00, 0xa7, 0x56)); //intv's GREEN
	palette.set_pen_color(GIC_WHITE, rgb_t(0xff, 0xfc, 0xff));
}


void unichamp_state::unichamp_mem(address_map &map)
{
	map.global_mask(0x1FFF); //B13/B14/B15 are grounded!
	map(0x0000, 0x00FF).rw(FUNC(unichamp_state::gicram_r), FUNC(unichamp_state::gicram_w)).umask16(0x00ff);
	map(0x0100, 0x07FF).rw(FUNC(unichamp_state::trapl_r), FUNC(unichamp_state::trapl_w));
	map(0x0800, 0x0FFF).rom().region("maincpu", 0);   // Carts and EXE ROM, 10-bits wide
}


static INPUT_PORTS_START( unichamp )
	PORT_START( "CTRLS" )
	PORT_BIT( 0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')// P1 YES (EBCA0)
	PORT_BIT( 0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N')// P1 NO  (EBCA1)
	PORT_BIT( 0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')// P2 YES (EBCA2)
	PORT_BIT( 0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S')// P2 NO  (EBCA3)
	PORT_BIT( 0x10,  IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20,  IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40,  IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x80,  IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


uint8_t unichamp_state::bext_r(offs_t offset)
{
	//The BEXT instruction pushes a user-defined nibble out on the four EBCA pins (EBCA0 to EBCA3)
	//and reads the ECBI input pin for HIGH or LOW signal to know whether or not to branch

	//The unisonic control system couldnt be simpler in design.
	//Each of the two player controllers has three buttons:
	//one tying !RESET(GIC pin 21) to ground when closed - resetting the WHOLE system.
	//a YES button (connecting EBCA0 to EBCI for Player1 and EBC2 to EBCI for Player2)
	//a NO  button (connecting EBCA1 to EBCI for Player1 and EBC3 to EBCI for Player2)

	//The CPU outputs a MASK of whatever it needs and checks the result.
	//EG: Any player can choose if one or two players are going to play the game for instance

	uint8_t port = ioport("CTRLS")->read() & 0x0F; ////only lower nibble

	//We need to return logical high or low on the EBCI pin
	return (port & offset)>0?1:0;
}


uint16_t unichamp_state::read_ff()
{
	return 0xffff;
}

void unichamp_state::machine_start()
{
	if (m_cart->exists()){
		//flip endians in more "this surely exists in MAME" way?
		//NOTE The unichamp roms have the same endianness as intv on disk and in memory
		uint8_t*ptr   = m_cart->get_rom_base();
		size_t size = m_cart->get_rom_size();
		for(size_t i=0;i<size;i+=2){
			uint8_t TEMP = ptr[i];
			ptr[i] = ptr[i+1];
			ptr[i+1] = TEMP;
		}
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x17ff,
					read16s_delegate(*m_cart, FUNC(generic_slot_device::read16_rom)));
	} else
		m_maincpu->space(AS_PROGRAM).install_read_handler(0x1000, 0x17ff,
					read16smo_delegate(*this, FUNC(unichamp_state::read_ff)));

	memset(m_ram, 0, sizeof(m_ram));
}

/* Set Reset and INTR/INTRM Vector */
uint16_t unichamp_state::iab_r()
{
	/*
	the intv driver did not explain this but from the CP1600 manual:
	When MSYNC* goes inactive (high), the bus control signals issue lAB,
	and the CPU inputs from the bus into the PC the starting address of the main program.
	Note that the initialization address can be defined by the user at any desired bus address or
	can be the default address resulting from the logical state of the non-driven bus
	*/

	//The Unisonic EXEC ROM chip (9501-01009) is self mapped at 0x0800
	//The cart ROMS are self mapped to 0x1000
	//upon boot the EXEC ROM puts 0x0800 on the bus for the CPU to use as first INT vector

	/* Set initial PC */
	return 0x0800;
}

uint32_t unichamp_state::screen_update_unichamp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return m_gic->screen_update(screen, bitmap, cliprect);
}

uint8_t unichamp_state::gicram_r(offs_t offset)
{
	return m_ram[offset];
}

void unichamp_state::gicram_w(offs_t offset, uint8_t data)
{
	m_ram[offset] = data;
}

uint16_t unichamp_state::trapl_r(offs_t offset)
{
	logerror("trapl_r(%x)\n",offset);
	return (int)0;
}

void unichamp_state::trapl_w(offs_t offset, uint16_t data)
{
	logerror("trapl_w(%x) = %x\n",offset,data);
}

void unichamp_state::unichamp(machine_config &config)
{
	/* basic machine hardware */

	//The CPU is really clocked this way:
	//CP1610(config, m_maincpu, XTAL(3'579'545)/4);
	//But since it is only running 7752/29868 th's of the time...
	//TODO find a more accurate method? (the emulation will be the same though)
	CP1610(config, m_maincpu, (7752.0/29868.0)*XTAL(3'579'545)/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &unichamp_state::unichamp_mem);
	m_maincpu->bext().set(FUNC(unichamp_state::bext_r));
	m_maincpu->iab().set(FUNC(unichamp_state::iab_r));

	config.set_maximum_quantum(attotime::from_hz(60));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(3'579'545),
		gic_device::LINE_CLOCKS, gic_device::START_ACTIVE_SCAN, gic_device::END_ACTIVE_SCAN,
		gic_device::LINES,       gic_device::START_Y,           gic_device::START_Y + gic_device::SCREEN_HEIGHT);
	screen.set_screen_update(FUNC(unichamp_state::screen_update_unichamp));
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(unichamp_state::unichamp_palette), 4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	GIC(config, m_gic, XTAL(3'579'545));
	m_gic->set_screen("screen");
	m_gic->ram_callback().set(FUNC(unichamp_state::gicram_r));
	m_gic->add_route(ALL_OUTPUTS, "mono", 0.40);

	/* cartridge */
	GENERIC_CARTSLOT(config, m_cart, generic_linear_slot, "unichamp_cart", "bin,rom");
	SOFTWARE_LIST(config, "cart_list").set_original("unichamp");
}

ROM_START(unichamp)
	ROM_REGION(0x1000,"maincpu", ROMREGION_ERASEFF)

	ROM_LOAD16_WORD( "9501-01009.u2", 0, 0x1000, CRC(49a0bd8f) SHA1(f4d126d3462ad351da4b75d76c75942d5a6f27ef))

	//these below are for local tests. you can use them in softlist or -cart
	//ROM_LOAD16_WORD( "pac-02.bin",   0x1000<<1, 0x1000, CRC(fe3213be) SHA1(5b9c407fe86865f3454d4be824a7f2bf53478f73))
	//ROM_LOAD16_WORD( "pac-03.bin",   0x1000<<1, 0x1000, CRC(f81f04bd) SHA1(82e2a0fda1787d5835c457ee5745b0db0cebe079))
	//ROM_LOAD16_WORD( "pac-04.bin",   0x1000<<1, 0x1000, CRC(cac09841) SHA1(bc9db83f26ed0810938156db6b104b4576754225))
	//ROM_LOAD16_WORD( "pac-05.bin",   0x1000<<1, 0x1000, CRC(d54a6090) SHA1(e85593096f43dcf14b08fd2c9fda277008a8df8b))
ROM_END

} // anonymous namespace


CONS( 1977, unichamp, 0, 0, unichamp, unichamp, unichamp_state, empty_init, "Unisonic", "Champion 2711", 0/*MACHINE_IMPERFECT_GRAPHICS*/ )

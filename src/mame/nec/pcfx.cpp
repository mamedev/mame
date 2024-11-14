// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************************************

  pcfx.cpp

  Driver file to handle emulation of the NEC PC-FX.

**************************************************************************************************/


#include "emu.h"
#include "cpu/v810/v810.h"
#include "sound/huc6230.h"
#include "video/huc6261.h"
#include "video/huc6270.h"
#include "video/huc6271.h"
#include "video/huc6272.h"
#include "screen.h"
#include "speaker.h"


namespace {

// TODO: should really inherit from a common pcfx_motherboard_device
// - pcfxga needs to be exposed as a ISA16 and C-Bus boards, it's not a real stand-alone driver.
class pcfx_state : public driver_device
{
public:
	pcfx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_huc6261(*this, "huc6261"),
		m_pads(*this, "P%u", 1U) { }

	void pcfx(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint16_t irq_read(offs_t offset);
	void irq_write(offs_t offset, uint16_t data);
	uint16_t pad_r(offs_t offset);
	void pad_w(offs_t offset, uint16_t data);
	[[maybe_unused]] uint8_t extio_r(offs_t offset);
	[[maybe_unused]] void extio_w(offs_t offset, uint8_t data);

	[[maybe_unused]] void irq8_w(int state);
	[[maybe_unused]] void irq9_w(int state);
	[[maybe_unused]] void irq10_w(int state);
	[[maybe_unused]] void irq11_w(int state);
	void irq12_w(int state);
	void irq13_w(int state);
	void irq14_w(int state);
	[[maybe_unused]] void irq15_w(int state);
	template <int Pad> TIMER_CALLBACK_MEMBER(pad_func);

	void pcfx_io(address_map &map) ATTR_COLD;
	void pcfx_mem(address_map &map) ATTR_COLD;

	// Interrupt controller (component unknown)
	uint16_t m_irq_mask = 0;
	uint16_t m_irq_pending = 0;
	uint8_t m_irq_priority[8]{};

	struct pcfx_pad_t
	{
		uint8_t ctrl[2]{};
		uint8_t status[2]{};
		uint32_t latch[2]{};
	};

	pcfx_pad_t m_pad;
	emu_timer *m_pad_timers[2];

	inline void check_irqs();
	inline void set_irq_line(int line, int state);

	required_device<cpu_device> m_maincpu;
	required_device<huc6261_device> m_huc6261;
	required_ioport_array<2> m_pads;
};


uint8_t pcfx_state::extio_r(offs_t offset)
{
	address_space &io_space = m_maincpu->space(AS_IO);

	return io_space.read_byte(offset);
}

void pcfx_state::extio_w(offs_t offset, uint8_t data)
{
	address_space &io_space = m_maincpu->space(AS_IO);

	io_space.write_byte(offset, data);
}

uint16_t pcfx_state::pad_r(offs_t offset)
{
	uint16_t res;
	uint8_t port_type = ((offset<<1) & 0x80) >> 7;

	if(((offset<<1) & 0x40) == 0)
	{
		// status
		/*
		---- x---
		---- ---x incoming data state (0=available)
		*/
		res = m_pad.status[port_type];
		//printf("STATUS %d\n",port_type);
	}
	else
	{
		// received data
		res = m_pad.latch[port_type] >> (((offset<<1) & 2) ? 16 : 0);

		if(((offset<<1) & 0x02) == 0)
		{
			m_pad.status[port_type] &= ~8; // clear latch on LSB read according to docs
			set_irq_line(11, 0);
		}
	}

	return res;
}

template <int Pad>
TIMER_CALLBACK_MEMBER(pcfx_state::pad_func)
{
	m_pad.latch[Pad] = m_pads[Pad]->read();
	m_pad.status[Pad] |= 8;
	m_pad.ctrl[Pad] &= ~1; // ack TX line
	// TODO: pad IRQ
	set_irq_line(11, 1);
}

void pcfx_state::pad_w(offs_t offset, uint16_t data)
{
	uint8_t port_type = ((offset<<1) & 0x80) >> 7;

	if(((offset<<1) & 0x40) == 0)
	{
		// control
		/*
		---- -x-- receiver enable
		---- --x- enable multi-tap
		---- ---x enable send (0->1 transition)
		*/
		if(data & 1 && (!(m_pad.ctrl[port_type] & 1)))
		{
			m_pad_timers[port_type]->adjust(attotime::from_usec(100)); // TODO: time
		}

		m_pad.ctrl[port_type] = data & 7;
		//printf("%04x CONTROL %d\n",data,port_type);
	}
	else
	{
		// transmitted data
		//printf("%04x TX %d\n",data,port_type);
	}
}

void pcfx_state::pcfx_mem(address_map &map)
{
	map(0x00000000, 0x001FFFFF).ram();   /* RAM */
//  map(0x80000000, 0x80FFFFFF).rw(FUNC(pcfx_state::extio_r), FUNC(pcfx_state::extio_w));    /* EXTIO */
//  map(0x80700000, 0x807FFFFF).rom().region("scsi_rom", 0); // EXTIO ROM area
	map(0xE0000000, 0xE7FFFFFF).noprw();   /* BackUp RAM */
	map(0xE8000000, 0xE9FFFFFF).noprw();   /* Extended BackUp RAM */
//  map(0xF8000000, 0xF8000007).noprw();   /* PIO, needed by pcfxga for reading backup "RAM" from DOS/V host */
	map(0xFFF00000, 0xFFFFFFFF).rom().region("ipl", 0);  /* ROM */
}

void pcfx_state::pcfx_io(address_map &map)
{
	map(0x00000000, 0x000000FF).rw(FUNC(pcfx_state::pad_r), FUNC(pcfx_state::pad_w)); /* PAD */
	map(0x00000100, 0x000001FF).w("huc6230", FUNC(huc6230_device::write)).umask32(0x00ff00ff);   /* HuC6230 */
	map(0x00000200, 0x000002FF).m("huc6271", FUNC(huc6271_device::regs)).umask32(0x0000ffff);   /* HuC6271 */
	map(0x00000300, 0x000003FF).rw(m_huc6261, FUNC(huc6261_device::read), FUNC(huc6261_device::write)).umask32(0x0000ffff);  /* HuC6261 */
	map(0x00000400, 0x000004FF).rw("huc6270_a", FUNC(huc6270_device::read), FUNC(huc6270_device::write)).umask32(0x0000ffff); /* HuC6270-A */
	map(0x00000500, 0x000005FF).rw("huc6270_b", FUNC(huc6270_device::read), FUNC(huc6270_device::write)).umask32(0x0000ffff); /* HuC6270-B */
	map(0x00000600, 0x00000607).mirror(0xf8).m("huc6272", FUNC(huc6272_device::amap)); // King
//  map(0x00000C80, 0x00000C83).noprw(); // backup RAM control
	map(0x00000E00, 0x00000EFF).rw(FUNC(pcfx_state::irq_read), FUNC(pcfx_state::irq_write)).umask32(0x0000ffff);    /* Interrupt controller */
//  map(0x00000F00, 0x00000FFF).noprw(); // Timer
//  map(0x00600000, 0x006FFFFF).r(FUNC(pcfx_state::scsi_ctrl_r)); // SCSI control for EXTIO
//  map(0x00700000, 0x007FFFFF).rom().region("scsi_rom", 0); // EXTIO ROM area
	map(0x80500000, 0x805000FF).noprw(); // pcfxga Kubota/Hudson HuC6273 "Aurora" 3d controller
}


static INPUT_PORTS_START( pcfx )
	/*
	xxxx ---- ---- ---- ID (0xf = 6 button pad, 0xe = multitap, 0xd = mouse, 0 = none)
	*/
	PORT_START("P1")
	PORT_BIT( 0xf0000000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ID pad
	// pcfxga expects latches to go active high on main menu
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Button I")
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Button II")
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Button III")
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 Button IV")
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 Button V")
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Button VI")
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_SELECT ) PORT_PLAYER(1) PORT_NAME("P1 Select Button")
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_START1 ) PORT_PLAYER(1) PORT_NAME("P1 RUN Button")
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_NAME("P1 Up")
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 Right")
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_NAME("P1 Down")
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_NAME("P1 Left")
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Switch 1")
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("P1 Switch 2")
	PORT_BIT( 0x0fffa000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xf0000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // ID unconnect
	PORT_BIT( 0x0fffffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


uint16_t pcfx_state::irq_read(offs_t offset)
{
	uint16_t data = 0;

	switch( offset )
	{
		// Interrupts pending
		// Same bit order as mask
		case 0x00/4:
			data = m_irq_pending;
			break;

		// Interrupt mask
		case 0x40/4:
			data = m_irq_mask;
			break;

		// Interrupt priority 0
		case 0x80/4:
			data = m_irq_priority[4] | ( m_irq_priority[5] << 3 ) | ( m_irq_priority[6] << 6 ) | ( m_irq_priority[7] << 9 );
			break;

		// Interrupt priority 1
		case 0xC0/4:
			data = m_irq_priority[0] | ( m_irq_priority[1] << 3 ) | ( m_irq_priority[2] << 6 ) | ( m_irq_priority[3] << 9 );
			break;
	}

	return data;
}


void pcfx_state::irq_write(offs_t offset, uint16_t data)
{
	switch( offset )
	{
		// Interrupts pending
		case 0x00/4:
			logerror("irq_write: Attempt to write to irq pending register\n");
			break;

		// Interrupt mask
		// --------x------- Mask interrupt level 8  (Unknown)
		// ---------x------ Mask interrupt level 9  (Timer)
		// ----------x----- Mask interrupt level 10 (Unknown)
		// -----------x---- Mask interrupt level 11 (Pad)
		// ------------x--- Mask interrupt level 12 (HuC6270-A)
		// -------------x-- Mask interrupt level 13 (HuC6272)
		// --------------x- Mask interrupt level 14 (HuC6270-B)
		// ---------------x Mask interrupt level 15 (HuC6273)
		// 0 - allow, 1 - ignore interrupt
		case 0x40/4:
			m_irq_mask = data;
			check_irqs();
			break;

		// Interrupt priority 0
		// ----xxx--------- Priority level interrupt 12
		// -------xxx------ Priority level interrupt 13
		// ----------xxx--- Priority level interrupt 14
		// -------------xxx Priority level interrupt 15
		case 0x80/4:
			m_irq_priority[7] = ( data >> 0 ) & 0x07;
			m_irq_priority[6] = ( data >> 3 ) & 0x07;
			m_irq_priority[5] = ( data >> 6 ) & 0x07;
			m_irq_priority[4] = ( data >> 9 ) & 0x07;
			check_irqs();
			break;

		// Interrupt priority 1
		// ----xxx--------- Priority level interrupt 8
		// -------xxx------ Priority level interrupt 9
		// ----------xxx--- Priority level interrupt 10
		// -------------xxx Priority level interrupt 11
		case 0xC0/4:
			m_irq_priority[3] = ( data >> 0 ) & 0x07;
			m_irq_priority[2] = ( data >> 3 ) & 0x07;
			m_irq_priority[1] = ( data >> 6 ) & 0x07;
			m_irq_priority[0] = ( data >> 9 ) & 0x07;
			check_irqs();
			break;
	}
}


inline void pcfx_state::check_irqs()
{
	uint16_t active_irqs = m_irq_pending & ~m_irq_mask;
	int highest_prio = -1;

	for (auto & elem : m_irq_priority)
	{
		if ( active_irqs & 0x80 )
		{
			if ( elem >= highest_prio )
			{
				highest_prio = elem;
			}
		}
		active_irqs <<= 1;
	}

	if ( highest_prio >= 0 )
	{
		m_maincpu->set_input_line(8 + highest_prio, ASSERT_LINE );
	}
	else
	{
		m_maincpu->set_input_line(0, CLEAR_LINE );
	}
}


inline void pcfx_state::set_irq_line(int line, int state)
{
	if ( state )
	{
//printf("Setting irq line %d\n", line);
		m_irq_pending |= ( 1 << ( 15 - line ) );
	}
	else
	{
//printf("Clearing irq line %d\n", line);
		m_irq_pending &= ~( 1 << ( 15 - line ) );
	}
	check_irqs();
}

void pcfx_state::irq8_w(int state)
{
	set_irq_line(8, state);
}

void pcfx_state::irq9_w(int state)
{
	set_irq_line(9, state);
}

void pcfx_state::irq10_w(int state)
{
	set_irq_line(10, state);
}

void pcfx_state::irq11_w(int state)
{
	set_irq_line(11, state);
}

void pcfx_state::irq12_w(int state)
{
	set_irq_line(12, state);
}

void pcfx_state::irq13_w(int state)
{
	set_irq_line(13, state);
}

void pcfx_state::irq14_w(int state)
{
	set_irq_line(14, state);
}

void pcfx_state::irq15_w(int state)
{
	set_irq_line(15, state);
}


void pcfx_state::machine_start()
{
	for (int i = 0; i < 2; i++)
	{
		m_pad.ctrl[i] = 0;
		m_pad.status[i] = 0;
		m_pad.latch[i] = 0;
	};

	m_pad_timers[0] = timer_alloc(FUNC(pcfx_state::pad_func<0>), this);
	m_pad_timers[1] = timer_alloc(FUNC(pcfx_state::pad_func<1>), this);
}

void pcfx_state::machine_reset()
{
	m_irq_mask = 0xFF;
	m_irq_pending = 0;
}


uint32_t pcfx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_huc6261->video_update( bitmap, cliprect );
	return 0;
}


void pcfx_state::pcfx(machine_config &config)
{
	V810(config, m_maincpu, XTAL(21'477'272));
	m_maincpu->set_addrmap(AS_PROGRAM, &pcfx_state::pcfx_mem);
	m_maincpu->set_addrmap(AS_IO, &pcfx_state::pcfx_io);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(FUNC(pcfx_state::screen_update));
	screen.set_raw(XTAL(21'477'272), huc6261_device::WPF, 64, 64 + 1024 + 64, huc6261_device::LPF, 18, 18 + 242);

	huc6270_device &huc6270_a(HUC6270(config, "huc6270_a", 0));
	huc6270_a.set_vram_size(0x20000);
	huc6270_a.irq().set(FUNC(pcfx_state::irq12_w));

	huc6270_device &huc6270_b(HUC6270(config, "huc6270_b", 0));
	huc6270_b.set_vram_size(0x20000);
	huc6270_b.irq().set(FUNC(pcfx_state::irq14_w));

	HUC6261(config, m_huc6261, XTAL(21'477'272));
	m_huc6261->set_vdc1_tag("huc6270_a");
	m_huc6261->set_vdc2_tag("huc6270_b");
	m_huc6261->set_king_tag("huc6272");

	huc6272_device &huc6272(HUC6272(config, "huc6272", XTAL(21'477'272)));
	huc6272.irq_changed_callback().set(FUNC(pcfx_state::irq13_w));
	huc6272.set_rainbow_tag("huc6271");

	HUC6271(config, "huc6271", XTAL(21'477'272));

	SOFTWARE_LIST(config, "cd_list").set_original("pcfx");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	huc6230_device &huc6230(HuC6230(config, "huc6230", XTAL(21'477'272)));
	huc6230.adpcm_update_cb<0>().set("huc6272", FUNC(huc6272_device::adpcm_update_0));
	huc6230.adpcm_update_cb<1>().set("huc6272", FUNC(huc6272_device::adpcm_update_1));
	huc6230.vca_callback().set("huc6272", FUNC(huc6272_device::cdda_update));
	huc6230.add_route(0, "lspeaker", 1.0);
	huc6230.add_route(1, "rspeaker", 1.0);
}


ROM_START( pcfx )
	ROM_REGION32_LE( 0x100000, "ipl", 0 )
	ROM_SYSTEM_BIOS( 0, "v100", "BIOS v1.00 - 2 Sep 1994" )
	ROMX_LOAD( "pcfxbios.bin", 0x000000, 0x100000, CRC(76ffb97a) SHA1(1a77fd83e337f906aecab27a1604db064cf10074), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "v101", "BIOS v1.01 - 5 Dec 1994" )
	ROMX_LOAD( "pcfxv101.bin", 0x000000, 0x100000, CRC(236102c9) SHA1(8b662f7548078be52a871565e19511ccca28c5c8), ROM_BIOS(1) )

	ROM_REGION32_LE( 0x100000, "scsi_rom", ROMREGION_ERASEFF )
	// TODO: "PC-FX EXTIO Boot", really belongs to FX-SCSI PC-FX expansion board
	// r/w to I/O $600000 SCSI if ROM enabled in both memory and I/O areas,
	// allows PC-FX to act as a CD drive for a PC-98 host ...
	ROM_LOAD( "fx-scsi.rom", 0x00000, 0x80000, CRC(f3e60e5e) SHA1(65482a23ac5c10a6095aee1db5824cca54ead6e5) )
	ROM_RELOAD( 0x80000, 0x80000 )
ROM_END


ROM_START( pcfxga )
	ROM_REGION32_LE( 0x100000, "ipl", 0 )
	ROM_LOAD( "pcfxga.rom", 0x000000, 0x100000, CRC(41c3776b) SHA1(a9372202a5db302064c994fcda9b24d29bb1b41c) )

	ROM_REGION32_LE( 0x100000, "scsi_rom", ROMREGION_ERASEFF )
ROM_END

} // Anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME                  FLAGS
CONS( 1994, pcfx,   0,      0,      pcfx,    pcfx,  pcfx_state, empty_init, "NEC",   "PC-FX",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 199?, pcfxga, pcfx,   0,      pcfx,    pcfx,  pcfx_state, empty_init, "NEC",   "PC-FX/GA (PC ISA Card)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

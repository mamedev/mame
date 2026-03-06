// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**************************************************************************************************

  pcfx.cpp

  Driver file to handle emulation of the NEC PC-FX.

**************************************************************************************************/


#include "emu.h"

#include "cpu/v810/v810.h"
#include "machine/nvram.h"
#include "machine/pcfx_intc.h"
#include "sound/huc6230.h"
#include "video/huc6261.h"
#include "video/huc6270.h"
#include "video/huc6271.h"
#include "video/huc6272.h"

#include "screen.h"
#include "speaker.h"
#include "softlist_dev.h"


namespace {

// TODO: should really compose everything as devices
// - pcfxga needs to be exposed as a ISA16 and C-Bus boards, it's not a real stand-alone driver.
// - FX-SCSI requires either a specific NSCSI target with enough boilerplate comms (i.e. no video),
//   or multiple instances of MAME being capable to communicate via bitbanger.
class pcfx_state : public driver_device
{
public:
	pcfx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_huc6261(*this, "huc6261")
		, m_intc(*this, "intc")
		, m_pads(*this, "P%u", 1U)
	{ }

	void pcfx(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void int_w(offs_t line, u8 state);

	template <int Pad> u16 pad_r(offs_t offset);
	template <int Pad> void pad_w(offs_t offset, u16 data);
	[[maybe_unused]] u8 extio_r(offs_t offset);
	[[maybe_unused]] void extio_w(offs_t offset, u8 data);

	template <int Pad> TIMER_CALLBACK_MEMBER(pad_func);

	void pcfx_io(address_map &map) ATTR_COLD;
	void pcfx_mem(address_map &map) ATTR_COLD;

	struct pcfx_pad_t
	{
		u8 ctrl[2]{};
		u8 status[2]{};
		u32 latch[2]{};
	};

	pcfx_pad_t m_pad;
	emu_timer *m_pad_timers[2]{};

	u8 m_bram_control = 0;
	std::unique_ptr<u8[]> m_nvram_ptr;

	required_device<cpu_device> m_maincpu;
	required_device<huc6261_device> m_huc6261;
	required_device<pcfx_intc_device> m_intc;
	required_ioport_array<2> m_pads;
};


u8 pcfx_state::extio_r(offs_t offset)
{
	address_space &io_space = m_maincpu->space(AS_IO);

	return io_space.read_byte(offset);
}

void pcfx_state::extio_w(offs_t offset, u8 data)
{
	address_space &io_space = m_maincpu->space(AS_IO);

	io_space.write_byte(offset, data);
}

template <int Pad>
u16 pcfx_state::pad_r(offs_t offset)
{
	u16 res;

	if (BIT(~offset, 5))
	{
		// status
		/*
		---- x---
		---- ---x incoming data state (0=available)
		*/
		res = m_pad.status[Pad];
		//logerror("%s: STATUS %d\n", machine().describe_context(), Pad);
	}
	else
	{
		// received data
		res = m_pad.latch[Pad] >> (BIT(offset, 0) << 4);

		if (BIT(~offset, 0))
		{
			if (!machine().side_effects_disabled())
			{
				m_pad.status[Pad] &= ~8; // clear latch on LSB read according to docs
				m_intc->irq_w<11>(CLEAR_LINE);
			}
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
	m_intc->irq_w<11>(ASSERT_LINE);
}

template <int Pad>
void pcfx_state::pad_w(offs_t offset, u16 data)
{
	if (BIT(~offset, 5))
	{
		// control
		/*
		---- -x-- receiver enable
		---- --x- enable multi-tap
		---- ---x enable send (0->1 transition)
		*/
		if ((data & 1) && !(m_pad.ctrl[Pad] & 1))
		{
			m_pad_timers[Pad]->adjust(attotime::from_usec(100)); // TODO: time
		}

		m_pad.ctrl[Pad] = data & 7;
		//logerror("%s: %04x CONTROL %d\n", machine().describe_context(), data, Pad);
	}
	else
	{
		// transmitted data
		//logerror("%s: %04x TX %d\n", machine().describe_context(), data, Pad);
	}
}

void pcfx_state::pcfx_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x001fffff).ram();   /* RAM */
//  map(0x80000000, 0x80ffffff).rw(FUNC(pcfx_state::extio_r), FUNC(pcfx_state::extio_w));    /* EXTIO */
//  map(0x80700000, 0x807fffff).rom().region("scsi_rom", 0); // EXTIO ROM area
	// internal backup control
	map(0xe0000000, 0xe7ffffff).lrw8(
		NAME([this] (offs_t offset) {
			return m_nvram_ptr[offset & 0x7fff];
		}),
		NAME([this] (offs_t offset, u8 data) {
			if (!BIT(m_bram_control, 0))
				return;
			m_nvram_ptr[offset & 0x7fff] = data;
		})
	).umask32(0x00ff00ff);
	map(0xe8000000, 0xe9ffffff).noprw();   /* Extended BackUp RAM */
//  map(0xf8000000, 0xf8000007).noprw();   /* PIO, needed by pcfxga for reading backup "RAM" from DOS/V host */
	map(0xfff00000, 0xffffffff).rom().region("ipl", 0);  /* ROM */
}

void pcfx_state::pcfx_io(address_map &map)
{
	map(0x00000000, 0x0000007f).rw(FUNC(pcfx_state::pad_r<0>), FUNC(pcfx_state::pad_w<0>)); /* PAD */
	map(0x00000080, 0x000000ff).rw(FUNC(pcfx_state::pad_r<1>), FUNC(pcfx_state::pad_w<1>)); /* PAD */
	map(0x00000100, 0x000001ff).w("huc6230", FUNC(huc6230_device::write)).umask32(0x00ff00ff);   /* HuC6230 */
	map(0x00000200, 0x000002ff).m("huc6271", FUNC(huc6271_device::amap));   /* HuC6271 */
	map(0x00000300, 0x000003ff).rw(m_huc6261, FUNC(huc6261_device::read), FUNC(huc6261_device::write)).umask32(0x0000ffff);  /* HuC6261 */
	map(0x00000400, 0x000004ff).rw("huc6270_a", FUNC(huc6270_device::read16), FUNC(huc6270_device::write16)).umask32(0x0000ffff); /* HuC6270-A */
	map(0x00000500, 0x000005ff).rw("huc6270_b", FUNC(huc6270_device::read16), FUNC(huc6270_device::write16)).umask32(0x0000ffff); /* HuC6270-B */
	map(0x00000600, 0x00000607).mirror(0xf8).m("huc6272", FUNC(huc6272_device::amap)); // King
	map(0x00000c00, 0x00000c00).r("huc6270_a", FUNC(huc6270_device::get_ar));
	map(0x00000c40, 0x00000c40).r("huc6270_b", FUNC(huc6270_device::get_ar));
	map(0x00000c80, 0x00000c80).lrw8(
		NAME([this] (offs_t offset) { return m_bram_control; }),
		NAME([this] (offs_t offset, u8 data) {
			m_bram_control = data & 3;
			// TODO: bit 1 to FX-BMP
		})
	);
	map(0x00000e00, 0x00000eff).rw(m_intc, FUNC(pcfx_intc_device::read), FUNC(pcfx_intc_device::write)).umask32(0x0000ffff);
//  map(0x00000f00, 0x00000fff).noprw(); // Timer
//  map(0x00500000, 0x005000ff) Aurora mirror
//  map(0x00600000, 0x006fffff).r(FUNC(pcfx_state::scsi_ctrl_r)); // SCSI control for EXTIO
//  map(0x00700000, 0x007fffff).rom().region("scsi_rom", 0); // EXTIO ROM area
	map(0x00700000, 0x007fffff).lr8(NAME([] () { return 0; })); // suppress logging
	map(0x80500000, 0x805000ff).noprw(); // pcfxga Kubota/Hudson HuC6273 "Aurora" 3d controller
}


static INPUT_PORTS_START( pcfx )
	/*
	xxxx ---- ---- ---- ID (0xf = 6 button pad, 0xe = multitap, 0xd = mouse, 0 = none)
	*/
	PORT_START("P1")
	PORT_BIT( 0xf0000000, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ID pad
	// pcfxga expects latches to go active high on main menu
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1 )        PORT_PLAYER(1) PORT_NAME("P1 Button I")
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2 )        PORT_PLAYER(1) PORT_NAME("P1 Button II")
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_BUTTON3 )        PORT_PLAYER(1) PORT_NAME("P1 Button III")
	PORT_BIT( 0x00000008, IP_ACTIVE_HIGH, IPT_BUTTON4 )        PORT_PLAYER(1) PORT_NAME("P1 Button IV")
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_BUTTON5 )        PORT_PLAYER(1) PORT_NAME("P1 Button V")
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_BUTTON6 )        PORT_PLAYER(1) PORT_NAME("P1 Button VI")
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_SELECT )         PORT_PLAYER(1) PORT_NAME("P1 Select Button")
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_START1 )         PORT_PLAYER(1) PORT_NAME("P1 RUN Button")
	PORT_BIT( 0x00000100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_PLAYER(1) PORT_NAME("P1 Up")
	PORT_BIT( 0x00000200, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_NAME("P1 Right")
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_PLAYER(1) PORT_NAME("P1 Down")
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_PLAYER(1) PORT_NAME("P1 Left")
	PORT_BIT( 0x00001000, IP_ACTIVE_HIGH, IPT_BUTTON7 )        PORT_PLAYER(1) PORT_NAME("P1 Switch 1")
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_BUTTON8 )        PORT_PLAYER(1) PORT_NAME("P1 Switch 2")
	PORT_BIT( 0x0fffa000, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0xf0000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // ID unconnect
	PORT_BIT( 0x0fffffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

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

	m_nvram_ptr = make_unique_clear<u8[]>(0x8000);
	subdevice<nvram_device>("nvram")->set_base(&m_nvram_ptr[0], 0x8000);

	save_pointer(NAME(m_nvram_ptr), 0x8000);
	save_item(NAME(m_bram_control));
}

void pcfx_state::machine_reset()
{
	m_bram_control = 0;
}

void pcfx_state::int_w(offs_t line, u8 state)
{
	m_maincpu->set_input_line(line, state ? ASSERT_LINE : CLEAR_LINE);
}


void pcfx_state::pcfx(machine_config &config)
{
	V810(config, m_maincpu, XTAL(21'477'272));
	m_maincpu->set_addrmap(AS_PROGRAM, &pcfx_state::pcfx_mem);
	m_maincpu->set_addrmap(AS_IO, &pcfx_state::pcfx_io);

	PCFX_INTC(config, m_intc, 0);
	m_intc->int_cb().set(FUNC(pcfx_state::int_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update(m_huc6261, FUNC(huc6261_device::screen_update));
	screen.set_raw(XTAL(21'477'272), huc6261_device::WPF, 64, 64 + 1024 + 64, huc6261_device::LPF, 18, 18 + 242);

	huc6270_device &huc6270_a(HUC6270(config, "huc6270_a", XTAL(21'477'272)));
	huc6270_a.set_vram_size(0x20000);
	huc6270_a.irq().set(m_intc, FUNC(pcfx_intc_device::irq_w<12>));

	huc6270_device &huc6270_b(HUC6270(config, "huc6270_b", XTAL(21'477'272)));
	huc6270_b.set_vram_size(0x20000);
	huc6270_b.irq().set(m_intc, FUNC(pcfx_intc_device::irq_w<14>));

	HUC6261(config, m_huc6261, XTAL(21'477'272));
	m_huc6261->set_vdc1_tag("huc6270_a");
	m_huc6261->set_vdc2_tag("huc6270_b");
	m_huc6261->set_rainbow_tag("huc6271");
	m_huc6261->set_king_tag("huc6272");

	huc6272_device &huc6272(HUC6272(config, "huc6272", XTAL(21'477'272)));
	huc6272.irq_changed_callback().set(m_intc, FUNC(pcfx_intc_device::irq_w<13>));
	huc6272.set_rainbow_tag("huc6271");

	HUC6271(config, "huc6271", XTAL(21'477'272));

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	huc6230_device &huc6230(HuC6230(config, "huc6230", XTAL(21'477'272)));
	huc6230.adpcm_update_cb<0>().set("huc6272", FUNC(huc6272_device::adpcm_update_0));
	huc6230.adpcm_update_cb<1>().set("huc6272", FUNC(huc6272_device::adpcm_update_1));
	huc6230.vca_callback().set("huc6272", FUNC(huc6272_device::cdda_update));
	huc6230.add_route(0, "speaker", 1.0, 0);
	huc6230.add_route(1, "speaker", 1.0, 1);

	SOFTWARE_LIST(config, "cd_list").set_original("pcfx");
	SOFTWARE_LIST(config, "photocd_list").set_compatible("photo_cd");
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
	// allows PC-FX to act as a CD drive for a PC-98 host.
	ROM_LOAD( "fx-scsi.rom", 0x00000, 0x80000, CRC(f3e60e5e) SHA1(65482a23ac5c10a6095aee1db5824cca54ead6e5) )
	ROM_RELOAD( 0x80000, 0x80000 )
ROM_END


ROM_START( pcfxga )
	ROM_REGION32_LE( 0x100000, "ipl", 0 )
	ROM_LOAD( "pcfxga.rom", 0x000000, 0x100000, CRC(41c3776b) SHA1(a9372202a5db302064c994fcda9b24d29bb1b41c) )

	ROM_REGION32_LE( 0x100000, "scsi_rom", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY  FULLNAME                  FLAGS
CONS( 1994, pcfx,   0,      0,      pcfx,    pcfx,  pcfx_state, empty_init, "NEC",   "PC-FX",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 199?, pcfxga, pcfx,   0,      pcfx,    pcfx,  pcfx_state, empty_init, "NEC",   "PC-FX/GA (PC ISA Card)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )

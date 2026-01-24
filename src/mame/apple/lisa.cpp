// license:BSD-3-Clause
// copyright-holders:Raphael Nabet, Olivier Galibert
/*********************************************************************

    drivers/lisa.cpp

    Experimental LISA driver

    Raphael Nabet, 2000
    In-depth rewrite, O. Galibert, 2023

*********************************************************************/

#include "emu.h"

#include "lisafdc.h"
#include "lisammu.h"
#include "lisavideo.h"

#include "bus/applepp/applepp.h"
#include "cpu/cop400/cop400.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"
#include "machine/74259.h"
#include "machine/input_merger.h"
#include "machine/quadmouse.h"
#include "machine/z80scc.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "softlist_dev.h"
#include "speaker.h"

#include "formats/ap_dsk35.h"


namespace {

class lisa_state : public driver_device
{
public:
	lisa_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mmu(*this, "mmu"),
		m_iocop(*this, "iocop"),
		m_kbcop(*this, "kbcop"),
		m_ioir(*this, "ioir"),
		m_via0(*this, "via6522_0"),
		m_via1(*this, "via6522_1"),
		m_video(*this, "video"),
		m_fdc(*this, "fdc"),
		m_scc(*this, "scc"),
		m_speaker(*this, "speaker"),
		m_pp(*this, "pp"),
		m_mainram(*this, "mainram"),
		m_mouse(*this, "mouse"),
		m_mousebtn(*this, "mousebtn"),
		m_keyboard(*this, "k%02x", 0U)
	{ }

	void lisa(machine_config &config) ATTR_COLD;
	void lisa2(machine_config &config) ATTR_COLD;
	void lisa210(machine_config &config) ATTR_COLD;
	void macxl(machine_config &config) ATTR_COLD;

private:
	required_device<m68000_device> m_maincpu;
	required_device<lisa_mmu_device> m_mmu;
	required_device<cop421_cpu_device> m_iocop;
	required_device<cop421_cpu_device> m_kbcop;
	required_device<input_merger_device> m_ioir;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_device<lisa_video_device> m_video;
	required_device<lisa_base_fdc_device> m_fdc;
	required_device<z80scc_device> m_scc;
	required_device<speaker_sound_device> m_speaker;
	required_device<applepp_connector> m_pp;

	required_shared_ptr<uint16_t> m_mainram;

	required_device<quadmouse_device> m_mouse;
	required_ioport m_mousebtn;
	required_ioport_array<32> m_keyboard;

	emu_timer *m_power_button_timer;
	emu_timer *m_iocop_g3;

	bool m_power_button_forced_pressed, m_power_on, m_crdy, m_iocop_g3_forced_low;
	bool m_kbd, m_iocop_kbd, m_kbcop_kbd, m_kbcop_so;
	u8 m_iocop_select, m_kbcop_d;

	void lisa_ram_map(address_map &map) ATTR_COLD;
	void lisa_io_map(address_map &map) ATTR_COLD;
	void lisa_special_io_map(address_map &map) ATTR_COLD;

	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(power_button_release);
	TIMER_CALLBACK_MEMBER(iocop_g3_freed);

	u8 iocop_g_r();
	u8 kbcop_g_r();

	u8 kbcop_l_r();

	void iocop_d_w(u8 data);
	void kbcop_d_w(u8 data);

	void iocop_kbd_w(int state);
	void kbcop_kbd_w(int state);
	void kbcop_so_w(int state);

	void kbd_update();
};

void lisa_state::lisa_special_io_map(address_map &map)
{
	map(0x000000, 0x003fff).rom().region("maincpu", 0).mirror(0xfe0000);
	map(0x008000, 0x008001).rw(m_mmu, FUNC(lisa_mmu_device::slr_r), FUNC(lisa_mmu_device::slr_w)).mirror(0x003ff6).select(0xfe0000);
	map(0x008008, 0x008009).rw(m_mmu, FUNC(lisa_mmu_device::sor_r), FUNC(lisa_mmu_device::sor_w)).mirror(0x003ff6).select(0xfe0000);
}

void lisa_state::lisa_io_map(address_map &map)
{
	// Extension cards range
	map(0x000000, 0x00bfff).lr16(NAME([]() -> u16 { return 0xffff; })).nopw();

	map(0x00c000, 0x00c7ff).rw(m_fdc, FUNC(lisa_base_fdc_device::ram_r), FUNC(lisa_base_fdc_device::ram_w)).umask16(0x00ff);

	map(0x00d241, 0x00d241).rw(m_scc, FUNC(z80scc_device::cb_r), FUNC(z80scc_device::cb_w));
	map(0x00d243, 0x00d243).rw(m_scc, FUNC(z80scc_device::ca_r), FUNC(z80scc_device::ca_w));
	map(0x00d245, 0x00d245).rw(m_scc, FUNC(z80scc_device::db_r), FUNC(z80scc_device::db_w));
	map(0x00d247, 0x00d247).rw(m_scc, FUNC(z80scc_device::da_r), FUNC(z80scc_device::da_w));

	map(0x00d901, 0x00d901)
		//.before_time(m_maincpu, FUNC(m68000_device::vpa_sync))
		//.after_delay(m_maincpu, FUNC(m68000_device::vpa_after))
		.lr8(NAME([this](offs_t offset) -> u8 { return m_via1->read(offset >> 3); })).lw8(NAME([this](offs_t offset, u8 data) { return m_via1->write(offset >> 3, data); })).select(0x78);

	map(0x00dd80, 0x00dd9f)
		//.before_time(m_maincpu, FUNC(m68000_device::vpa_sync))
		//.after_delay(m_maincpu, FUNC(m68000_device::vpa_after))
		.m(m_via0, FUNC(via6522_device::map)).umask16(0x00ff);

	map(0x00e000, 0x00e001).lr16(NAME([this]() -> u16 { m_mmu->diag1_0();   return 0; })).lw16(NAME([this](u16) { m_mmu->diag1_0();   }));
	map(0x00e002, 0x00e003).lr16(NAME([this]() -> u16 { m_mmu->diag1_1();   return 0; })).lw16(NAME([this](u16) { m_mmu->diag1_1();   }));
	map(0x00e004, 0x00e005).lr16(NAME([this]() -> u16 { m_mmu->diag2_0();   return 0; })).lw16(NAME([this](u16) { m_mmu->diag2_0();   }));
	map(0x00e006, 0x00e007).lr16(NAME([this]() -> u16 { m_mmu->diag2_1();   return 0; })).lw16(NAME([this](u16) { m_mmu->diag2_1();   }));
	map(0x00e008, 0x00e009).lr16(NAME([this]() -> u16 { m_mmu->seg1_0();    return 0; })).lw16(NAME([this](u16) { m_mmu->seg1_0();    }));
	map(0x00e00a, 0x00e00b).lr16(NAME([this]() -> u16 { m_mmu->seg1_1();    return 0; })).lw16(NAME([this](u16) { m_mmu->seg1_1();    }));
	map(0x00e00c, 0x00e00d).lr16(NAME([this]() -> u16 { m_mmu->seg2_0();    return 0; })).lw16(NAME([this](u16) { m_mmu->seg2_0();    }));
	map(0x00e00e, 0x00e00f).lr16(NAME([this]() -> u16 { m_mmu->seg2_1();    return 0; })).lw16(NAME([this](u16) { m_mmu->seg2_1();    }));
	map(0x00e010, 0x00e011).lr16(NAME([this]() -> u16 { m_mmu->setup_1();   return 0; })).lw16(NAME([this](u16) { m_mmu->setup_1();   }));
	map(0x00e012, 0x00e013).lr16(NAME([this]() -> u16 { m_mmu->setup_0();   return 0; })).lw16(NAME([this](u16) { m_mmu->setup_0();   }));
	map(0x00e014, 0x00e015).lr16(NAME([this]() -> u16 { m_mmu->serr_1();    return 0; })).lw16(NAME([this](u16) { m_mmu->serr_1();    }));
	map(0x00e016, 0x00e017).lr16(NAME([this]() -> u16 { m_mmu->serr_0();    return 0; })).lw16(NAME([this](u16) { m_mmu->serr_0();    }));
	map(0x00e018, 0x00e019).lr16(NAME([this]() -> u16 { m_video->vtmsk_1(); return 0; })).lw16(NAME([this](u16) { m_video->vtmsk_1(); }));
	map(0x00e01a, 0x00e01b).lr16(NAME([this]() -> u16 { m_video->vtmsk_0(); return 0; })).lw16(NAME([this](u16) { m_video->vtmsk_0(); }));
	map(0x00e01c, 0x00e01d).lr16(NAME([this]() -> u16 { m_mmu->herr_1();    return 0; })).lw16(NAME([this](u16) { m_mmu->herr_1();    }));
	map(0x00e01e, 0x00e01f).lr16(NAME([this]() -> u16 { m_mmu->herr_0();    return 0; })).lw16(NAME([this](u16) { m_mmu->herr_0();    }));

	map(0x00e800, 0x00e800).rw(m_video, FUNC(lisa_video_device::base_r), FUNC(lisa_video_device::base_w));
	map(0x00f000, 0x00f001).r(m_mmu, FUNC(lisa_mmu_device::parity_error_address_r));
	map(0x00f800, 0x00f801).r(m_mmu, FUNC(lisa_mmu_device::status_r));
}

void lisa_state::lisa_ram_map(address_map &map)
{
	map(0x000000, 0x1fffff).ram().share(m_mainram);
}

/***************************************************************************
    MACHINE DRIVER
***************************************************************************/

/* Lisa1 machine */
void lisa_state::lisa(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20.37504_MHz_XTAL / 4); // CPUCK is nominally 5 MHz
	m_maincpu->enable_mmu();

	INPUT_MERGER_ANY_HIGH(config, m_ioir);
	m_ioir->output_handler().set_inputline(m_maincpu, 1);

	LISAMMU(config, m_mmu);
	m_mmu->set_addrmap(lisa_mmu_device::AS_RAM,        &lisa_state::lisa_ram_map);
	m_mmu->set_addrmap(                 AS_IO,         &lisa_state::lisa_io_map);
	m_mmu->set_addrmap(lisa_mmu_device::AS_SPECIAL_IO, &lisa_state::lisa_special_io_map);
	m_mmu->set_cpu(m_maincpu);
	m_mmu->set_video(m_video);
	m_mmu->write_parity_err().set_inputline(m_maincpu, INPUT_LINE_NMI);

	COP421(config, m_iocop, 3.93216_MHz_XTAL); // U9F (I/O board)
	m_iocop->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, true);
	m_iocop->read_si().set(m_via0, FUNC(via6522_device::ca2_r));
	m_iocop->read_g().set(FUNC(lisa_state::iocop_g_r));
	m_iocop->write_d().set(FUNC(lisa_state::iocop_d_w));
	m_iocop->write_l().set(m_via0, FUNC(via6522_device::write_pa));
	m_iocop->read_l().set(m_via0, FUNC(via6522_device::read_pa));
	m_iocop->write_so().set(m_via0, FUNC(via6522_device::write_ca1));
	m_iocop->write_sk().set(FUNC(lisa_state::iocop_kbd_w));

	COP421(config, m_kbcop, 3.93216_MHz_XTAL); // same clock as iocop - required for working serial comms
	m_kbcop->set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, true);
	m_kbcop->read_si().set_constant(0);
	m_kbcop->read_l().set(FUNC(lisa_state::kbcop_l_r));
	m_kbcop->write_d().set(FUNC(lisa_state::kbcop_d_w));
	m_kbcop->write_sk().set(FUNC(lisa_state::kbcop_kbd_w));
	m_kbcop->write_so().set(FUNC(lisa_state::kbcop_so_w));
	m_kbcop->read_g().set(FUNC(lisa_state::kbcop_g_r));

	LISAFDC(config, m_fdc);
	m_fdc->write_diag().set(m_via1, FUNC(mos6522_device::write_pb6));
	m_fdc->write_fdir().set(m_ioir, FUNC(input_merger_device::in_w<0>));
	m_fdc->write_fdir().append(m_via0, FUNC(mos6522_device::write_pb4));

	LISAVIDEO(config, m_video, 20.37504_MHz_XTAL);
	m_video->set_ram(m_mainram);
	m_video->write_vint().set(m_ioir, FUNC(input_merger_device::in_w<1>));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 1.00);

	// software lists
	SOFTWARE_LIST(config, "disk_list").set_original("lisa");

	// via
	MOS6522(config, m_via0, 20.37504_MHz_XTAL / 40); // CPU E clock (nominally 500 kHz)
	m_via0->irq_handler().set_inputline(m_maincpu, 2);

	MOS6522(config, m_via1, 20.37504_MHz_XTAL / 40); // CPU E clock (nominally 500 kHz)
	m_via1->irq_handler().set(m_ioir, FUNC(input_merger_device::in_w<2>));

	SCC8530(config, m_scc, 7833600);

	QUADMOUSE(config, m_mouse);

	APPLEPP_CONNECTOR(config, m_pp, applepp_intf, nullptr);
	m_pp->write_pd().set(m_via1, FUNC(mos6522_device::write_pa));
	m_pp->write_pchk().set(m_via1, FUNC(mos6522_device::write_pb0));
	m_pp->write_pbsy().set(m_via1, FUNC(mos6522_device::write_pb1));
	m_pp->write_pbsy().append(m_via1, FUNC(mos6522_device::write_ca1));
	m_pp->write_pparity().set(m_via1, FUNC(mos6522_device::write_cb2));
	m_via1->ca2_handler().set(m_pp, FUNC(applepp_connector::pstrb_w));
	m_via1->writepa_handler().set(m_pp, FUNC(applepp_connector::pd_w));
	m_via1->writepb_handler().set([this](u8 data) {
									  m_pp->prw_w(BIT(data, 3));
									  m_pp->pcmd_w(BIT(data, 4));
									  m_pp->pres_w(BIT(data, 7));
								  });

	config.set_perfect_quantum(m_iocop);
}

void lisa_state::lisa2(machine_config &config)
{
	lisa(config);
	LISA2FDC(config.replace(), m_fdc);
	m_fdc->write_diag().set(m_via1, FUNC(mos6522_device::write_pb6));
	m_fdc->write_fdir().set(m_ioir, FUNC(input_merger_device::in_w<0>));
	m_fdc->write_fdir().append(m_via0, FUNC(mos6522_device::write_pb4));
}

void lisa_state::lisa210(machine_config &config)
{
	lisa(config);

	MACXLFDC(config.replace(), m_fdc);
	m_fdc->write_diag().set(m_via1, FUNC(mos6522_device::write_pb6));
	m_fdc->write_fdir().set(m_ioir, FUNC(input_merger_device::in_w<0>));
	m_fdc->write_fdir().append(m_via0, FUNC(mos6522_device::write_pb4));

	// via
	m_via0->set_clock(20.37504_MHz_XTAL / 16);
	m_via1->set_clock(20.37504_MHz_XTAL / 16);
}

void lisa_state::macxl(machine_config &config)
{
	lisa210(config);

	MACXLVIDEO(config.replace(), m_video, 20.37504_MHz_XTAL);
	m_video->set_ram(m_mainram);
	m_video->write_vint().set(m_ioir, FUNC(input_merger_device::in_w<1>));
}

void lisa_state::machine_start()
{
	// Leave stopped in reset at power-on, wait for the iocop to turn on the power
	m_maincpu->suspend(SUSPEND_REASON_RESET, true);

	save_item(NAME(m_power_button_forced_pressed));
	save_item(NAME(m_power_on));
	save_item(NAME(m_iocop_select));
	save_item(NAME(m_crdy));
	save_item(NAME(m_iocop_g3_forced_low));
	save_item(NAME(m_iocop_kbd));
	save_item(NAME(m_kbcop_kbd));
	save_item(NAME(m_kbd));
	save_item(NAME(m_kbcop_so));
	save_item(NAME(m_kbcop_d));

	m_power_button_timer = timer_alloc(FUNC(lisa_state::power_button_release), this);
	m_iocop_g3 = timer_alloc(FUNC(lisa_state::iocop_g3_freed), this);

	m_power_button_forced_pressed = true;
	m_power_on = false;
	m_iocop_g3_forced_low = false;
	m_crdy = false;
	m_iocop_select = 0;
	m_iocop_kbd = false;
	m_kbcop_kbd = false;
	m_kbd = true;
	m_kbcop_so = false;
	m_kbcop_d = 0;
}

void lisa_state::machine_reset()
{
	if(m_power_button_forced_pressed)
		m_power_button_timer->adjust(attotime::from_msec(100));
}

TIMER_CALLBACK_MEMBER(lisa_state::power_button_release)
{
	m_power_button_forced_pressed = false;
}

TIMER_CALLBACK_MEMBER(lisa_state::iocop_g3_freed)
{
	m_iocop_g3_forced_low = false;
}

void lisa_state::kbd_update()
{
	m_kbd = !(m_iocop_kbd || m_kbcop_kbd);
	if(0)
		logerror("kbd %d %d -> %d (%d)\n", m_iocop_kbd, m_kbcop_kbd, m_kbd, (machine().time().as_ticks(3932160*2)+1)/2);
}

u8 lisa_state::iocop_g_r()
{
	u8 g = (((!m_power_button_forced_pressed) && !m_iocop_g3_forced_low) ? 8 : 0) | 4;

	switch(m_iocop_select) {
	case 0:
		g |= (m_mouse->right_r() ? 1 : 0) | (m_mouse->left_r() ? 2 : 0);
		break;
	case 1:
		g |= (m_mouse->down_r() ? 1 : 0) | (m_mouse->up_r() ? 2 : 0);
		break;
	case 2:
		g |= m_mousebtn->read();
		break;
	case 3:
		g |= 2 | (m_kbd ? 1 : 0);
	}

	//  logerror("iocop g read %d\n", (machine().time().as_ticks(3932160*2)+1)/2);

	return g;
}

u8 lisa_state::kbcop_g_r()
{
	if(m_kbcop_so) {
		u32 pc = m_kbcop->pcbase();
		//      logerror("kbcop g read %d (%x)\n", (machine().time().as_ticks(3932160*2)+1)/2, m_kbcop->pcbase());
		// bit3=0 seems to mean reset?
		return 0xe | (m_kbd  && (pc != 0xb2000) ? 0 : 1);

	} else {
		int index = (m_kbcop_kbd ? 0x00 : 0x10) | m_kbcop_d;
		return m_keyboard[index]->read();
	}
}

void lisa_state::iocop_kbd_w(int state)
{
	m_iocop_kbd = state;
	kbd_update();
}

void lisa_state::kbcop_kbd_w(int state)
{
	//  logerror("kbcop sk %d\n", state);
	m_kbcop_kbd = state;
	kbd_update();
}

void lisa_state::kbcop_so_w(int state)
{
	m_kbcop_so = state;
	static int prev = 2;
	if(prev != state) {
		prev = state;
		logerror("kbcop_so_w %d\n", state);
	}
}

u8 lisa_state::kbcop_l_r()
{
	logerror("kbcop_l_r\n");
	return 0x01;
}

void lisa_state::kbcop_d_w(u8 data)
{
	m_kbcop_d = data;
	//  logerror("kbcop_d_w %x (%03x)\n", data, m_kbcop->pcbase());
}

void lisa_state::iocop_d_w(u8 data)
{
	bool old_power_on = m_power_on;
	m_power_on = data & 1;
	m_iocop_select = (data >> 1) & 3;
	m_crdy = data & 8;

	if(old_power_on != m_power_on) {
		if(m_power_on) {
			m_maincpu->resume(SUSPEND_REASON_RESET);
			m_iocop_g3_forced_low = true;
			m_iocop_g3->adjust(attotime::from_msec(5));
		} else
			m_maincpu->suspend(SUSPEND_REASON_RESET, true);
	}

	m_via0->write_pb6(m_crdy);

	if(0)
	logerror("iocop power %s select %d crdy %d\n",
			 m_power_on ? "on" : "off",
			 m_iocop_select,
			 m_crdy);
}


/* 2008-05 FP:
Small note about natural keyboard support: currently,
- "Clear" (on the Keypad) is mapped to 'F1'
- "Enter" (different from Return) is mapped to 'F2'

   2008-05 FP: Differences in European Layout (based on a couple of pictures found in the web):
- at KEYCODE_ESC, "`  ~" is replaced by "?  #"
- Shift + 3  gives the pound symbol (\xC2\xA3)
- There is no "\  |" key after "]  }"
- There is an additional key at 3rd row, 3rd key from 'L', and it's  "`  ~"
- Between Left Shift and Z there is another key, but the image is not clear on that key
*/

static INPUT_PORTS_START( lisa )
	PORT_START("mousebtn")
	PORT_BIT(0x0001, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Mouse Button 1")
	PORT_BIT(0x0002, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("Mouse Button 2")

	// In kab, b = line number, a = low bit of r.  Two top bits of r are the bit number in the port

	PORT_START("k00")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Clear") PORT_CODE(KEYCODE_NUMLOCK) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)             PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')

	PORT_START("k01")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)         PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)            PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('^')

	PORT_START("k02")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad + [\xe2\x97\x80]") PORT_CODE(KEYCODE_ASTERISK) PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
#if 1
	/* US layout */
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_UNUSED)
#else
	/* European layout */
	/* 2008-05 FP: Differences in European Layout (based on a couple of pictures found in the web):
	- at KEYCODE_ESC, "`  ~" is replaced by "?  #"
	- Shift + 3  gives the pound symbol (\xC2\xA3)
	- There is no "\  |" key after "]  }"
	- There is an additional key at 3rd row, 3rd key from 'L', and it's  "`  ~"
	- Between Left Shift and Z there is another key, but the image is not clear on that key
	*/
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("~  `") PORT_CODE(KEYCODE_BACKSLASH2)
#endif
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("k03")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad * [\xe2\x96\xb6]") PORT_CODE(KEYCODE_MINUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)         PORT_CHAR('\\') PORT_CHAR('|') // this one would be 2nd row, 3rd key from 'P'
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('*')

	PORT_START("k04")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)             PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("k05")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)             PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)     PORT_CHAR(8)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')

	PORT_START("k06")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)             PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_MENU) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')

	PORT_START("k07")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad / [\xe2\x96\xb2]") PORT_CODE(KEYCODE_PLUS_PAD) PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("k08")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)             PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)           PORT_CHAR('`') PORT_CHAR('~')

	PORT_START("k09")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)             PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)         PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')

	PORT_START("k0a")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)             PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("k0b")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad , [\xe2\x96\xbc]") PORT_CODE(KEYCODE_COMMA_PAD) PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))    // this one would be between '+' and 'Enter' on a modern keypad.
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("k0c")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)           PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')

	PORT_START("k0d")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)             PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)         PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')

	PORT_START("k0e")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)             PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Option") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(RALT))
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')

	PORT_START("k0f")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)         PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')

	PORT_START("k10")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("k11")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('@')

	PORT_START("k12")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("k13")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')

	PORT_START("k14")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("k15")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')

	PORT_START("k16")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("k17")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("k18")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('l') PORT_CHAR('M')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')

	PORT_START("k19")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('m') PORT_CHAR('L')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')

	PORT_START("k1a")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')

	PORT_START("k1b")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("k1c")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Option") PORT_CODE(KEYCODE_LALT)                 PORT_CHAR(UCHAR_MAMEKEY(LALT))

	PORT_START("k1d")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Alpha Lock") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE  PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("k1e")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)                     PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("k1f")
	PORT_BIT(1, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(2, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(4, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(8, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Command") PORT_CODE(KEYCODE_LCONTROL)                 PORT_CHAR(UCHAR_SHIFT_2)
INPUT_PORTS_END

	/* Note we're missing a whole bunch of lisa bootrom revisions; based on http://www.cs.dartmouth.edu/~woz/bootrom.pdf :
	?A?(10/12/82) ?B?(11/19/82) ?C?(1/28/83) D(5/12/83) 3B(9/8/83) E(10/20/83) F(12/21/83) G(2/8/84) and H(2/24/84) are known to exist. Earlier prototypes existed as well. Only F and H are dumped. */
	/* Based on http://www.cs.dartmouth.edu/~woz/bootrom.pdf and other information, at least the following systems existed:
	 * Lisa: two 890K twiggy drives, old MB (slow 500khz-clock parallel port via), old I/O board w/io40 disk ROM, supports profile HDD, all bootrom revs (practically speaking, it only appeared with bootroms A, B, C, D, E or F)
	 * Lisa2 (aka Lisa2/5): one 400K SSDD drive, old MB (slow 500khz-clock parallel port via), old I/O board w/ioa8 disk ROM, supports profile HDD, bootrom revs "3B", E, F, G, H
	 * Lisa2/10: one 400K SSDD drive, new MB (fast 1.25MHz-clock parallel port via), new i/o board w/ioa8 disk rom, internal widget 10mb drive (no profile hdd ports), bootrom revs F, G, H
	 * MacXL: one 400K SSDD drive, new MB (fast 1.25MHz-clock parallel port via), new I/O board w/io88 disk rom, internal widget 10mb drive (no profile hdd ports), bootrom rev 3A, different screen aspect, no serial number in video state rom so lisa office would not run
	 * Sun-remanufactured MacXL: one 800K DSDD drive, new MB (fast 1.25MHz-clock parallel port via), sun-made custom disk rom (3 revisions exist), internal custom 10mb,20mb, or 40mb drive (?no profile hdd ports?), bootrom rev 3A, different screen aspect, no serial number in video state rom so lisa apps would not run
	 */
		/* the old I/O board has a battery pack on it which often leaks and destroys the board, and has a socket for an amd 2915 math co-procesor; the new I/O board lacks the battery pack and the socket for the coprocessor, and integrates some of the function of the old twiggy-to-400k drive convertor board (which all lisa2/5s had) on it, requiring a mod to be done to the old convertor if used with a new I/O board.*/
	/* Twiggy disk format notes: twiggy disks seem to have wide '48tpi' heads, but cram 62.5tpi on the media by closely spacing the tracks! Twiggy media is DSHD-grade (needing strong magnetic field to set due to high data rate, see http://www.folklore.org/StoryView.py?project=Macintosh&story=Hide_Under_This_Desk.txt). The twiggy format is *PROBABLY* GCR encoding similar to apple2 and mac800k. The disks are 5.25" disks with TWO holes for the drive heads on both sides of the media, and the write protect notch in an unusual place (the index hole is in its normal position, though). By using variable motor speed similar to the later apple 3.5" disks, double sided disks, and tight track spacing, 871,424 bytes are stored per disk. see http://www.brouhaha.com/~eric/retrocomputing/lisa/twiggy.html
	The drives were notoriously unreliable and were replaced by a single SSDD Sony-made varialble speed 400k drive in the lisa2, which was also used on the original macintosh. */
	/* which systems used the 341-0193-A parallel interface card rom? */


ROM_START( lisa ) /* with twiggy drives, io40 i/o rom; technically any of the bootroms will work on this. */
	ROM_REGION16_BE(0x4000,"maincpu",0)   /* 68k rom and ram */
	ROM_DEFAULT_BIOS( "revh" )

	ROM_SYSTEM_BIOS( 0, "revh", "LISA Bootrom Rev H (2/24/84)")
	ROMX_LOAD("341-0175-h", 0x000000, 0x2000, CRC(adfd4516) SHA1(97a89ce1218b8aa38f69f92f6f363f435c887914), ROM_SKIP(1) | ROM_BIOS(0)) // 341-0175-H LISA Bootrom Rev H (2/24/84) (High)
	ROMX_LOAD("341-0176-h", 0x000001, 0x2000, CRC(546d6603) SHA1(2a81e4d483f50ae8a2519621daeb7feb440a3e4d), ROM_SKIP(1) | ROM_BIOS(0)) // 341-0176-H LISA Bootrom Rev H (2/24/84) (Low)

	ROM_SYSTEM_BIOS( 1, "revg", "LISA Bootrom Rev G (2/08/84)") // limited test release before release of rom rev H
	ROMX_LOAD("341-0175-g", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1)) // 341-0175-G LISA Bootrom Rev G (2/08/84) (High)
	ROMX_LOAD("341-0176-g", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1)) // 341-0176-G LISA Bootrom Rev G (2/08/84) (Low)

	ROM_SYSTEM_BIOS( 2, "revf", "LISA Bootrom Rev F (12/21/83)")
	ROMX_LOAD("341-0175-f", 0x000000, 0x2000, CRC(701b9dab) SHA1(b116e5fada7b9a51f1b6e25757b2814d1b2737a5), ROM_SKIP(1) | ROM_BIOS(2)) // 341-0175-F LISA Bootrom Rev F (12/21/83) (High)
	ROMX_LOAD("341-0176-f", 0x000001, 0x2000, CRC(036010b6) SHA1(ac93e6dbe4ce59396d7d191ee3e3e79a504e518f), ROM_SKIP(1) | ROM_BIOS(2)) // 341-0176-F LISA Bootrom Rev F (12/21/83) (Low)

	ROM_SYSTEM_BIOS( 3, "reve", "LISA Bootrom Rev E (10/20/83)")
	ROMX_LOAD("341-0175-e", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(3)) // 341-0175-E LISA Bootrom Rev E (10/20/83) (High)
	ROMX_LOAD("341-0176-e", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(3)) // 341-0176-E LISA Bootrom Rev E (10/20/83) (Low)

	ROM_SYSTEM_BIOS( 4, "revd", "LISA Bootrom Rev D (5/12/83)")
	ROMX_LOAD("341-0175-d", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4)) // 341-0175-D LISA Bootrom Rev D (5/12/83) (High)
	ROMX_LOAD("341-0176-d", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4)) // 341-0176-D LISA Bootrom Rev D (5/12/83) (Low)

	ROM_SYSTEM_BIOS( 5, "revc", "LISA Bootrom Rev C (1/28/83?)")
	ROMX_LOAD("341-0175-c", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(5)) // 341-0175-C LISA Bootrom Rev C (1/28/83) (High)
	ROMX_LOAD("341-0176-c", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(5)) // 341-0176-C LISA Bootrom Rev C (1/28/83) (Low)

	ROM_SYSTEM_BIOS( 6, "revb", "LISA Bootrom Rev B (11/19/82?)")
	ROMX_LOAD("341-0175-b", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(6)) // 341-0175-B LISA Bootrom Rev B (11/19/82?) (High)
	ROMX_LOAD("341-0176-b", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(6)) // 341-0176-B LISA Bootrom Rev B (11/19/82?) (Low)

	ROM_SYSTEM_BIOS( 7, "reva", "LISA Bootrom Rev A (10/12/82?)") // possibly only prototypes
	ROMX_LOAD("341-0175-a", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(7)) // 341-0175-A LISA Bootrom Rev A (10/12/82?) (High)
	ROMX_LOAD("341-0176-a", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(7)) // 341-0176-A LISA Bootrom Rev A (10/12/82?) (Low)

	ROM_REGION( 0x400, "iocop", 0 )
	ROM_LOAD("341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b))

	ROM_REGION( 0x400, "kbcop", 0 )
	ROM_LOAD("341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b))

	ROM_REGION(0x4000,"profile", 0)     // Profile/5 HDD
	ROM_LOAD_OPTIONAL("341-0080-b", 0x0000, 0x800, CRC(26df0b8d) SHA1(08f6689afb517e0a2bdaa48433003e62a66ae3c7)) // 341-0080-B z8 MCU piggyback ROM

	// TODO: the 341-0193-A parallel interface card rom should be loaded here as well for the lisa 1 and 2/5?
ROM_END

ROM_START( lisa2 ) /* internal apple codename was 'pepsi'; has one SSDD 400K drive, ioa8 i/o rom */
	ROM_REGION16_BE(0x4000,"maincpu",0)   /* 68k rom and ram */
	ROM_DEFAULT_BIOS( "revh" )

	ROM_SYSTEM_BIOS( 0, "revh", "LISA Bootrom Rev H (2/24/84)")
	ROMX_LOAD("341-0175-h", 0x000000, 0x2000, CRC(adfd4516) SHA1(97a89ce1218b8aa38f69f92f6f363f435c887914), ROM_SKIP(1) | ROM_BIOS(0)) // 341-0175-H LISA Bootrom Rev H (2/24/84) (High)
	ROMX_LOAD("341-0176-h", 0x000001, 0x2000, CRC(546d6603) SHA1(2a81e4d483f50ae8a2519621daeb7feb440a3e4d), ROM_SKIP(1) | ROM_BIOS(0)) // 341-0176-H LISA Bootrom Rev H (2/24/84) (Low)

	ROM_SYSTEM_BIOS( 1, "revg", "LISA Bootrom Rev G (2/08/84)") // limited test release before release of rom rev H
	ROMX_LOAD("341-0175-g", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1)) // 341-0175-G LISA Bootrom Rev G (2/08/84) (High)
	ROMX_LOAD("341-0176-g", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1)) // 341-0176-G LISA Bootrom Rev G (2/08/84) (Low)

	ROM_SYSTEM_BIOS( 2, "revf", "LISA Bootrom Rev F (12/21/83)")
	ROMX_LOAD("341-0175-f", 0x000000, 0x2000, CRC(701b9dab) SHA1(b116e5fada7b9a51f1b6e25757b2814d1b2737a5), ROM_SKIP(1) | ROM_BIOS(2)) // 341-0175-F LISA Bootrom Rev F (12/21/83) (High)
	ROMX_LOAD("341-0176-f", 0x000001, 0x2000, CRC(036010b6) SHA1(ac93e6dbe4ce59396d7d191ee3e3e79a504e518f), ROM_SKIP(1) | ROM_BIOS(2)) // 341-0176-F LISA Bootrom Rev F (12/21/83) (Low)

	ROM_SYSTEM_BIOS( 3, "reve", "LISA Bootrom Rev E (10/20/83)")
	ROMX_LOAD("341-0175-e", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(3)) // 341-0175-E LISA Bootrom Rev E (10/20/83) (High)
	ROMX_LOAD("341-0176-e", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(3)) // 341-0176-E LISA Bootrom Rev E (10/20/83) (Low)

	ROM_SYSTEM_BIOS( 4, "rev3b", "LISA Bootrom Rev 3B (9/8/83)") // Earliest lisa2 rom, prototype.
	ROMX_LOAD("341-0175-3b", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4)) // ?label? 341-0175-3b LISA Bootrom Rev 3B (9/8/83) (High)
	ROMX_LOAD("341-0176-3b", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(4)) // ?label? 341-0176-3b LISA Bootrom Rev 3B (9/8/83) (Low)

	ROM_REGION( 0x400, "iocop", 0 )
	ROM_LOAD("341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b))

	ROM_REGION( 0x400, "kbcop", 0 )
	ROM_LOAD("341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b))

	ROM_REGION(0x4000,"profile", 0)     // Profile/5 HDD
	ROM_LOAD_OPTIONAL("341-0080-b", 0x0000, 0x800, CRC(26df0b8d) SHA1(08f6689afb517e0a2bdaa48433003e62a66ae3c7)) // 341-0080-B z8 MCU piggyback ROM

	// TODO: the 341-0193-A parallel interface card rom should be loaded here as well for the lisa 1 and 2/5?
ROM_END

ROM_START( lisa210 ) // newer motherboard and I/O board; has io88 I/O ROM, built in widget HDD
	ROM_REGION16_BE(0x204000,"maincpu", 0)  // 68k ROM and RAM
	ROM_DEFAULT_BIOS( "revh" )
	ROM_SYSTEM_BIOS(0, "revh", "LISA Bootrom Rev H (2/24/84)")
	ROMX_LOAD("341-0175-h", 0x000000, 0x2000, CRC(adfd4516) SHA1(97a89ce1218b8aa38f69f92f6f363f435c887914), ROM_SKIP(1) | ROM_BIOS(0)) // 341-0175-H LISA Bootrom Rev H (2/24/84) (High)
	ROMX_LOAD("341-0176-h", 0x000001, 0x2000, CRC(546d6603) SHA1(2a81e4d483f50ae8a2519621daeb7feb440a3e4d), ROM_SKIP(1) | ROM_BIOS(0)) // 341-0176-H LISA Bootrom Rev H (2/24/84) (Low)

	ROM_SYSTEM_BIOS(1, "revg", "LISA Bootrom Rev G (2/08/84)") // limited test release before release of rom rev H
	ROMX_LOAD("341-0175-g", 0x000000, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1)) // 341-0175-G LISA Bootrom Rev G (2/08/84) (High)
	ROMX_LOAD("341-0176-g", 0x000001, 0x2000, NO_DUMP, ROM_SKIP(1) | ROM_BIOS(1)) // 341-0176-G LISA Bootrom Rev G (2/08/84) (Low)

	ROM_SYSTEM_BIOS(2, "revf", "LISA Bootrom Rev F (12/21/83)")
	ROMX_LOAD("341-0175-f", 0x000000, 0x2000, CRC(701b9dab) SHA1(b116e5fada7b9a51f1b6e25757b2814d1b2737a5), ROM_SKIP(1) | ROM_BIOS(2)) // 341-0175-F LISA Bootrom Rev F (12/21/83) (High)
	ROMX_LOAD("341-0176-f", 0x000001, 0x2000, CRC(036010b6) SHA1(ac93e6dbe4ce59396d7d191ee3e3e79a504e518f), ROM_SKIP(1) | ROM_BIOS(2)) // 341-0176-F LISA Bootrom Rev F (12/21/83) (Low)

	ROM_REGION( 0x400, "iocop", 0 )
	ROM_LOAD("341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b))

	ROM_REGION( 0x400, "kbcop", 0 )
	ROM_LOAD("341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b))

	ROM_REGION(0x4000,"widget", 0)      // Widget HDD controller
	ROM_LOAD("341-0288-a", 0x0000, 0x0800, CRC(a26ef1c6) SHA1(5aaeb6ff7f7d4f7ce7c70402f75e82533635dda4)) // 341-0288-A z8 MCU piggyback ROM
	ROM_LOAD("341-0289-d", 0x2000, 0x2000, CRC(25e86e95) SHA1(72a346c2074d2256adde491b930023ebdcb5f51a)) // 341-0289-D external rom on widget board

	ROM_REGION(0x100,"gfx1", 0)     // video ROM (includes S/N)
	ROM_LOAD("vidstate.rom", 0x00, 0x100, CRC(75904783) SHA1(3b0023bd90f2ca1be0b099160a566b044856885d))
ROM_END

ROM_START( macxl )
	ROM_REGION16_BE(0x4000,"maincpu", 0)  /* 68k rom and ram */
	ROM_LOAD16_BYTE("341-0347-a", 0x000000, 0x2000, CRC(80add605) SHA1(82215688b778d8c712a8186235f7981e3dc4dd7f)) // 341-0347-A Mac XL '3A' Bootrom Hi (boot3a.hi)
	ROM_LOAD16_BYTE("341-0346-a", 0x000001, 0x2000, CRC(edf5222f) SHA1(b0388ee8dbbc51a2d628473dc29b65ce913fcd76)) // 341-0346-A Mac XL '3A' Bootrom Lo (boot3a.lo)

	ROM_REGION( 0x400, "iocop", 0 )
	ROM_LOAD("341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b))

	ROM_REGION( 0x400, "kbcop", 0 )
	ROM_LOAD("341-0064a.u9f", 0x000, 0x400, CRC(e6849910) SHA1(d46e67df75c9e3e773d20542fb9d5b1d2ac0fb9b))

#if 1
	ROM_REGION(0x1000,"fdccpu", 0)      // 6504 RAM and ROM
	ROM_LOAD("341-0281-d", 0x0000, 0x1000, CRC(e343fe74) SHA1(a0e484ead2d2315fca261f39fff2f211ff61b0ef)) // 341-0281-D LISA 2/10 Disk Rom (io88), supports widget on internal port
#else
	ROM_REGION(0x1000,"fdccpu", 0)      // 6504 RAM and ROM
	ROM_LOAD("341-8003-c", 0x0000, 0x1000, CRC(8c67959a) SHA1(aa446b0c4acb4cb6c9d0adfbbea900fb8c04c1e9)) // 341-8003-C Sun Mac XL Disk rom for 800k drives (Rev C, from Goodwill XL) (io88800k)
	// Note: there are two earlier/alternate versions of this ROM as well which are dumped
#endif

	ROM_REGION(0x100,"gfx1", 0)     // video ROM (includes S/N)
	ROM_LOAD("341-0348a_mb7118e.u6c", 0x00, 0x100, CRC(778fc5d9) SHA1(ec2533a6bd9e75d02fa69754fb82c7c28f0366ab))
ROM_END

} // anonymous namespace

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT          COMPANY           FULLNAME */
COMP( 1983, lisa,    0,      0,      lisa,    lisa,  lisa_state, empty_init,   "Apple Computer", "Lisa",         0 )
COMP( 1984, lisa2,   0,      0,      lisa2,   lisa,  lisa_state, empty_init,   "Apple Computer", "Lisa2",        0 )
COMP( 1984, lisa210, lisa2,  0,      lisa210, lisa,  lisa_state, empty_init,   "Apple Computer", "Lisa2/10",     0 )
COMP( 1985, macxl,   lisa2,  0,      macxl,   lisa,  lisa_state, empty_init,   "Apple Computer", "Macintosh XL", 0 )

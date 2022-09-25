// license: BSD-3-Clause
// copyright-holders: Charles MacDonald, Devin Hill, Fabio Dalla Libera

/*
  license:BSD-3-Clause
  Charles MacDonald
  Devin Hill
  Fabio Dalla Libera

  6805 dumped using https://github.com/charlesmacd/HD6805_Reader

  IC list:

  * M5L8035LP (Mitsubishi clone of I8035)

  P1 is the deserialized output data
  P2.4 is the input serial data
  P2.5 is busy (output to the computer)
  P2.6 is the output strobe
  P2.7 is busy (input from the 6805)


  * HD6805V (Hitachi chip, pin compatible with MC6805P2, with 4K of ROM)

  port B connected to the motors
  port C connected to the pen and misc signals
  port D connected to M5L8035LP parallel data

*/


#include "emu.h"
#include "sp400.h"


sp400_printer_device::sp400_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock):
	device_t(mconfig, SP400_PRINTER, tag, owner, clock),
	device_sk1100_printer_port_interface(mconfig, *this),
	m_dsercpu(*this, "dsercpu"),
	m_motcpu(*this, "motcpu"),
	m_data(0),
	m_busy(1),
	m_dserdata(0xFF),
	m_dserstrobe(1),
	m_motbusy(0),
	m_motPenUp(1),
	m_motPenDown(1),
	m_plotter(*this, "plotter"),
	m_frontbuttons(*this, "FRONTBUTTONS"),
	m_misc(*this, "MISC")
{
}

void sp400_printer_device::device_start()
{
}

void sp400_printer_device::dser_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void sp400_printer_device::dser_p1_w(uint8_t v)
{
	m_dserdata = v;
}

void sp400_printer_device::dser_p2_w(uint8_t v)
{
	m_busy = BIT(v, 5);
	m_dserstrobe = BIT(v, 6);
	m_motcpu->set_input_line(M6805_IRQ_LINE, m_dserstrobe);
}

uint8_t sp400_printer_device::dser_p2_r()
{
	return
		(m_motbusy << 7) |
		(m_dserstrobe << 6) |
		(m_busy << 5) |
		(m_data << 4) |
		(1 << 3) |
		(1 << 2) |
		(1 << 1) |
		(1 << 0);
}

uint8_t sp400_printer_device::mot_pa_r()
{
	return
		(0 << 0) |
		(0 << 1) |
		(m_frontbuttons->read() << 2) |  // bits 2,3 and 4
		(0 << 5) |
		(0 << 6) |
                (1 << 7);
}

void sp400_printer_device::mot_pb_w(uint8_t v)
{
	m_plotter->update_motors(v & 0x0F, (v >> 4) & 0x0F);
}


uint8_t sp400_printer_device::mot_pc_r()
{
	return
		(m_motPenUp << 0) |
		(m_motPenDown << 1) |
		(1 << 2) |
		(m_plotter->get_reedswitch_state() << 3) |
		(m_motbusy << 4) |
		(1 << 5) | (1 << 6) | (1 << 7); //unconfirmed
}

void sp400_printer_device::mot_pc_w(uint8_t v)
{
	m_motPenUp = BIT(v, 0);
	m_motPenDown = BIT (v, 1);
	m_motbusy = BIT(v, 4);
	update_pen_state();
}

uint8_t sp400_printer_device::mot_pd_r()
{
	return sp400_printer_device::m_dserdata;
}

void sp400_printer_device::update_pen_state()
{
	if (!m_motPenDown)
	{
		m_plotter->pen_down();
	}
	if (!m_motPenUp)
	{
		m_plotter->pen_up();
	}
}

void sp400_printer_device::device_add_mconfig(machine_config &config)
{
	I8035(config, m_dsercpu, 4_MHz_XTAL);
	m_dsercpu->p1_out_cb().set(FUNC(sp400_printer_device::dser_p1_w));
	m_dsercpu->p2_in_cb().set(FUNC(sp400_printer_device::dser_p2_r));
	m_dsercpu->p2_out_cb().set(FUNC(sp400_printer_device::dser_p2_w));
	m_dsercpu->set_addrmap(AS_PROGRAM, &sp400_printer_device::dser_map);
	M6805U3(config, m_motcpu, 4_MHz_XTAL);
	m_motcpu->porta_r().set(FUNC(sp400_printer_device::mot_pa_r));
	m_motcpu->portb_w().set(FUNC(sp400_printer_device::mot_pb_w));
	m_motcpu->portc_r().set(FUNC(sp400_printer_device::mot_pc_r));
	m_motcpu->portc_w().set(FUNC(sp400_printer_device::mot_pc_w));
	m_motcpu->portd_r().set(FUNC(sp400_printer_device::mot_pd_r));
	ALPS_DPG1302(config, m_plotter);
	m_plotter->set_panel_update(FUNC(sp400_printer_device::update_panel));
}

uint32_t sp400_printer_device::update_panel(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
        static constexpr int xstart=50;
        static constexpr int xpitch=110;
        static constexpr int width=60;
        static constexpr int height=20;
        static constexpr int h=450-50+15;

        static constexpr uint32_t NORMALCOLOR = 0x808080;
	static constexpr uint32_t PRESSEDCOLOR = 0x404040;

        int buttons = m_frontbuttons->read();

        bitmap.plot_box(xstart+0*xpitch,h,width,height,0x00C000);
	bitmap.plot_box(xstart+1*xpitch,h,width,height,m_motbusy ? 0xFF0000 : 0x100000);
	bitmap.plot_box(xstart+2*xpitch,h,width,height,buttons&0x02?NORMALCOLOR:PRESSEDCOLOR);
	bitmap.plot_box(xstart+3*xpitch,h,width,height,buttons&0x01?NORMALCOLOR:PRESSEDCOLOR);
	bitmap.plot_box(xstart+4*xpitch,h,width,height,buttons&0x04?NORMALCOLOR:PRESSEDCOLOR);

	return 0;
}

INPUT_CHANGED_MEMBER(sp400_printer_device::misc_interaction)
{
	if (BIT(m_misc->read(), 0) == 0)
	{
		m_plotter->write_snapshot_to_file();
	}
	if (BIT(m_misc->read(), 1) == 0)
	{
		m_plotter->change_paper();
	}
}

ROM_START( sp400 )
	ROM_REGION( 0x1000, "dsercpu", 0 )
	ROM_LOAD( "sp400_8035.bin", 0x0000, 0x1000, CRC(0eb48272) SHA1(08a2727f1592f5d2ecb2e368126ad7bfc5d3c270) )

	ROM_REGION(0x1000, "motcpu", 0 )
	ROM_LOAD( "sp400_hd6805v1.bin", 0x0000, 0x1000, CRC(aa073745) SHA1(65016f3b022af30cc6b084af1e43b29168721a60) )
ROM_END

const tiny_rom_entry * sp400_printer_device::device_rom_region() const
{
	return ROM_NAME( sp400 );
}

static INPUT_PORTS_START( sp400_ipt )
	PORT_START("FRONTBUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COLOR SELECT") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINE FEED") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("PEN CHANGE") PORT_CODE(KEYCODE_3_PAD)
	PORT_START("MISC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, sp400_printer_device, misc_interaction, 0 )  PORT_NAME("Save snapshot") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHANGED_MEMBER(DEVICE_SELF, sp400_printer_device, misc_interaction, 0 )  PORT_NAME("Change paper") PORT_CODE(KEYCODE_8_PAD)
INPUT_PORTS_END

ioport_constructor sp400_printer_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(sp400_ipt);
}

DEFINE_DEVICE_TYPE(SP400_PRINTER, sp400_printer_device, "sp_400", "SP-400 Plotter")

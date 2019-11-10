// license:GPL-2.0+
// copyright-holders:Dirk Best, Carl
/***************************************************************************

    Siemens PC-D

    For PC-X HDD should have 306,4,9 chs at 1024Bps or 18 at 512Bps

***************************************************************************/

#include "emu.h"
#include "machine/pcd_kbd.h"
#include "video/pcd.h"

#include "bus/rs232/rs232.h"
#include "bus/scsi/omti5100.h"
#include "cpu/i86/i186.h"
#include "imagedev/floppy.h"
#include "machine/mc146818.h"
#include "machine/mc2661.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "sound/spkrdev.h"

#include "speaker.h"

#include "formats/pc_dsk.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pcd_state : public driver_device
{
public:
	pcd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pic1(*this, "pic1")
		, m_pic2(*this, "pic2")
		, m_speaker(*this, "speaker")
		, m_fdc(*this, "fdc")
		, m_rtc(*this, "rtc")
		, m_usart(*this, "usart%u", 1U)
		, m_scsi(*this, "scsi")
		, m_scsi_data_out(*this, "scsi_data_out")
		, m_scsi_data_in(*this, "scsi_data_in")
		, m_ram(*this, "ram")
	{ }

	void pcx(machine_config &config);
	void pcd(machine_config &config);

private:
	DECLARE_READ8_MEMBER( irq_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( timer0_tick );
	DECLARE_WRITE_LINE_MEMBER( i186_timer1_w );

	DECLARE_READ8_MEMBER( nmi_io_r );
	DECLARE_WRITE8_MEMBER( nmi_io_w );
	DECLARE_READ8_MEMBER( stat_r );
	DECLARE_WRITE8_MEMBER( stat_w );
	DECLARE_READ8_MEMBER( led_r );
	DECLARE_WRITE8_MEMBER( led_w );
	DECLARE_READ16_MEMBER( dskctl_r );
	DECLARE_WRITE16_MEMBER( dskctl_w );

	DECLARE_READ8_MEMBER( scsi_r );
	DECLARE_WRITE8_MEMBER( scsi_w );
	DECLARE_READ16_MEMBER( mmu_r );
	DECLARE_WRITE16_MEMBER( mmu_w );
	DECLARE_READ16_MEMBER( mem_r );
	DECLARE_WRITE16_MEMBER( mem_w );

	DECLARE_FLOPPY_FORMATS( floppy_formats );
	DECLARE_WRITE_LINE_MEMBER(write_scsi_bsy);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_cd);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_io);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_msg);
	DECLARE_WRITE_LINE_MEMBER(write_scsi_req);

	void pcd_io(address_map &map);
	void pcd_map(address_map &map);
	void pcx_io(address_map &map);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<i80186_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<speaker_sound_device> m_speaker;
	required_device<wd2793_device> m_fdc;
	required_device<mc146818_device> m_rtc;
	required_device_array<mc2661_device, 3> m_usart;
	required_device<scsi_port_device> m_scsi;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<ram_device> m_ram;
	uint8_t m_stat, m_led;
	int m_msg, m_bsy, m_io, m_cd, m_req, m_rst;
	uint16_t m_dskctl;
	struct {
		uint16_t ctl;
		uint16_t regs[1024];
		int type;
		bool sc;
	} m_mmu;

	void check_scsi_irq();
};


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void pcd_state::machine_start()
{
	save_item(NAME(m_mmu.ctl));
	save_item(NAME(m_mmu.regs));
}

void pcd_state::machine_reset()
{
	m_stat = 0;
	m_led = 0;
	m_dskctl = 0;
	m_rst = 0;
	m_mmu.ctl = 0;
	m_mmu.sc = false;
	if(ioport("mmu"))
		m_mmu.type = ioport("mmu")->read();
	else
		m_mmu.type = 0;
}

READ8_MEMBER( pcd_state::irq_callback )
{
	return (offset ? m_pic2 : m_pic1)->acknowledge();
}

TIMER_DEVICE_CALLBACK_MEMBER( pcd_state::timer0_tick )
{
	m_maincpu->tmrin0_w(0);
	m_maincpu->tmrin0_w(1);
}

WRITE_LINE_MEMBER( pcd_state::i186_timer1_w )
{
	if(m_dskctl & 0x20)
		m_speaker->level_w(state);
}

READ8_MEMBER( pcd_state::nmi_io_r )
{
	if(machine().side_effects_disabled())
		return 0;
	logerror("%s: unmapped %s %04x\n", machine().describe_context(), space.name(), offset);
	m_stat |= 0xfd;
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	return 0;
}

WRITE8_MEMBER( pcd_state::nmi_io_w )
{
	if(machine().side_effects_disabled())
		return;
	logerror("%s: unmapped %s %04x\n", machine().describe_context(), space.name(), offset);
	m_stat |= 0xfd;
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

READ8_MEMBER( pcd_state::stat_r )
{
	return m_stat;
}

WRITE8_MEMBER( pcd_state::stat_w )
{
	m_stat &= ~data;
}

READ16_MEMBER( pcd_state::dskctl_r )
{
	return m_dskctl;
}

WRITE16_MEMBER( pcd_state::dskctl_w )
{
	floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();

	COMBINE_DATA(&m_dskctl);

	if((m_dskctl & 1) && floppy0)
		m_fdc->set_floppy(floppy0);
	if((m_dskctl & 2) && floppy1)
		m_fdc->set_floppy(floppy1);

	if(floppy0)
	{
		floppy0->mon_w(!(m_dskctl & 4));
		floppy0->ss_w((m_dskctl & 8) != 0);
	}
	if(floppy1)
	{
		floppy1->mon_w(!(m_dskctl & 4));
		floppy1->ss_w((m_dskctl & 8) != 0);
	}
	m_fdc->dden_w((m_dskctl & 0x10) ? 1 : 0);
}

READ8_MEMBER( pcd_state::led_r )
{
	// DIPs?
	// 0x01 no mmu
	// 0x10 enter monitor after post
	// 0x20 enter monitor before post
	return 0x01;
}

WRITE8_MEMBER( pcd_state::led_w )
{
	for(int i = 0; i < 6; i++)
		logerror("%c", (data & (1 << i)) ? '-' : '*');
	logerror("\n");
	m_led = data;
}

READ16_MEMBER( pcd_state::mmu_r )
{
	uint16_t data = m_mmu.regs[((m_mmu.ctl & 0x1f) << 5) | ((offset >> 2) & 0x1f)];
	//logerror("%s: mmu read %04x %04x\n", machine().describe_context(), (offset << 1) + 0x8000, data);
	if(!offset)
		return m_mmu.ctl;
	else if((offset >= 0x200) && (offset < 0x300) && !(offset & 3))
		return (data << 4) | (data >> 12) | (m_mmu.sc && (offset == 0x200) ? 0xc0 : 0);
	else if(offset == 0x400)
	{
		m_mmu.sc = false;
		m_pic1->ir0_w(CLEAR_LINE);
	}
	return 0;
}

WRITE16_MEMBER( pcd_state::mmu_w )
{
	//logerror("%s: mmu write %04x %04x\n", machine().describe_context(), (offset << 1) + 0x8000, data);
	if(!offset)
		m_mmu.ctl = data;
	else if((offset >= 0x200) && (offset < 0x300) && !(offset & 3))
		m_mmu.regs[((m_mmu.ctl & 0x1f) << 5) | ((offset >> 2) & 0x1f)] = (data >> 4) | (data << 12);
	else if(offset == 0x400)
	{
		m_mmu.sc = true;
		m_pic1->ir0_w(ASSERT_LINE);
	}
}

READ8_MEMBER(pcd_state::scsi_r)
{
	uint8_t ret = 0;

	switch(offset)
	{
		case 0:
		case 2:
			ret = m_scsi_data_in->read();
			m_scsi->write_ack(1);
			if(!offset)
				m_maincpu->drq0_w(0);
			break;

		case 1:
			ret = (m_cd << 7) | (m_req << 5) | (m_bsy << 4);
			break;
	}

	return ret;
}

WRITE8_MEMBER(pcd_state::scsi_w)
{
	switch(offset)
	{
		case 0:
			m_scsi_data_out->write(data);
			m_scsi->write_ack(1);
			m_maincpu->drq0_w(0);
			break;
		case 1:
			if(data & 4)
			{
				m_rst = 1;
				m_scsi->write_rst(1);
				break;
			}
			if(m_rst)
			{
				m_rst = 0;
				m_scsi->write_rst(0);
				break;
			}

			if(!m_bsy)
			{
				m_scsi_data_out->write(0);
				m_scsi->write_sel(1);
			}
			break;
	}
}

void pcd_state::check_scsi_irq()
{
	m_pic1->ir5_w(m_io && m_cd && m_req);
}

WRITE_LINE_MEMBER(pcd_state::write_scsi_bsy)
{
	m_bsy = state ? 1 : 0;
	m_scsi->write_sel(0);
}
WRITE_LINE_MEMBER(pcd_state::write_scsi_cd)
{
	m_cd = state ? 1 : 0;
	check_scsi_irq();
}
WRITE_LINE_MEMBER(pcd_state::write_scsi_io)
{
	m_io = state ? 1 : 0;
	if(state)
		m_scsi_data_out->write(0);
	check_scsi_irq();
}
WRITE_LINE_MEMBER(pcd_state::write_scsi_msg)
{
	m_msg = state ? 1 : 0;
}

WRITE_LINE_MEMBER(pcd_state::write_scsi_req)
{
	m_req = state ? 1 : 0;
	if(state)
	{
		if(!m_cd)
		{
			m_maincpu->drq0_w(1);
		}
		else if(m_msg)
		{
			m_scsi_data_in->read();
			m_scsi->write_ack(1);
		}
	}
	else
	{
		m_scsi->write_ack(0);
	}
	check_scsi_irq();
}

WRITE16_MEMBER(pcd_state::mem_w)
{
	uint16_t *ram = (uint16_t *)m_ram->pointer();
	if((m_mmu.ctl & 0x20) && m_mmu.type)
	{
		uint16_t reg;
		if(m_mmu.type == 2)
			reg = m_mmu.regs[((offset >> 10) & 0xff) | ((m_mmu.ctl & 0x18) << 5)];
		else
			reg = m_mmu.regs[((offset >> 10) & 0x7f) | ((m_mmu.ctl & 0x1c) << 5)];
		if(!reg && !machine().side_effects_disabled())
		{
			offset <<= 1;
			logerror("%s: Null mmu entry %06x\n", machine().describe_context(), offset);
			nmi_io_w(space, offset, data, mem_mask);
			return;
		}
		offset = ((reg << 3) & 0x7fc00) | (offset & 0x3ff);
	}
	COMBINE_DATA(&ram[offset]);
}

READ16_MEMBER(pcd_state::mem_r)
{
	uint16_t *ram = (uint16_t *)m_ram->pointer();
	if((m_mmu.ctl & 0x20) && m_mmu.type)
	{
		uint16_t reg;
		if(m_mmu.type == 2)
			reg = m_mmu.regs[((offset >> 10) & 0xff) | ((m_mmu.ctl & 0x18) << 5)];
		else
			reg = m_mmu.regs[((offset >> 10) & 0x7f) | ((m_mmu.ctl & 0x1c) << 5)];
		if(!reg && !machine().side_effects_disabled())
		{
			offset <<= 1;
			logerror("%s: Null mmu entry %06x\n", machine().describe_context(), offset);
			return nmi_io_r(space, offset, mem_mask);
		}
		offset = ((reg << 3) & 0x7fc00) | (offset & 0x3ff);
	}
	return ram[offset];
}

//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void pcd_state::pcd_map(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(pcd_state::nmi_io_r), FUNC(pcd_state::nmi_io_w));
	map(0x00000, 0x7ffff).rw(FUNC(pcd_state::mem_r), FUNC(pcd_state::mem_w));
	map(0xfc000, 0xfffff).rom().region("bios", 0);
}

void pcd_state::pcd_io(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xefff).rw(FUNC(pcd_state::nmi_io_r), FUNC(pcd_state::nmi_io_w));
	map(0xf000, 0xf7ff).ram().share("nvram");
	map(0xf800, 0xf801).rw(m_pic1, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf820, 0xf821).rw(m_pic2, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0xf840, 0xf840).rw(FUNC(pcd_state::stat_r), FUNC(pcd_state::stat_w));
	map(0xf841, 0xf841).rw(FUNC(pcd_state::led_r), FUNC(pcd_state::led_w));
	map(0xf880, 0xf8bf).rw(m_rtc, FUNC(mc146818_device::read_direct), FUNC(mc146818_device::write_direct));
	map(0xf900, 0xf903).rw(m_fdc, FUNC(wd2793_device::read), FUNC(wd2793_device::write));
	map(0xf904, 0xf905).rw(FUNC(pcd_state::dskctl_r), FUNC(pcd_state::dskctl_w));
	map(0xf940, 0xf943).rw(FUNC(pcd_state::scsi_r), FUNC(pcd_state::scsi_w));
	map(0xf980, 0xf9bf).m("video", FUNC(pcdx_video_device::map));
	map(0xf9c0, 0xf9c3).rw(m_usart[0], FUNC(mc2661_device::read), FUNC(mc2661_device::write));  // UARTs
	map(0xf9d0, 0xf9d3).rw(m_usart[1], FUNC(mc2661_device::read), FUNC(mc2661_device::write));
	map(0xf9e0, 0xf9e3).rw(m_usart[2], FUNC(mc2661_device::read), FUNC(mc2661_device::write));
//  map(0xfa00, 0xfa7f) // pcs4-n (peripheral chip select)
	map(0xfb00, 0xfb00).rw(FUNC(pcd_state::nmi_io_r), FUNC(pcd_state::nmi_io_w));
	map(0xfb02, 0xffff).rw(FUNC(pcd_state::nmi_io_r), FUNC(pcd_state::nmi_io_w));
}

void pcd_state::pcx_io(address_map &map)
{
	map.unmap_value_high();
	pcd_io(map);
	map(0x8000, 0x8fff).rw(FUNC(pcd_state::mmu_r), FUNC(pcd_state::mmu_w));
	map(0xfb01, 0xfb01).rw(FUNC(pcd_state::nmi_io_r), FUNC(pcd_state::nmi_io_w));
}

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static void pcd_floppies(device_slot_interface &device)
{
	device.option_add("55f", TEAC_FD_55F); // 80 tracks
	device.option_add("55g", TEAC_FD_55G); // 77 tracks
}

FLOPPY_FORMATS_MEMBER( pcd_state::floppy_formats )
	FLOPPY_PC_FORMAT
FLOPPY_FORMATS_END

static INPUT_PORTS_START(pcx)
	PORT_START("mmu")
	PORT_CONFNAME(0x03, 0x00, "MMU Type")
	PORT_CONFSETTING(0x00, "None")
	PORT_CONFSETTING(0x01, "SINIX 1.0")
	PORT_CONFSETTING(0x02, "SINIX 1.2")
INPUT_PORTS_END

void pcd_state::pcd(machine_config &config)
{
	I80186(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &pcd_state::pcd_map);
	m_maincpu->set_addrmap(AS_IO, &pcd_state::pcd_io);
	m_maincpu->tmrout1_handler().set(FUNC(pcd_state::i186_timer1_w));
	m_maincpu->read_slave_ack_callback().set(FUNC(pcd_state::irq_callback));

	TIMER(config, "timer0_tick").configure_periodic(FUNC(pcd_state::timer0_tick), attotime::from_hz(16_MHz_XTAL / 24)); // adjusted to pass post

	PIC8259(config, m_pic1, 0);
	m_pic1->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));

	PIC8259(config, m_pic2, 0);
	m_pic2->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int1_w));

	PCD_VIDEO(config, "video", 0);

	RAM(config, RAM_TAG).set_default_size("1M");

	// nvram
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// floppy disk controller
	WD2793(config, m_fdc, 16_MHz_XTAL / 8);
	m_fdc->intrq_wr_callback().set(m_pic1, FUNC(pic8259_device::ir6_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq1_w));
	m_fdc->enmf_rd_callback().set_constant(0);

	// floppy drives
	FLOPPY_CONNECTOR(config, "fdc:0", pcd_floppies, "55f", pcd_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pcd_floppies, "55f", pcd_state::floppy_formats);

	// usart
	MC2661(config, m_usart[0], 4.9152_MHz_XTAL);
	m_usart[0]->rxrdy_handler().set(m_pic1, FUNC(pic8259_device::ir3_w));
	m_usart[0]->txrdy_handler().set(m_pic1, FUNC(pic8259_device::ir3_w));
	m_usart[0]->txd_handler().set("rs232_1", FUNC(rs232_port_device::write_txd));

	MC2661(config, m_usart[1], 4.9152_MHz_XTAL);
	m_usart[1]->rxrdy_handler().set(m_pic1, FUNC(pic8259_device::ir2_w));
	//m_usart[1]->.txrdy_handler().set(m_pic1, FUNC(pic8259_device::ir2_w)); // this gets stuck high causing the keyboard to not work
	m_usart[1]->txd_handler().set("keyboard", FUNC(pcd_keyboard_device::t0_w));

	MC2661(config, m_usart[2], 4.9152_MHz_XTAL);
	m_usart[2]->rxrdy_handler().set(m_pic1, FUNC(pic8259_device::ir4_w));
	m_usart[2]->txrdy_handler().set(m_pic1, FUNC(pic8259_device::ir4_w));
	m_usart[2]->txd_handler().set("rs232_2", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232_1(RS232_PORT(config, "rs232_1", default_rs232_devices, nullptr));
	rs232_1.rxd_handler().set(m_usart[0], FUNC(mc2661_device::rx_w));
	rs232_port_device &rs232_2(RS232_PORT(config, "rs232_2", default_rs232_devices, nullptr));
	rs232_2.rxd_handler().set(m_usart[2], FUNC(mc2661_device::rx_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	// rtc
	MC146818(config, m_rtc, 32.768_kHz_XTAL);
	m_rtc->irq().set(m_pic1, FUNC(pic8259_device::ir7_w));
	m_rtc->set_binary(true);
	m_rtc->set_binary_year(true);
	m_rtc->set_epoch(1900);
	m_rtc->set_24hrs(true);

	pcd_keyboard_device &keyboard(PCD_KEYBOARD(config, "keyboard", 0));
	keyboard.out_tx_handler().set(m_usart[1], FUNC(mc2661_device::rx_w));

	SCSI_PORT(config, m_scsi, 0);
	m_scsi->set_data_input_buffer("scsi_data_in");
	m_scsi->msg_handler().set(FUNC(pcd_state::write_scsi_msg));
	m_scsi->bsy_handler().set(FUNC(pcd_state::write_scsi_bsy));
	m_scsi->io_handler().set(FUNC(pcd_state::write_scsi_io));
	m_scsi->cd_handler().set(FUNC(pcd_state::write_scsi_cd));
	m_scsi->req_handler().set(FUNC(pcd_state::write_scsi_req));

	output_latch_device &scsi_out(OUTPUT_LATCH(config, "scsi_data_out", 0));
	m_scsi->set_output_latch(scsi_out);

	INPUT_BUFFER(config, "scsi_data_in", 0);
	m_scsi->set_slot_device(1, "harddisk", OMTI5100, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
}

void pcd_state::pcx(machine_config &config)
{
	pcd(config);
	m_maincpu->set_addrmap(AS_IO, &pcd_state::pcx_io);

	pcx_video_device &video(PCX_VIDEO(config.replace(), "video", 0));
	video.txd_handler().set("keyboard", FUNC(pcd_keyboard_device::t0_w));

	subdevice<pcd_keyboard_device>("keyboard")->out_tx_handler().set("video", FUNC(pcx_video_device::rx_w));

	m_usart[1]->txd_handler().set_nop();

	SOFTWARE_LIST(config, "flop_ls").set_original("pcx_flop");
}

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pcd )
	ROM_REGION(0x4000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v2", "V2 GS")  // from mainboard SYBAC S26361-D359 V2 GS
	ROMX_LOAD("s26361-d359.d42", 0x0001, 0x2000, CRC(e20244dd) SHA1(0ebc5ddb93baacd9106f1917380de58aac64fe73), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("s26361-d359.d43", 0x0000, 0x2000, CRC(e03db2ec) SHA1(fcae8b0c9e7543706817b0a53872826633361fda), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v3", "V3 GS4") // from mainboard SYBAC S26361-D359 V3 GS4
	ROMX_LOAD("361d0359.d42", 0x0001, 0x2000, CRC(5b4461e4) SHA1(db6756aeabb2e6d3921dc7571a5bed3497b964bf), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("361d0359.d43", 0x0000, 0x2000, CRC(71c3189d) SHA1(e8dd6c632bfc833074d3a833ea7f59bb5460f313), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

ROM_START( pcx )
	ROM_REGION(0x4000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v2", "V2 GS")  // from mainboard SYBAC S26361-D359 V2 GS
	ROMX_LOAD("s26361-d359.d42", 0x0001, 0x2000, CRC(e20244dd) SHA1(0ebc5ddb93baacd9106f1917380de58aac64fe73), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("s26361-d359.d43", 0x0000, 0x2000, CRC(e03db2ec) SHA1(fcae8b0c9e7543706817b0a53872826633361fda), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v3", "V3 GS4") // from mainboard SYBAC S26361-D359 V3 GS4
	ROMX_LOAD("361d0359.d42", 0x0001, 0x2000, CRC(5b4461e4) SHA1(db6756aeabb2e6d3921dc7571a5bed3497b964bf), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("361d0359.d43", 0x0000, 0x2000, CRC(71c3189d) SHA1(e8dd6c632bfc833074d3a833ea7f59bb5460f313), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1984, pcd, 0,   0, pcd, 0,   pcd_state, empty_init, "Siemens", "PC-D", MACHINE_NOT_WORKING )
COMP( 1984, pcx, pcd, 0, pcx, pcx, pcd_state, empty_init, "Siemens", "PC-X", MACHINE_NOT_WORKING )

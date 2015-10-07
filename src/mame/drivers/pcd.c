// license:GPL-2.0+
// copyright-holders:Dirk Best, Carl
/***************************************************************************

    Siemens PC-D

    For PC-X HDD should have 306,4,9 chs at 1024Bps or 18 at 512Bps

***************************************************************************/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/pic8259.h"
#include "machine/mc2661.h"
#include "machine/wd_fdc.h"
#include "machine/mc146818.h"
#include "machine/pcd_kbd.h"
#include "video/pcd.h"
#include "sound/speaker.h"
#include "formats/pc_dsk.h"
#include "bus/scsi/omti5100.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class pcd_state : public driver_device
{
public:
	pcd_state(const machine_config &mconfig, device_type type, const char *tag) :
	driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_pic1(*this, "pic1"),
	m_pic2(*this, "pic2"),
	m_speaker(*this, "speaker"),
	m_fdc(*this, "fdc"),
	m_rtc(*this, "rtc"),
	m_scsi(*this, "scsi"),
	m_scsi_data_out(*this, "scsi_data_out"),
	m_scsi_data_in(*this, "scsi_data_in"),
	m_ram(*this, "ram"),
	m_req_hack(NULL)
	{ }

	DECLARE_READ8_MEMBER( irq_callback );
	TIMER_DEVICE_CALLBACK_MEMBER( timer0_tick );
	DECLARE_WRITE_LINE_MEMBER( i186_timer1_w );

	DECLARE_READ8_MEMBER( nmi_io_r );
	DECLARE_WRITE8_MEMBER( nmi_io_w );
	DECLARE_READ8_MEMBER( rtc_r );
	DECLARE_WRITE8_MEMBER( rtc_w );
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

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	required_device<i80186_cpu_device> m_maincpu;
	required_device<pic8259_device> m_pic1;
	required_device<pic8259_device> m_pic2;
	required_device<speaker_sound_device> m_speaker;
	required_device<wd2793_t> m_fdc;
	required_device<mc146818_device> m_rtc;
	required_device<SCSI_PORT_DEVICE> m_scsi;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<ram_device> m_ram;
	UINT8 m_stat, m_led;
	int m_msg, m_bsy, m_io, m_cd, m_req, m_rst;
	emu_timer *m_req_hack;
	UINT16 m_dskctl;
	struct {
		UINT16 ctl;
		UINT16 regs[1024];
		int type;
		bool sc;
	} m_mmu;

	void check_scsi_irq();
};


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void pcd_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	// TODO: remove this hack
	if(m_req)
		m_maincpu->drq0_w(1);
}

void pcd_state::machine_start()
{
	m_req_hack = timer_alloc();
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
	if(space.debugger_access())
		return 0;
	logerror("%s: unmapped %s %04x\n", machine().describe_context(), space.name(), offset);
	m_stat |= 0xfd;
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	return 0;
}

WRITE8_MEMBER( pcd_state::nmi_io_w )
{
	if(space.debugger_access())
		return;
	logerror("%s: unmapped %s %04x\n", machine().describe_context(), space.name(), offset);
	m_stat |= 0xfd;
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

READ8_MEMBER( pcd_state::rtc_r )
{
	m_rtc->write(space, 0, offset);
	return m_rtc->read(space, 1);
}

WRITE8_MEMBER( pcd_state::rtc_w )
{
	m_rtc->write(space, 0, offset);
	m_rtc->write(space, 1, data);
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
	UINT16 data = m_mmu.regs[((m_mmu.ctl & 0x1f) << 5) | ((offset >> 2) & 0x1f)];
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
	UINT8 ret = 0;

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
			m_req_hack->adjust(attotime::from_msec(10)); // poke the dmac
		}
		else if(m_msg)
		{
			m_scsi_data_in->read();
			m_scsi->write_ack(1);
		}
	}
	else
	{
		if(m_req_hack) // this might be called before machine_start
			m_req_hack->adjust(attotime::never);
		m_scsi->write_ack(0);
	}
	check_scsi_irq();
}

WRITE16_MEMBER(pcd_state::mem_w)
{
	UINT16 *ram = (UINT16 *)m_ram->pointer();
	if((m_mmu.ctl & 0x20) && m_mmu.type)
	{
		UINT16 reg;
		if(m_mmu.type == 2)
			reg = m_mmu.regs[((offset >> 10) & 0xff) | ((m_mmu.ctl & 0x18) << 5)];
		else
			reg = m_mmu.regs[((offset >> 10) & 0x7f) | ((m_mmu.ctl & 0x1c) << 5)];
		if(!reg && !space.debugger_access())
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
	UINT16 *ram = (UINT16 *)m_ram->pointer();
	if((m_mmu.ctl & 0x20) && m_mmu.type)
	{
		UINT16 reg;
		if(m_mmu.type == 2)
			reg = m_mmu.regs[((offset >> 10) & 0xff) | ((m_mmu.ctl & 0x18) << 5)];
		else
			reg = m_mmu.regs[((offset >> 10) & 0x7f) | ((m_mmu.ctl & 0x1c) << 5)];
		if(!reg && !space.debugger_access())
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

static ADDRESS_MAP_START( pcd_map, AS_PROGRAM, 16, pcd_state )
	AM_RANGE(0x00000, 0x7ffff) AM_READWRITE(mem_r, mem_w)
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("bios", 0)
	AM_RANGE(0x00000, 0xfffff) AM_READWRITE8(nmi_io_r, nmi_io_w, 0xffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcd_io, AS_IO, 16, pcd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xefff) AM_READWRITE8(nmi_io_r, nmi_io_w, 0xffff)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xf800, 0xf801) AM_DEVREADWRITE8("pic1", pic8259_device, read, write, 0xffff)
	AM_RANGE(0xf820, 0xf821) AM_DEVREADWRITE8("pic2", pic8259_device, read, write, 0xffff)
	AM_RANGE(0xf840, 0xf841) AM_READWRITE8(stat_r, stat_w, 0x00ff)
	AM_RANGE(0xf840, 0xf841) AM_READWRITE8(led_r, led_w, 0xff00)
	AM_RANGE(0xf880, 0xf8bf) AM_READWRITE8(rtc_r, rtc_w, 0xffff)
	AM_RANGE(0xf900, 0xf903) AM_DEVREADWRITE8("fdc", wd2793_t, read, write, 0xffff)
	AM_RANGE(0xf904, 0xf905) AM_READWRITE(dskctl_r, dskctl_w)
	AM_RANGE(0xf940, 0xf943) AM_READWRITE8(scsi_r, scsi_w, 0xffff)
	AM_RANGE(0xf980, 0xf9bf) AM_DEVICE("video", pcdx_video_device, map)
	AM_RANGE(0xf9c0, 0xf9c3) AM_DEVREADWRITE8("usart1",mc2661_device,read,write,0xffff)  // UARTs
	AM_RANGE(0xf9d0, 0xf9d3) AM_DEVREADWRITE8("usart2",mc2661_device,read,write,0xffff)
	AM_RANGE(0xf9e0, 0xf9e3) AM_DEVREADWRITE8("usart3",mc2661_device,read,write,0xffff)
//  AM_RANGE(0xfa00, 0xfa7f) // pcs4-n (peripheral chip select)
	AM_RANGE(0xfb00, 0xffff) AM_READWRITE8(nmi_io_r, nmi_io_w, 0xffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcx_io, AS_IO, 16, pcd_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x8000, 0x8fff) AM_READWRITE(mmu_r, mmu_w)
	AM_RANGE(0xfb00, 0xfb01) AM_READWRITE8(nmi_io_r, nmi_io_w, 0xff00)
	AM_IMPORT_FROM(pcd_io)
ADDRESS_MAP_END

//**************************************************************************
//  MACHINE DRIVERS
//**************************************************************************

static SLOT_INTERFACE_START( pcd_floppies )
	SLOT_INTERFACE("55f", TEAC_FD_55F) // 80 tracks
	SLOT_INTERFACE("55g", TEAC_FD_55G) // 77 tracks
SLOT_INTERFACE_END

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

static MACHINE_CONFIG_START( pcd, pcd_state )
	MCFG_CPU_ADD("maincpu", I80186, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(pcd_map)
	MCFG_CPU_IO_MAP(pcd_io)
	MCFG_80186_TMROUT1_HANDLER(WRITELINE(pcd_state, i186_timer1_w))
	MCFG_80186_IRQ_SLAVE_ACK(READ8(pcd_state, irq_callback))

	MCFG_TIMER_DRIVER_ADD_PERIODIC("timer0_tick", pcd_state, timer0_tick, attotime::from_hz(XTAL_16MHz / 24)) // adjusted to pass post

	MCFG_PIC8259_ADD("pic1", DEVWRITELINE("maincpu", i80186_cpu_device, int0_w), VCC, NULL)
	MCFG_PIC8259_ADD("pic2", DEVWRITELINE("maincpu", i80186_cpu_device, int1_w), VCC, NULL)

	MCFG_DEVICE_ADD("video", PCD_VIDEO, 0)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("1M")

	// nvram
	MCFG_NVRAM_ADD_1FILL("nvram")

	// floppy disk controller
	MCFG_WD2793_ADD("fdc", XTAL_16MHz / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(DEVWRITELINE("pic1", pic8259_device, ir6_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("maincpu", i80186_cpu_device, drq1_w))
	MCFG_WD_FDC_ENMF_CALLBACK(GND)

	// floppy drives
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", pcd_floppies, "55f", pcd_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", pcd_floppies, "55f", pcd_state::floppy_formats)

	// usart
	MCFG_DEVICE_ADD("usart1", MC2661, XTAL_4_9152MHz)
	MCFG_MC2661_RXRDY_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir3_w))
	MCFG_MC2661_TXRDY_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir3_w))
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("rs232_1", rs232_port_device, write_txd))
	MCFG_DEVICE_ADD("usart2", MC2661, XTAL_4_9152MHz)
	MCFG_MC2661_RXRDY_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir2_w))
	//MCFG_MC2661_TXRDY_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir2_w)) // this gets stuck high causing the keyboard to not work
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("keyboard", pcd_keyboard_device, t0_w))
	MCFG_DEVICE_ADD("usart3", MC2661, XTAL_4_9152MHz)
	MCFG_MC2661_RXRDY_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir4_w))
	MCFG_MC2661_TXRDY_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir4_w))
	MCFG_MC2661_TXD_HANDLER(DEVWRITELINE("rs232_2", rs232_port_device, write_txd))

	MCFG_RS232_PORT_ADD("rs232_1", default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("usart1", mc2661_device, rx_w))
	MCFG_RS232_PORT_ADD("rs232_2", default_rs232_devices, NULL)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("usart3", mc2661_device, rx_w))

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	// rtc
	MCFG_MC146818_ADD("rtc", XTAL_32_768kHz)
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic1", pic8259_device, ir7_w))
	MCFG_MC146818_BINARY(true)
	MCFG_MC146818_EPOCH(1900)
	MCFG_MC146818_24_12(true)

	MCFG_DEVICE_ADD("keyboard", PCD_KEYBOARD, 0)
	MCFG_PCD_KEYBOARD_OUT_TX_HANDLER(DEVWRITELINE("usart2", mc2661_device, rx_w))

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSI_DATA_INPUT_BUFFER("scsi_data_in")
	MCFG_SCSI_MSG_HANDLER(WRITELINE(pcd_state, write_scsi_msg))
	MCFG_SCSI_BSY_HANDLER(WRITELINE(pcd_state, write_scsi_bsy))
	MCFG_SCSI_IO_HANDLER(WRITELINE(pcd_state, write_scsi_io))
	MCFG_SCSI_CD_HANDLER(WRITELINE(pcd_state, write_scsi_cd))
	MCFG_SCSI_REQ_HANDLER(WRITELINE(pcd_state, write_scsi_req))

	MCFG_SCSI_OUTPUT_LATCH_ADD("scsi_data_out", "scsi")
	MCFG_DEVICE_ADD("scsi_data_in", INPUT_BUFFER, 0)
	MCFG_SCSIDEV_ADD("scsi:1", "harddisk", OMTI5100, SCSI_ID_0)
MACHINE_CONFIG_END

MACHINE_CONFIG_DERIVED(pcx, pcd)
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(pcx_io)

	MCFG_DEVICE_REPLACE("video", PCX_VIDEO, 0)
	MCFG_PCX_VIDEO_TXD_HANDLER(DEVWRITELINE("keyboard", pcd_keyboard_device, t0_w))

	MCFG_DEVICE_MODIFY("keyboard")
	MCFG_PCD_KEYBOARD_OUT_TX_HANDLER(DEVWRITELINE("video", pcx_video_device, rx_w))

	MCFG_DEVICE_MODIFY("usart2")
	MCFG_MC2661_TXD_HANDLER(NULL)
MACHINE_CONFIG_END

//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( pcd )
	ROM_REGION(0x4000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v2", "V2 GS")  // from mainboard SYBAC S26361-D359 V2 GS
	ROMX_LOAD("s26361-d359.d42", 0x0001, 0x2000, CRC(e20244dd) SHA1(0ebc5ddb93baacd9106f1917380de58aac64fe73), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("s26361-d359.d43", 0x0000, 0x2000, CRC(e03db2ec) SHA1(fcae8b0c9e7543706817b0a53872826633361fda), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v3", "V3 GS4") // from mainboard SYBAC S26361-D359 V3 GS4
	ROMX_LOAD("361d0359.d42", 0x0001, 0x2000, CRC(5b4461e4) SHA1(db6756aeabb2e6d3921dc7571a5bed3497b964bf), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("361d0359.d43", 0x0000, 0x2000, CRC(71c3189d) SHA1(e8dd6c632bfc833074d3a833ea7f59bb5460f313), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

ROM_START( pcx )
	ROM_REGION(0x4000, "bios", 0)
	ROM_SYSTEM_BIOS(0, "v2", "V2 GS")  // from mainboard SYBAC S26361-D359 V2 GS
	ROMX_LOAD("s26361-d359.d42", 0x0001, 0x2000, CRC(e20244dd) SHA1(0ebc5ddb93baacd9106f1917380de58aac64fe73), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("s26361-d359.d43", 0x0000, 0x2000, CRC(e03db2ec) SHA1(fcae8b0c9e7543706817b0a53872826633361fda), ROM_SKIP(1) | ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v3", "V3 GS4") // from mainboard SYBAC S26361-D359 V3 GS4
	ROMX_LOAD("361d0359.d42", 0x0001, 0x2000, CRC(5b4461e4) SHA1(db6756aeabb2e6d3921dc7571a5bed3497b964bf), ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD("361d0359.d43", 0x0000, 0x2000, CRC(71c3189d) SHA1(e8dd6c632bfc833074d3a833ea7f59bb5460f313), ROM_SKIP(1) | ROM_BIOS(2))
ROM_END

//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

COMP( 1984, pcd, 0, 0, pcd, 0, driver_device, 0, "Siemens", "PC-D", MACHINE_NOT_WORKING )
COMP( 1984, pcx, pcd, 0, pcx, pcx, driver_device, 0, "Siemens", "PC-X", MACHINE_NOT_WORKING )

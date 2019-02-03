// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Robbbert
/*
    Gimix 6809-Based Computers

    Representative group of GIMIX users included:  Government Research and Scientific Organizations, Universities, Industrial, Computer Mainframe and
    Peripheral Manufacturers and Software Houses.

    This system is most notable for being the development base of the "Vid Kidz", a pair of programmers (Eugene Jarvis and Larry DeMar) who formerly
    worked for Williams Electronics on the game, Defender.  They left Willams and continued work on other games eventually making a deal with Williams
    to continue to design games producing the titles: Stargate, Robotron: 2084 and Blaster.

    Information Link:  http://www.backglass.org/duncan/gimix/

    TODO:  Everything

    Usage:
    System boots into GMXBUG-09
    To boot Flex, insert the Flex system disk (3.3 or later, must support the DMA disk controller), type U and press enter.
    To boot OS-9, insert the OS-9 system disk, type O, and press Enter.
    Note that booting OS-9 doesn't currently work without a timer hack.
*/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "imagedev/floppy.h"
#include "machine/mm58167.h"
#include "machine/6840ptm.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/wd_fdc.h"
#include "machine/bankdev.h"
#include "machine/ram.h"
#include "machine/timer.h"
#include "formats/flex_dsk.h"
#include "softlist.h"

#define DMA_DRQ         (m_dma_status & 0x80)
#define DMA_INTRQ       (m_dma_status & 0x40)
#define DMA_MOTOR_DELAY (m_dma_status & 0x20)
#define DMA_ENABLED     (m_dma_status & 0x10)
#define DMA_FAULT       (m_dma_status & 0x08)
#define DMA_DRIVE_SIZE  (m_dma_status & 0x04)
#define DMA_DIP_SENSE   (m_dma_status & 0x01)

#define DMA_CONNECT_SEL (m_dma_drive_select & 0x40)
#define DMA_DENSITY     (m_dma_drive_select & 0x20)
#define DMA_WR_PROTECT  (m_dma_drive_select & 0x10)
#define DMA_SEL_DRV3    (m_dma_drive_select & 0x08)
#define DMA_SEL_DRV2    (m_dma_drive_select & 0x04)
#define DMA_SEL_DRV1    (m_dma_drive_select & 0x02)
#define DMA_SEL_DRV0    (m_dma_drive_select & 0x01)

#define DMA_IRQ_ENABLE  (m_dma_ctrl & 0x80)
#define DMA_SIDE_SEL    (m_dma_ctrl & 0x40)
#define DMA_DIRECTION   (m_dma_ctrl & 0x20)
#define DMA_ENABLE      (m_dma_ctrl & 0x10)
#define DMA_BANK        (m_dma_ctrl & 0x0f)

#define DMA_START_ADDR  (((m_dma_ctrl & 0x0f) << 16) | m_dma_start_addr)

class gimix_state : public driver_device
{
public:
	gimix_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy0(*this, "fdc:0")
		, m_floppy1(*this, "fdc:1")
		, m_ram(*this, RAM_TAG)
		, m_rom(*this, "roms")
		, m_acia1(*this, "acia1")
		, m_acia2(*this, "acia2")
		, m_acia3(*this, "acia3")
		, m_acia4(*this, "acia4")
		, m_bank(*this, "bank%u", 1U)
		, m_rombank1(*this, "rombank1")
		, m_rombank2(*this, "rombank2")
		, m_fixedrombank(*this, "fixedrombank")
		, m_dma_dip(*this, "dma_s2")
	{}

	void gimix(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(drive_size_cb);

private:
	DECLARE_WRITE8_MEMBER(system_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_READ8_MEMBER(dma_r);
	DECLARE_WRITE8_MEMBER(dma_w);
	DECLARE_READ8_MEMBER(fdc_r);
	DECLARE_WRITE8_MEMBER(fdc_w);
	DECLARE_READ8_MEMBER(pia_pa_r);
	DECLARE_WRITE8_MEMBER(pia_pa_w);
	DECLARE_READ8_MEMBER(pia_pb_r);
	DECLARE_WRITE8_MEMBER(pia_pb_w);
	TIMER_DEVICE_CALLBACK_MEMBER(test_timer_w);

	DECLARE_FLOPPY_FORMATS(floppy_formats);

	void gimix_banked_mem(address_map &map);
	void gimix_mem(address_map &map);

	uint8_t m_term_data;
	uint8_t m_dma_status;
	uint8_t m_dma_ctrl;
	uint8_t m_dma_drive_select;
	uint16_t m_dma_start_addr;
	uint32_t m_dma_current_addr;
	uint8_t m_task;
	uint8_t m_task_banks[16][16];
	uint8_t m_selected_drive;
	bool m_floppy0_ready;
	bool m_floppy1_ready;

	uint8_t m_pia1_pa;
	uint8_t m_pia1_pb;

	virtual void machine_reset() override;
	virtual void machine_start() override;
	virtual void driver_start() override;

	void refresh_memory();

	required_device<cpu_device> m_maincpu;
	required_device<fd1797_device> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;
	required_device<acia6850_device> m_acia1;
	required_device<acia6850_device> m_acia2;
	required_device<acia6850_device> m_acia3;
	required_device<acia6850_device> m_acia4;

	required_device_array<address_map_bank_device, 16> m_bank;
	required_memory_bank m_rombank1;
	required_memory_bank m_rombank2;
	required_memory_bank m_fixedrombank;

	required_ioport m_dma_dip;
};

void gimix_state::gimix_banked_mem(address_map &map)
{
	map(0x00000, 0x0dfff).bankrw("lower_ram");
	map(0x0e000, 0x0e001).rw(m_acia1, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x0e004, 0x0e005).rw(m_acia2, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	//AM_RANGE(0x0e018, 0x0e01b) AM_READWRITE(fdc_r, fdc_w)  // FD1797 FDC (PIO)
	map(0x0e100, 0x0e1ff).ram();
	//AM_RANGE(0x0e200, 0x0e20f) // 9511A / 9512 Arithmetic Processor
	map(0x0e210, 0x0e21f).rw("timer", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x0e220, 0x0e23f).rw("rtc", FUNC(mm58167_device::read), FUNC(mm58167_device::write));
	map(0x0e240, 0x0e3af).ram();
	map(0x0e3b0, 0x0e3b3).rw(FUNC(gimix_state::dma_r), FUNC(gimix_state::dma_w));  // DMA controller (custom?)
	map(0x0e3b4, 0x0e3b7).rw(FUNC(gimix_state::fdc_r), FUNC(gimix_state::fdc_w));  // FD1797 FDC
	map(0x0e400, 0x0e7ff).ram();  // scratchpad RAM
	map(0x0e800, 0x0efff).ram();
	map(0x0f000, 0x0f7ff).bankr("rombank2");
	map(0x0f800, 0x0ffff).bankr("rombank1");
	//AM_RANGE(0x10000, 0x1ffff) AM_RAM
}

void gimix_state::gimix_mem(address_map &map)
{
	for (int bank = 0; bank < 16; bank++)
	{
		map(bank << 12, (bank << 12) | 0x0fff).rw(m_bank[bank], FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
	}
	map(0xff00, 0xffff).bankr("fixedrombank").w(FUNC(gimix_state::system_w));
}

static INPUT_PORTS_START( gimix )
	PORT_START("dma_s2")
	PORT_DIPNAME(0x00000100,0x00000000,"5.25\" / 8\" floppy drive 0") PORT_DIPLOCATION("S2:9") PORT_CHANGED_MEMBER(DEVICE_SELF,gimix_state,drive_size_cb,nullptr)
	PORT_DIPSETTING(0x00000000,"5.25\"")
	PORT_DIPSETTING(0x00000100,"8\"")

INPUT_PORTS_END

void gimix_state::refresh_memory()
{
	for (int bank = 0; bank < 16; bank++)
	{
		m_bank[bank]->set_bank(m_task_banks[m_task][bank]);
	}
}

WRITE8_MEMBER( gimix_state::system_w )
{
	if(offset == 0x7f)  // task register
	{
		if(data & 0x20)  // FPLA software latch
		{
			m_rombank1->set_entry(2);
			m_rombank2->set_entry(3);
			m_fixedrombank->set_entry(2);
			logerror("SYS: FPLA software latch set\n");
		}
		else
		{
			m_rombank1->set_entry(0);
			m_rombank2->set_entry(1);
			m_fixedrombank->set_entry(0);
			logerror("SYS: FPLA software latch reset\n");
		}
		m_task = data & 0x0f;
		refresh_memory();
		logerror("SYS: Task set to %02x\n",data & 0x0f);
	}
	if(offset >= 0xf0)  // Dynamic Address Translation RAM (write only)
	{
		m_bank[offset-0xf0]->set_bank(data & 0x0f);
		m_task_banks[m_task][offset-0xf0] = data & 0x0f;
		logerror("SYS: Bank %i set to physical bank %02x\n",offset-0xf0,data);
	}
}

READ8_MEMBER(gimix_state::dma_r)
{
	switch(offset)
	{
	case 0:
		if(m_dma_dip->read() & 0x00000100)
			m_dma_status |= 0x01;   // 8"
		else
			m_dma_status &= ~0x01;  // 5.25"
		return m_dma_status;
	case 1:
		return m_dma_ctrl;
	case 2:
		return (m_dma_start_addr & 0xff00) >> 8;
	case 3:
		return (m_dma_start_addr & 0x00ff);
	default:
		logerror("DMA: Unknown or invalid DMA register %02x read\n",offset);
	}
	return 0xff;
}

WRITE8_MEMBER(gimix_state::dma_w)
{
	switch(offset)
	{
	case 0:
		logerror("DMA: Drive select %02x\n",data);
		m_dma_drive_select = data;
		m_fdc->dden_w(DMA_DENSITY ? 1 : 0);
		if(data & 0x40)  // 8" / 5.25" connector select
			m_dma_status |= 0x04;
		else
			m_dma_status &= ~0x04;
		if(data & 0x01)
		{
			m_fdc->set_floppy(m_floppy0->get_device());
			m_selected_drive = 1;
			m_floppy1->get_device()->mon_w(1);  // switch off the motor of other drives...
			m_floppy1_ready = false;
			logerror("FDC: Floppy drive 1 motor off\n");
		}
		if(data & 0x02)
		{
			m_fdc->set_floppy(m_floppy1->get_device());
			m_selected_drive = 2;
			m_floppy0->get_device()->mon_w(1);  // switch off the motor of other drives...
			m_floppy0_ready = false;
			logerror("FDC: Floppy drive 0 motor off\n");
		}
		break;
	case 1:
		logerror("DMA: DMA control %02x\n",data);
		m_dma_ctrl = data;
		if(data & 0x10)
			m_dma_status |= 0x12;
		else
			m_dma_status &= ~0x12;
		if(data & 0x40)
		{
			if(m_selected_drive == 1)
				m_floppy0->get_device()->ss_w(1);
			if(m_selected_drive == 2)
				m_floppy1->get_device()->ss_w(1);
		}
		else
		{
			if(m_selected_drive == 1)
				m_floppy0->get_device()->ss_w(0);
			if(m_selected_drive == 2)
				m_floppy1->get_device()->ss_w(0);
		}
		break;
	case 2:
		logerror("DMA: DMA start address MSB %02x\n",data);
		m_dma_start_addr = (m_dma_start_addr & 0x00ff) | (data << 8);
		m_dma_current_addr = DMA_START_ADDR;
		break;
	case 3:
		logerror("DMA: DMA start address LSB %02x\n",data);
		m_dma_start_addr = (m_dma_start_addr & 0xff00) | data;
		m_dma_current_addr = DMA_START_ADDR;
		break;
	default:
		logerror("DMA: Unknown or invalid DMA register %02x write %02x\n",offset,data);
	}
}

READ8_MEMBER(gimix_state::fdc_r)
{
	// motors are switched on on FDC access
	if(m_selected_drive == 1 && m_floppy0_ready == false)
	{
		m_floppy0->get_device()->mon_w(0);
		m_floppy0_ready = true;
		logerror("FDC: Floppy drive 0 motor on\n");
	}
	if(m_selected_drive == 2 && m_floppy1_ready == false)
	{
		m_floppy1->get_device()->mon_w(0);
		m_floppy1_ready = true;
		logerror("FDC: Floppy drive 1 motor on\n");
	}
	return m_fdc->read(offset);
}

WRITE8_MEMBER(gimix_state::fdc_w)
{
	// motors are switched on on FDC access
	if(m_selected_drive == 1)
		m_floppy0->get_device()->mon_w(0);
	if(m_selected_drive == 2)
		m_floppy1->get_device()->mon_w(0);
	m_fdc->write(offset,data);
}

READ8_MEMBER(gimix_state::pia_pa_r)
{
	return m_pia1_pa;
}

WRITE8_MEMBER(gimix_state::pia_pa_w)
{
	m_pia1_pa = data;
	logerror("PIA: Port A write %02x\n",data);
}

READ8_MEMBER(gimix_state::pia_pb_r)
{
	return m_pia1_pb;
}

WRITE8_MEMBER(gimix_state::pia_pb_w)
{
	m_pia1_pb = data;
	logerror("PIA: Port B write %02x\n",data);
}


WRITE_LINE_MEMBER(gimix_state::irq_w)
{
	m_maincpu->set_input_line(M6809_IRQ_LINE,state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(gimix_state::fdc_irq_w)
{
	if(state)
		m_dma_status |= 0x40;
	else
		m_dma_status &= ~0x40;
}

WRITE_LINE_MEMBER(gimix_state::fdc_drq_w)
{
	if(state && DMA_ENABLED)
	{
		m_dma_status |= 0x80;
		// do a DMA transfer
		if(DMA_DIRECTION)
		{
			// write to disk
			m_fdc->data_w(m_ram->read(m_dma_current_addr));
//          logerror("DMA: read from RAM %05x\n",m_dma_current_addr);
		}
		else
		{
			// read from disk
			m_ram->write(m_dma_current_addr,m_fdc->data_r());
//          logerror("DMA: write to RAM %05x\n",m_dma_current_addr);
		}
		m_dma_current_addr++;
	}
	else
		m_dma_status &= ~0x80;
}

INPUT_CHANGED_MEMBER(gimix_state::drive_size_cb)
{
	// set FDC clock based on DIP Switch S2-9 (5.25"/8" drive select)
	if(m_dma_dip->read() & 0x00000100)
		m_fdc->set_unscaled_clock(8_MHz_XTAL / 4); // 8 inch (2MHz)
	else
		m_fdc->set_unscaled_clock(8_MHz_XTAL / 8); // 5.25 inch (1MHz)
}

void gimix_state::machine_reset()
{
	m_term_data = 0;
	m_rombank1->set_entry(0);  // RAM banks are undefined on startup
	m_rombank2->set_entry(1);
	m_fixedrombank->set_entry(0);
	m_dma_status = 0x00;
	m_dma_ctrl = 0x00;
	m_task = 0x00;
	m_selected_drive = 0;
	m_floppy0_ready = false;
	m_floppy1_ready = false;
	membank("lower_ram")->set_base(m_ram->pointer());
	if(m_ram->size() > 65536)
		membank("upper_ram")->set_base(m_ram->pointer()+0x10000);

	// initialise FDC clock based on DIP Switch S2-9 (5.25"/8" drive select)
	if(m_dma_dip->read() & 0x00000100)
		m_fdc->set_unscaled_clock(8_MHz_XTAL / 4); // 8 inch (2MHz)
	else
		m_fdc->set_unscaled_clock(8_MHz_XTAL / 8); // 5.25 inch (1MHz)
}

void gimix_state::machine_start()
{
	uint8_t* ROM = m_rom->base();
	m_rombank1->configure_entries(0,4,ROM,0x800);
	m_rombank2->configure_entries(0,4,ROM,0x800);
	m_fixedrombank->configure_entries(0,4,ROM+0x700,0x800);
	m_rombank1->set_entry(0);  // RAM banks are undefined on startup
	m_rombank2->set_entry(1);
	m_fixedrombank->set_entry(0);
	// install any extra RAM
	if(m_ram->size() > 65536)
	{
		for (int bank = 0; bank < 16; bank++)
		{
			m_bank[bank]->space(AS_PROGRAM).install_readwrite_bank(0x10000,m_ram->size()-1,"upper_ram");
		}
	}
	m_floppy0->get_device()->set_rpm(300);
	m_floppy1->get_device()->set_rpm(300);
}

void gimix_state::driver_start()
{
}


TIMER_DEVICE_CALLBACK_MEMBER(gimix_state::test_timer_w)
{
	static bool prev;
	if(!prev)
	{
		m_maincpu->set_input_line(M6809_IRQ_LINE,ASSERT_LINE);
		prev = true;
	}
	else
	{
		m_maincpu->set_input_line(M6809_IRQ_LINE,CLEAR_LINE);
		prev = false;
	}
}

FLOPPY_FORMATS_MEMBER( gimix_state::floppy_formats )
	FLOPPY_FLEX_FORMAT
FLOPPY_FORMATS_END

static void gimix_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("8dd", FLOPPY_8_DSDD);
}

void gimix_state::gimix(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &gimix_state::gimix_mem);

	/* rtc */
	mm58167_device &rtc(MM58167(config, "rtc", 32.768_kHz_XTAL));
	rtc.irq().set(FUNC(gimix_state::irq_w));

	/* timer */
	ptm6840_device &ptm(PTM6840(config, "timer", 2'000'000));  // clock is a guess
	ptm.irq_callback().set(FUNC(gimix_state::irq_w));  // PCB pictures show both the RTC and timer set to generate IRQs (are jumper configurable)

	/* floppy disks */
	FD1797(config, m_fdc, 8_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(gimix_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(gimix_state::fdc_drq_w));
	m_fdc->set_force_ready(true);
	FLOPPY_CONNECTOR(config, "fdc:0", gimix_floppies, "525hd", gimix_state::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", gimix_floppies, "525hd", gimix_state::floppy_formats);

	/* parallel ports */
	pia6821_device &pia1(PIA6821(config, "pia1", 2'000'000));
	pia1.writepa_handler().set(FUNC(gimix_state::pia_pa_w));
	pia1.writepb_handler().set(FUNC(gimix_state::pia_pb_w));
	pia1.readpa_handler().set(FUNC(gimix_state::pia_pa_r));
	pia1.readpb_handler().set(FUNC(gimix_state::pia_pb_r));

	PIA6821(config, "pia2", 2'000'000);

	/* serial ports */
	ACIA6850(config, m_acia1, 2'000'000);
	m_acia1->txd_handler().set("serial1", FUNC(rs232_port_device::write_txd));
	m_acia1->rts_handler().set("serial1", FUNC(rs232_port_device::write_rts));

	ACIA6850(config, m_acia2, 2'000'000);
	m_acia2->txd_handler().set("serial2", FUNC(rs232_port_device::write_txd));
	m_acia2->rts_handler().set("serial2", FUNC(rs232_port_device::write_rts));

	ACIA6850(config, m_acia3, 2'000'000);
	m_acia3->txd_handler().set("serial3", FUNC(rs232_port_device::write_txd));
	m_acia3->rts_handler().set("serial3", FUNC(rs232_port_device::write_rts));

	ACIA6850(config, m_acia4, 2'000'000);
	m_acia4->txd_handler().set("serial4", FUNC(rs232_port_device::write_txd));
	m_acia4->rts_handler().set("serial4", FUNC(rs232_port_device::write_rts));

	rs232_port_device &serial1(RS232_PORT(config, "serial1", default_rs232_devices, nullptr));
	serial1.rxd_handler().set(m_acia1, FUNC(acia6850_device::write_rxd));
	serial1.cts_handler().set(m_acia1, FUNC(acia6850_device::write_cts));

	rs232_port_device &serial2(RS232_PORT(config, "serial2", default_rs232_devices, "terminal"));
	serial2.rxd_handler().set(m_acia2, FUNC(acia6850_device::write_rxd));
	serial2.cts_handler().set(m_acia2, FUNC(acia6850_device::write_cts));

	rs232_port_device &serial3(RS232_PORT(config, "serial3", default_rs232_devices, nullptr));
	serial3.rxd_handler().set(m_acia3, FUNC(acia6850_device::write_rxd));
	serial3.cts_handler().set(m_acia3, FUNC(acia6850_device::write_cts));

	rs232_port_device &serial4(RS232_PORT(config, "serial4", default_rs232_devices, nullptr));
	serial4.rxd_handler().set(m_acia4, FUNC(acia6850_device::write_rxd));
	serial4.cts_handler().set(m_acia4, FUNC(acia6850_device::write_cts));

	clock_device &acia_clock(CLOCK(config, "acia_clock", 153600));
	acia_clock.signal_handler().set(m_acia1, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia1, FUNC(acia6850_device::write_rxc));
	acia_clock.signal_handler().append(m_acia2, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia2, FUNC(acia6850_device::write_rxc));

	/* banking */
	for (int bank = 0; bank < 16; bank++)
	{
		ADDRESS_MAP_BANK(config, m_bank[bank]).set_map(&gimix_state::gimix_banked_mem).set_options(ENDIANNESS_LITTLE, 8, 32, 0x1000);
	}

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("128K").set_extra_options("56K,256K,512K");

	SOFTWARE_LIST(config, "flop_list").set_original("gimix");

	// uncomment this timer to use a hack that generates a regular IRQ, this will get OS-9 to boot
	// for some unknown reason, OS-9 does not touch the 6840, and only clears/disables IRQs on the RTC
	//TIMER(config, "test_timer").configure_periodic(FUNC(gimix_state::test_timer_w), attotime::from_msec(100));
}

ROM_START( gimix )
	ROM_REGION( 0x10000, "roms", 0)
/* CPU board U4: gimixf8.bin  - checksum 68DB - 2716 - GMXBUG09 V2.1 | (c)1981 GIMIX | $F800 I2716 */
		ROM_LOAD( "gimixf8.u4",  0x000000, 0x000800, CRC(7d60f838) SHA1(eb7546e8bbf50d33e181f3e86c3e4c5c9032cab2) )
/* CPU board U5: gimixv14.bin - checksum 97E2 - 2716 - GIMIX 6809 | AUTOBOOT | V1.4 I2716 */
		ROM_LOAD( "gimixv14.u5", 0x000800, 0x000800, CRC(f795b8b9) SHA1(eda2de51cc298d94b36605437d900ce971b3b276) )
/* CPU board U6: os9l1v11.bin - checksum 2C84 - 2716 - OS-9tmL1 V1 | GIMIX P1 " (c)1982 MSC
   CPU board U7: os9l1v12.bin - checksum 7694 - 2716 - OS-9tmL1 V1 | GIMIX P2-68 | (c)1982 MSC */
		ROM_LOAD( "os9l1v11.u6", 0x001000, 0x000800, CRC(0d6527a0) SHA1(1435a22581c6e9e0ae338071a72eed646f429530) )
		ROM_LOAD( "os9l1v12.u7", 0x001800, 0x000800, CRC(b3c65feb) SHA1(19d1ea1e84473b25c95cbb8449e6b9828567e998) )

/* Hard drive controller board 2 (XEBEC board) 11H: gimixhd.bin - checksum 2436 - 2732 - 104521D */
	ROM_REGION( 0x10000, "xebec", 0)
		ROM_LOAD( "gimixhd.h11",  0x000000, 0x001000, CRC(35c12201) SHA1(51ac9052f9757d79c7f5bd3aa5d8421e98cfcc37) )
ROM_END

COMP( 1980, gimix, 0, 0, gimix, gimix, gimix_state, empty_init, "Gimix", "Gimix 6809 System", MACHINE_IS_SKELETON )

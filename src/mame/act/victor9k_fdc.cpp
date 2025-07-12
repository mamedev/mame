// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Victor 9000 floppy disk controller emulation

**********************************************************************/

/*

    value   error description

    01      no sync pulse detected
    02      no header track
    03      checksum error in header
    04      not right track
    05      not right sector
    06      not a data block
    07      data checksum error
    08      sync too long
    99      not a system disc

    11      Noise on sync
    FF      No sync (bad or unformatted disk)

*/

/*
    TODO: write protect
*/

#include "emu.h"
#include "victor9k_fdc.h"

//**************************************************************************
//  LOGGING
//**************************************************************************

#define LOG_VIA     (1U << 1)
#define LOG_SCP     (1U << 2)
#define LOG_DISK    (1U << 3)
#define LOG_BITS    (1U << 4)

//#define VERBOSE (LOG_VIA | LOG_SCP | LOG_DISK | LOG_BITS)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGVIA(...)      LOGMASKED(LOG_VIA,  __VA_ARGS__)
#define LOGDISK(...)     LOGMASKED(LOG_DISK,  __VA_ARGS__)
#define LOGBITS(...)     LOGMASKED(LOG_BITS,  __VA_ARGS__)
#define LOGSCP(...)      LOGMASKED(LOG_SCP,  __VA_ARGS__)


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define I8048_TAG       "5d"
#define M6522_4_TAG     "1f"
#define M6522_5_TAG     "1k"
#define M6522_6_TAG     "1h"

// this is exactly the same decode/encode as used in the Commodore 4040/8050 series drives
#define GCR_DECODE(_e, _i) \
	((BIT(_e, 6) << 7) | (BIT(_i, 7) << 6) | (_e & 0x33) | (BIT(_e, 2) << 3) | (_i & 0x04))

// E7 E6 I7 E5 E4 E3 E2 I2 E1 E0
#define GCR_ENCODE(_e, _i) \
	((_e & 0xc0) << 2 | (_i & 0x80) | (_e & 0x3c) << 1 | (_i & 0x04) | (_e & 0x03))

// Tandon TM-100 spindle @ 300RPM, measured TACH 12VAC 256Hz
// TACH = RPM / 60 * SPINDLE RATIO * MOTOR POLES
// 256 = 300 / 60 * 6.4 * 8
#define SPINDLE_RATIO   6.4     // Motor RPM divided by Disk RPM
#define MOTOR_POLES     8.0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VICTOR_9000_FDC, victor_9000_fdc_device, "victor9k_fdc", "Victor 9000 FDC")


//-------------------------------------------------
//  ROM( victor_9000_fdc )
//-------------------------------------------------

ROM_START( victor_9000_fdc )
	ROM_REGION( 0x400, I8048_TAG, 0)
	ROM_LOAD( "36080.5d", 0x000, 0x400, CRC(9bf49f7d) SHA1(b3a11bb65105db66ae1985b6f482aab6ea1da38b) )

	ROM_REGION( 0x800, "gcr", 0 )
	ROM_LOAD( "100836-001.4k", 0x000, 0x800, CRC(adc601bd) SHA1(6eeff3d2063ae2d97452101aa61e27ef83a467e5) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *victor_9000_fdc_device::device_rom_region() const
{
	return ROM_NAME( victor_9000_fdc );
}


void victor_9000_fdc_device::load0_cb(floppy_image_device *device)
{
	// DOOR OPEN 0
	m_via4->write_ca1(0);
}

void victor_9000_fdc_device::unload0_cb(floppy_image_device *device)
{
	// DOOR OPEN 0
	m_via4->write_ca1(1);
}

void victor_9000_fdc_device::load1_cb(floppy_image_device *device)
{
	// DOOR OPEN 1
	m_via4->write_cb1(0);
}

void victor_9000_fdc_device::unload1_cb(floppy_image_device *device)
{
	// DOOR OPEN 1
	m_via4->write_cb1(1);
}

void victor_9000_fdc_device::floppy_formats(format_registration &fr)
{
	fr.add(FLOPPY_VICTOR_9000_FORMAT);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void victor_9000_fdc_device::device_add_mconfig(machine_config &config)
{
	I8048(config, m_maincpu, XTAL(30'000'000)/6);
	m_maincpu->p1_in_cb().set(FUNC(victor_9000_fdc_device::floppy_p1_r));
	m_maincpu->p1_out_cb().set(FUNC(victor_9000_fdc_device::floppy_p1_w));
	m_maincpu->p2_in_cb().set(FUNC(victor_9000_fdc_device::floppy_p2_r));
	m_maincpu->p2_out_cb().set(FUNC(victor_9000_fdc_device::floppy_p2_w));
	m_maincpu->t0_in_cb().set(FUNC(victor_9000_fdc_device::tach0_r));
	m_maincpu->t1_in_cb().set(FUNC(victor_9000_fdc_device::tach1_r));
	m_maincpu->bus_out_cb().set(FUNC(victor_9000_fdc_device::da_w));

	MOS6522(config, m_via4, XTAL(30'000'000)/30);
	m_via4->readpa_handler().set(FUNC(victor_9000_fdc_device::via4_pa_r));
	m_via4->writepa_handler().set(FUNC(victor_9000_fdc_device::via4_pa_w));
	m_via4->readpb_handler().set(FUNC(victor_9000_fdc_device::via4_pb_r));
	m_via4->writepb_handler().set(FUNC(victor_9000_fdc_device::via4_pb_w));
	m_via4->ca2_handler().set(FUNC(victor_9000_fdc_device::wrsync_w));
	m_via4->irq_handler().set(FUNC(victor_9000_fdc_device::via4_irq_w));

	MOS6522(config, m_via5, XTAL(30'000'000)/30);
	m_via5->irq_handler().set(FUNC(victor_9000_fdc_device::via5_irq_w));
	m_via5->readpa_handler().set(FUNC(victor_9000_fdc_device::via5_pa_r));
	m_via5->writepb_handler().set(FUNC(victor_9000_fdc_device::via5_pb_w));

	MOS6522(config, m_via6, XTAL(30'000'000)/30);
	m_via6->readpa_handler().set(FUNC(victor_9000_fdc_device::via6_pa_r));
	m_via6->readpb_handler().set(FUNC(victor_9000_fdc_device::via6_pb_r));
	m_via6->writepa_handler().set(FUNC(victor_9000_fdc_device::via6_pa_w));
	m_via6->writepb_handler().set(FUNC(victor_9000_fdc_device::via6_pb_w));
	m_via6->ca2_handler().set(FUNC(victor_9000_fdc_device::drw_w));
	m_via6->cb2_handler().set(FUNC(victor_9000_fdc_device::erase_w));
	m_via6->irq_handler().set(FUNC(victor_9000_fdc_device::via6_irq_w));

	for (auto &connector : m_floppy)
	{
		FLOPPY_CONNECTOR(config, connector);
		connector->option_add("525ssqd", FLOPPY_525_SSQD); // Tandon TM100-3 with custom electronics
		connector->option_add("525qd", FLOPPY_525_QD); // Tandon TM100-4 with custom electronics
		connector->set_default_option("525qd");
		connector->set_formats(floppy_formats);
		connector->enable_sound(true);
	}
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  victor_9000_fdc_device - constructor
//-------------------------------------------------

victor_9000_fdc_device::victor_9000_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VICTOR_9000_FDC, tag, owner, clock),
	m_irq_cb(*this),
	m_syn_cb(*this),
	m_lbrdy_cb(*this),
	m_maincpu(*this, I8048_TAG),
	m_via4(*this, M6522_4_TAG),
	m_via5(*this, M6522_5_TAG),
	m_via6(*this, M6522_6_TAG),
	m_floppy(*this, I8048_TAG":%u", 0U),
	m_gcr_rom(*this, "gcr"),
	m_leds(*this, "led%u", 0U),
	m_da{0, 0},
	m_start{1, 1},
	m_stop{1, 1},
	m_sel{0, 0},
	m_tach{0, 0},
	m_tach_hz{0, 0},
	m_scp_rdy0(0),
	m_scp_rdy1(0),
	m_via_rdy0(1),
	m_via_rdy1(1),
	m_scp_l0ms(0),
	m_scp_l1ms(0),
	m_via_l0ms(0),
	m_via_l1ms(0),
	m_st{0, 0},
	m_stp{0, 0},
	m_drive(0),
	m_side(0),
	m_via4_irq(CLEAR_LINE),
	m_via5_irq(CLEAR_LINE),
	m_via6_irq(CLEAR_LINE),
	m_period(attotime::from_hz(XTAL(15'000'000)/32))
{
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victor_9000_fdc_device::device_start()
{
	m_leds.resolve();

	// allocate timer
	t_gen = timer_alloc(FUNC(victor_9000_fdc_device::gen_tick), this);
	t_tach[0] = timer_alloc(FUNC(victor_9000_fdc_device::tach0_tick), this);
	t_tach[1] = timer_alloc(FUNC(victor_9000_fdc_device::tach1_tick), this);

	// state saving
	save_item(NAME(m_da));
	save_item(NAME(m_start));
	save_item(NAME(m_stop));
	save_item(NAME(m_sel));
	save_item(NAME(m_tach));
	save_item(NAME(m_tach_hz));
	save_item(NAME(m_scp_rdy0));
	save_item(NAME(m_scp_rdy1));
	save_item(NAME(m_via_rdy0));
	save_item(NAME(m_via_rdy1));
	save_item(NAME(m_scp_l0ms));
	save_item(NAME(m_scp_l1ms));
	save_item(NAME(m_via_l0ms));
	save_item(NAME(m_via_l1ms));
	save_item(NAME(m_st));
	save_item(NAME(m_stp));
	save_item(NAME(m_drive));
	save_item(NAME(m_side));
	save_item(NAME(m_drw));
	save_item(NAME(m_erase));
	save_item(NAME(m_via4_irq));
	save_item(NAME(m_via5_irq));
	save_item(NAME(m_via6_irq));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void victor_9000_fdc_device::device_reset()
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);

	live_abort();

	// reset devices
	m_via4->reset();
	m_via5->reset();
	m_via6->reset();

	// set floppy callbacks
	if (m_floppy[0]->get_device())
	{
		m_floppy[0]->get_device()->setup_load_cb(floppy_image_device::load_cb(&victor_9000_fdc_device::load0_cb, this));
		m_floppy[0]->get_device()->setup_unload_cb(floppy_image_device::unload_cb(&victor_9000_fdc_device::unload0_cb, this));
	}

	if (m_floppy[1]->get_device())
	{
		m_floppy[1]->get_device()->setup_load_cb(floppy_image_device::load_cb(&victor_9000_fdc_device::load1_cb, this));
		m_floppy[1]->get_device()->setup_unload_cb(floppy_image_device::unload_cb(&victor_9000_fdc_device::unload1_cb, this));
	}
}


//-------------------------------------------------
//  timer events
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(victor_9000_fdc_device::gen_tick)
{
	live_sync();
	live_run();
}

TIMER_CALLBACK_MEMBER(victor_9000_fdc_device::tach0_tick)
{
	m_tach[0] = !m_tach[0];
	LOGSCP("%s TACH0 %u\n", machine().time().as_string(), m_tach[0]);

	t_tach[0]->adjust(attotime::from_hz(m_tach_hz[0]*2));
}

TIMER_CALLBACK_MEMBER(victor_9000_fdc_device::tach1_tick)
{
	m_tach[1] = !m_tach[1];
	LOGSCP("%s TACH1 %u\n", machine().time().as_string(), m_tach[1]);

	t_tach[1]->adjust(attotime::from_hz(m_tach_hz[1]*2));
}


//-------------------------------------------------
//  floppy_p1_r -
//-------------------------------------------------

uint8_t victor_9000_fdc_device::floppy_p1_r()
{
	/*

	    bit     description

	    0       L0MS0
	    1       L0MS1
	    2       L0MS2
	    3       L0MS3
	    4       L1MS0
	    5       L1MS1
	    6       L1MS2
	    7       L1MS3

	*/

	return (m_via_l1ms << 4) | m_via_l0ms;
}


//-------------------------------------------------
//  floppy_p1_w -
//-------------------------------------------------

void victor_9000_fdc_device::floppy_p1_w(uint8_t data)
{
	/*

	    bit     description

	    0       L0MS0
	    1       L0MS1
	    2       L0MS2
	    3       L0MS3
	    4       L1MS0
	    5       L1MS1
	    6       L1MS2
	    7       L1MS3

	*/

	m_scp_l0ms = data & 0x0f;
	m_scp_l1ms = data >> 4;
}


//-------------------------------------------------
//  floppy_p2_r -
//-------------------------------------------------

uint8_t victor_9000_fdc_device::floppy_p2_r()
{
	/*

	    bit     description

	    0
	    1
	    2
	    3
	    4
	    5
	    6       RDY0
	    7       RDY1

	*/

	uint8_t data = m_p2 & 0x3f;

	data |= m_via_rdy0 << 6;
	data |= m_via_rdy1 << 7;

	return data;
}


//-------------------------------------------------
//  floppy_p2_w -
//-------------------------------------------------

void victor_9000_fdc_device::floppy_p2_w(uint8_t data)
{
	/*

	    bit     description

	    0       START0
	    1       STOP0
	    2       START1
	    3       STOP1
	    4       SEL1
	    5       SEL0
	    6       RDY0
	    7       RDY1

	*/

	m_p2 = data;

	bool sync = false;

	int start0 = BIT(data, 0);
	if (m_start[0] != start0) sync = true;

	int stop0 = BIT(data, 1);
	if (m_stop[0] != stop0) sync = true;

	int start1 = BIT(data, 2);
	if (m_start[1] != start1) sync = true;

	int stop1 = BIT(data, 3);
	if (m_stop[1] != stop1) sync = true;

	int sel0 = BIT(data, 5);
	if (m_sel[0] != sel0) sync = true;

	int sel1 = BIT(data, 4);
	if (m_sel[1] != sel1) sync = true;

	m_scp_rdy0 = BIT(data, 6);
	m_scp_rdy1 = BIT(data, 7);
	update_rdy();

	LOGSCP("%s %s START0/STOP0/SEL0/RDY0 %u/%u/%u/%u START1/STOP1/SEL1/RDY1 %u/%u/%u/%u\n", machine().time().as_string(), machine().describe_context(), start0, stop0, sel0, m_scp_rdy0, start1, stop1, sel1, m_scp_rdy1);

	if (sync)
	{
		live_sync();

		m_start[0] = start0;
		m_stop[0] = stop0;
		m_sel[0] = sel0;
		update_rpm(m_floppy[0]->get_device(), t_tach[0], m_sel[0], m_da[0], m_tach_hz[0]);
		update_spindle_motor(m_floppy[0]->get_device(), t_tach[0], m_start[0], m_stop[0], m_sel[0], m_tach_hz[0]);

		m_start[1] = start1;
		m_stop[1] = stop1;
		m_sel[1] = sel1;
		update_rpm(m_floppy[1]->get_device(), t_tach[1], m_sel[1], m_da[1], m_tach_hz[1]);
		update_spindle_motor(m_floppy[1]->get_device(), t_tach[1], m_start[1], m_stop[1], m_sel[1], m_tach_hz[1]);

		checkpoint();

		if ((m_floppy[0]->get_device() && !m_floppy[0]->get_device()->mon_r()) || (m_floppy[1]->get_device() && !m_floppy[1]->get_device()->mon_r()))
		{
			if(cur_live.state == IDLE)
			{
				live_start();
			}
		}
		else
		{
			live_abort();
		}

		live_run();
	}
}


//-------------------------------------------------
//  tach0_r -
//-------------------------------------------------

int victor_9000_fdc_device::tach0_r()
{
	LOGSCP("%s %s Read TACH0 %u\n", machine().time().as_string(), machine().describe_context(), m_tach[0]);
	return m_tach[0];
}


//-------------------------------------------------
//  tach1_r -
//-------------------------------------------------

int victor_9000_fdc_device::tach1_r()
{
	LOGSCP("%s %s Read TACH1 %u\n", machine().time().as_string(), machine().describe_context(), m_tach[1]);
	return m_tach[1];
}


void victor_9000_fdc_device::update_stepper_motor(floppy_image_device *floppy, int stp, int old_st, int st)
{
	if (stp) return;

	int tracks = 0;

	switch (old_st)
	{
	case   6: if (st == 0xa) tracks++; else if (st ==   5) tracks--; break;
	case   5: if (st ==   6) tracks++; else if (st ==   9) tracks--; break;
	case   9: if (st ==   5) tracks++; else if (st == 0xa) tracks--; break;
	case 0xa: if (st ==   9) tracks++; else if (st ==   6) tracks--; break;
	}

	if (tracks == -1)
	{
		floppy->dir_w(1);
		floppy->stp_w(1);
		floppy->stp_w(0);
	}
	else if (tracks == 1)
	{
		floppy->dir_w(0);
		floppy->stp_w(1);
		floppy->stp_w(0);
	}

	floppy->set_rpm(victor9k_format::get_rpm(m_side, floppy->get_cyl()));
}

void victor_9000_fdc_device::update_spindle_motor(floppy_image_device *floppy, emu_timer *t_tach, bool start, bool stop, bool sel, float tach_hz)
{
	if (start && !stop && floppy->mon_r())
	{
		LOGSCP("%s: motor start\n", floppy->tag());
		floppy->mon_w(0);
		t_tach->adjust(attotime::from_hz(tach_hz));
		t_tach->enable(true);
	}
	else if (stop && !floppy->mon_r())
	{
		LOGSCP("%s: motor stop\n", floppy->tag());
		floppy->mon_w(1);
		t_tach->enable(false);
	}
}

void victor_9000_fdc_device::update_rpm(floppy_image_device *floppy, emu_timer *t_tach, bool sel, uint8_t dacval, float &tach_hz)
{
	if (sel)
	{
		// Map DAC value to RPM, using the range of RPMs for zones 0 to 8,
		// and adding some margin.  The SCP will adjust the DAC value
		// until it measures the desired frequency at the tach inputs.
		const float rpm_min = 240, rpm_max = 430;
		float rpm = rpm_min + ((255-dacval)/255.0f) * (rpm_max-rpm_min);

		tach_hz = rpm / 60.0 * SPINDLE_RATIO * MOTOR_POLES;

		LOGSCP("%s: motor speed %u rpm / tach %0.1f hz %0.9f s (DAC %02x)\n", floppy->tag(), rpm, (double)tach_hz, 1.0/(double)tach_hz, dacval);
	}
}

void victor_9000_fdc_device::update_rdy()
{
	uint8_t output_mask = m_via6->read(via6522_device::VIA_DDRB);

	// The SCP and VIA chips can both output the RDY0/RDY1 (and L0MS/L1MS)
	// signals.  When the VIA chip is configured to output the signal, it wins.
	m_via5->write_ca2((output_mask & 1) ? m_via_rdy0 : m_scp_rdy0);
	m_via5->write_cb2((output_mask & 2) ? m_via_rdy1 : m_scp_rdy1);
}


//-------------------------------------------------
//  da_w -
//-------------------------------------------------

void victor_9000_fdc_device::da_w(uint8_t data)
{
	LOGSCP("%s %s DA %02x SEL0 %u SEL1 %u\n", machine().time().as_string(), machine().describe_context(), data, m_sel[0], m_sel[1]);

	live_sync();
	if (m_sel[0])
	{
		m_da[0] = data;
	}
	if (m_sel[1])
	{
		m_da[1] = data;
	}
	if (m_floppy[0]->get_device()) update_rpm(m_floppy[0]->get_device(), t_tach[0], m_sel[0], m_da[0], m_tach_hz[0]);
	if (m_floppy[1]->get_device()) update_rpm(m_floppy[1]->get_device(), t_tach[1], m_sel[1], m_da[1], m_tach_hz[1]);
	checkpoint();
	live_run();
}

uint8_t victor_9000_fdc_device::via4_pa_r()
{
	/*

	    bit     description

	    PA0     L0MS0
	    PA1     L0MS1
	    PA2     L0MS2
	    PA3     L0MS3
	    PA4
	    PA5
	    PA6
	    PA7

	*/

	uint8_t output_mask = m_via4->read(via6522_device::VIA_DDRA);

	return (m_via_l0ms & output_mask) | (m_scp_l0ms & ~output_mask);
}

void victor_9000_fdc_device::via4_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     L0MS0
	    PA1     L0MS1
	    PA2     L0MS2
	    PA3     L0MS3
	    PA4     ST0A
	    PA5     ST0B
	    PA6     ST0C
	    PA7     ST0D

	*/

	m_via_l0ms = data & 0x0f;

	uint8_t st0 = data >> 4;

	LOGVIA("%s %s L0MS %01x ST0 %01x\n", machine().time().as_string(), machine().describe_context(), m_via_l0ms, st0);

	if (m_st[0] != st0)
	{
		live_sync();
		if (m_floppy[0]->get_device()) update_stepper_motor(m_floppy[0]->get_device(), m_stp[0], st0, m_st[0]);
		m_st[0] = st0;
		checkpoint();
		live_run();
	}
}

uint8_t victor_9000_fdc_device::via4_pb_r()
{
	/*

	    bit     description

	    PB0     L1MS0
	    PB1     L1MS1
	    PB2     L1MS2
	    PB3     L1MS3
	    PB4
	    PB5
	    PB6
	    PB7

	*/

	uint8_t output_mask = m_via4->read(via6522_device::VIA_DDRB);

	return (m_via_l1ms & output_mask) | (m_scp_l1ms & ~output_mask);
}

void victor_9000_fdc_device::via4_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     L1MS0
	    PB1     L1MS1
	    PB2     L1MS2
	    PB3     L1MS3
	    PB4     ST1A
	    PB5     ST1B
	    PB6     ST1C
	    PB7     ST1D

	*/

	m_via_l1ms = data & 0x0f;

	uint8_t st1 = data >> 4;

	LOGVIA("%s %s L1MS %01x ST1 %01x\n", machine().time().as_string(), machine().describe_context(), m_via_l1ms, st1);

	if (m_st[1] != st1)
	{
		live_sync();
		if (m_floppy[1]->get_device()) update_stepper_motor(m_floppy[1]->get_device(), m_stp[1], st1, m_st[1]);
		m_st[1] = st1;
		checkpoint();
		live_run();
	}
}

void victor_9000_fdc_device::wrsync_w(int state)
{
	if (m_wrsync != state)
	{
		live_sync();
		m_wrsync = state;
		cur_live.wrsync = state;
		checkpoint();
		LOGVIA("%s %s WRSYNC %u\n", machine().time().as_string(), machine().describe_context(), state);
		live_run();
	}
}

void victor_9000_fdc_device::via4_irq_w(int state)
{
	m_via4_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}

uint8_t victor_9000_fdc_device::via5_pa_r()
{
	/*

	    bit     description

	    PA0     E0
	    PA1     E1
	    PA2     I1
	    PA3     E2
	    PA4     E4
	    PA5     E5
	    PA6     I7
	    PA7     E6

	*/

	return GCR_DECODE(checkpoint_live.e, checkpoint_live.i);
}

void victor_9000_fdc_device::via5_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     WD0
	    PB1     WD1
	    PB2     WD2
	    PB3     WD3
	    PB4     WD4
	    PB5     WD5
	    PB6     WD6
	    PB7     WD7

	*/

	LOGVIA("%s %s WD %02x\n", machine().time().as_string(), machine().describe_context(), data);

	m_via5->write_cb1(BIT(data, 7));

	if (m_wd != data)
	{
		live_sync();
		m_wd = cur_live.wd = data;
		checkpoint();
		live_run();
	}
}

void victor_9000_fdc_device::via5_irq_w(int state)
{
	m_via5_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}


uint8_t victor_9000_fdc_device::via6_pa_r()
{
	/*

	    bit     description

	    PA0
	    PA1     _TRK0D0
	    PA2
	    PA3     _TRK0D1
	    PA4
	    PA5
	    PA6     WPS
	    PA7     _SYNC

	*/

	LOGVIA("%s %s TRK0D0 %u TRK0D1 %u SYNC %u\n", machine().time().as_string(), machine().describe_context(), m_floppy[0]->get_device() ? m_floppy[0]->get_device()->trk00_r() : 0, m_floppy[1]->get_device() ? m_floppy[1]->get_device()->trk00_r() : 0, checkpoint_live.sync);

	uint8_t data = 0;

	// track 0 drive A sense
	data |= (m_floppy[0]->get_device() ? m_floppy[0]->get_device()->trk00_r() : 0) << 1;

	// track 0 drive B sense
	data |= (m_floppy[1]->get_device() ? m_floppy[1]->get_device()->trk00_r() : 0) << 3;

	// write protect sense
	data |= (m_drive ? (m_floppy[1]->get_device() ? m_floppy[1]->get_device()->wpt_r() : 0) : (m_floppy[0]->get_device() ? m_floppy[0]->get_device()->wpt_r() : 0)) << 6;

	// disk sync detect
	data |= checkpoint_live.sync << 7;

	return data;
}

void victor_9000_fdc_device::via6_pa_w(uint8_t data)
{
	/*

	    bit     description

	    PA0     LED0A
	    PA1
	    PA2     LED1A
	    PA3
	    PA4     SIDE SELECT
	    PA5     DRIVE SELECT
	    PA6
	    PA7

	*/

	// LED, drive A
	m_leds[LED_A] = BIT(data, 0);

	// LED, drive B
	m_leds[LED_B] = BIT(data, 2);

	bool sync = false;

	// dual side select
	int side = BIT(data, 4);
	if (m_side != side)
		sync = true;

	// select drive A/B
	int drive = BIT(data, 5);
	if (m_drive != drive)
		sync = true;

	if (sync)
	{
		live_sync();

		if (side != m_side)
		{
			m_side = side;
			cur_live.side = side;

			for (int i = 0; i < 2; i++)
			{
				floppy_image_device *floppy = m_floppy[i]->get_device();
				if (floppy)
				{
					floppy->ss_w(side);

					// RPM may have changed since the zones are different for
					// the upper and lower heads.
					floppy->set_rpm(victor9k_format::get_rpm(side, floppy->get_cyl()));
				}
			}
		}


		m_drive = drive;
		cur_live.drive = drive;

		LOGVIA("%s %s SIDE %u DRIVE %u\n", machine().time().as_string(), machine().describe_context(), side, drive);

		checkpoint();
		live_run();
	}
}

uint8_t victor_9000_fdc_device::via6_pb_r()
{
	/*

	    bit     description

	    PB0     RDY0 to SCP    Motor speed status, drive A
	    PB1     RDY1 to SCP    Motor speed status, drive B
	    PB2     _SCRESET       Motor speed controller (8048) reset, output
	    PB3     DS1            Door B sense, input       ->order is correct, leads with B
	    PB4     DSO            Door A sense, input
	    PB5     SINGLE/_DOUBLE SIDED
	    PB6     STP0           Stepper enable A
	    PB7     STP1           Stepper enable B

	*/

	uint8_t data = 0;

	uint8_t output_mask = m_via6->read(via6522_device::VIA_DDRB);

	// motor speed status, drive A
	data |= (output_mask & 1) ? m_via_rdy0 : m_scp_rdy0;

	// motor speed status, drive B
	data |= ((output_mask & 2) ? m_via_rdy1 : m_scp_rdy1) << 1;

	// door B sense
	data |= ((m_floppy[1]->get_device() && m_floppy[1]->get_device()->exists()) ? 0 : 1) << 3;

	// door A sense
	data |= ((m_floppy[0]->get_device() && m_floppy[0]->get_device()->exists()) ? 0 : 1) << 4;

	// single/double sided jumper
	data |= (2U << 5);

	return data;
}

void victor_9000_fdc_device::via6_pb_w(uint8_t data)
{
	/*

	    bit     description

	    PB0     Data Ready     Handshake signal during speed table download
	    PB1     Data Ack       Handshake signal during speed table download

	    PB0     RDY0 from SCP  In Normal Operation: drive A speed converged
	    PB1     RDY1 from SCP  In Normal Operation: drive B speed converged
	    PB2     _SCRESET       Motor speed controller (8048) reset, output
	    PB3     DS1            Door B sense, input       ->order is correct, leads with B
	    PB4     DSO            Door A sense, input
	    PB5     SINGLE/_DOUBLE SIDED
	    PB6     STP0           Stepper enable A
	    PB7     STP1           Stepper enable B

	*/

	uint8_t output_mask = m_via6->read(via6522_device::VIA_DDRB);

	m_via_rdy0 = BIT(data, 0);
	m_via_rdy1 = BIT(data, 1);
	update_rdy();

	// _SCRESET in the schematic is misnamed.  There's an inverter between this
	// signal and the _RESET pin of the 8048, so the reset signal is actually
	// active high.
	//
	// Hold the 8048 in reset, unless the reset signal is actively driven low.
	if (BIT(output_mask, 2) && !BIT(data, 2))
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	}

	bool sync = false;

	// stepper enable A
	int stp0 = BIT(data, 6);
	if (m_stp[0] != stp0)
		sync = true;

	// stepper enable B
	int stp1 = BIT(data, 7);
	if (m_stp[1] != stp1)
		sync = true;
	m_via6->write_cb1(stp1);

	if (sync)
	{
		live_sync();

		m_stp[0] = stp0;
		if (m_floppy[0]->get_device())
			update_stepper_motor(m_floppy[0]->get_device(), m_stp[0], m_st[0], m_st[0]);

		m_stp[1] = stp1;
		if (m_floppy[1]->get_device())
			update_stepper_motor(m_floppy[1]->get_device(), m_stp[1], m_st[1], m_st[1]);

		LOGVIA("%s %s STP0 %u STP1 %u\n", machine().time().as_string(), machine().describe_context(), stp0, stp1);

		checkpoint();
		live_run();
	}
}

void victor_9000_fdc_device::drw_w(int state)
{
	if (m_drw != state)
	{
		live_sync();
		m_drw = cur_live.drw = state;
		LOGVIA("%s %s DRW %u\n", machine().time().as_string(), machine().describe_context(), state);
		if (state)
		{
			pll_stop_writing(get_floppy(), cur_live.tm);
		}
		else
		{
			pll_start_writing(cur_live.tm);
		}
		checkpoint();
		live_run();
	}
}

void victor_9000_fdc_device::erase_w(int state)
{
	if (m_erase != state)
	{
		live_sync();
		m_erase = cur_live.erase = state;
		checkpoint();
		LOGVIA("%s %s ERASE %u\n", machine().time().as_string(), machine().describe_context(), state);
		live_run();
	}
}

void victor_9000_fdc_device::via6_irq_w(int state)
{
	m_via6_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}

uint8_t victor_9000_fdc_device::cs7_r(offs_t offset)
{
	m_lbrdy_cb(1);

	LOGVIA("%s %s LBRDY 1 : %02x\n", machine().time().as_string(), machine().describe_context(), m_via5->read(offset));

	return m_via5->read(offset);
}

void victor_9000_fdc_device::cs7_w(offs_t offset, uint8_t data)
{
	m_lbrdy_cb(1);

	LOGVIA("%s %s LBRDY 1\n", machine().time().as_string(), machine().describe_context());

	m_via5->write(offset, data);
}

floppy_image_device* victor_9000_fdc_device::get_floppy()
{
	return m_floppy[m_drive ? 1 : 0]->get_device();
}

void victor_9000_fdc_device::live_start()
{
	cur_live.tm = machine().time();
	cur_live.state = RUNNING;
	cur_live.next_state = -1;

	cur_live.shift_reg = 0;
	cur_live.shift_reg_write = 0;
	cur_live.bit_counter = 0;
	cur_live.sync_bit_counter = 0;
	cur_live.sync_byte_counter = 0;

	cur_live.drive = m_drive;
	cur_live.side = m_side;
	cur_live.drw = m_drw;
	cur_live.wd = m_wd;
	cur_live.wrsync = m_wrsync;
	cur_live.erase = m_erase;

	pll_reset(cur_live.tm);
	checkpoint_live = cur_live;
	pll_save_checkpoint();

	live_run();
}

void victor_9000_fdc_device::pll_reset(const attotime &when)
{
	cur_pll.reset(when);
	cur_pll.set_clock(attotime::from_nsec(2130));
}

void victor_9000_fdc_device::pll_start_writing(const attotime &tm)
{
	pll_reset(cur_live.tm);
	cur_pll.start_writing(tm);
}

void victor_9000_fdc_device::pll_commit(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.commit(floppy, tm);
}

void victor_9000_fdc_device::pll_stop_writing(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.stop_writing(floppy, tm);
	pll_reset(cur_live.tm);
}

void victor_9000_fdc_device::pll_save_checkpoint()
{
	checkpoint_pll = cur_pll;
}

void victor_9000_fdc_device::pll_retrieve_checkpoint()
{
	cur_pll = checkpoint_pll;
}

int victor_9000_fdc_device::pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.get_next_bit(tm, floppy, limit);
}

bool victor_9000_fdc_device::pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.write_next_bit(bit, tm, floppy, limit);
}

void victor_9000_fdc_device::checkpoint()
{
	pll_commit(get_floppy(), cur_live.tm);
	checkpoint_live = cur_live;
	pll_save_checkpoint();
}

void victor_9000_fdc_device::rollback()
{
	cur_live = checkpoint_live;
	pll_retrieve_checkpoint();
}

void victor_9000_fdc_device::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
	else
		live_sync();
}

void victor_9000_fdc_device::live_sync()
{
	if(!cur_live.tm.is_never())
	{
		if(cur_live.tm > machine().time())
		{
			rollback();
			live_run(machine().time());
			pll_commit(get_floppy(), cur_live.tm);
		}
		else
		{
			pll_commit(get_floppy(), cur_live.tm);
			if(cur_live.next_state != -1)
			{
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE)
			{
				pll_stop_writing(get_floppy(), cur_live.tm);
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void victor_9000_fdc_device::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time())
	{
		rollback();
		live_run(machine().time());
	}

	pll_stop_writing(get_floppy(), cur_live.tm);

	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;

	cur_live.brdy = 1;
	cur_live.lbrdy_changed = true;
	cur_live.sync = 1;
	cur_live.syn = 1;
	cur_live.syn_changed = true;
	cur_live.gcr_err = 1;
}

void victor_9000_fdc_device::live_run(const attotime &limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	for(;;)
	{
		switch(cur_live.state)
		{
		case RUNNING:
		{
			bool syncpoint = false;

			if (cur_live.tm > limit)
				return;

			// read bit
			int bit = 0;
			if (cur_live.drw)
			{
				bit = pll_get_next_bit(cur_live.tm, get_floppy(), limit);
				if(bit < 0)
					return;
			}

			// write bit
			int write_bit = 0;
			if (!cur_live.drw) // TODO WPS
			{
				write_bit = BIT(cur_live.shift_reg_write, 9);
				if (pll_write_next_bit(write_bit, cur_live.tm, get_floppy(), limit))
					return;
			}

			// clock read shift register
			cur_live.shift_reg <<= 1;
			cur_live.shift_reg |= bit;
			cur_live.shift_reg &= 0x3ff;

			// sync
			int sync = !(cur_live.shift_reg == 0x3ff);

			// bit counter
			if (cur_live.drw)
			{
				if (!sync)
				{
					cur_live.bit_counter = 0;
				}
				else if (cur_live.sync)
				{
					cur_live.bit_counter++;
					if (cur_live.bit_counter == 10)
					{
						cur_live.bit_counter = 0;
					}
				}
			}
			else
			{
				cur_live.bit_counter++;
				if (cur_live.bit_counter == 10)
				{
					cur_live.bit_counter = 0;
				}
			}

			// sync counter
			if (sync)
			{
				cur_live.sync_bit_counter = 0;
				cur_live.sync_byte_counter = 10; // TODO 9 in schematics
			}
			else if (!cur_live.sync)
			{
				cur_live.sync_bit_counter++;
				if (cur_live.sync_bit_counter == 10)
				{
					cur_live.sync_bit_counter = 0;
					cur_live.sync_byte_counter++;
					if (cur_live.sync_byte_counter == 16)
					{
						cur_live.sync_byte_counter = 0;
					}
				}
			}

			// syn
			int syn = !(cur_live.sync_byte_counter == 15);

			// GCR decoder
			if (cur_live.drw)
			{
				cur_live.i = cur_live.drw << 10 | cur_live.shift_reg;
			}
			else
			{
				cur_live.i = cur_live.drw << 10 | 0x200 | ((cur_live.wd & 0xf0) << 1) | cur_live.wrsync << 4 | (cur_live.wd & 0x0f);
			}

			cur_live.e = m_gcr_rom->base()[cur_live.i];

			attotime next = cur_live.tm + m_period;
			LOGDISK("%s:%s cyl %u bit %u sync %u bc %u sr %03x sbc %u sBC %u syn %u i %03x e %02x\n",cur_live.tm.as_string(),next.as_string(),get_floppy()->get_cyl(),bit,sync,cur_live.bit_counter,cur_live.shift_reg,cur_live.sync_bit_counter,cur_live.sync_byte_counter,syn,cur_live.i,cur_live.e);

			// byte ready
			int brdy = !(cur_live.bit_counter == 9);

			// GCR error
			int gcr_err = !(brdy || BIT(cur_live.e, 3));

			if (cur_live.drw)
				LOGBITS("%s cyl %u bit %u sync %u bc %u sr %03x i %03x e %02x\n",cur_live.tm.as_string(),get_floppy()->get_cyl(),bit,sync,cur_live.bit_counter,cur_live.shift_reg,cur_live.i,cur_live.e);
			else
				LOGBITS("%s cyl %u writing bit %u bc %u sr %03x i %03x e %02x\n",cur_live.tm.as_string(),get_floppy()->get_cyl(),write_bit,cur_live.bit_counter,cur_live.shift_reg_write,cur_live.i,cur_live.e);

			if (!brdy)
			{
				// load write shift register
				cur_live.shift_reg_write = GCR_ENCODE(cur_live.e, cur_live.i);

				LOGDISK("%s load write shift register %03x\n",cur_live.tm.as_string(),cur_live.shift_reg_write);
			}
			else
			{
				// clock write shift register
				cur_live.shift_reg_write <<= 1;
				cur_live.shift_reg_write &= 0x3ff;
			}

			if (brdy != cur_live.brdy)
			{
				LOGDISK("%s BRDY %u\n", cur_live.tm.as_string(),brdy);
				if (!brdy)
				{
					cur_live.lbrdy_changed = true;
					LOGDISK("%s LBRDY 0 : %02x\n", cur_live.tm.as_string(), GCR_DECODE(cur_live.e, cur_live.i));
				}
				cur_live.brdy = brdy;
				syncpoint = true;
			}

			if (sync != cur_live.sync)
			{
				LOGDISK("%s SYNC %u\n", cur_live.tm.as_string(),sync);
				cur_live.sync = sync;
				syncpoint = true;
			}

			if (syn != cur_live.syn)
			{
				LOGDISK("%s SYN %u\n", cur_live.tm.as_string(),syn);
				cur_live.syn = syn;
				cur_live.syn_changed = true;
				syncpoint = true;
			}

			if (gcr_err != cur_live.gcr_err)
			{
				LOGDISK("%s GCR ERR %u\n", cur_live.tm.as_string(),gcr_err);
				cur_live.gcr_err = gcr_err;
				syncpoint = true;
			}

			if (syncpoint)
			{
				live_delay(RUNNING_SYNCPOINT);
				return;
			}
			break;
		}

		case RUNNING_SYNCPOINT:
		{
			if (cur_live.lbrdy_changed)
			{
				m_lbrdy_cb(0);
				cur_live.lbrdy_changed = false;
			}

			if (cur_live.syn_changed)
			{
				m_syn_cb(cur_live.syn);
				cur_live.syn_changed = false;
			}

			m_via5->write_ca1(cur_live.brdy);

			cur_live.state = RUNNING;
			checkpoint();
			break;
		}
		}
	}
}

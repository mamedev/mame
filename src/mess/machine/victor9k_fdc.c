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

    TODO:

    - write protect
    - separate read/write methods
    - communication error with SCP after loading boot sector
        - bp ff1a8
        - patch ff1ab=c3
    - single/double sided jumper
    - header sync length unknown (6 is too short)
    - 8048 spindle speed control

*/

#include "victor9k_fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LOG 0
#define LOG_VIA 0
#define LOG_SCP 0
#define LOG_BITS 0

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
#define SPINDLE_RATIO   6.4
#define MOTOR_POLES     8

// TODO wrong values here! motor speed is controlled by an LM2917, with help from the spindle TACH and a DAC0808 whose value is set by the SCP 8048
const int victor_9000_fdc_t::rpm[] = { 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 254, 255, 257, 259, 260, 262, 264, 266, 267, 269, 271, 273, 275, 276, 278, 280, 282, 284, 286, 288, 290, 291, 293, 295, 297, 299, 301, 303, 305, 307, 309, 311, 313, 315, 318, 320, 322, 324, 326, 328, 330, 333, 335, 337, 339, 342, 344, 346, 348, 351, 353, 355, 358, 360, 362, 365, 367, 370, 372, 375, 377, 380, 382, 385, 387, 390, 392, 395, 398, 400, 403, 406, 408, 411, 414, 416, 419, 422, 425, 428, 430, 433, 436, 439, 442, 445, 448, 451, 454, 457, 460, 463, 466, 469, 472, 475, 478, 482, 485, 488, 491, 494, 498, 501, 504, 508, 511, 514, 518, 521, 525, 528, 532, 535, 539, 542, 546, 550, 553, 557, 561, 564, 568, 572, 576, 579, 583, 587, 591, 595, 599, 603, 607, 611, 615, 619, 623, 627, 631, 636, 640, 644, 648, 653, 657, 661, 666, 670, 674, 679, 683, 688, 693, 697, 702, 706, 711, 716, 721, 725, 730, 735, 740, 745, 750, 755, 760, 765, 770, 775, 780, 785, 790, 796, 801, 806, 812, 817, 822, 828, 833, 839, 844, 850, 856, 861, 867, 873, 878, 884 };



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type VICTOR_9000_FDC = &device_creator<victor_9000_fdc_t>;


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

const rom_entry *victor_9000_fdc_t::device_rom_region() const
{
	return ROM_NAME( victor_9000_fdc );
}


//-------------------------------------------------
//  ADDRESS_MAP( floppy_io )
//-------------------------------------------------

static ADDRESS_MAP_START( floppy_io, AS_IO, 8, victor_9000_fdc_t )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READWRITE(floppy_p1_r, floppy_p1_w)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_READWRITE(floppy_p2_r, floppy_p2_w)
	AM_RANGE(MCS48_PORT_T0, MCS48_PORT_T0) AM_READ(tach0_r)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(tach1_r)
	AM_RANGE(MCS48_PORT_BUS, MCS48_PORT_BUS) AM_WRITE(da_w)
ADDRESS_MAP_END


//-------------------------------------------------
//  SLOT_INTERFACE( victor9k_floppies )
//-------------------------------------------------

int victor_9000_fdc_t::load0_cb(floppy_image_device *device)
{
	// DOOR OPEN 0
	m_via4->write_ca1(0);

	return IMAGE_INIT_PASS;
}

void victor_9000_fdc_t::unload0_cb(floppy_image_device *device)
{
	// DOOR OPEN 0
	m_via4->write_ca1(1);
}

int victor_9000_fdc_t::load1_cb(floppy_image_device *device)
{
	// DOOR OPEN 1
	m_via4->write_cb1(0);

	return IMAGE_INIT_PASS;
}

void victor_9000_fdc_t::unload1_cb(floppy_image_device *device)
{
	// DOOR OPEN 1
	m_via4->write_cb1(1);
}

static SLOT_INTERFACE_START( victor9k_floppies )
	SLOT_INTERFACE( "525ssqd", FLOPPY_525_SSQD ) // Tandon TM100-3 with custom electronics
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD ) // Tandon TM100-4 with custom electronics
SLOT_INTERFACE_END

FLOPPY_FORMATS_MEMBER( victor_9000_fdc_t::floppy_formats )
	FLOPPY_VICTOR_9000_FORMAT
FLOPPY_FORMATS_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( victor_9000_fdc )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( victor_9000_fdc )
	MCFG_CPU_ADD(I8048_TAG, I8048, XTAL_30MHz/6)
	MCFG_CPU_IO_MAP(floppy_io)

	MCFG_DEVICE_ADD(M6522_4_TAG, VIA6522, XTAL_30MHz/30)
	MCFG_VIA6522_READPA_HANDLER(READ8(victor_9000_fdc_t, via4_pa_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(victor_9000_fdc_t, via4_pa_w))
	MCFG_VIA6522_READPB_HANDLER(READ8(victor_9000_fdc_t, via4_pb_r))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(victor_9000_fdc_t, via4_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(victor_9000_fdc_t, wrsync_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(victor_9000_fdc_t, via4_irq_w))

	MCFG_DEVICE_ADD(M6522_5_TAG, VIA6522, XTAL_30MHz/30)
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(victor_9000_fdc_t, via5_irq_w))
	MCFG_VIA6522_READPA_HANDLER(READ8(victor_9000_fdc_t, via5_pa_r))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(victor_9000_fdc_t, via5_pb_w))

	MCFG_DEVICE_ADD(M6522_6_TAG, VIA6522, XTAL_30MHz/30)
	MCFG_VIA6522_READPA_HANDLER(READ8(victor_9000_fdc_t, via6_pa_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(victor_9000_fdc_t, via6_pb_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(victor_9000_fdc_t, via6_pa_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(victor_9000_fdc_t, via6_pb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(victor_9000_fdc_t, drw_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(victor_9000_fdc_t, erase_w))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(victor_9000_fdc_t, via6_irq_w))

	MCFG_FLOPPY_DRIVE_ADD(I8048_TAG":0", victor9k_floppies, "525qd", victor_9000_fdc_t::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(I8048_TAG":1", victor9k_floppies, "525qd", victor_9000_fdc_t::floppy_formats)
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor victor_9000_fdc_t::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( victor_9000_fdc );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  victor_9000_fdc_t - constructor
//-------------------------------------------------

victor_9000_fdc_t::victor_9000_fdc_t(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, VICTOR_9000_FDC, "Victor 9000 FDC", tag, owner, clock, "victor9k_fdc", __FILE__),
	m_irq_cb(*this),
	m_syn_cb(*this),
	m_lbrdy_cb(*this),
	m_maincpu(*this, I8048_TAG),
	m_via4(*this, M6522_4_TAG),
	m_via5(*this, M6522_5_TAG),
	m_via6(*this, M6522_6_TAG),
	m_floppy0(*this, I8048_TAG":0:525qd"),
	m_floppy1(*this, I8048_TAG":1:525qd"),
	m_gcr_rom(*this, "gcr"),
	m_da(0),
	m_da0(0),
	m_da1(0),
	m_start0(1),
	m_stop0(1),
	m_start1(1),
	m_stop1(1),
	m_sel0(0),
	m_sel1(0),
	m_tach0(0),
	m_tach1(0),
	m_rdy0(0),
	m_rdy1(0),
	m_l0ms(0),
	m_l1ms(0),
	m_st0(0),
	m_st1(0),
	m_stp0(0),
	m_stp1(0),
	m_drive(0),
	m_side(0),
	m_via4_irq(CLEAR_LINE),
	m_via5_irq(CLEAR_LINE),
	m_via6_irq(CLEAR_LINE),
	m_period(attotime::from_hz(XTAL_15MHz/32))
{
	cur_live.tm = attotime::never;
	cur_live.state = IDLE;
	cur_live.next_state = -1;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void victor_9000_fdc_t::device_start()
{
	// resolve callbacks
	m_irq_cb.resolve_safe();
	m_syn_cb.resolve_safe();
	m_lbrdy_cb.resolve_safe();

	// allocate timer
	t_gen = timer_alloc(TM_GEN);
	t_tach0 = timer_alloc(TM_TACH0);
	t_tach1 = timer_alloc(TM_TACH1);

	// state saving
	save_item(NAME(m_da));
	save_item(NAME(m_da0));
	save_item(NAME(m_da1));
	save_item(NAME(m_start0));
	save_item(NAME(m_stop0));
	save_item(NAME(m_start1));
	save_item(NAME(m_stop1));
	save_item(NAME(m_sel0));
	save_item(NAME(m_sel1));
	save_item(NAME(m_tach0));
	save_item(NAME(m_tach1));
	save_item(NAME(m_rdy0));
	save_item(NAME(m_rdy1));
	save_item(NAME(m_l0ms));
	save_item(NAME(m_l1ms));
	save_item(NAME(m_st0));
	save_item(NAME(m_st1));
	save_item(NAME(m_stp0));
	save_item(NAME(m_stp1));
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

void victor_9000_fdc_t::device_reset()
{
	live_abort();

	// reset devices
	m_via4->reset();
	m_via5->reset();
	m_via6->reset();

	// set floppy callbacks
	m_floppy0->setup_load_cb(floppy_image_device::load_cb(FUNC(victor_9000_fdc_t::load0_cb), this));
	m_floppy0->setup_unload_cb(floppy_image_device::unload_cb(FUNC(victor_9000_fdc_t::unload0_cb), this));

	m_floppy1->setup_load_cb(floppy_image_device::load_cb(FUNC(victor_9000_fdc_t::load1_cb), this));
	m_floppy1->setup_unload_cb(floppy_image_device::unload_cb(FUNC(victor_9000_fdc_t::unload1_cb), this));
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void victor_9000_fdc_t::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TM_GEN:
		live_sync();
		live_run();
		break;

	case TM_TACH0:
		m_tach0 = !m_tach0;
		if (LOG_SCP) logerror("TACH0 %u\n", m_tach0);
		break;

	case TM_TACH1:
		m_tach1 = !m_tach1;
		if (LOG_SCP) logerror("TACH1 %u\n", m_tach1);
		break;
	}
}


//-------------------------------------------------
//  floppy_p1_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::floppy_p1_r )
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

	return (m_l1ms << 4) | m_l0ms;
}


//-------------------------------------------------
//  floppy_p1_w -
//-------------------------------------------------

WRITE8_MEMBER( victor_9000_fdc_t::floppy_p1_w )
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

	m_l0ms = data & 0x0f;
	m_l1ms = data >> 4;
}


//-------------------------------------------------
//  floppy_p2_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::floppy_p2_r )
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

	UINT8 data = m_p2 & 0x3f;

	data |= m_rdy0 << 6;
	data |= m_rdy1 << 7;

	return data;
}


//-------------------------------------------------
//  floppy_p2_w -
//-------------------------------------------------

WRITE8_MEMBER( victor_9000_fdc_t::floppy_p2_w )
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
	if (m_start0 != start0) sync = true;

	int stop0 = BIT(data, 1);
	if (m_stop0 != stop0) sync = true;

	int start1 = BIT(data, 2);
	if (m_start1 != start1) sync = true;

	int stop1 = BIT(data, 3);
	if (m_stop1 != stop1) sync = true;

	int sel0 = BIT(data, 5);
	if (m_sel0 != sel0) sync = true;

	int sel1 = BIT(data, 4);
	if (m_sel1 != sel1) sync = true;

	set_rdy0(BIT(data, 6));
	set_rdy1(BIT(data, 7));

	if (LOG_SCP) logerror("%s %s START0/STOP0/SEL0/RDY0 %u/%u/%u/%u START1/STOP1/SEL1/RDY1 %u/%u/%u/%u\n", machine().time().as_string(), machine().describe_context(), start0, stop0, sel0, m_rdy0, start1, stop1, sel1, m_rdy1);

	if (sync)
	{
		live_sync();

		m_start0 = start0;
		m_stop0 = stop0;
		m_sel0 = sel0;
		//update_spindle_motor(m_floppy0, t_tach0, m_start0, m_stop0, m_sel0, m_da0);

		m_start1 = start1;
		m_stop1 = stop1;
		m_sel1 = sel1;
		//update_spindle_motor(m_floppy1, t_tach1, m_start1, m_stop1, m_sel1, m_da1);

		checkpoint();

		if (!m_floppy0->mon_r() || !m_floppy1->mon_r()) {
			if(cur_live.state == IDLE) {
				live_start();
			}
		} else {
			live_abort();
		}

		live_run();
	}
}


//-------------------------------------------------
//  tach0_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::tach0_r )
{
	return m_tach0;
}


//-------------------------------------------------
//  tach1_r -
//-------------------------------------------------

READ8_MEMBER( victor_9000_fdc_t::tach1_r )
{
	return m_tach1;
}


void victor_9000_fdc_t::update_stepper_motor(floppy_image_device *floppy, int stp, int old_st, int st)
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
}

void victor_9000_fdc_t::update_spindle_motor(floppy_image_device *floppy, emu_timer *t_tach, bool start, bool stop, bool sel, UINT8 &da)
{
	if (start && !stop && floppy->mon_r()) {
		if (LOG_SCP) logerror("%s: motor start\n", floppy->tag());
		floppy->mon_w(0);
	} else if (stop && !floppy->mon_r()) {
		if (LOG_SCP) logerror("%s: motor stop\n", floppy->tag());
		floppy->mon_w(1);
		t_tach->reset();
	}

	if (sel) {
		da = m_da;
		if (!floppy->mon_r()) {
			float tach = rpm[da] / 60 * SPINDLE_RATIO * MOTOR_POLES;

			if (LOG_SCP) logerror("%s: motor speed %u rpm / tach %0.1f hz (DA %02x)\n", floppy->tag(), rpm[da], (double) tach, da);

			t_tach->adjust(attotime::from_hz(tach*2), 0, attotime::from_hz(tach*2));
			floppy->set_rpm(rpm[da]);
		}
	}
}

void victor_9000_fdc_t::set_rdy0(int state)
{
	//m_rdy0 = state;
	//m_via5->write_ca2(m_rdy0);
}

void victor_9000_fdc_t::set_rdy1(int state)
{
	//m_rdy1 = state;
	//m_via5->write_cb2(m_rdy1);
}


//-------------------------------------------------
//  da_w -
//-------------------------------------------------

WRITE8_MEMBER( victor_9000_fdc_t::da_w )
{
	if (LOG_SCP) logerror("%s %s DA %02x\n", machine().time().as_string(), machine().describe_context(), data);

	if (m_da != data)
	{
		live_sync();
		m_da = data;
		update_spindle_motor(m_floppy0, t_tach0, m_start0, m_stop0, m_sel0, m_da0);
		update_spindle_motor(m_floppy1, t_tach1, m_start1, m_stop1, m_sel1, m_da1);
		checkpoint();
		live_run();
	}
}

READ8_MEMBER( victor_9000_fdc_t::via4_pa_r )
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

	return m_l0ms;
}

WRITE8_MEMBER( victor_9000_fdc_t::via4_pa_w )
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

	m_l0ms = data & 0x0f;

	{ // HACK to bypass SCP
		m_floppy0->mon_w((m_l0ms == 0xf) ? 1 : 0);
		m_floppy0->set_rpm(victor9k_format::get_rpm(m_side, m_floppy0->get_cyl()));
		m_rdy0 = (m_l0ms == 0xf) ? 0 : 1;
		m_via5->write_ca2(m_rdy0);
	}

	UINT8 st0 = data >> 4;

	if (LOG_VIA) logerror("%s %s L0MS %01x ST0 %01x\n", machine().time().as_string(), machine().describe_context(), m_l0ms, st0);

	if (m_st0 != st0)
	{
		live_sync();
		update_stepper_motor(m_floppy0, m_stp0, st0, m_st0);
		m_st0 = st0;
		checkpoint();
		live_run();
	}
}

READ8_MEMBER( victor_9000_fdc_t::via4_pb_r )
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

	return m_l1ms;
}

WRITE8_MEMBER( victor_9000_fdc_t::via4_pb_w )
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

	m_l1ms = data & 0x0f;

	{ // HACK to bypass SCP
		m_floppy1->mon_w((m_l1ms == 0xf) ? 1 : 0);
		m_floppy1->set_rpm(victor9k_format::get_rpm(m_side, m_floppy1->get_cyl()));
		m_rdy1 = (m_l1ms == 0xf) ? 0 : 1;
		m_via5->write_cb2(m_rdy1);
	}

	UINT8 st1 = data >> 4;

	if (LOG_VIA) logerror("%s %s L1MS %01x ST1 %01x\n", machine().time().as_string(), machine().describe_context(), m_l1ms, st1);

	if (m_st1 != st1)
	{
		live_sync();
		update_stepper_motor(m_floppy1, m_stp1, st1, m_st1);
		m_st1 = st1;
		checkpoint();
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::wrsync_w )
{
	if (m_wrsync != state)
	{
		live_sync();
		m_wrsync = state;
		cur_live.wrsync = state;
		checkpoint();
		if (LOG_VIA) logerror("%s %s WRSYNC %u\n", machine().time().as_string(), machine().describe_context(), state);
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::via4_irq_w )
{
	m_via4_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}

READ8_MEMBER( victor_9000_fdc_t::via5_pa_r )
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

WRITE8_MEMBER( victor_9000_fdc_t::via5_pb_w )
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

	if (LOG_VIA) logerror("%s %s WD %02x\n", machine().time().as_string(), machine().describe_context(), data);

	if (m_wd != data)
	{
		live_sync();
		m_wd = cur_live.wd = data;
		checkpoint();
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::via5_irq_w )
{
	m_via5_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}


READ8_MEMBER( victor_9000_fdc_t::via6_pa_r )
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

	if (LOG_VIA) logerror("%s %s TRK0D0 %u TRK0D1 %u SYNC %u\n", machine().time().as_string(), machine().describe_context(), m_floppy0->trk00_r(), m_floppy1->trk00_r(), checkpoint_live.sync);

	UINT8 data = 0;

	// track 0 drive A sense
	data |= m_floppy0->trk00_r() << 1;

	// track 0 drive B sense
	data |= m_floppy1->trk00_r() << 3;

	// write protect sense
	data |= (m_drive ? m_floppy1->wpt_r() : m_floppy0->wpt_r()) << 6;

	// disk sync detect
	data |= checkpoint_live.sync << 7;

	return data;
}

WRITE8_MEMBER( victor_9000_fdc_t::via6_pa_w )
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
	output_set_led_value(LED_A, BIT(data, 0));

	// LED, drive B
	output_set_led_value(LED_B, BIT(data, 2));

	bool sync = false;

	// dual side select
	int side = BIT(data, 4);
	if (m_side != side) sync = true;

	// select drive A/B
	int drive = BIT(data, 5);
	if (m_drive != drive) sync = true;

	if (sync)
	{
		live_sync();

		m_side = side;
		cur_live.side = side;
		m_floppy0->ss_w(side);
		m_floppy1->ss_w(side);

		m_drive = drive;
		cur_live.drive = drive;

		if (LOG_VIA) logerror("%s %s SIDE %u DRIVE %u\n", machine().time().as_string(), machine().describe_context(), side, drive);

		checkpoint();
		live_run();
	}
}

READ8_MEMBER( victor_9000_fdc_t::via6_pb_r )
{
	/*

	    bit     description

	    PB0     RDY0
	    PB1     RDY1
	    PB2
	    PB3     _DS1
	    PB4     _DS0
	    PB5     SINGLE/_DOUBLE SIDED
	    PB6
	    PB7

	*/

	UINT8 data = 0;

	// motor speed status, drive A
	data |= m_rdy0;

	// motor speed status, drive B
	data |= m_rdy1 << 1;

	// door B sense
	data |= (m_floppy1->exists() ? 0 : 1) << 3;

	// door A sense
	data |= (m_floppy0->exists() ? 0 : 1) << 4;

	// single/double sided jumper
	//data |= 0x20;

	return data;
}

WRITE8_MEMBER( victor_9000_fdc_t::via6_pb_w )
{
	/*

	    bit     description

	    PB0     RDY0
	    PB1     RDY1
	    PB2     _SCRESET
	    PB3
	    PB4
	    PB5
	    PB6     STP0
	    PB7     STP1

	*/

	set_rdy0(BIT(data, 0));
	set_rdy1(BIT(data, 1));

	// motor speed controller reset
	if (!BIT(data, 2))
		m_maincpu->reset();

	bool sync = false;

	// stepper enable A
	int stp0 = BIT(data, 6);
	if (m_stp0 != stp0) sync = true;

	// stepper enable B
	int stp1 = BIT(data, 7);
	if (m_stp1 != stp1) sync = true;

	if (sync)
	{
		live_sync();

		m_stp0 = stp0;
		update_stepper_motor(m_floppy0, m_stp0, m_st0, m_st0);

		m_stp1 = stp1;
		update_stepper_motor(m_floppy1, m_stp1, m_st1, m_st1);

		if (LOG_VIA) logerror("%s %s STP0 %u STP1 %u\n", machine().time().as_string(), machine().describe_context(), stp0, stp1);

		checkpoint();
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::drw_w )
{
	if (m_drw != state)
	{
		live_sync();
		m_drw = cur_live.drw = state;
		checkpoint();
		if (LOG_VIA) logerror("%s %s DRW %u\n", machine().time().as_string(), machine().describe_context(), state);
		if (state) {
			pll_stop_writing(get_floppy(), machine().time());
		} else {
			pll_start_writing(machine().time());
		}
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::erase_w )
{
	if (m_erase != state)
	{
		live_sync();
		m_erase = cur_live.erase = state;
		checkpoint();
		if (LOG_VIA) logerror("%s %s ERASE %u\n", machine().time().as_string(), machine().describe_context(), state);
		live_run();
	}
}

WRITE_LINE_MEMBER( victor_9000_fdc_t::via6_irq_w )
{
	m_via6_irq = state;

	m_irq_cb(m_via4_irq || m_via5_irq || m_via6_irq);
}

READ8_MEMBER( victor_9000_fdc_t::cs7_r )
{
	m_lbrdy_cb(1);

	if (LOG_VIA) logerror("%s %s LBRDY 1 : %02x\n", machine().time().as_string(), machine().describe_context(), m_via5->read(space, offset));

	return m_via5->read(space, offset);
}

WRITE8_MEMBER( victor_9000_fdc_t::cs7_w )
{
	m_lbrdy_cb(1);

	if (LOG_VIA) logerror("%s %s LBRDY 1\n", machine().time().as_string(), machine().describe_context());

	m_via5->write(space, offset, data);
}

floppy_image_device* victor_9000_fdc_t::get_floppy()
{
	return m_drive ? m_floppy1 : m_floppy0;
}

void victor_9000_fdc_t::live_start()
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

void victor_9000_fdc_t::pll_reset(const attotime &when)
{
	cur_pll.reset(when);
	cur_pll.set_clock(attotime::from_nsec(2130));
}

void victor_9000_fdc_t::pll_start_writing(const attotime &tm)
{
	cur_pll.start_writing(tm);
	pll_reset(cur_live.tm);
}

void victor_9000_fdc_t::pll_commit(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.commit(floppy, tm);
}

void victor_9000_fdc_t::pll_stop_writing(floppy_image_device *floppy, const attotime &tm)
{
	cur_pll.stop_writing(floppy, tm);
	pll_reset(cur_live.tm);
}

void victor_9000_fdc_t::pll_save_checkpoint()
{
	checkpoint_pll = cur_pll;
}

void victor_9000_fdc_t::pll_retrieve_checkpoint()
{
	cur_pll = checkpoint_pll;
}

int victor_9000_fdc_t::pll_get_next_bit(attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.get_next_bit(tm, floppy, limit);
}

bool victor_9000_fdc_t::pll_write_next_bit(bool bit, attotime &tm, floppy_image_device *floppy, const attotime &limit)
{
	return cur_pll.write_next_bit(bit, tm, floppy, limit);
}

void victor_9000_fdc_t::checkpoint()
{
	pll_commit(get_floppy(), cur_live.tm);
	checkpoint_live = cur_live;
	pll_save_checkpoint();
}

void victor_9000_fdc_t::rollback()
{
	cur_live = checkpoint_live;
	pll_retrieve_checkpoint();
}

void victor_9000_fdc_t::live_delay(int state)
{
	cur_live.next_state = state;
	if(cur_live.tm != machine().time())
		t_gen->adjust(cur_live.tm - machine().time());
	else
		live_sync();
}

void victor_9000_fdc_t::live_sync()
{
	if(!cur_live.tm.is_never()) {
		if(cur_live.tm > machine().time()) {
			rollback();
			live_run(machine().time());
			pll_commit(get_floppy(), cur_live.tm);
		} else {
			pll_commit(get_floppy(), cur_live.tm);
			if(cur_live.next_state != -1) {
				cur_live.state = cur_live.next_state;
				cur_live.next_state = -1;
			}
			if(cur_live.state == IDLE) {
				pll_stop_writing(get_floppy(), cur_live.tm);
				cur_live.tm = attotime::never;
			}
		}
		cur_live.next_state = -1;
		checkpoint();
	}
}

void victor_9000_fdc_t::live_abort()
{
	if(!cur_live.tm.is_never() && cur_live.tm > machine().time()) {
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

void victor_9000_fdc_t::live_run(const attotime &limit)
{
	if(cur_live.state == IDLE || cur_live.next_state != -1)
		return;

	for(;;) {
		switch(cur_live.state) {
		case RUNNING: {
			bool syncpoint = false;

			if (cur_live.tm > limit)
				return;

			// read bit
			int bit = 0;
			if (cur_live.drw) {
				bit = pll_get_next_bit(cur_live.tm, get_floppy(), limit);
				if(bit < 0)
					return;
			}

			// write bit
			int write_bit = 0;
			if (!cur_live.drw) { // TODO WPS
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
			if (cur_live.drw) {
				if (!sync) {
					cur_live.bit_counter = 0;
				} else if (cur_live.sync) {
					cur_live.bit_counter++;
					if (cur_live.bit_counter == 10) {
						cur_live.bit_counter = 0;
					}
				}
			} else {
				cur_live.bit_counter++;
				if (cur_live.bit_counter == 10) {
					cur_live.bit_counter = 0;
				}
			}

			// sync counter
			if (sync) {
				cur_live.sync_bit_counter = 0;
				cur_live.sync_byte_counter = 10; // TODO 9 in schematics
			} else if (!cur_live.sync) {
				cur_live.sync_bit_counter++;
				if (cur_live.sync_bit_counter == 10) {
					cur_live.sync_bit_counter = 0;
					cur_live.sync_byte_counter++;
					if (cur_live.sync_byte_counter == 16) {
						cur_live.sync_byte_counter = 0;
					}
				}
			}

			// syn
			int syn = !(cur_live.sync_byte_counter == 15);

			// GCR decoder
			if (cur_live.drw) {
				cur_live.i = cur_live.drw << 10 | cur_live.shift_reg;
			} else {
				cur_live.i = cur_live.drw << 10 | 0x200 | ((cur_live.wd & 0xf0) << 1) | cur_live.wrsync << 4 | (cur_live.wd & 0x0f);
			}

			cur_live.e = m_gcr_rom->base()[cur_live.i];

			attotime next = cur_live.tm + m_period;
			if (LOG) logerror("%s:%s cyl %u bit %u sync %u bc %u sr %03x sbc %u sBC %u syn %u i %03x e %02x\n",cur_live.tm.as_string(),next.as_string(),get_floppy()->get_cyl(),bit,sync,cur_live.bit_counter,cur_live.shift_reg,cur_live.sync_bit_counter,cur_live.sync_byte_counter,syn,cur_live.i,cur_live.e);

			// byte ready
			int brdy = !(cur_live.bit_counter == 9);

			// GCR error
			int gcr_err = !(brdy || BIT(cur_live.e, 3));

			if (LOG_BITS) {
				if (cur_live.drw) {
					logerror("%s cyl %u bit %u sync %u bc %u sr %03x i %03x e %02x\n",cur_live.tm.as_string(),get_floppy()->get_cyl(),bit,sync,cur_live.bit_counter,cur_live.shift_reg,cur_live.i,cur_live.e);
				} else {
					logerror("%s cyl %u writing bit %u bc %u sr %03x i %03x e %02x\n",cur_live.tm.as_string(),get_floppy()->get_cyl(),write_bit,cur_live.bit_counter,cur_live.shift_reg_write,cur_live.i,cur_live.e);
				}
			}

			if (!brdy) {
				// load write shift register
				cur_live.shift_reg_write = GCR_ENCODE(cur_live.e, cur_live.i);

				if (LOG) logerror("%s load write shift register %03x\n",cur_live.tm.as_string(),cur_live.shift_reg_write);
			} else {
				// clock write shift register
				cur_live.shift_reg_write <<= 1;
				cur_live.shift_reg_write &= 0x3ff;
			}

			if (brdy != cur_live.brdy) {
				if (LOG) logerror("%s BRDY %u\n", cur_live.tm.as_string(),brdy);
				if (!brdy)
				{
					cur_live.lbrdy_changed = true;
					if (LOG_VIA) logerror("%s LBRDY 0 : %02x\n", cur_live.tm.as_string(), GCR_DECODE(cur_live.e, cur_live.i));
				}
				cur_live.brdy = brdy;
				syncpoint = true;
			}

			if (sync != cur_live.sync) {
				if (LOG) logerror("%s SYNC %u\n", cur_live.tm.as_string(),sync);
				cur_live.sync = sync;
				syncpoint = true;
			}

			if (syn != cur_live.syn) {
				if (LOG) logerror("%s SYN %u\n", cur_live.tm.as_string(),syn);
				cur_live.syn = syn;
				cur_live.syn_changed = true;
				syncpoint = true;
			}

			if (gcr_err != cur_live.gcr_err) {
				if (LOG) logerror("%s GCR ERR %u\n", cur_live.tm.as_string(),gcr_err);
				cur_live.gcr_err = gcr_err;
				syncpoint = true;
			}

			if (syncpoint) {
				live_delay(RUNNING_SYNCPOINT);
				return;
			}
			break;
		}

		case RUNNING_SYNCPOINT: {
			if (cur_live.lbrdy_changed) {
				m_lbrdy_cb(0);
				cur_live.lbrdy_changed = false;
			}

			if (cur_live.syn_changed) {
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

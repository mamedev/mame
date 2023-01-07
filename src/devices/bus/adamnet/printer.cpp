// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam printer emulation

**********************************************************************/

#include "emu.h"
#include "printer.h"


// Run the following code in a web browser to build the typesheet and save the file as artwork/adam_printer.png

/*
<canvas id="canvas" width="1920" height="1080"></canvas>

<script>
const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');

size = 50
ctx.font = ' '+size+'px Courier New';

ctx.fillStyle = "white";
ctx.fillStyle = "#FFFFFE";  // #FFFFFF does not work for transpen
ctx.fillRect(0,0,1920,1080);  // need to make background white

ctx.fillStyle="black";

// cent = 00a2
// not = 00ac
for (i=32;i<128;i++)
{

    str = String.fromCharCode(i)

    if (i==32) str = "¢"
    if (i==127) str = "¬"

    textMetrics = ctx.measureText(str);
    width = textMetrics['actualBoundingBoxRight'] - textMetrics['actualBoundingBoxLeft']

    hsize = 34
    vsize = size

    x0 = 35
    y0 = 14

    x1 = (Math.floor((i-32) % 16) ) * hsize + x0
    x2 = x1 + hsize
    y1 = (Math.floor((i-32) / 16) ) * vsize + y0
    y2 = y1 + vsize

    ctx.fillText(
        str,
        x1 - width / 2 + hsize / 2,
        y1 + vsize - 15);

    drawredlines = false
    if (drawredlines)
    {
        ctx.strokeStyle = 'red';
        ctx.beginPath();
        ctx.moveTo( x1, y1 ); ctx.lineTo( x1, y2 ); ctx.lineTo( x2, y2 ); ctx.lineTo( x2, y1 ); ctx.lineTo( x1, y1 );
        ctx.stroke();
}
}

function fillstylepad(x)
{
    num = "000000" + x.toString(16); num = num.slice(num.length-6, num.length);
    ctx.fillStyle="#"+num;
}

ctx.fillStyle = fillstylepad(x0)
ctx.fillRect(0,0,1,1)
ctx.fillStyle = fillstylepad(y0)
ctx.fillRect(1,0,1,1)
ctx.fillStyle = fillstylepad(hsize)
ctx.fillRect(2,0,1,1)
ctx.fillStyle = fillstylepad(vsize)
ctx.fillRect(3,0,1,1)

</script>
*/


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define M6801_TAG       "u2"

#define PAPER_WIDTH  300 * 8.53  // 300 dpi * 8.5 inches
#define PAPER_HEIGHT 300 * 11   // 300 dpi * 11 inches

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ADAM_PRN, adam_printer_device, "adam_prn", "Adam printer")


//-------------------------------------------------
//  ROM( adam_prn )
//-------------------------------------------------

ROM_START( adam_prn )
	ROM_REGION( 0x800, M6801_TAG, 0 )
	ROM_LOAD( "printer.u2", 0x000, 0x800, CRC(e8db783b) SHA1(32b40679749ad0317c2c9ee9ca619fad6d850ce7) )
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *adam_printer_device::device_rom_region() const
{
	return ROM_NAME( adam_prn );
}


//-------------------------------------------------
//  ADDRESS_MAP( adam_prn_mem )
//-------------------------------------------------

void adam_printer_device::adam_prn_mem(address_map &map)
{
	map(0x0000, 0x001f).m(M6801_TAG, FUNC(m6801_cpu_device::m6801_io));
	map(0x0080, 0x00ff).ram();
	map(0xf800, 0xffff).rom().region(M6801_TAG, 0);
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void adam_printer_device::device_add_mconfig(machine_config &config)
{
	M6801(config, m_maincpu, XTAL(4'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &adam_printer_device::adam_prn_mem);
	m_maincpu->out_p1_cb().set(FUNC(adam_printer_device::p1_w));
	m_maincpu->in_p2_cb().set(FUNC(adam_printer_device::p2_r));
	m_maincpu->out_p2_cb().set(FUNC(adam_printer_device::p2_w));
	m_maincpu->in_p3_cb().set(FUNC(adam_printer_device::p3_r));
	m_maincpu->in_p4_cb().set(FUNC(adam_printer_device::p4_r));
	m_maincpu->out_p4_cb().set(FUNC(adam_printer_device::p4_w));
	//m_maincpu->set_disable(); // TODO

	BITMAP_PRINTER(config, m_bitmap_printer, PAPER_WIDTH, PAPER_HEIGHT, 300, 300);  // do 300 dpi
	m_bitmap_printer->set_pf_stepper_ratio(3,1);  // 3 pixels per step
	m_bitmap_printer->set_cr_stepper_ratio(8,1);  // 8 pixels per step

	STEPPER(config, m_daisywheel_stepper);
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  adam_printer_device - constructor
//-------------------------------------------------

adam_printer_device::adam_printer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ADAM_PRN, tag, owner, clock),
		device_adamnet_card_interface(mconfig, *this),
		m_maincpu(*this, M6801_TAG),
		m_bitmap_printer(*this, "bitmap_printer"),
		m_daisywheel_stepper(*this, "daisywheel")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void adam_printer_device::device_start()
{
	m_platen_motor_timer = timer_alloc(FUNC(adam_printer_device::platen_motor_timer), this);
	m_platen_motor_timer->adjust(attotime::from_double(1.0/2400));

	std::string fullname;
	std::error_condition filerr;
	util::core_file::ptr file;

	/* get the filename for the image */
	fullname = "artwork/adam_printer.png";
	/* open the file */
	filerr = util::core_file::open(fullname, OPEN_FLAG_READ, file);

	/* if that worked, load the file */
	if (!filerr)
	{
			util::png_read_bitmap(*file, m_typesheet);
			file.reset();
	}

}


//-------------------------------------------------
//  adamnet_reset_w -
//-------------------------------------------------

void adam_printer_device::adamnet_reset_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, state);
}


//-------------------------------------------------
//  p1_w -
//-------------------------------------------------

void adam_printer_device::p1_w(uint8_t data)
{
	/*

	    bit     description

	    0       M2 phase D   0-3 is M2 carriage motor
	    1       M2 phase B
	    2       M2 phase C
	    3       M2 phase A
	    4       M1 phase B   4-7 is M1 daisy wheel motor
	    5       M1 phase D
	    6       M1 phase A
	    7       M1 phase C

	*/

	//printf("p1 = %x    cr_stepper=%x  daisywheel=%d\n",data, m_bitmap_printer->m_xpos, m_daisywheel_stepper->get_absolute_position());

	m_bitmap_printer->update_cr_stepper(bitswap<4>(data, 1, 3, 2, 0));

	m_daisywheel_stepper->update(bitswap<4>(data, 5, 7, 6, 4));

	// wrap daisywheel around
	if (m_daisywheel_stepper->get_absolute_position() >= 96 * 2)
		m_daisywheel_stepper->set_absolute_position(m_daisywheel_stepper->get_absolute_position() - 96 * 2);

	if (m_daisywheel_stepper->get_absolute_position() < 0)
		m_daisywheel_stepper->set_absolute_position(m_daisywheel_stepper->get_absolute_position() + 96 * 2);
}


//-------------------------------------------------
//  p2_r -
//-------------------------------------------------

uint8_t adam_printer_device::p2_r()
{
	/*

	    bit     description

	    0       mode bit 0
	    1       mode bit 1
	    2       mode bit 2
	    3       NET RXD
	    4       NET TXD

	*/

	uint8_t data = M6801_MODE_7;

	// NET RXD
	data |= m_bus->rxd_r(this) << 3;

	return data;
}


//-------------------------------------------------
//  p2_w -
//-------------------------------------------------

void adam_printer_device::p2_w(uint8_t data)
{
	/*

	    bit     description

	    0       mode bit 0
	    1       mode bit 1
	    2       mode bit 2
	    3       NET RXD
	    4       NET TXD

	*/

	m_bus->txd_w(this, BIT(data, 4));
}


//-------------------------------------------------
//  p3_r -
//-------------------------------------------------

uint8_t adam_printer_device::p3_r()
{
	return 0xff;
}


//-------------------------------------------------
//  p4_r -
//-------------------------------------------------

uint8_t adam_printer_device::p4_r()
{
	/*
	    bit     description

	    0
	    1
	    2
	    3
	    4       left margin
	    5       platen detent
	    6       wheel home
	    7       self-test
	*/

	return  0x80 |  // make this 0x0 for self test  (active low = self test)
			(m_bitmap_printer->m_xpos < 4)                         << 4 |  // 1 if we are at left margin
			(m_platen_counter > 72 && m_platen_counter < 144)      << 5 |  // 1 is open, 0 is closed
			(m_daisywheel_stepper->get_absolute_position() == wheel_home_sensor_pos) << 6;
}


//-------------------------------------------------
//  p4_w -
//-------------------------------------------------

void adam_printer_device::p4_w(uint8_t data)
{
	/*
	    bit     description

	    0       print hammer solenoid    // active low
	    1       ribbon advance solenoid  // active low
	    2       platen motor advance  0 = motor on
	    3       platen motor break    1 = brake is off
	    4
	    5
	    6
	    7
	*/
	m_p4_data = data;
}


TIMER_CALLBACK_MEMBER(adam_printer_device::platen_motor_timer)
{
	// main timer interrupt gets called every 1/2400 second

	static const uint8_t drivetable[] = {12, 9, 3, 6};

	if (!BIT(m_p4_data, 2) && (BIT(m_p4_data, 3))) // motor = 0, brake = 1
	{
		m_platen_counter++;  // simulates motor position, update in 1/2400 increments
		const int platen_counter_divisor = 36;
		m_bitmap_printer->update_pf_stepper(bitswap<4>(drivetable[(m_platen_counter / platen_counter_divisor) % 4], 1, 3, 2, 0));
		m_platen_counter %= 144;
	}

	if (!BIT(m_p4_data, 0))  // active low solenoid engaged
	{
		const int wheelpos = mod_positive(m_daisywheel_stepper->get_absolute_position() / 2 + wheel_offset(), 96);
		int wheelchar = m_daisywheel.at(wheelpos);


		if (m_typesheet.width() != 0)
		{
			int horigin = ((u32 *) m_typesheet.raw_pixptr(0,0))[0] & 0xffffff;
			int vorigin = ((u32 *) m_typesheet.raw_pixptr(0,0))[1] & 0xffffff;
			int hsize =   ((u32 *) m_typesheet.raw_pixptr(0,0))[2] & 0xffffff;
			int vsize =   ((u32 *) m_typesheet.raw_pixptr(0,0))[3] & 0xffffff;
	/*
	        int horigin = 35;
	        int vorigin = 14;
	        int vsize = 50;  // at 300dpi 66 lines = 6 lpi = 50 pixels vert
	        int hsize = 34;  // at 300dpi 80 char = 10 cpi = 30 pixels horiz
	*/
			int i = wheelchar;

			int srcx1 = hsize * ((i-32) % 16) + horigin;
			int srcy1 = vsize * ((i-32) / 16) + vorigin;

			int destx1 = m_bitmap_printer->m_xpos;
			int destx2 = destx1 + hsize - 1;
			if (destx2 > m_bitmap_printer->get_bitmap().width()) destx2 = m_bitmap_printer->get_bitmap().width();
			int desty1 = m_bitmap_printer->m_ypos;
			int desty2 = desty1 + vsize;
			if (desty2 > m_bitmap_printer->get_bitmap().height()) desty2 = m_bitmap_printer->get_bitmap().height();

			rectangle m_clip(destx1, destx2, desty1, desty2);
			copybitmap_trans(m_bitmap_printer->get_bitmap(), (bitmap_rgb32&) m_typesheet, 0, 0,
				destx1 - srcx1, desty1 - srcy1, m_clip, 0xfffffffe);  // FFFFFFFF treated as no transparency special case
		}
	}

	m_platen_motor_timer->adjust(attotime::from_double(1.0/2400));


}

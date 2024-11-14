// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    coco_ssc.cpp

    Code for emulating the CoCo Speech / Sound Cartridge

    The SSC was a complex sound cartridge. A TMS-7040 microcontroller,
    SPO256-AL2 speech processor, AY-3-8913 programable sound generator, and
    2K of RAM.

    All four ports of the microcontroller are under software control.

    Port A is input from the host CPU.
    Port B is A0-A7 for the 2k of RAM.
    Port C is the internal bus controller:
        bit 7 6 5 4 3 2 1 0
            | | | | | | | |
            | | | | | | | + A8 for RAM and BC1 of AY3-8913
            | | | | | | +-- A9 for RAM
            | | | | | +---- A10 for RAM
            | | | | +------ R/W* for RAM and BDIR for AY3-8913
            | | | +-------- CS* for RAM
            | | +---------- ALD* for SP0256
            | +------------ CS* for AY3-8913
            +-------------- BUSY* for host CPU (connects to a latch)
        * – Active low
    Port D is the 8-bit data bus.

***************************************************************************/

#include "emu.h"
#include "coco_ssc.h"

#include "cpu/tms7000/tms7000.h"
#include "machine/ram.h"
#include "sound/ay8910.h"
#include "sound/sp0256.h"

#include "speaker.h"

#define LOG_INTERFACE   (1U << 1)
#define LOG_INTERNAL    (1U << 2)
#define VERBOSE (0)
// #define VERBOSE (LOG_INTERFACE)
// #define VERBOSE (LOG_INTERFACE|LOG_INTERNAL)

#include "logmacro.h"

#define LOGINTERFACE(...) LOGMASKED(LOG_INTERFACE, __VA_ARGS__)
#define LOGINTERNAL(...) LOGMASKED(LOG_INTERNAL, __VA_ARGS__)

#define PIC_TAG "pic7040"
#define AY_TAG "cocossc_ay"
#define SP0256_TAG "sp0256"

#define SP0256_GAIN 1.75
#define AY8913_GAIN 2.0

#define C_A8   0x01
#define C_BC1  0x01
#define C_A9   0x02
#define C_A10  0x04
#define C_RRW  0x08
#define C_BDR  0x08
#define C_RCS  0x10
#define C_ALD  0x20
#define C_ACS  0x40
#define C_BSY  0x80

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

namespace
{
	class cocossc_sac_device;

	// ======================> coco_ssc_device

	class coco_ssc_device :
			public device_t,
			public device_cococart_interface
	{
	public:
		// construction/destruction
		coco_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
		virtual void device_reset() override ATTR_COLD;

		u8 ssc_port_a_r();
		void ssc_port_b_w(u8 data);
		u8 ssc_port_c_r();
		void ssc_port_c_w(u8 data);
		u8 ssc_port_d_r();
		void ssc_port_d_w(u8 data);

	protected:
		// device-level overrides
		virtual void device_start() override ATTR_COLD;
		u8 ff7d_read(offs_t offset);
		void ff7d_write(offs_t offset, u8 data);
		virtual void set_sound_enable(bool sound_enable) override;

	private:
		u8                                      m_reset_line;
		bool                                    m_tms7000_busy;
		u8                                      m_tms7000_porta;
		u8                                      m_tms7000_portb;
		u8                                      m_tms7000_portc;
		u8                                      m_tms7000_portd;
		required_device<tms7040_device>         m_tms7040;
		required_device<ram_device>             m_staticram;
		required_device<ay8910_device>          m_ay;
		required_device<sp0256_device>          m_spo;
		required_device<cocossc_sac_device>     m_sac;
	};

	// ======================> Color Computer Sound Activity Circuit filter

	class cocossc_sac_device : public device_t,
		public device_sound_interface
	{
	public:
		cocossc_sac_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
		~cocossc_sac_device() { }
		bool sound_activity_circuit_output();

	protected:
		// device-level overrides
		virtual void device_start() override ATTR_COLD;

		// sound stream update overrides
		virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

		// Power of 2
		static constexpr int BUFFER_SIZE = 4;
	private:
		sound_stream*  m_stream;
		float m_rms[BUFFER_SIZE];
		int m_index;
	};
};


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(COCO_SSC, device_cococart_interface, coco_ssc_device, "coco_ssc", "CoCo S/SC PAK");
DEFINE_DEVICE_TYPE(COCOSSC_SAC, cocossc_sac_device, "cocossc_sac", "CoCo SSC Sound Activity Circuit");


//**************************************************************************
//  MACHINE FRAGMENTS AND ADDRESS MAPS
//**************************************************************************

void coco_ssc_device::device_add_mconfig(machine_config &config)
{
	TMS7040(config, m_tms7040, DERIVED_CLOCK(2, 1));
	m_tms7040->in_porta().set(FUNC(coco_ssc_device::ssc_port_a_r));
	m_tms7040->out_portb().set(FUNC(coco_ssc_device::ssc_port_b_w));
	m_tms7040->in_portc().set(FUNC(coco_ssc_device::ssc_port_c_r));
	m_tms7040->out_portc().set(FUNC(coco_ssc_device::ssc_port_c_w));
	m_tms7040->in_portd().set(FUNC(coco_ssc_device::ssc_port_d_r));
	m_tms7040->out_portd().set(FUNC(coco_ssc_device::ssc_port_d_w));

	RAM(config, "staticram").set_default_size("2K").set_default_value(0);

	SPEAKER(config, "ssc_audio").front_center();

	SP0256(config, m_spo, XTAL(3'120'000));
	m_spo->add_route(ALL_OUTPUTS, "ssc_audio", SP0256_GAIN);
	m_spo->data_request_callback().set_inputline(m_tms7040, TMS7000_INT1_LINE);

	AY8913(config, m_ay, DERIVED_CLOCK(2, 1));
	m_ay->set_flags(AY8910_SINGLE_OUTPUT);
	m_ay->add_route(ALL_OUTPUTS, "coco_sac_tag", AY8913_GAIN);

	COCOSSC_SAC(config, m_sac, DERIVED_CLOCK(2, 1));
	m_sac->add_route(ALL_OUTPUTS, "ssc_audio", 1.0);
}

ROM_START(coco_ssc)
	ROM_REGION(0x1000, PIC_TAG, 0)
	ROM_LOAD("pic-7040-510.bin", 0x0000, 0x1000, CRC(a8e2eb98) SHA1(7c17dcbc21757535ce0b3a9e1ce5ca61319d3606)) // pic7040 cpu rom
	ROM_REGION(0x10000, SP0256_TAG, 0)
	ROM_LOAD("sp0256-al2.bin", 0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc))
ROM_END

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  coco_ssc_device - constructor
//-------------------------------------------------

coco_ssc_device::coco_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, COCO_SSC, tag, owner, clock),
		device_cococart_interface(mconfig, *this ),
		m_tms7040(*this, PIC_TAG),
		m_staticram(*this, "staticram"),
		m_ay(*this, AY_TAG),
		m_spo(*this, SP0256_TAG),
		m_sac(*this, "coco_sac_tag")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_ssc_device::device_start()
{
	// install $ff7d-e handler
	install_readwrite_handler(0xff7d, 0xff7e,
			read8sm_delegate(*this, FUNC(coco_ssc_device::ff7d_read)),
			write8sm_delegate(*this, FUNC(coco_ssc_device::ff7d_write)));

	save_item(NAME(m_reset_line));
	save_item(NAME(m_tms7000_busy));
	save_item(NAME(m_tms7000_porta));
	save_item(NAME(m_tms7000_portb));
	save_item(NAME(m_tms7000_portc));
	save_item(NAME(m_tms7000_portd));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_ssc_device::device_reset()
{
	m_reset_line = 1;
	m_tms7000_busy = false;
}


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *coco_ssc_device::device_rom_region() const
{
	return ROM_NAME( coco_ssc );
}


//-------------------------------------------------
//  set_sound_enable
//-------------------------------------------------

void coco_ssc_device::set_sound_enable(bool sound_enable)
{
	if( sound_enable )
	{
		m_sac->set_output_gain(0, 1.0);
		m_spo->set_output_gain(0, 1.0);
	}
	else
	{
		m_sac->set_output_gain(0, 0.0);
		m_spo->set_output_gain(0, 0.0);
	}
}

//-------------------------------------------------
//  ff7d_read
//-------------------------------------------------

u8 coco_ssc_device::ff7d_read(offs_t offset)
{
	u8 data = 0xff;

	switch(offset)
	{
		case 0x00:
			data = 0xff;
			LOGINTERFACE( "[%s] ff7d read: %02x\n", machine().describe_context(), data );
			break;

		case 0x01:
			data = 0x1f;

			if( m_tms7000_busy == false )
			{
				data |= 0x80;
			}

			if( m_spo->sby_r() )
			{
				data |= 0x40;
			}

			if(  m_sac->sound_activity_circuit_output() )
			{
				data |= 0x20;
			}

			LOGINTERFACE( "[%s] ff7e read: %c%c%c%c %c%c%c%c (%02x)\n",
					machine().describe_context(),
					data & 0x80 ? 'b' : 'B',
					data & 0x40 ? 's' : 'S',
					data & 0x20 ? 'p' : 'P',
					data & 0x10 ? '1' : '0',
					data & 0x08 ? '1' : '0',
					data & 0x04 ? '1' : '0',
					data & 0x02 ? '1' : '0',
					data & 0x01 ? '1' : '0',
					data );
			break;
	}

	return data;
}


//-------------------------------------------------
//  ff7d_write
//-------------------------------------------------

void coco_ssc_device::ff7d_write(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0x00:
			LOGINTERFACE( "[%s] ff7d write: %02x\n", machine().describe_context(), data );

			if( (data & 1) == 1 )
			{
				m_spo->reset();
			}

			if( ((m_reset_line & 1) == 1) && ((data & 1) == 0) )
			{
				m_tms7040->reset();
				m_ay->reset();
				m_tms7000_busy = false;
			}

			m_reset_line = data;
			break;

		case 0x01:
			LOGINTERFACE( "[%s] ff7e write: %02x\n", machine().describe_context(), data );
			m_tms7000_porta = data;
			m_tms7000_busy = true;
			m_tms7040->set_input_line(TMS7000_INT3_LINE, ASSERT_LINE);
			break;
	}
}


//-------------------------------------------------
//  Handlers for secondary CPU ports
//-------------------------------------------------

u8 coco_ssc_device::ssc_port_a_r()
{
	LOGINTERNAL( "[%s] port a read: %02x\n", machine().describe_context(), m_tms7000_porta );

	if( !machine().side_effects_disabled() )
	{
		m_tms7040->set_input_line(TMS7000_INT3_LINE, CLEAR_LINE);
	}

	return m_tms7000_porta;
}

void coco_ssc_device::ssc_port_b_w(u8 data)
{
	LOGINTERNAL( "[%s] port b write: %02x\n", machine().describe_context(), data );

	m_tms7000_portb = data;
}

u8 coco_ssc_device::ssc_port_c_r()
{
	LOGINTERNAL( "[%s] port c read: %02x\n", machine().describe_context(), m_tms7000_portc );

	return m_tms7000_portc;
}

void coco_ssc_device::ssc_port_c_w(u8 data)
{
	if( (data & C_RCS) == 0 && (data & C_RRW) == 0 ) /* static RAM write */
	{
		u16 address = u16(data) << 8;
		address += m_tms7000_portb;
		address &= 0x7ff;

		m_staticram->write(address, m_tms7000_portd);
	}

	if( (data & C_ACS) == 0 ) /* chip select for AY-3-8913 */
	{
		if( (data & (C_BDR|C_BC1)) == (C_BDR|C_BC1) ) /* BDIR = 1, BC1 = 1: latch address */
		{
			m_ay->address_w(m_tms7000_portd);
		}

		if( ((data & C_BDR) == C_BDR) && ((data & C_BC1) == 0) ) /* BDIR = 1, BC1 = 0: write data */
		{
			m_ay->data_w(m_tms7000_portd);
		}
	}

	if( ((m_tms7000_portc & C_ALD) == C_ALD) && ((data & C_ALD) == 0) && (m_tms7000_portd < 64) )
	{
		m_spo->ald_w(m_tms7000_portd); /* load allophone */
	}

	if( ((m_tms7000_portc & C_BSY) == 0) && ((data & C_BSY) == C_BSY) )
	{
		m_tms7000_busy = false;
	}

	LOGINTERNAL( "[%s] port c write: %c%c%c%c %c%c%c%c (%02x)\n",
			machine().describe_context(),
			data & 0x80 ? '.' : 'B',
			data & 0x40 ? '.' : 'P',
			data & 0x20 ? '.' : 'V',
			data & 0x10 ? '.' : 'R',
			data & 0x40 ? (data & 0x08 ? 'R' : 'W') : (data & 0x08 ? 'D' : '.'),
			data & 0x04 ? '1' : '0',
			data & 0x02 ? '1' : '0',
			data & 0x40 ? (data & 0x01 ? '1' : '0') : (data & 0x01 ? 'C' : '.'),
			data );

	m_tms7000_portc = data;
}

u8 coco_ssc_device::ssc_port_d_r()
{
	if( ((m_tms7000_portc & C_RCS) == 0) && ((m_tms7000_portc & C_ACS) == 0) )
		logerror( "[%s] Warning: Reading RAM and PSG at the same time!\n", machine().describe_context() );

	if( ((m_tms7000_portc & C_RCS) == 0)  && ((m_tms7000_portc & C_RRW) == C_RRW)) /* static ram chip select (low) and static ram chip read (high) */
	{
		u16 address = u16(m_tms7000_portc) << 8;
		address += m_tms7000_portb;
		address &= 0x7ff;

		m_tms7000_portd = m_staticram->read(address);
	}

	if( (m_tms7000_portc & C_ACS) == 0 ) /* chip select for AY-3-8913 */
	{
		if( ((m_tms7000_portc & C_BDR) == 0) && ((m_tms7000_portc & C_BC1) == C_BC1) ) /* psg read data */
		{
			m_tms7000_portd = m_ay->data_r();
		}
	}

	LOGINTERNAL( "[%s] port d read: %02x\n", machine().describe_context(), m_tms7000_portd );

	return m_tms7000_portd;
}

void coco_ssc_device::ssc_port_d_w(u8 data)
{
	LOGINTERNAL( "[%s] port d write: %02x\n", machine().describe_context(), data );

	m_tms7000_portd = data;
}


//-------------------------------------------------
//  cocossc_sac_device - constructor
//-------------------------------------------------

cocossc_sac_device::cocossc_sac_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, COCOSSC_SAC, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_index(0)
{
	std::fill(std::begin(m_rms), std::end(m_rms), 0.0f);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cocossc_sac_device::device_start()
{
	m_stream = stream_alloc(1, 1, machine().sample_rate());
}


//-------------------------------------------------
//  sound_stream_update - handle a stream update
//-------------------------------------------------

void cocossc_sac_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	auto &src = inputs[0];
	auto &dst = outputs[0];

	int count = dst.samples();
	m_rms[m_index] = 0;

	if( count > 0 )
	{
		for( int sampindex = 0; sampindex < count; sampindex++ )
		{
			auto source_sample = src.get(sampindex);
			m_rms[m_index] += source_sample * source_sample;
			dst.put(sampindex, source_sample);
		}

		m_rms[m_index] = m_rms[m_index] / count;
		m_rms[m_index] = sqrt(m_rms[m_index]);
	}

	m_index++;
	m_index &= (BUFFER_SIZE-1);
}


//-------------------------------------------------
//  sound_activity_circuit_output - making sound
//-------------------------------------------------

bool cocossc_sac_device::sound_activity_circuit_output()
{
	float sum = std::accumulate(std::begin(m_rms), std::end(m_rms), 0.0f);
	float average = (sum / BUFFER_SIZE);

	return average < 0.317f;
}

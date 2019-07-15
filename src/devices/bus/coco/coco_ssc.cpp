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
#include "machine/netlist.h"
#include "machine/ram.h"
#include "netlist/devices/net_lib.h"
#include "sound/ay8910.h"
#include "sound/sp0256.h"

#include "speaker.h"

#define LOG_SSC 0
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
		coco_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override;
		virtual void device_add_mconfig(machine_config &config) override;
		virtual void device_reset() override;

		DECLARE_READ8_MEMBER(ssc_port_a_r);
		DECLARE_WRITE8_MEMBER(ssc_port_b_w);
		DECLARE_READ8_MEMBER(ssc_port_c_r);
		DECLARE_WRITE8_MEMBER(ssc_port_c_w);
		DECLARE_READ8_MEMBER(ssc_port_d_r);
		DECLARE_WRITE8_MEMBER(ssc_port_d_w);

	protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
		virtual DECLARE_READ8_MEMBER(ff7d_read);
		virtual DECLARE_WRITE8_MEMBER(ff7d_write);
		virtual void set_sound_enable(bool sound_enable) override;
		static constexpr device_timer_id BUSY_TIMER_ID  = 0;

	private:
		uint8_t                                 m_reset_line;
		bool                                    m_tms7000_busy;
		uint8_t                                 m_tms7000_porta;
		uint8_t                                 m_tms7000_portb;
		uint8_t                                 m_tms7000_portc;
		uint8_t                                 m_tms7000_portd;
		emu_timer                               *m_tms7000_busy_timer;
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
		cocossc_sac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
		~cocossc_sac_device() { }
		bool sound_activity_circuit_output();

	protected:
		// device-level overrides
		virtual void device_start() override;

		// sound stream update overrides
		virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	private:
		sound_stream*  m_stream;
		double m_rms[16];
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

coco_ssc_device::coco_ssc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
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
	// install $FF7D-E handler
	write8_delegate wh = write8_delegate(FUNC(coco_ssc_device::ff7d_write), this);
	read8_delegate rh = read8_delegate(FUNC(coco_ssc_device::ff7d_read), this);
	install_readwrite_handler(0xFF7D, 0xFF7E, rh, wh);

	save_item(NAME(m_reset_line));
	save_item(NAME(m_tms7000_busy));
	save_item(NAME(m_tms7000_porta));
	save_item(NAME(m_tms7000_portb));
	save_item(NAME(m_tms7000_portc));
	save_item(NAME(m_tms7000_portd));

	m_tms7000_busy_timer = timer_alloc(BUSY_TIMER_ID);

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_ssc_device::device_reset()
{
	m_reset_line = 0;
	m_tms7000_busy = false;
}


//-------------------------------------------------
//  device_timer - handle timer callbacks
//-------------------------------------------------

void coco_ssc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
		case BUSY_TIMER_ID:
			m_tms7000_busy = false;
			m_tms7000_busy_timer->adjust(attotime::never);
			break;

		default:
			break;

	}
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

READ8_MEMBER(coco_ssc_device::ff7d_read)
{
	uint8_t data = 0xff;

	switch(offset)
	{
		case 0x00:
			data = 0xff;

			if (LOG_SSC)
			{
				logerror( "[%s] ff7d read: %02x\n", machine().describe_context(), data );
			}
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

			if( ! m_sac->sound_activity_circuit_output() )
			{
				data |= 0x20;
			}

			if (LOG_SSC)
			{
				logerror( "[%s] ff7e read: %c%c%c%c %c%c%c%c (%02x)\n",
					machine().describe_context(),
					data & 0x80 ? '.' : 'B',
					data & 0x40 ? '.' : 'S',
					data & 0x20 ? '.' : 'P',
					data & 0x10 ? '.' : '1',
					data & 0x08 ? '.' : '1',
					data & 0x04 ? '.' : '1',
					data & 0x02 ? '.' : '1',
					data & 0x01 ? '.' : '1',
					data );
			}
			break;
	}

	return data;
}


//-------------------------------------------------
//  ff7d_write
//-------------------------------------------------

WRITE8_MEMBER(coco_ssc_device::ff7d_write)
{
	switch(offset)
	{
		case 0x00:
			if (LOG_SSC)
			{
				logerror( "[%s] ff7d write: %02x\n", machine().describe_context(), data );
			}

			if( (m_reset_line & 1) == 1 )
			{
				if( (data & 1) == 0 )
				{
					m_tms7040->reset();
					m_ay->reset();
					m_spo->reset();
					m_tms7000_busy = false;
				}
			}

			m_reset_line = data;
			break;

		case 0x01:

			if (LOG_SSC)
			{
				logerror( "[%s] ff7e write: %02x\n", machine().describe_context(), data );
			}

			m_tms7000_porta = data;
			m_tms7000_busy = true;
			m_tms7040->set_input_line(TMS7000_INT3_LINE, ASSERT_LINE);
			break;
	}
}


//-------------------------------------------------
//  Handlers for secondary CPU ports
//-------------------------------------------------

READ8_MEMBER(coco_ssc_device::ssc_port_a_r)
{
	if (LOG_SSC)
	{
		logerror( "[%s] port a read: %02x\n", machine().describe_context(), m_tms7000_porta );
	}

	m_tms7040->set_input_line(TMS7000_INT3_LINE, CLEAR_LINE);

	return m_tms7000_porta;
}

WRITE8_MEMBER(coco_ssc_device::ssc_port_b_w)
{
	if (LOG_SSC)
	{
		logerror( "[%s] port b write: %02x\n", machine().describe_context(), data );
	}

	m_tms7000_portb = data;
}

READ8_MEMBER(coco_ssc_device::ssc_port_c_r)
{
	if (LOG_SSC)
	{
		logerror( "[%s] port c read: %02x\n", machine().describe_context(), m_tms7000_portc );
	}

	return m_tms7000_portc;
}

WRITE8_MEMBER(coco_ssc_device::ssc_port_c_w)
{
	if( (data & C_RCS) == 0 && (data & C_RRW) == 0) /* static RAM write */
	{
		uint16_t address = (uint16_t)data << 8;
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

	if( (data & C_ALD) == 0 )
	{
		m_spo->ald_w(m_tms7000_portd);
	}

	if( ((m_tms7000_portc & C_BSY) == 0) && ((data & C_BSY) == C_BSY) )
	{
		m_tms7000_busy_timer->adjust(attotime::from_usec(1800));
	}

	if (LOG_SSC)
	{
		logerror( "[%s] port c write: %c%c%c%c %c%c%c%c (%02x)\n",
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
	}

	m_tms7000_portc = data;
}

READ8_MEMBER(coco_ssc_device::ssc_port_d_r)
{
	if( ((m_tms7000_portc & C_RCS) == 0) && ((m_tms7000_portc & C_ACS) == 0))
		logerror( "[%s] Warning: Reading RAM and PSG at the same time!\n", machine().describe_context() );

	if( ((m_tms7000_portc & C_RCS) == 0)  && ((m_tms7000_portc & C_RRW) == C_RRW)) /* static ram chip select (low) and static ram chip read (high) */
	{
		uint16_t address = (uint16_t)m_tms7000_portc << 8;
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

	if (LOG_SSC)
	{
		logerror( "[%s] port d read: %02x\n", machine().describe_context(), m_tms7000_portd );
	}

	return m_tms7000_portd;
}

WRITE8_MEMBER(coco_ssc_device::ssc_port_d_w)
{
	if (LOG_SSC)
	{
		logerror( "[%s] port d write: %02x\n", machine().describe_context(), data );
	}

	m_tms7000_portd = data;
}


//-------------------------------------------------
//  cocossc_sac_device - constructor
//-------------------------------------------------

cocossc_sac_device::cocossc_sac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, COCOSSC_SAC, tag, owner, clock),
		device_sound_interface(mconfig, *this),
		m_stream(nullptr),
		m_index(0)
{
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

void cocossc_sac_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples)
{
	stream_sample_t *src = inputs[0];
	stream_sample_t *dst = outputs[0];

	double n = samples;

	while (samples--)
	{
		m_rms[m_index] += ( (double)*src * (double)*src );
		*dst++ = (*src++);
	}

	m_rms[m_index] = m_rms[m_index] / n;
	m_rms[m_index] = sqrt(m_rms[m_index]);

	m_index++;
	m_index &= 0x0f;
}


//-------------------------------------------------
//  sound_activity_circuit_output - making sound
//-------------------------------------------------

bool cocossc_sac_device::sound_activity_circuit_output()
{
  double average = m_rms[0] + m_rms[1] + m_rms[2] + m_rms[3] + m_rms[4] +
	m_rms[5] + m_rms[6] + m_rms[7] + m_rms[8] + m_rms[9] + m_rms[10] +
	m_rms[11] + m_rms[12] + m_rms[13] + m_rms[14] + m_rms[15];

	average /= 16.0;

	if( average > 10400.0 )
		return true;

	return false;
}

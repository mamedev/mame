/*********************************************************************

    er59256.c

    Microchip ER59256 serial eeprom.


*********************************************************************/

#include "emu.h"
#include "er59256.h"

/* LOGLEVEL 0=no logging, 1=just commands and data, 2=everything ! */

#define LOGLEVEL            0

#define LOG(level, ...)     if(LOGLEVEL>=level) logerror(__VA_ARGS__)
#define LOG_BITS(bits)      logerror("CS=%d CK=%d DI=%d DO=%d", (bits&CS_MASK) ? 1 : 0, (bits&CK_MASK) ? 1 : 0, (bits&DI_MASK) ? 1 : 0, (bits&DO_MASK) ? 1 : 0)

const device_type ER59256 = &device_creator<er59256_device>;

er59256_device::er59256_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ER59256, "Microchip ER59256 serial eeprom.", tag, owner, clock, "er59256", __FILE__),
	m_io_bits(0),
	m_old_io_bits(0),
	m_in_shifter(0),
	m_out_shifter(0),
	m_bitcount(0),
	m_command(0),
	m_flags(0)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void er59256_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void er59256_device::device_start()
{
	// Start with rom defaulted to erased
	memset(&m_eerom, 0xFF, EEROM_WORDS*2);

	m_command=CMD_INVALID;

	m_flags&= ~FLAG_DATA_LOADED;

	save_item(NAME(m_eerom));
	save_item(NAME(m_io_bits));
	save_item(NAME(m_old_io_bits));
	save_item(NAME(m_in_shifter));
	save_item(NAME(m_out_shifter));
	save_item(NAME(m_bitcount));
	save_item(NAME(m_command));
	save_item(NAME(m_flags));
}

//-------------------------------------------------
//  device_stop - device-specific stop
//-------------------------------------------------

void er59256_device::device_stop()
{
	/* Save contents of eerom */
}

/***************************************************************************
    IMPLEMENTATION
***************************************************************************/

void er59256_device::preload_rom(const UINT16 *rom_data, int count)
{
	int WordNo;

	logerror("Preloading %d words of data\n",count);

	if(count>EEROM_WORDS)
		memcpy(&m_eerom,rom_data,count*2);
	else
		memcpy(&m_eerom,rom_data,EEROM_WORDS*2);

	for(WordNo=0;WordNo<EEROM_WORDS;WordNo++)
		logerror("%04X ",m_eerom[WordNo]);

	logerror("\n");
}

UINT8 er59256_device::data_loaded()
{
	return (m_flags & FLAG_DATA_LOADED) ? 1 : 0;
}

void er59256_device::set_iobits(UINT8 newbits)
{
	//UINT32  bit;

	// Make sure we only apply valid bits
	newbits&=ALL_MASK;

	if(LOGLEVEL>1)
	{
		logerror("er59256:newbits=%02X : ",newbits);
		LOG_BITS(newbits);
		logerror(" io_bits=%02X : ",m_io_bits);
		LOG_BITS(m_io_bits);
		logerror(" old_io_bits=%02X : ",m_old_io_bits);
		LOG_BITS(m_old_io_bits);
		logerror(" bitcount=%d, in_shifter=%04X, out_shifter=%05X, flags=%02X\n",m_bitcount,m_in_shifter,m_out_shifter,m_flags);
	}
	// Only do anything if the inputs have changed
	if((newbits&IN_MASK)!=(m_io_bits&IN_MASK))
	{
		// save the current state, then set the new one, remembering to preserve data out
		m_old_io_bits=m_io_bits;
		m_io_bits=(newbits & ~DO_MASK) | (m_old_io_bits&DO_MASK);

		if(CS_RISE())
		{
			m_flags&=~FLAG_START_BIT;
			m_command=CMD_INVALID;
		}

		if(LOGLEVEL>1)
		{
			if(CK_RISE()) logerror("er59256:CK rise\n");
			if(CS_RISE()) logerror("er59256:CS rise\n");
			if(CK_FALL()) logerror("er59256:CK fall\n");
			if(CS_FALL()) logerror("er59256:CS fall\n");
		}

		if(CK_RISE() && CS_VALID())
		{
			if((STARTED()==0) && (GET_DI()==1))
			{
				m_bitcount=0;
				m_flags|=FLAG_START_BIT;
			}
			else
			{
				SHIFT_IN();
				m_bitcount++;

				if(m_bitcount==CMD_BITLEN)
					decode_command();

				if((m_bitcount==WRITE_BITLEN) && ((m_command & CMD_MASK)==CMD_WRITE))
				{
					m_eerom[m_command & ADDR_MASK]=m_in_shifter;
					LOG(1,"er59256:write[%02X]=%04X\n",(m_command & ADDR_MASK),m_in_shifter);
					m_command=CMD_INVALID;
				}
				LOG(1,"out_shifter=%05X, io_bits=%02X\n",m_out_shifter,m_io_bits);
				SHIFT_OUT();
			}

			LOG(2,"io_bits:out=%02X\n",m_io_bits);
		}
	}
}

UINT8 er59256_device::get_iobits()
{
	return m_io_bits;
}


void er59256_device::decode_command()
{
	m_out_shifter=0x0000;
	m_command=(m_in_shifter & (CMD_MASK | ADDR_MASK));

	switch(m_command & CMD_MASK)
	{
		case CMD_READ   : m_out_shifter=m_eerom[m_command & ADDR_MASK];
							LOG(1,"er59256:read[%02X]=%04X\n",(m_command&ADDR_MASK),m_eerom[m_command & ADDR_MASK]);
							break;
		case CMD_WRITE  : break;
		case CMD_ERASE  : if (WRITE_ENABLED()) m_eerom[m_command & ADDR_MASK]=0xFF;
							LOG(1,"er59256:erase[%02X]\n",(m_command&ADDR_MASK));
							break;
		case CMD_EWEN   : m_flags|=FLAG_WRITE_EN;
							LOG(1,"er59256:erase/write enabled\n");
							break;
		case CMD_EWDS   : m_flags&=~FLAG_WRITE_EN;
							LOG(1,"er59256:erase/write disabled\n");
							break;
		case CMD_ERAL   : if (WRITE_ENABLED()) memset(&m_eerom, 0xFF, EEROM_WORDS*2);
							LOG(1,"er59256:erase all\n");
							break;
	}

	if ((m_command & CMD_MASK)!=CMD_WRITE)
		m_command=CMD_INVALID;
}

/*********************************************************************

    er59256.h

    Microchip ER59256 serial eeprom.


*********************************************************************/

#ifndef _ER59256_H_
#define _ER59256_H_

#define EEROM_WORDS         0x10

/***************************************************************************
    MACROS
***************************************************************************/

class er59256_device : public device_t
{
public:
	er59256_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~er59256_device() {}

	void set_iobits(UINT8 newbits);
	UINT8 get_iobits();
	void preload_rom(const UINT16 *rom_data, int count);
	UINT8 data_loaded();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_stop();

private:
	// internal state

	/* The actual memory */
	UINT16  m_eerom[EEROM_WORDS];

	/* Bits as they appear on the io pins, current state */
	UINT8   m_io_bits;

	/* Bits as they appear on the io pins, previous state */
	UINT8   m_old_io_bits;


	/* the 16 bit shift in/out reg */
	UINT16  m_in_shifter;
	UINT32  m_out_shifter;

	/* Count of bits received since last CS low->high */
	UINT8   m_bitcount;

	/* Command & addresss */
	UINT8   m_command;

	/* Write enable and write in progress flags */
	UINT8   m_flags;

	void decode_command();
};

extern const device_type ER59256;


#define MCFG_ER59256_ADD(_tag)  \
	MCFG_DEVICE_ADD((_tag), ER59256, 0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CK_SHIFT            0x00
#define DI_SHIFT            0x01
#define DO_SHIFT            0x02
#define CS_SHIFT            0x03

#define CK_MASK             (1<<CK_SHIFT)
#define DI_MASK             (1<<DI_SHIFT)
#define DO_MASK             (1<<DO_SHIFT)
#define CS_MASK             (1<<CS_SHIFT)

#define ALL_MASK            (CK_MASK | DI_MASK | DO_MASK | CS_MASK)
#define IN_MASK             (CK_MASK | DI_MASK | CS_MASK)

#define GET_CK()         ((m_io_bits & CK_MASK) >> CK_SHIFT)
#define GET_DI()         ((m_io_bits & DI_MASK) >> DI_SHIFT)
#define GET_DO()         ((m_io_bits & DO_MASK) >> DO_SHIFT)
#define GET_CS()         ((m_io_bits & CS_MASK) >> CS_SHIFT)

#define SET_CK(data)    m_io_bits=((m_io_bits & ~CK_MASK) | ((data & 0x01) << CK_SHIFT))
#define SET_DI(data)    m_io_bits=((m_io_bits & ~DI_MASK) | ((data & 0x01) << DI_SHIFT))
#define SET_DO(data)    m_io_bits=((m_io_bits & ~DO_MASK) | ((data & 0x01) << DO_SHIFT))
#define SET_CS(data)    m_io_bits=((m_io_bits & ~CS_MASK) | ((data & 0x01) << CS_SHIFT))

#define CK_RISE()        ((m_io_bits & CK_MASK) & ~(m_old_io_bits & CK_MASK))
#define CS_RISE()        ((m_io_bits & CS_MASK) & ~(m_old_io_bits & CS_MASK))
#define CS_VALID()       ((m_io_bits & CS_MASK) & (m_old_io_bits & CS_MASK))

#define CK_FALL()        (~(m_io_bits & CK_MASK) & (m_old_io_bits & CK_MASK))
#define CS_FALL()        (~(m_io_bits & CS_MASK) & (m_old_io_bits & CS_MASK))


#define SHIFT_IN()       m_in_shifter=(m_in_shifter<<1) | GET_DI()
#define SHIFT_OUT()      SET_DO((m_out_shifter & 0x10000)>>16); m_out_shifter=(m_out_shifter<<1)

#define CMD_READ            0x80
#define CMD_WRITE           0x40
#define CMD_ERASE           0xC0
#define CMD_EWEN            0x30
#define CMD_EWDS            0x00
#define CMD_ERAL            0x20
#define CMD_INVALID         0xF0

#define CMD_MASK            0xF0
#define ADDR_MASK           0x0F

// CMD_BITLEN is 1 start bit plus 4 command bits plus 4 address bits
#define CMD_BITLEN          8
#define DATA_BITLEN         16
#define WRITE_BITLEN        CMD_BITLEN+DATA_BITLEN

#define FLAG_WRITE_EN       0x01
#define FLAG_START_BIT      0x02
#define FLAG_DATA_LOADED    0x04

#define WRITE_ENABLED()  ((m_flags & FLAG_WRITE_EN) ? 1 : 0)
#define STARTED()        ((m_flags & FLAG_START_BIT) ? 1 : 0)

#endif

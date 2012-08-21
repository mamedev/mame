/*********************************************************************

    er59256.h

    Microchip ER59256 serial eeprom.


*********************************************************************/

#ifndef _ER59256_H_
#define _ER59256_H_

/***************************************************************************
    MACROS
***************************************************************************/

DECLARE_LEGACY_DEVICE(ER59256, er59256);

#define MCFG_ER59256_ADD(_tag)	\
	MCFG_DEVICE_ADD((_tag), ER59256, 0)

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define EEROM_WORDS         0x10

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

#define GET_CK(eep)         ((eep->io_bits & CK_MASK) >> CK_SHIFT)
#define GET_DI(eep)         ((eep->io_bits & DI_MASK) >> DI_SHIFT)
#define GET_DO(eep)         ((eep->io_bits & DO_MASK) >> DO_SHIFT)
#define GET_CS(eep)         ((eep->io_bits & CS_MASK) >> CS_SHIFT)

#define SET_CK(eep,data)    eep->io_bits=((eep->io_bits & ~CK_MASK) | ((data & 0x01) << CK_SHIFT))
#define SET_DI(eep,data)    eep->io_bits=((eep->io_bits & ~DI_MASK) | ((data & 0x01) << DI_SHIFT))
#define SET_DO(eep,data)    eep->io_bits=((eep->io_bits & ~DO_MASK) | ((data & 0x01) << DO_SHIFT))
#define SET_CS(eep,data)    eep->io_bits=((eep->io_bits & ~CS_MASK) | ((data & 0x01) << CS_SHIFT))

#define CK_RISE(eep)        ((eep->io_bits & CK_MASK) & ~(eep->old_io_bits & CK_MASK))
#define CS_RISE(eep)        ((eep->io_bits & CS_MASK) & ~(eep->old_io_bits & CS_MASK))
#define CS_VALID(eep)       ((eep->io_bits & CS_MASK) & (eep->old_io_bits & CS_MASK))

#define CK_FALL(eep)        (~(eep->io_bits & CK_MASK) & (eep->old_io_bits & CK_MASK))
#define CS_FALL(eep)        (~(eep->io_bits & CS_MASK) & (eep->old_io_bits & CS_MASK))


#define SHIFT_IN(eep)       eep->in_shifter=(eep->in_shifter<<1) | GET_DI(eep)
#define SHIFT_OUT(eep)      SET_DO(eep,(eep->out_shifter & 0x10000)>>16); eep->out_shifter=(eep->out_shifter<<1)

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

#define WRITE_ENABLED(eep)  ((eep->flags & FLAG_WRITE_EN) ? 1 : 0)
#define STARTED(eep)        ((eep->flags & FLAG_START_BIT) ? 1 : 0)

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

void er59256_set_iobits(device_t *device, UINT8 newbits);
UINT8 er59256_get_iobits(device_t *device);
void er59256_preload_rom(device_t *device, const UINT16 *rom_data, int count);
UINT8 er59256_data_loaded(device_t *device);
#endif

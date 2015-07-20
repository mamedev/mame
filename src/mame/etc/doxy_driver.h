/*! @file doxy_driver.h
    @license BSD-3-Clause
    @copyright Angelo Salese & the MAME team

    @basehw 4enraya
    Main CPU: Z80
    Sound Chip: AY8910

    @memmap 4enraya,Z80
    0x0000-0xbfff ROM
    0xc000-0xcfff work RAM
    0xd000-0xdfff VRAM mirrored write,
        tilemap offset = address & 0x3ff
        tile number =  bits 0-7 = data, bits 8,9  = address bits 10,11
    0xe000-0xefff VRAM mirror
    0xf000-0xffff NOP

    @iomap 4enraya,Z80
    0x00-0x00 R dip-switches
    0x01-0x01 R inputs
    0x02-0x02 R system inputs
    0x23-0x23 W ay8910 data write
    0x33-0x33 W ay8910 control write

    @irq 4enraya,Z80
    @irqnum 0 @irqname Timer IRQ
 */


/**
 template for Doxygen commenting style for drivers
 All accepted special commands that aren't in Doxygen needs to be hooked up into the ini file.
 memmap/iomap/irq -> [%s,%s] -> [name_of_romset,cpu_name]
 irqnum -> type of irq, might accept one or two commands (for vectors)
 irqname -> A description of the irq used, might use brief and extended description.

 @sa http://www.stack.nl/~dimitri/doxygen/manual/commands.html
 @todo needs discussion for accepted standardized syntax
 */

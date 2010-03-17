/* machine/n64.c - contains N64 hardware emulation shared between MAME and MESS */

#include "emu.h"
#include "cpu/mips/mips3.h"
#include "cpu/mips/mips3com.h"
#include "streams.h"
#include "includes/n64.h"
#include "sound/dmadac.h"
#include "profiler.h"

UINT32 *rdram;
UINT32 *rsp_imem;
UINT32 *rsp_dmem;

// Memory Interface
static UINT32 mi_version = 0;
static UINT32 mi_interrupt = 0;
static UINT32 mi_intr_mask = 0;
static UINT32 mi_mode = 0;

// Memory Interface (MI)

#define MI_CLR_INIT             0x0080      /* Bit  7: clear init mode */
#define MI_SET_INIT             0x0100      /* Bit  8: set init mode */
#define MI_CLR_EBUS             0x0200      /* Bit  9: clear ebus test */
#define MI_SET_EBUS             0x0400      /* Bit 10: set ebus test mode */
#define MI_CLR_DP_INTR          0x0800      /* Bit 11: clear dp interrupt */
#define MI_CLR_RDRAM            0x1000      /* Bit 12: clear RDRAM reg */
#define MI_SET_RDRAM            0x2000      /* Bit 13: set RDRAM reg mode */
#define MI_MODE_INIT            0x0080      /* Bit  7: init mode */
#define MI_MODE_EBUS            0x0100      /* Bit  8: ebus test mode */
#define MI_MODE_RDRAM           0x0200      /* Bit  9: RDRAM reg mode */

READ32_HANDLER( n64_mi_reg_r )
{
	switch (offset)
	{
        case 0x00/4:            // MI_MODE_REG
            return mi_mode;

		case 0x04/4:			// MI_VERSION_REG
			return mi_version;

		case 0x08/4:			// MI_INTR_REG
			return mi_interrupt;

		case 0x0c/4:			// MI_INTR_MASK_REG
			return mi_intr_mask;

		default:
			logerror("mi_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}

	return 0;
}

WRITE32_HANDLER( n64_mi_reg_w )
{
	switch (offset)
	{
		case 0x00/4:		// MI_INIT_MODE_REG
            if (data & MI_CLR_INIT)     mi_mode &= ~MI_MODE_INIT;
            if (data & MI_SET_INIT)     mi_mode |=  MI_MODE_INIT;
            if (data & MI_CLR_EBUS)     mi_mode &= ~MI_MODE_EBUS;
            if (data & MI_SET_EBUS)     mi_mode |=  MI_MODE_EBUS;
            if (data & MI_CLR_RDRAM)    mi_mode &= ~MI_MODE_RDRAM;
            if (data & MI_SET_RDRAM)    mi_mode |=  MI_MODE_RDRAM;
            if (data & MI_CLR_DP_INTR)
			{
				clear_rcp_interrupt(space->machine, DP_INTERRUPT);
			}
			break;

		case 0x04/4:		// MI_VERSION_REG
			mi_version = data;
			break;

		case 0x0c/4:		// MI_INTR_MASK_REG
		{
            if (data & 0x0001)
            {
                mi_intr_mask &= ~0x1;      // clear SP mask
            }
            if (data & 0x0002)
            {
                mi_intr_mask |= 0x1;           // set SP mask
            }
            if (data & 0x0004)
            {
                mi_intr_mask &= ~0x2;      // clear SI mask
            }
            if (data & 0x0008)
            {
                mi_intr_mask |= 0x2;           // set SI mask
            }
            if (data & 0x0010)
            {
                mi_intr_mask &= ~0x4;      // clear AI mask
            }
            if (data & 0x0020)
            {
                mi_intr_mask |= 0x4;           // set AI mask
            }
            if (data & 0x0040)
            {
                mi_intr_mask &= ~0x8;      // clear VI mask
            }
            if (data & 0x0080)
            {
                mi_intr_mask |= 0x8;           // set VI mask
            }
            if (data & 0x0100)
            {
                mi_intr_mask &= ~0x10;     // clear PI mask
            }
            if (data & 0x0200)
            {
                mi_intr_mask |= 0x10;      // set PI mask
            }
            if (data & 0x0400)
            {
                mi_intr_mask &= ~0x20;     // clear DP mask
            }
            if (data & 0x0800)
            {
                mi_intr_mask |= 0x20;      // set DP mask
            }
			break;
		}

		default:
			logerror("mi_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}
}

static running_device *dmadac[2];

void signal_rcp_interrupt(running_machine *machine, int interrupt)
{
	if (mi_intr_mask & interrupt)
	{
		mi_interrupt |= interrupt;

		cputag_set_input_line(machine, "maincpu", INPUT_LINE_IRQ0, ASSERT_LINE);
	}
}

void clear_rcp_interrupt(running_machine *machine, int interrupt)
{
	mi_interrupt &= ~interrupt;

	//if (!mi_interrupt)
	{
		cputag_set_input_line(machine, "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

static UINT8 is64_buffer[0x10000];

READ32_HANDLER( n64_is64_r )
{
    switch(offset)
    {
        case 0x0000/4:
            return 0x49533634;

        case 0x0004/4:
        case 0x0008/4:
        case 0x000c/4:
        case 0x0010/4:
        case 0x0014/4:
        case 0x0018/4:
        case 0x001c/4:
            return 0;

        default:
            return ( is64_buffer[(offset << 2) + 0] << 24 ) |
                   ( is64_buffer[(offset << 2) + 1] << 16 ) |
                   ( is64_buffer[(offset << 2) + 2] <<  8 ) |
                   ( is64_buffer[(offset << 2) + 3] <<  0 );
    }
}

WRITE32_HANDLER( n64_is64_w )
{
    int i = 0;

    switch(offset)
    {
        case 0x0014/4:
            for(i = 0x20; i < (0x20 + data); i++)
            {
                printf( "%c", is64_buffer[i] );
                if(is64_buffer[i] == 0x0a)
                {
                    printf( "%c", 0x0d );
                }
                is64_buffer[i] = 0;
            }
            break;

        default:
            is64_buffer[(offset << 2) + 0] = (data >> 24) & 0x000000ff;
            is64_buffer[(offset << 2) + 1] = (data >> 16) & 0x000000ff;
            is64_buffer[(offset << 2) + 2] = (data >>  8) & 0x000000ff;
            is64_buffer[(offset << 2) + 3] = (data >>  0) & 0x000000ff;
            break;
    }
}

READ32_HANDLER( n64_open_r )
{
    UINT32 retval = (offset << 2) & 0x0000ffff;
    retval = (retval << 16) | retval;
    return retval;
}

WRITE32_HANDLER( n64_open_w )
{
    // Do nothing
}

// RDRAM registers
static UINT32 rdram_config;
static UINT32 rdram_device_id;
static UINT32 rdram_delay;
static UINT32 rdram_mode;
static UINT32 rdram_ref_interval;
static UINT32 rdram_ref_row;
static UINT32 rdram_ras_interval;
static UINT32 rdram_min_interval;
static UINT32 rdram_addr_select;
static UINT32 rdram_device_manuf;

READ32_HANDLER( n64_rdram_reg_r )
{
    switch (offset)
    {
        case 0x00/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_config;

        case 0x04/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_device_id;

        case 0x08/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_delay;

        case 0x0c/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_mode;

        case 0x10/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_ref_interval;

        case 0x14/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_ref_row;

        case 0x18/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_ras_interval;

        case 0x1c/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_min_interval;

        case 0x20/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_addr_select;

        case 0x24/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            return rdram_device_manuf;

        default:
            logerror("rdram_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
            break;
    }
    return 0;
}

WRITE32_HANDLER( n64_rdram_reg_w )
{
    switch (offset)
    {
        case 0x00/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_config = data;
            break;

        case 0x04/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_device_id = data;
            break;

        case 0x08/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_delay = data;
            break;

        case 0x0c/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_mode = data;
            break;

        case 0x10/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_ref_interval = data;
            break;

        case 0x14/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_ref_row = data;
            break;

        case 0x18/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_ras_interval = data;
            break;

        case 0x1c/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_min_interval = data;
            break;

        case 0x20/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_addr_select = data;
            break;

        case 0x24/4:            // RDRAM_CONFIG_REG / RDRAM_DEVICE_TYPE_REG
            rdram_device_manuf = data;
            break;

        default:
            logerror("mi_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
            break;
    }
}

// RSP Interface

static UINT32 sp_mem_addr;
static UINT32 sp_dram_addr;
static int sp_dma_length;
static int sp_dma_count;
static int sp_dma_skip;
static UINT32 sp_semaphore;
static UINT32 dp_clock = 0;

static void sp_dma(int direction)
{
	UINT8 *src, *dst;
	int i, c;

	if (sp_dma_length == 0)
	{
		return;
	}

	sp_dma_length++;
	if ((sp_dma_length & 7) != 0)
	{
		//fatalerror("sp_dma (%s): sp_dma_length unaligned %08X\n", cpu ? "RSP" : "R4300i", sp_dma_length);
		//sp_dma_length = sp_dma_length & ~3;
        sp_dma_length = (sp_dma_length + 7) & ~7;

		//sp_dma_length &= ~7;
	}

	if (sp_mem_addr & 0x3)
	{
        //sp_mem_addr = (sp_mem_addr + 3) & ~3;
        sp_mem_addr = sp_mem_addr & ~3;
        // sp_mem_addr &= ~0x3;
        // fatalerror("sp_dma (%s): sp_mem_addr unaligned: %08X\n", cpu ? "RSP" : "R4300i", sp_mem_addr);
	}
	if (sp_dram_addr & 0x7)
	{
        sp_dram_addr = sp_dram_addr & ~7;
	}

	if (sp_dma_count > 0)
	{
		// fatalerror("sp_dma: dma_count = %d\n", sp_dma_count);
	}
	if (sp_dma_skip > 0)
	{
		// fatalerror("sp_dma: dma_skip = %d\n", sp_dma_skip);
	}

	if ((sp_mem_addr & 0xfff) + (sp_dma_length) > 0x1000)
	{
		printf("sp_dma: dma out of memory area: %08X, %08X\n", sp_mem_addr, sp_dma_length);
		//fatalerror("sp_dma: dma out of memory area: %08X, %08X\n", sp_mem_addr, sp_dma_length);
		sp_dma_length = 0x1000 - (sp_mem_addr & 0xfff);
	}

	if (direction == 0)		// RDRAM -> I/DMEM
	{
        for (c=0; c <= sp_dma_count; c++)
        {
            src = (UINT8*)&rdram[sp_dram_addr / 4];
            dst = (sp_mem_addr & 0x1000) ? (UINT8*)&rsp_imem[(sp_mem_addr & 0xfff) / 4] : (UINT8*)&rsp_dmem[(sp_mem_addr & 0xfff) / 4];

            for (i=0; i < sp_dma_length; i++)
            {
                dst[BYTE4_XOR_BE(i)] = src[BYTE4_XOR_BE(i)];
            }

            sp_mem_addr += sp_dma_length;
            sp_dram_addr += sp_dma_length;

            sp_mem_addr += sp_dma_skip;
        }
	}
	else					// I/DMEM -> RDRAM
	{
        for (c=0; c <= sp_dma_count; c++)
        {
            src = (sp_mem_addr & 0x1000) ? (UINT8*)&rsp_imem[(sp_mem_addr & 0xfff) / 4] : (UINT8*)&rsp_dmem[(sp_mem_addr & 0xfff) / 4];
            dst = (UINT8*)&rdram[sp_dram_addr / 4];

            for (i=0; i < sp_dma_length; i++)
            {
                dst[BYTE4_XOR_BE(i)] = src[BYTE4_XOR_BE(i)];
            }

            sp_mem_addr += sp_dma_length;
            sp_dram_addr += sp_dma_length;

            sp_dram_addr += sp_dma_skip;
        }
	}
}

static void sp_set_status(running_device *device, UINT32 status)
{
	if (status & 0x1)
	{
		//cpuexec_trigger(device->machine, 6789);

		cpu_set_input_line(device, INPUT_LINE_HALT, ASSERT_LINE);
        cpu_set_reg(device, RSP_SR, cpu_get_reg(device, RSP_SR) | RSP_STATUS_HALT);
		//rsp_sp_status |= SP_STATUS_HALT;
	}
	if (status & 0x2)
	{
		//rsp_sp_status |= SP_STATUS_BROKE;
        cpu_set_reg(device, RSP_SR, cpu_get_reg(device, RSP_SR) | RSP_STATUS_BROKE);

        if (cpu_get_reg(device, RSP_SR) & RSP_STATUS_INTR_BREAK)
		{
			signal_rcp_interrupt(device->machine, SP_INTERRUPT);
		}
	}
}

READ32_DEVICE_HANDLER( n64_sp_reg_r )
{
	switch (offset)
	{
		case 0x00/4:		// SP_MEM_ADDR_REG
			return sp_mem_addr;

		case 0x04/4:		// SP_DRAM_ADDR_REG
			return sp_dram_addr;

		case 0x08/4:		// SP_RD_LEN_REG
			return (sp_dma_skip << 20) | (sp_dma_count << 12) | sp_dma_length;

		case 0x10/4:		// SP_STATUS_REG
            return cpu_get_reg(device, RSP_SR);

		case 0x14/4:		// SP_DMA_FULL_REG
			return 0;

		case 0x18/4:		// SP_DMA_BUSY_REG
			return 0;

		case 0x1c/4:		// SP_SEMAPHORE_REG
            if( sp_semaphore )
            {
                return 1;
            }
            else
            {
                sp_semaphore = 1;
                return 0;
            }

        case 0x20/4:        // DP_CMD_START
        case 0x24/4:        // DP_CMD_END
        case 0x28/4:        // DP_CMD_CURRENT
        case 0x34/4:        // DP_CMD_BUSY
        case 0x38/4:        // DP_CMD_PIPE_BUSY
        case 0x3c/4:        // DP_CMD_TMEM_BUSY
            return 0;

        case 0x2c/4:        // DP_CMD_STATUS
        	return 0x88;

        case 0x30/4:        // DP_CMD_CLOCK
        	return ++dp_clock;

        case 0x40000/4:     // PC
            return cpu_get_reg(device, RSP_PC) & 0x00000fff;

        default:
            logerror("sp_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(device));
            break;
	}

	return 0;
}

WRITE32_DEVICE_HANDLER( n64_sp_reg_w )
{
	if ((offset & 0x10000) == 0)
	{
		switch (offset & 0xffff)
		{
			case 0x00/4:		// SP_MEM_ADDR_REG
				sp_mem_addr = data;
				break;

			case 0x04/4:		// SP_DRAM_ADDR_REG
				sp_dram_addr = data & 0xffffff;
				break;

			case 0x08/4:		// SP_RD_LEN_REG
				sp_dma_length = data & 0xfff;
				sp_dma_count = (data >> 12) & 0xff;
				sp_dma_skip = (data >> 20) & 0xfff;
				sp_dma(0);
				break;

			case 0x0c/4:		// SP_WR_LEN_REG
				sp_dma_length = data & 0xfff;
				sp_dma_count = (data >> 12) & 0xff;
				sp_dma_skip = (data >> 20) & 0xfff;
				sp_dma(1);
				break;

            case 0x10/4:        // RSP_STATUS_REG
            {
            	UINT32 oldstatus = cpu_get_reg(device, RSP_SR);
            	UINT32 newstatus = oldstatus;

                // printf( "RSP_STATUS_REG Write; %08x\n", data );
                if (data & 0x00000001)      // clear halt
                {
                    //if (first_rsp)
                    //{
                    //  cpu_spinuntil_trigger(device, 6789);

                        // printf( "Clearing RSP_STATUS_HALT\n" );
                        cpu_set_input_line(device, INPUT_LINE_HALT, CLEAR_LINE);
                        newstatus &= ~RSP_STATUS_HALT;
                        // RSP_STATUS &= ~RSP_STATUS_HALT;
                    //}
                    //else
                    //{
                    //  first_rsp = 1;
                    //}
                }
                if (data & 0x00000002)      // set halt
                {
                    // printf( "Setting RSP_STATUS_HALT\n" );
                    cpu_set_input_line(device, INPUT_LINE_HALT, ASSERT_LINE);
                    newstatus |= RSP_STATUS_HALT;
                    // RSP_STATUS |= RSP_STATUS_HALT;
                }
                if (data & 0x00000004)
                {
                    //printf( "Clearing RSP_STATUS_BROKE\n" );
                    newstatus &= ~RSP_STATUS_BROKE;
                    // RSP_STATUS &= ~RSP_STATUS_BROKE;     // clear broke
                }
                if (data & 0x00000008)      // clear interrupt
                {
                    clear_rcp_interrupt(device->machine, SP_INTERRUPT);
                }
                if (data & 0x00000010)      // set interrupt
                {
                    signal_rcp_interrupt(device->machine, SP_INTERRUPT);
                }
                if (data & 0x00000020)
                {
                    // printf( "Clearing RSP_STATUS_SSTEP\n" );
                    newstatus &= ~RSP_STATUS_SSTEP;
                    // RSP_STATUS &= ~RSP_STATUS_SSTEP;     // clear single step
                }
                if (data & 0x00000040)
                {
                    //printf( "Setting RSP_STATUS_SSTEP\n" );
                    newstatus |= RSP_STATUS_SSTEP;
                    if( !( oldstatus & ( RSP_STATUS_BROKE | RSP_STATUS_HALT ) ) )
                    {
                        cpu_set_reg(device, RSP_STEPCNT, 1 );
                    }
                    // RSP_STATUS |= RSP_STATUS_SSTEP;      // set single step
                }
                if (data & 0x00000080)
                {
                    newstatus &= ~RSP_STATUS_INTR_BREAK;
                    // RSP_STATUS &= ~RSP_STATUS_INTR_BREAK;    // clear interrupt on break
                }
                if (data & 0x00000100)
                {
                    newstatus |= RSP_STATUS_INTR_BREAK;
                    // RSP_STATUS |= RSP_STATUS_INTR_BREAK; // set interrupt on break
                }
                if (data & 0x00000200)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL0;
                    // RSP_STATUS &= ~RSP_STATUS_SIGNAL0;       // clear signal 0
                }
                if (data & 0x00000400)
                {
                    newstatus |= RSP_STATUS_SIGNAL0;
                    // RSP_STATUS |= RSP_STATUS_SIGNAL0;        // set signal 0
                }
                if (data & 0x00000800)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL1;
                    // RSP_STATUS &= ~RSP_STATUS_SIGNAL1;       // clear signal 1
                }
                if (data & 0x00001000)
                {
                    newstatus |= RSP_STATUS_SIGNAL1;
                    // RSP_STATUS |= RSP_STATUS_SIGNAL1;        // set signal 1
                }
                if (data & 0x00002000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL2 ;
                    // RSP_STATUS &= ~RSP_STATUS_SIGNAL2;       // clear signal 2
                }
                if (data & 0x00004000)
                {
                    newstatus |= RSP_STATUS_SIGNAL2;
                    // RSP_STATUS |= RSP_STATUS_SIGNAL2;        // set signal 2
                }
                if (data & 0x00008000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL3;
                    // RSP_STATUS &= ~RSP_STATUS_SIGNAL3;       // clear signal 3
                }
                if (data & 0x00010000)
                {
                    newstatus |= RSP_STATUS_SIGNAL3;
                    // RSP_STATUS |= RSP_STATUS_SIGNAL3;        // set signal 3
                }
                if (data & 0x00020000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL4;
                    // RSP_STATUS &= ~RSP_STATUS_SIGNAL4;       // clear signal 4
                }
                if (data & 0x00040000)
                {
                    newstatus |= RSP_STATUS_SIGNAL4;
                    // RSP_STATUS |= RSP_STATUS_SIGNAL4;        // set signal 4
                }
                if (data & 0x00080000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL5;
                    // RSP_STATUS &= ~RSP_STATUS_SIGNAL5;       // clear signal 5
                }
                if (data & 0x00100000)
                {
                    newstatus |= RSP_STATUS_SIGNAL5;
                    // RSP_STATUS |= RSP_STATUS_SIGNAL5;        // set signal 5
                }
                if (data & 0x00200000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL6;
                    // RSP_STATUS &= ~RSP_STATUS_SIGNAL6;       // clear signal 6
                }
                if (data & 0x00400000)
                {
                    newstatus |= RSP_STATUS_SIGNAL6;
                    // RSP_STATUS |= RSP_STATUS_SIGNAL6;        // set signal 6
                }
                if (data & 0x00800000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL7;
                    // RSP_STATUS &= ~RSP_STATUS_SIGNAL7;       // clear signal 7
                }
                if (data & 0x01000000)
                {
                    newstatus |= RSP_STATUS_SIGNAL7;
                    // RSP_STATUS |= RSP_STATUS_SIGNAL7;        // set signal 7
                }
                cpu_set_reg(device, RSP_SR, newstatus);
                break;
            }

			case 0x1c/4:		// SP_SEMAPHORE_REG
				if(data == 0)
				{
                	sp_semaphore = 0;
				}
		//      mame_printf_debug("sp_semaphore = %08X\n", sp_semaphore);
				break;

			default:
				logerror("sp_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(device));
				break;
		}
	}
	else
	{
        switch (offset & 0xffff)
        {
            case 0x00/4:        // SP_PC_REG
                //printf( "Setting PC to: %08x\n", 0x04001000 | (data & 0xfff ) );
                if( cpu_get_reg(device, RSP_NEXTPC) != 0xffffffff )
                {
                    cpu_set_reg(device, RSP_NEXTPC, 0x1000 | (data & 0xfff));
                }
                else
                {
                    cpu_set_reg(device, RSP_PC, 0x1000 | (data & 0xfff));
                }
                break;

            default:
                logerror("sp_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(device));
                break;
		}
	}
}

// RDP Interface
UINT32 dp_start;
UINT32 dp_end;
UINT32 dp_current;
UINT32 dp_status = 0x88;


void dp_full_sync(running_machine *machine)
{
	signal_rcp_interrupt(machine, DP_INTERRUPT);
}

READ32_DEVICE_HANDLER( n64_dp_reg_r )
{
	switch (offset)
	{
		case 0x00/4:		// DP_START_REG
			return dp_start;

		case 0x04/4:		// DP_END_REG
			return dp_end;

		case 0x08/4:		// DP_CURRENT_REG
			return dp_current;

		case 0x0c/4:		// DP_STATUS_REG
			return dp_status;

		default:
			logerror("dp_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(device));
			break;
	}

	return 0;
}

WRITE32_DEVICE_HANDLER( n64_dp_reg_w )
{
	switch (offset)
	{
		case 0x00/4:		// DP_START_REG
			dp_start = data;
			dp_current = dp_start;
			break;

		case 0x04/4:		// DP_END_REG
			dp_end = data;
			profiler_mark_start(PROFILER_USER1);
			rdp_process_list(device->machine);
			profiler_mark_end();
			break;

		case 0x0c/4:		// DP_STATUS_REG
			if (data & 0x00000001)	dp_status &= ~DP_STATUS_XBUS_DMA;
			if (data & 0x00000002)	dp_status |= DP_STATUS_XBUS_DMA;
			if (data & 0x00000004)	dp_status &= ~DP_STATUS_FREEZE;
			if (data & 0x00000008)	dp_status |= DP_STATUS_FREEZE;
			if (data & 0x00000010)	dp_status &= ~DP_STATUS_FLUSH;
			if (data & 0x00000020)	dp_status |= DP_STATUS_FLUSH;
			break;

		default:
			logerror("dp_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(device));
			break;
	}
}


const rsp_config n64_rsp_config =
{
	n64_dp_reg_r,
	n64_dp_reg_w,
	n64_sp_reg_r,
	n64_sp_reg_w,
	sp_set_status
};


// Video Interface
UINT32 n64_vi_width;
UINT32 n64_vi_origin;
UINT32 n64_vi_control;
UINT32 n64_vi_blank;
UINT32 n64_vi_hstart;
UINT32 n64_vi_vstart;
UINT32 n64_vi_xscale;
UINT32 n64_vi_yscale;
static UINT32 n64_vi_burst, n64_vi_vsync,  n64_vi_hsync,  n64_vi_leap;
static UINT32 n64_vi_intr,  n64_vi_vburst;


static void n64_vi_recalculate_resolution(running_machine *machine)
{
    int x_start = (n64_vi_hstart & 0x03ff0000) >> 16;
    int x_end = n64_vi_hstart & 0x000003ff;
    int y_start = ((n64_vi_vstart & 0x03ff0000) >> 16) / 2;
    int y_end = (n64_vi_vstart & 0x000003ff) / 2;
    int width = ((n64_vi_xscale & 0x00000fff) * (x_end - x_start)) / 0x400;
    int height = ((n64_vi_yscale & 0x00000fff) * (y_end - y_start)) / 0x400;
    rectangle visarea = *video_screen_get_visible_area(machine->primary_screen);
    attoseconds_t period = video_screen_get_frame_period(machine->primary_screen).attoseconds;

    if (width == 0 || height == 0)
    {
        n64_vi_blank = 1;
        /*
        FIXME: MAME doesn't handle well a h/w res of zero (otherwise it hardlocks the emu, seen especially in Aleck 64 games
        that sets the res after a longer delay than n64), guess that this just disables drawing?
        */
        return;
    }
    else
    {
        n64_vi_blank = 0;
    }

//  if (width == 0)
//      width = 1;

//  if (height == 0)
//      height = 1;

    if (width > 640)
        width = 640;

    if (height > 480)
        height = 480;

	fb_height = height;

    visarea.max_x = width - 1;
    visarea.max_y = height - 1;
    video_screen_configure(machine->primary_screen, width, 525, &visarea, period);
}

READ32_HANDLER( n64_vi_reg_r )
{
	switch (offset)
	{
		case 0x00/4:		// VI_CONTROL_REG
			return n64_vi_control;

		case 0x04/4:		// VI_ORIGIN_REG
            return n64_vi_origin;

		case 0x08/4:		// VI_WIDTH_REG
            return n64_vi_width;

		case 0x0c/4:
            return n64_vi_intr;

		case 0x10/4:		// VI_CURRENT_REG
			return video_screen_get_vpos(space->machine->primary_screen);

		case 0x14/4:		// VI_BURST_REG
            return n64_vi_burst;

		case 0x18/4:		// VI_V_SYNC_REG
            return n64_vi_vsync;

		case 0x1c/4:		// VI_H_SYNC_REG
            return n64_vi_hsync;

		case 0x20/4:		// VI_LEAP_REG
            return n64_vi_leap;

		case 0x24/4:		// VI_H_START_REG
            return n64_vi_hstart;

		case 0x28/4:		// VI_V_START_REG
            return n64_vi_vstart;

		case 0x2c/4:		// VI_V_BURST_REG
            return n64_vi_vburst;

		case 0x30/4:		// VI_X_SCALE_REG
            return n64_vi_xscale;

		case 0x34/4:		// VI_Y_SCALE_REG
            return n64_vi_yscale;

		default:
			logerror("vi_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}
	return 0;
}

WRITE32_HANDLER( n64_vi_reg_w )
{
	switch (offset)
	{
		case 0x00/4:		// VI_CONTROL_REG
            n64_vi_control = data;
            n64_vi_recalculate_resolution(space->machine);
			break;

		case 0x04/4:		// VI_ORIGIN_REG
            n64_vi_origin = data & 0xffffff;
			break;

		case 0x08/4:		// VI_WIDTH_REG
            if (n64_vi_width != data && data > 0)
			{
                n64_vi_recalculate_resolution(space->machine);
			}
            n64_vi_width = data;
		    fb_width = data;
			break;

		case 0x0c/4:		// VI_INTR_REG
            n64_vi_intr = data;
			break;

		case 0x10/4:		// VI_CURRENT_REG
			clear_rcp_interrupt(space->machine, VI_INTERRUPT);
			break;

		case 0x14/4:		// VI_BURST_REG
            n64_vi_burst = data;
			break;

		case 0x18/4:		// VI_V_SYNC_REG
            n64_vi_vsync = data;
			break;

		case 0x1c/4:		// VI_H_SYNC_REG
            n64_vi_hsync = data;
			break;

		case 0x20/4:		// VI_LEAP_REG
            n64_vi_leap = data;
			break;

		case 0x24/4:		// VI_H_START_REG
            n64_vi_hstart = data;
            n64_vi_recalculate_resolution(space->machine);
			break;

		case 0x28/4:		// VI_V_START_REG
            n64_vi_vstart = data;
            n64_vi_recalculate_resolution(space->machine);
			break;

		case 0x2c/4:		// VI_V_BURST_REG
            n64_vi_vburst = data;
			break;

		case 0x30/4:		// VI_X_SCALE_REG
            n64_vi_xscale = data;
            n64_vi_recalculate_resolution(space->machine);
			break;

		case 0x34/4:		// VI_Y_SCALE_REG
            n64_vi_yscale = data;
            n64_vi_recalculate_resolution(space->machine);
			break;

        /*
        Uncomment this for convenient homebrew debugging
        */
        case 0x44/4:        // TEMP DEBUG
            printf( "E Ping: %08x\n", data );
            break;

		default:
			logerror("vi_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}
}


// Audio Interface
static UINT32 ai_dram_addr;
static UINT32 ai_len;
static UINT32 ai_control = 0;
static int ai_dacrate;
static int ai_bitrate;
static UINT32 ai_status = 0;

static emu_timer *audio_timer;

#define AUDIO_DMA_DEPTH     2

static void start_audio_dma(running_machine *machine);

typedef struct
{
    UINT32 address;
    UINT32 length;
} AUDIO_DMA;

static AUDIO_DMA audio_fifo[AUDIO_DMA_DEPTH];
static int audio_fifo_wpos = 0;
static int audio_fifo_rpos = 0;
static int audio_fifo_num = 0;

static void audio_fifo_push(running_machine *machine, UINT32 address, UINT32 length)
{
    AUDIO_DMA *current;

    if (audio_fifo_num == AUDIO_DMA_DEPTH)
    {
        mame_printf_debug("audio_fifo_push: tried to push to full DMA FIFO!!!\n");
    }

//  mame_printf_debug("fifo_push: adr %08x len %08x\n", address, length);

    audio_fifo[audio_fifo_wpos].address = address;
    audio_fifo[audio_fifo_wpos].length = length;
    current = &audio_fifo[audio_fifo_wpos];

    audio_fifo_wpos++;
    audio_fifo_num++;

    if (audio_fifo_wpos >= AUDIO_DMA_DEPTH)
    {
        audio_fifo_wpos = 0;
    }

    if (audio_fifo_num >= AUDIO_DMA_DEPTH)
    {
        ai_status |= 0x80000001;    // FIFO full
    }

    if (! (ai_status & 0x40000000))
    {
        signal_rcp_interrupt(machine, AI_INTERRUPT);
        start_audio_dma(machine);
    }
}

static void audio_fifo_pop(running_machine *machine)
{
    audio_fifo_rpos++;
    audio_fifo_num--;

    if (audio_fifo_num < 0)
    {
        fatalerror("audio_fifo_pop: FIFO underflow!\n");
    }

    if (audio_fifo_rpos >= AUDIO_DMA_DEPTH)
    {
        audio_fifo_rpos = 0;
    }

    if (audio_fifo_num < AUDIO_DMA_DEPTH)
    {
        ai_status &= ~0x80000001;   // FIFO not full
        signal_rcp_interrupt(machine, AI_INTERRUPT);
    }
}

static AUDIO_DMA *audio_fifo_get_top(void)
{
    if (audio_fifo_num > 0)
    {
        return &audio_fifo[audio_fifo_rpos];
    }
    else
    {
        return NULL;
    }
}

static void start_audio_dma(running_machine *machine)
{
    INT16 *ram = (INT16*)rdram;
    AUDIO_DMA *current = audio_fifo_get_top();
    attotime period;

    //static FILE * audio_dump = NULL;
    //
    //if (audio_dump == NULL)
    //    audio_dump = fopen("audio_dump.raw","wb");
    //
    //fwrite(&ram[current->address/2],current->length,1,audio_dump);

    ram = &ram[current->address/2];

//  mame_printf_debug("DACDMA: %x for %x bytes\n", current->address, current->length);

	dmadac[0] = devtag_get_device(machine, "dac1");
	dmadac[1] = devtag_get_device(machine, "dac2");
    dmadac_transfer(&dmadac[0], 2, 2, 2, current->length/4, ram);

    ai_status |= 0x40000000;

   // adjust the timer
   period = attotime_mul(ATTOTIME_IN_HZ(DACRATE_NTSC), (ai_dacrate + 1) * (current->length / 4));
   timer_adjust_oneshot(audio_timer, period, 0);
}

static TIMER_CALLBACK( audio_timer_callback )
{
    audio_fifo_pop(machine);

    // keep playing if there's another DMA queued
    if (audio_fifo_get_top() != NULL)
    {
        start_audio_dma(machine);
        signal_rcp_interrupt(machine, AI_INTERRUPT);
    }
    else
    {
        ai_status &= ~0x40000000;
    }
}

READ32_HANDLER( n64_ai_reg_r )
{
    switch (offset)
    {
        case 0x04/4:        // AI_LEN_REG
        {
            if (ai_status & 0x80000001)
            {
                return ai_len;
            }
            else if (ai_status & 0x40000000)
            {
                double secs_left = attotime_to_double(attotime_sub(timer_firetime(audio_timer),timer_get_time(space->machine)));
                unsigned int samples_left = secs_left * DACRATE_NTSC / (ai_dacrate + 1);
                return samples_left * 4;
            }
            else return 0;
        }

        case 0x0c/4:        // AI_STATUS_REG
            return ai_status;

        default:
            logerror("ai_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
            break;
    }

    return 0;
}

WRITE32_HANDLER( n64_ai_reg_w )
{
//  UINT16 *ram = (UINT16*)rdram;

    switch (offset)
    {
        case 0x00/4:        // AI_DRAM_ADDR_REG
//          mame_printf_debug("ai_dram_addr = %08X at %08X\n", data, cpu_get_pc(space->cpu));
            ai_dram_addr = data & 0xffffff;
            break;

        case 0x04/4:        // AI_LEN_REG
//          mame_printf_debug("ai_len = %08X at %08X\n", data, cpu_get_pc(space->cpu));
            ai_len = data & 0x3ffff;        // Hardware v2.0 has 18 bits, v1.0 has 15 bits
            audio_fifo_push(space->machine, ai_dram_addr, ai_len);
            break;

        case 0x08/4:        // AI_CONTROL_REG
//          mame_printf_debug("ai_control = %08X at %08X\n", data, cpu_get_pc(space->cpu));
            ai_control = data;
            break;

        case 0x0c/4:
            clear_rcp_interrupt(space->machine, AI_INTERRUPT);
            break;

        case 0x10/4:        // AI_DACRATE_REG
            ai_dacrate = data & 0x3fff;
            dmadac_set_frequency(&dmadac[0], 2, (double)DACRATE_NTSC / (double)(ai_dacrate+1));
            printf( "frequency: %f\n", (double)DACRATE_NTSC / (double)(ai_dacrate+1) );
            dmadac_enable(&dmadac[0], 2, 1);
            break;

        case 0x14/4:        // AI_BITRATE_REG
//          mame_printf_debug("ai_bitrate = %08X\n", data);
            ai_bitrate = data & 0xf;
            break;

        default:
            logerror("ai_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
            break;
    }
}


// Peripheral Interface

static UINT32 pi_dram_addr, pi_cart_addr;
static UINT32 pi_first_dma = 1;
static UINT32 pi_rd_len = 0;
static UINT32 pi_wr_len = 0;
static UINT32 pi_status = 0;
static UINT32 pi_bsd_dom1_lat = 0;
static UINT32 pi_bsd_dom1_pwd = 0;
static UINT32 pi_bsd_dom1_pgs = 0;
static UINT32 pi_bsd_dom1_rls = 0;
static UINT32 pi_bsd_dom2_lat = 0;
static UINT32 pi_bsd_dom2_pwd = 0;
static UINT32 pi_bsd_dom2_pgs = 0;
static UINT32 pi_bsd_dom2_rls = 0;

READ32_HANDLER( n64_pi_reg_r )
{
	switch (offset)
	{
		case 0x00/4:		// PI_DRAM_ADDR_REG
			return pi_dram_addr;

		case 0x04/4:		// PI_CART_ADDR_REG
			return pi_cart_addr;

		case 0x10/4:		// PI_STATUS_REG
			return pi_status;

        case 0x14/4:        // PI_BSD_DOM1_LAT
            return pi_bsd_dom1_lat;

        case 0x18/4:        // PI_BSD_DOM1_PWD
            return pi_bsd_dom1_pwd;

        case 0x1c/4:        // PI_BSD_DOM1_PGS
            return pi_bsd_dom1_pgs;

        case 0x20/4:        // PI_BSD_DOM1_RLS
            return pi_bsd_dom1_rls;

        case 0x24/4:        // PI_BSD_DOM2_LAT
            return pi_bsd_dom2_lat;

        case 0x28/4:        // PI_BSD_DOM2_PWD
            return pi_bsd_dom2_pwd;

        case 0x2c/4:        // PI_BSD_DOM2_PGS
            return pi_bsd_dom2_pgs;

        case 0x30/4:        // PI_BSD_DOM2_RLS
            return pi_bsd_dom2_rls;

		default:
			logerror("pi_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}
	return 0;
}

WRITE32_HANDLER( n64_pi_reg_w )
{
	switch (offset)
	{
		case 0x00/4:		// PI_DRAM_ADDR_REG
		{
			pi_dram_addr = data;
			break;
		}

		case 0x04/4:		// PI_CART_ADDR_REG
		{
			pi_cart_addr = data;
			break;
		}

		case 0x08/4:		// PI_RD_LEN_REG
		{
			int i;
			UINT32 dma_length = (data + 1);
            pi_rd_len = data;

			/*if (dma_length & 3)
            {
                dma_length = (dma_length + 3) & ~3;
            }*/

			//mame_printf_debug("PI DMA: %08X to %08X, length %08X\n", pi_dram_addr, pi_cart_addr, dma_length);

			if (pi_dram_addr != 0xffffffff)
			{
				for (i=0; i < dma_length; i++)
				{
					UINT8 b = memory_read_byte(space, pi_dram_addr);
					memory_write_byte(space, pi_cart_addr & 0x1fffffff, b);
					pi_cart_addr += 1;
					pi_dram_addr += 1;
				}
			}

			signal_rcp_interrupt(space->machine, PI_INTERRUPT);
			break;
		}

		case 0x0c/4:		// PI_WR_LEN_REG
		{
			int i;
			UINT32 dma_length = (data + 1);
            pi_wr_len = data;

			if (dma_length & 3)
            {
                dma_length = (dma_length + 3) & ~3;
            }

			//mame_printf_debug("PI DMA: %08X to %08X, length %08X\n", pi_cart_addr, pi_dram_addr, dma_length);

			if (pi_dram_addr != 0xffffffff)
			{
				for (i=0; i < dma_length; i++)
				{
					/*UINT32 d = memory_read_dword(space, pi_cart_addr);
                    memory_write_dword(space, pi_dram_addr, d);
                    pi_cart_addr += 4;
                    pi_dram_addr += 4;*/

					UINT8 b = memory_read_byte(space, pi_cart_addr);
					memory_write_byte(space, pi_dram_addr & 0x1fffffff, b);
					pi_cart_addr += 1;
					pi_dram_addr += 1;
				}
			}
			signal_rcp_interrupt(space->machine, PI_INTERRUPT);

			if (pi_first_dma)
			{
				// TODO: CIC-6105 has different address...
				memory_write_dword(space, 0x00000318, 0x400000);
				memory_write_dword(space, 0x000003f0, 0x800000);
				pi_first_dma = 0;
			}

			break;
		}

		case 0x10/4:		// PI_STATUS_REG
		{
			if (data & 0x2)
			{
				clear_rcp_interrupt(space->machine, PI_INTERRUPT);
			}
			break;
		}

        case 0x14/4:        // PI_BSD_DOM1_LAT
            pi_bsd_dom1_lat = data;
            break;

        case 0x18/4:        // PI_BSD_DOM1_PWD
            pi_bsd_dom1_pwd = data;
            break;

        case 0x1c/4:        // PI_BSD_DOM1_PGS
            pi_bsd_dom1_pgs = data;
            break;

        case 0x20/4:        // PI_BSD_DOM1_RLS
            pi_bsd_dom1_rls = data;
            break;

        case 0x24/4:        // PI_BSD_DOM2_LAT
            pi_bsd_dom2_lat = data;
            break;

        case 0x28/4:        // PI_BSD_DOM2_PWD
            pi_bsd_dom2_pwd = data;
            break;

        case 0x2c/4:        // PI_BSD_DOM2_PGS
            pi_bsd_dom2_pgs = data;
            break;

        case 0x30/4:        // PI_BSD_DOM2_RLS
            pi_bsd_dom2_rls = data;
            break;

		default:
			logerror("pi_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}
}

// RDRAM Interface
static UINT32 ri_mode = 0;
static UINT32 ri_config = 0;
static UINT32 ri_current_load = 0;
static UINT32 ri_select = 0;
static UINT32 ri_count = 0;
static UINT32 ri_latency = 0;
static UINT32 ri_rerror = 0;
static UINT32 ri_werror = 0;

READ32_HANDLER( n64_ri_reg_r )
{
    //printf( "n64_ri_reg_r: 0x%02x/4 (%08x)\n", offset << 2, mem_mask );
	switch (offset)
	{
        case 0x00/4: // RI_MODE_REG
            return ri_mode;

        case 0x04/4: // RI_CONFIG_REG
            return ri_config;

        case 0x08/4: // RI_CURRENT_LOAD_REG
            return ri_current_load;

        case 0x0c/4: // RI_SELECT_REG
            return ri_select;

        case 0x10/4: // RI_COUNT_REG
            return ri_count;

        case 0x14/4: // RI_LATENCY_REG
            return ri_latency;

        case 0x18/4: // RI_RERROR_REG
            return ri_rerror;

        case 0x1c/4: // RI_WERROR_REG
            return ri_werror;

		default:
			logerror("ri_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}

	return 0;
}

WRITE32_HANDLER( n64_ri_reg_w )
{
    //printf( "n64_ri_reg_w: 0x%02x/4 = %08x (%08x)\n", offset << 2, data, mem_mask );
	switch (offset)
	{
        case 0x00/4: // RI_MODE_REG
            ri_mode = data;
            break;

        case 0x04/4: // RI_CONFIG_REG
            ri_config = data;
            break;

        case 0x08/4: // RI_CURRENT_LOAD_REG
            ri_current_load = data;
            break;

        case 0x0c/4: // RI_SELECT_REG
            ri_select = data;
            break;

        case 0x10/4: // RI_COUNT_REG
            ri_count = data;
            break;

        case 0x14/4: // RI_LATENCY_REG
            ri_latency = data;
            break;

        case 0x18/4: // RI_RERROR_REG
            ri_rerror = data;
            break;

        case 0x1c/4: // RI_WERROR_REG
            ri_werror = data;
            break;

		default:
			logerror("ri_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
			break;
	}
}

// Serial Interface
static UINT8 pif_ram[0x40];
static UINT8 pif_cmd[0x40];
static UINT32 si_dram_addr = 0;
static UINT32 si_pif_addr = 0;
static UINT32 si_pif_addr_rd64b = 0;
static UINT32 si_pif_addr_wr64b = 0;
static UINT32 si_status = 0;

static UINT8 eeprom[512];
static UINT8 mempack[0x8000];

static UINT8 calc_mempack_crc(UINT8 *buffer, int length)
{
	int i, j;
	UINT32 crc = 0;
	UINT32 temp2 = 0;

	for (i=0; i <= length; i++)
	{
		for (j=7; j >= 0; j--)
		{
			if ((crc & 0x80) != 0)
			{
				temp2 = 0x85;
			}
			else
			{
				temp2 = 0;
			}

			crc <<= 1;

			if (i == length)
			{
				crc &= 0xff;
			}
			else
			{
				if ((buffer[i] & (1 << j)) != 0)
				{
					crc |= 0x1;
				}
			}

			crc ^= temp2;
		}
	}

	return crc;
}

static int pif_channel_handle_command(running_machine *machine, int channel, int slength, UINT8 *sdata, int rlength, UINT8 *rdata)
{
	int i;
	UINT8 command = sdata[0];

	switch (command)
	{
		case 0x00:		// Read status
		{
			if (slength != 1 || rlength != 3)
			{
				// osd_die("handle_pif: read status (bytes to send %d, bytes to receive %d)\n", bytes_to_send, bytes_to_recv);
			}

			switch (channel)
			{
				case 0:
				case 1:
				{
					rdata[0] = 0x05;
					rdata[1] = 0x00;
					rdata[2] = 0x02;
					return 0;
				}
				case 2:
				case 3:
				{
					// not connected
					return 1;
				}
				case 4:
				{
					rdata[0] = 0x00;
					rdata[1] = 0x80;
					rdata[2] = 0x00;
					//rdata[0] = 0xff;
					//rdata[1] = 0xff;
					//rdata[2] = 0xff;

					return 1;
				}
				case 5:
				{
					mame_printf_debug("EEPROM2? read status\n");
					return 1;
				}
			}

			break;
		}

		case 0x01:		// Read button values
		{
			UINT16 buttons = 0;
			INT8 x = 0, y = 0;
			/* add here tags for P3 and P4 when implemented */
			static const char *const portnames[] = { "P1", "P1_ANALOG_X", "P1_ANALOG_Y", "P2", "P2_ANALOG_X", "P2_ANALOG_Y" };

			if (slength != 1 || rlength != 4)
			{
				fatalerror("handle_pif: read button values (bytes to send %d, bytes to receive %d)\n", slength, rlength);
			}

			switch (channel)
			{
				case 0: //p1 inputs
				case 1: //p2 inputs
				{
                    buttons = input_port_read(machine, portnames[(channel*3) + 0]);
                    x = input_port_read(machine, portnames[(channel*3) + 1]) - 128;
                    y = input_port_read(machine, portnames[(channel*3) + 2]) - 128;

					rdata[0] = (buttons >> 8) & 0xff;
					rdata[1] = (buttons >> 0) & 0xff;
					rdata[2] = (UINT8)(x);
					rdata[3] = (UINT8)(y);
					return 0;
				}
				case 2:
				case 3:
				{
					// not connected
					return 1;
				}
			}

			break;
		}

		case 0x02:
		{
			UINT32 address, checksum;

			/*mame_printf_debug("Read from mempack, rlength = %d, slength = %d\n", rlength, slength);
            for (i=0; i < slength; i++)
            {
                mame_printf_debug("%02X ", sdata[i]);
            }
            mame_printf_debug("\n");*/

			address = (sdata[1] << 8) | (sdata[2]);
			checksum = address & 0x1f;
			address &= ~0x1f;

			if (address == 0x400)
			{
				for (i=0; i < rlength-1; i++)
				{
					rdata[i] = 0x00;
				}

				rdata[rlength-1] = calc_mempack_crc(rdata, rlength-1);
		//      mame_printf_debug("CRC = %02X\n", rdata[rlength-1]);
			}
			else if (address < 0x7fe0)
			{
				for (i=0; i < rlength-1; i++)
				{
					rdata[i] = mempack[address+i];
				}

				rdata[rlength-1] = calc_mempack_crc(rdata, rlength-1);
		//      mame_printf_debug("CRC = %02X\n", rdata[rlength-1]);
			}
			return 1;
		}
		case 0x03:
		{
			UINT32 address, checksum;
			/*mame_printf_debug("Write to mempack, rlength = %d, slength = %d\n", rlength, slength);
            for (i=0; i < slength; i++)
            {
                mame_printf_debug("%02X ", sdata[i]);
            }
            mame_printf_debug("\n");*/

			address = (sdata[1] << 8) | (sdata[2]);
			checksum = address & 0x1f;
			address &= ~0x1f;

			if (address == 0x8000)
			{

			}
			else
			{
				for (i=3; i < slength; i++)
				{
					mempack[address++] = sdata[i];
				}
			}

			rdata[0] = calc_mempack_crc(&sdata[3], slength-3);

			return 1;
		}

		case 0x04:		// Read from EEPROM
		{
			UINT8 block_offset;

			if (channel != 4)
			{
				//fatalerror("Tried to write to EEPROM on channel %d\n", channel);
				return 1;
			}

			if (slength != 2 || rlength != 8)
			{
				fatalerror("handle_pif: write EEPROM (bytes to send %d, bytes to receive %d)\n", slength, rlength);
			}

			block_offset = sdata[1] * 8;

			for (i=0; i < 8; i++)
			{
				rdata[i] = eeprom[block_offset+i];
			}

			return 1;
		}

		case 0x05:		// Write to EEPROM
		{
			UINT8 block_offset;

			if (channel != 4)
			{
				//fatalerror("Tried to write to EEPROM on channel %d\n", channel);
				return 1;
			}

			if (slength != 10 || rlength != 1)
			{
				fatalerror("handle_pif: write EEPROM (bytes to send %d, bytes to receive %d)\n", slength, rlength);
			}

			block_offset = sdata[1] * 8;
			//mame_printf_debug("Write EEPROM: offset %02X: ", block_offset);
			for (i=0; i < 8; i++)
			{
				//mame_printf_debug("%02X ", sdata[2+i]);
				eeprom[block_offset+i] = sdata[2+i];
			}
			//mame_printf_debug("\n");

			//rdata[0] = 0;

			return 1;
		}

		case 0xff:		// reset
		{
			rdata[0] = 0xff;
			rdata[1] = 0xff;
			rdata[2] = 0xff;
			return 0;
		}

		default:
		{
			mame_printf_debug("handle_pif: unknown/unimplemented command %02X\n", command);
			return 1;
		}
	}

	return 0;
}

static void handle_pif(running_machine *machine)
{
	int j;

	/*
    {
        int i;
        for (i=0; i < 8; i++)
        {
            int j = i * 8;
            mame_printf_debug("PIFCMD%d: %02X %02X %02X %02X %02X %02X %02X %02X\n", i, pif_cmd[j], pif_cmd[j+1], pif_cmd[j+2], pif_cmd[j+3], pif_cmd[j+4], pif_cmd[j+5], pif_cmd[j+6], pif_cmd[j+7]);
        }
        mame_printf_debug("\n");
    }
    */

	if (pif_cmd[0x3f] == 0x1)		// only handle the command if the last byte is 1
	{
		int channel = 0;
		int end = 0;
		int cmd_ptr = 0;

		while (cmd_ptr < 0x3f && !end)
		{
			UINT8 bytes_to_send;
			INT8 bytes_to_recv;

			bytes_to_send = pif_cmd[cmd_ptr++];

			if (bytes_to_send == 0xfe)
			{
				end = 1;
			}
			else if (bytes_to_send == 0xff)
			{
				// do nothing
			}
			else
			{
				if (bytes_to_send > 0 && (bytes_to_send & 0xc0) == 0)
				{
					int res;
					UINT8 recv_buffer[0x40];
					UINT8 send_buffer[0x40];

					bytes_to_recv = pif_cmd[cmd_ptr++];

					for (j=0; j < bytes_to_send; j++)
					{
						send_buffer[j] = pif_cmd[cmd_ptr++];
					}

                    res = pif_channel_handle_command(machine, channel, bytes_to_send, send_buffer, bytes_to_recv, recv_buffer);

					if (res == 0)
					{
						if (cmd_ptr + bytes_to_recv > 0x3f)
						{
							fatalerror("cmd_ptr overflow\n");
						}
						for (j=0; j < bytes_to_recv; j++)
						{
							pif_ram[cmd_ptr++] = recv_buffer[j];
						}
					}
					else if (res == 1)
					{
						int offset = 0;//bytes_to_send;
						pif_ram[cmd_ptr-offset-2] |= 0x80;
					}
				}

				channel++;
			}
		}

		pif_ram[0x3f] = 0;
	}

	/*
    {
        int i;
        for (i=0; i < 8; i++)
        {
            int j = i * 8;
            mame_printf_debug("PIFRAM%d: %02X %02X %02X %02X %02X %02X %02X %02X\n", i, pif_ram[j], pif_ram[j+1], pif_ram[j+2], pif_ram[j+3], pif_ram[j+4], pif_ram[j+5], pif_ram[j+6], pif_ram[j+7]);
        }
        mame_printf_debug("\n");
    }
    */
}

static void pif_dma(running_machine *machine, int direction)
{
	int i;
	UINT32 *src, *dst;

	if (si_dram_addr & 0x3)
	{
		fatalerror("pif_dma: si_dram_addr unaligned: %08X\n", si_dram_addr);
	}

	if (direction)		// RDRAM -> PIF RAM
	{
		src = (UINT32*)&rdram[(si_dram_addr & 0x1fffffff) / 4];

		for (i=0; i < 64; i+=4)
		{
			UINT32 d = *src++;
			pif_ram[i+0] = (d >> 24) & 0xff;
			pif_ram[i+1] = (d >> 16) & 0xff;
			pif_ram[i+2] = (d >>  8) & 0xff;
			pif_ram[i+3] = (d >>  0) & 0xff;
		}

		memcpy(pif_cmd, pif_ram, 0x40);
	}
	else				// PIF RAM -> RDRAM
	{
		handle_pif(machine);

		dst = (UINT32*)&rdram[(si_dram_addr & 0x1fffffff) / 4];

		for (i=0; i < 64; i+=4)
		{
			UINT32 d = 0;
			d |= pif_ram[i+0] << 24;
			d |= pif_ram[i+1] << 16;
			d |= pif_ram[i+2] <<  8;
			d |= pif_ram[i+3] <<  0;

			*dst++ = d;
		}
	}

	si_status |= 0x1000;
	signal_rcp_interrupt(machine, SI_INTERRUPT);
}

READ32_HANDLER( n64_si_reg_r )
{
	switch (offset)
	{
		//case 0x00/4:      // SI_DRAM_ADDR_REG
			//return si_dram_addr;

		case 0x18/4:		// SI_STATUS_REG
			return si_status;
	}
	return 0;
}

WRITE32_HANDLER( n64_si_reg_w )
{
	switch (offset)
	{
		case 0x00/4:		// SI_DRAM_ADDR_REG
			si_dram_addr = data;
	//      mame_printf_debug("si_dram_addr = %08X\n", si_dram_addr);
			break;

		case 0x04/4:		// SI_PIF_ADDR_RD64B_REG
			// PIF RAM -> RDRAM
			si_pif_addr = data;
            si_pif_addr_rd64b = data;
            pif_dma(space->machine, 0);
			break;

		case 0x10/4:		// SI_PIF_ADDR_WR64B_REG
			// RDRAM -> PIF RAM
			si_pif_addr = data;
            si_pif_addr_wr64b = data;
            pif_dma(space->machine, 1);
			break;

		case 0x18/4:		// SI_STATUS_REG
			si_status &= ~0x1000;
			clear_rcp_interrupt(space->machine, SI_INTERRUPT);
			break;

		default:
			logerror("si_reg_w: %08X, %08X, %08X\n", data, offset, mem_mask);
			break;
	}
}

static UINT32 cic_status = 0x00000000;

READ32_HANDLER( n64_pif_ram_r )
{
    /*mame_printf_debug( "pif_ram_r: %08X, %08X = %08X\n", offset << 2, mem_mask, ( ( pif_ram[offset*4+0] << 24 ) | ( pif_ram[offset*4+1] << 16 ) | ( pif_ram[offset*4+2] <<  8 ) | ( pif_ram[offset*4+3] <<  0 ) ) & mem_mask );*/
    if(!space->debugger_access)
    {
    	if( offset == ( 0x24 / 4 ) )
    	{
    	    cic_status = 0x00000080;
    	}
    	if( offset == ( 0x3C / 4 ) )
    	{
    	    return cic_status;
    	}
	}
    return ( ( pif_ram[offset*4+0] << 24 ) | ( pif_ram[offset*4+1] << 16 ) | ( pif_ram[offset*4+2] <<  8 ) | ( pif_ram[offset*4+3] <<  0 ) ) & mem_mask;
}

WRITE32_HANDLER( n64_pif_ram_w )
{
    /*mame_printf_debug("pif_ram_w: %08X, %08X, %08X\n", data, offset << 4, mem_mask);*/
    if( mem_mask & 0xff000000 )
    {
        pif_ram[offset*4+0] = ( data >> 24 ) & 0x000000ff;
    }
    if( mem_mask & 0x00ff0000 )
    {
        pif_ram[offset*4+1] = ( data >> 16 ) & 0x000000ff;
    }
    if( mem_mask & 0x0000ff00 )
    {
        pif_ram[offset*4+2] = ( data >>  8 ) & 0x000000ff;
    }
    if( mem_mask & 0x000000ff )
    {
        pif_ram[offset*4+3] = ( data >>  0 ) & 0x000000ff;
    }

    signal_rcp_interrupt(space->machine, SI_INTERRUPT);
}

//static UINT16 crc_seed = 0x3f;

MACHINE_START( n64 )
{
	mips3drc_set_options(devtag_get_device(machine, "maincpu"), MIPS3DRC_FASTEST_OPTIONS + MIPS3DRC_STRICT_VERIFY);

	/* configure fast RAM regions for DRC */
	mips3drc_add_fastram(devtag_get_device(machine, "maincpu"), 0x00000000, 0x007fffff, FALSE, rdram);

	rspdrc_set_options(devtag_get_device(machine, "rsp"), 0);
	rspdrc_add_imem(devtag_get_device(machine, "rsp"), rsp_imem);
	rspdrc_add_dmem(devtag_get_device(machine, "rsp"), rsp_dmem);
	rspdrc_flush_drc_cache(devtag_get_device(machine, "rsp"));

	audio_timer = timer_alloc(machine, audio_timer_callback, NULL);
}

MACHINE_RESET( n64 )
{
	int i;
	//UINT32 *pif_rom   = (UINT32*)memory_region(machine, "user1");
	UINT32 *cart = (UINT32*)memory_region(machine, "user2");
	UINT64 boot_checksum;

	mi_version = 0;
	mi_interrupt = 0;
	mi_intr_mask = 0;
	mi_mode = 0;

	sp_mem_addr = 0;
	sp_dram_addr = 0;
	sp_dma_length = 0;
	sp_dma_count = 0;
	sp_dma_skip = 0;
	sp_semaphore = 0;

	dp_start = 0;
	dp_end = 0;
	dp_current = 0;
	dp_status = 0x88;

	n64_vi_width = 0;
	n64_vi_origin = 0;
	n64_vi_control = 0;
	n64_vi_burst = n64_vi_vsync = n64_vi_hsync = n64_vi_leap = n64_vi_hstart = n64_vi_vstart = 0;
	n64_vi_intr = n64_vi_vburst = n64_vi_xscale = n64_vi_yscale = 0;

	ai_dram_addr = 0;
	ai_len = 0 ;
	ai_control = 0;
	ai_dacrate = 0;
	ai_bitrate = 0;
	ai_status = 0;

	memset(audio_fifo, 0, sizeof(audio_fifo));
	audio_fifo_wpos = 0;
	audio_fifo_rpos = 0;
	audio_fifo_num = 0;

	pi_dram_addr = 0;
	pi_cart_addr = 0;
	pi_first_dma = 1;

	memset(pif_ram, 0, sizeof(pif_ram));
	memset(pif_cmd, 0, sizeof(pif_cmd));
	si_dram_addr = 0;
	si_pif_addr = 0;
	si_status = 0;

	memset(eeprom, 0, sizeof(eeprom));
	memset(mempack, 0, sizeof(mempack));

	cic_status = 0;

	timer_adjust_oneshot(audio_timer, attotime_never, 0);

	cputag_set_input_line(machine, "rsp", INPUT_LINE_HALT, ASSERT_LINE);

    // bootcode differs between CIC-chips, so we can use its checksum to detect the CIC-chip
    boot_checksum = 0;
    for (i=0x40; i < 0x1000; i+=4)
    {
        boot_checksum += cart[i/4]+i;
    }

    if (boot_checksum == U64(0x000000d057e84864))
    {
        // CIC-NUS-6101
        printf("CIC-NUS-6102 detected\n");
        pif_ram[0x24] = 0x00;
        pif_ram[0x25] = 0x02;
        pif_ram[0x26] = 0x3f;
        pif_ram[0x27] = 0x3f;
        // crc_seed = 0x3f;
    }
    else if (boot_checksum == U64(0x000000cffb830843) || boot_checksum == U64(0x000000d0027fdf31))
    {
        // CIC-NUS-6103
        printf("CIC-NUS-6101 detected\n");
        // crc_seed = 0x78;
        pif_ram[0x24] = 0x00;
        pif_ram[0x25] = 0x06;
        pif_ram[0x26] = 0x3f;
        pif_ram[0x27] = 0x3f;
    }
    else if (boot_checksum == U64(0x000000d6499e376b))
    {
        // CIC-NUS-6103
        printf("CIC-NUS-6103 detected\n");
        // crc_seed = 0x78;
        pif_ram[0x24] = 0x00;
        pif_ram[0x25] = 0x02;
        pif_ram[0x26] = 0x78;
        pif_ram[0x27] = 0x3f;
    }
    else if (boot_checksum == U64(0x0000011a4a1604b6))
    {
        // CIC-NUS-6105
        printf("CIC-NUS-6105 detected\n");
        // crc_seed = 0x91;

        // first_rsp = 0;
        pif_ram[0x24] = 0x00;
        pif_ram[0x25] = 0x02;
        pif_ram[0x26] = 0x91;
        pif_ram[0x27] = 0x3f;
    }
    else if (boot_checksum == U64(0x000000d6d5de4ba0))
    {
        // CIC-NUS-6106
        printf("CIC-NUS-6106 detected\n");
        // crc_seed = 0x85;
        pif_ram[0x24] = 0x00;
        pif_ram[0x25] = 0x02;
        pif_ram[0x26] = 0x85;
        pif_ram[0x27] = 0x3f;
    }
    else
    {
        printf("Unknown BootCode Checksum %08X%08X\n", (UINT32)(boot_checksum>>32),(UINT32)(boot_checksum));
    }
}

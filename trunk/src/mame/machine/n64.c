/* machine/n64.c - contains N64 hardware emulation shared between MAME and MESS */

#include "emu.h"
#include "debugger.h"
#include "cpu/mips/mips3.h"
#include "cpu/mips/mips3com.h"
#include "includes/n64.h"
#include "profiler.h"

UINT32 *rdram;
UINT32 *rsp_imem;
UINT32 *rsp_dmem;

// device type definition
const device_type N64PERIPH = &device_creator<n64_periphs>;

static TIMER_CALLBACK(ai_timer_callback);
static TIMER_CALLBACK(pi_dma_callback);

n64_periphs::n64_periphs(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, N64PERIPH, "N64 Periphal Chips", tag, owner, clock)
{
}


void n64_periphs::device_start()
{
	ai_timer = machine().scheduler().timer_alloc(FUNC(ai_timer_callback));
	pi_dma_timer = machine().scheduler().timer_alloc(FUNC(pi_dma_callback));
}

void n64_periphs::device_reset()
{
	UINT32 *cart = (UINT32*)machine().region("user2")->base();

	maincpu = machine().device("maincpu");
	rspcpu = machine().device("rsp");
	mem_map = maincpu->memory().space(AS_PROGRAM);

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

	vi_width = 0;
	vi_origin = 0;
	vi_control = 0;
	vi_burst = 0;
	vi_vsync = 0;
	vi_hsync = 0;
	vi_leap = 0;
	vi_hstart = 0;
	vi_vstart = 0;
	vi_intr = 0;
	vi_vburst = 0;
	vi_xscale = 0;
	vi_yscale = 0;

	ai_dac[0] = machine().device<dmadac_sound_device>("dac1");
	ai_dac[1] = machine().device<dmadac_sound_device>("dac2");
	ai_timer->adjust(attotime::never);
	memset(ai_fifo, 0, sizeof(ai_fifo));
	ai_fifo_wpos = 0;
	ai_fifo_rpos = 0;
	ai_fifo_num = 0;
	ai_dram_addr = 0;
	ai_len = 0 ;
	ai_control = 0;
	ai_dacrate = 0;
	ai_bitrate = 0;
	ai_status = 0;

	pi_dma_timer->adjust(attotime::never);
	pi_first_dma = 1;
	pi_rd_len = 0;
	pi_wr_len = 0;
	pi_status = 0;
	pi_bsd_dom1_lat = 0;
	pi_bsd_dom1_pwd = 0;
	pi_bsd_dom1_pgs = 0;
	pi_bsd_dom1_rls = 0;
	pi_bsd_dom2_lat = 0;
	pi_bsd_dom2_pwd = 0;
	pi_bsd_dom2_pgs = 0;
	pi_bsd_dom2_rls = 0;
	pi_dma_dir = 0;

	memset(pif_ram, 0, sizeof(pif_ram));
	memset(pif_cmd, 0, sizeof(pif_cmd));
	si_dram_addr = 0;
	si_pif_addr = 0;
	si_status = 0;

	memset(eeprom, 0, sizeof(eeprom));
	memset(mempack, 0, sizeof(mempack));

	dp_clock = 0;

	cic_status = 0;

    // bootcode differs between CIC-chips, so we can use its checksum to detect the CIC-chip
    UINT64 boot_checksum = 0;
    for(int i = 0x40; i < 0x1000; i+=4)
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

READ32_MEMBER( n64_periphs::mi_reg_r )
{
	//printf("mi_reg_r %08x\n", offset * 4);
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
			logerror("mi_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(&mem_map->device()));
			break;
	}

	return 0;
}

WRITE32_MEMBER( n64_periphs::mi_reg_w )
{
	//printf("mi_reg_w %08x %08x %08x\n", offset * 4, data, mem_mask);
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
				clear_rcp_interrupt(DP_INTERRUPT);
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
			logerror("mi_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(maincpu));
			break;
	}
}

void signal_rcp_interrupt(running_machine &machine, int interrupt)
{
	machine.device<n64_periphs>("rcp")->signal_rcp_interrupt(interrupt);
}

void n64_periphs::signal_rcp_interrupt(int interrupt)
{
	if (mi_intr_mask & interrupt)
	{
		mi_interrupt |= interrupt;

		cputag_set_input_line(machine(), "maincpu", INPUT_LINE_IRQ0, ASSERT_LINE);
	}
}

void n64_periphs::clear_rcp_interrupt(int interrupt)
{
	mi_interrupt &= ~interrupt;

	//if (!mi_interrupt)
	{
		cputag_set_input_line(machine(), "maincpu", INPUT_LINE_IRQ0, CLEAR_LINE);
	}
}

READ32_MEMBER( n64_periphs::is64_r )
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

WRITE32_MEMBER( n64_periphs::is64_w )
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

READ32_MEMBER( n64_periphs::open_r )
{
    UINT32 retval = (offset << 2) & 0x0000ffff;
    retval = (retval << 16) | retval;
    return retval;
}

WRITE32_MEMBER( n64_periphs::open_w )
{
    // Do nothing
}

// RDRAM Interface (RI)

#define RDRAM_CONFIG		(0)
#define RDRAM_DEVICE_ID 	(1)
#define RDRAM_DELAY			(2)
#define RDRAM_MODE			(3)
#define RDRAM_REF_INTERVAL	(4)
#define RDRAM_REF_ROW		(5)
#define RDRAM_RAS_INTERVAL	(6)
#define RDRAM_MIN_INTERVAL	(7)
#define RDRAM_ADDR_SELECT	(8)
#define RDRAM_DEVICE_MANUF	(9)

READ32_MEMBER( n64_periphs::rdram_reg_r )
{
	//printf("rdram_reg_r %08x\n", offset * 4);
	if(offset > 0x24/4)
	{
		logerror("rdram_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(maincpu));
		return 0;
	}
	return rdram_regs[offset];
}

WRITE32_MEMBER( n64_periphs::rdram_reg_w )
{
	//printf("rdram_reg_w %08x %08x %08x\n", offset * 4, data, mem_mask);
	if(offset > 0x24/4)
	{
		logerror("mi_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(maincpu));
		return;
	}
	COMBINE_DATA(&rdram_regs[offset]);
}

// RSP Interface

void n64_periphs::sp_dma(int direction)
{
	sp_dma_length++;
	if ((sp_dma_length & 7) != 0)
	{
        sp_dma_length = (sp_dma_length + 7) & ~7;
	}

	if (sp_mem_addr & 0x3)
	{
        sp_mem_addr = sp_mem_addr & ~3;
	}
	if (sp_dram_addr & 0x7)
	{
        sp_dram_addr = sp_dram_addr & ~7;
	}

	if ((sp_mem_addr & 0xfff) + (sp_dma_length) > 0x1000)
	{
		printf("sp_dma: dma out of memory area: %08X, %08X\n", sp_mem_addr, sp_dma_length);
		//fatalerror("sp_dma: dma out of memory area: %08X, %08X\n", sp_mem_addr, sp_dma_length);
		sp_dma_length = 0x1000 - (sp_mem_addr & 0xfff);
	}

	UINT32 *sp_mem[2] = { rsp_dmem, rsp_imem };

	if(direction == 0)// RDRAM -> I/DMEM
	{
        for(int c = 0; c <= sp_dma_count; c++)
        {
            UINT32 src = (sp_dram_addr & 0x007fffff) >> 2;
            UINT32 dst = (sp_mem_addr & 0x1fff) >> 2;

            for(int i = 0; i < sp_dma_length / 4; i++)
            {
				sp_mem[(dst + i) >> 10][(dst + i) & 0x3ff] = rdram[src + i];
            }

            sp_mem_addr += sp_dma_length;
            sp_dram_addr += sp_dma_length;

            sp_mem_addr += sp_dma_skip;
        }
	}
	else					// I/DMEM -> RDRAM
	{
        for(int c = 0; c <= sp_dma_count; c++)
        {
            UINT32 src = (sp_mem_addr & 0x1fff) >> 2;
            UINT32 dst = (sp_dram_addr & 0x007fffff) >> 2;

            for(int i = 0; i < sp_dma_length / 4; i++)
            {
				rdram[dst + i] = sp_mem[(src + i) >> 10][(src + i) & 0x3ff];
            }

            sp_mem_addr += sp_dma_length;
            sp_dram_addr += sp_dma_length;

            sp_dram_addr += sp_dma_skip;
        }
	}
}

static void sp_set_status(device_t *device, UINT32 status)
{
	device->machine().device<n64_periphs>("rcp")->sp_set_status(status);
}

void n64_periphs::sp_set_status(UINT32 status)
{
	if (status & 0x1)
	{
		device_set_input_line(rspcpu, INPUT_LINE_HALT, ASSERT_LINE);
        cpu_set_reg(rspcpu, RSP_SR, cpu_get_reg(rspcpu, RSP_SR) | RSP_STATUS_HALT);
	}

	if (status & 0x2)
	{
        cpu_set_reg(rspcpu, RSP_SR, cpu_get_reg(rspcpu, RSP_SR) | RSP_STATUS_BROKE);

        if (cpu_get_reg(rspcpu, RSP_SR) & RSP_STATUS_INTR_BREAK)
		{
			signal_rcp_interrupt(SP_INTERRUPT);
		}
	}
}

UINT32 n64_periphs::sp_reg_r(UINT32 offset)
{
	//printf("sp_reg_r %08x\n", offset * 4);
	switch (offset)
	{
		case 0x00/4:		// SP_MEM_ADDR_REG
			return sp_mem_addr;

		case 0x04/4:		// SP_DRAM_ADDR_REG
			return sp_dram_addr;

		case 0x08/4:		// SP_RD_LEN_REG
			return (sp_dma_skip << 20) | (sp_dma_count << 12) | sp_dma_length;

		case 0x10/4:		// SP_STATUS_REG
            return cpu_get_reg(rspcpu, RSP_SR);

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
            return cpu_get_reg(rspcpu, RSP_PC) & 0x00000fff;

        default:
            logerror("sp_reg_r: %08X at %08X\n", offset, cpu_get_pc(maincpu));
            break;
	}

	return 0;
}

READ32_DEVICE_HANDLER( n64_sp_reg_r )
{
	return device->machine().device<n64_periphs>("rcp")->sp_reg_r(offset);
}

void n64_periphs::sp_reg_w(UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	//printf("sp_reg_w %08x %08x %08x\n", offset * 4, data, mem_mask);
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
            	UINT32 oldstatus = cpu_get_reg(rspcpu, RSP_SR);
            	UINT32 newstatus = oldstatus;

                // printf( "RSP_STATUS_REG Write; %08x\n", data );
                if (data & 0x00000001)      // clear halt
                {
					device_set_input_line(rspcpu, INPUT_LINE_HALT, CLEAR_LINE);
					newstatus &= ~RSP_STATUS_HALT;
                }
                if (data & 0x00000002)      // set halt
                {
                    device_set_input_line(rspcpu, INPUT_LINE_HALT, ASSERT_LINE);
                    newstatus |= RSP_STATUS_HALT;
                }
                if (data & 0x00000004)
                {
                    newstatus &= ~RSP_STATUS_BROKE;
                }
                if (data & 0x00000008)      // clear interrupt
                {
                    clear_rcp_interrupt(SP_INTERRUPT);
                }
                if (data & 0x00000010)      // set interrupt
                {
                    signal_rcp_interrupt(SP_INTERRUPT);
                }
                if (data & 0x00000020)
                {
                    newstatus &= ~RSP_STATUS_SSTEP;
                }
                if (data & 0x00000040)
                {
                    newstatus |= RSP_STATUS_SSTEP;	// set single step
                    if(!(oldstatus & (RSP_STATUS_BROKE | RSP_STATUS_HALT)))
                    {
                        cpu_set_reg(rspcpu, RSP_STEPCNT, 1 );
                    }
                }
                if (data & 0x00000080)
                {
                    newstatus &= ~RSP_STATUS_INTR_BREAK;	// clear interrupt on break
                }
                if (data & 0x00000100)
                {
                    newstatus |= RSP_STATUS_INTR_BREAK;		// set interrupt on break
                }
                if (data & 0x00000200)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL0;		// clear signal 0
                }
                if (data & 0x00000400)
                {
                    newstatus |= RSP_STATUS_SIGNAL0;		// set signal 0
                }
                if (data & 0x00000800)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL1;		// clear signal 1
                }
                if (data & 0x00001000)
                {
                    newstatus |= RSP_STATUS_SIGNAL1;		// set signal 1
                }
                if (data & 0x00002000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL2 ;		// clear signal 2
                }
                if (data & 0x00004000)
                {
                    newstatus |= RSP_STATUS_SIGNAL2;		// set signal 2
                }
                if (data & 0x00008000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL3;		// clear signal 3
                }
                if (data & 0x00010000)
                {
                    newstatus |= RSP_STATUS_SIGNAL3;		// set signal 3
                }
                if (data & 0x00020000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL4;		// clear signal 4
                }
                if (data & 0x00040000)
                {
                    newstatus |= RSP_STATUS_SIGNAL4;		// set signal 4
                }
                if (data & 0x00080000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL5;		// clear signal 5
                }
                if (data & 0x00100000)
                {
                    newstatus |= RSP_STATUS_SIGNAL5;		// set signal 5
                }
                if (data & 0x00200000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL6;		// clear signal 6
                }
                if (data & 0x00400000)
                {
                    newstatus |= RSP_STATUS_SIGNAL6;		// set signal 6
                }
                if (data & 0x00800000)
                {
                    newstatus &= ~RSP_STATUS_SIGNAL7;		// clear signal 7
                }
                if (data & 0x01000000)
                {
                    newstatus |= RSP_STATUS_SIGNAL7;		// set signal 7
                }
                cpu_set_reg(rspcpu, RSP_SR, newstatus);
                break;
            }

			case 0x1c/4:		// SP_SEMAPHORE_REG
				if(data == 0)
				{
                	sp_semaphore = 0;
				}
				break;

			default:
				logerror("sp_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(maincpu));
				break;
		}
	}
	else
	{
        switch (offset & 0xffff)
        {
            case 0x00/4:        // SP_PC_REG
                if( cpu_get_reg(rspcpu, RSP_NEXTPC) != 0xffffffff )
                {
                    cpu_set_reg(rspcpu, RSP_NEXTPC, 0x1000 | (data & 0xfff));
                }
                else
                {
                    cpu_set_reg(rspcpu, RSP_PC, 0x1000 | (data & 0xfff));
                }
                break;

            default:
                logerror("sp_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(maincpu));
                break;
		}
	}
}

WRITE32_DEVICE_HANDLER( n64_sp_reg_w )
{
	device->machine().device<n64_periphs>("rcp")->sp_reg_w(offset, data, mem_mask);
}

// RDP Interface

void dp_full_sync(running_machine &machine)
{
	signal_rcp_interrupt(machine, DP_INTERRUPT);
}

READ32_DEVICE_HANDLER( n64_dp_reg_r )
{
	_n64_state *state = device->machine().driver_data<_n64_state>();

	//printf("%08x\n", offset);
	switch (offset)
	{
		case 0x00/4:		// DP_START_REG
			return state->m_rdp.GetStartReg();

		case 0x04/4:		// DP_END_REG
			return state->m_rdp.GetEndReg();

		case 0x08/4:		// DP_CURRENT_REG
			return state->m_rdp.GetCurrentReg();

		case 0x0c/4:		// DP_STATUS_REG
			return state->m_rdp.GetStatusReg();

		default:
			logerror("dp_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(device));
			break;
	}

	return 0;
}

WRITE32_DEVICE_HANDLER( n64_dp_reg_w )
{
	_n64_state *state = device->machine().driver_data<_n64_state>();

	//printf("%08x: %08x\n", offset, data);
	switch (offset)
	{
		case 0x00/4:		// DP_START_REG
			state->m_rdp.SetStartReg(data);
			state->m_rdp.SetCurrentReg(state->m_rdp.GetStartReg());
			break;

		case 0x04/4:		// DP_END_REG
			state->m_rdp.SetEndReg(data);
			g_profiler.start(PROFILER_USER1);
			state->m_rdp.ProcessList();
			g_profiler.stop();
			break;

		case 0x0c/4:		// DP_STATUS_REG
		{
			UINT32 current_status = state->m_rdp.GetStatusReg();
			if (data & 0x00000001)	current_status &= ~DP_STATUS_XBUS_DMA;
			if (data & 0x00000002)	current_status |= DP_STATUS_XBUS_DMA;
			if (data & 0x00000004)	current_status &= ~DP_STATUS_FREEZE;
			if (data & 0x00000008)	current_status |= DP_STATUS_FREEZE;
			if (data & 0x00000010)	current_status &= ~DP_STATUS_FLUSH;
			if (data & 0x00000020)	current_status |= DP_STATUS_FLUSH;
			state->m_rdp.SetStatusReg(current_status);
			break;
		}

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
void n64_periphs::vi_recalculate_resolution()
{
	_n64_state *state = machine().driver_data<_n64_state>();

    int x_start = (vi_hstart & 0x03ff0000) >> 16;
    int x_end = vi_hstart & 0x000003ff;
    int y_start = ((vi_vstart & 0x03ff0000) >> 16) / 2;
    int y_end = (vi_vstart & 0x000003ff) / 2;
    int width = ((vi_xscale & 0x00000fff) * (x_end - x_start)) / 0x400;
    int height = ((vi_yscale & 0x00000fff) * (y_end - y_start)) / 0x400;
    rectangle visarea = machine().primary_screen->visible_area();
    attoseconds_t period = machine().primary_screen->frame_period().attoseconds;

    if (width == 0 || height == 0)
    {
        vi_blank = 1;
        /*
        FIXME: MAME doesn't handle well a h/w res of zero (otherwise it hardlocks the emu, seen especially in Aleck 64 games
        that sets the res after a longer delay than n64), guess that this just disables drawing?
        */
        return;
    }
    else
    {
        vi_blank = 0;
    }

    if (width > 640)
        width = 640;

    if (height > 480)
        height = 480;

	state->m_rdp.MiscState.FBHeight = height;

    visarea.max_x = width - 1;
    visarea.max_y = height - 1;
    machine().primary_screen->configure(width, 525, visarea, period);
}

READ32_MEMBER( n64_periphs::vi_reg_r )
{
	//printf("vi_reg_r %08x\n", offset * 4);
	switch (offset)
	{
		case 0x00/4:		// VI_CONTROL_REG
			return vi_control;

		case 0x04/4:		// VI_ORIGIN_REG
            return vi_origin;

		case 0x08/4:		// VI_WIDTH_REG
            return vi_width;

		case 0x0c/4:
            return vi_intr;

		case 0x10/4:		// VI_CURRENT_REG
			return machine().primary_screen->vpos();

		case 0x14/4:		// VI_BURST_REG
            return vi_burst;

		case 0x18/4:		// VI_V_SYNC_REG
            return vi_vsync;

		case 0x1c/4:		// VI_H_SYNC_REG
            return vi_hsync;

		case 0x20/4:		// VI_LEAP_REG
            return vi_leap;

		case 0x24/4:		// VI_H_START_REG
            return vi_hstart;

		case 0x28/4:		// VI_V_START_REG
            return vi_vstart;

		case 0x2c/4:		// VI_V_BURST_REG
            return vi_vburst;

		case 0x30/4:		// VI_X_SCALE_REG
            return vi_xscale;

		case 0x34/4:		// VI_Y_SCALE_REG
            return vi_yscale;

		default:
			logerror("vi_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(maincpu));
			break;
	}
	return 0;
}

WRITE32_MEMBER( n64_periphs::vi_reg_w )
{
	//printf("vi_reg_w %08x %08x %08x\n", offset * 4, data, mem_mask);
	_n64_state *state = machine().driver_data<_n64_state>();

	switch (offset)
	{
		case 0x00/4:		// VI_CONTROL_REG
            vi_control = data;
            vi_recalculate_resolution();
			break;

		case 0x04/4:		// VI_ORIGIN_REG
            vi_origin = data & 0xffffff;
			break;

		case 0x08/4:		// VI_WIDTH_REG
            if (vi_width != data && data > 0)
			{
                vi_recalculate_resolution();
			}
            vi_width = data;
		    state->m_rdp.MiscState.FBWidth = data;
			break;

		case 0x0c/4:		// VI_INTR_REG
            vi_intr = data;
			break;

		case 0x10/4:		// VI_CURRENT_REG
			clear_rcp_interrupt(VI_INTERRUPT);
			break;

		case 0x14/4:		// VI_BURST_REG
            vi_burst = data;
			break;

		case 0x18/4:		// VI_V_SYNC_REG
            vi_vsync = data;
			break;

		case 0x1c/4:		// VI_H_SYNC_REG
            vi_hsync = data;
			break;

		case 0x20/4:		// VI_LEAP_REG
            vi_leap = data;
			break;

		case 0x24/4:		// VI_H_START_REG
            vi_hstart = data;
            vi_recalculate_resolution();
			break;

		case 0x28/4:		// VI_V_START_REG
            vi_vstart = data;
            vi_recalculate_resolution();
			break;

		case 0x2c/4:		// VI_V_BURST_REG
            vi_vburst = data;
			break;

		case 0x30/4:		// VI_X_SCALE_REG
            vi_xscale = data;
            vi_recalculate_resolution();
			break;

		case 0x34/4:		// VI_Y_SCALE_REG
            vi_yscale = data;
            vi_recalculate_resolution();
			break;

        /*
        Uncomment this for convenient homebrew debugging
        case 0x44/4:        // TEMP DEBUG
            printf( "E Ping: %08x\n", data );
            break;
        */

		default:
			logerror("vi_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(maincpu));
			break;
	}
}


// Audio Interface
void n64_periphs::ai_fifo_push(UINT32 address, UINT32 length)
{
    if (ai_fifo_num == AUDIO_DMA_DEPTH)
    {
        mame_printf_debug("ai_fifo_push: tried to push to full DMA FIFO!!!\n");
    }

    ai_fifo[ai_fifo_wpos].address = address;
    ai_fifo[ai_fifo_wpos].length = length;

    ai_fifo_wpos++;
    ai_fifo_num++;

    if (ai_fifo_wpos >= AUDIO_DMA_DEPTH)
    {
        ai_fifo_wpos = 0;
    }

    if (ai_fifo_num >= AUDIO_DMA_DEPTH)
    {
        ai_status |= 0x80000001;    // FIFO full
    }

    if (! (ai_status & 0x40000000))
    {
        signal_rcp_interrupt(AI_INTERRUPT);
        ai_dma();
    }
}

void n64_periphs::ai_fifo_pop()
{
    ai_fifo_rpos++;
    ai_fifo_num--;

    if (ai_fifo_num < 0)
    {
        fatalerror("ai_fifo_pop: FIFO underflow!\n");
    }

    if (ai_fifo_rpos >= AUDIO_DMA_DEPTH)
    {
        ai_fifo_rpos = 0;
    }

    if (ai_fifo_num < AUDIO_DMA_DEPTH)
    {
        ai_status &= ~0x80000001;   // FIFO not full
        signal_rcp_interrupt(AI_INTERRUPT);
    }
}

n64_periphs::AUDIO_DMA *n64_periphs::ai_fifo_get_top()
{
    if (ai_fifo_num > 0)
    {
        return &ai_fifo[ai_fifo_rpos];
    }
    else
    {
        return NULL;
    }
}

#define N64_ATTOTIME_NORMALIZE(a)   do { while ((a).attoseconds >= ATTOSECONDS_PER_SECOND) { (a).seconds++; (a).attoseconds -= ATTOSECONDS_PER_SECOND; } } while (0)

void n64_periphs::ai_dma()
{
    INT16 *ram = (INT16*)rdram;
    AUDIO_DMA *current = ai_fifo_get_top();
    attotime period;

    //static FILE * audio_dump = NULL;
    //
    //if (audio_dump == NULL)
    //    audio_dump = fopen("audio_dump.raw","wb");
    //
    //fwrite(&ram[current->address/2],current->length,1,audio_dump);

    ram = &ram[current->address/2];

//  mame_printf_debug("DACDMA: %x for %x bytes\n", current->address, current->length);

    dmadac_transfer(&ai_dac[0], 2, 1, 2, current->length/4, ram);

    ai_status |= 0x40000000;

   // adjust the timer
   period = attotime::from_hz(DACRATE_NTSC) * ((ai_dacrate + 1) * (current->length / 4));
   ai_timer->adjust(period);
}

static TIMER_CALLBACK(ai_timer_callback)
{
	machine.device<n64_periphs>("rcp")->ai_timer_tick();
}

void n64_periphs::ai_timer_tick()
{
    ai_fifo_pop();

    // keep playing if there's another DMA queued
    if (ai_fifo_get_top() != NULL)
    {
        ai_dma();
        signal_rcp_interrupt(AI_INTERRUPT);
    }
    else
    {
        ai_status &= ~0x40000000;
    }
}

READ32_MEMBER( n64_periphs::ai_reg_r )
{
	//printf("ai_reg_r %08x\n", offset * 4);
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
                double secs_left = (ai_timer->expire() - machine().time()).as_double();
                unsigned int samples_left = secs_left * DACRATE_NTSC / (ai_dacrate + 1);
                return samples_left * 4;
            }
            else return 0;
        }

        case 0x0c/4:        // AI_STATUS_REG
            return ai_status;

        default:
            logerror("ai_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(maincpu));
            break;
    }

    return 0;
}

WRITE32_MEMBER( n64_periphs::ai_reg_w )
{
	//printf("ai_reg_w %08x %08x %08x\n", offset * 4, data, mem_mask);
    switch (offset)
    {
        case 0x00/4:        // AI_DRAM_ADDR_REG
            ai_dram_addr = data & 0xffffff;
            break;

        case 0x04/4:        // AI_LEN_REG
            ai_len = data & 0x3ffff;        // Hardware v2.0 has 18 bits, v1.0 has 15 bits
            ai_fifo_push(ai_dram_addr, ai_len);
            break;

        case 0x08/4:        // AI_CONTROL_REG
            ai_control = data;
            break;

        case 0x0c/4:
            clear_rcp_interrupt(AI_INTERRUPT);
            break;

        case 0x10/4:        // AI_DACRATE_REG
            ai_dacrate = data & 0x3fff;
            dmadac_set_frequency(&ai_dac[0], 2, (double)DACRATE_NTSC / (double)(ai_dacrate+1));
            //printf( "frequency: %f\n", (double)DACRATE_NTSC / (double)(ai_dacrate+1) );
            dmadac_enable(&ai_dac[0], 2, 1);
            break;

        case 0x14/4:        // AI_BITRATE_REG
            ai_bitrate = data & 0xf;
            break;

        default:
            logerror("ai_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(maincpu));
            break;
    }
}


// Peripheral Interface

static TIMER_CALLBACK(pi_dma_callback)
{
	machine.device<n64_periphs>("rcp")->pi_dma_tick();
}

void n64_periphs::pi_dma_tick()
{
	UINT16 *cart16 = (UINT16*)machine().region("user2")->base();
	UINT16 *dram16 = (UINT16*)rdram;

	UINT32 cart_addr = (pi_cart_addr & 0x0fffffff) >> 1;
	UINT32 dram_addr = (pi_dram_addr & 0x007fffff) >> 1;

    cart_addr &= ((machine().region("user2")->bytes() >> 1) - 1);

	if(pi_dma_dir == 1)
	{
		UINT32 dma_length = pi_wr_len + 1;
		if (dma_length & 3)
		{
			dma_length = (dma_length + 3) & ~3;
		}

		if (pi_dram_addr != 0xffffffff)
		{
			for(int i = 0; i < dma_length / 2; i++)
			{
				dram16[BYTE_XOR_BE(dram_addr + i)] = cart16[BYTE_XOR_BE(cart_addr + i)];
			}

			pi_cart_addr += dma_length;
			pi_dram_addr += dma_length;
		}

		if (pi_first_dma)
		{
			// TODO: CIC-6105 has different address...
			mem_map->write_dword(0x00000318, 0x400000);
			mem_map->write_dword(0x000003f0, 0x800000);
			pi_first_dma = 0;
		}
	}
	else
	{
		UINT32 dma_length = pi_rd_len + 1;
		if (dma_length & 3)
		{
			dma_length = (dma_length + 3) & ~3;
		}

		if (pi_dram_addr != 0xffffffff)
		{
			for(int i = 0; i < dma_length / 2; i++)
			{
				cart16[BYTE_XOR_BE(cart_addr + i)] = dram16[BYTE_XOR_BE(dram_addr + i)];
			}

			pi_cart_addr += dma_length;
			pi_dram_addr += dma_length;
		}
	}

	pi_status &= ~1; // Clear DMA_BUSY
	pi_status |= 8; // Set INTERRUPT

	signal_rcp_interrupt(PI_INTERRUPT);

	pi_dma_timer->adjust(attotime::never);
}

READ32_MEMBER( n64_periphs::pi_reg_r )
{
	//printf("pi_reg_r %08x\n", offset * 4);
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
			logerror("pi_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(maincpu));
			break;
	}
	return 0;
}

WRITE32_MEMBER( n64_periphs::pi_reg_w )
{
	//printf("pi_reg_w %08x %08x %08x\n", offset * 4, data, mem_mask);
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
            pi_rd_len = data;
			pi_dma_dir = 0;
			pi_status |= 1;

			attotime dma_period = attotime::from_hz(93750000) * (pi_rd_len + 1) * 3;
			//printf("want read dma in %d\n", (pi_rd_len + 1));
			pi_dma_timer->adjust(dma_period);
			break;
		}

		case 0x0c/4:		// PI_WR_LEN_REG
		{
            pi_wr_len = data;
			pi_dma_dir = 1;
			pi_status |= 1;

			attotime dma_period = attotime::from_hz(93750000) * (pi_wr_len + 1) * 3;
			//printf("want write dma in %d\n", (pi_wr_len + 1));
			pi_dma_timer->adjust(dma_period);
			break;
		}

		case 0x10/4:		// PI_STATUS_REG
		{
			if (data & 0x2)
			{
				pi_status &= ~8; // Clear INTERRUPT
				clear_rcp_interrupt(PI_INTERRUPT);
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
			logerror("pi_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(maincpu));
			break;
	}
}

// RDRAM Interface

READ32_MEMBER( n64_periphs::ri_reg_r )
{
	//printf("ri_reg_r %08x\n", offset * 4);
	if(offset > 0x1c/4)
	{
		logerror("ri_reg_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(maincpu));
		return 0;
	}
	return ri_regs[offset];
}

WRITE32_MEMBER( n64_periphs::ri_reg_w )
{
	//printf("ri_reg_w %08x %08x %08x\n", offset * 4, data, mem_mask);
	if(offset > 0x1c/4)
	{
		logerror("ri_reg_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(maincpu));
		return;
	}
	COMBINE_DATA(&ri_regs[offset]);
}

// Serial Interface
UINT8 n64_periphs::calc_mempack_crc(UINT8 *buffer, int length)
{
	UINT32 crc = 0;
	UINT32 temp2 = 0;

	for(int i = 0; i <= length; i++)
	{
		for(int j = 7; j >= 0; j--)
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

int n64_periphs::pif_channel_handle_command(int channel, int slength, UINT8 *sdata, int rlength, UINT8 *rdata)
{
	UINT8 command = sdata[0];

	switch (command)
	{
		case 0x00:		// Read status
		{
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
                    buttons = input_port_read(machine(), portnames[(channel*3) + 0]);
                    x = input_port_read(machine(), portnames[(channel*3) + 1]) - 128;
                    y = input_port_read(machine(), portnames[(channel*3) + 2]) - 128;

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
			UINT32 address;

			address = (sdata[1] << 8) | (sdata[2]);
			address &= ~0x1f;

			if(address == 0x400)
			{
				for(int i = 0; i < rlength-1; i++)
				{
					rdata[i] = 0x00;
				}

				rdata[rlength-1] = calc_mempack_crc(rdata, rlength-1);
			}
			else if(address < 0x7fe0)
			{
				for(int i = 0; i < rlength-1; i++)
				{
					rdata[i] = mempack[address+i];
				}

				rdata[rlength-1] = calc_mempack_crc(rdata, rlength-1);
			}
			return 1;
		}
		case 0x03:
		{
			UINT32 address = (sdata[1] << 8) | (sdata[2]);
			address &= ~0x1f;

			if (address == 0x8000)
			{

			}
			else
			{
				for(int i = 3; i < slength; i++)
				{
					mempack[address++] = sdata[i];
				}
			}

			rdata[0] = calc_mempack_crc(&sdata[3], slength-3);

			return 1;
		}

		case 0x04:		// Read from EEPROM
		{
			if (channel != 4)
			{
				return 1;
			}

			if (slength != 2 || rlength != 8)
			{
				fatalerror("handle_pif: write EEPROM (bytes to send %d, bytes to receive %d)\n", slength, rlength);
			}

			UINT8 block_offset = sdata[1] * 8;

			for(int i=0; i < 8; i++)
			{
				rdata[i] = eeprom[block_offset+i];
			}

			return 1;
		}

		case 0x05:		// Write to EEPROM
		{
			if (channel != 4)
			{
				return 1;
			}

			if (slength != 10 || rlength != 1)
			{
				fatalerror("handle_pif: write EEPROM (bytes to send %d, bytes to receive %d)\n", slength, rlength);
			}

			UINT8 block_offset = sdata[1] * 8;
			for(int i = 0; i < 8; i++)
			{
				eeprom[block_offset+i] = sdata[2+i];
			}

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

void n64_periphs::handle_pif()
{
	if(pif_cmd[0x3f] == 0x1)		// only handle the command if the last byte is 1
	{
		int channel = 0;
		int end = 0;
		int cmd_ptr = 0;

		while(cmd_ptr < 0x3f && !end)
		{
			UINT8 bytes_to_send = pif_cmd[cmd_ptr++];

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
				if(bytes_to_send > 0 && (bytes_to_send & 0xc0) == 0)
				{
					UINT8 recv_buffer[0x40];
					UINT8 send_buffer[0x40];

					UINT8 bytes_to_recv = pif_cmd[cmd_ptr++];

					for(int j = 0; j < bytes_to_send; j++)
					{
						send_buffer[j] = pif_cmd[cmd_ptr++];
					}

                    int res = pif_channel_handle_command(channel, bytes_to_send, send_buffer, bytes_to_recv, recv_buffer);

					if (res == 0)
					{
						if (cmd_ptr + bytes_to_recv > 0x3f)
						{
							fatalerror("cmd_ptr overflow\n");
						}
						for(int j = 0; j < bytes_to_recv; j++)
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
}

void n64_periphs::pif_dma(int direction)
{
	UINT32 *src, *dst;

	if (si_dram_addr & 0x3)
	{
		fatalerror("pif_dma: si_dram_addr unaligned: %08X\n", si_dram_addr);
	}

	if (direction)		// RDRAM -> PIF RAM
	{
		src = (UINT32*)&rdram[(si_dram_addr & 0x1fffffff) / 4];

		for(int i = 0; i < 64; i+=4)
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
		handle_pif();

		dst = (UINT32*)&rdram[(si_dram_addr & 0x1fffffff) / 4];

		for(int i = 0; i < 64; i+=4)
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
	signal_rcp_interrupt(SI_INTERRUPT);
}

READ32_MEMBER( n64_periphs::si_reg_r )
{
	//printf("si_reg_r %08x\n", offset * 4);
	switch (offset)
	{
		//case 0x00/4:      // SI_DRAM_ADDR_REG
			//return si_dram_addr;

		case 0x18/4:		// SI_STATUS_REG
			return si_status;
	}
	return 0;
}

WRITE32_MEMBER( n64_periphs::si_reg_w )
{
	//printf("si_reg_w %08x %08x %08x\n", offset * 4, data, mem_mask);
	switch (offset)
	{
		case 0x00/4:		// SI_DRAM_ADDR_REG
			si_dram_addr = data;
			break;

		case 0x04/4:		// SI_PIF_ADDR_RD64B_REG
			// PIF RAM -> RDRAM
			si_pif_addr = data;
            si_pif_addr_rd64b = data;
            pif_dma(0);
			break;

		case 0x10/4:		// SI_PIF_ADDR_WR64B_REG
			// RDRAM -> PIF RAM
			si_pif_addr = data;
            si_pif_addr_wr64b = data;
            pif_dma(1);
			break;

		case 0x18/4:		// SI_STATUS_REG
			si_status &= ~0x1000;
			clear_rcp_interrupt(SI_INTERRUPT);
			break;

		default:
			logerror("si_reg_w: %08X, %08X, %08X\n", data, offset, mem_mask);
			break;
	}
}

READ32_MEMBER( n64_periphs::pif_ram_r )
{
    if(!space.debugger_access())
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

WRITE32_MEMBER( n64_periphs::pif_ram_w )
{
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

    signal_rcp_interrupt(SI_INTERRUPT);
}

//static UINT16 crc_seed = 0x3f;

MACHINE_START( n64 )
{
	mips3drc_set_options(machine.device("maincpu"), MIPS3DRC_FASTEST_OPTIONS + MIPS3DRC_STRICT_VERIFY + MIPS3DRC_STRICT_COP1);

	/* configure fast RAM regions for DRC */
	mips3drc_add_fastram(machine.device("maincpu"), 0x00000000, 0x007fffff, FALSE, rdram);

	rspdrc_set_options(machine.device("rsp"), RSPDRC_STRICT_VERIFY);
	rspdrc_flush_drc_cache(machine.device("rsp"));
	rspdrc_add_dmem(machine.device("rsp"), rsp_dmem);
	rspdrc_add_imem(machine.device("rsp"), rsp_imem);
}

MACHINE_RESET( n64 )
{
	cputag_set_input_line(machine, "rsp", INPUT_LINE_HALT, ASSERT_LINE);
}

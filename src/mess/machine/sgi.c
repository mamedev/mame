/*********************************************************************

    sgi.c

    Silicon Graphics MC (Memory Controller) code

*********************************************************************/

#include "emu.h"
#include "sgi.h"

#define VERBOSE_LEVEL ( 2 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ... )
{
	if( VERBOSE_LEVEL >= n_level )
	{
		va_list v;
		char buf[ 32768 ];
		va_start( v, s_fmt );
		vsprintf( buf, s_fmt, v );
		va_end( v );
		logerror( "%08x: %s", (unsigned) machine.device("maincpu")->safe_pc(), buf );
	}
}

struct MC_t
{
	emu_timer *tUpdateTimer;
	UINT32 nCPUControl0;
	UINT32 nCPUControl1;
	UINT32 nWatchdog;
	UINT32 nSysID;
	UINT32 nRPSSDiv;
	UINT32 nRefCntPreload;
	UINT32 nRefCnt;
	UINT32 nGIO64ArbParam;
	UINT32 nArbCPUTime;
	UINT32 nArbBurstTime;
	UINT32 nMemCfg0;
	UINT32 nMemCfg1;
	UINT32 nCPUMemAccCfg;
	UINT32 nGIOMemAccCfg;
	UINT32 nCPUErrorAddr;
	UINT32 nCPUErrorStatus;
	UINT32 nGIOErrorAddr;
	UINT32 nGIOErrorStatus;
	UINT32 nSysSemaphore;
	UINT32 nGIOLock;
	UINT32 nEISALock;
	UINT32 nGIO64TransMask;
	UINT32 nGIO64Subst;
	UINT32 nDMAIntrCause;
	UINT32 nDMAControl;
	UINT32 nDMATLBEntry0Hi;
	UINT32 nDMATLBEntry0Lo;
	UINT32 nDMATLBEntry1Hi;
	UINT32 nDMATLBEntry1Lo;
	UINT32 nDMATLBEntry2Hi;
	UINT32 nDMATLBEntry2Lo;
	UINT32 nDMATLBEntry3Hi;
	UINT32 nDMATLBEntry3Lo;
	UINT32 nRPSSCounter;
	UINT32 nDMAMemAddr;
	UINT32 nDMALineCntWidth;
	UINT32 nDMALineZoomStride;
	UINT32 nDMAGIO64Addr;
	UINT32 nDMAMode;
	UINT32 nDMAZoomByteCnt;
	UINT32 nDMARunning;
};
static MC_t *pMC;

READ32_HANDLER( sgi_mc_r )
{
	offset <<= 2;
	switch( offset )
	{
	case 0x0000:
	case 0x0004:
		//verboselog( space.machine(), 3, "CPU Control 0 Read: %08x (%08x)\n", pMC->nCPUControl0, mem_mask );
		return pMC->nCPUControl0;
	case 0x0008:
	case 0x000c:
		//verboselog( space.machine(), 2, "CPU Control 1 Read: %08x (%08x)\n", pMC->nCPUControl1, mem_mask );
		return pMC->nCPUControl1;
	case 0x0010:
	case 0x0014:
		//verboselog( space.machine(), 2, "Watchdog Timer Read: %08x (%08x)\n", pMC->nWatchdog, mem_mask );
		return pMC->nWatchdog;
	case 0x0018:
	case 0x001c:
		//verboselog( space.machine(), 2, "System ID Read: %08x (%08x)\n", pMC->nSysID, mem_mask );
		return pMC->nSysID;
	case 0x0028:
	case 0x002c:
		//verboselog( space.machine(), 2, "RPSS Divider Read: %08x (%08x)\n", pMC->nRPSSDiv, mem_mask );
		return pMC->nRPSSDiv;
	case 0x0030:
	case 0x0034:
		//verboselog( space.machine(), 2, "R4000 EEPROM Read\n" );
		return 0;
	case 0x0040:
	case 0x0044:
		//verboselog( space.machine(), 2, "Refresh Count Preload Read: %08x (%08x)\n", pMC->nRefCntPreload, mem_mask );
		return pMC->nRefCntPreload;
	case 0x0048:
	case 0x004c:
		//verboselog( space.machine(), 2, "Refresh Count Read: %08x (%08x)\n", pMC->nRefCnt, mem_mask );
		return pMC->nRefCnt;
	case 0x0080:
	case 0x0084:
		//verboselog( space.machine(), 2, "GIO64 Arbitration Param Read: %08x (%08x)\n", pMC->nGIO64ArbParam, mem_mask );
		return pMC->nGIO64ArbParam;
	case 0x0088:
	case 0x008c:
		//verboselog( space.machine(), 2, "Arbiter CPU Time Read: %08x (%08x)\n", pMC->nArbCPUTime, mem_mask );
		return pMC->nArbCPUTime;
	case 0x0098:
	case 0x009c:
		//verboselog( space.machine(), 2, "Arbiter Long Burst Time Read: %08x (%08x)\n", pMC->nArbBurstTime, mem_mask );
		return pMC->nArbBurstTime;
	case 0x00c0:
	case 0x00c4:
		//verboselog( space.machine(), 3, "Memory Configuration Register 0 Read: %08x (%08x)\n", pMC->nMemCfg0, mem_mask );
		return pMC->nMemCfg0;
	case 0x00c8:
	case 0x00cc:
		//verboselog( space.machine(), 3, "Memory Configuration Register 1 Read: %08x (%08x)\n", pMC->nMemCfg1, mem_mask );
		return pMC->nMemCfg1;
	case 0x00d0:
	case 0x00d4:
		//verboselog( space.machine(), 2, "CPU Memory Access Config Params Read: %08x (%08x)\n", pMC->nCPUMemAccCfg, mem_mask );
		return pMC->nCPUMemAccCfg;
	case 0x00d8:
	case 0x00dc:
		//verboselog( space.machine(), 2, "GIO Memory Access Config Params Read: %08x (%08x)\n", pMC->nGIOMemAccCfg, mem_mask );
		return pMC->nGIOMemAccCfg;
	case 0x00e0:
	case 0x00e4:
		//verboselog( space.machine(), 2, "CPU Error Address Read: %08x (%08x)\n", pMC->nCPUErrorAddr, mem_mask );
		return pMC->nCPUErrorAddr;
	case 0x00e8:
	case 0x00ec:
		//verboselog( space.machine(), 2, "CPU Error Status Read: %08x (%08x)\n", pMC->nCPUErrorStatus, mem_mask );
		return pMC->nCPUErrorStatus;
	case 0x00f0:
	case 0x00f4:
		//verboselog( space.machine(), 2, "GIO Error Address Read: %08x (%08x)\n", pMC->nGIOErrorAddr, mem_mask );
		return pMC->nGIOErrorAddr;
	case 0x00f8:
	case 0x00fc:
		//verboselog( space.machine(), 2, "GIO Error Status Read: %08x (%08x)\n", pMC->nGIOErrorStatus, mem_mask );
		return pMC->nGIOErrorStatus;
	case 0x0100:
	case 0x0104:
		//verboselog( space.machine(), 2, "System Semaphore Read: %08x (%08x)\n", pMC->nSysSemaphore, mem_mask );
		return pMC->nSysSemaphore;
	case 0x0108:
	case 0x010c:
		//verboselog( space.machine(), 2, "GIO Lock Read: %08x (%08x)\n", pMC->nGIOLock, mem_mask );
		return pMC->nGIOLock;
	case 0x0110:
	case 0x0114:
		//verboselog( space.machine(), 2, "EISA Lock Read: %08x (%08x)\n", pMC->nEISALock, mem_mask );
		return pMC->nEISALock;
	case 0x0150:
	case 0x0154:
		//verboselog( space.machine(), 2, "GIO64 Translation Address Mask Read: %08x (%08x)\n", pMC->nGIO64TransMask, mem_mask );
		return pMC->nGIO64TransMask;
	case 0x0158:
	case 0x015c:
		//verboselog( space.machine(), 2, "GIO64 Translation Address Substitution Bits Read: %08x (%08x)\n", pMC->nGIO64Subst, mem_mask );
		return pMC->nGIO64Subst;
	case 0x0160:
	case 0x0164:
		//verboselog( space.machine(), 2, "DMA Interrupt Cause: %08x (%08x)\n", pMC->nDMAIntrCause, mem_mask );
		return pMC->nDMAIntrCause;
	case 0x0168:
	case 0x016c:
		//verboselog( space.machine(), 2, "DMA Control Read: %08x (%08x)\n", pMC->nDMAControl, mem_mask );
		return pMC->nDMAControl;
	case 0x0180:
	case 0x0184:
		//verboselog( space.machine(), 2, "DMA TLB Entry 0 High Read: %08x (%08x)\n", pMC->nDMATLBEntry0Hi, mem_mask );
		return pMC->nDMATLBEntry0Hi;
	case 0x0188:
	case 0x018c:
		//verboselog( space.machine(), 2, "DMA TLB Entry 0 Low Read: %08x (%08x)\n", pMC->nDMATLBEntry0Lo, mem_mask );
		return pMC->nDMATLBEntry0Lo;
	case 0x0190:
	case 0x0194:
		//verboselog( space.machine(), 2, "DMA TLB Entry 1 High Read: %08x (%08x)\n", pMC->nDMATLBEntry1Hi, mem_mask );
		return pMC->nDMATLBEntry1Hi;
	case 0x0198:
	case 0x019c:
		//verboselog( space.machine(), 2, "DMA TLB Entry 1 Low Read: %08x (%08x)\n", pMC->nDMATLBEntry1Lo, mem_mask );
		return pMC->nDMATLBEntry1Lo;
	case 0x01a0:
	case 0x01a4:
		//verboselog( space.machine(), 2, "DMA TLB Entry 2 High Read: %08x (%08x)\n", pMC->nDMATLBEntry2Hi, mem_mask );
		return pMC->nDMATLBEntry2Hi;
	case 0x01a8:
	case 0x01ac:
		//verboselog( space.machine(), 2, "DMA TLB Entry 2 Low Read: %08x (%08x)\n", pMC->nDMATLBEntry2Lo, mem_mask );
		return pMC->nDMATLBEntry2Lo;
	case 0x01b0:
	case 0x01b4:
		//verboselog( space.machine(), 2, "DMA TLB Entry 3 High Read: %08x (%08x)\n", pMC->nDMATLBEntry3Hi, mem_mask );
		return pMC->nDMATLBEntry3Hi;
	case 0x01b8:
	case 0x01bc:
		//verboselog( space.machine(), 2, "DMA TLB Entry 3 Low Read: %08x (%08x)\n", pMC->nDMATLBEntry3Lo, mem_mask );
		return pMC->nDMATLBEntry3Lo;
	case 0x1000:
	case 0x1004:
		//verboselog( space.machine(), 2, "RPSS 100ns Counter Read: %08x (%08x)\n", pMC->nRPSSCounter, mem_mask );
		return pMC->nRPSSCounter;
	case 0x2000:
	case 0x2004:
	case 0x2008:
	case 0x200c:
		//verboselog( space.machine(), 0, "DMA Memory Address Read: %08x (%08x)\n", pMC->nDMAMemAddr, mem_mask );
		return pMC->nDMAMemAddr;
	case 0x2010:
	case 0x2014:
		//verboselog( space.machine(), 0, "DMA Line Count and Width Read: %08x (%08x)\n", pMC->nDMALineCntWidth, mem_mask );
		return pMC->nDMALineCntWidth;
	case 0x2018:
	case 0x201c:
		//verboselog( space.machine(), 0, "DMA Line Zoom and Stride Read: %08x (%08x)\n", pMC->nDMALineZoomStride, mem_mask );
		return pMC->nDMALineZoomStride;
	case 0x2020:
	case 0x2024:
	case 0x2028:
	case 0x202c:
		//verboselog( space.machine(), 0, "DMA GIO64 Address Read: %08x (%08x)\n", pMC->nDMAGIO64Addr, mem_mask );
		return pMC->nDMAGIO64Addr;
	case 0x2030:
	case 0x2034:
		//verboselog( space.machine(), 0, "DMA Mode Write: %08x (%08x)\n", pMC->nDMAMode, mem_mask );
		return pMC->nDMAMode;
	case 0x2038:
	case 0x203c:
		//verboselog( space.machine(), 0, "DMA Zoom Count Read: %08x (%08x)\n", pMC->nDMAZoomByteCnt, mem_mask );
		return pMC->nDMAZoomByteCnt;
//  case 0x2040:
//  case 0x2044:
//      //verboselog( space.machine(), 2, "DMA Start Write: %08x (%08x)\n", data, mem_mask );
		// Start DMA
//      pMC->nDMARunning = 1;
//      break;
	case 0x2048:
	case 0x204c:
		//verboselog( space.machine(), 0, "VDMA Running Read: %08x (%08x)\n", pMC->nDMARunning, mem_mask );
		if( pMC->nDMARunning == 1 )
		{
			pMC->nDMARunning = 0;
			return 0x00000040;
		}
		else
		{
			return 0;
		}
	}
	//verboselog( space.machine(), 0, "Unmapped MC read: 0x%08x (%08x)\n", 0x1fa00000 + offset, mem_mask );
	return 0;
}

WRITE32_HANDLER( sgi_mc_w )
{
	offset <<= 2;
	switch( offset )
	{
	case 0x0000:
	case 0x0004:
		//verboselog( space.machine(), 2, "CPU Control 0 Write: %08x (%08x)\n", data, mem_mask );
		pMC->nCPUControl0 = data;
		break;
	case 0x0008:
	case 0x000c:
		//verboselog( space.machine(), 2, "CPU Control 1 Write: %08x (%08x)\n", data, mem_mask );
		pMC->nCPUControl1 = data;
		break;
	case 0x0010:
	case 0x0014:
		//verboselog( space.machine(), 2, "Watchdog Timer Clear" );
		pMC->nWatchdog = 0;
		break;
	case 0x0028:
	case 0x002c:
		//verboselog( space.machine(), 2, "RPSS Divider Write: %08x (%08x)\n", data, mem_mask );
		pMC->nRPSSDiv = data;
		break;
	case 0x0030:
	case 0x0034:
		//verboselog( space.machine(), 2, "R4000 EEPROM Write\n" );
		break;
	case 0x0040:
	case 0x0044:
		//verboselog( space.machine(), 2, "Refresh Count Preload Write: %08x (%08x)\n", data, mem_mask );
		pMC->nRefCntPreload = data;
		break;
	case 0x0080:
	case 0x0084:
		//verboselog( space.machine(), 2, "GIO64 Arbitration Param Write: %08x (%08x)\n", data, mem_mask );
		pMC->nGIO64ArbParam = data;
		break;
	case 0x0088:
	case 0x008c:
		//verboselog( space.machine(), 3, "Arbiter CPU Time Write: %08x (%08x)\n", data, mem_mask );
		pMC->nArbCPUTime = data;
		break;
	case 0x0098:
	case 0x009c:
		//verboselog( space.machine(), 3, "Arbiter Long Burst Time Write: %08x (%08x)\n", data, mem_mask );
		pMC->nArbBurstTime = data;
		break;
	case 0x00c0:
	case 0x00c4:
		//verboselog( space.machine(), 3, "Memory Configuration Register 0 Write: %08x (%08x)\n", data, mem_mask );
		pMC->nMemCfg0 = data;
		break;
	case 0x00c8:
	case 0x00cc:
		//verboselog( space.machine(), 3, "Memory Configuration Register 1 Write: %08x (%08x)\n", data, mem_mask );
		pMC->nMemCfg1 = data;
		break;
	case 0x00d0:
	case 0x00d4:
		//verboselog( space.machine(), 2, "CPU Memory Access Config Params Write: %08x (%08x)\n", data, mem_mask );
		pMC->nCPUMemAccCfg = data;
		break;
	case 0x00d8:
	case 0x00dc:
		//verboselog( space.machine(), 2, "GIO Memory Access Config Params Write: %08x (%08x)\n", data, mem_mask );
		pMC->nGIOMemAccCfg = data;
		break;
	case 0x00e8:
	case 0x00ec:
		//verboselog( space.machine(), 2, "CPU Error Status Clear\n" );
		pMC->nCPUErrorStatus = 0;
		break;
	case 0x00f8:
	case 0x00fc:
		//verboselog( space.machine(), 2, "GIO Error Status Clear\n" );
		pMC->nGIOErrorStatus = 0;
		break;
	case 0x0100:
	case 0x0104:
		//verboselog( space.machine(), 2, "System Semaphore Write: %08x (%08x)\n", data, mem_mask );
		pMC->nSysSemaphore = data;
		break;
	case 0x0108:
	case 0x010c:
		//verboselog( space.machine(), 2, "GIO Lock Write: %08x (%08x)\n", data, mem_mask );
		pMC->nGIOLock = data;
		break;
	case 0x0110:
	case 0x0114:
		//verboselog( space.machine(), 2, "EISA Lock Write: %08x (%08x)\n", data, mem_mask );
		pMC->nEISALock = data;
		break;
	case 0x0150:
	case 0x0154:
		//verboselog( space.machine(), 2, "GIO64 Translation Address Mask Write: %08x (%08x)\n", data, mem_mask );
		pMC->nGIO64TransMask = data;
		break;
	case 0x0158:
	case 0x015c:
		//verboselog( space.machine(), 2, "GIO64 Translation Address Substitution Bits Write: %08x (%08x)\n", data, mem_mask );
		pMC->nGIO64Subst = data;
		break;
	case 0x0160:
	case 0x0164:
		//verboselog( space.machine(), 0, "DMA Interrupt Cause Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMAIntrCause = data;
		break;
	case 0x0168:
	case 0x016c:
		//verboselog( space.machine(), 0, "DMA Control Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMAControl = data;
		break;
	case 0x0180:
	case 0x0184:
		//verboselog( space.machine(), 0, "DMA TLB Entry 0 High Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMATLBEntry0Hi = data;
		break;
	case 0x0188:
	case 0x018c:
		//verboselog( space.machine(), 0, "DMA TLB Entry 0 Low Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMATLBEntry0Lo = data;
		break;
	case 0x0190:
	case 0x0194:
		//verboselog( space.machine(), 0, "DMA TLB Entry 1 High Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMATLBEntry1Hi = data;
		break;
	case 0x0198:
	case 0x019c:
		//verboselog( space.machine(), 0, "DMA TLB Entry 1 Low Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMATLBEntry1Lo = data;
		break;
	case 0x01a0:
	case 0x01a4:
		//verboselog( space.machine(), 0, "DMA TLB Entry 2 High Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMATLBEntry2Hi = data;
		break;
	case 0x01a8:
	case 0x01ac:
		//verboselog( space.machine(), 0, "DMA TLB Entry 2 Low Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMATLBEntry2Lo = data;
		break;
	case 0x01b0:
	case 0x01b4:
		//verboselog( space.machine(), 0, "DMA TLB Entry 3 High Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMATLBEntry3Hi = data;
		break;
	case 0x01b8:
	case 0x01bc:
		//verboselog( space.machine(), 0, "DMA TLB Entry 3 Low Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMATLBEntry3Lo = data;
		break;
	case 0x2000:
	case 0x2004:
		//verboselog( space.machine(), 0, "DMA Memory Address Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMAMemAddr = data;
		break;
	case 0x2008:
	case 0x200c:
		//verboselog( space.machine(), 0, "DMA Memory Address + Default Params Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMAMemAddr = data;
		break;
	case 0x2010:
	case 0x2014:
		//verboselog( space.machine(), 0, "DMA Line Count and Width Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMALineCntWidth = data;
		break;
	case 0x2018:
	case 0x201c:
		//verboselog( space.machine(), 0, "DMA Line Zoom and Stride Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMALineZoomStride = data;
		break;
	case 0x2020:
	case 0x2024:
		//verboselog( space.machine(), 0, "DMA GIO64 Address Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMAGIO64Addr = data;
		break;
	case 0x2028:
	case 0x202c:
		//verboselog( space.machine(), 0, "DMA GIO64 Address Write + Start DMA: %08x (%08x)\n", data, mem_mask );
		pMC->nDMAGIO64Addr = data;
		// Start DMA
		pMC->nDMARunning = 1;
		break;
	case 0x2030:
	case 0x2034:
		//verboselog( space.machine(), 0, "DMA Mode Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMAMode = data;
		break;
	case 0x2038:
	case 0x203c:
		//verboselog( space.machine(), 0, "DMA Zoom Count + Byte Count Write: %08x (%08x)\n", data, mem_mask );
		pMC->nDMAZoomByteCnt = data;
		break;
	case 0x2040:
	case 0x2044:
		//verboselog( space.machine(), 0, "DMA Start Write: %08x (%08x)\n", data, mem_mask );
		// Start DMA
		pMC->nDMARunning = 1;
		break;
	case 0x2070:
	case 0x2074:
		//verboselog( space.machine(), 0, "DMA GIO64 Address Write + Default Params Write + Start DMA: %08x (%08x)\n", data, mem_mask );
		pMC->nDMAGIO64Addr = data;
		// Start DMA
		pMC->nDMARunning = 1;
		break;
	default:
		//verboselog( space.machine(), 0, "Unmapped MC write: 0x%08x: %08x (%08x)\n", 0x1fa00000 + offset, data, mem_mask );
		break;
	}
}

static void sgi_mc_update(void)
{
	pMC->nRPSSCounter += 1000;
}

static TIMER_CALLBACK(mc_update_callback)
{
	sgi_mc_update();
}

static void sgi_mc_timer_init(running_machine &machine)
{
	pMC->tUpdateTimer = machine.scheduler().timer_alloc(FUNC(mc_update_callback));
	pMC->tUpdateTimer->adjust(attotime::from_hz(10000), 0, attotime::from_hz(10000));
}

void sgi_mc_init(running_machine &machine)
{
	pMC = auto_alloc_clear(machine, MC_t);

	// if Indigo2, ID appropriately
	if (!strcmp(machine.system().name, "ip244415"))
	{
		pMC->nSysID = 0x11;	// rev. B MC, EISA bus present
	}

	sgi_mc_timer_init(machine);
}

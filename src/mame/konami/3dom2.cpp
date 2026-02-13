// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    3DO M2 Bulldog ASIC

***************************************************************************/

#include "emu.h"
#include "3dom2.h"

#include <algorithm> // std::min


//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

// Device type definitions
DEFINE_DEVICE_TYPE(M2_BDA, m2_bda_device, "m2bda", "3DO M2 Bulldog ASIC")
DEFINE_DEVICE_TYPE(M2_POWERBUS, m2_powerbus_device, "m2powerbus", "BDA PowerBus Controller")
DEFINE_DEVICE_TYPE(M2_MEMCTL, m2_memctl_device, "m2memctl", "BDA Memory Controller")
DEFINE_DEVICE_TYPE(M2_VDU, m2_vdu_device, "m2vdu", "BDA VDU")
DEFINE_DEVICE_TYPE(M2_CTRLPORT, m2_ctrlport_device, "m2ctrlport", "BDA Control Ports")
DEFINE_DEVICE_TYPE(M2_MPEG, m2_mpeg_device, "m2mpeg", "3DO MPEG Decoder")
DEFINE_DEVICE_TYPE(M2_CDE, m2_cde_device, "m2cde", "3DO M2 CDE ASIC")



//**************************************************************************
//  VDU REGISTER DEFINITIONS
//**************************************************************************

#define VDU_VLOC                0x00
#define VDU_VINT                0x04
#define VDU_VDC0                0x08
#define VDU_VDC1                0x0c
#define VDU_FV0A                0x10
#define VDU_FV1A                0x14
#define VDU_AVDI                0x1c
#define VDU_VDLI                0x20
#define VDU_VCFG                0x24
#define VDU_DMT0                0x28
#define VDU_DMT1                0x2c
#define VDU_LFSR                0x30
#define VDU_VRST                0x34

//-------------------------------------------------
//  VLOC
//-------------------------------------------------
#define VDU_VLOC_VCOUNT_MASK    0x00003FF8
#define VDU_VLOC_VCOUNT_SHIFT   3
#define VDU_VLOC_VIDEOFIELD     0x00004000
#define VDU_VLOC_RESERVED       0xFFFF8007

//-------------------------------------------------
//  VINT
//-------------------------------------------------
#define VDU_VINT_VINT0          0x80000000
#define VDU_VINT_VLINE0_MASK    0x7FF00000
#define VDU_VINT_VLINE0_SHIFT   20
#define VDU_VINT_VINT1          0x00008000
#define VDU_VINT_VLINE1_MASK    0x00007FF0
#define VDU_VINT_VLINE1_SHIFT   4
#define VDU_VINT_RESERVED       0x000F000F

//-------------------------------------------------
//  VDC0/VDC1
//-------------------------------------------------
#define VDU_VDC_HINT            0x02000000
#define VDU_VDC_VINT            0x01000000
#define VDU_VDC_DITHER          0x00400000
#define VDU_VDC_MTXBYP          0x00200000
#define VDU_VDC_RESERVED        0xFC9FFFFF

//-------------------------------------------------
//  AVDI
//-------------------------------------------------
#define VDU_AVDI_HSTART_MASK    0xFFE00000
#define VDU_AVDI_HWIDTH_MASK    0x0003FF80
#define VDU_AVDI_HDOUBLE        0x00000008
#define VDU_AVDI_VDOUBLE        0x00000004
#define VDU_AVDI_RESERVED       0x001C0073
#define VDU_AVDI_HSTART_SHIFT   21
#define VDU_AVDI_HWIDTH_SHIFT   7

//-------------------------------------------------
//  VDLI
//-------------------------------------------------
#define VDU_VDLI_BYPASSTYPE     0x10000000
#define VDU_VDLI_FBFORMAT       0x04000000
#define VDU_VDLI_ONEVINTDIS     0x00400000
#define VDU_VDLI_RANDOMDITHER   0x00200000
#define VDU_VDLI_RESERVED       0xEB9FFFFF
#define VDU_VDLI_BYPASSTYPE_MSB 0
#define VDU_VDLI_BYPASSTYPE_LSB 0x10000000
#define VDU_VDLI_FBFORMAT_16    0
#define VDU_VDLI_FBFORMAT_32    0x04000000

//-------------------------------------------------
//  VCFG
//-------------------------------------------------

//-------------------------------------------------
//  VRST
//-------------------------------------------------
#define VDU_VRST_DVERESET       0x00000002
#define VDU_VRST_VIDRESET       0x00000001
#define VDU_VRST_RESERVED       0xFFFFFFFC


//-------------------------------------------------
//  VDL DMA CONTROL WORD
//-------------------------------------------------
#define VDL_DMA_MOD_MASK        0xFF000000
#define VDL_DMA_ENABLE          0x00200000
#define VDL_DMA_NOBUCKET        0x00020000
#define VDL_DMA_LDLOWER         0x00010000
#define VDL_DMA_LDUPPER         0x00008000
#define VDL_DMA_NWORDS_MASK     0x00007E00
#define VDL_DMA_NLINES_MASK     0x000001FF
#define VDL_DMA_RESERVED        0x00DC0000
#define VDL_DMA_NWORDS_SHIFT    9
#define VDL_DMA_MOD_SHIFT       24
#define VDL_DMA_NLINES_SHIFT    0

//-------------------------------------------------
//  VDL DC CONTROL WORD
//-------------------------------------------------
#define VDL_DC                  0x80000000
#define VDL_DC_0                0x00000000
#define VDL_DC_1                0x10000000
#define VDL_DC_HINTCTL_MASK     0x00060000
#define VDL_DC_HINTCTL_SHIFT    17
#define VDL_DC_VINTCTL_MASK     0x00018000
#define VDL_DC_VINTCTL_SHIFT    15
#define VDL_DC_DITHERCTL_MASK   0x00001800
#define VDL_DC_DITHERCTL_SHIFT  11
#define VDL_DC_MTXBYPCTL_MASK   0x00000600
#define VDL_DC_MTXBYPCTL_SHIFT  9
#define VDL_DC_RESERVED         0x0FF861FF
#define VDL_CTL_DISABLE         0
#define VDL_CTL_ENABLE          1
#define VDL_CTL_NOP             2

//-------------------------------------------------
//  VDL AV CONTROL WORD
//-------------------------------------------------
#define VDL_AV                  0xA0000000
#define VDL_AV_HSTART_MASK      0x1FFC0000
#define VDL_AV_HSTART_SHIFT     18
#define VDL_AV_LD_HSTART        0x00020000
#define VDL_AV_HWIDTH_MASK      0x0001FFC0
#define VDL_AV_HWIDTH_SHIFT     6
#define VDL_AV_LD_HWIDTH        0x00000020
#define VDL_AV_HDOUBLE          0x00000010
#define VDL_AV_VDOUBLE          0x00000008
#define VDL_AV_LD_HDOUBLE       0x00000004
#define VDL_AV_LD_VDOUBLE       0x00000002
#define VDL_AV_RESERVED         0x00000001

//-------------------------------------------------
//  VDL LC CONTROL WORD
//-------------------------------------------------
#define VDL_LC                  0xC0000000
#define VDL_LC_BYPASSTYPE       0x02000000
#define VDL_LC_FBFORMAT         0x00800000
#define VDL_LC_ONEVINTDIS       0x00080000
#define VDL_LC_RANDOMDITHER     0x00040000
#define VDL_LC_LD_BYPASSTYPE    0x00002000
#define VDL_LC_LD_FBFORMAT      0x00001000
#define VDL_LC_RESERVED         0x1D73CFFF
#define VDL_LC_BYPASSTYPE_MSB   0x00000000
#define VDL_LC_BYPASSTYPE_LSB   0x02000000
#define VDL_LC_FBFORMAT_16      0x00000000
#define VDL_LC_FBFORMAT_32      0x00800000

//-------------------------------------------------
//  VDL DMA CONTROL WORD
//-------------------------------------------------

#define VDL_NOP                 0xe1000000



/***************************************************************************
    SUPPORT FUNCTIONS
***************************************************************************/

static void write_m2_reg(uint32_t &reg, uint32_t data, m2_reg_wmode mode)
{
	switch (mode)
	{
		case REG_WRITE: reg = data;     break;
		case REG_SET:   reg |= data;    break;
		case REG_CLEAR: reg &= ~data;   break;
		default:
			throw emu_fatalerror("write_m2_reg: Bad register write mode");
	}
}



//**************************************************************************
//  BDA DEVICE
//**************************************************************************

//-------------------------------------------------
//  m2_bda_device - constructor
//-------------------------------------------------

m2_bda_device::m2_bda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M2_BDA, tag, owner, clock),
	m_cpu1(*this, finder_base::DUMMY_TAG),
	m_cpu2(*this, finder_base::DUMMY_TAG),
	m_cde(*this, finder_base::DUMMY_TAG),
	m_videores_in(*this, 0),
	m_memctl(*this, "memctl"),
	m_powerbus(*this, "powerbus"),
	m_vdu(*this, "vdu"),
	m_ctrlport(*this, "ctrlport"),
	m_dspp(*this, "dspp"),
	m_mpeg(*this, "mpeg"),
	m_te(*this, "te"),
	m_dac_l(*this),
	m_dac_r(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m2_bda_device::device_start()
{
	// Allocate RAM
	uint32_t ram_size = (m_rambank_size[0] + m_rambank_size[1]) * 1024 * 1024;
	m_ram = std::make_unique<uint32_t[]>(ram_size / sizeof(uint32_t));
	m_ram_mask = ram_size - 1;

	// Install RAM and handlers into the CPU address spaces
	configure_ppc_address_map(m_cpu1->space(AS_PROGRAM));
	configure_ppc_address_map(m_cpu2->space(AS_PROGRAM));

	// Register state for saving
	save_pointer(NAME(m_ram), ram_size / sizeof(uint32_t));

	// Set a timer to pull data from the DSPP FIFO into the DACs
	m_dac_timer = timer_alloc(FUNC(m2_bda_device::dac_update), this);
	m_dac_timer->adjust(attotime::from_hz(16.9345));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m2_bda_device::device_reset()
{

}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void m2_bda_device::device_post_load()
{

}


//-------------------------------------------------
//  machine_config_fragment - declare sub-devices
//-------------------------------------------------

void m2_bda_device::device_add_mconfig(machine_config &config)
{
	M2_MEMCTL(config, m_memctl, DERIVED_CLOCK(1, 1));

	M2_POWERBUS(config, m_powerbus, DERIVED_CLOCK(1, 1));

	M2_VDU(config, m_vdu, DERIVED_CLOCK(1, 1));
	m_vdu->vint0_int_handler().set(m_powerbus, FUNC(m2_powerbus_device::int_line<BDAINT_VINT0_LINE>));
	m_vdu->vint1_int_handler().set(m_powerbus, FUNC(m2_powerbus_device::int_line<BDAINT_VINT1_LINE>));

	M2_CTRLPORT(config, m_ctrlport, DERIVED_CLOCK(1, 1));

	M2_MPEG(config, m_mpeg, DERIVED_CLOCK(1, 1));
//  m_mpeg->int_handler().set(m_powerbus, FUNC(m2_powerbus_device::int_line<BDAINT_MPEG_LINE>));

	DSPP_BULLDOG(config, m_dspp, DERIVED_CLOCK(1, 1));
	m_dspp->int_handler().set(m_powerbus, FUNC(m2_powerbus_device::int_line<BDAINT_DSP_LINE>));
	m_dspp->dma_read_handler().set(FUNC(m2_bda_device::read_bus8));
	m_dspp->dma_write_handler().set(FUNC(m2_bda_device::write_bus8));

	M2_TE(config, m_te, DERIVED_CLOCK(1, 1));
	m_te->general_int_handler().set(m_powerbus, FUNC(m2_powerbus_device::int_line<BDAINT_TRIGEN_LINE>));
	m_te->dfinstr_int_handler().set(m_powerbus, FUNC(m2_powerbus_device::int_line<BDAINT_TRIDFINST_LINE>));
	m_te->iminstr_int_handler().set(m_powerbus, FUNC(m2_powerbus_device::int_line<BDAINT_TRIDMINST_LINE>));
	m_te->listend_int_handler().set(m_powerbus, FUNC(m2_powerbus_device::int_line<BDAINT_TRILISTEND_LINE>));
	m_te->winclip_int_handler().set(m_powerbus, FUNC(m2_powerbus_device::int_line<BDAINT_TRIWINCLIP_LINE>));
}


//-------------------------------------------------
//  dac_update - pull DAC data from the DSPP
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(m2_bda_device::dac_update)
{
	m_dac_l(m_dspp->read_output_fifo());
	m_dac_r(m_dspp->read_output_fifo());
	m_dac_timer->adjust(attotime::from_hz(44100));
}



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  cpu_id_r - read from CPU ID register
//-------------------------------------------------

uint32_t m2_bda_device::cpu_id_r(address_space &space)
{
	uint32_t data = 0;

	// .x...... ........ ........ ........ -  Video type (0 = Arcade, 1 = NTSC/PAL)
	// x....... ........ ........ ........ -  CPU ID     (0 = CPU1, 1 = CPU2)

	if (&space.device() == m_cpu2)
		data |= 0x80000000;

	if (m_videores_in() != 0)
		data |= 0x40000000;

	return data;
}


//-------------------------------------------------
//  cpu_id_w - Write to CPU ID register
//-------------------------------------------------

void m2_bda_device::cpu_id_w(address_space &space, uint32_t data)
{
	// TODO: How should this work?
	logerror("%s: CPUID: %x\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  read_bus8 - Read 8-bit data from the PowerBus
//-------------------------------------------------

uint8_t m2_bda_device::read_bus8(offs_t offset)
{
	assert(offset >= RAM_BASE && offset <= RAM_BASE + m_ram_mask);

	offset &= m_ram_mask;
	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_ram[0]) + BYTE8_XOR_BE(offset);
	return *ptr;
}


//-------------------------------------------------
//  read_bus16 - Read 16-bit data from the PowerBus
//-------------------------------------------------

uint16_t m2_bda_device::read_bus16(offs_t offset)
{
	assert(offset >= RAM_BASE && offset <= RAM_BASE + m_ram_mask);

	offset &= m_ram_mask;
	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_ram[0]) + WORD2_XOR_BE(offset);
	return *reinterpret_cast<uint16_t *>(ptr);
}


//-------------------------------------------------
//  read_bus32 - Read 32-bit data from the PowerBus
//-------------------------------------------------

uint32_t m2_bda_device::read_bus32(offs_t offset)
{
	assert(offset >= RAM_BASE && offset <= RAM_BASE + m_ram_mask);

	offset &= m_ram_mask;

	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_ram[0]) + DWORD_XOR_BE(offset);
	return *reinterpret_cast<uint32_t *>(ptr);
}


//-------------------------------------------------
//  write_bus8 - Write 8-bit data to the PowerBus
//-------------------------------------------------

void m2_bda_device::write_bus8(offs_t offset, uint8_t data)
{
	assert(offset >= RAM_BASE && offset <= RAM_BASE + m_ram_mask);

	offset &= m_ram_mask;
	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_ram[0]) + BYTE8_XOR_BE(offset);
	*ptr = data;
}


//-------------------------------------------------
//  write_bus16 - Write 16-bit data to the PowerBus
//-------------------------------------------------

void m2_bda_device::write_bus16(offs_t offset, uint16_t data)
{
	assert(offset >= RAM_BASE && offset <= RAM_BASE + m_ram_mask);

	offset &= m_ram_mask;
	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_ram[0]) + WORD2_XOR_BE(offset);
	*reinterpret_cast<uint16_t *>(ptr) = data;
}


//-------------------------------------------------
//  write_bus32 - Write 32-bit data to the PowerBus
//-------------------------------------------------

void m2_bda_device::write_bus32(offs_t offset, uint32_t data)
{
	assert(offset >= RAM_BASE && offset <= RAM_BASE + m_ram_mask);

	offset &= m_ram_mask;
	uint8_t *ptr = reinterpret_cast<uint8_t *>(&m_ram[0]) + DWORD_XOR_BE(offset);
	*reinterpret_cast<uint32_t *>(ptr) = data;
}



/***************************************************************************
    PRIVATE FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  configure_ppc_address_map -
//-------------------------------------------------

void m2_bda_device::configure_ppc_address_map(address_space &space)
{
	// Install shared RAM
	space.install_ram(RAM_BASE, RAM_BASE + m_ram_mask, m_ram.get());

	// Install TE texture RAM window
	space.install_ram(TE_TRAM_BASE, TE_TRAM_BASE + TE_TRAM_MASK, m_te->tram_ptr());

	// Install BDA sub-devices
	space.install_readwrite_handler(POWERBUS_BASE,  POWERBUS_BASE + DEVICE_MASK,read32sm_delegate(*m_powerbus, FUNC(m2_powerbus_device::read)),    write32sm_delegate(*m_powerbus, FUNC(m2_powerbus_device::write)),    0xffffffffffffffffULL);
	space.install_readwrite_handler(MEMCTL_BASE,    MEMCTL_BASE + DEVICE_MASK,  read32s_delegate(*m_memctl,    FUNC(m2_memctl_device::read)),      write32s_delegate(*m_memctl,    FUNC(m2_memctl_device::write)),      0xffffffffffffffffULL);
	space.install_readwrite_handler(VDU_BASE,       VDU_BASE + DEVICE_MASK,     read32s_delegate(*m_vdu,       FUNC(m2_vdu_device::read)),         write32s_delegate(*m_vdu,       FUNC(m2_vdu_device::write)),         0xffffffffffffffffULL);
	space.install_readwrite_handler(TE_BASE,        TE_BASE + DEVICE_MASK,      read32sm_delegate(*m_te,       FUNC(m2_te_device::read)),          write32sm_delegate(*m_te,       FUNC(m2_te_device::write)),          0xffffffffffffffffULL);
	space.install_readwrite_handler(DSP_BASE,       DSP_BASE + DEVICE_MASK,     read32sm_delegate(*m_dspp,     FUNC(dspp_bulldog_device::host_read)), write32sm_delegate(*m_dspp,     FUNC(dspp_bulldog_device::host_write)), 0xffffffffffffffffULL);
	space.install_readwrite_handler(CTRLPORT_BASE,  CTRLPORT_BASE + DEVICE_MASK,read32sm_delegate(*m_ctrlport, FUNC(m2_ctrlport_device::read)),    write32sm_delegate(*m_ctrlport, FUNC(m2_ctrlport_device::write)),    0xffffffffffffffffULL);
	space.install_readwrite_handler(MPEG_BASE,      MPEG_BASE + DEVICE_MASK,    read32sm_delegate(*m_mpeg,     FUNC(m2_mpeg_device::read)),        write32sm_delegate(*m_mpeg,     FUNC(m2_mpeg_device::write)),        0xffffffffffffffffULL);

	space.install_readwrite_handler(CPUID_BASE,     CPUID_BASE + DEVICE_MASK,   read32mo_delegate(*this,       FUNC(m2_bda_device::cpu_id_r)),     write32mo_delegate(*this,       FUNC(m2_bda_device::cpu_id_w)),      0xffffffffffffffffULL);

	space.install_readwrite_handler(SLOT4_BASE,     SLOT4_BASE + SLOT_MASK,     read32_delegate(*m_cde,        FUNC(m2_cde_device::read)),         write32_delegate(*m_cde,        FUNC(m2_cde_device::write)), 0xffffffffffffffffULL);
}



//**************************************************************************
//  POWERBUS DEVICE
//**************************************************************************


//-------------------------------------------------
//  m2_powerbus_device - constructor
//-------------------------------------------------

m2_powerbus_device::m2_powerbus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M2_POWERBUS, tag, owner, clock),
	m_int_handler(*this)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m2_powerbus_device::device_start()
{
	// Register state for saving
	save_item(NAME(m_ctrl));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_int_status));
	save_item(NAME(m_err_status));
	save_item(NAME(m_err_address));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m2_powerbus_device::device_reset()
{
	m_ctrl = 0;
	m_int_enable = 0;
	m_int_status = 0;
	m_err_status = 0;
	m_err_address = 0;
}


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  read -
//-------------------------------------------------

uint32_t m2_powerbus_device::read(offs_t offset)
{
	const uint32_t byte_offs = offset << 2;
	uint32_t data = 0;

	switch (byte_offs)
	{
		case BDAPCTL_PBINTENSET:
		{
			data = m_int_enable;
			break;
		}
		case BDAPCTL_PBINTSTAT:
		{
			data = m_int_status;
			break;
		}
		default:
			logerror("%s: POWERBUS R: [%x] %x\n", machine().describe_context(), byte_offs, data);
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void m2_powerbus_device::write(offs_t offset, uint32_t data)
{
	uint32_t byte_offs = offset << 2;

	switch (byte_offs & ~0x400)
	{
		case BDAPCTL_PBINTENSET:
		{
			write_m2_reg(m_int_enable, data, byte_offs & 0x400 ? REG_CLEAR : REG_SET);
			update_interrupts();
			break;
		}
		case BDAPCTL_ERRSTAT:
		{
#if 1 // TODO
			if (byte_offs & 0x400)
			{
				write_m2_reg(m_int_status, data, REG_CLEAR);
			}
			else
			{
				if (data == 1)
					write_m2_reg(m_int_status, data, REG_SET);
			}
			update_interrupts();
#endif
			break;
		}
		default:
			logerror("%s: POWERBUS W: [%x] %x\n", machine().describe_context(), byte_offs, data);
	}
}


/***************************************************************************
    PRIVATE FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  update_interrupts -
//-------------------------------------------------

void m2_powerbus_device::update_interrupts()
{
	m_int_handler(m_int_status & m_int_enable ? ASSERT_LINE : CLEAR_LINE);
}



//**************************************************************************
//  MEMORY CONTROLLER DEVICE
//**************************************************************************


//-------------------------------------------------
//  m2_memctl_device - constructor
//-------------------------------------------------

m2_memctl_device::m2_memctl_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M2_MEMCTL, tag, owner, clock),
	m_gpio_in(*this, 0),
	m_gpio_out(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m2_memctl_device::device_start()
{
	// TODO: DELETE ME
	m2_bda_device *m_bda = (m2_bda_device*)owner(); // TEMP

	// Configure the memory configuration register
	uint32_t bank1 = m_bda->get_rambank_size(0);
	uint32_t bank2 = m_bda->get_rambank_size(1);

	m_mcfg = (ramsize_to_mcfg_field(bank2) << MCFG_SS1_SHIFT) | (ramsize_to_mcfg_field(bank1) << MCFG_SS0_SHIFT);

	// Register state for saving
	save_item(NAME(m_mcfg));
	save_item(NAME(m_mref));
	save_item(NAME(m_mcntl));
	save_item(NAME(m_reset));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m2_memctl_device::device_reset()
{
	// TODO: Need postload to set GPIO also?
	m_mref = 0;
}


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  read -
//-------------------------------------------------

uint32_t m2_memctl_device::read(offs_t offset, uint32_t mem_mask)
{
	uint32_t byte_offs = offset << 2;
	uint32_t data = 0;

	switch (byte_offs)
	{
		case MCTL_MCONFIG:
		{
			data = m_mcfg;
			break;
		}
		case MCTL_MREF:
		{
			if ((m_mref & MREF_GPIO0_GP) && !(m_mref & MREF_GPIO0_OUT))
			{
				if (m_gpio_in[0]())
					m_mref |= MREF_GPIO0_VALUE;
				else
					m_mref &= ~MREF_GPIO0_VALUE;
			}

			if ((m_mref & MREF_GPIO1_GP) && !(m_mref & MREF_GPIO1_OUT))
			{
				if (m_gpio_in[1]())
					m_mref |= MREF_GPIO1_VALUE;
				else
					m_mref &= ~MREF_GPIO1_VALUE;
			}

			if ((m_mref & MREF_GPIO2_GP) && !(m_mref & MREF_GPIO2_OUT))
			{
				if (m_gpio_in[2]())
					m_mref |= MREF_GPIO2_VALUE;
				else
					m_mref &= ~MREF_GPIO2_VALUE;
			}

			if ((m_mref & MREF_GPIO3_GP) && !(m_mref & MREF_GPIO3_OUT))
			{
				if (m_gpio_in[3]())
					m_mref |= MREF_GPIO3_VALUE;
				else
					m_mref &= ~MREF_GPIO3_VALUE;
			}

			data = m_mref;
			break;
		}
		case MCTL_MCNTL:
		case MCTL_MRESET:
			//logerror("%s: MEMCTL READ: %x %x\n", machine().describe_context(), byte_offs, mem_mask);
			break;
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void m2_memctl_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t byte_offs = offset << 2;

	switch (byte_offs)
	{
		case MCTL_MCONFIG:
		{
			m_mcfg = data;
			break;
		}
		case MCTL_MREF:
		{
			// Set any general purpose outputs
			if (data & (MREF_GPIO0_GP | MREF_GPIO0_OUT))
				m_gpio_out[0](data & MREF_GPIO0_VALUE ? 1 : 0);

			if (data & (MREF_GPIO1_GP | MREF_GPIO1_OUT))
				m_gpio_out[1](data & MREF_GPIO1_VALUE ? 1 : 0);

			if (data & (MREF_GPIO2_GP | MREF_GPIO2_OUT))
				m_gpio_out[2](data & MREF_GPIO2_VALUE ? 1 : 0);

			if (data & (MREF_GPIO3_GP | MREF_GPIO3_OUT))
				m_gpio_out[3](data & MREF_GPIO3_VALUE ? 1 : 0);

			m_mref = data;
			break;
		}
		case MCTL_MCNTL:
		case MCTL_MRESET:
			//logerror("%s: MEMCTL WRITE: %x %x %x\n", machine().describe_context(), data, byte_offs);
			break;
	}
}



//**************************************************************************
//  VDU DEVICE
//**************************************************************************


//-------------------------------------------------
//  m2_vdu_device - constructor
//-------------------------------------------------

m2_vdu_device::m2_vdu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, M2_VDU, tag, owner, clock),
	m_screen(*this, finder_base::DUMMY_TAG),
	m_vint0_int_handler(*this),
	m_vint1_int_handler(*this)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m2_vdu_device::device_start()
{
	// Initialize line interrupt timers
	m_vint0_timer = timer_alloc(FUNC(m2_vdu_device::vint0_set), this);
	m_vint1_timer = timer_alloc(FUNC(m2_vdu_device::vint1_set), this);

	// Calculate H/V count bias values (1 = start of blanking)
	const rectangle visarea = m_screen->visible_area();

	m_hstart = visarea.min_x;
	m_htotal = visarea.max_x + 1;

	m_vstart = visarea.min_y;
	m_vtotal = visarea.max_y + 1;

	// Register state for saving
	save_item(NAME(m_vint));
	save_item(NAME(m_vdc0));
	save_item(NAME(m_vdc1));
	save_item(NAME(m_fv0a));
	save_item(NAME(m_fv1a));
	save_item(NAME(m_avdi));
	save_item(NAME(m_vdli));
	save_item(NAME(m_vcfg));
	save_item(NAME(m_dmt0));
	save_item(NAME(m_dmt1));
	save_item(NAME(m_vrst));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m2_vdu_device::device_reset()
{
	m_fv0a = 0;
	m_fv1a = 0;
	m_avdi = 0;
	m_vdli = 0;
	m_vint = 0;
	m_vcfg = 0;
	m_dmt0 = 0;
	m_dmt1 = 0;

	m_vint0_timer->adjust(attotime::never);
	m_vint1_timer->adjust(attotime::never);
}


//-------------------------------------------------
//  vertical interrupt timer callbacks
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(m2_vdu_device::vint0_set)
{
	m_vint |= VDU_VINT_VINT0;
	m_vint0_int_handler(ASSERT_LINE);
	set_vint_timer(0);
}

TIMER_CALLBACK_MEMBER(m2_vdu_device::vint1_set)
{
	m_vint |= VDU_VINT_VINT1;
	m_vint1_int_handler(ASSERT_LINE);
	set_vint_timer(1);
}



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  read -
//-------------------------------------------------

uint32_t m2_vdu_device::read(offs_t offset, uint32_t mem_mask)
{
	uint32_t byte_offs = offset << 2;
	uint32_t data = 0;

	switch (byte_offs)
	{
		case VDU_VLOC:
		{
			// TODO: Check me
			uint32_t mpos = m_screen->vpos();
			uint32_t vpos = ((mpos + m_vstart) % m_vtotal) + 1;
			data = vpos << VDU_VLOC_VCOUNT_SHIFT;
			break;
		}
		case VDU_VINT:
		{
			data = m_vint;
			break;
		}
		case VDU_VDC0:
		case VDU_VDC1:
		case VDU_FV0A:
		case VDU_FV1A:
		case VDU_AVDI:
		case VDU_VDLI:
		case VDU_VCFG:
		case VDU_DMT0:
		case VDU_DMT1:
		case VDU_LFSR:
		{
			//logerror("%s: VDU READ: %x %x\n", machine().describe_context(), byte_offs, mem_mask);
			break;
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void m2_vdu_device::write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t byte_offs = offset << 2;
	m2_reg_wmode wmode = byte_offs & 0x400 ? REG_CLEAR : REG_WRITE;

//  logerror("%s: VDU WRITE: %03x %08x %x\n", machine().describe_context(), byte_offs, data, mem_mask);
	byte_offs &= ~0x400;
	switch (byte_offs)
	{
		case VDU_VINT:
		{
			uint32_t old = m_vint;
			write_m2_reg(m_vint, data, wmode);

			// Update line interrupts if changed
			if ((m_vint & VDU_VINT_VLINE0_MASK) != (old & VDU_VINT_VLINE0_MASK))
				set_vint_timer(0);

			if ((m_vint & VDU_VINT_VLINE1_MASK) != (old & VDU_VINT_VLINE1_MASK))
				set_vint_timer(1);

			// Clear interrupt bits
			if ((old & VDU_VINT_VINT0) && !(m_vint & VDU_VINT_VINT0))
				m_vint0_int_handler(CLEAR_LINE);

			if ((old & VDU_VINT_VINT1) && !(m_vint & VDU_VINT_VINT1))
				m_vint1_int_handler(CLEAR_LINE);

			break;
		}
		case VDU_FV0A:
		{
			m_fv0a = data;
			break;
		}
		case VDU_FV1A:
		{
			m_fv1a = data;
			break;
		}
		case VDU_VCFG:
		{
			m_vcfg = data;
			break;
		}
		case VDU_VRST:
		{
			m_vrst = data;
			break;
		}
		default:
		{
			logerror("%s: VDU WRITE: %x %x %x\n", machine().describe_context(), byte_offs, data, mem_mask);
			break;
		}
	}
}


//-------------------------------------------------
//  parse_dc_word -
//-------------------------------------------------

void m2_vdu_device::parse_dc_word(uint32_t cmd)
{
	// Determine the control register
	uint32_t &vdc = cmd & VDL_DC_1 ? m_vdc1 : m_vdc0;

	// Horizontal interpolation
	uint32_t hint = (cmd & VDL_DC_HINTCTL_MASK) >> VDL_DC_HINTCTL_SHIFT;

	if (hint == VDL_CTL_ENABLE)
		vdc |= VDU_VDC_HINT;
	else if (hint == VDL_CTL_DISABLE)
		vdc &= ~VDU_VDC_HINT;


	// Vertical interpolation
	uint32_t vint = (cmd & VDL_DC_VINTCTL_MASK) >> VDL_DC_VINTCTL_SHIFT;

	if (vint == VDL_CTL_ENABLE)
		vdc |= VDU_VDC_VINT;
	else if (vint == VDL_CTL_DISABLE)
		vdc &= ~VDU_VDC_VINT;


	// Dithering
	uint32_t dith = (cmd & VDL_DC_DITHERCTL_MASK) >> VDL_DC_DITHERCTL_SHIFT;

	if (dith == VDL_CTL_ENABLE)
		vdc |= VDU_VDC_DITHER;
	else if (dith == VDL_CTL_DISABLE)
		vdc &= ~VDU_VDC_DITHER;


	// MTXBYP (?)
	uint32_t mtxbyp = (cmd & VDL_DC_MTXBYPCTL_MASK) >> VDL_DC_MTXBYPCTL_SHIFT;

	if (mtxbyp == VDL_CTL_ENABLE)
		vdc |= VDU_VDC_MTXBYP;
	else if (mtxbyp == VDL_CTL_DISABLE)
		vdc &= ~VDU_VDC_MTXBYP;
}


//-------------------------------------------------
//  parse_av_word -
//-------------------------------------------------

void m2_vdu_device::parse_av_word(uint32_t cmd)
{
	if (cmd & VDL_AV_LD_HSTART)
	{
		uint32_t hstart = (cmd & VDL_AV_HSTART_MASK) >> VDL_AV_HSTART_SHIFT;
		m_avdi &= ~VDU_AVDI_HSTART_MASK;
		m_avdi |= hstart << VDU_AVDI_HSTART_SHIFT;
	}
	if (cmd & VDL_AV_LD_HWIDTH)
	{
		uint32_t hwidth = (cmd & VDL_AV_HWIDTH_MASK) >> VDL_AV_HWIDTH_SHIFT;
		m_avdi &= ~VDU_AVDI_HWIDTH_MASK;
		m_avdi |= hwidth << VDU_AVDI_HWIDTH_SHIFT;
	}
	if (cmd & VDL_AV_LD_HDOUBLE)
	{
		if (cmd & VDL_AV_HDOUBLE)
			m_avdi |= VDU_AVDI_HDOUBLE;
		else
			m_avdi &= ~VDU_AVDI_HDOUBLE;
	}
	if (cmd & VDL_AV_LD_VDOUBLE)
	{
		if (cmd & VDL_AV_VDOUBLE)
			m_avdi |= VDU_AVDI_VDOUBLE;
		else
			m_avdi &= ~VDU_AVDI_VDOUBLE;
	}
}


//-------------------------------------------------
//  parse_lc_word -
//-------------------------------------------------

void m2_vdu_device::parse_lc_word(uint32_t cmd)
{
	// TODO: This may not be used
	if (cmd & VDL_LC_LD_BYPASSTYPE)
	{
		m_vdli &= ~VDU_VDLI_BYPASSTYPE;
		m_vdli |= (cmd & VDU_VDLI_BYPASSTYPE) == VDU_VDLI_BYPASSTYPE_MSB ? VDU_VDLI_BYPASSTYPE_MSB : VDU_VDLI_BYPASSTYPE_LSB;
	}
	if (cmd & VDL_LC_LD_FBFORMAT)
	{
		m_vdli &= ~VDU_VDLI_FBFORMAT;
		m_vdli |= (cmd & VDL_LC_FBFORMAT) == VDL_LC_FBFORMAT_32 ? VDU_VDLI_FBFORMAT_32 : VDU_VDLI_FBFORMAT_16;
	}

	// Seems these two are always set by the command word
	if (cmd & VDL_LC_RANDOMDITHER)
		m_vdli |= VDU_VDLI_RANDOMDITHER;
	else
		m_vdli &= ~VDU_VDLI_RANDOMDITHER;

	if (cmd & VDL_LC_ONEVINTDIS)
		m_vdli |= VDU_VDLI_ONEVINTDIS;
	else
		m_vdli &= ~VDU_VDLI_ONEVINTDIS;
}


//-------------------------------------------------
//  draw_scanline - Draw a scanline
//-------------------------------------------------

void m2_vdu_device::draw_scanline(uint32_t *dst, uint32_t srclower, uint32_t srcupper)
{
	m2_bda_device *m_bda = (m2_bda_device*)owner(); // TEMP

	uint32_t hs = (m_avdi & VDU_AVDI_HSTART_MASK) >> VDU_AVDI_HSTART_SHIFT;
	uint32_t hw = (m_avdi & VDU_AVDI_HWIDTH_MASK) >> VDU_AVDI_HWIDTH_SHIFT;

	bool is32bpp = m_vdli & VDU_VDLI_FBFORMAT_32 ? true : false;
//  bool bypassmsb = m_vdli & VDU_VDLI_BYPASSTYPE_MSB ? true : false;
//  bool randomdith = m_vdli & VDU_VDLI_RANDOMDITHER ? true : false;

	uint32_t h = 0;

	// Left border
	while (h < hs)
	{
		*dst++ = rgb_t::black();
		++h;
	}

	// Active video area
	uint32_t vismax = std::min<uint32_t>(h + hw, m_htotal);

	if (is32bpp)
	{
		while (h < vismax)
		{
			*dst++ = m_bda->read_bus32(srclower);
			srclower += 4;
			++h;
		}
	}
	else
	{
		while (h < vismax)
		{
			uint16_t srcdata = m_bda->read_bus16(srclower);
			*dst++ = pal555(srcdata, 10, 5, 0);
			srclower += 2;
			++h;
		}
	}

	// Right border
	while (h < m_htotal)
	{
		*dst++ = rgb_t::black();
		++h;
	}
}


//-------------------------------------------------
//  draw_scanline_double - Draw a pixel-doubled scanline
//-------------------------------------------------

void m2_vdu_device::draw_scanline_double(uint32_t *dst, uint32_t srclower, uint32_t srcupper)
{
	m2_bda_device *m_bda = (m2_bda_device*)owner(); // TEMP

	uint32_t hs = (m_avdi & VDU_AVDI_HSTART_MASK) >> VDU_AVDI_HSTART_SHIFT;
	uint32_t hw = (m_avdi & VDU_AVDI_HWIDTH_MASK) >> VDU_AVDI_HWIDTH_SHIFT;

	bool is32bpp = m_vdli & VDU_VDLI_FBFORMAT_32 ? true : false;
//  bool bypassmsb = m_vdli & VDU_VDLI_BYPASSTYPE_MSB ? true : false;
//  bool randomdith = m_vdli & VDU_VDLI_RANDOMDITHER ? true : false;

	uint32_t h = 0;

	// Left border
	while (h < hs)
	{
		*dst++ = rgb_t::black();
		++h;
	}

	// Active video area
	uint32_t vismax = std::min<uint32_t>(h + hw, m_htotal);

	if (is32bpp)
	{
		while (h < vismax)
		{
			uint32_t srcdata = m_bda->read_bus32(srclower);

			srclower += 4;
			*dst++ = srcdata;
			*dst++ = srcdata;
			++h;
		}
	}
	else
	{
		while (h < vismax)
		{
			uint32_t srcdata = m_bda->read_bus16(srclower);
			srcdata = pal555(srcdata, 10, 5, 0);

			srclower += 2;
			*dst++ = srcdata;
			*dst++ = srcdata;
			++h;
		}
	}

	// Right border
	while (h < m_htotal)
	{
		*dst++ = rgb_t::black();
		++h;
	}
}


//-------------------------------------------------
//  core_update_screen -
//-------------------------------------------------



//-------------------------------------------------
//  screen_update -
//-------------------------------------------------

uint32_t m2_vdu_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m2_bda_device *m_bda = (m2_bda_device*)owner(); // TEMP

	// TODO: Interlace mode
	uint32_t addr = m_fv0a;//screen.frame_number() & 1 ? m_fv1a : m_fv0a;

	// Fill entire screen with black if disabled
	if ((m_vrst & (VDU_VRST_VIDRESET | VDU_VRST_DVERESET)) || addr == 0)
	{
		bitmap.fill(rgb_t::black());
		return 0;
	}

	// Processing begins at VSYNC
	uint32_t v = 0;

	// Process VDLs until all lines are exhausted
	while (v < m_vtotal)
	{
		// Fetch the 4 header words
		uint32_t dmactl = m_bda->read_bus32(addr);
		uint32_t lower = m_bda->read_bus32(addr + 4);
		uint32_t upper = m_bda->read_bus32(addr + 8);
		uint32_t next = m_bda->read_bus32(addr + 12);

		addr += 16;

		// Word count includes the header
		uint32_t words = (dmactl & VDL_DMA_NWORDS_MASK) >> VDL_DMA_NWORDS_SHIFT;
		words -= 4;

		// Check and adjust the line count
		uint32_t lines = (dmactl & VDL_DMA_NLINES_MASK) >> VDL_DMA_NLINES_SHIFT;

		if (lines > 0)
		{
			uint32_t vend = v + lines;

			if (vend > m_vtotal)
				lines = vend - m_vtotal;
		}
		else
		{
			// A zero count denotes all of the remaining screen lines
			lines = m_vtotal - v;
		}

		// Parse the command list and update video registers accordingly
		while (words-- > 0)
		{
			uint32_t cmd = m_bda->read_bus32(addr);
			addr += 4;

			switch (cmd & 0xe0000000)
			{
				case VDL_DC:
				{
					parse_dc_word(cmd);
					break;
				}
				case VDL_AV:
				{
					parse_av_word(cmd);
					break;
				}
				case VDL_LC:
				{
					parse_lc_word(cmd);
					break;
				}
				default:
				{
					if (cmd != VDL_NOP)
						fatalerror("VDU: Unknown VDL command word\n");
					break;
				}
			}
		}

		// DMA from RAM to the display
		if (dmactl & VDL_DMA_ENABLE)
		{
			bool hdouble = m_avdi & VDU_AVDI_HDOUBLE ? true : false;
			bool vdouble = m_avdi & VDU_AVDI_VDOUBLE ? true : false;
//          bool onevintdis = m_vdli & VDU_VDLI_ONEVINTDIS ? true : false;

			uint32_t srclower = lower;
			uint32_t srcupper = upper;
			uint32_t mod = ((dmactl & VDL_DMA_MOD_MASK) >> VDL_DMA_MOD_SHIFT) << 5;

			// Draw these lines
			while (lines--)
			{
				// Line doubling is easily handled
				for (uint32_t ys = vdouble ? 2 : 1; ys > 0; --ys)
				{
					if (hdouble)
						draw_scanline_double(&bitmap.pix(v, 0), srclower, srcupper);
					else
						draw_scanline(&bitmap.pix(v, 0), srclower, srcupper);

					++v;
				}
				// Update the source addresses
				srclower += mod;
				srcupper += mod;
			}
		}
		else
		{
			// Blank this block of lines if DMA is disabled
			while (lines--)
			{
				uint32_t *dst = &bitmap.pix(v, cliprect.min_x);

				for (uint32_t x = cliprect.min_x; x <= cliprect.max_x; ++x)
					*dst++ = rgb_t::black();

				++v;
			}
		}

		// Jump to the next VDL
		addr = next;
	}

	return 0;
}


//-------------------------------------------------
//  set_vint_timer -
//-------------------------------------------------

void m2_vdu_device::set_vint_timer(uint32_t id)
{
	uint32_t v;
	emu_timer *timer = (id == 0) ? m_vint0_timer : m_vint1_timer;

	if (id == 0)
		v = (m_vint & VDU_VINT_VLINE0_MASK) >> VDU_VINT_VLINE0_SHIFT;
	else
		v = (m_vint & VDU_VINT_VLINE1_MASK) >> VDU_VINT_VLINE1_SHIFT;

	if (v == 0)
	{
		// Apparently 0 is invalid
		timer->adjust(attotime::never);
	}
	else
	{
		// Adjust the count to what the core expects
		uint32_t vadj = (v - 1 + (m_vtotal - m_vstart)) % m_vtotal;
		timer->adjust(m_screen->time_until_pos(vadj));
	}
}



//**************************************************************************
//  CONTROL PORTS DEVICE
//**************************************************************************

//-------------------------------------------------
//  m2_ctrlport_device - constructor
//-------------------------------------------------

m2_ctrlport_device::m2_ctrlport_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, M2_CTRLPORT, tag, owner, clock)
{

}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m2_ctrlport_device::device_start()
{

}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m2_ctrlport_device::device_reset()
{

}


/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  read -
//-------------------------------------------------

uint32_t m2_ctrlport_device::read(offs_t offset)
{
	//const uint32_t byte_offs = offset << 2;
	uint32_t data = machine().rand();

	//switch (byte_offs)
	//{
		//default:
			//logerror("%s: CTRLPORT R: [%x] %x\n", machine().describe_context(), byte_offs, data);
	//}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void m2_ctrlport_device::write(offs_t offset, uint32_t data)
{
	//uint32_t byte_offs = offset << 2;

	//switch (byte_offs)
	//{
		//default:
			//logerror("%s: CTRLPORT W: [%x] %x\n", machine().describe_context(), byte_offs, data);
	//}
}


/***************************************************************************
    PRIVATE FUNCTIONS
***************************************************************************/





//**************************************************************************
//  CDE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m2_cde_device - constructor
//-------------------------------------------------

m2_cde_device::m2_cde_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, M2_CDE, tag, owner, clock),
	m_cpu1(*this, finder_base::DUMMY_TAG),
	m_bda(*this, finder_base::DUMMY_TAG),
	m_int_handler(*this),
	m_sdbg_out_handler(*this),
	m_cd_ready_timer(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m2_cde_device::device_start()
{
	// Init DMA
	m_dma[0].m_timer = timer_alloc(FUNC(m2_cde_device::next_dma), this);
	m_dma[1].m_timer = timer_alloc(FUNC(m2_cde_device::next_dma), this);

	// Register state for saving
	save_item(NAME(m_sdbg_cntl));
	save_item(NAME(m_int_status));
	save_item(NAME(m_int_enable));
	save_item(NAME(m_bblock_en));
	save_item(NAME(m_visa_dis));

	for (uint32_t i = 0; i < 8; ++i)
	{
		save_item(NAME(m_bio_device[i].m_setup), i);
		save_item(NAME(m_bio_device[i].m_cycle_time), i);
	}

	for (uint32_t i = 0; i < 2; ++i)
	{
		save_item(NAME(m_dma[i].m_cntl), i);
		save_item(NAME(m_dma[i].m_cbad), i);
		save_item(NAME(m_dma[i].m_cpad), i);
		save_item(NAME(m_dma[i].m_ccnt), i);
		save_item(NAME(m_dma[i].m_nbad), i);
		save_item(NAME(m_dma[i].m_npad), i);
		save_item(NAME(m_dma[i].m_ncnt), i);
	}

	// Allocate other timers
	m_cd_ready_timer = timer_alloc(FUNC(m2_cde_device::trigger_ready_int), this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m2_cde_device::device_reset()
{
	m_sdbg_cntl = 0;
	m_int_status = 0;
	m_int_enable = 0;

	// TODO? Boot block is clear on reset
	m_bblock_en = 1; // ?
	m_visa_dis = 0;

	reset_dma(0);
	reset_dma(1);
}


//-------------------------------------------------
//  device_post_load - device-specific post-load
//-------------------------------------------------

void m2_cde_device::device_post_load()
{

}


//-------------------------------------------------
//  trigger_ready_int - flag CD ready
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(m2_cde_device::trigger_ready_int)
{
	// TODO: Do we need to do more things here?
	set_interrupt(CDE_ID_READY);
}


//-------------------------------------------------
//  set_interrupt -
//-------------------------------------------------

void m2_cde_device::set_interrupt(uint32_t intmask)
{
	m_int_status |= (uint32_t)intmask;
	update_interrupts();
}


//-------------------------------------------------
//  update_interrupts -
//-------------------------------------------------

void m2_cde_device::update_interrupts()
{
	if (m_int_status & m_int_enable)
		m_int_handler(ASSERT_LINE);
	else
		m_int_handler(CLEAR_LINE);
}



/***************************************************************************
    PUBLIC FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  read -
//-------------------------------------------------

uint32_t m2_cde_device::read(address_space &space, offs_t offset, uint32_t mem_mask)
{
	const uint32_t byte_offs = offset << 2;
	uint32_t data = 0;

	switch (byte_offs)
	{
		case CDE_DEVICE_ID:
		{
			data = 0x00010000;
			break;
		}
		case CDE_VERSION:
		{
			data = 0; // TODO
			break;
		}
		case CDE_SDBG_CNTL:
		{
			data = m_sdbg_cntl;
			break;
		}
		case CDE_SDBG_RD:
		{
			data = m_sdbg_in;
			break;
		}
		case CDE_INT_STS:
		{
			data = m_int_status;
			break;
		}
		case CDE_CD_STS_RD:
		{
			data = 0x000; // Status ready = 0x100
			break;
		}
		case CDE_INT_ENABLE:
		{
			data = m_int_enable;
			break;
		}
		case CDE_DEV_DETECT:
		{
			data = 0x0; // ?
			break;
		};
		case CDE_BBLOCK:
		{
			// 8, 80
			data = 0x80; // Needs to be non-zero
			break;
		}
		case CDE_UNIQ_ID_RD:
		{
			data = 0xffffffff; // ?
			break;
		}
		case CDE_BBLOCK_EN:
		{
			data = m_bblock_en;
			break;
		}
		case CDE_SYSTEM_CONF:
		{
			data = m_syscfg;
			break;
		}
		case CDE_MICRO_STATUS:
		{
			data = 0x20; // TODO
			break;
		}
		case CDE_MICRO_RWS:
		{
			break;
		}
		case CDE_VISA_DIS:
		{
			data = m_visa_dis;
			break;
		}
		case CDE_DMA1_CNTL:
		case CDE_DMA2_CNTL:
		{
			uint32_t ch = (byte_offs & 0x20) ? 1 : 0;
			data = m_dma[ch].m_cntl;
			break;
		}
		default:
		{
			//logerror("%s: CDE_R UNHANDLED: 0x%.8x 0x%.8x\n", machine().describe_context(), byte_offs, mem_mask));
		}
	}

	return data;
}


//-------------------------------------------------
//  write -
//-------------------------------------------------

void m2_cde_device::write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	uint32_t byte_offs = offset << 2;
	uint32_t dmach = byte_offs & 0x20 ? 1 : 0;

	m2_reg_wmode wm_cw = byte_offs & 0x400 ? REG_CLEAR : REG_WRITE;
	m2_reg_wmode wm_cs = byte_offs & 0x400 ? REG_CLEAR : REG_SET;

	byte_offs &= ~0x400;

	switch (byte_offs)
	{
		case CDE_SDBG_CNTL:
			// ........ ........ xxxxxxxx xxxx....      Clock scaler (written with 33MHz/38400 = 868)
			write_m2_reg(m_sdbg_cntl, data, wm_cw);
			break;
		case CDE_SDBG_WRT:
			m_sdbg_out_handler(data);
			set_interrupt(CDE_SDBG_WRT_DONE);
			break;
		case CDE_INT_STS:
			write_m2_reg(m_int_status, data, wm_cw);
			update_interrupts();
			break;
		case CDE_INT_ENABLE:
			write_m2_reg(m_int_enable, data, wm_cs);
			update_interrupts();
			break;
		case CDE_RESET_CNTL:
			if (data & 1)
			{
				// TODO: Should we reset both CPUs?
				downcast<cpu_device *>(&space.device())->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

				// TODO: Is this correct?
				m_bblock_en = 0;
			}
			else if (data & 2)
			{
				// TODO: Hard reset
			}

			break;
		case CDE_CD_CMD_WRT:
			//set_interrupt(CDE_CD_CMD_WRT_DONE); // ?
			//set_interrupt(CDE_CD_STS_FL_DONE); // ?
			break;
		case CDE_UNIQ_ID_CMD:
			// TODO: What is this?
			m_cd_ready_timer->adjust(attotime::from_usec(250));
			break;
		case CDE_BBLOCK:
			break;

		case CDE_DEV0_SETUP:
		case CDE_DEV1_SETUP:
		case CDE_DEV2_SETUP:
		case CDE_DEV3_SETUP:
		case CDE_DEV4_SETUP:
		case CDE_DEV5_SETUP:
		case CDE_DEV6_SETUP:
		case CDE_DEV7_SETUP:
		{
			uint32_t id = (byte_offs - CDE_DEV0_SETUP) >> 3;
			write_m2_reg(m_bio_device[id].m_setup, data, wm_cw);
			break;
		}

		case CDE_DEV0_CYCLE_TIME:
		case CDE_DEV1_CYCLE_TIME:
		case CDE_DEV2_CYCLE_TIME:
		case CDE_DEV3_CYCLE_TIME:
		case CDE_DEV4_CYCLE_TIME:
		case CDE_DEV5_CYCLE_TIME:
		case CDE_DEV6_CYCLE_TIME:
		case CDE_DEV7_CYCLE_TIME:
		{
			uint32_t id = (byte_offs - CDE_DEV0_CYCLE_TIME) >> 3;
			write_m2_reg(m_bio_device[id].m_cycle_time, data, wm_cw);
			break;
		}

//      case CDE_SYSTEM_CONF:
		case CDE_VISA_DIS:
			write_m2_reg(m_visa_dis, data, wm_cw);
			break;
		case CDE_MICRO_RWS:
		case CDE_MICRO_WI:
		case CDE_MICRO_WOB:
		case CDE_MICRO_WO:
		case CDE_MICRO_STATUS:
			break;

		case CDE_DMA1_CNTL:
		case CDE_DMA2_CNTL:
		{
			uint32_t &ctrl = m_dma[dmach].m_cntl;
			uint32_t old = ctrl;

			write_m2_reg(ctrl, data, wm_cw);

			if (!(old & CDE_DMA_RESET) && (ctrl & CDE_DMA_RESET))
				reset_dma(dmach);

			if (!(old & CDE_DMA_CURR_VALID) && (ctrl & CDE_DMA_CURR_VALID))
				start_dma(dmach);

			break;
		}
		case CDE_DMA1_CBAD:
		case CDE_DMA2_CBAD:
			write_m2_reg(m_dma[dmach].m_cbad, data, wm_cw);
			break;
		case CDE_DMA1_CPAD:
		case CDE_DMA2_CPAD:
			write_m2_reg(m_dma[dmach].m_cpad, data, wm_cw);
			break;
		case CDE_DMA1_CCNT:
		case CDE_DMA2_CCNT:
			write_m2_reg(m_dma[dmach].m_ccnt, data, wm_cw);
			break;
		default:
			//logerror("%s: CDE_W UNHANDLED: 0x%.8x 0x%.8x 0x%.8x\n", machine().describe_context(), byte_offs, data, mem_mask);
			break;
	}
}


//-------------------------------------------------
//  sdbg_in -
//-------------------------------------------------

void m2_cde_device::sdbg_in(uint32_t data)
{
	m_sdbg_in = data;
	set_interrupt(CDE_SDBG_RD_DONE);
}




/***************************************************************************
    PRIVATE FUNCTIONS
***************************************************************************/

//-------------------------------------------------
//  reset_dma - Reset a DMA channel
//-------------------------------------------------

void m2_cde_device::reset_dma(uint32_t ch)
{
	m_dma[ch].m_cntl = 0;
	m_dma[ch].m_timer->adjust(attotime::never);
}


//-------------------------------------------------
//  start_dma - DMA between the PowerBus and BioBus
//-------------------------------------------------

void m2_cde_device::start_dma(uint32_t ch)
{
	dma_channel &dma_ch = m_dma[ch];
	address_space *dma_space = &m_cpu1->space();

	// TODO: DMA timing is probably inaccurate
	attotime delay = attotime::from_nsec(10);// * dma_ch.m_ccnt;

//  attotime delay = clocks_to_attotime(4 * dma_ch.m_ccnt);
	dma_ch.m_timer->adjust(delay, (int)ch);

	if (dma_ch.m_cntl & CDE_DMA_DIRECTION)
	{
		// PowerBus to BioBus
		throw emu_fatalerror("m2_cde_device::start_dma: CDE PowerBus to BioBus DMA currently unsupported");
	}
	else
	{
		// BioBus to PowerBus
#if 0
		logerror("%s: CDE DMA %u: [%.8x] -> [%.8x], 0x%.8x bytes\n", machine().describe_context(), ch, dma_ch.m_cbad, dma_ch.m_cpad, dma_ch.m_ccnt);
#endif
		// Determine the BioBus device from the address
		const uint32_t slot = address_to_biobus_slot(dma_ch.m_cbad);

		// Get the device parameters
		const uint32_t setup = m_bio_device[slot].m_setup;

		if (setup & CDE_DATAWIDTH_16)
		{
			// 16-bit case
			if (dma_ch.m_ccnt & 1)
				throw emu_fatalerror("m2_cde_device::start_dma: 16-bit DMA: Byte count must be even?");
			if (dma_ch.m_cpad & 1)
				throw emu_fatalerror("m2_cde_device::start_dma: 16-bit DMA: DMA destination must be word aligned?");

			const uint32_t srcinc = setup & CDE_READ_SETUP_IO ? 0 : 2;

			while (dma_ch.m_ccnt > 0)
			{
				uint16_t data = dma_space->read_word_unaligned(dma_ch.m_cbad); // FIX ME
				dma_space->write_word(dma_ch.m_cpad, data);

				dma_ch.m_cbad += srcinc;
				dma_ch.m_cpad += 2;
				dma_ch.m_ccnt -= 2;
			}
		}
		else
		{
			// 8-bit case
			const uint32_t srcinc = setup & CDE_READ_SETUP_IO ? 0 : 1;

			fatalerror("8-bit DMA untested\n");

			while (dma_ch.m_ccnt > 0)
			{
				uint8_t data = dma_space->read_byte(dma_ch.m_cbad);
				dma_space->write_byte(dma_ch.m_cpad, data);

				dma_ch.m_cbad += srcinc;
				dma_ch.m_cpad += 1;
				dma_ch.m_ccnt -= 1;
			}
		}
	}
}

//-------------------------------------------------
//  next_dma - Start the next DMA if set
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(m2_cde_device::next_dma)
{
	const uint32_t ch = (uint32_t)param;
	dma_channel &dma_ch = m_dma[ch];

	if (dma_ch.m_ccnt != 0)
		throw emu_fatalerror("m2_cde_device::next_dma: DMA count non-zero during next DMA");

	if (dma_ch.m_cntl & CDE_DMA_NEXT_VALID)
	{
		logerror("NEXT DMA CODE UNTESTED");

		// Update current address and count registers
		dma_ch.m_cbad = dma_ch.m_nbad;
		dma_ch.m_cpad = dma_ch.m_npad;
		dma_ch.m_ccnt = dma_ch.m_ncnt;
		dma_ch.m_cntl |= CDE_DMA_CURR_VALID;

		// Disable looping
		if (!(dma_ch.m_cntl & CDE_DMA_GO_FOREVER))
			dma_ch.m_cntl &= ~CDE_DMA_NEXT_VALID;

		start_dma(ch);
	}
	else
	{
		// DMA complete
		dma_ch.m_cntl &= ~CDE_DMA_CURR_VALID;
		set_interrupt(ch == 0 ? CDE_DMA1_DONE : CDE_DMA2_DONE);
	}
}


/***************************************************************************
 MPEG DEVICE
 ***************************************************************************/

//-------------------------------------------------
//  m2_mpeg_device - constructor
//-------------------------------------------------

m2_mpeg_device::m2_mpeg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M2_MPEG, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m2_mpeg_device::device_start()
{

}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m2_mpeg_device::device_reset()
{

}

//-------------------------------------------------
//  read
//-------------------------------------------------

uint32_t m2_mpeg_device::read(offs_t offset)
{
	logerror("%s: MPEG READ: %08X\n", machine().describe_context(), offset);
	return 0;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void m2_mpeg_device::write(offs_t offset, uint32_t data)
{
	logerror("%s: MPEG WRITE: %08X %08X\n", machine().describe_context(), offset, data);
}

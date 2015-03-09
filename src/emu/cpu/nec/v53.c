/* V53 */

// V33 / V33A cores with onboard peripherals

// Interrupt Controller is uPD71059 equivalent (a PIC8259 clone?)
// DMA Controller can operate in modes providing a subset of the uPD71071 or uPD71037 functionality (some modes unavailable / settings ignored) (uPD71071 mode is an extended am9517a, uPD71037 mode is ??)
// Serial Controller is based on the uPD71051 but with some changes (i8251 clone?)
// Timer Unit is functionally identical to uPD71054 (which in turn is said to be the same as a pit8253)

#include "emu.h"
#include "v53.h"


const device_type V53 = &device_creator<v53_device>;
const device_type V53A =&device_creator<v53a_device>;

WRITE8_MEMBER(v53_base_device::BSEL_w)
{
	printf("v53: BSEL_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::BADR_w)
{
	printf("v53: BADR_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::BRC_w)
{
	printf("v53: BRC_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WMB0_w)
{
	printf("v53: WMB0_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY1_w)
{
	printf("v53: WCY1_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY0_w)
{
	printf("v53: WCY0_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WAC_w)
{
	printf("v53: WAC_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::TCKS_w)
{
	printf("v53: TCKS_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::SBCR_w)
{
	printf("v53: SBCR_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::REFC_w)
{
	printf("v53: REFC_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WMB1_w)
{
	printf("v53: WMB1_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY2_w)
{
	printf("v53: WCY2_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY3_w)
{
	printf("v53: WCY3_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::WCY4_w)
{
	printf("v53: WCY4_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::SULA_w)
{
	printf("v53: SULA_w %02x\n", data);
	m_SULA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::TULA_w)
{
	printf("v53: TULA_w %02x\n", data);
	m_TULA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::IULA_w)
{
	printf("v53: IULA_w %02x\n", data);
	m_IULA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::DULA_w)
{
	printf("v53: DULA_w %02x\n", data);
	m_DULA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::OPHA_w)
{
	printf("v53: OPHA_w %02x\n", data);
	m_OPHA = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::OPSEL_w)
{
	printf("v53: OPSEL_w %02x\n", data);
	m_OPSEL = data;
	install_peripheral_io();
}

WRITE8_MEMBER(v53_base_device::SCTL_w)
{
	// bit 7: unused
	// bit 6: unused
	// bit 5: unused
	// bit 4: SCU input clock source
	// bit 3: uPD71037 DMA mode - Carry A20
	// bit 2: uPD71037 DMA mode - Carry A16
	// bit 1: uPD71037 DMA mode enable (otherwise in uPD71071 mode)
	// bit 0: Onboard pripheral I/O maps to 8-bit boundaries? (otherwise 16-bit)

	printf("v53: SCTL_w %02x\n", data);
	m_SCTL = data;
	install_peripheral_io();
}
/*
m_WCY0 = 0x07;
m_WCY1 = 0x77;
m_WCY2 = 0x77;
m_WCY3 = 0x77;
m_WCY4 = 0x77;
m_WMB0 = 0x77;
m_WMB1 = 0x77;
m_WAC =  0x00;
m_TCKS = 0x00;
m_RFC =  0x80;
m_SBCR = 0x00;
m_BRC =  0x00;
// SCU
m_SMD =  0x4b;
m_SCM =  0x00;
m_SIMK = 0x03;
m_SST = 0x04;
// DMA
m_DCH = 0x01;
m_DMD = 0x00;
m_DCC = 0x0000;
m_DST = 0x00;
m_DMK = 0x0f;
*/

void v53_base_device::device_reset()
{
	nec_common_device::device_reset();

	m_SCTL = 0x00;
	m_OPSEL= 0x00;

	// peripheral addresses
	m_SULA = 0x00;
	m_TULA = 0x00;
	m_IULA = 0x00;
	m_DULA = 0x00;
	m_OPHA = 0x00;

}

void v53_base_device::device_start()
{
	nec_common_device::device_start();
}

void v53_base_device::install_peripheral_io()
{
	// unmap everything in I/O space up to the fixed position registers (we avoid overwriting them, it isn't a valid config)
	space(AS_IO).unmap_readwrite(0x1000, 0xfeff); // todo, we need to have a way to NOT unmap things defined in the drivers, but instead have this act as an overlay mapping / unampping only!!

	// IOAG determines if the handlers used 8-bit or 16-bit access
	// the hng64.c games first set everything up in 8-bit mode, then
	// do the procedure again in 16-bit mode before using them?!

	int IOAG = m_SCTL & 1;

	if (m_OPSEL & 0x01) // DMA Unit available
	{
		UINT16 base = (m_OPHA << 8) | m_DULA;
		base &= 0xfffe;

		if (m_SCTL & 0x02) // uPD71037 mode
		{
			if (IOAG) // 8-bit 
			{

			}
			else
			{
			}
		}
		else // uPD71071 mode
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x0f, read8_delegate(FUNC(upd71071_v53_device::read), (upd71071_v53_device*)m_dma_71071mode), write8_delegate(FUNC(upd71071_v53_device::write),  (upd71071_v53_device*)m_dma_71071mode), 0xffff);
		}
	}

	if (m_OPSEL & 0x02) // Interupt Control Unit available
	{
		UINT16 base = (m_OPHA << 8) | m_IULA;
		base &= 0xfffe;

		if (IOAG) // 8-bit 
		{

		}
		else
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8_delegate(FUNC(v53_base_device::icu_0_r), this), write8_delegate(FUNC(v53_base_device::icu_0_w), this), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x02, base+0x03, read8_delegate(FUNC(v53_base_device::icu_1_r), this), write8_delegate(FUNC(v53_base_device::icu_1_w), this), 0x00ff);

		}
	}

	if (m_OPSEL & 0x04) // Timer Control Unit available
	{
		UINT16 base = (m_OPHA << 8) | m_TULA;
		//printf("installing TCU to %04x\n", base);
		base &= 0xfffe;

		if (IOAG) // 8-bit 
		{

		}
		else
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8_delegate(FUNC(v53_base_device::tmu_tst0_r), this), write8_delegate(FUNC(v53_base_device::tmu_tct0_w), this), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x02, base+0x03, read8_delegate(FUNC(v53_base_device::tmu_tst1_r), this), write8_delegate(FUNC(v53_base_device::tmu_tct1_w), this), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x04, base+0x05, read8_delegate(FUNC(v53_base_device::tmu_tst2_r), this), write8_delegate(FUNC(v53_base_device::tmu_tct2_w), this), 0x00ff);
			space(AS_IO).install_write_handler(base+0x06, base+0x07, write8_delegate(FUNC(v53_base_device::tmu_tmd_w), this), 0x00ff);
		}
	}

	if (m_OPSEL & 0x08) // Serial Control Unit available
	{
		UINT16 base = (m_OPHA << 8) | m_SULA;
		base &= 0xfffe;

		if (IOAG) // 8-bit 
		{

		}
		else
		{
			space(AS_IO).install_readwrite_handler(base+0x00, base+0x01, read8_delegate(FUNC(v53_base_device::scu_srb_r), this), write8_delegate(FUNC(v53_base_device::scu_stb_w), this), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x02, base+0x03, read8_delegate(FUNC(v53_base_device::scu_sst_r), this), write8_delegate(FUNC(v53_base_device::scu_scm_w), this), 0x00ff);
			space(AS_IO).install_write_handler(base+0x04, base+0x05, write8_delegate(FUNC(v53_base_device::scu_smd_w), this), 0x00ff);
			space(AS_IO).install_readwrite_handler(base+0x06, base+0x07, read8_delegate(FUNC(v53_base_device::scu_simk_r), this), write8_delegate(FUNC(v53_base_device::scu_simk_w), this), 0x00ff);

		}
	}

}

/*** ICU ***/



READ8_MEMBER(v53_base_device::icu_0_r)
{
	printf("v53: icu_0_r\n");
	return 0;
}

WRITE8_MEMBER(v53_base_device::icu_0_w)
{
	printf("v53: icu_0_w %02x\n", data);
}

READ8_MEMBER(v53_base_device::icu_1_r)
{
	printf("v53: icu_1_r\n");
	return 0;
}

WRITE8_MEMBER(v53_base_device::icu_1_w)
{
	printf("v53: icu_1_w %02x\n", data);
}

/*** SCU ***/

READ8_MEMBER(v53_base_device::scu_srb_r)
{
	printf("v53: scu_srb_r\n");
	return 0;
}

WRITE8_MEMBER(v53_base_device::scu_stb_w)
{
	printf("v53: scu_stb_w %02x\n", data);
}

READ8_MEMBER(v53_base_device::scu_sst_r)
{
	printf("v53: scu_sst_r\n");
	return 0;
}

WRITE8_MEMBER(v53_base_device::scu_scm_w)
{
	printf("v53: scu_scm_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::scu_smd_w)
{
	printf("v53: scu_smd_w %02x\n", data);
}

READ8_MEMBER(v53_base_device::scu_simk_r)
{
	printf("v53: scu_simk_r\n");
	return 0;
}

WRITE8_MEMBER(v53_base_device::scu_simk_w)
{
	printf("v53: scu_simk_w %02x\n", data);
}


/*** TCU ***/

WRITE8_MEMBER(v53_base_device::tmu_tct0_w) { m_pit->write(space, 0, data); }
WRITE8_MEMBER(v53_base_device::tmu_tct1_w) { m_pit->write(space, 1, data); }
WRITE8_MEMBER(v53_base_device::tmu_tct2_w) { m_pit->write(space, 2, data); }
WRITE8_MEMBER(v53_base_device::tmu_tmd_w)  { m_pit->write(space, 3, data); }


READ8_MEMBER(v53_base_device::tmu_tst0_r) {	return m_pit->read(space, 0); }
READ8_MEMBER(v53_base_device::tmu_tst1_r) {	return m_pit->read(space, 1); }
READ8_MEMBER(v53_base_device::tmu_tst2_r) {	return m_pit->read(space, 2); }

/*** DMA ***/

// could be wrong / nonexistent 
WRITE_LINE_MEMBER(v53_base_device::dreq0_trampoline_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_dma_71071mode->dreq0_w(state);
	}
	else
	{
		printf("v53: dreq0 not in 71071mode\n");
	}
}

WRITE_LINE_MEMBER(v53_base_device::dreq1_trampoline_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_dma_71071mode->dreq1_w(state);
	}
	else
	{
		printf("v53: dreq1 not in 71071mode\n");
	}
}

WRITE_LINE_MEMBER(v53_base_device::dreq2_trampoline_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_dma_71071mode->dreq2_w(state);
	}
	else
	{
		printf("v53: dreq2 not in 71071mode\n");
	}
}

WRITE_LINE_MEMBER(v53_base_device::dreq3_trampoline_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_dma_71071mode->dreq3_w(state);
	}
	else
	{
		printf("v53: dreq3 not in 71071mode\n");
	}
}

WRITE_LINE_MEMBER(v53_base_device::hack_trampoline_w)
{
	if (!(m_SCTL & 0x02))
	{
		m_dma_71071mode->hack_w(state);
	}
	else
	{
		printf("v53: hack_trampoline_w not in 71071mode\n");
	}
}

/* General stuff */

static ADDRESS_MAP_START( v53_internal_port_map, AS_IO, 16, v53_base_device )
	AM_RANGE(0xffe0, 0xffe1) AM_WRITE8( BSEL_w,  0x00ff) // 0xffe0 // uPD71037 DMA mode bank selection register
	AM_RANGE(0xffe0, 0xffe1) AM_WRITE8( BADR_w,  0xff00) // 0xffe1 // uPD71037 DMA mode bank register peripheral mapping (also uses OPHA)
//	AM_RANGE(0xffe2, 0xffe3) // (reserved     ,  0x00ff) // 0xffe2
//	AM_RANGE(0xffe2, 0xffe3) // (reserved     ,  0xff00) // 0xffe3
//	AM_RANGE(0xffe4, 0xffe5) // (reserved     ,  0x00ff) // 0xffe4
//	AM_RANGE(0xffe4, 0xffe5) // (reserved     ,  0xff00) // 0xffe5
//	AM_RANGE(0xffe6, 0xffe7) // (reserved     ,  0x00ff) // 0xffe6
//	AM_RANGE(0xffe6, 0xffe7) // (reserved     ,  0xff00) // 0xffe7
//	AM_RANGE(0xffe8, 0xffe9) // (reserved     ,  0x00ff) // 0xffe8
	AM_RANGE(0xffe8, 0xffe9) AM_WRITE8( BRC_w ,  0xff00) // 0xffe9 // baud rate counter (used for serial peripheral)
	AM_RANGE(0xffea, 0xffeb) AM_WRITE8( WMB0_w,  0x00ff) // 0xffea // waitstate control
	AM_RANGE(0xffea, 0xffeb) AM_WRITE8( WCY1_w,  0xff00) // 0xffeb // waitstate control
	AM_RANGE(0xffec, 0xffed) AM_WRITE8( WCY0_w,  0x00ff) // 0xffec // waitstate control
	AM_RANGE(0xffec, 0xffed) AM_WRITE8( WAC_w,   0xff00) // 0xffed // waitstate control
//	AM_RANGE(0xffee, 0xffef) // (reserved     ,  0x00ff) // 0xffee
//	AM_RANGE(0xffee, 0xffef) // (reserved     ,  0xff00) // 0xffef
	AM_RANGE(0xfff0, 0xfff1) AM_WRITE8( TCKS_w,  0x00ff) // 0xfff0 // timer clocks
	AM_RANGE(0xfff0, 0xfff1) AM_WRITE8( SBCR_w,  0xff00) // 0xfff1 // internal clock divider, halt behavior etc.
	AM_RANGE(0xfff2, 0xfff3) AM_WRITE8( REFC_w,  0x00ff) // 0xfff2 // ram refresh control
	AM_RANGE(0xfff2, 0xfff3) AM_WRITE8( WMB1_w,  0xff00) // 0xfff3 // waitstate control
	AM_RANGE(0xfff4, 0xfff5) AM_WRITE8( WCY2_w,  0x00ff) // 0xfff4 // waitstate control
	AM_RANGE(0xfff4, 0xfff5) AM_WRITE8( WCY3_w,  0xff00) // 0xfff5 // waitstate control
	AM_RANGE(0xfff6, 0xfff7) AM_WRITE8( WCY4_w,  0x00ff) // 0xfff6 // waitstate control
//	AM_RANGE(0xfff6, 0xfff7) // (reserved     ,  0xff00) // 0xfff7
	AM_RANGE(0xfff8, 0xfff9) AM_WRITE8( SULA_w,  0x00ff) // 0xfff8 // peripheral mapping
	AM_RANGE(0xfff8, 0xfff9) AM_WRITE8( TULA_w,  0xff00) // 0xfff9 // peripheral mapping
	AM_RANGE(0xfffa, 0xfffb) AM_WRITE8( IULA_w,  0x00ff) // 0xfffa // peripheral mapping
	AM_RANGE(0xfffa, 0xfffb) AM_WRITE8( DULA_w,  0xff00) // 0xfffb // peripheral mapping
	AM_RANGE(0xfffc, 0xfffd) AM_WRITE8( OPHA_w,  0x00ff) // 0xfffc // peripheral mapping (upper bits, common)
	AM_RANGE(0xfffc, 0xfffd) AM_WRITE8( OPSEL_w, 0xff00) // 0xfffd // peripheral enabling
	AM_RANGE(0xfffe, 0xffff) AM_WRITE8( SCTL_w,  0x00ff) // 0xfffe // peripheral configuration (& byte / word mapping)
//	AM_RANGE(0xfffe, 0xffff) // (reserved     ,  0xff00) // 0xffff
ADDRESS_MAP_END

WRITE_LINE_MEMBER(v53_base_device::dma_hrq_changed)
{
	// pass this back to the driver? / expose externally? 
	m_dma_71071mode->hack_w(state);
}

WRITE8_MEMBER(v53_base_device::dma_io_3_w)
{
//	logerror("dma_io_3_w %02x\n", data);
}

READ8_MEMBER(v53_base_device::dma_memin_r)
{
	UINT8 ret = rand();
//	logerror("dma_memin_r offset %08x %02x\n", offset, ret);
	return ret;
}

READ8_MEMBER(v53_base_device::get_pic_ack)
{
	return 0;
}

WRITE_LINE_MEMBER( v53_base_device::upd71059_irq_w)
{
	printf("upd71059_irq_w %d", state);
}

static MACHINE_CONFIG_FRAGMENT( v53 )
	MCFG_DEVICE_ADD("pit", PIT8254, 0) // functionality identical to uPD71054
	MCFG_PIT8253_CLK0(16000000/2/8)
	//MCFG_PIT8253_OUT0_HANDLER(WRITELINE(v53_base_device, pit_out0))

	MCFG_DEVICE_ADD("upd71071dma", UPD71071_V53, 4000000)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(v53_base_device, dma_hrq_changed))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(v53_base_device, dma_io_3_w))
	MCFG_I8237_IN_MEMR_CB(READ8(v53_base_device, dma_memin_r))
	
	
	MCFG_PIC8259_ADD( "upd71059pic", WRITELINE(v53_base_device, upd71059_irq_w), VCC, READ8(v53_base_device,get_pic_ack))

	MCFG_DEVICE_ADD("upd71051", I8251, 0) 

MACHINE_CONFIG_END

machine_config_constructor v53_base_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( v53 );
}


v53_base_device::v53_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, offs_t fetch_xor, UINT8 prefetch_size, UINT8 prefetch_cycles, UINT32 chip_type)
	: nec_common_device(mconfig, type, name, tag, owner, clock, shortname, true, fetch_xor, prefetch_size, prefetch_cycles, chip_type),
	m_io_space_config( "io", ENDIANNESS_LITTLE, 16, 16, 0, ADDRESS_MAP_NAME( v53_internal_port_map ) ),
	m_pit(*this, "pit"),
	m_dma_71071mode(*this, "upd71071dma"),
	m_upd71059(*this, "upd71059pic"),
	m_upd71051(*this, "upd71051")
{
}


v53_device::v53_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: v53_base_device(mconfig, V53, "V53", tag, owner, clock, "v53", BYTE_XOR_LE(0), 6, 1, V33_TYPE)
{
}


v53a_device::v53a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: v53_base_device(mconfig, V53A, "V53A", tag, owner, clock, "v53a", BYTE_XOR_LE(0), 6, 1, V33_TYPE)
{
}


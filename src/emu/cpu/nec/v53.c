/* V53 */

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
	space(AS_IO).unmap_readwrite(0x0000, 0xfeff);

	// IOAG determines if the handlers used 8-bit or 16-bit access
	// the hng64.c games first set everything up in 8-bit mode, then
	// do the procedure again in 16-bit mode before using them?!

	int IOAG = m_SCTL & 1;

	if (m_OPSEL & 0x01) // DMA Unit available
	{
		if (IOAG) // 8-bit 
		{

		}
		else
		{
			
		}
	}

	if (m_OPSEL & 0x02) // Interupt Control Unit available
	{
		if (IOAG) // 8-bit 
		{

		}
		else
		{

		}
	}

	if (m_OPSEL & 0x04) // Timer Control Unit available
	{
		UINT16 base = (m_OPHA << 8) | m_TULA;
		printf("installing TCU to %04x\n", base);

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

		if (IOAG) // 8-bit 
		{

		}
		else
		{

		}
	}

}

/*** TCU ***/

READ8_MEMBER(v53_base_device::tmu_tst0_r)
{
	printf("v53: tmu_tst0_r\n");
	return 0;
}

WRITE8_MEMBER(v53_base_device::tmu_tct0_w)
{
	printf("v53: tmu_tct0_w %02x\n", data);
}

READ8_MEMBER(v53_base_device::tmu_tst1_r)
{
	printf("v53: tmu_tst1_r\n");
	return 0;
}

WRITE8_MEMBER(v53_base_device::tmu_tct1_w)
{
	printf("v53: tmu_tct1_w %02x\n", data);
}

READ8_MEMBER(v53_base_device::tmu_tst2_r)
{
	printf("v53: tmu_tst2_r\n");
	return 0;
}

WRITE8_MEMBER(v53_base_device::tmu_tct2_w)
{
	printf("v53: tmu_tct2_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::tmu_tmd_w)
{
	printf("v53: tmu_tmd_w %02x\n", data);
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

v53_base_device::v53_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, offs_t fetch_xor, UINT8 prefetch_size, UINT8 prefetch_cycles, UINT32 chip_type)
	: nec_common_device(mconfig, type, name, tag, owner, clock, shortname, true, fetch_xor, prefetch_size, prefetch_cycles, chip_type),
	m_io_space_config( "io", ENDIANNESS_LITTLE, 16, 16, 0, ADDRESS_MAP_NAME( v53_internal_port_map ) )
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


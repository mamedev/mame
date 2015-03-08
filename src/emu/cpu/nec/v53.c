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
}

WRITE8_MEMBER(v53_base_device::TULA_w)
{
	printf("v53: TULA_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::IULA_w)
{
	printf("v53: IULA_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::DULA_w)
{
	printf("v53: DULA_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::OPHA_w)
{
	printf("v53: OPHA_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::OPSEL_w)
{
	printf("v53: OPSEL_w %02x\n", data);
}

WRITE8_MEMBER(v53_base_device::SCTL_w)
{
	printf("v53: SCTL_w %02x\n", data);
}

static ADDRESS_MAP_START( v53_internal_port_map, AS_IO, 16, v53_base_device )
	AM_RANGE(0xffe0, 0xffe1) AM_WRITE8( BSEL_w,  0x00ff) // 0xffe0
	AM_RANGE(0xffe0, 0xffe1) AM_WRITE8( BADR_w,  0xff00) // 0xffe1
//	AM_RANGE(0xffe2, 0xffe3) // (reserved     ,  0x00ff) // 0xffe2
//	AM_RANGE(0xffe2, 0xffe3) // (reserved     ,  0xff00) // 0xffe3
//	AM_RANGE(0xffe4, 0xffe5) // (reserved     ,  0x00ff) // 0xffe4
//	AM_RANGE(0xffe4, 0xffe5) // (reserved     ,  0xff00) // 0xffe5
//	AM_RANGE(0xffe6, 0xffe7) // (reserved     ,  0x00ff) // 0xffe6
//	AM_RANGE(0xffe6, 0xffe7) // (reserved     ,  0xff00) // 0xffe7
//	AM_RANGE(0xffe8, 0xffe9) // (reserved     ,  0x00ff) // 0xffe8
	AM_RANGE(0xffe8, 0xffe9) AM_WRITE8( BRC_w ,  0xff00) // 0xffe9
	AM_RANGE(0xffea, 0xffeb) AM_WRITE8( WMB0_w,  0x00ff) // 0xffea
	AM_RANGE(0xffea, 0xffeb) AM_WRITE8( WCY1_w,  0xff00) // 0xffeb
	AM_RANGE(0xffec, 0xffed) AM_WRITE8( WCY0_w,  0x00ff) // 0xffec
	AM_RANGE(0xffec, 0xffed) AM_WRITE8( WAC_w,   0xff00) // 0xffed
//	AM_RANGE(0xffee, 0xffef) // (reserved     ,  0x00ff) // 0xffee
//	AM_RANGE(0xffee, 0xffef) // (reserved     ,  0xff00) // 0xffef
	AM_RANGE(0xfff0, 0xfff1) AM_WRITE8( TCKS_w,  0x00ff) // 0xfff0
	AM_RANGE(0xfff0, 0xfff1) AM_WRITE8( SBCR_w,  0xff00) // 0xfff1
	AM_RANGE(0xfff2, 0xfff3) AM_WRITE8( REFC_w,  0x00ff) // 0xfff2
	AM_RANGE(0xfff2, 0xfff3) AM_WRITE8( WMB1_w,  0xff00) // 0xfff3
	AM_RANGE(0xfff4, 0xfff5) AM_WRITE8( WCY2_w,  0x00ff) // 0xfff4
	AM_RANGE(0xfff4, 0xfff5) AM_WRITE8( WCY3_w,  0xff00) // 0xfff5
	AM_RANGE(0xfff6, 0xfff7) AM_WRITE8( WCY4_w,  0x00ff) // 0xfff6
//	AM_RANGE(0xfff6, 0xfff7) // (reserved     ,  0xff00) // 0xfff7
	AM_RANGE(0xfff8, 0xfff9) AM_WRITE8( SULA_w,  0x00ff) // 0xfff8
	AM_RANGE(0xfff8, 0xfff9) AM_WRITE8( TULA_w,  0xff00) // 0xfff9
	AM_RANGE(0xfffa, 0xfffb) AM_WRITE8( IULA_w,  0x00ff) // 0xfffa
	AM_RANGE(0xfffa, 0xfffb) AM_WRITE8( DULA_w,  0xff00) // 0xfffb
	AM_RANGE(0xfffc, 0xfffd) AM_WRITE8( OPHA_w,  0x00ff) // 0xfffc
	AM_RANGE(0xfffc, 0xfffd) AM_WRITE8( OPSEL_w, 0xff00) // 0xfffd
	AM_RANGE(0xfffe, 0xffff) AM_WRITE8( SCTL_w,  0x00ff) // 0xfffe
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


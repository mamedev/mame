// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    cop400.c

    National Semiconductor COP400 Emulator.

****************************************************************************

    Type        ROM     RAM     G       D       IN

    COP410      512x8   32x4                    N/A
    COP411      512x8   32x4    0-2     0-1     N/A
    COP413      512x8   32x4            N/A     N/A
    COP401      N/A     32x4                    N/A

    COP420      1024x8  64x4
    COP421      1024x8  64x4                    N/A
    COP422      1024x8  64x4    2-3     2-3     N/A
    COP402      N/A     64x4

    COP444L     2048x8  128x4
    COP445L     2048x8  128x4                   N/A
    COP404L     N/A     128x4

    COP424C     1024x8  64x4
    COP425C     1024x8  64x4                    N/A
    COP426C     1024x8  64x4    2-3     2-3

    COP444C     2048x8  128x4
    COP445C     2048x8  128x4                   N/A
    COP446C     2048x8  128x4   2-3     2-3
    COP404C     N/A     128x4

****************************************************************************

    Prefix      Temperature Range

    COP4xx      0C ... 70C
    COP3xx      -40C ... +85C
    COP2xx      -55C ... +125C

***************************************************************************/

/*

    TODO:

    - COP410L/COP410C
    - save internal RAM when CKO is RAM power supply pin
    - COP404L opcode map switching, dual timer, microbus enable
    - improve SIO output timing for interface with 93CXX serial EEPROMs

*/

#include "emu.h"
#include "debugger.h"
#include "cop400.h"


DEFINE_DEVICE_TYPE(COP401, cop401_cpu_device, "cop401", "COP401")
DEFINE_DEVICE_TYPE(COP410, cop410_cpu_device, "cop410", "COP410")
DEFINE_DEVICE_TYPE(COP411, cop411_cpu_device, "cop411", "COP411")
DEFINE_DEVICE_TYPE(COP402, cop402_cpu_device, "cop402", "COP402")
DEFINE_DEVICE_TYPE(COP420, cop420_cpu_device, "cop420", "COP420")
DEFINE_DEVICE_TYPE(COP421, cop421_cpu_device, "cop421", "COP421")
DEFINE_DEVICE_TYPE(COP422, cop422_cpu_device, "cop422", "COP422")
DEFINE_DEVICE_TYPE(COP404L, cop404l_cpu_device, "cop404l", "COP404L")
DEFINE_DEVICE_TYPE(COP444L, cop444l_cpu_device, "cop444l", "COP444L")
DEFINE_DEVICE_TYPE(COP445L, cop445l_cpu_device, "cop445l", "COP445L")
DEFINE_DEVICE_TYPE(COP404C, cop404c_cpu_device, "cop404c", "COP404C")
DEFINE_DEVICE_TYPE(COP424C, cop424c_cpu_device, "cop424c", "COP424C")
DEFINE_DEVICE_TYPE(COP425C, cop425c_cpu_device, "cop425c", "COP425C")
DEFINE_DEVICE_TYPE(COP426C, cop426c_cpu_device, "cop426c", "COP426C")
DEFINE_DEVICE_TYPE(COP444C, cop444c_cpu_device, "cop444c", "COP444C")
DEFINE_DEVICE_TYPE(COP445C, cop445c_cpu_device, "cop445c", "COP445C")
DEFINE_DEVICE_TYPE(COP446C, cop446c_cpu_device, "cop446c", "COP446C")



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define LOG_MICROBUS 0


/***************************************************************************
    MACROS
***************************************************************************/

#define ROM(a)          m_direct->read_byte(a)
#define RAM_R(a)        m_data->read_byte(a)
#define RAM_W(a, v)     m_data->write_byte(a, v)

#define IN_G()          (m_read_g(0, 0xff) & m_g_mask)
#define IN_L()          m_read_l(0, 0xff)
#define IN_SI()         BIT(m_read_si(), 0)
#define IN_CKO()        BIT(m_read_cko(), 0)
#define IN_IN()         (m_in_mask ? m_read_in(0, 0xff) : 0)

#define OUT_G(v)        m_write_g(0, (v) & m_g_mask, 0xff)
#define OUT_L(v)        m_write_l(0, v, 0xff)
#define OUT_D(v)        m_write_d(0, (v) & m_d_mask, 0xff)
#define OUT_SK(v)       m_write_sk(v)
#define OUT_SO(v)       m_write_so(v)

#define PC              m_pc
#define A               m_a
#define B               m_b
#define C               m_c
#define G               m_g
#define Q               m_q
#define H               m_h
#define R               m_r
#define EN              m_en
#define SA              m_sa
#define SB              m_sb
#define SC              m_sc
#define SIO             m_sio
#define SKL             m_skl
#define T               m_t
#define IL              m_il


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( program_512b, AS_PROGRAM, 8, cop400_cpu_device )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( program_1kb, AS_PROGRAM, 8, cop400_cpu_device )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( program_2kb, AS_PROGRAM, 8, cop400_cpu_device )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( data_32b, AS_DATA, 8, cop400_cpu_device )
	AM_RANGE(0x00, 0x07) AM_MIRROR(0x08) AM_RAM
	AM_RANGE(0x10, 0x17) AM_MIRROR(0x08) AM_RAM
	AM_RANGE(0x20, 0x27) AM_MIRROR(0x08) AM_RAM
	AM_RANGE(0x30, 0x37) AM_MIRROR(0x08) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( data_64b, AS_DATA, 8, cop400_cpu_device )
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( data_128b, AS_DATA, 8, cop400_cpu_device )
	AM_RANGE(0x00, 0x7f) AM_RAM
ADDRESS_MAP_END


cop400_cpu_device::cop400_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, uint8_t program_addr_bits, uint8_t data_addr_bits, uint8_t featuremask, uint8_t g_mask, uint8_t d_mask, uint8_t in_mask, bool has_counter, bool has_inil, address_map_constructor internal_map_program, address_map_constructor internal_map_data)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, program_addr_bits, 0, internal_map_program)
	, m_data_config("data", ENDIANNESS_LITTLE, 8, data_addr_bits, 0, internal_map_data) // data width is really 4
	, m_read_l(*this)
	, m_read_l_tristate(*this)
	, m_write_l(*this)
	, m_read_g(*this)
	, m_write_g(*this)
	, m_write_d(*this)
	, m_read_in(*this)
	, m_read_si(*this)
	, m_write_so(*this)
	, m_write_sk(*this)
	, m_read_cko(*this)
	, m_cki(COP400_CKI_DIVISOR_16)
	, m_cko(COP400_CKO_OSCILLATOR_OUTPUT)
	, m_has_microbus(false)
	, m_has_counter(has_counter)
	, m_has_inil(has_inil)
	, m_featuremask(featuremask)
	, m_g_mask(g_mask)
	, m_d_mask(d_mask)
	, m_in_mask(in_mask)
{
	for (int i = 0; i < 256; i++) {
		m_InstLen[i] = 1;
	}

	m_InstLen[0x33] = m_InstLen[0x23] = 2;

	switch (featuremask)
	{
		case COP410_FEATURE:
			m_opcode_map = COP410_OPCODE_MAP;

			for (int r = 0; r < 2; r++) {
				m_InstLen[0x60 + r] = 2; // JMP
				m_InstLen[0x68 + r] = 2; // JSR
			}
			break;

		case COP420_FEATURE:
			m_opcode_map = COP420_OPCODE_MAP;

			for (int r = 0; r < 4; r++) {
				m_InstLen[0x60 + r] = 2; // JMP
				m_InstLen[0x68 + r] = 2; // JSR
			}
			break;

		case COP444L_FEATURE:
			m_opcode_map = COP444L_OPCODE_MAP;

			for (int r = 0; r < 8; r++) {
				m_InstLen[0x60 + r] = 2; // JMP
				m_InstLen[0x68 + r] = 2; // JSR
			}
			break;

		case COP424C_FEATURE:
			m_opcode_map = COP424C_OPCODE_MAP;

			for (int r = 0; r < 8; r++) {
				m_InstLen[0x60 + r] = 2; // JMP
				m_InstLen[0x68 + r] = 2; // JSR
			}
			break;

		default:
			fatalerror("No or unknown featuremask supplied\n");
	}
}

cop401_cpu_device::cop401_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP401, tag, owner, clock, 9, 6, COP410_FEATURE, 0xf, 0xf, 0, false, false, nullptr, ADDRESS_MAP_NAME(data_32b))
{
}

cop410_cpu_device::cop410_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP410, tag, owner, clock, 9, 6, COP410_FEATURE, 0xf, 0xf, 0, false, false, ADDRESS_MAP_NAME(program_512b), ADDRESS_MAP_NAME(data_32b))
{
}

cop411_cpu_device::cop411_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP411, tag, owner, clock, 9, 6, COP410_FEATURE, 0x7, 0x3, 0, false, false, ADDRESS_MAP_NAME(program_512b), ADDRESS_MAP_NAME(data_32b))
{
}

cop402_cpu_device::cop402_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP402, tag, owner, clock, 10, 6, COP420_FEATURE, 0xf, 0xf, 0xf, true, true, nullptr, ADDRESS_MAP_NAME(data_64b))
{
}

cop420_cpu_device::cop420_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP420, tag, owner, clock, 10, 6, COP420_FEATURE, 0xf, 0xf, 0xf, true, true, ADDRESS_MAP_NAME(program_1kb), ADDRESS_MAP_NAME(data_64b))
{
}

cop421_cpu_device::cop421_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP421, tag, owner, clock, 10, 6, COP420_FEATURE, 0xf, 0xf, 0, true, false, ADDRESS_MAP_NAME(program_1kb), ADDRESS_MAP_NAME(data_64b))
{
}

cop422_cpu_device::cop422_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP422, tag, owner, clock, 10, 6, COP420_FEATURE, 0xe, 0xe, 0, true, false, ADDRESS_MAP_NAME(program_1kb), ADDRESS_MAP_NAME(data_64b))
{
}

cop404l_cpu_device::cop404l_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP404L, tag, owner, clock, 11, 7, COP444L_FEATURE, 0xf, 0xf, 0xf, true, true, nullptr, ADDRESS_MAP_NAME(data_128b))
{
}

cop444l_cpu_device::cop444l_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP444L, tag, owner, clock, 11, 7, COP444L_FEATURE, 0xf, 0xf, 0xf, true, true, ADDRESS_MAP_NAME(program_2kb), ADDRESS_MAP_NAME(data_128b))
{
}

cop445l_cpu_device::cop445l_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP445L, tag, owner, clock, 11, 7, COP444L_FEATURE, 0x7, 0x3, 0, true, false, ADDRESS_MAP_NAME(program_2kb), ADDRESS_MAP_NAME(data_128b))
{
}

cop404c_cpu_device::cop404c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP404C, tag, owner, clock, 11, 7, COP424C_FEATURE, 0xf, 0xf, 0xf, true, true, nullptr, ADDRESS_MAP_NAME(data_128b))
{
}

cop424c_cpu_device::cop424c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP424C, tag, owner, clock, 10, 6, COP424C_FEATURE, 0xf, 0xf, 0xf, true, true, ADDRESS_MAP_NAME(program_1kb), ADDRESS_MAP_NAME(data_64b))
{
}

cop425c_cpu_device::cop425c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP425C, tag, owner, clock, 10, 6, COP424C_FEATURE, 0xf, 0xf, 0, true, false, ADDRESS_MAP_NAME(program_1kb), ADDRESS_MAP_NAME(data_64b))
{
}

cop426c_cpu_device::cop426c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP426C, tag, owner, clock, 10, 6, COP424C_FEATURE, 0xe, 0xe, 0xf, true, true, ADDRESS_MAP_NAME(program_1kb), ADDRESS_MAP_NAME(data_64b))
{
}

cop444c_cpu_device::cop444c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP444C, tag, owner, clock, 11, 7, COP424C_FEATURE, 0xf, 0xf, 0xf, true, true, ADDRESS_MAP_NAME(program_2kb), ADDRESS_MAP_NAME(data_128b))
{
}

cop445c_cpu_device::cop445c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP445C, tag, owner, clock, 11, 7, COP424C_FEATURE, 0xf, 0xf, 0, true, false, ADDRESS_MAP_NAME(program_2kb), ADDRESS_MAP_NAME(data_128b))
{
}

cop446c_cpu_device::cop446c_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cop400_cpu_device(mconfig, COP446C, tag, owner, clock, 11, 7, COP424C_FEATURE, 0xe, 0xe, 0xf, true, true, ADDRESS_MAP_NAME(program_2kb), ADDRESS_MAP_NAME(data_128b))
{
}

device_memory_interface::space_config_vector cop400_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_DATA,    &m_data_config)
	};
}



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/


void cop400_cpu_device::PUSH(uint16_t data)
{
	if (m_featuremask != COP410_FEATURE)
	{
		SC = SB;
	}

	SB = SA;
	SA = data;
}

void cop400_cpu_device::POP()
{
	PC = SA;
	SA = SB;

	if (m_featuremask != COP410_FEATURE)
	{
		SB = SC;
	}
}

void cop400_cpu_device::WRITE_Q(uint8_t data)
{
	Q = data;

	if (!m_has_microbus && BIT(EN, 2))
	{
		OUT_L(Q);
	}
}

void cop400_cpu_device::WRITE_G(uint8_t data)
{
	G = data;

	OUT_G(G);
}

/***************************************************************************
    OPCODE HANDLERS
***************************************************************************/

#define INSTRUCTION(mnemonic) void (cop400_cpu_device::mnemonic)(uint8_t opcode)
#define OP(mnemonic) &cop400_cpu_device::mnemonic

INSTRUCTION(illegal)
{
	logerror("COP400: PC = %03x, Illegal opcode = %02x\n", PC-1, ROM(PC-1));
}

#include "cop400op.hxx"

/***************************************************************************
    OPCODE TABLES
***************************************************************************/

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP410_OPCODE_23_MAP[256] =
{
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(xad)       ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)
};

void cop400_cpu_device::cop410_op23(uint8_t opcode)
{
	uint8_t opcode23 = fetch();

	(this->*COP410_OPCODE_23_MAP[opcode23])(opcode23);
}

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP410_OPCODE_33_MAP[256] =
{
	OP(illegal)     , OP(skgbz0)    , OP(illegal)   , OP(skgbz2)    , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(skgbz1)    , OP(illegal)   , OP(skgbz3)    , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(skgz)      , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(ing)       , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(inl)       , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(halt)        , OP(illegal)   , OP(omg)       , OP(illegal)   , OP(camq)      , OP(illegal)   , OP(obd)       , OP(illegal)   ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(lei)         , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       ,
	OP(lei)         , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)
};

void cop400_cpu_device::cop410_op33(uint8_t opcode)
{
	uint8_t opcode33 = fetch();

	(this->*COP410_OPCODE_33_MAP[opcode33])(opcode33);
}

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP410_OPCODE_MAP[256] =
{
	OP(clra)        , OP(skmbz0)    , OP(xor_)      , OP(skmbz2)        , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(illegal)     , OP(skmbz1)    , OP(illegal)   , OP(skmbz3)        , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(skc)         , OP(ske)       , OP(sc)        , OP(cop410_op23)   , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(asc)         , OP(add)       , OP(rc)        , OP(cop410_op33)   , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,

	OP(comp)        , OP(illegal)   , OP(rmb2)      , OP(rmb3)          , OP(nop)       , OP(rmb1)      , OP(smb2)      , OP(smb1)      ,
	OP(ret)         , OP(retsk)     , OP(illegal)   , OP(smb3)          , OP(rmb0)      , OP(smb0)      , OP(cba)       , OP(xas)       ,
	OP(cab)         , OP(aisc)      , OP(aisc)      , OP(aisc)          , OP(aisc)      , OP(aisc)      , OP(aisc)      , OP(aisc)      ,
	OP(aisc)        , OP(aisc)      , OP(aisc)      , OP(aisc)          , OP(aisc)      , OP(aisc)      , OP(aisc)      , OP(aisc)      ,
	OP(jmp)         , OP(jmp)       , OP(illegal)   , OP(illegal)       , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(jsr)         , OP(jsr)       , OP(illegal)   , OP(illegal)       , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(stii)        , OP(stii)      , OP(stii)      , OP(stii)          , OP(stii)      , OP(stii)      , OP(stii)      , OP(stii)      ,
	OP(stii)        , OP(stii)      , OP(stii)      , OP(stii)          , OP(stii)      , OP(stii)      , OP(stii)      , OP(stii)      ,

	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(lqid)      ,

	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jid)
};

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP420_OPCODE_23_MAP[256] =
{
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,

	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)
};

void cop400_cpu_device::cop420_op23(uint8_t opcode)
{
	uint8_t opcode23 = fetch();

	(this->*COP420_OPCODE_23_MAP[opcode23])(opcode23);
}

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP420_OPCODE_33_MAP[256] =
{
	OP(illegal)     , OP(skgbz0)    , OP(illegal)   , OP(skgbz2)    , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(skgbz1)    , OP(illegal)   , OP(skgbz3)    , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(skgz)      , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(inin)        , OP(inil)      , OP(ing)       , OP(illegal)   , OP(cqma)      , OP(illegal)   , OP(inl)       , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(omg)       , OP(illegal)   , OP(camq)      , OP(illegal)   , OP(obd)       , OP(illegal)   ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(ogi)         , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       ,
	OP(ogi)         , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       ,
	OP(lei)         , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       ,
	OP(lei)         , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,

	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)
};

void cop400_cpu_device::cop420_op33(uint8_t opcode)
{
	uint8_t opcode33 = fetch();

	(this->*COP420_OPCODE_33_MAP[opcode33])(opcode33);
}

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP420_OPCODE_MAP[256] =
{
	OP(clra)        , OP(skmbz0)    , OP(xor_)      , OP(skmbz2)        , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(casc)        , OP(skmbz1)    , OP(xabr)      , OP(skmbz3)        , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(skc)         , OP(ske)       , OP(sc)        , OP(cop420_op23)   , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(asc)         , OP(add)       , OP(rc)        , OP(cop420_op33)   , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,

	OP(comp)        , OP(skt)       , OP(rmb2)      , OP(rmb3)          , OP(nop)       , OP(rmb1)      , OP(smb2)      , OP(smb1)      ,
	OP(cop420_ret)  , OP(retsk)     , OP(adt)       , OP(smb3)          , OP(rmb0)      , OP(smb0)      , OP(cba)       , OP(xas)       ,
	OP(cab)         , OP(aisc)      , OP(aisc)      , OP(aisc)          , OP(aisc)      , OP(aisc)      , OP(aisc)      , OP(aisc)      ,
	OP(aisc)        , OP(aisc)      , OP(aisc)      , OP(aisc)          , OP(aisc)      , OP(aisc)      , OP(aisc)      , OP(aisc)      ,
	OP(jmp)         , OP(jmp)       , OP(jmp)       , OP(jmp)           , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(jsr)         , OP(jsr)       , OP(jsr)       , OP(jsr)           , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(stii)        , OP(stii)      , OP(stii)      , OP(stii)          , OP(stii)      , OP(stii)      , OP(stii)      , OP(stii)      ,
	OP(stii)        , OP(stii)      , OP(stii)      , OP(stii)          , OP(stii)      , OP(stii)      , OP(stii)      , OP(stii)      ,

	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(lqid)      ,

	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jid)
};

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP444L_OPCODE_23_MAP[256] =
{
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,

	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,

	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,

	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)
};

void cop400_cpu_device::cop444l_op23(uint8_t opcode)
{
	uint8_t opcode23 = fetch();

	(this->*COP444L_OPCODE_23_MAP[opcode23])(opcode23);
}

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP444L_OPCODE_33_MAP[256] =
{
	OP(illegal)     , OP(skgbz0)    , OP(illegal)   , OP(skgbz2)    , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(skgbz1)    , OP(illegal)   , OP(skgbz3)    , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(skgz)      , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(inin)        , OP(inil)      , OP(ing)       , OP(illegal)   , OP(cqma)      , OP(illegal)   , OP(inl)       , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(omg)       , OP(illegal)   , OP(camq)      , OP(illegal)   , OP(obd)       , OP(illegal)   ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(ogi)         , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       ,
	OP(ogi)         , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       ,
	OP(lei)         , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       ,
	OP(lei)         , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,

	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,

	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)
};

void cop400_cpu_device::cop444l_op33(uint8_t opcode)
{
	uint8_t opcode33 = fetch();

	(this->*COP444L_OPCODE_33_MAP[opcode33])(opcode33);
}

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP444L_OPCODE_MAP[256] =
{
	OP(clra)        , OP(skmbz0)    , OP(xor_)      , OP(skmbz2)        , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(casc)        , OP(skmbz1)    , OP(cop444l_xabr), OP(skmbz3)      , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(skc)         , OP(ske)       , OP(sc)        , OP(cop444l_op23)  , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(asc)         , OP(add)       , OP(rc)        , OP(cop444l_op33)  , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,

	OP(comp)        , OP(skt)       , OP(rmb2)      , OP(rmb3)          , OP(nop)       , OP(rmb1)      , OP(smb2)      , OP(smb1)      ,
	OP(cop420_ret)  , OP(retsk)     , OP(adt)       , OP(smb3)          , OP(rmb0)      , OP(smb0)      , OP(cba)       , OP(xas)       ,
	OP(cab)         , OP(aisc)      , OP(aisc)      , OP(aisc)          , OP(aisc)      , OP(aisc)      , OP(aisc)      , OP(aisc)      ,
	OP(aisc)        , OP(aisc)      , OP(aisc)      , OP(aisc)          , OP(aisc)      , OP(aisc)      , OP(aisc)      , OP(aisc)      ,
	OP(jmp)         , OP(jmp)       , OP(jmp)       , OP(jmp)           , OP(jmp)       , OP(jmp)       , OP(jmp)       , OP(jmp)       ,
	OP(jsr)         , OP(jsr)       , OP(jsr)       , OP(jsr)           , OP(jsr)       , OP(jsr)       , OP(jsr)       , OP(jsr)       ,
	OP(stii)        , OP(stii)      , OP(stii)      , OP(stii)          , OP(stii)      , OP(stii)      , OP(stii)      , OP(stii)      ,
	OP(stii)        , OP(stii)      , OP(stii)      , OP(stii)          , OP(stii)      , OP(stii)      , OP(stii)      , OP(stii)      ,

	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(lqid)      ,

	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jid)
};

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP424C_OPCODE_23_MAP[256] =
{
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,

	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,
	OP(ldd)         , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       , OP(ldd)       ,

	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,

	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       ,
	OP(xad)         , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)       , OP(xad)
};

void cop400_cpu_device::cop424c_op23(uint8_t opcode)
{
	uint8_t opcode23 = fetch();

	(this->*COP424C_OPCODE_23_MAP[opcode23])(opcode23);
}

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP424C_OPCODE_33_MAP[256] =
{
	OP(illegal)     , OP(skgbz0)    , OP(illegal)   , OP(skgbz2)    , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(skgbz1)    , OP(illegal)   , OP(skgbz3)    , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(skgz)      , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(inin)        , OP(inil)      , OP(ing)       , OP(illegal)   , OP(cqma)      , OP(illegal)   , OP(inl)       , OP(ctma)      ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(it)          , OP(illegal)   , OP(omg)       , OP(illegal)   , OP(camq)      , OP(illegal)   , OP(obd)       , OP(camt)      ,

	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(ogi)         , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       ,
	OP(ogi)         , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       , OP(ogi)       ,
	OP(lei)         , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       ,
	OP(lei)         , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       , OP(lei)       ,
	OP(illegal)     , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,
	OP(halt)        , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   , OP(illegal)   ,

	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,

	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)
};

void cop400_cpu_device::cop424c_op33(uint8_t opcode)
{
	uint8_t opcode33 = fetch();

	(this->*COP424C_OPCODE_33_MAP[opcode33])(opcode33);
}

const cop400_cpu_device::cop400_opcode_func cop400_cpu_device::COP424C_OPCODE_MAP[256] =
{
	OP(clra)        , OP(skmbz0)    , OP(xor_)      , OP(skmbz2)        , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(casc)        , OP(skmbz1)    , OP(cop444l_xabr), OP(skmbz3)      , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(skc)         , OP(ske)       , OP(sc)        , OP(cop444l_op23)  , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,
	OP(asc)         , OP(add)       , OP(rc)        , OP(cop444l_op33)  , OP(xis)       , OP(ld)        , OP(x)         , OP(xds)       ,
	OP(lbi)         , OP(lbi)       , OP(lbi)       , OP(lbi)           , OP(lbi)       , OP(lbi)       , OP(lbi)       , OP(lbi)       ,

	OP(comp)        , OP(skt)       , OP(rmb2)      , OP(rmb3)          , OP(nop)       , OP(rmb1)      , OP(smb2)      , OP(smb1)      ,
	OP(cop420_ret)  , OP(retsk)     , OP(adt)       , OP(smb3)          , OP(rmb0)      , OP(smb0)      , OP(cba)       , OP(xas)       ,
	OP(cab)         , OP(aisc)      , OP(aisc)      , OP(aisc)          , OP(aisc)      , OP(aisc)      , OP(aisc)      , OP(aisc)      ,
	OP(aisc)        , OP(aisc)      , OP(aisc)      , OP(aisc)          , OP(aisc)      , OP(aisc)      , OP(aisc)      , OP(aisc)      ,
	OP(jmp)         , OP(jmp)       , OP(jmp)       , OP(jmp)           , OP(jmp)       , OP(jmp)       , OP(jmp)       , OP(jmp)       ,
	OP(jsr)         , OP(jsr)       , OP(jsr)       , OP(jsr)           , OP(jsr)       , OP(jsr)       , OP(jsr)       , OP(jsr)       ,
	OP(stii)        , OP(stii)      , OP(stii)      , OP(stii)          , OP(stii)      , OP(stii)      , OP(stii)      , OP(stii)      ,
	OP(stii)        , OP(stii)      , OP(stii)      , OP(stii)          , OP(stii)      , OP(stii)      , OP(stii)      , OP(stii)      ,

	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(lqid)      ,

	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jp)        ,
	OP(jp)          , OP(jp)        , OP(jp)        , OP(jp)            , OP(jp)        , OP(jp)        , OP(jp)        , OP(jid)
};

/***************************************************************************
    TIMER CALLBACKS
***************************************************************************/

void cop400_cpu_device::serial_tick()
{
	if (BIT(EN, 0))
	{
		/*

		    SIO is an asynchronous binary counter decrementing its value by one upon each low-going pulse ("1" to "0") occurring on the SI input.
		    Each pulse must remain at each logic level at least two instruction cycles. SK outputs the value of the C upon the execution of an XAS
		    and remains latched until the execution of another XAS instruction. The SO output is equal to the value of EN3.

		*/

		// serial output

		OUT_SO(BIT(EN, 3));

		// serial clock

		OUT_SK(SKL);

		// serial input

		m_si <<= 1;
		m_si = (m_si & 0x0e) | IN_SI();

		if ((m_si & 0x0f) == 0x0c) // 1100
		{
			SIO--;
			SIO &= 0x0f;
		}
	}
	else
	{
		/*

		    SIO is a serial shift register, shifting continuously left each instruction cycle time. The data present at SI goes into the least
		    significant bit of SIO: SO can be enabled to output the most significant bit of SIO each cycle time. SK output becomes a logic-
		    controlled clock, providing a SYNC signal each instruction time. It will start outputting a SYNC pulse upon the execution of an XAS
		    instruction with C = "1," stopping upon the execution of a subsequent XAS with C = "0".

		    If EN0 is changed from "1" to "0" ("0" to "1") the SK output will change from "1" to SYNC (SYNC to "1") without the execution of
		    an XAS instruction.

		*/

		// serial output

		if (BIT(EN, 3))
		{
			OUT_SO(BIT(SIO, 3));
		}
		else
		{
			OUT_SO(0);
		}

		// serial clock

		if (SKL)
		{
			OUT_SK(1); // SYNC
		}
		else
		{
			OUT_SK(0);
		}

		// serial input

		SIO = ((SIO << 1) | IN_SI()) & 0x0f;
	}
}

void cop400_cpu_device::counter_tick()
{
	T++;

	if (!T) {
		m_skt_latch = 1;
		m_idle = false;
	}
}

void cop400_cpu_device::inil_tick()
{
	uint8_t in;
	int i;

	in = IN_IN();

	for (i = 0; i < 4; i++)
	{
		m_in[i] = (m_in[i] << 1) | BIT(in, i);

		if ((m_in[i] & 0x07) == 0x04) // 100
		{
			IL |= (1 << i);
		}
	}
}

/***************************************************************************
    INITIALIZATION
***************************************************************************/

void cop400_cpu_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_SERIAL:
		serial_tick();
		break;

	case TIMER_COUNTER:
		counter_tick();
		break;

	case TIMER_INIL:
		inil_tick();
		break;
	}
}


void cop400_cpu_device::device_start()
{
	/* find address spaces */
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_data = &space(AS_DATA);

	/* find i/o handlers */
	m_read_l.resolve_safe(0);
	m_read_l_tristate.resolve_safe(0);
	m_write_l.resolve_safe();
	m_read_g.resolve_safe(0);
	m_write_g.resolve_safe();
	m_write_d.resolve_safe();
	m_read_in.resolve_safe(0);
	m_read_si.resolve_safe(0);
	m_write_so.resolve_safe();
	m_write_sk.resolve_safe();
	m_read_cko.resolve_safe(0);

	/* allocate serial timer */
	m_serial_timer = timer_alloc(TIMER_SERIAL);
	m_serial_timer->adjust(attotime::zero, 0, attotime::from_ticks(m_cki, clock()));

	/* allocate counter timer */
	m_counter_timer = nullptr;
	if (m_has_counter)
	{
		m_counter_timer = timer_alloc(TIMER_COUNTER);
		m_counter_timer->adjust(attotime::zero, 0, attotime::from_ticks(m_cki * 4, clock()));
	}

	/* allocate IN latch timer */
	m_inil_timer = nullptr;
	if (m_has_inil)
	{
		m_inil_timer = timer_alloc(TIMER_INIL);
		m_inil_timer->adjust(attotime::zero, 0, attotime::from_ticks(m_cki, clock()));
	}

	/* register for state saving */
	save_item(NAME(m_pc));
	save_item(NAME(m_prevpc));
	save_item(NAME(m_sa));
	save_item(NAME(m_sb));
	save_item(NAME(m_sc));
	save_item(NAME(m_a));
	save_item(NAME(m_b));
	save_item(NAME(m_c));
	save_item(NAME(m_g));
	save_item(NAME(m_q));
	save_item(NAME(m_en));
	save_item(NAME(m_sio));
	save_item(NAME(m_skl));
	save_item(NAME(m_t));
	save_item(NAME(m_skip));
	save_item(NAME(m_skip_lbi));
	save_item(NAME(m_skt_latch));
	save_item(NAME(m_si));
	save_item(NAME(m_last_skip));
	save_item(NAME(m_in));
	save_item(NAME(m_halt));
	save_item(NAME(m_idle));

	// setup debugger state display
	offs_t pc_mask = m_program->addrmask();

	state_add(STATE_GENPC, "GENPC", m_pc).mask(pc_mask).noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_prevpc).mask(pc_mask).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS", m_flags).mask(0x7).callimport().callexport().noshow().formatstr("%3s");
	state_add(COP400_PC, "PC", m_pc).mask(pc_mask);
	state_add(COP400_SA, "SA", m_sa).mask(pc_mask);
	state_add(COP400_SB, "SB", m_sb).mask(pc_mask);
	if (!(m_featuremask & COP410_FEATURE)) {
		state_add(COP400_SC, "SC", m_sc).mask(pc_mask);
	}
	state_add(COP400_B, "B", m_b);
	state_add(COP400_A, "A", m_a).mask(0xf);
	state_add(COP400_G, "G", m_g).mask(0xf);
	state_add(COP400_Q, "Q", m_q);
	state_add(COP400_SIO, "SIO", m_sio).mask(0xf);
	state_add(COP400_EN, "EN", m_en).mask(0xf);
	if (m_featuremask & COP424C_FEATURE) {
		state_add(COP400_T, "T", m_t);
	}

	m_icountptr = &m_icount;

	m_q = 0;
	m_sa = 0;
	m_sb = 0;
	m_sc = 0;
	m_sio = 0;
	m_flags = 0;
	m_il = 0;
	m_in[0] = m_in[1] = m_in[2] = m_in[3] = 0;
	m_si = 0;
	m_skip_lbi = 0;
	m_last_skip = false;
	m_skip = false;
}


/***************************************************************************
    RESET
***************************************************************************/

void cop400_cpu_device::device_reset()
{
	PC = 0;
	A = 0;
	B = 0;
	C = 0;
	OUT_D(0);
	EN = 0;
	WRITE_G(0);
	SKL = 1;

	T = 0;
	m_skt_latch = 1;

	m_halt = false;
	m_idle = false;
}

/***************************************************************************
    EXECUTION
***************************************************************************/

uint8_t cop400_cpu_device::fetch()
{
	m_icount--;

	return ROM(PC++);
}

void cop400_cpu_device::execute_run()
{
	do
	{
		if (!m_skip) {
			// debugger hook
			m_prevpc = PC;
			debugger_instruction_hook(this, PC);
		}

		// halt logic
		if (m_cko == COP400_CKO_HALT_IO_PORT) {
			m_halt = IN_CKO();
		}

		if (m_halt || m_idle) {
			m_icount--;
			continue;
		}

		// fetch opcode
		uint8_t opcode = fetch();
		cop400_opcode_func function = m_opcode_map[opcode];

		// check for interrupt
		if (BIT(EN, 1) && BIT(IL, 1)) {
			// all successive transfer of control instructions and successive LBIs have been completed
			if ((function != OP(jp)) && (function != OP(jmp)) && (function != OP(jsr)) && !m_skip_lbi) {
				// store skip logic
				m_last_skip = m_skip;
				m_skip = false;

				// push next PC
				PUSH(PC);

				// jump to interrupt service routine
				PC = 0x0ff;

				// disable interrupt
				EN &= ~0x02;
			}

			IL &= ~2;
		}

		if (m_skip) {
			// skip instruction
			if (m_InstLen[opcode] == 2) {
				// fetch second byte
				opcode = fetch();
			}

			m_skip = false;
			continue;
		}

		// execute instruction
		(this->*(function))(opcode);

		// LBI skip logic
		if (m_skip_lbi > 0) {
			m_skip_lbi--;
		}
	} while (m_icount > 0);
}



/***************************************************************************
    GENERAL CONTEXT ACCESS
***************************************************************************/

void cop400_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		m_skt_latch = BIT(m_flags, 2);
		m_c = BIT(m_flags, 1);
		m_skl = BIT(m_flags, 0);
		break;
	}
}

void cop400_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		m_flags = (m_skt_latch ? 0x04 : 0x00) | (m_c ? 0x02 : 0x00) | (m_skl ? 0x01 : 0x00);
		break;
	}
}

void cop400_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
	case STATE_GENFLAGS:
		str = string_format("%c%c%c",
				m_c ? 'C' : '.',
				m_skl ? 'S' : '.',
				m_skt_latch ? 'T' : '.');
		break;
	}
}


offs_t cop400_cpu_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE( cop410 );
	extern CPU_DISASSEMBLE( cop420 );
	extern CPU_DISASSEMBLE( cop444 );
	extern CPU_DISASSEMBLE( cop424 );

	if ( m_featuremask & COP424C_FEATURE )
	{
		return CPU_DISASSEMBLE_NAME(cop424)(this, stream, pc, oprom, opram, options);
	}

	if ( m_featuremask & COP444L_FEATURE )
	{
		return CPU_DISASSEMBLE_NAME(cop444)(this, stream, pc, oprom, opram, options);
	}

	if ( m_featuremask & COP420_FEATURE )
	{
		return CPU_DISASSEMBLE_NAME(cop420)(this, stream, pc, oprom, opram, options);
	}

	return CPU_DISASSEMBLE_NAME(cop410)(this, stream, pc, oprom, opram, options);
}

READ8_MEMBER( cop400_cpu_device::microbus_rd )
{
	if (LOG_MICROBUS) logerror("%s %s MICROBUS RD %02x\n", machine().time().as_string(), machine().describe_context(), Q);

	return Q;
}

WRITE8_MEMBER( cop400_cpu_device::microbus_wr )
{
	if (LOG_MICROBUS) logerror("%s %s MICROBUS WR %02x\n", machine().time().as_string(), machine().describe_context(), data);

	WRITE_G(G & 0xe);

	Q = data;
}

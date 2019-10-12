// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/*********************************************************************

    saa7191.cpp

    Philips SAA7191B Digital Multistandard Colour Decoder (DMSD)

    TODO:
    - Actual functionality

*********************************************************************/

#include "emu.h"
#include "saa7191.h"

#define LOG_UNKNOWN     (1 << 0)
#define LOG_READS       (1 << 1)
#define LOG_WRITES      (1 << 2)
#define LOG_ERRORS      (1 << 3)
#define LOG_I2C_IGNORES (1 << 4)
#define LOG_DEFAULT     (LOG_READS | LOG_WRITES | LOG_ERRORS | LOG_I2C_IGNORES | LOG_UNKNOWN)

#define VERBOSE         (LOG_DEFAULT)
#include "logmacro.h"

DEFINE_DEVICE_TYPE(SAA7191, saa7191_device, "saa7191", "Philips SAA7191 DMSD")

saa7191_device::saa7191_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, SAA7191, tag, owner, clock)
	, m_chr_in(*this)
	, m_cvbs_in(*this)
	, m_y_out(*this)
	, m_uv_out(*this)
	, m_hs_out(*this)
	, m_vs_out(*this)
{
}

void saa7191_device::device_start()
{
	save_item(NAME(m_status));
	save_item(NAME(m_regs));
	save_item(NAME(m_i2c_write_addr));
	save_item(NAME(m_i2c_read_addr));
	save_item(NAME(m_i2c_subaddr));
	save_item(NAME(m_i2c_state));

	m_chr_in.resolve_safe(0);
	m_cvbs_in.resolve_safe(0);

	m_y_out.resolve_safe();
	m_uv_out.resolve_safe();
	m_hs_out.resolve_safe();
	m_vs_out.resolve_safe();

	m_input_clock = timer_alloc(TIMER_INPUT_CLOCK);
	m_input_clock->adjust(attotime::never);
}

void saa7191_device::device_reset()
{
	m_status = 0;
	memset(m_regs, 0, sizeof(uint8_t) * REG_COUNT);

	m_i2c_write_addr = 0x8a;
	m_i2c_read_addr = 0x8b;
	m_i2c_subaddr = 0x00;
	m_i2c_state = I2C_STATE_IDLE;

	m_input_clock->adjust(attotime::never);
}

void saa7191_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
}

WRITE8_MEMBER(saa7191_device::i2c_data_w)
{
	switch (m_i2c_state)
	{
	case I2C_STATE_IDLE:
		if (data == m_i2c_write_addr)
			m_i2c_state = I2C_STATE_SUBADDR_WRITE;
		else if (data == m_i2c_read_addr)
			m_i2c_state = I2C_STATE_SUBADDR_READ;
		else
			LOGMASKED(LOG_I2C_IGNORES, "I2C idle, address %02x ignored (mine are R:%02x/W:%02x)\n", data, m_i2c_read_addr, m_i2c_write_addr);
		break;
	case I2C_STATE_SUBADDR_WRITE:
		m_i2c_subaddr = data;
		m_i2c_state = I2C_STATE_DATA_WRITE;
		break;
	case I2C_STATE_SUBADDR_READ:
		m_i2c_subaddr = data;
		m_i2c_state = I2C_STATE_DATA_READ;
		break;
	case I2C_STATE_DATA_WRITE:
		reg_w(space, m_i2c_subaddr, data);
		m_i2c_subaddr = (m_i2c_subaddr + 1) % REG_COUNT;
		break;
	case I2C_STATE_DATA_READ:
		LOGMASKED(LOG_ERRORS, "I2C is expecting a data read, but data was written, returning to idle\n");
		m_i2c_state = I2C_STATE_IDLE;
		break;
	default:
		LOGMASKED(LOG_ERRORS, "Unknown I2C state %d, returning to idle\n", m_i2c_state);
		m_i2c_state = I2C_STATE_IDLE;
		break;
	}
}

READ8_MEMBER(saa7191_device::i2c_data_r)
{
	if (m_i2c_state != I2C_STATE_DATA_READ)
	{
		LOGMASKED(LOG_ERRORS, "i2c_data_r called, but we are in state %d and not expecting a data read, returning to idle\n", m_i2c_state);
		m_i2c_state = I2C_STATE_IDLE;
		return 0;
	}
	if (m_i2c_subaddr == 0x01)
	{
		LOGMASKED(LOG_READS, "i2c_data_r: Status = %02x\n", m_status);
		m_i2c_subaddr = (m_i2c_subaddr + 1) % REG_COUNT;
		return m_status;
	}
	LOGMASKED(LOG_UNKNOWN, "i2c_data_r: Unknown Sub-Address %02x, returning 0\n", m_i2c_subaddr);
	m_i2c_subaddr = (m_i2c_subaddr + 1) % REG_COUNT;
	return 0;
}

WRITE_LINE_MEMBER(saa7191_device::i2c_stop_w)
{
	if (state)
		m_i2c_state = I2C_STATE_IDLE;
}

WRITE_LINE_MEMBER(saa7191_device::iicsa_w)
{
	m_i2c_write_addr = state ? 0x8e : 0x8a;
	m_i2c_read_addr = m_i2c_write_addr | 1;
}

WRITE8_MEMBER(saa7191_device::reg_w)
{
	if (m_i2c_subaddr < REG_COUNT)
	{
		m_regs[m_i2c_subaddr] = data;
	}

	switch (m_i2c_subaddr)
	{
	case REG_IDEL:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Increment delay = %02x\n", data);
		break;
	case REG_HSYB:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H sync begin, 50Hz = %02x\n", data);
		break;
	case REG_HSYS:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H sync stop, 50Hz = %02x\n", data);
		break;
	case REG_HCLB:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H clamp begin, 50Hz = %02x\n", data);
		break;
	case REG_HCLS:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H clamp stop, 50Hz = %02x\n", data);
		break;
	case REG_HPHI:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H sync after PHI1, 50Hz = %02x\n", data);
		break;
	case REG_LUMC:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Luminance control = %02x\n", data);
		break;
	case REG_HUEC:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Hue control = %02x\n", data);
		break;
	case REG_CKTQ:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Colour killer threshold QAM = %02x\n", data);
		break;
	case REG_CKTS:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Colour killer threshold SECAM = %02x\n", data);
		break;
	case REG_PLSE:
		LOGMASKED(LOG_WRITES, "i2c_data_w: PAL switch sensitivity = %02x\n", data);
		break;
	case REG_SESE:
		LOGMASKED(LOG_WRITES, "i2c_data_w: SECAM switch sensitivity = %02x\n", data);
		break;
	case REG_GAIN:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Chroma gain control settings = %02x\n", data);
		break;
	case REG_STDC:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Standard/mode control = %02x\n", data);
		break;
	case REG_IOCK:
		LOGMASKED(LOG_WRITES, "i2c_data_w: I/O and clock control = %02x\n", data);
		break;
	case REG_CTL1:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Control #1 = %02x\n", data);
		break;
	case REG_CTL2:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Control #2 = %02x\n", data);
		break;
	case REG_CHCV:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Chroma gain reference = %02x\n", data);
		break;
	case REG_HS6B:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H sync begin, 60Hz = %02x\n", data);
		break;
	case REG_HS6S:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H sync stop, 60Hz = %02x\n", data);
		break;
	case REG_HC6B:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H clamp begin, 60Hz = %02x\n", data);
		break;
	case REG_HC6S:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H clamp stop, 60Hz = %02x\n", data);
		break;
	case REG_HP6I:
		LOGMASKED(LOG_WRITES, "i2c_data_w: H sync after PHI1, 60Hz = %02x\n", data);
		break;
	default:
		LOGMASKED(LOG_WRITES, "i2c_data_w: Unknown Register %02x = %02x (ignored)\n", m_i2c_subaddr, data);
		if (m_i2c_subaddr < REG_COUNT)
		{
			m_regs[m_i2c_subaddr] = 0;
		}
		break;
	}
}


// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Motorola M68HC16Z series modular microcontrollers

    Most microcontrollers in this series include the following modules:
    • Central Processing Unit (CPU16)
    • System Integration Module (SIM)
    • Standby RAM (SRAM)
    • Analog-to-Digital Converter (ADC)
    • Queued Serial Module (QSM)
    • General-Purpose Timer (GPT)

    MC68HC11Z4/MC68CK11Z4 uses special low-power versions of the CPU and
    SIM, and replaces the QSM with a simpler SCI/SPI combination (MCCI).

    SRAM size is 1, 2 or 4 KB, depending on the model. 8 KB of mask ROM is
    also provided on MC68HC11Z2 and MC68HC11Z3.

    Package types are 132-pin PFQP and 144-pin TQFP.

***************************************************************************/

#include "emu.h"
#include "m68hc16z.h"


// device type definition
DEFINE_DEVICE_TYPE(MC68HC16Z1, mc68hc16z1_device, "mc68hc16z1", "Motorola MC68HC16Z1")

m68hc16z_device::m68hc16z_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, address_map_constructor map)
	: cpu16_device(mconfig, type, tag, owner, clock, map)
	, m_pitr(0)
	, m_sccr{0, 0}
	, m_spcr{0, 0, 0, 0}
	, m_tmsk1(0)
{
}

mc68hc16z1_device::mc68hc16z1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m68hc16z_device(mconfig, MC68HC16Z1, tag, owner, clock, address_map_constructor(FUNC(mc68hc16z1_device::internal_map), this))
{
}

void m68hc16z_device::device_start()
{
	cpu16_device::device_start();

	save_item(NAME(m_pitr));
	save_item(NAME(m_sccr));
	save_item(NAME(m_spcr));
	save_item(NAME(m_tmsk1));
}

void m68hc16z_device::device_reset()
{
	cpu16_device::device_reset();

	m_pitr = 0;
	m_sccr[0] = 0x0004;
	m_sccr[1] = 0;
	m_spcr[0] = 0x0104;
	m_spcr[1] = 0x0404;
	m_spcr[2] = 0;
	m_spcr[3] = 0;
	m_tmsk1 = 0;
}


void m68hc16z_device::sim_map(address_map &map)
{
	map(0xffa00, 0xffa01).w(FUNC(m68hc16z_device::simcr_w));
	map(0xffa04, 0xffa05).w(FUNC(m68hc16z_device::syncr_w));
	map(0xffa07, 0xffa07).r(FUNC(m68hc16z_device::rsr_r));
	map(0xffa11, 0xffa11).mirror(2).w(FUNC(m68hc16z_device::porte_w));
	map(0xffa15, 0xffa15).w(FUNC(m68hc16z_device::ddre_w));
	map(0xffa17, 0xffa17).w(FUNC(m68hc16z_device::pepar_w));
	map(0xffa19, 0xffa19).mirror(2).w(FUNC(m68hc16z_device::portf_w));
	map(0xffa1d, 0xffa1d).w(FUNC(m68hc16z_device::ddrf_w));
	map(0xffa1f, 0xffa1f).w(FUNC(m68hc16z_device::pfpar_w));
	map(0xffa21, 0xffa21).w(FUNC(m68hc16z_device::sypcr_w));
	map(0xffa22, 0xffa23).w(FUNC(m68hc16z_device::picr_w));
	map(0xffa24, 0xffa25).rw(FUNC(m68hc16z_device::pitr_r), FUNC(m68hc16z_device::pitr_w));
	map(0xffa27, 0xffa27).w(FUNC(m68hc16z_device::swsr_w));
	map(0xffa44, 0xffa47).w(FUNC(m68hc16z_device::cspar_w));
	map(0xffa48, 0xffa77).w(FUNC(m68hc16z_device::csbar_csor_w));
}

void m68hc16z_device::sram_map(address_map &map)
{
	map(0xffb00, 0xffb01).w(FUNC(m68hc16z_device::rammcr_w));
	map(0xffb04, 0xffb07).w(FUNC(m68hc16z_device::ramba_w));
}

void m68hc16z_device::adc_map(address_map &map)
{
	map(0xff700, 0xff701).w(FUNC(m68hc16z_device::adcmcr_w));
	map(0xff70b, 0xff70b).w(FUNC(m68hc16z_device::adctl0_w));
	map(0xff70d, 0xff70d).w(FUNC(m68hc16z_device::adctl1_w));
	map(0xff70e, 0xff70f).r(FUNC(m68hc16z_device::adcstat_r));
	map(0xff710, 0xff71f).r(FUNC(m68hc16z_device::rjurr_r));
	map(0xff720, 0xff72f).r(FUNC(m68hc16z_device::ljsrr_r));
	map(0xff730, 0xff73f).r(FUNC(m68hc16z_device::ljurr_r));
}

void m68hc16z_device::qsm_map(address_map &map)
{
	map(0xffc00, 0xffc01).w(FUNC(m68hc16z_device::qsmcr_w));
	map(0xffc04, 0xffc04).w(FUNC(m68hc16z_device::qsilr_w));
	map(0xffc05, 0xffc05).w(FUNC(m68hc16z_device::qsivr_w));
	map(0xffc08, 0xffc0b).rw(FUNC(m68hc16z_device::sccr_r), FUNC(m68hc16z_device::sccr_w));
	map(0xffc0c, 0xffc0d).r(FUNC(m68hc16z_device::scsr_r));
	map(0xffc0e, 0xffc0f).w(FUNC(m68hc16z_device::scdr_w));
	map(0xffc15, 0xffc15).w(FUNC(m68hc16z_device::portqs_w));
	map(0xffc16, 0xffc16).w(FUNC(m68hc16z_device::pqspar_w));
	map(0xffc17, 0xffc17).w(FUNC(m68hc16z_device::ddrqs_w));
	map(0xffc18, 0xffc1f).rw(FUNC(m68hc16z_device::spcr_r), FUNC(m68hc16z_device::spcr_w));
	map(0xffd00, 0xffd4f).ram(); // Receive RAM, Transmit RAM, Command RAM
}

void m68hc16z_device::gpt_map(address_map &map)
{
	map(0xff900, 0xff901).w(FUNC(m68hc16z_device::gptmcr_w));
	map(0xff904, 0xff905).w(FUNC(m68hc16z_device::gpticr_w));
	map(0xff906, 0xff906).w(FUNC(m68hc16z_device::ddrgp_w));
	map(0xff907, 0xff907).w(FUNC(m68hc16z_device::portgp_w));
	map(0xff908, 0xff908).w(FUNC(m68hc16z_device::oc1m_w));
	map(0xff90a, 0xff90b).r(FUNC(m68hc16z_device::tcnt_r));
	map(0xff90c, 0xff90c).w(FUNC(m68hc16z_device::pactl_w));
	map(0xff914, 0xff91d).w(FUNC(m68hc16z_device::toc_w));
	map(0xff91e, 0xff91e).w(FUNC(m68hc16z_device::tctl1_w));
	map(0xff91f, 0xff91f).w(FUNC(m68hc16z_device::tctl2_w));
	map(0xff920, 0xff920).rw(FUNC(m68hc16z_device::tmsk1_r), FUNC(m68hc16z_device::tmsk1_w));
	map(0xff921, 0xff921).w(FUNC(m68hc16z_device::tmsk2_w));
	map(0xff922, 0xff922).w(FUNC(m68hc16z_device::tflg1_w));
	map(0xff923, 0xff923).w(FUNC(m68hc16z_device::tflg2_w));
}

void mc68hc16z1_device::internal_map(address_map &map)
{
	map(0xfe000, 0xfe3ff).ram(); // FIXME: SRAM can be relocated by setting RAMBAH:RAMBAL
	sim_map(map);
	sram_map(map);
	adc_map(map);
	qsm_map(map);
	gpt_map(map);
}


void m68hc16z_device::simcr_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: SIMCR = $%04X\n", machine().describe_context(), data);
}

void m68hc16z_device::syncr_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: SYNCR = $%04X\n", machine().describe_context(), data);
}

u8 m68hc16z_device::rsr_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: Read from RSR\n", machine().describe_context());
	return 0;
}

void m68hc16z_device::porte_w(u8 data)
{
	logerror("%s: PORTE = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::ddre_w(u8 data)
{
	logerror("%s: DDRE = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::pepar_w(u8 data)
{
	logerror("%s: PEPAR = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::portf_w(u8 data)
{
	logerror("%s: PORTF = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::ddrf_w(u8 data)
{
	logerror("%s: DDRF = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::pfpar_w(u8 data)
{
	logerror("%s: PFPAR = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::sypcr_w(u8 data)
{
	logerror("%s: SYPCR = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::picr_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: PICR = $%04X\n", machine().describe_context(), data);
}

u16 m68hc16z_device::pitr_r()
{
	return m_pitr;
}

void m68hc16z_device::pitr_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 old_pitr = std::exchange(m_pitr, (m_pitr & ~mem_mask) | (data & mem_mask & 0x01ff));
	if (m_pitr != old_pitr)
		logerror("%s: PITR = $%04X\n", machine().describe_context(), m_pitr);
}

void m68hc16z_device::swsr_w(u8 data)
{
	if (data != 0x55 && data != 0xaa)
		logerror("%s: SWSR = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::cspar_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: CSPAR%d = $%04X\n", machine().describe_context(), offset, data);
}

void m68hc16z_device::csbar_csor_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (offset <= 1)
		logerror("%s: CS%sRBT = $%04X\n", machine().describe_context(), BIT(offset, 0) ? "BA" : "O", data);
	else
		logerror("%s: CS%sR%d = $%04X\n", machine().describe_context(), BIT(offset, 0) ? "BA" : "O", (offset >> 1) - 1, data);
}


void m68hc16z_device::rammcr_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: RAMMCR = $%04X\n", machine().describe_context(), data);
}

void m68hc16z_device::ramba_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: RAMBA%c = $%04X\n", machine().describe_context(), offset ? 'L' : 'H', data);
}


void m68hc16z_device::adcmcr_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: ADCMCR = $%04X\n", machine().describe_context(), data);
}

void m68hc16z_device::adctl0_w(u8 data)
{
	logerror("%s: ADCTL0 = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::adctl1_w(u8 data)
{
	logerror("%s: ADCTL1 = $%02X\n", machine().describe_context(), data);
}

u16 m68hc16z_device::adcstat_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: Read from ADCSTAT\n", machine().describe_context());
	return 0;
}

u16 m68hc16z_device::rjurr_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		logerror("%s: Read from RJURR%d\n", machine().describe_context(), offset);
	return 0;
}

u16 m68hc16z_device::ljsrr_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		logerror("%s: Read from LJSRR%d\n", machine().describe_context(), offset);
	return 0;
}

u16 m68hc16z_device::ljurr_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
		logerror("%s: Read from LJURR%d\n", machine().describe_context(), offset);
	return 0;
}


void m68hc16z_device::qsmcr_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: QSMCR = $%04X\n", machine().describe_context(), data);
}

void m68hc16z_device::qsilr_w(u8 data)
{
	logerror("%s: QSILR = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::qsivr_w(u8 data)
{
	logerror("%s: QSIVR = $%02X\n", machine().describe_context(), data);
}

u16 m68hc16z_device::sccr_r(offs_t offset)
{
	return m_sccr[offset];
}

void m68hc16z_device::sccr_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 old_sccr = std::exchange(m_sccr[offset], (m_sccr[offset] & ~mem_mask) | (data & mem_mask));
	if (m_sccr[offset] != old_sccr)
		logerror("%s: SCCR%d = $%04X\n", machine().describe_context(), offset, m_sccr[offset]);
}

u16 m68hc16z_device::scsr_r()
{
	return 0x0180;
}

void m68hc16z_device::scdr_w(u16 data)
{
	if (BIT(m_sccr[1], 9))
		logerror("%s: SCDR = $%03X\n", machine().describe_context(), data & 0x01ff);
	else
	{
		data &= 0x00ff;
		logerror("%s: SCDR = $%02X%s\n", machine().describe_context(), data & 0x00ff,
				data >= 0x20 && data < 0x7f ? util::string_format(" ('%c')", data) : std::string());
	}
}

void m68hc16z_device::portqs_w(u8 data)
{
	logerror("%s: PORTQS = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::pqspar_w(u8 data)
{
	logerror("%s: PQSPAR = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::ddrqs_w(u8 data)
{
	logerror("%s: DDRQS = $%02X\n", machine().describe_context(), data);
}

u16 m68hc16z_device::spcr_r(offs_t offset)
{
	return m_spcr[offset];
}

void m68hc16z_device::spcr_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 old_spcr = std::exchange(m_spcr[offset], (m_spcr[offset] & ~mem_mask) | (data & mem_mask));
	if (m_spcr[offset] != old_spcr)
		logerror("%s: SPCR%d = $%04X\n", machine().describe_context(), offset, m_spcr[offset]);

	// HACK: always reset SPE
	if (offset == 1)
		m_spcr[offset] &= 0x7fff;
}


void m68hc16z_device::gptmcr_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: GPTMCR = $%04X\n", machine().describe_context(), data);
}

void m68hc16z_device::gpticr_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: GPTICR = $%04X\n", machine().describe_context(), data);
}

void m68hc16z_device::ddrgp_w(u8 data)
{
	logerror("%s: DDRGP = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::portgp_w(u8 data)
{
	logerror("%s: PORTGP = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::oc1m_w(u8 data)
{
	logerror("%s: OC1M = $%02X\n", machine().describe_context(), data);
}

u16 m68hc16z_device::tcnt_r()
{
	if (!machine().side_effects_disabled())
		logerror("%s: TCNT read\n", machine().describe_context());
	return 0;
}

void m68hc16z_device::pactl_w(u8 data)
{
	logerror("%s: PACTL = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::toc_w(offs_t offset, u16 data, u16 mem_mask)
{
	logerror("%s: TOC%d = $%04X\n", machine().describe_context(), offset + 1, data);
}

void m68hc16z_device::tctl1_w(u8 data)
{
	logerror("%s: TCTL1 = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::tctl2_w(u8 data)
{
	logerror("%s: TCTL2 = $%02X\n", machine().describe_context(), data);
}

u8 m68hc16z_device::tmsk1_r()
{
	return m_tmsk1;
}

void m68hc16z_device::tmsk1_w(u8 data)
{
	if (m_tmsk1 != data)
	{
		m_tmsk1 = data;
		//logerror("%s: TMSK1 = $%02X\n", machine().describe_context(), data);
	}
}

void m68hc16z_device::tmsk2_w(u8 data)
{
	logerror("%s: TMSK2 = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::tflg1_w(u8 data)
{
	logerror("%s: TFLG1 = $%02X\n", machine().describe_context(), data);
}

void m68hc16z_device::tflg2_w(u8 data)
{
	logerror("%s: TFLG2 = $%02X\n", machine().describe_context(), data);
}

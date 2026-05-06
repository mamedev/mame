// license:BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

Winbond W83877TF

**************************************************************************************************/


#include "emu.h"
#include "w83877tf.h"

#include "formats/naslite_dsk.h"

#include <algorithm>

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"

DEFINE_DEVICE_TYPE(W83877TF, w83877tf_device, "w83877tf", "Winbond W83877TF Super I/O")

w83877tf_device::w83877tf_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, W83877TF, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, device_memory_interface(mconfig, *this)
	, m_space_config("superio_config_regs", ENDIANNESS_LITTLE, 8, 8, 0, address_map_constructor(FUNC(w83877tf_device::config_map), this))
	, m_fdc(*this, "fdc")
	, m_com(*this, "com%d", 1U)
	, m_lpt(*this, "lpt")
	, m_irq1_callback(*this)
	, m_irq8_callback(*this)
	, m_irq9_callback(*this)
	, m_txd1_callback(*this)
	, m_ndtr1_callback(*this)
	, m_nrts1_callback(*this)
	, m_txd2_callback(*this)
	, m_ndtr2_callback(*this)
	, m_nrts2_callback(*this)
	, m_index(0)
{
}

w83877tf_device::~w83877tf_device()
{
}

void w83877tf_device::device_start()
{
	set_isa_device();
	m_isa->set_dma_channel(0, this, true);
	m_isa->set_dma_channel(1, this, true);
	m_isa->set_dma_channel(2, this, true);
	m_isa->set_dma_channel(3, this, true);

	save_item(NAME(m_ipd));
	save_item(NAME(m_pnpcvs));
	save_item(NAME(m_clkinsel));

	save_item(NAME(m_fdc_ad));
	save_item(NAME(m_fdc_iqs));
	save_item(NAME(m_fdc_dqs));
	save_item(NAME(m_fdctri));
	save_item(NAME(m_fdcpwd));
	save_item(NAME(m_fipurdwm));
	save_item(NAME(m_sel4fdd));
	save_item(NAME(m_fddmode));
	save_item(NAME(m_floppy_boot));
	save_item(NAME(m_floppy_mediaid));
	save_item(NAME(m_swwp));
	save_item(NAME(m_disfddwr));
	save_item(NAME(m_en3mode));
	save_item(NAME(m_invertz));
	save_item(NAME(m_fdd_mode));
	save_item(NAME(m_abchg));

	save_item(NAME(m_prt_ad));
	save_item(NAME(m_prt_iqs));
	save_item(NAME(m_prt_dqs));
	save_item(NAME(m_prtmods));
	save_item(NAME(m_prtpwd));
	save_item(NAME(m_ecpfthr));
	save_item(NAME(m_eppver));
	save_item(NAME(m_prttri));

	save_item(NAME(m_uart_ad));
	save_item(NAME(m_uart_iqs));
	save_item(NAME(m_irqin_iqs));
	save_item(NAME(m_suamidi));
	save_item(NAME(m_submidi));
	save_item(NAME(m_uratri));
	save_item(NAME(m_urbtri));
	save_item(NAME(m_urapwd));
	save_item(NAME(m_urbpwd));
	save_item(NAME(m_rxw4c));
	save_item(NAME(m_txw4c));
	save_item(NAME(m_urirsel));
	save_item(NAME(m_tura));
	save_item(NAME(m_turb));
	save_item(NAME(m_tx2inv));
	save_item(NAME(m_rx2inv));
	save_item(NAME(m_ir_mode));
	save_item(NAME(m_hduplx));
	save_item(NAME(m_sirrx));
	save_item(NAME(m_sirtx));
	save_item(NAME(m_fasta));
	save_item(NAME(m_fastb));

	save_item(NAME(m_pm1_ad));
	save_item(NAME(m_gpe_ad));
}

void w83877tf_device::device_reset()
{
	m_lock_sequence = 1 + (m_hefras & 1);
	m_fdc_ad = 0;
	m_prt_ad = 0;
	m_pm1_ad = 0;
	m_gpe_ad = 0;
	m_uart_ad[0] = m_uart_ad[1] = 0;
	// TODO: can be POR strapped
	m_pnpcvs = 1;
	pnp_init();

	m_fdc->set_mode(upd765_family_device::mode_t::AT);
	m_fdc->set_rate(500000);

	m_last_dma_line = -1;

	remap(AS_IO, 0, 0x400);
}

void w83877tf_device::pnp_init()
{
	// All zeroes if PNPCVS is low
	const u8 pnpmask = m_pnpcvs ? 0xff : 0x00;
	space().write_byte(0x20, 0xfc & pnpmask);
	space().write_byte(0x23, 0xde & pnpmask);
	space().write_byte(0x24, 0xfe & pnpmask);
	space().write_byte(0x25, 0xbe & pnpmask);
	space().write_byte(0x26, 0x23 & pnpmask);
	space().write_byte(0x27, 0x05 & pnpmask);
	space().write_byte(0x28, 0x43 & pnpmask);
	space().write_byte(0x29, 0x60 & pnpmask);
}

device_memory_interface::space_config_vector w83877tf_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("35ed", FLOPPY_35_ED);
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void w83877tf_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_NASLITE_FORMAT);
}

void w83877tf_device::device_add_mconfig(machine_config &config)
{
	N82077AA(config, m_fdc, XTAL(24'000'000), upd765_family_device::mode_t::AT);
	m_fdc->intrq_wr_callback().set(FUNC(w83877tf_device::irq_floppy_w));
	m_fdc->drq_wr_callback().set(FUNC(w83877tf_device::drq_floppy_w));
	FLOPPY_CONNECTOR(config, "fdc:0", pc_hd_floppies, "35hd", w83877tf_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "fdc:1", pc_hd_floppies, "35hd", w83877tf_device::floppy_formats);

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set(FUNC(w83877tf_device::irq_parallel_w));

	NS16550(config, m_com[0], XTAL(24'000'000) / 13);
	m_com[0]->out_int_callback().set(FUNC(w83877tf_device::irq_serial1_w));
	m_com[0]->out_tx_callback().set(FUNC(w83877tf_device::txd_serial1_w));
	m_com[0]->out_dtr_callback().set(FUNC(w83877tf_device::dtr_serial1_w));
	m_com[0]->out_rts_callback().set(FUNC(w83877tf_device::rts_serial1_w));

	NS16550(config, m_com[1], XTAL(24'000'000) / 13);
	m_com[1]->out_int_callback().set(FUNC(w83877tf_device::irq_serial2_w));
	m_com[1]->out_tx_callback().set(FUNC(w83877tf_device::txd_serial2_w));
	m_com[1]->out_dtr_callback().set(FUNC(w83877tf_device::dtr_serial2_w));
	m_com[1]->out_rts_callback().set(FUNC(w83877tf_device::rts_serial2_w));

}

void w83877tf_device::remap(int space_id, offs_t start, offs_t end)
{
	if (space_id == AS_IO)
	{
		// every single device here won't map if bit 7-6 are 0
		if (m_fdc_ad & 0x300)
		{
			m_isa->install_device(m_fdc_ad, m_fdc_ad + 7, *m_fdc, &n82077aa_device::map);
		}

		if (m_prt_ad & 0x300)
		{
			m_isa->install_device(m_prt_ad, m_prt_ad + 3, *m_lpt, &pc_lpt_device::isa_map);
		}

		for (int i = 0; i < 2; i++)
		{
			const u16 uart_address = m_uart_ad[i];
			m_isa->install_device(uart_address, uart_address + 7, read8sm_delegate(*m_com[i], FUNC(ns16450_device::ins8250_r)), write8sm_delegate(*m_com[i], FUNC(ns16450_device::ins8250_w)));
		}

		// needs to be done after FDC
		u16 superio_base = m_hefras ? 0x3f0 : 0x251;
		m_isa->install_device(superio_base, superio_base + 1, read8sm_delegate(*this, FUNC(w83877tf_device::read)), write8sm_delegate(*this, FUNC(w83877tf_device::write)));
	}
}

uint8_t w83877tf_device::read(offs_t offset)
{
	if (m_lock_sequence)
		return 0;

	if (offset == 0)
		return m_index;

	return space().read_byte(m_index);
}

void w83877tf_device::write(offs_t offset, u8 data)
{
	if (offset == 0)
	{
		if (m_lock_sequence)
		{
			if (data == (0x86 + m_hefere) + ((m_hefras * 2) ^ 2))
			{
				m_lock_sequence --;
				//if (m_lock_sequence == 0)
				//  LOG("Config unlocked\n");
			}
		}
		else
		{
			if ((data == 0xaa && m_hefras) || (data != (0x88 + m_hefere) && !m_hefras))
			{
				//LOG("Config locked\n");
				m_lock_sequence = 1 + (m_hefras & 1);
				return;
			}
			m_index = data;
		}
	}
	else
	{
		if (!m_lock_sequence)
			space().write_byte(m_index, data);
	}

}

void w83877tf_device::config_map(address_map &map)
{
	map(0x00, 0x00).lrw8(
		NAME([this] (offs_t offset) {
			return ((m_prtmods & 3) << 2) | m_ipd;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// legacy power down
			m_ipd = BIT(data, 0);
			// parallel port mode
			m_prtmods &= 0x4;
			m_prtmods |= (data >> 2) & 3;
			LOG("CR0: %02x (IPD %d PRTMODS10 %d)\n"
				, data
				, m_ipd
				, m_prtmods & 3
			);
		})
	);
	map(0x01, 0x01).lrw8(
		NAME([this] (offs_t offset) {
			return m_abchg << 7;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// FDC AB Change Mode
			m_abchg = BIT(data, 7);
			LOG("CR1: %02x (ABCHG %d)\n"
				, data
				, m_abchg
			);
		})
	);
	// 0x02 <reserved>
	map(0x03, 0x03).lrw8(
		NAME([this] (offs_t offset) {
			// claims bit 4 <reserved> and high as default (?)
			return (m_eppver << 5) | 0x10 | (m_suamidi << 1) | (m_submidi);
		}),
		NAME([this] (offs_t offset, u8 data) {
			// MIDI clock support
			m_submidi = BIT(data, 0);
			m_suamidi = BIT(data, 1);
			// EPP 1.7 (1) / EPP 1.9 (0)
			m_eppver = BIT(data, 5);
			LOG("CR3: %02x (SUBMIDI %d SUAMIDI %d EPPVER %d)\n"
				, data
				, m_submidi
				, m_suamidi
				, m_eppver
			);
		})
	);
	map(0x04, 0x04).lrw8(
		NAME([this] (offs_t offset) {
			return (m_prtpwd << 7) | (m_urapwd << 5) | (m_urbpwd << 4) | (m_prttri << 3) | (m_uratri << 1) | (m_urbtri);
		}),
		NAME([this] (offs_t offset, u8 data) {
			// tristate outputs
			m_urbtri = BIT(data, 0);
			m_uratri = BIT(data, 1);
			m_prttri = BIT(data, 3);
			// power-down mode
			m_urbpwd = BIT(data, 4);
			m_urapwd = BIT(data, 5);
			m_prtpwd = BIT(data, 7);
			LOG("CR4: %02x (URBTRI %d URATRI %d PRTTRI %d URBPWD %d URAPWD %d PRTPWD %d)\n", data
				, m_urbtri
				, m_uratri
				, m_prttri
				, m_urbpwd
				, m_urapwd
				, m_prtpwd
			);
		})
	);
	map(0x05, 0x05).lrw8(
		NAME([this] (offs_t offset) {
			return m_ecpfthr;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// ECP FIFO threshold
			m_ecpfthr = data & 0xf;
			LOG("CR5: %02x (ECPFTHR %d)\n", data, m_ecpfthr);
		})
	);
	map(0x06, 0x06).lrw8(
		NAME([this] (offs_t offset) {
			return (m_fdctri << 1);
		}),
		NAME([this] (offs_t offset, u8 data) {
			// FDC tristate outputs
			m_fdctri = BIT(data, 1);
			// FDC power-down
			m_fdcpwd = BIT(data, 3);
			// internal pull-up for RDATA / INDEX / TRAK0 / DSKCHG / WP
			m_fipurdwm = BIT(data, 4);
			// select 4 FDD mode
			m_sel4fdd = BIT(data, 5);
			LOG("CR6: %02x (FDCTRI %d FDCPWD %d FIPURDWM %d SEL4FDD %d)\n"
				, data
				, m_fdctri
				, m_fdcpwd
				, m_fipurdwm
				, m_sel4fdd
			);
		})
	);
	map(0x07, 0x07).lrw8(
		NAME([this] (offs_t offset) {
			return m_fddmode;
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fddmode = data;
			LOG("CR07: FDD mode %02x\n", data);
		})
	);
	map(0x08, 0x08).lrw8(
		NAME([this] (offs_t offset) {
			return (m_floppy_boot);
		}),
		NAME([this] (offs_t offset, u8 data) {
			// 3-mode related
			m_floppy_boot = data & 3;
			m_floppy_mediaid = (data >> 2) & 3;
			// FDD write protect
			m_swwp = BIT(data, 4);
			// disable FDD write
			m_disfddwr = BIT(data, 5);
			LOG("CR8: %02x (Boot drive %d Media ID %d SWWP %d DISFDDWR %d)\n"
				, data
				, m_floppy_boot
				, m_floppy_mediaid
				, m_swwp
				, m_disfddwr
			);
		})
	);
	map(0x09, 0x09).lrw8(
		NAME([this] (offs_t offset) {
			//LOG("CR9: read chip ID\n");
			return (BIT(m_prtmods, 2) << 7) | (m_en3mode << 5) | 0x0c;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// Enable 3-mode
			m_en3mode = BIT(data, 5);
			m_prtmods &= 3;
			m_prtmods |= (BIT(data, 7) << 2);
			LOG("CR9: %02x (EN3MODE %d PRTMODS2 %d)\n", data, m_en3mode, BIT(m_prtmods, 2));
			if (BIT(data, 6))
			{
				m_lock_sequence = 1 + (m_hefras & 1);
				LOG("\tLOCKREG issued\n");
			}
		})
	);
	// 0x0a <reserved>
	map(0x0b, 0x0b).lrw8(
		NAME([this] (offs_t offset) {
			// TODO: DRV2EN bit 0, active low (PS/2 mode only)
			return (m_invertz << 1) | 1;
		}),
		NAME([this] (offs_t offset, u8 data) {
			// FDC invert signals
			m_invertz = BIT(data, 1);
			// IDENT / MFM
			m_fdd_mode = (data >> 2) & 3;
			if (BIT(data, 4))
			{
				switch(m_fdd_mode)
				{
					case 0: m_fdc->set_mode(upd765_family_device::mode_t::M30); break;
					case 1: m_fdc->set_mode(upd765_family_device::mode_t::PS2); break;
					case 2:
					case 3: m_fdc->set_mode(upd765_family_device::mode_t::AT);  break;
				}
			}
			// IR controller 4-character period wait
			m_rxw4c = BIT(data, 5);
			m_txw4c = BIT(data, 6);
			LOG("CR0B: %02x (DRV2EN %d INVERTZ %d FDD mode %d ENIFCHG %d RXW4C %d TXW4C)\n"
				, data
				, !BIT(data, 0)
				, m_invertz
				, m_fdd_mode
				, BIT(data, 4)
				, m_rxw4c
				, m_txw4c
			);
		})
	);
	map(0x0c, 0x0c).lrw8(
		NAME([this] (offs_t offset) {
			return (m_tura << 7) | (m_turb << 6) | (m_hefere << 5) | (m_urirsel << 3) | (m_rx2inv << 1) | (m_tx2inv << 0);
		}),
		NAME([this] (offs_t offset, u8 data) {
			// UARTB invert SOUTB / SINB pins
			m_tx2inv = BIT(data, 0);
			m_rx2inv = BIT(data, 1);
			// select UARTB as IR if 1
			m_urirsel = BIT(data, 3);
			// chip EFER enable method
			m_hefere = BIT(data, 5);
			// UARTA/B clock source (0) 24 MHz / 13 (1) 24 Mhz
			m_turb = BIT(data, 6);
			m_tura = BIT(data, 7);
			LOG("CR0C: %02x ()\n"
				, data
				, m_tx2inv
				, m_rx2inv
				, m_urirsel
				, m_hefere
				, m_turb
				, m_tura
			);
		})
	);
	map(0x0d, 0x0d).lrw8(
		NAME([this] (offs_t offset) {
			return (m_ir_mode & 7);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_ir_mode = data & 7;
			m_hduplx = BIT(data, 3);
			m_sirrx = (data >> 4) & 3;
			m_sirtx = (data >> 6) & 3;
			LOG("CR0D: %02x (IRMODE %d HDUPLX %d SIRRX %d SIRTX %d)\n"
				, data
				, m_ir_mode
				, m_hduplx
				, m_sirrx
				, m_sirtx
			);
		})
	);
	// 0x0e, 0x0f <reserved> test mode
	// 0x10, 0x15 GIO
	map(0x16, 0x16).lrw8(
		NAME([this] (offs_t offset) {
			return (m_pnpcvs << 2) | (m_hefras << 0);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_hefras = BIT(data, 0);
			m_pnpcvs = BIT(data, 2);
			LOG("CR16: %02x (HEFRAS %d G0IQSEL %d G1IQSEL %d)\n"
				, data
				, m_hefras
				, BIT(data, 4)
				, BIT(data, 5)
			);
			LOG("\tInitiate PnP sequence PNPCVS %d\n", BIT(data, 2));
			pnp_init();
		})
	);
	// 0x17 legacy IRQ
	// 0x18 ISA sharing IRQ
	map(0x19, 0x19).lrw8(
		NAME([this] (offs_t offset) {
			return (m_fasta << 1) | (m_fastb << 0);
		}),
		NAME([this] (offs_t offset, u8 data) {
			// UART clock select for modem (1) 14.769 MHz
			m_fastb = BIT(data, 0);
			m_fasta = BIT(data, 1);
			LOG("CR19: %02x (FASTB %d FASTA %d)\n"
				, data
				, m_fastb
				, m_fasta
			);
		})
	);
	map(0x20, 0x20).lrw8(
		NAME([this] (offs_t offset) {
			return (m_fdc_ad >> 2);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_fdc_ad = (data & 0xfc) << 2;
			LOG("CR20: %02x (FDCAD %04x)\n", data, m_fdc_ad);
			remap(AS_IO, 0, 0x400);
		})
	);
	map(0x23, 0x23).lrw8(
		NAME([this] (offs_t offset) {
			return (m_prt_ad >> 2);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_prt_ad = (data & 0xff) << 2;
			LOG("CR23: %02x (PRTAD %04x)\n", data, m_prt_ad);
			remap(AS_IO, 0, 0x400);
		})
	);
	// URAAD / URBAD
	map(0x24, 0x25).lrw8(
		NAME([this] (offs_t offset) {
			return (m_uart_ad[offset] >> 2);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_uart_ad[offset] = (data & 0xfe) << 2;
			LOG("CR%02X: %02x (%s %04x)\n", offset + 0x24, data, offset ? "URBAD" : "URAAD", m_uart_ad[offset]);
			remap(AS_IO, 0, 0x400);
		})
	);
	map(0x26, 0x26).lrw8(
		NAME([this] (offs_t offset) {
			return (m_fdc_dqs << 4) | (m_prt_dqs << 0);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_prt_dqs = data & 0xf;
			m_fdc_dqs = (data >> 4) & 0xf;
			LOG("CR26: %02x (PRTDQS %d FDCDQS %d)\n", data, m_prt_dqs, m_fdc_dqs);
			update_dreq_mapping(m_fdc_dqs, 0);
		})
	);
	map(0x27, 0x27).lrw8(
		NAME([this] (offs_t offset) {
			return (m_ecpirq << 5) | (m_prt_iqs << 0);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_prt_iqs = data & 0xf;
			m_ecpirq = (data >> 5) & 7;
			LOG("CR27: %02x (PRTIQS %d ECPIRQ %d)\n", data, m_prt_iqs, m_ecpirq);
		})
	);
	map(0x28, 0x28).lrw8(
		NAME([this] (offs_t offset) {
			return (m_uart_iqs[0] << 4) | (m_uart_iqs[1] << 0);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_uart_iqs[1] = data & 0xf;
			m_uart_iqs[0] = (data >> 4) & 0xf;
			LOG("CR28: %02x (URAIQS %d URBIQS %d)\n", data, m_uart_iqs[0], m_uart_iqs[1]);
		})
	);
	map(0x29, 0x29).lrw8(
		NAME([this] (offs_t offset) {
			return (m_fdc_iqs << 4) | (m_irqin_iqs << 0);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_irqin_iqs = data & 0xf;
			m_fdc_iqs = (data >> 4) & 0xf;
			LOG("CR29: %02x (FDCIQS %d IQNIQS %d)\n", data, m_fdc_iqs, m_irqin_iqs);
		})
	);
	map(0x2c, 0x2c).lrw8(
		NAME([this] (offs_t offset) {
			return (m_clkinsel << 2);
		}),
		NAME([this] (offs_t offset, u8 data) {
			// (0) 24 MHz (1) 48 MHz
			m_clkinsel = BIT(data, 2);
			LOG("CR2C: %02x (CLKINSEL %d)\n", data, m_clkinsel);
		})
	);
	// 0x2d FDC data rate selection + PRECOMP
	// 0x31 IRQMODS (ISA sharing mode) + SCI IRQ
	// 0x32 Power Management enable
	map(0x33, 0x33).lrw8(
		NAME([this] (offs_t offset) {
			return (m_pm1_ad >> 2);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_pm1_ad = (data & 0xfc) << 2;
			LOG("CR33: %02x (PM1AD %04x)\n", data, m_pm1_ad);
			remap(AS_IO, 0, 0x400);
		})
	);
	map(0x34, 0x34).lrw8(
		NAME([this] (offs_t offset) {
			return (m_gpe_ad >> 2);
		}),
		NAME([this] (offs_t offset, u8 data) {
			m_gpe_ad = (data & 0xfe) << 2;
			LOG("CR34: %02x (GPEAD %04x)\n", data, m_gpe_ad);
			remap(AS_IO, 0, 0x400);
		})
	);
	// 0x35 / 0x36 URACNT ~ URBCNT idle counter
	// 0x37 FDCCNT idle counter
	// 0x38 PRTCNT idle counter
	// 0x39 GSBCNT global stand-by idle counter
	// 0x3a SMI enable, TMIN_SEL, pull up of IRQSER
	// 0x40 device idle status (w/c)
	// 0x41 device trap status (w/c)
	// 0x42 device irq status (r/o)
	// 0x43 <reserved>
	// 0x44 <reserved>
	// 0x45 device SMI enable
}

void w83877tf_device::request_irq(int irq, int state)
{
	switch (irq)
	{
	case 1:
		m_irq1_callback(state);
		break;
	// 2 is reserved for SMI
	case 3:
		m_isa->irq3_w(state);
		break;
	case 4:
		m_isa->irq4_w(state);
		break;
	case 5:
		m_isa->irq5_w(state);
		break;
	case 6:
		m_isa->irq6_w(state);
		break;
	case 7:
		m_isa->irq7_w(state);
		break;
	case 8:
		m_irq8_callback(state);
		break;
	case 9:
		m_irq9_callback(state);
		break;
	case 10:
		m_isa->irq10_w(state);
		break;
	case 11:
		m_isa->irq11_w(state);
		break;
	case 12:
		m_isa->irq12_w(state);
		break;
	//case 13:
	//	m_isa->irq13_w(state);
	//	break;
	case 14:
		m_isa->irq14_w(state);
		break;
	case 15:
		m_isa->irq15_w(state);
		break;
	}
}

void w83877tf_device::request_dma(int dreq, int state)
{
	switch (dreq)
	{
	case 0:
		m_isa->drq0_w(state);
		break;
	case 1:
		m_isa->drq1_w(state);
		break;
	case 2:
		m_isa->drq2_w(state);
		break;
	case 3:
		m_isa->drq3_w(state);
		break;
	}
}

/*
 * FDC
 */

void w83877tf_device::irq_floppy_w(int state)
{
	if ((m_fdc_ad & 0x300) == 0)
		return;
	request_irq(m_fdc_iqs, state ? ASSERT_LINE : CLEAR_LINE);
}

void w83877tf_device::drq_floppy_w(int state)
{
	if ((m_fdc_ad & 0x300) == 0)
		return;
	request_dma(m_fdc_dqs, state ? ASSERT_LINE : CLEAR_LINE);
}

/*
 * Parallel
 */

void w83877tf_device::irq_parallel_w(int state)
{
	if ((m_prt_ad & 0x300) == 0)
		return;
	request_irq(m_prt_iqs, state ? ASSERT_LINE : CLEAR_LINE);
}

/*
 * COM1/2 Serial ports
 */

void w83877tf_device::irq_serial1_w(int state)
{
	if ((m_uart_ad[0] & 0x300) == 0)
		return;
	request_irq(m_uart_iqs[0], state ? ASSERT_LINE : CLEAR_LINE);
}

void w83877tf_device::irq_serial2_w(int state)
{
	if ((m_uart_ad[1] & 0x300) == 0)
		return;
	request_irq(m_uart_iqs[1], state ? ASSERT_LINE : CLEAR_LINE);
}

void w83877tf_device::txd_serial1_w(int state)
{
	if ((m_uart_ad[0] & 0x300) == 0)
		return;
	m_txd1_callback(state);
}

void w83877tf_device::txd_serial2_w(int state)
{
	if ((m_uart_ad[1] & 0x300) == 0)
		return;
	m_txd2_callback(state);
}

void w83877tf_device::dtr_serial1_w(int state)
{
	if ((m_uart_ad[0] & 0x300) == 0)
		return;
	m_ndtr1_callback(state);
}

void w83877tf_device::dtr_serial2_w(int state)
{
	if ((m_uart_ad[1] & 0x300) == 0)
		return;
	m_ndtr2_callback(state);
}

void w83877tf_device::rts_serial1_w(int state)
{
	if ((m_uart_ad[0] & 0x300) == 0)
		return;
	m_nrts1_callback(state);
}

void w83877tf_device::rts_serial2_w(int state)
{
	if ((m_uart_ad[1] & 0x300) == 0)
		return;
	m_nrts2_callback(state);
}

void w83877tf_device::rxd1_w(int state)
{
	m_com[0]->rx_w(state);
}

void w83877tf_device::ndcd1_w(int state)
{
	m_com[0]->dcd_w(state);
}

void w83877tf_device::ndsr1_w(int state)
{
	m_com[0]->dsr_w(state);
}

void w83877tf_device::nri1_w(int state)
{
	m_com[0]->ri_w(state);
}

void w83877tf_device::ncts1_w(int state)
{
	m_com[0]->cts_w(state);
}

void w83877tf_device::rxd2_w(int state)
{
	m_com[1]->rx_w(state);
}

void w83877tf_device::ndcd2_w(int state)
{
	m_com[1]->dcd_w(state);
}

void w83877tf_device::ndsr2_w(int state)
{
	m_com[1]->dsr_w(state);
}

void w83877tf_device::nri2_w(int state)
{
	m_com[1]->ri_w(state);
}

void w83877tf_device::ncts2_w(int state)
{
	m_com[1]->cts_w(state);
}


/*
 * DMA
 */

void w83877tf_device::update_dreq_mapping(int dreq, int logical)
{
	if ((dreq < 0) || (dreq >= 4))
		return;
	for (int n = 0; n < 4; n++)
		if (m_dreq_mapping[n] == logical)
			m_dreq_mapping[n] = -1;
	m_dreq_mapping[dreq] = logical;
}

void w83877tf_device::eop_w(int state)
{
	// dma transfer finished
	if (m_last_dma_line < 0)
		return;
	switch (m_dreq_mapping[m_last_dma_line])
	{
	case 0:
		m_fdc->tc_w(state == ASSERT_LINE);
		break;
	default:
		break;
	}
	//m_last_dma_line = -1;
}

// TODO: LPT bindings
uint8_t w83877tf_device::dack_r(int line)
{
	// transferring data from device to memory using dma
	// read one byte from device
	m_last_dma_line = line;
	switch (m_dreq_mapping[line])
	{
	case 0:
		return m_fdc->dma_r();
	default:
		break;
	}
	return 0;
}

void w83877tf_device::dack_w(int line, uint8_t data)
{
	// transferring data from memory to device using dma
	// write one byte to device
	m_last_dma_line = line;
	switch (m_dreq_mapping[line])
	{
	case 0:
		m_fdc->dma_w(data);
		break;
	default:
		break;
	}
}



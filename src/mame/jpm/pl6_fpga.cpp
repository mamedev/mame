// license:BSD-3-Clause
// copyright-holders: NaokiS

#include "emu.h"
#include "pl6_fpga.h"


#define LOG_FPGA    (1U << 1)
#define LOG_OUTPUTS (1U << 2)
#define LOG_LAMPS   (1U << 3)
#define LOG_UART    (1U << 4)
#define VERBOSE     ( LOG_FPGA | LOG_UART )

#include "logmacro.h"

#define LOGFPGA(...)        LOGMASKED(LOG_FPGA, __VA_ARGS__)
#define LOGOUTPUTS(...)     LOGMASKED(LOG_OUTPUTS, __VA_ARGS__)
#define LOGLAMPS(...)       LOGMASKED(LOG_LAMPS, __VA_ARGS__)

static const char *const PL6_ROUTE_NAMES[][16] = {
	{"BACTA Port (B)"},
	{"RS232 Port A"},
	{"RS232 Port C"},
	{"RS232 Port D"},
	{"CCTALK Port 1"},
	{"CCTALK Port 2"},
	{"TTL Port"},
	{"RS485 Port"}
};

//************************************
//  FPGA Registers
//************************************

uint8_t pl6fpga_device::xcra_read(){
	if(!machine().side_effects_disabled()) LOGFPGA( "%s: xcra_read( %02x )\n", this->machine().describe_context(), xcra_reg );
	// xcra & 0x1 is hard reset
	return xcra_reg;
}

void pl6fpga_device::xcra_write(uint8_t data){
	LOGFPGA( "%s: xcra_write( %02x )\n", this->machine().describe_context(), data );
	xcra_reg = data;
}

void pl6fpga_device::sfxv_write(uint8_t data){
	LOGFPGA( "%s: sfxv_write( %02x )\n", this->machine().describe_context(), data );
	// Todo: Add volume support
}

void pl6fpga_device::sfxr1_write(uint16_t data){
	LOGFPGA( "%s: sfxr1_write( %04x )\n", this->machine().describe_context(), data );
	m_ldac->write(data);
}

void pl6fpga_device::sfxr2_write(uint16_t data){
	LOGFPGA( "%s: sfxr2_write( %04x, )\n", this->machine().describe_context(), data );
	m_rdac->write(data);
}

void pl6fpga_device::xmpx_write(uint8_t data){
	LOGLAMPS("%s: xmpx_write( %02x )\n", this->machine().describe_context(), data );
	mpx_row = ((data + 1) & 0xF);   // Lamp(0,0) starts at 0x0F. Dunno why.
}

void pl6fpga_device::aux_write(offs_t offset, uint8_t data)
{
	LOGOUTPUTS("%s: aux_write( %02x, %02x )\n", this->machine().describe_context(), offset, data );
	auxout_cb(offset, ((data != 0) ? 1 : 0));
}

void pl6fpga_device::output_write(offs_t offset, uint32_t data)
{
	// Despite regular naming convetion, each byte is reversed compared to the PCB
	//  labeling. OP63 is bit 8 of 0xX0001804 etc

	LOGOUTPUTS( "%s: out_write( %02x, %08x )\n", this->machine().describe_context(), offset, data );
	for(int i = 0; i < 4; i++){
		for(int x = 0; x < 8; x++){
			output_cb(x + (24 - (8 * i)) + (32 * offset), (data & (1 << ((8 * i) + x))) ? 1 : 0);
		}
	}
}

void pl6fpga_device::lamp_write(offs_t offset, uint32_t data)
{
	// 0-15: Lamps
	// 15-23: LED Display x+0
	// 24-31: LED Display x+1
	// Offset 0-7: Light on period 0 - 7

	LOGLAMPS( "%s: lamp_write_input( %02x, %08x )\n", this->machine().describe_context(), offset, data );

	if(mpx_regs[mpx_row][offset] != data){
		mpx_regs[mpx_row][offset] = data;

		// As light period is stored across the 8 registers,
		//  its required to cycle through them.
		for(int i = 0; i < 32; i++){
			// i = Bit in reg number 0-7
			int lampval = 0;
			for(int r = 0; r < 8; r++){
				// x = Reg number 0-7
				if(mpx_regs[mpx_row][r] & (1 << i)) lampval++;
			}
			if(i < 16){
				if(xcra_reg & MPX_EN) lamp_cb((16 * mpx_row) + i, lampval);
			}
			else {
				if(xcra_reg & MPX_EN) led_cb((16 * mpx_row) + (i - 16), lampval);
			}
		}
	}
}

//************************************
//  UART + Routing
//************************************

static DEVICE_INPUT_DEFAULTS_START( pluto6_rs232_defaults )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_2400 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


void pl6fpga_device::xuctrl_write(uint8_t data){
	LOGFPGA( "%s: ColdFire UART Route write ( %02x, 0: %s, 1: %s )\n", this->machine().describe_context(), data, PL6_ROUTE_NAMES[data & 0xf], PL6_ROUTE_NAMES[((data & 0xf0) >> 4)] );
	xuctrl_reg = data;
	map_uart_endpoint(coldfire_uart_0, data & 0x07);
	map_uart_endpoint(coldfire_uart_1, (data & 0x70) >> 4);
}

void pl6fpga_device::xdurt_write(uint8_t data){
	LOGFPGA( "%s: DUART Route write ( %02x, 0: %s, 1: %s )\n", this->machine().describe_context(), data, PL6_ROUTE_NAMES[data & 0xf], PL6_ROUTE_NAMES[((data & 0xf0) >> 4)] );
	xdurt_reg = data;
	map_uart_endpoint(duart_uart_0, data & 0x07);
	map_uart_endpoint(duart_uart_1, (data & 0x70) >> 4);
}

void pl6fpga_device::xfpurt0_write(uint8_t data){
	LOGFPGA( "%s: FPGA UART0 Route write ( %02x, 0: %s)\n", this->machine().describe_context(), data, PL6_ROUTE_NAMES[data & 0xf]);
	xfpurt0_reg = data;
	map_uart_endpoint(fpga_uart_0, data & 0x07);
}

void pl6fpga_device::xfpurt1_write(uint8_t data){
	LOGFPGA( "%s: FPGA UART1 Route write ( %02x, 1: %s)\n", this->machine().describe_context(), data, PL6_ROUTE_NAMES[data & 0xf]);
	xfpurt1_reg = data;
	map_uart_endpoint(fpga_uart_1, data & 0x07);
}

void pl6fpga_device::map_uart_endpoint(uint8_t device, uint8_t route){
	if(device >= device_invalid) {
		LOGFPGA( "%s: map_uart_endpoint: Invalid device: %d )\n", this->machine().describe_context(), device );
		return;
	}
	if(route >= endpoint_invalid) {
		LOGFPGA( "%s: map_uart_endpoint: Invalid route: %d )\n", this->machine().describe_context(), route );
		return;
	}

	//
	//for(int i = 0; i < device_invalid; i++){
	//  if(uart_route_map[i] == route) uart_route_map[i] = endpoint_invalid;
	//}
	uart_route_map[device] = route;
}

void pl6fpga_device::uart_txd(uint8_t device, int state){
	if(device >= device_invalid) return;
	switch (uart_route_map[device]){
		case endpoint_rs232a: m_rs232a->write_txd(state); break;
		case endpoint_bacta: m_bacta->write_txd(state); break;
		case endpoint_rs232c: m_rs232c->write_txd(state); break;
		case endpoint_rs232d: m_rs232d->write_txd(state); break;
		case endpoint_cctalk1: m_cctalk1->write_txd(state); break;
		case endpoint_cctalk2: m_cctalk2->write_txd(state); break;
		case endpoint_ttl: m_serttl->write_txd(state); break;
		case endpoint_rs485: m_rs485->write_txd(state); break;
		default: break;
	}
}

void pl6fpga_device::uart_rxd(uint8_t device, int state){
	if(device >= device_invalid) return;
	for(int i = 0; i < endpoint_invalid; i++){
		if(uart_route_map[i] == device){
			switch(i){
				case coldfire_uart_0: cfuart_rx_a_cb(state); break;
				case coldfire_uart_1: cfuart_rx_b_cb(state); break;
				case fpga_uart_0: /*fpga_a_rw_w(state);*/ break;
				case fpga_uart_1: /*fpga_b_rw_w(state);*/ break;
				case duart_uart_0: duart_rx_a_cb(state); break;
				case duart_uart_1: duart_rx_b_cb(state); break;
				default: break;
			}
			return;
		}
	}
}

//************************************
//  FPGA Bitstream Maps
//************************************
void pl6fpga_device::developer_map(address_map &map)
{
	// Regs
	map(0x0000, 0x0000).rw(FUNC(pl6fpga_device::xcra_read), FUNC(pl6fpga_device::xcra_write));
	map(0x0001, 0x0001).rw(FUNC(pl6fpga_device::fpga_id), FUNC(pl6fpga_device::xmpx_write));
	map(0x0002, 0x0002).rw(FUNC(pl6fpga_device::fpga_iss), FUNC(pl6fpga_device::sfxv_write));
	map(0x0003, 0x0003).rw(FUNC(pl6fpga_device::xuctrl_read), FUNC(pl6fpga_device::xuctrl_write));
	map(0x0004, 0x0007).portr("IN1");
	map(0x0004, 0x0005).w(FUNC(pl6fpga_device::sfxr1_write));
	map(0x0006, 0x0007).w(FUNC(pl6fpga_device::sfxr2_write));
	map(0x0008, 0x0008).rw(FUNC(pl6fpga_device::xdurt_read), FUNC(pl6fpga_device::xdurt_write));
	map(0x0009, 0x0009).w(FUNC(pl6fpga_device::crcchk_write));
	// MPX Lamps
	map(0x0800, 0x081f).w(FUNC(pl6fpga_device::lamp_write));
	// UART
	map(0x1002, 0x1002).rw(FUNC(pl6fpga_device::xfpurt0_read), FUNC(pl6fpga_device::xfpurt0_write));
	//map(0x1003, 0x1003).w(FUNC(pl6fpga_device::fpuart0_rx));?                         // FPUART0 RX
	map(0x1006, 0x1006).rw(FUNC(pl6fpga_device::xfpurt1_read), FUNC(pl6fpga_device::xfpurt1_write));
	//map(0x1007, 0x1007).w(FUNC(pl6fpga_device::fpuart1_rx));?                         // FPUART1 RX
	// DUART
	map(0x1010, 0x101f).rw(FUNC(pl6fpga_device::duart_r), FUNC(pl6fpga_device::duart_w));
	// IO
	map(0x1800, 0x1803).r(FUNC(pl6fpga_device::input_r));
	map(0x1800, 0x1807).w(FUNC(pl6fpga_device::output_write));
	map(0x1900, 0x191f).w(FUNC(pl6fpga_device::aux_write));
}

void pl6fpga_device::betcom_map(address_map &map){
	map(0x03b9, 0x03b9).w(FUNC(pl6fpga_device::crcchk_write));
	map(0x2900, 0x2907).w(FUNC(pl6fpga_device::aux_write));
}

void pl6fpga_device::jpm_map(address_map &map){
	map(0x0010, 0x0010).rw(FUNC(pl6fpga_device::xcra_read), FUNC(pl6fpga_device::xcra_write));  // ?
	//map(0x0011, 0x0011).rw(FUNC(pl6fpga_device::), FUNC(pl6fpga_device::));
	map(0x0018, 0x0018).w(FUNC(pl6fpga_device::crcchk_write));
	map(0x0900, 0x091f).w(FUNC(pl6fpga_device::lamp_write));    // ?
	map(0x1900, 0x1903).rw(FUNC(pl6fpga_device::input_r), FUNC(pl6fpga_device::output_write));      // ?
	map(0x1904, 0x1904).w(FUNC(pl6fpga_device::xmpx_write));    // ?
	map(0x1a00, 0x1a07).w(FUNC(pl6fpga_device::aux_write));     // ?
}

//************************************
//  Device Driver Functions
//************************************

pl6fpga_device::pl6fpga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HEBER_PLUTO6_FPGA, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, m_space_config("fpga_regs", ENDIANNESS_BIG, 32, 14, 0)
	, m_ldac(*this, "ldac")
	, m_rdac(*this, "rdac")
	, m_rs232a(*this, "rs232a")
	, m_bacta(*this, "bactaport")
	, m_rs232c(*this, "rs232c")
	, m_rs232d(*this, "rs232d")
	, m_cctalk1(*this, "cctalk1")
	, m_cctalk2(*this, "cctalk2")
	, m_serttl(*this, "ttl")
	, m_rs485(*this, "rs485")
	, input_cb(*this, 0xFFFFFFFF)
	, lamp_cb(*this)
	, led_cb(*this)
	, output_cb(*this)
	, auxout_cb(*this)
	, write_irq(*this)
	, cfuart_rx_a_cb(*this)
	, cfuart_rx_b_cb(*this)
	, duart_rx_a_cb(*this)
	, duart_rx_b_cb(*this)
	, duart_r_cb(*this, 0xFF)
	, duart_w_cb(*this)
	, m_in1(*this, "IN1")
	, fpga_type(invalid_fpga)
{
}

device_memory_interface::space_config_vector pl6fpga_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(0, &m_space_config)
	};
}

void pl6fpga_device::set_fpga_type(int type){
	if(!(type >= invalid_fpga)){
		fpga_type = type;
		switch(fpga_type){
			case developer_fpga: m_space_config.m_internal_map = address_map_constructor(FUNC(pl6fpga_device::developer_map), this); break;
			case betcom_fpga: m_space_config.m_internal_map = address_map_constructor(FUNC(pl6fpga_device::betcom_map), this); break;
			case jpm_fpga: m_space_config.m_internal_map = address_map_constructor(FUNC(pl6fpga_device::jpm_map), this); break;
			default: break;
		}
	}
}

void pl6fpga_device::device_start(){
	save_item(NAME(xcra_reg));
	save_item(NAME(xuctrl_reg));
	save_item(NAME(xdurt_reg));
	save_item(NAME(xfpurt0_reg));
	save_item(NAME(xfpurt1_reg));
	save_item(NAME(mpx_row));
	save_item(NAME(mpx_regs));

	save_item(NAME(fpga_type));
}

void pl6fpga_device::device_reset(){
	fpga_type = invalid_fpga;
	xcra_reg = 0;
	xuctrl_reg = 0;
	xdurt_reg = 0;
	xfpurt0_reg = 0;
	xfpurt1_reg = 0;

	mpx_row = 0x0f;
	memset(mpx_regs, 0, sizeof(mpx_regs));
	for(int o = 0; o < 64; o++){
		output_cb(o, 0);
	}
	for(int a = 0; a < 6; a++){
		auxout_cb(a, 0);
	}
	for(int i = 0; i < 256; i++){
		led_cb(i, 0);
		lamp_cb(i, 0);
	}

}

void pl6fpga_device::device_add_mconfig(machine_config &config)
{
	CS4334(config, m_ldac, 0).add_route(ALL_OUTPUTS, ":speaker", 1.0, 0);
	CS4334(config, m_rdac, 0).add_route(ALL_OUTPUTS, ":speaker", 1.0, 1);

	RS232_PORT(config, m_rs232a, default_rs232_devices, nullptr);
	m_rs232a->rxd_handler().set(FUNC(pl6fpga_device::rs232a_rx_w));
	RS232_PORT(config, m_bacta, default_rs232_devices, nullptr);
	m_bacta->rxd_handler().set(FUNC(pl6fpga_device::bacta_rx_w));
	RS232_PORT(config, m_rs232c, default_rs232_devices, nullptr);
	m_rs232c->rxd_handler().set(FUNC(pl6fpga_device::rs232c_rx_w));
	RS232_PORT(config, m_rs232d, default_rs232_devices, nullptr);
	m_rs232d->rxd_handler().set(FUNC(pl6fpga_device::rs232d_rx_w));
	RS232_PORT(config, m_cctalk1, default_rs232_devices, nullptr);
	m_cctalk1->rxd_handler().set(FUNC(pl6fpga_device::cctalk1_rx_w));
	RS232_PORT(config, m_cctalk2, default_rs232_devices, nullptr);
	m_cctalk2->rxd_handler().set(FUNC(pl6fpga_device::cctalk2_rx_w));
	RS232_PORT(config, m_serttl, default_rs232_devices, nullptr);
	m_serttl->rxd_handler().set(FUNC(pl6fpga_device::serttl_rx_w));
	RS232_PORT(config, m_rs485, default_rs232_devices, nullptr);
	m_rs485->rxd_handler().set(FUNC(pl6fpga_device::rs485_rx_w));
}

uint32_t pl6fpga_device::dev_r(offs_t offset, uint32_t mem_mask)
{
	address_space &reg_space = this->space();
	return reg_space.read_dword(offset*4, mem_mask);
}

void pl6fpga_device::dev_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	address_space &reg_space = this->space();
	reg_space.write_dword(offset*4, data, mem_mask);
}

INPUT_PORTS_START(pluto6_fpga)
	PORT_START( "IN1" )
	PORT_BIT( 0x01010101, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10101010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_TOGGLE PORT_NAME("Meter Sense")
	PORT_BIT( 0x02020202, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("SW3")
	PORT_BIT( 0x04040404, IP_ACTIVE_LOW, IPT_UNKNOWN )  // MPX Test?
	PORT_BIT( 0x08080808, IP_ACTIVE_LOW, IPT_UNKNOWN )  // MPX Test?
INPUT_PORTS_END

ioport_constructor pl6fpga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pluto6_fpga);
}

DEFINE_DEVICE_TYPE(HEBER_PLUTO6_FPGA, pl6fpga_device, "pl6_fpga", "Heber Pluto 6 FPGA")

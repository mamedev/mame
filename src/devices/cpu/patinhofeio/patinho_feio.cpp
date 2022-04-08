// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/*
    CPU emulation for Patinho Feio, the first computer designed and manufactured in Brazil
*/

#include "emu.h"
#include "patinhofeio_cpu.h"
#include "patinho_feio_dasm.h"
#include "includes/patinhofeio.h" // FIXME: this is a dependency from devices on MAME

#define PC       m_pc //The program counter is called "contador de instrucoes" (IC) in portuguese
#define ACC      m_acc
#define EXT      m_ext
#define RC       read_panel_keys_register()
#define FLAGS    m_flags

#define V 0x01 // V = "Vai um" (Carry)
#define T 0x02 // T = "Transbordo" (Overflow)

#define READ_BYTE_PATINHO(A) (m_program->read_byte(A))
#define WRITE_BYTE_PATINHO(A,V) (m_program->write_byte(A,V))

#define READ_WORD_PATINHO(A) (READ_BYTE_PATINHO(A+1)*256 + READ_BYTE_PATINHO(A))

#define READ_INDEX_REG() READ_BYTE_PATINHO(0x000)
#define WRITE_INDEX_REG(V) { WRITE_BYTE_PATINHO(0x000, V); m_idx = V; }

#define READ_ACC_EXTENSION_REG() READ_BYTE_PATINHO(0x001)
#define WRITE_ACC_EXTENSION_REG(V) { WRITE_BYTE_PATINHO(0x001, V); m_ext = V; }

#define ADDRESS_MASK_4K    0xFFF
#define INCREMENT_PC_4K    (PC = (PC+1) & ADDRESS_MASK_4K)

void patinho_feio_cpu_device::set_flag(uint8_t flag, bool state){
	if (state){
		FLAGS |= flag;
	} else {
		FLAGS &= ~flag;
	}
}

void patinho_feio_cpu_device::compute_effective_address(unsigned int addr){
	m_addr = addr;
	if (m_indirect_addressing){
		m_addr = READ_WORD_PATINHO(m_addr);
		if (m_addr & 0x1000)
			compute_effective_address(m_addr & 0xFFF);
	}
}

DEFINE_DEVICE_TYPE(PATO_FEIO_CPU, patinho_feio_cpu_device, "pato_feio_cpu", "Patinho Feio CPU")

//Internal 4kbytes of RAM
void patinho_feio_cpu_device::prog_8bit(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("internalram");
}

patinho_feio_cpu_device::patinho_feio_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, PATO_FEIO_CPU, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 12, 0, address_map_constructor(FUNC(patinho_feio_cpu_device::prog_8bit), this))
	, m_icount(0)
	, m_rc_read_cb(*this)
	, m_buttons_read_cb(*this)
	, m_iodev_read_cb(*this)
	, m_iodev_write_cb(*this)
	, m_iodev_status_cb(*this)
{
}

device_memory_interface::space_config_vector patinho_feio_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

uint16_t patinho_feio_cpu_device::read_panel_keys_register(){
	if (!m_rc_read_cb.isnull())
		m_rc = m_rc_read_cb(0);
	else
		m_rc = 0;

	return m_rc;
}

void patinho_feio_cpu_device::transfer_byte_from_external_device(uint8_t channel, uint8_t data){
	m_iodev_incoming_byte[channel] = data;
	m_iodev_status[channel] = IODEV_READY;
	m_iodev_control[channel] = NO_REQUEST;
}

void patinho_feio_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);

//TODO: implement handling of these special purpose registers
//      which are also mapped to the first few main memory positions:
//
//      ERI: "Endereco de Retorno de Interrupcao"
//           "Interrupt Return Address"
//           stored at addresses 002 and 003
//
//      ETI: "inicio de uma rotina de tratamento de interrupcao (se houver)"
//           "start of an interrupt service routine (if any)"
//           stored at address 004 (and 005 as well?)
//
// It seems that the general purpose memory starts at address 006.

	save_item(NAME(m_pc));
	save_item(NAME(m_acc));
	save_item(NAME(m_ext));
	save_item(NAME(m_rc));
	save_item(NAME(m_idx));
	save_item(NAME(m_flags));
	save_item(NAME(m_addr));
	save_item(NAME(m_opcode));

	// Register state for debugger
	state_add( PATINHO_FEIO_CI,         "CI",       m_pc         ).mask(0xFFF);
	state_add( PATINHO_FEIO_RC,         "RC",       m_rc         ).mask(0xFFF);
	state_add( PATINHO_FEIO_ACC,        "ACC",      m_acc        ).mask(0xFF);
	state_add( PATINHO_FEIO_EXT,        "EXT",      m_ext        ).mask(0xFF);
	state_add( PATINHO_FEIO_IDX,        "IDX",      m_idx        ).mask(0xFF);
	state_add(STATE_GENPC, "GENPC", m_pc).formatstr("0%06O").noshow();
	state_add(STATE_GENPCBASE, "CURPC", m_pc).formatstr("0%06O").noshow();
	state_add(STATE_GENFLAGS,  "GENFLAGS",  m_flags).noshow().formatstr("%8s");

	m_rc_read_cb.resolve();
	if (m_rc_read_cb.isnull()){
		fatalerror("Panel keys register not found!");
	}

	m_buttons_read_cb.resolve();

	m_iodev_read_cb.resolve_all();
	m_iodev_write_cb.resolve_all();
	m_iodev_status_cb.resolve_all(); // unused?

	set_icountptr(m_icount);
}

void patinho_feio_cpu_device::device_reset()
{
	m_pc = 0;
	//m_pc = 0x006; //"PATINHO FEIO" hello-world
	//m_pc = 0x010; //micro-pre-loader
	//m_pc = 0xE00; //HEXAM
	m_rc = 0;
	m_acc = 0;
	m_ext = READ_ACC_EXTENSION_REG();
	m_idx = READ_INDEX_REG();
	m_flags = 0;
	m_run = false;
	m_scheduled_IND_bit_reset = false;
	m_indirect_addressing = false;
	m_addr = 0;
	m_opcode = 0;
	m_mode = ADDRESSING_MODE;
	((patinho_feio_state*) owner())->update_panel(ACC, m_opcode, READ_BYTE_PATINHO(m_addr), m_addr, PC, FLAGS, RC, m_mode);
}

/* execute instructions on this CPU until icount expires */
void patinho_feio_cpu_device::execute_run() {
	do {
		read_panel_keys_register();
		m_ext = READ_ACC_EXTENSION_REG();
		m_idx = READ_INDEX_REG();
		((patinho_feio_state*) owner())->update_panel(ACC, READ_BYTE_PATINHO(PC), READ_BYTE_PATINHO(m_addr), m_addr, PC, FLAGS, RC, m_mode);
		debugger_instruction_hook(PC);

		if (!m_run){
			if (!m_buttons_read_cb.isnull()){
				uint16_t buttons = m_buttons_read_cb(0);
				if (buttons & BUTTON_PARTIDA){
					/* "startup" button */
					switch (m_mode){
						case ADDRESSING_MODE: PC = RC; break;
						case NORMAL_MODE: m_run = true; break;
						case DATA_STORE_MODE: WRITE_BYTE_PATINHO(PC, RC & 0xFF); break; //TODO: we also need RE (address register, instead of using PC directly)
						/*TODO: case DATA_VIEW_MODE: RD = READ_BYTE_PATINHO(RC); break; //we need to implement RD (the 'data register') */
						default: break;
					}
				}
				if (buttons & BUTTON_NORMAL) m_mode = NORMAL_MODE;
				if (buttons & BUTTON_ENDERECAMENTO) m_mode = ADDRESSING_MODE;
				if (buttons & BUTTON_EXPOSICAO) m_mode = DATA_VIEW_MODE;
				if (buttons & BUTTON_ARMAZENAMENTO) m_mode = DATA_STORE_MODE;
				if (buttons & BUTTON_CICLO_UNICO) m_mode = CYCLE_STEP_MODE;
				if (buttons & BUTTON_INSTRUCAO_UNICA) m_mode = INSTRUCTION_STEP_MODE;
				if (buttons & BUTTON_PREPARACAO) device_reset();
			}
			m_icount = 0;   /* if processor is stopped, just burn cycles */
		} else {
			execute_instruction();
			m_icount --;
		}
	}
	while (m_icount > 0);
}

/* execute one instruction */
void patinho_feio_cpu_device::execute_instruction()
{
	bool skip;
	unsigned int tmp;
	unsigned char value, channel, function;
	m_opcode = READ_BYTE_PATINHO(PC);
	INCREMENT_PC_4K;

	if (m_scheduled_IND_bit_reset)
		m_indirect_addressing = false;

	if (m_indirect_addressing)
		m_scheduled_IND_bit_reset = true;

	switch (m_opcode){
		case 0xD2:
			//XOR: Computes the bitwise XOR of an immediate into the accumulator
			ACC ^= READ_BYTE_PATINHO(PC);
			INCREMENT_PC_4K;
			//TODO: update T and V flags
			return;
		case 0xD4:
			//NAND: Computes the bitwise XOR of an immediate into the accumulator
			ACC = ~(ACC & READ_BYTE_PATINHO(PC));
			INCREMENT_PC_4K;
			//TODO: update T and V flags
			return;
		case 0xD8:
			//SOMI="Soma Imediato":
			//     Add an immediate into the accumulator
			set_flag(V, ((((int16_t) ACC) + ((int16_t) READ_BYTE_PATINHO(PC))) >> 8));
			set_flag(T, ((((int8_t) (ACC & 0x7F)) + ((int8_t) (READ_BYTE_PATINHO(PC) & 0x7F))) >> 7) == V);
			ACC += READ_BYTE_PATINHO(PC);
			INCREMENT_PC_4K;
			return;
		case 0xDA:
			//CARI="Carrega Imediato":
			//     Load an immediate into the accumulator
			ACC = READ_BYTE_PATINHO(PC);
			INCREMENT_PC_4K;
			return;
		case 0x80:
			//LIMPO:
			//    Clear accumulator and flags
			ACC = 0;
			FLAGS = 0;
			return;
		case 0x81:
			//UM="One":
			//    Load 1 into accumulator
			//    and clear the flags
			ACC = 1;
			FLAGS = 0;
			return;
		case 0x82:
			//CMP1:
			// Compute One's complement of the accumulator
			//    and clear the flags
			ACC = ~ACC;
			FLAGS = 0;
			return;
		case 0x83:
			//CMP2:
			// Compute Two's complement of the accumulator
			//    and updates flags according to the result of the operation
			ACC = ~ACC + 1;
			FLAGS = 0; //TODO: fix-me (I'm not sure yet how to compute the flags here)
			return;
		case 0x84:
			//LIM="Limpa":
			// Clear flags
			FLAGS = 0;
			return;
		case 0x85:
			//INC:
			// Increment accumulator
			ACC++;
			FLAGS = 0; //TODO: fix-me (I'm not sure yet how to compute the flags here)
			return;
		case 0x86:
			//UNEG="Um Negativo":
			// Load -1 into accumulator and clear flags
			ACC = -1;
			FLAGS = 0;
			return;
		case 0x87:
			//LIMP1:
			//    Clear accumulator, reset T and set V
			ACC = 0;
			FLAGS = V;
			return;
		case 0x88:
			//PNL 0:
			ACC = (RC & 0xFF);
			FLAGS = 0;
			return;
		case 0x89:
			//PNL 1:
			ACC = (RC & 0xFF) + 1;
			//TODO: FLAGS = ?;
			return;
		case 0x8A:
			//PNL 2:
			ACC = (RC & 0xFF) - ACC - 1;
			//TODO: FLAGS = ?;
			return;
		case 0x8B:
			//PNL 3:
			ACC = (RC & 0xFF) - ACC;
			//TODO: FLAGS = ?;
			return;
		case 0x8C:
			//PNL 4:
			ACC = (RC & 0xFF) + ACC;
			//TODO: FLAGS = ?;
			return;
		case 0x8D:
			//PNL 5:
			ACC = (RC & 0xFF) + ACC + 1;
			//TODO: FLAGS = ?;
			return;
		case 0x8E:
			//PNL 6:
			ACC = (RC & 0xFF) - 1;
			//TODO: FLAGS = ?;
			return;
		case 0x8F:
			//PNL 7:
			ACC = (RC & 0xFF);
			FLAGS = V;
			return;
		case 0x90:
			//ST 0 = "Se T=0, Pula"
			//       If T is zero, skip the next instruction
						if ((FLAGS & T) == 0)
				INCREMENT_PC_4K; //skip
			return;
		case 0x91:
			//STM 0 = "Se T=0, Pula e muda"
			//        If T is zero, skip the next instruction
			//        and toggle T.
			if ((FLAGS & T) == 0){
				INCREMENT_PC_4K; //skip
				FLAGS |= T; //set T=1
			}
			return;
		case 0x92:
			//ST 1 = "Se T=1, Pula"
			//       If T is one, skip the next instruction
						if ((FLAGS & T) == T)
				INCREMENT_PC_4K; //skip
			return;
		case 0x93:
			//STM 1 = "Se T=1, Pula e muda"
			//        If T is one, skip the next instruction
			//        and toggle T.
			if ((FLAGS & T) == T){
				INCREMENT_PC_4K; //skip
				FLAGS &= ~T; //set T=0
			}
			return;
		case 0x94:
			//SV 0 = "Se V=0, Pula"
			//       If V is zero, skip the next instruction
						if ((FLAGS & V) == 0)
				INCREMENT_PC_4K; //skip
			return;
		case 0x95:
			//SVM 0 = "Se V=0, Pula e muda"
			//        If V is zero, skip the next instruction
			//        and toggle V.
			if ((FLAGS & V) == 0){
				INCREMENT_PC_4K; //skip
				FLAGS |= V; //set V=1
			}
			return;
		case 0x96:
			//SV 1 = "Se V=1, Pula"
			//       If V is one, skip the next instruction
						if ((FLAGS & V) == 1)
				INCREMENT_PC_4K; //skip
			return;
		case 0x97:
			//SVM 1 = "Se V=1, Pula e muda"
			//        If V is one, skip the next instruction
			//        and toggle V.
			if ((FLAGS & V) == 1){
				INCREMENT_PC_4K; //skip
				FLAGS &= ~V; //set V=0
			}
			return;
		case 0x98:
			//PUL="Pula para /002 a limpa estado de interrupcao"
			//     Jump to address /002 and disables interrupts
						PC = 0x002;
			m_interrupts_enabled = false;
			return;
		case 0x99:
			//TRE="Troca conteudos de ACC e EXT"
			//     Exchange the value of the accumulator with the ACC extension register
						value = ACC;
						ACC = READ_ACC_EXTENSION_REG();
						WRITE_ACC_EXTENSION_REG(value);
			return;
		case 0x9A:
			//INIB="Inibe"
			//     disables interrupts
			m_interrupts_enabled = false;
			return;
		case 0x9B:
			//PERM="Permite"
			//     enables interrupts
			m_interrupts_enabled = true;
			return;
		case 0x9C:
			//ESP="Espera":
			//    Holds execution and waits for an interrupt to occur.
			m_run = false;
			m_wait_for_interrupt = true;
			return;
		case 0x9D:
			//PARE="Pare":
			//    Holds execution. This can only be recovered by
			//    manually triggering execution again by
			//    pressing the "Partida" (start) button in the panel
			m_run = false;
			m_wait_for_interrupt = false;
			return;
		case 0x9E:
			//TRI="Troca com Indexador":
			//     Exchange the value of the accumulator with the index register
			value = ACC;
			ACC = READ_INDEX_REG();
			WRITE_INDEX_REG(value);
			return;
		case 0x9F:
			//IND="Enderecamento indireto":
			//     Sets memory addressing for the next instruction to be indirect.
			m_indirect_addressing = true;
			m_scheduled_IND_bit_reset = false; //the next instruction execution will schedule it.
			return;
		case 0xD1:
			//Bit-Shift/Bit-Rotate instructions
			value = READ_BYTE_PATINHO(PC);
			INCREMENT_PC_4K;
			for (int i=0; i<4; i++){
				if (value & (1<<i)){
					/* The number of shifts or rotations is determined by the
					   ammount of 1 bits in the lower 4 bits of 'value' */
					switch(value & 0xF0)
					{
						case 0x00:
							//DD="Deslocamento para a Direita"
							//    Shift right
							FLAGS &= ~V;
							if (ACC & 1)
								FLAGS |= V;

							ACC >>= 1;
							break;
						case 0x20:
							//GD="Giro para a Direita"
							//    Rotate right
							FLAGS &= ~V;
							if (ACC & 1)
								FLAGS |= V;

							ACC = ((ACC & 1) << 7) | (ACC >> 1);
							break;
						case 0x10: //DDV="Deslocamento para a Direita com Vai-um"
								//     Shift right with Carry
						case 0x30: //GDV="Giro para a Direita com Vai-um"
								//     Rotate right with Carry

							//both instructions are equivalent
							if (FLAGS & V)
								tmp = 0x100 | ACC;
							else
								tmp = ACC;

							FLAGS &= ~V;
							if (ACC & 1)
								FLAGS |= V;

							ACC = tmp >> 1;
							break;
						case 0x40: //DE="Deslocamento para a Esquerda"
								//    Shift left
							FLAGS &= ~V;
							if (ACC & (1<<7))
								FLAGS |= V;

							ACC <<= 1;
							break;
						case 0x60: //GE="Giro para a Esquerda"
								//    Rotate left
							FLAGS &= ~V;
							if (ACC & (1<<7))
								FLAGS |= V;

							ACC = (ACC << 1) | ((ACC >> 7) & 1);
							break;
						case 0x50: //DEV="Deslocamento para a Esquerda com Vai-um"
								//     Shift left with Carry
						case 0x70: //GEV="Giro para a Esquerda com Vai-um"
								//     Rotate left with Carry

							//both instructions are equivalent
							if (FLAGS & V)
								tmp = (ACC << 1) | 1;
							else
								tmp = (ACC << 1);

							FLAGS &= ~V;
							if (tmp & (1<<8))
								FLAGS |= V;

							ACC = tmp & 0xFF;
							break;
						case 0x80: //DDS="Deslocamento para a Direita com duplicacao de Sinal"
								//     Rotate right with signal duplication
							FLAGS &= ~V;
							if (ACC & 1)
								FLAGS |= V;

							ACC = (ACC & (1 << 7)) | ACC >> 1;
							break;
						default:
							printf("Illegal instruction: %02X %02X\n", m_opcode, value);
							return;
					}
				}
			}
			return;
	}

	switch (m_opcode & 0xF0){
		case 0x00:
			//PLA = "Pula": Jump to address
			compute_effective_address((m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC));
			INCREMENT_PC_4K;
			PC = m_addr;
			return;
		case 0x10:
			//PLAX = "Pula indexado": Jump to indexed address
			tmp = (m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC);
			INCREMENT_PC_4K;
			m_idx = READ_INDEX_REG();
			compute_effective_address(m_idx + tmp);
			PC = m_addr;
			return;
		case 0x20:
			//ARM = "Armazena": Store the value of the accumulator into a given memory position
			compute_effective_address((m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC));
			INCREMENT_PC_4K;
			WRITE_BYTE_PATINHO(m_addr, ACC);
			return;
		case 0x30:
			//ARMX = "Armazena indexado": Store the value of the accumulator into a given indexed memory position
			tmp = (m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC);
			INCREMENT_PC_4K;
			m_idx = READ_INDEX_REG();
			compute_effective_address(m_idx + tmp);
			WRITE_BYTE_PATINHO(m_addr, ACC);
			return;
		case 0x40:
			//CAR = "Carrega": Load a value from a given memory position into the accumulator
			compute_effective_address((m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC));
			INCREMENT_PC_4K;
			ACC = READ_BYTE_PATINHO(m_addr);
			return;
		case 0x50:
			//CARX = "Carga indexada": Load a value from a given indexed memory position into the accumulator
			tmp = (m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC);
			INCREMENT_PC_4K;
			m_idx = READ_INDEX_REG();
			compute_effective_address(m_idx + tmp);
			ACC = READ_BYTE_PATINHO(m_addr);
			return;
		case 0x60:
			//SOM = "Soma": Add a value from a given memory position into the accumulator
			compute_effective_address((m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC));
			INCREMENT_PC_4K;
			ACC += READ_BYTE_PATINHO(m_addr);
			//TODO: update V and T flags
			return;
		case 0x70:
			//SOMX = "Soma indexada": Add a value from a given indexed memory position into the accumulator
			tmp = (m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC);
			INCREMENT_PC_4K;
			m_idx = READ_INDEX_REG();
			compute_effective_address(m_idx + tmp);
			ACC += READ_BYTE_PATINHO(m_addr);
			//TODO: update V and T flags
			return;
		case 0xA0:
			//PLAN = "Pula se ACC negativo": Jump to a given address if ACC is negative
			compute_effective_address((m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC));
			INCREMENT_PC_4K;
			if ((signed char) ACC < 0)
				PC = m_addr;
			return;
		case 0xB0:
			//PLAZ = "Pula se ACC for zero": Jump to a given address if ACC is zero
			compute_effective_address((m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC));
			INCREMENT_PC_4K;
			if (ACC == 0)
				PC = m_addr;
			return;
		case 0xC0:
			//Executes I/O functions
			//TODO: Implement-me!
			value = READ_BYTE_PATINHO(PC);
			INCREMENT_PC_4K;
			channel = m_opcode & 0x0F;
			function = value & 0x0F;
			switch(value & 0xF0){
				case 0x10:
					switch(function)
					{
						case 0:
							// FNC /n0: Desliga flip-flop PERMITE/IMPEDE para
							//          o dispositivo n (isto e, impede inter-
							//          -rupcao do dispositivo n).
							//
							//          Turns off the interrupt ENABLE/DISABLE
							//          flip-flop for channel n.
							//TODO: Implement-me!
							break;
						case 1:
							// FNC /n1: Desliga flip-flop de ESTADO do dispo-
							//          -sitivo n ( ESTADO = "busy" ).
							//
							//          Turns off STATUS flip-flop for
							//          channel n ( STATUS = "busy" ).
							m_iodev_status[channel] = IODEV_BUSY;
							break;
						case 2:
							// FNC /n2: Liga flip-flop de ESTADO do dispo-
							//          -sitivo n ( ESTADO = "ready" ).
							//
							//          Turns on STATUS flip-flop for
							//          channel n ( STATUS = "ready" ).
							m_iodev_status[channel] = IODEV_READY;
							break;
						case 4:
							// FNC /n4: Desliga flip-flop de PEDIDO de inter-
							//          rupcao do  dispositivo n.
							//
							//          Turns off the interrupt REQUEST
							//          flip-flop for channel n.
							//TODO: Implement-me!
							break;
						case 5:
							// FNC /n5: Liga flip-flop PERMITE/IMPEDE para  o
							//          dispositivo n (isto e, permite inter-
							//          -rupcao do dispositivo n).
							//
							//          Turns on the interrupt ENABLE/DISABLE
							//          flip-flop for channel n.
							//TODO: Implement-me!
							break;
						case 6:
							// FNC /n6: Liga flip-flop de CONTROLE e  desliga
							//          flip-flop de ESTADO (ESTADO = "BUSY")
							//          do dispositivo n .
							//
							//          Turns on the CONTROL flip-flop and
							//          turns off the STATUS flip-flop for
							//          channel n ( STATUS = "BUSY").
							m_iodev_control[channel] = REQUEST;
							m_iodev_status[channel] = IODEV_BUSY;
							break;
						case 7:
							// FNC /n7: Desliga flip-flop de CONTROLE do dis-
							//          positivo n.
							//
							//          Turns off the CONTROL flip-flop for
							//          for channel n.
							m_iodev_control[channel] = NO_REQUEST;
							break;
						case 8:
							// FNC /n8: So funciona na leitora de fita, ca-
							//          nal /E. Ignora todos os "feed-fra-
							//          -mes" ("bytes" nulos) da fita, ate' a
							//          proxima perfuracao (1o "byte" nao
							//          nulo).
							//
							//          Only works with the punched tape reader,
							//          device on channel /E. Ignores all
							//          "feed-frames" (null 'bytes') of the tape,
							//          until the first punch (1st non-zero 'byte').
							if (channel==0xE){
								//TODO: Implement-me!
							} else {
								printf("Function 8 of the /FNC instruction can only be used with"\
										"the papertape reader device at channel /E.\n");
							}
							break;
						default:
							printf("Invalid function (#%d) specified in /FNC instruction.\n", function);
					}
					break;
				case 0x20:
					//SAL="Salta"
					//    Skips a couple bytes if a condition is met
										skip = false;
					switch(function)
					{
						case 1:
							skip = (m_iodev_status[channel] == IODEV_READY);
							break;
						case 2:
							/* TODO:
							skip = false;
							if (! m_iodev_is_ok_cb[channel].isnull()
							    && m_iodev_is_ok_cb[channel](0)) */
								skip = true;
							break;
						case 4:
							/*TODO:
							skip =false;
							if (! m_iodev_IRQ_cb[channel].isnull()
							    && m_iodev_IRQ_cb[channel](0) == true)*/
								skip = true;
							break;
					}

					if (skip){
						INCREMENT_PC_4K;
						INCREMENT_PC_4K;
					}
					break;
				case 0x40:
					/* ENTR = "Input data from I/O device" */
					ACC = m_iodev_incoming_byte[channel];
					m_iodev_control[channel] = NO_REQUEST; //TODO: <-- check if this is correct
					break;
				case 0x80:
					/* SAI = "Output data to I/O device" */
					if (m_iodev_write_cb[channel].isnull()){
						printf("Warning: There's no device hooked up at I/O address 0x%X", channel);
					} else {
						m_iodev_write_cb[channel](ACC);
					}
					break;
			}
			return;
		case 0xE0:
			//SUS = "Subtrai um ou Salta": Subtract one from the data in the given address
			//                             or, if the data is zero, then simply skip a couple bytes.
			compute_effective_address((m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC));
			INCREMENT_PC_4K;
			value = READ_BYTE_PATINHO(m_addr);
			if (value > 0){
				WRITE_BYTE_PATINHO(m_addr, value-1);
			} else {
				INCREMENT_PC_4K;
				INCREMENT_PC_4K;
			}
			return;
		case 0xF0:
			//PUG = "Pula e guarda": Jump and store.
			//      It stores the return address to addr and addr+1
			//      And then jumps to addr+2
			compute_effective_address((m_opcode & 0x0F) << 8 | READ_BYTE_PATINHO(PC));
			INCREMENT_PC_4K;
			WRITE_BYTE_PATINHO(m_addr, (PC >> 8) & 0x0F);
			WRITE_BYTE_PATINHO(m_addr+1, PC & 0xFF);
			PC = m_addr+2;
			return;
	}
	printf("unimplemented opcode: 0x%02X\n", m_opcode);
}

std::unique_ptr<util::disasm_interface> patinho_feio_cpu_device::create_disassembler()
{
	return std::make_unique<patinho_feio_disassembler>();
}

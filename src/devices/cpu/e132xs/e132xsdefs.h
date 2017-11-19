/* Memory access */
/* read byte */
#define READ_B(addr)            m_program->read_byte((addr))
/* read half-word */
#define READ_HW(addr)           m_program->read_word((addr) & ~1)
/* read word */
#define READ_W(addr)            m_program->read_dword((addr) & ~3)

/* write byte */
#define WRITE_B(addr, data)     m_program->write_byte(addr, data)
/* write half-word */
#define WRITE_HW(addr, data)    m_program->write_word((addr) & ~1, data)
/* write word */
#define WRITE_W(addr, data)     m_program->write_dword((addr) & ~3, data)


/* I/O access */
/* read word */
#define IO_READ_W(addr)         m_io->read_dword(((addr) >> 11) & 0x7ffc)
/* write word */
#define IO_WRITE_W(addr, data)  m_io->write_dword(((addr) >> 11) & 0x7ffc, data)


#define READ_OP(addr)          m_direct->read_word((addr), m_opcodexor)

// set C in adds/addsi/subs/sums
#define SETCARRYS 0
#define MISSIONCRAFT_FLAGS 1

/* Registers */

/* Internal registers */

#define SREG  decode.src_value
#define SREGF decode.next_src_value
#define DREG  decode.dst_value
#define DREGF decode.next_dst_value
#define EXTRA_U decode.extra.u
#define EXTRA_S decode.extra.s

#define SET_SREG( _data_ )  (decode.src_is_local ? set_local_register(decode.src, (uint32_t)_data_) : set_global_register(decode.src, (uint32_t)_data_))
#define SET_SREGF( _data_ ) (decode.src_is_local ? set_local_register(decode.src + 1, (uint32_t)_data_) : set_global_register(decode.src + 1, (uint32_t)_data_))
#define SET_DREG( _data_ )  (decode.dst_is_local ? set_local_register(decode.dst, (uint32_t)_data_) : set_global_register(decode.dst, (uint32_t)_data_))
#define SET_DREGF( _data_ ) (decode.dst_is_local ? set_local_register(decode.dst + 1, (uint32_t)_data_) : set_global_register(decode.dst + 1, (uint32_t)_data_))

#define SRC_IS_PC      (!decode.src_is_local && decode.src == PC_REGISTER)
#define DST_IS_PC      (!decode.dst_is_local && decode.dst == PC_REGISTER)
#define SRC_IS_SR      (!decode.src_is_local && decode.src == SR_REGISTER)
#define DST_IS_SR      (!decode.dst_is_local && decode.dst == SR_REGISTER)
#define SAME_SRC_DST   decode.same_src_dst
#define SAME_SRC_DSTF  decode.same_src_dstf
#define SAME_SRCF_DST  decode.same_srcf_dst


/* Memory access */
/* read byte */
#define READ_B(addr)            m_program->read_byte((addr))
/* read half-word */
#define READ_HW(addr)           m_program->read_word((addr) & ~1)
/* read word */
#define READ_W(addr)            m_program->read_dword((addr) & ~3)

/* write byte */
#define WRITE_B(addr, data)     m_program->write_byte(addr, data)
/* write half-word */
#define WRITE_HW(addr, data)    m_program->write_word((addr) & ~1, data)
/* write word */
#define WRITE_W(addr, data)     m_program->write_dword((addr) & ~3, data)


/* I/O access */
/* read word */
#define IO_READ_W(addr)         m_io->read_dword(((addr) >> 11) & 0x7ffc)
/* write word */
#define IO_WRITE_W(addr, data)  m_io->write_dword(((addr) >> 11) & 0x7ffc, data)


#define READ_OP(addr)          m_direct->read_word((addr), m_opcodexor)


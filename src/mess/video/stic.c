#include "emu.h"
#include "video/stic.h"
#include "includes/intv.h"

/* STIC variables */

READ16_MEMBER( intv_state::intv_stic_r )
{
//  intv_state *state = space->machine().driver_data<intv_state>();
	//logerror("%x = stic_r(%x)\n",0,offset);
	if (m_bus_copy_mode || !m_stic_handshake)
	{
	switch (offset)
	{
		case STIC_MXR + STIC_MOB0:
		case STIC_MXR + STIC_MOB1:
		case STIC_MXR + STIC_MOB2:
		case STIC_MXR + STIC_MOB3:
		case STIC_MXR + STIC_MOB4:
		case STIC_MXR + STIC_MOB5:
		case STIC_MXR + STIC_MOB6:
		case STIC_MXR + STIC_MOB7:
			return 0x3800 | (m_stic_registers[offset] & 0x07FF);
		case STIC_MYR + STIC_MOB0:
		case STIC_MYR + STIC_MOB1:
		case STIC_MYR + STIC_MOB2:
		case STIC_MYR + STIC_MOB3:
		case STIC_MYR + STIC_MOB4:
		case STIC_MYR + STIC_MOB5:
		case STIC_MYR + STIC_MOB6:
		case STIC_MYR + STIC_MOB7:
			return 0x3000 | (m_stic_registers[offset] & 0x0FFF);
		case STIC_MAR + STIC_MOB0:
		case STIC_MAR + STIC_MOB1:
		case STIC_MAR + STIC_MOB2:
		case STIC_MAR + STIC_MOB3:
		case STIC_MAR + STIC_MOB4:
		case STIC_MAR + STIC_MOB5:
		case STIC_MAR + STIC_MOB6:
		case STIC_MAR + STIC_MOB7:
			return m_stic_registers[offset] & 0x3FFF;
		case STIC_MCR + STIC_MOB0:
		case STIC_MCR + STIC_MOB1:
		case STIC_MCR + STIC_MOB2:
		case STIC_MCR + STIC_MOB3:
		case STIC_MCR + STIC_MOB4:
		case STIC_MCR + STIC_MOB5:
		case STIC_MCR + STIC_MOB6:
		case STIC_MCR + STIC_MOB7:
			return 0x3C00 | (m_stic_registers[offset] & 0x03FF);
		case STIC_GMR:
			m_color_stack_mode = 1;
			//logerror("Setting color stack mode\n");
			/*** fall through ***/
		case STIC_DER:
			return 0x3FFF;
		case STIC_CSR + STIC_CSR0:
		case STIC_CSR + STIC_CSR1:
		case STIC_CSR + STIC_CSR2:
		case STIC_CSR + STIC_CSR3:
		case STIC_BCR:
			return 0x3FF0 | (m_stic_registers[offset] & 0x000F);
		case STIC_HDR:
		case STIC_VDR:
			return 0x3FF8 | (m_stic_registers[offset] & 0x0007);
		case STIC_CBR:
			return 0x3FFC | (m_stic_registers[offset] & 0x0003);
		default:
			//logerror("unmapped read from STIC register %02X\n", offset);
			return 0x3FFF;
	}
	}
	else { return (offset); }
}

WRITE16_MEMBER( intv_state::intv_stic_w )
{
	//intv_state *state = space->machine().driver_data<intv_state>();
	intv_sprite_type *s;

	//logerror("stic_w(%x) = %x\n",offset,data);
	if (m_bus_copy_mode || !m_stic_handshake)
	{
	switch (offset)
	{
		/* X Positions */
		case STIC_MXR + STIC_MOB0:
		case STIC_MXR + STIC_MOB1:
		case STIC_MXR + STIC_MOB2:
		case STIC_MXR + STIC_MOB3:
		case STIC_MXR + STIC_MOB4:
		case STIC_MXR + STIC_MOB5:
		case STIC_MXR + STIC_MOB6:
		case STIC_MXR + STIC_MOB7:
			s =  &m_sprite[offset & (STIC_MOBS - 1)];
			s->doublex = !!(data & STIC_MXR_XSIZE);
			s->visible = !!(data & STIC_MXR_VIS);
			s->coll = !!(data & STIC_MXR_COL);
			s->xpos = (data & STIC_MXR_X);
			break;
		/* Y Positions */
		case STIC_MYR + STIC_MOB0:
		case STIC_MYR + STIC_MOB1:
		case STIC_MYR + STIC_MOB2:
		case STIC_MYR + STIC_MOB3:
		case STIC_MYR + STIC_MOB4:
		case STIC_MYR + STIC_MOB5:
		case STIC_MYR + STIC_MOB6:
		case STIC_MYR + STIC_MOB7:
			s =  &m_sprite[offset & (STIC_MOBS - 1)];
			s->yflip = !!(data & STIC_MYR_YFLIP);
			s->xflip = !!(data & STIC_MYR_XFLIP);
			s->quady = !!(data & STIC_MYR_YSIZE);
			s->doubley = !!(data & STIC_MYR_YFULL);
			s->doubleyres = !!(data & STIC_MYR_YRES);
			s->ypos = (data & STIC_MYR_Y);
			break;
		/* Attributes */
		case STIC_MAR + STIC_MOB0:
		case STIC_MAR + STIC_MOB1:
		case STIC_MAR + STIC_MOB2:
		case STIC_MAR + STIC_MOB3:
		case STIC_MAR + STIC_MOB4:
		case STIC_MAR + STIC_MOB5:
		case STIC_MAR + STIC_MOB6:
		case STIC_MAR + STIC_MOB7:
			s =  &m_sprite[offset & (STIC_MOBS - 1)];
			s->behind_foreground = !!(data & STIC_MAR_PRI);
			s->grom = !(data & STIC_MAR_SEL);
			s->card = ((data & STIC_MAR_C) >> 3);
			s->color = ((data & STIC_MAR_FG3) >> 9) | (data & STIC_MAR_FG20);
			break;
		/* Collision Detection - TBD */
		case STIC_MCR + STIC_MOB0:
		case STIC_MCR + STIC_MOB1:
		case STIC_MCR + STIC_MOB2:
		case STIC_MCR + STIC_MOB3:
		case STIC_MCR + STIC_MOB4:
		case STIC_MCR + STIC_MOB5:
		case STIC_MCR + STIC_MOB6:
		case STIC_MCR + STIC_MOB7:
			// a MOB's own collision bit is *always* zero, even if a
			// one is poked into it
			data &= ~(1 << (offset & (STIC_MOBS - 1)));
			break;
		/* Display enable */
		case STIC_DER:
			//logerror("***Writing a %x to the STIC handshake\n",data);
			m_stic_handshake = 1;
			break;
		/* Graphics Mode */
		case STIC_GMR:
			m_color_stack_mode = 0;
			break;
		/* Color Stack */
		case STIC_CSR + STIC_CSR0:
		case STIC_CSR + STIC_CSR1:
		case STIC_CSR + STIC_CSR2:
		case STIC_CSR + STIC_CSR3:
			logerror("Setting color_stack[%x] = %x (%x)\n", offset & (STIC_CSRS - 1),data & STIC_CSR_BG, cpu_get_pc(&space.device()));
			break;
		/* Border Color */
		case STIC_BCR:
			//logerror("***Writing a %x to the border color\n",data);
			m_border_color = data & STIC_BCR_BC;
			break;
		/* Framing */
		case STIC_HDR:
			m_col_delay = data & STIC_HDR_DEL;
			break;
		case STIC_VDR:
			m_row_delay = data & STIC_VDR_DEL;
			break;
		case STIC_CBR:
			m_left_edge_inhibit = (data & STIC_CBR_COL);
			m_top_edge_inhibit = (data & STIC_CBR_ROW) >> 1;
			break;
		default:
			//logerror("unmapped write to STIC register %02X: %04X\n", offset, data);
			break;
	}

	if (offset < sizeof(m_stic_registers) / sizeof(m_stic_registers[0]))
		m_stic_registers[offset] = data;
	}
}

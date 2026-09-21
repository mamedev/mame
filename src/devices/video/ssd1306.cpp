/****************************************************************************

    Solomon Systech SSD1306 OLED display driver
    "128x64 Dot Matrix OLED/PLED Segment/Common Driver with Controller"

    Datasheet:
    https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

    Graphics RAM is split into 8 pixel high "Pages" with one byte representing
    one vertical stripe of 8 pixels.

 ****************************************************************************/

#include "emu.h"
#include "ssd1306.h"

#define KEEP_LOW_NIBBLE(x)  x & 0x0F
#define KEEP_HIGH_NIBBLE(x) x & 0xF0

#define LOW4_AS_HIGH_NIBBLE(x) (x & 0x0F) << 4
#define LOW4_AS_LOW_NIBBLE(x) (x & 0x0F)

#define SET_LOW_NIBBLE_FROM_LOW4(reg, val)  reg = KEEP_HIGH_NIBBLE(reg) | LOW4_AS_LOW_NIBBLE(val);
#define SET_HIGH_NIBBLE_FROM_LOW4(reg, val) reg = KEEP_LOW_NIBBLE(reg) | LOW4_AS_HIGH_NIBBLE(val);

void ssd1306_device::device_init()
{
    // init all commandlengths to 0 (=256)
    memset(m_command_lengths, 0, sizeof(m_command_lengths));

    // 

}

void ssd1306_device::device_reset()
{

    // When RES# input is LOW, the chip is initialized with the following status:
    // 1. Display is OFF
    m_display_awake = false;
    
    // 2. 128 x 64 Display Mode
    
    // 3. Normal segment and display data column address and row address mapping (SEG0 mapped to
    // address 00h and COM0 mapped to address 00h)


    // 4. Shift register data clear in serial interface

    // 5. Display start line is set at display RAM address 0
    // 6. Column address counter is set at 0
    
    // 7. Normal scan direction of the COM outputs
    
    // 8. Contrast control register is set at 7Fh
    
    // 9. Normal display mode (Equivalent to A4h command)


    m_pagemode_column_start_address = 0;
    m_hvmode_page_start_end_address = 0x07;


    m_addressing_mode = PAGE;
}

void ssd1306_device::exec_command_2x()
{
    switch(m_command_fifo[0] & 0x0F)
    {
        case 0x0:
            m_addressing_mode = static_cast<ssd1306_addressing_mode_t>(m_command_fifo[1] & 3);
            break;

        case 0x1:
            // h/v addressing mode: set column start/end address
            break;

        case 0x2:
            // h/v addressing mode: set page start/end address
            break;

        case 0x6:
        case 0x7:
            // init right scroll (6) or left scroll (7)
            break;

        case 0x9:
        case 0xA:
            // init vertical + right scroll (9) or left scroll (A)
            break;

        case 0xE:
            // scroll disable
            break;
        
        case 0xF:
            // scroll enable
            
            break;

    }
}


void ssd1306_device::exec_command_ax()
{
    switch(m_command_fifo[0] & 0x0F)
    {
        case 0x0:
            // map column 0 to SEG0
            break;

        case 0x1:
            // map column 127 to SEG0
            break;

        case 0x3:
            // vertical scroll
            break;

        case 0x4:
            // display RAM contents
            break;

        case 0x5:
            // blank display (all pixels on???)
            break;

        case 0x6:
            m_inverting_pixels = false;
            break;

        case 0x7:
            m_inverting_pixels = true;
            break;

        case 0x8:
            // mux ratio
            break;

        case 0xE:
            m_display_awake = false;
            break;

        case 0xF:
            m_display_awake = true;
            break;

        default:
            break;
    }
}

void ssd1306_device::exec_command_dx()
{
    switch(m_command_fifo[0] & 0x0F)
    {
    }
}


void ssd1306_device::exec_command()
{
    switch(m_command_fifo[0] >> 4)
    {
        case 0x0:
            SET_LOW_NIBBLE_FROM_LOW4(m_pagemode_column_start_address, m_command_fifo[0]);
            break;

        case 0x1:
            SET_HIGH_NIBBLE_FROM_LOW4(m_pagemode_column_start_address, m_command_fifo[0]);
            break;

        case 0x2:
            exec_command_2x();
            break;
        
        case 0x4:
        case 0x5:
        case 0x6:
        case 0x7:
            m_display_start_line = m_command_fifo[0] & 0x3f;
            break;

        case 0x8:
            // $81 = contrast control
            break;
        
        case 0xA:
            exec_command_ax();
            break;

        case 0xB:
            // page addressing mode: set page start address
            if (0xB0 <= m_command_fifo[0] && m_command_fifo[0] <= 0xB7)
            {
                m_pagemode_page_start_address = m_command_fifo[0] & 7;
            }
            break;
        
        case 0xC:
            // column scan direction: $C0 normal, $C8 reverse
            break;
        
        case 0xD:
            exec_command_dx();
            break;

        default:
            if (m_command_fifo[0] == 0xE3)
            {
                // explicit NOP
                return;
            }

            logerror("%s: invalid/unimplemented command %02x\n", m_command_fifo[0]);
            break;
    }
}


void ssd1306_device::write(u8 data)
{
    if (!m_dc_line)
    {
        // incoming write is a command
        if (m_command_pointer == 0)
        {
            m_command_bytes_left = m_command_lengths[data];
        }

        m_command_fifo[m_command_pointer++] = data;
        m_command_bytes_left --;

        if (m_command_bytes_left == 0)
        {
            exec_command();
        }
    }

    // otherwise, display data is inbound

    m_command_pointer    = 0;
    m_command_bytes_left = 0;

    if (m_scroll_enable)
    {
        logerror("%s: attempt to write display data while scrolling enabled\n");
        return;
    }

    // Graphics RAM is split into 8 pixel high "Pages" with one byte representing
    // one vertical stripe of 8 pixels.
    // 
    // "Page addressing" mode = write to single page
    // 
    // "Horizontal addressing" mode = write pixels left to right,
    // top to bottom
    // 
    // "Vertical addressing" mode = write pixels top to bottom,
    // left to right
  

    m_gddram[ (m_page_address_pointer * 128) + m_column_address_pointer ] = data;

    switch(m_addressing_mode)
    {
        case PAGE:
            m_page_address_pointer ++;
            if (m_page_address_pointer > m_pagemode_column_end_address)
            {
                m_page_address_pointer = m_pagemode_column_start_address;
            }
            break;
        
        case HORIZONTAL:
            m_column_address_pointer ++;
            if (m_column_address_pointer > m_hvmode_column_end_address)
            {
                m_column_address_pointer = m_hvmode_column_start_address;

                m_page_address_pointer ++;
                if (m_page_address_pointer > m_hvmode_page_end_address)
                {
                    m_page_address_pointer = m_hvmode_page_start_address;
                }
            }
            break;
        
        case VERTICAL:
            m_page_address_pointer ++;
            if (m_page_address_pointer > m_hvmode_page_end_address)
            {
                m_page_address_pointer = m_hvmode_page_start_address;
                m_column_address_pointer ++;
                if (m_column_address_pointer > m_hvmode_column_end_address)
                {
                    m_column_address_pointer = m_hvmode_column_start_address;
                }
            }
            break;
    }

}
// license:BSD-3-Clause
// copyright-holders:Bart Eversdijk
/**********************************************************************

    P2000 High Resolution Graphics Card

**********************************************************************/


#include "emu.h"
#include "hires.h"
#include "screen.h"

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(P2000_HIRES, p2000_hires_device, "p2khires", "P2000 High Resolution Colour Graphics Card")

//-------------------------------------------------
//  MAP IO devices on hires cpu
//-------------------------------------------------
void p2000_hires_device::io(address_map &map)
{
    map.global_mask(0xff);
    
    map(0x80, 0x8f).w(FUNC(p2000_hires_device::port_808f_w));
    map(0x90, 0x9f).w(FUNC(p2000_hires_device::port_909f_w));
    map(0xa0, 0xaf).w(FUNC(p2000_hires_device::port_a0af_w));
    map(0xb0, 0xbf).w(FUNC(p2000_hires_device::port_b0bf_w));

    map(0xc0, 0xcf).w(FUNC(p2000_hires_device::port_c0cf_w));
    map(0xd0, 0xdf).w(FUNC(p2000_hires_device::port_d0df_w));
    map(0xe0, 0xef).w(FUNC(p2000_hires_device::port_e0ef_w));
    map(0xf0, 0xf3).rw(m_hirespio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

//-------------------------------------------------
//  Set hires cpu memory mapping
//-------------------------------------------------
void p2000_hires_device::mem(address_map &map)
{
    map(0x0000, 0xffff).rw(FUNC(p2000_hires_device::memory_read), FUNC(p2000_hires_device::memory_write));
}

//-------------------------------------------------
//  Hires cpu memory read can be ROM or RAM 
//-------------------------------------------------
u8 p2000_hires_device::memory_read(offs_t offset)
{
    // if port c0-cf bit 0 is set mem page 0 (0x0000-0x1fff) is addressed as rom on reading else as RAM 
    if ((offset < 0x2000) && m_hiresmem_bank0_ROM)
    {
        return m_hiresrom[offset];
    }
    return m_hiresram->read(offset);
}

//-------------------------------------------------
//  Hires cpu memory writing is always to RAM
//-------------------------------------------------
void p2000_hires_device::memory_write(offs_t offset, u8 data)
{
    m_hiresram->write(offset, data);
}

//-------------------------------------------------
//  Set daisu chain config
//-------------------------------------------------
static const z80_daisy_config hires_daisy_chain[] =
{
    { "hirespio" },
    { nullptr }
};

//-------------------------------------------------
//  ROM( p2000_hires_gos36 )
//-------------------------------------------------
ROM_START(p2000_hires_gos36)
    ROM_REGION(0x02000, "hiresrom",0) 
    ROM_LOAD("gos36.bin", 0x0000, 0x2000, CRC(279a13f8) SHA1(71bbe2275e63492747a98e1f469de126999fb617))
ROM_END
    
//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
void p2000_hires_device::device_add_mconfig(machine_config &config)
{
    LOG("Hires add mconfig\n");

	/* Basic machine hardware */
    Z80(config, m_hirescpu, 2.5_MHz_XTAL);
    m_hirescpu->set_addrmap(AS_PROGRAM, &p2000_hires_device::mem);
    m_hirescpu->set_addrmap(AS_IO, &p2000_hires_device::io);
    m_hirescpu->set_daisy_config(hires_daisy_chain);
    
    /* PIO devices */
    Z80PIO(config, m_mainpio, 2.5_MHz_XTAL);
    m_mainpio->in_pa_callback().set(FUNC(p2000_hires_device::mainpio_pa_r_cb));
    m_mainpio->out_pa_callback().set(FUNC(p2000_hires_device::mainpio_pa_w_cb));
    m_mainpio->in_pb_callback().set(FUNC(p2000_hires_device::mainpio_pb_r_cb));
    m_mainpio->out_pb_callback().set(FUNC(p2000_hires_device::mainpio_pb_w_cb));

    Z80PIO(config, m_hirespio, 2.5_MHz_XTAL);
    m_hirespio->out_int_callback().set_inputline(m_hirescpu, INPUT_LINE_IRQ0);
    m_hirespio->in_pa_callback().set(FUNC(p2000_hires_device::hirespio_pa_r_cb));
    m_hirespio->out_pa_callback().set(FUNC(p2000_hires_device::hirespio_pa_w_cb));
    m_hirespio->in_pb_callback().set(FUNC(p2000_hires_device::hirespio_pb_r_cb));
    m_hirespio->out_pb_callback().set(FUNC(p2000_hires_device::hirespio_pb_w_cb));
    
    /* internal ram */
    RAM(config, m_hiresram).set_default_size("64K");
}

//-------------------------------------------------
//  Get hires ROM
//-------------------------------------------------
const tiny_rom_entry *p2000_hires_device::device_rom_region() const
{
	return ROM_NAME( p2000_hires_gos36 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  p2000_hires_device - constructor
//-------------------------------------------------
p2000_hires_device::p2000_hires_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, P2000_HIRES, tag, owner, clock)
    , device_p2000_expansion_slot_card_interface(mconfig, *this)
        , m_hirescpu(*this, "hirescpu") 
        , m_hiresram(*this, "hiresram")
        , m_mainpio(*this, "mainpio")
        , m_hirespio(*this, "hirespio")
        , m_hiresrom(*this, "hiresrom")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void p2000_hires_device::device_start()
{
    LOG("Hires device starting\n");
    m_slot->io_space().install_write_handler(0x2c, 0x2c, write8smo_delegate(*this, FUNC(p2000_hires_device::port_2c_w)));
    m_slot->io_space().install_readwrite_handler(0x68, 0x6b, read8sm_delegate(*m_mainpio, FUNC(z80pio_device::read)), write8sm_delegate(*m_mainpio, FUNC(z80pio_device::write)));
}

/***************************************************************************
 IMPLEMENTATION
****************************************************************************
 -------------------- Hires CPU output/input ports ---------------------- 
 
   HiRES channels - P2000T (main) CPU side
   0x2c  reset Hires
 
   Status channel
   0x68  channel A data address  bits 012367 (0xCF) input bits 45 (0x30) output
   0x6A  channel A control address

   Data channel
   0x69  channel B data address     (output)
   0x6B  channel B control address  (output)

**************************************************************************** */
//-------------------------------------------------
//  Hires PIO channel A data read
//-------------------------------------------------
uint8_t p2000_hires_device::mainpio_pa_r_cb() 
{
    return (m_channel_a_data & 0xfe) | (m_hirespio->rdy_b() ? 0x1 : 0x0);
}

//-------------------------------------------------
//  Hires PIO channel A data write
//-------------------------------------------------
void p2000_hires_device::mainpio_pa_w_cb(uint8_t data) 
{
    // 00 11 00 00 (0x30) only bits 4+5 to write by P2000T rest remain as is
    m_channel_a_data = (data & 0x30) | (m_channel_a_data & ~0x30);  
    // main-PIO output is connected to hires-PIO input
    m_hirespio->strobe_a(1);
    // Clock data into Hires CPU PIO
    m_hirespio->strobe_a(0);
}

//-------------------------------------------------
//  Hires PIO channel B data read
//-------------------------------------------------
uint8_t p2000_hires_device::mainpio_pb_r_cb() 
{
    return m_channel_b_data;
}

//-------------------------------------------------
//  Hires PIO channel B data write
//-------------------------------------------------
void p2000_hires_device::mainpio_pb_w_cb(uint8_t data) 
{
    // main-PIO output is connected to hires-PIO input
    m_channel_b_data = data;
    m_hirespio->strobe_b(1);
    // Clock data into Hires CPU PIO
    m_hirespio->strobe_b(0);
}

//-------------------------------------------------
// Hires video sync emulation
// toggle bit 2 & 3 [00 00 11 00 =0xc] to emulate image syncs 
//-------------------------------------------------
void p2000_hires_device::hirespio_emulate_sync() 
{
    m_channel_a_data = (m_channel_a_data & 0x0c) ? (m_channel_a_data & ~0x0c) : (m_channel_a_data | 0x0c);
}

//-------------------------------------------------
//  Write to P2000 port 2c resets the hires CPU
//-------------------------------------------------
void p2000_hires_device::port_2c_w(uint8_t data)
{
    m_hires_lut_red_cnt = 0;
    m_hires_lut_blue_cnt = 0;
    m_hires_lut_green_cnt = 0;
    m_hirescpu->reset();
}

/* -------------------------------------------------
   HiRES channels - Hires CPU side
  
        80-8f       Red color table
        90-9f       Green color table
        a0-af       Red color table
        b0-bf       RGB-P2000T image select
        c0-cf       Memory map
        d0-df       Scroll register
        e0-ef       Mode register

    Status channel
        0xf0  channel A data address  bits 012367 (0xCF) output bits 45 (0x30) input
        0xf2  channel A control address

    Data channel
        0xf1  channel B data address    
        0xf3  channel B control address 
  
------------------------------------------------- */
//-------------------------------------------------
//  Hires port 80-8f sets red color table
//-------------------------------------------------
void p2000_hires_device::port_808f_w(uint8_t data) 
{
    m_hires_lut_red[m_hires_lut_red_cnt] = data << 4 | data; // Converting 4 bits colors to 8 bit colors
    m_hires_lut_red_cnt = (m_hires_lut_red_cnt + 1) % LUT_TABLE_SIZE;
}

//-------------------------------------------------
//  Hires port 90-9f sets green color table
//-------------------------------------------------
void p2000_hires_device::port_909f_w(uint8_t data) 
{
    m_hires_lut_green[m_hires_lut_green_cnt] = data << 4 | data; // Converting 4 bits colors to 8 bit colors
    m_hires_lut_green_cnt = (m_hires_lut_green_cnt + 1) % LUT_TABLE_SIZE;
}

//-------------------------------------------------
//  Hires port a0-af sets blue color table
//-------------------------------------------------
void p2000_hires_device::port_a0af_w(uint8_t data) 
{
    m_hires_lut_blue[m_hires_lut_blue_cnt] = data << 4 | data; // Converting 4 bits colors to 8 bit colors
    m_hires_lut_blue_cnt = (m_hires_lut_blue_cnt + 1) % LUT_TABLE_SIZE;
}

//-------------------------------------------------
//  Hires port b0-bf RGB-P2000T image select
//-------------------------------------------------
void p2000_hires_device::port_b0bf_w(uint8_t data) 
{
    m_hires_image_select = data & 0x0F; 
}

//-------------------------------------------------
//  Hires port c0-cf memory map select
//-------------------------------------------------
void p2000_hires_device::port_c0cf_w(uint8_t data) 
{
    m_hiresmem_bank0_ROM = (data & 0x1) ? false : true; 
}

//-------------------------------------------------
//  Hires port d0-df image scroll register
//-------------------------------------------------
void p2000_hires_device::port_d0df_w(uint8_t data) 
{
    m_hires_scroll_reg = data; 
}

//-------------------------------------------------
//  Hires port e0-ef mode register
//-------------------------------------------------
void p2000_hires_device::port_e0ef_w(uint8_t data) 
{
    m_hires_image_mode = data; 
}

/* -------------------------------------------------
    Status channel
        0xf0  channel A data address  
                bits 012367 (0xCF) output 
                bits 45 (0x30) input
        0xf2  channel A control address

    Data channel
        0xf1  channel B data address    
        0xf3  channel B control address 
------------------------------------------------- */
//-------------------------------------------------
//  P2000 PIO channel A data read
//-------------------------------------------------
uint8_t p2000_hires_device::hirespio_pa_r_cb() 
{
    return m_channel_a_data;
}

//-------------------------------------------------
//  P2000 PIO channel A data write
//-------------------------------------------------
void p2000_hires_device::hirespio_pa_w_cb(uint8_t data) 
{
    // 11 00 00 11 (0xc3) only bits 0,1,6,7 to write by hires rest remain as is
    m_channel_a_data = (data & 0xC3) | (m_channel_a_data & ~0xC3);  
    // hires-PIO output is connected to main-PIO input
    m_mainpio->strobe_a(1);
    // Clock data into MAIN CPU PIO
    m_mainpio->strobe_a(0);
}

//-------------------------------------------------
//  P2000 PIO channel B data read
//-------------------------------------------------
uint8_t p2000_hires_device::hirespio_pb_r_cb() 
{
    return m_channel_b_data;
}

//-------------------------------------------------
//  P2000 PIO channel B data write
//-------------------------------------------------
void p2000_hires_device::hirespio_pb_w_cb(uint8_t data) 
{
    // hires-PIO output is connected to main-PIO input
    m_channel_b_data = data;
    m_mainpio->strobe_b(1);
    // Clock data into MAIN CPU PIO
    m_mainpio->strobe_b(0);
}

// Hires image select register bit definition (port b0 - bf)
#define HIRES_IMAGE_RED_ENALBE_BIT 0
#define HIRES_IMAGE_GREEN_ENABLE_BIT 1
#define HIRES_IMAGE_BLUE_ENABLE_BIT 2
#define HIRES_IMAGE_P2000T_ENABLE_BIT 3

// Hires mode  register bit definition (port e0 - ef)
#define HIRES_MODE_PAGE_4_BIT 0
#define HIRES_MODE_PAGE_5_BIT 1
#define HIRES_MODE_PAGE_6_BIT 2
#define HIRES_MODE_PAGE_7_BIT 3
#define HIRES_MODE_UP_DOWN_BIT 4
#define HIRES_MODE_512_BIT 5
#define HIRES_MODE_1_ON_1_BIT 6
#define HIRES_MODE_RESSH_BIT 7

//-------------------------------------------------
//  Hires pin P2000 video on line implementation
//-------------------------------------------------
uint8_t p2000_hires_device::vidon_r()
{
    // m_hires_image_select, when set
    // bit  3: P2000T video channel visible
    return BIT(m_hires_image_select, HIRES_IMAGE_P2000T_ENABLE_BIT) ? 1 : 0;
}

//-------------------------------------------------
//  Hires image generation
//-------------------------------------------------
uint32_t p2000_hires_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
    float PIX_TRANS_X = (480 / 25.6);
    float PIX_TRANS_Y = (480 / 25.6);
    float PIX_TRANS_X_OFSET = 0;
    int PIX_TRANS_X_WIDTH = 0;

    hirespio_emulate_sync();
    /*
        m_hires_image_select, when set
        bit  0: Red channel visible
        bit  1: GREEN channel visible
        bit  2: BLUE channel visible
        bit  3: P2000T channel visible
    */
    if (BIT(m_hires_image_select, HIRES_IMAGE_RED_ENALBE_BIT) || 
        BIT(m_hires_image_select, HIRES_IMAGE_BLUE_ENABLE_BIT) || 
        BIT(m_hires_image_select, HIRES_IMAGE_GREEN_ENABLE_BIT)) 
    {
        uint8_t  pixel_byte0, pixel_byte1, pixel_byte2, pixel_byte3;
        int ofs_byte0, ofs_byte1, ofs_byte2, ofs_byte3;

        if (BIT(m_hires_image_mode, HIRES_MODE_512_BIT)) 
        {
            // Set tranlate parameters for 512 pixels per line
            if (BIT(m_hires_image_mode, HIRES_MODE_1_ON_1_BIT)) 
            {
                PIX_TRANS_X = ((m_slot->mode80_r() ? 800 : 400) / 51.2);
                PIX_TRANS_X_OFSET = (m_slot->mode80_r() ? 60 : 30);
                if (!BIT(m_hires_image_select, HIRES_IMAGE_P2000T_ENABLE_BIT))
                {
                    // Clear left and right columns when P2000- images is off and we are in 1 x 1  mode
                    bitmap.fill(rgb_t::black(), cliprect);
                }
            } 
            else 
            {
                PIX_TRANS_X = ((m_slot->mode80_r() ? 960 : 480) / 51.2);
                PIX_TRANS_X_OFSET = 0;
            }
            PIX_TRANS_X_WIDTH = (m_slot->mode80_r() ? 2 : 1);

            ofs_byte0 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_4_BIT) ? 4 : 0) * 0x2000;
            ofs_byte1 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_5_BIT) ? 6 : 2) * 0x2000;
            ofs_byte2 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_4_BIT) ? 5 : 1) * 0x2000;
            ofs_byte3 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_5_BIT) ? 7 : 3) * 0x2000;
        } 
        else 
        {
            // Set tranlate parameters for 256 pixels per line
            if (BIT(m_hires_image_mode, HIRES_MODE_1_ON_1_BIT)) 
            {
                PIX_TRANS_X = ((m_slot->mode80_r() ? 800 : 400) / 25.6);
                PIX_TRANS_X_OFSET = (m_slot->mode80_r() ? 60 : 30);
                if (!BIT(m_hires_image_select, HIRES_IMAGE_P2000T_ENABLE_BIT))
                {
                    // Clear left and right columns when P2000- images is off and we are in 1 x 1  mode
                    bitmap.fill(rgb_t::black(), cliprect);
                }
            }
            else
            {
                PIX_TRANS_X = ((m_slot->mode80_r() ? 960 : 480) / 25.6);
                PIX_TRANS_X_OFSET = 0;
            }
            PIX_TRANS_X_WIDTH = (m_slot->mode80_r() ? 4 : 2);

            ofs_byte0 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_4_BIT) ? 4 : 0) * 0x2000;
            ofs_byte1 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_5_BIT) ? 5 : 1) * 0x2000;
            ofs_byte2 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_6_BIT) ? 6 : 2) * 0x2000;
            ofs_byte3 = (BIT(m_hires_image_mode, HIRES_MODE_PAGE_7_BIT) ? 7 : 3) * 0x2000;
        }
        
        int pixel = 0; 
        uint32_t color = 0;
        int ypos = 0;
        for (int yposcnt = 0; yposcnt < 256; yposcnt++) 
        {
            // Y-lines are stored reversed in memory also take scroll reg into account
            ypos = (m_hires_scroll_reg + yposcnt) % 256;
            // Bit 4 of image mode toggles up-side down
            if (!BIT(m_hires_image_mode, HIRES_MODE_UP_DOWN_BIT)) 
            {
                ypos = 256 - ypos;
            }
            
            if (BIT(m_hires_image_mode, HIRES_MODE_512_BIT)) 
            {
                // We are in 512 pixels per line
                for (int xpos = 0; xpos < (512 / 16); xpos++) 
                {
                    // Read per byte (representing 2 times 8 pixels of 2 bits)
                    pixel_byte0 = m_hiresram->read(ofs_byte1 + (ypos * 32) + xpos);
                    pixel_byte1 = m_hiresram->read(ofs_byte0 + (ypos * 32) + xpos);

                    pixel_byte2 = m_hiresram->read(ofs_byte3 + (ypos * 32) + xpos);
                    pixel_byte3 = m_hiresram->read(ofs_byte2 + (ypos * 32) + xpos);
                    for (int xposb = 0; xposb < 8; xposb++) 
                    {
                        // if ressh bit is set (bit 7) a black hires image is generated 
                        if (!BIT(m_hires_image_mode, HIRES_MODE_RESSH_BIT) ) 
                        {
                            // Each video line has 512 pixels (so 16 bit * 32 bytes)
                            // Per pixel use 1 bit of the 2 video pages combined pages as 0-1, 3-4, 5-6, 7-8
                            // In 512 mode the color LUTs are  0=0,1,2,4,5 1=2,3,6,7 2=8,9,c,d 3=a,b,e,f
                            pixel = (BIT(pixel_byte0, xposb)) << 3 |    (BIT(pixel_byte1, xposb)) << 1;
                            color = rgb_t(
                                        BIT(m_hires_image_select, 0) ? m_hires_lut_red[pixel] : 0, 
                                        BIT(m_hires_image_select, 1) ? m_hires_lut_green[pixel] : 0,    
                                        BIT(m_hires_image_select, 2) ? m_hires_lut_blue[pixel] : 0
                                    );
                            pixel = (BIT(pixel_byte2, xposb)) << 3 |    (BIT(pixel_byte3, xposb)) << 1;
                            // Scale one pixel in 512*256 grid to multiple pixels in 480*480 grid (so we loose some pixels)
                            screen_update_draw_pixel(bitmap, 
                                                     (PIX_TRANS_Y * yposcnt) / 10, 
                                                   ((PIX_TRANS_X * ((xpos * 16) + (15-(xposb * 2)))) / 10) + PIX_TRANS_X_OFSET, 
                                                    color, 2, PIX_TRANS_X_WIDTH);

                            color = rgb_t(
                                        BIT(m_hires_image_select, 0) ? m_hires_lut_red[pixel] : 0, 
                                        BIT(m_hires_image_select, 1) ? m_hires_lut_green[pixel] : 0,    
                                        BIT(m_hires_image_select, 2) ? m_hires_lut_blue[pixel]  : 0
                                    );
                            // Scale one pixel in 512*256 grid to multiple pixels in 480*480 grid (so we loose some pixels)
                            screen_update_draw_pixel(bitmap, 
                                                      (PIX_TRANS_Y * yposcnt) / 10,
                                                    ((PIX_TRANS_X * ((xpos * 16) + (16-(xposb * 2)))) / 10) + PIX_TRANS_X_OFSET, 
                                                      color, 2, PIX_TRANS_X_WIDTH);
                        }
                        else
                        {
                            // Scale one pixel in 512*256 grid to multiple pixels in 480*480 grid
                            screen_update_draw_pixel(bitmap, 
                                                     (PIX_TRANS_Y * yposcnt) / 10, 
                                                    ((PIX_TRANS_X * ((xpos * 8) + (16-xposb))) / 10) + PIX_TRANS_X_OFSET,
                                                     rgb_t::black(), 2, PIX_TRANS_X_WIDTH * 2);
                        }
                    }
                }
            } 
            else
            {
                for (int xpos = 0; xpos < (256 / 8); xpos++) 
                {
                    // Read per byte (representing 8 pixels of 8 bit color)
                    pixel_byte0 = m_hiresram->read(ofs_byte0 + (ypos * 32) + xpos);
                    pixel_byte1 = m_hiresram->read(ofs_byte1 + (ypos * 32) + xpos);
                    pixel_byte2 = m_hiresram->read(ofs_byte2 + (ypos * 32) + xpos);
                    pixel_byte3 = m_hiresram->read(ofs_byte3 + (ypos * 32) + xpos);
                    for (int xposb = 0; xposb < 8; xposb++) 
                    {
                        // if ressh bit is set (bit 7) a black hires image is generated 
                        if (!BIT(m_hires_image_mode, HIRES_MODE_RESSH_BIT) ) 
                        {
                            // Each video line has 256 pixels (so 8 bit * 32 bytes)
                            // Per pixel use 1 bit of the 4 video pages
                            pixel = (BIT(pixel_byte3, xposb)) << 3 |
                                    (BIT(pixel_byte2, xposb)) << 2 |
                                    (BIT(pixel_byte1, xposb)) << 1 |
                                    (BIT(pixel_byte0, xposb));
                            color = rgb_t(
                                        BIT(m_hires_image_select, 0) ? m_hires_lut_red[pixel] : 0, 
                                        BIT(m_hires_image_select, 1) ? m_hires_lut_green[pixel] : 0,    
                                        BIT(m_hires_image_select, 2) ? m_hires_lut_blue[pixel] : 0
                                    );
                        }
                        else
                        {
                            color =  rgb_t::black();
                        }
                        // Scale one pixel in 256*256 grid to multiple pixels in 480*480 grid
                        screen_update_draw_pixel(bitmap, 
                                         (PIX_TRANS_Y * yposcnt) / 10, 
                                        ((PIX_TRANS_X * ((xpos * 8) + (8-xposb))) / 10) + PIX_TRANS_X_OFSET,
                                         color, 2, PIX_TRANS_X_WIDTH);
                    }
                }
            }
        }
    }
    else
    {
        if (!BIT(m_hires_image_select, HIRES_IMAGE_P2000T_ENABLE_BIT))
        {
            // Clear image when all channels are offf
            bitmap.fill(rgb_t::black(), cliprect);
        }
    }
    
    return 0;
}

//-------------------------------------------------
//  Hires pixel generation
//-------------------------------------------------
void p2000_hires_device::screen_update_draw_pixel(bitmap_rgb32 &bitmap, int ypos, int xpos, uint32_t color, int ylen, int xlen )
{
    for (int absypos = ypos; absypos < ylen + ypos; absypos++ ) 
    {
        for (int absxpos = xpos; absxpos < xlen + xpos; absxpos++ ) 
        {
            if (absypos < 480 && absxpos < 960 ) 
            {
                // Do not overwrite P2000T image pixel - if P2000 video is on
                if (!BIT(m_hires_image_select, HIRES_IMAGE_P2000T_ENABLE_BIT) || (bitmap.pix(absypos, absxpos) & 0x00ffffff) == 0) 
                {
                    bitmap.pix(absypos, absxpos) = color;
                }
            }
        }
    }
}


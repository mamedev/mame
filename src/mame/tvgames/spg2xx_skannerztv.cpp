// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, David Haywood

// Skannerz TV

/* I/O related notes from Tahg

Summary:
SEND(from console)              RECEIVE (to console)
--- Load monsters / items
1                               2
128 + 0                         M/I[0] HIBYTE, LOBYTE
...                             ...
128 + 99                        M/I[99] HIBYTE, LOBYTE
--- Save monsters / items
4                               5
128 + 0, M/I[0] HIBYTE, LOBYTE  6
...                             ...
128 + 99, M/I[99] HIBYTE, LOBYTE 6
--- Buttons
3                               BHI, BLO

Buttons
BLO:
Bit 0     Up
Bit 1     Down
Bit 2     2/B
Bit 3     1/A
Bit 4     Left
Bit 5     Right
Bit 6     Unused
Bit 7     Unused
BHI:      Unused

Port A
Bit 0     Enable "Left" Controller
Bit 1     Enable "Right" Controller

Player
0:
8:
B:
D:
E:
65:  monsters
B5:  items

doPlayer(player)
     var_3 = player - 1
     pData = Player[var_3]
     switch(PlayerState[var_3])
          case 0: // Request controller -> console state
               r1 = 79
               while(--r1 >= 0)
               PlayerState[var_3] = 1
               P_UART_Ctrl |= TxEn
               P_UART_Ctrl |= TxIntEn
               while(P_UART_Status & TXBUSY)
               P_UART_Ctrl &= ~RxEn
               P_UART_TxBuf = 1
               WaitCycle[var_3] = 6
               CurMI[var_3] = 0
               break
          case 1: // Receive controller -> console state (expects 2)
               if(Uartp != Uartg)
                    byte_1241 = UARTBuffer[Uartg]
                    if(byte_1241 == 2)
                         PlayerState[var_3] = 2
                         InitPlayer(var_3)
                    Uartg = Uartg == 7 ? 0 : Uartg + 1
               break
          case 2: // Sync monster / item
               if(CurMI[var_3] != 100)
                    var_6 = CurMI[var_3] | 80h
                    while(P_UART_Status & TXBUSY)
                    P_UART_Ctrl &= ~RxEn
                    P_UART_TxBuf = var_6
                    PlayerState[var_3] = 3
                    WaitCycle[var_3] = 6
                    Retries[var_3] = 3
                    CurRecByte[var_3] = 0
               else if(player == 1 && byte_1238 & 100h)
                    PlayerState[var_3] = 6
                    byte_1238 &= ~100h
               else if(player == 2 && byte_1238 & 200h)
                    PlayerState[var_3] = 6
                    byte_1238 &= ~200h
               else
                    PlayerState[var_3] = 4
                    if(player == 1)
                         P_IOA_Buffer |= 1
                         byte_1238 |= 11
                    else if(player == 2)
                         P_IOA_Buffer |= 2
                         byte_1238 |= 22
               break
          case 3: // Receive monster / item
               if(Uartp != Uartg)
                    r4 = CurMI[var_3]
                    var_A = UARTBuffer[Uartg]
                    if(CurRecByte[var_3] == 0)
                         if(r4 > 79)
                              pData.items[r4-80] = var_A << 8
                         else pData.monsters[r4] = var_A << 8
                         CurRecByte[var_3] += 1
                    else if(CurRecByte[var_3] == 1)
                         if(r4 > 79)
                              pData.items[r4-80] |= var_A
                         else pData.monsters[r4] |= var_A
                         PlayerState[var_3] = 2
                         CurMI[var_3] += 1
                    Uartg = Uartg == 7 ? 0 : Uartg + 1
               break
          case 4:
               if(player == 1)
                    if((byte_1238 & 4) == 0)
                         byte_1238 |= 4
                         r1 = 79
                         while(--r1 >= 0)
                         while(P_UART_Status & TXBUSY)
                    else break
               else if(player == 2)
                    if((byte_1238 & 8) == 0)
                         byte_1238 |= 8
                         r1 = 79
                         while(--r1 >= 0)
                         while(P_UART_Status & TXBUSY)
                         break
                    else break
               else break
               P_UART_Ctrl &= ~RxEn
               P_UART_TxBuf = 3
               PlayerState[var_3] = 5
               WaitCycle[var_3] = 6
               Retries[var_3] = 3
               CurRecByte[var_3] = 0
               break
          case 5:
               if(Uartp != Uartg)
                    var_4 = CurRecByte[var_3]
                    if(var_4 == 0)
                         byte_1242[var_3] = UARTBuffer[Uartg] & 3fh
                         CurRecByte[var_3] += 1
                    else if(var_4 == 1)
                         byte_1234[var_3] = UARTBuffer[Uartg] & 3fh
                         PlayerState = 2
                         WaitCycle[var_3] = 0
                         if(player == 1)
                              P_IOA_Buffer |= 1
                              byte_1238 &= ~4
                         else if(player == 2)
                              P_IOA_Buffer |= 2
                              byte_1238 &= ~8
                    Uartg = Uartg == 7 ? 0 : Uartg + 1
               break:
          case 6: // Request console -> controller state
               r1 = 79
               while(--r1 >= 0)
               while(P_UART_Status & TXBUSY)
               P_UART_Ctrl &= ~RxEn
               P_UART_TxBuf = 4
               PlayerState[var_3] = 7
               WaitCycle[var_3] = 6
               CurMI[var_3] = 0
               Retries[var_3] = 3
          case 7: // Receive console -> controller state (expects 5)
               if(Uartp != Uartg)
                    if(UARTBuffer[Uartg] == 5)
                         PlayerState = 8
                         WaitCycle[var_3] = 6
                         CurSendByte[var_3] = 0
                         Retries[var_3] = 3
                    Uartg = Uartg == 7 ? 0 : Uartg + 1
               break
          case 8: // Send monster / item
               r4 = CurMI[var_3]
               if( r4 <= 99)
                    var_9 = CurSendByte[var_3]
                    var_8 = 0
                    if((P_UART_Status & TXBUSY) == 0)
                         var_8 = 1
                    if(var_8)
                         if(var_9 == 0)
                              var_1 = r4 | 80h
                              while(P_UART_Status & TXBUSY)
                              P_UART_Ctrl &= ~RxEn
                              P_UART_TxBuf = var_1
                              WaitCycle[var_3] = 6
                              CurSendByte[var_3] += 1
                         else if(var_9 == 1)
                              if (r4 <= 79)
                                   var_1 = (pData.monsters[r4] & 0xFF00) >> 8
                                   while(P_UART_Status & TXBUSY)
                                   P_UART_Ctrl &= ~RxEn
                                   P_UART_TxBuf = var_1
                              else
                                   var_1 = (pData.monsters[r4-80] & 0xFF00) >> 8
                                   while(P_UART_Status & TXBUSY)
                                   P_UART_Ctrl &= ~RxEn
                                   P_UART_TxBuf = var_1
                              WaitCycle[var_3] = 6
                              CurSendByte[var_3] += 1
                         else if(var_9 == 2)
                              if (r4 <= 79)
                                   var_1 = pData.monsters[r4] & 0xFF
                                   while(P_UART_Status & TXBUSY)
                                   P_UART_Ctrl &= ~RxEn
                                   P_UART_TxBuf = var_1
                              else
                                   var_1 = pData.monsters[r4-80] & 0xFF
                                   while(P_UART_Status & TXBUSY)
                                   P_UART_Ctrl &= ~RxEn
                                   P_UART_TxBuf = var_1
                              WaitCycle[var_3] = 6
                              CurSendByte[var_3] += 1
                              PlayerState[var_3] = 9
               else
                    PlayerState = 4
                    if(player == 1)
                         P_IOA_Buffer |= 1
                         byte_1238 |= 4
                    else if(player == 2)
                         P_IOA_Buffer |= 2
                         byte_1238 |= 8
               break
          case 9: // Receive monster / item sent state (expects 6)
               if(Uartp != Uartg)
                    if(UARTBuffer[Uartg] == 6)
                         PlayerState[var_3] = 8
                         WaitCycle[var_3] = 0
                         CurSendByte[var_3] = 0
                         CurMI[var_3] += 1
                         Retries[var_3] = 3
               break
     doPlayerMore(player)

doPlayerMore(player)
     var_2 = WaitCycle[player-1]
     if(var_2 == 0) return
     var_2--
     WaitCycle[player-1] = var_2
     if(var_2 != 0) return
     switch(PlayerState[player-1])
          case 3:
               if(Retries[player-1]--)
                    var_3 = CurMI[player-1] | 80h
                    while(P_UART_Status & TXBUSY)
                    P_UART_Ctrl = var_1 = P_UART_Ctrl & ~RxEn
                    P_UART_TxBuf = var_3
                    WaitCycle[player-1] = 6
                    CurRecByte[player-1] = 0
                    return
               break
          case 5:
               if(Retries[player-1]--)
                    while(P_UART_Status & TXBUSY)
                    P_UART_Ctrl = var_1 = P_UART_Ctrl & ~RxEn
                    P_UART_TxBuf = 3
                    WaitCycle[player-1] = 6
                    CurRecByte[player-1] = 0
                    return
               break
          case 7:
               if(Retries[player-1]--)
                    while(P_UART_Status & TXBUSY)
                    P_UART_Ctrl = var_1 = P_UART_Ctrl & ~RxEn
                    P_UART_TxBuf = 4
                    WaitCycle[player-1] = 6
                    return
               break
          case 9:
               if(Retries[player-1]--)
                    PlayerState = 8
                    CurSendByte[player] = 0
               else
                    ClearPlayer(player)
               return
          case 1:
               ClearPlayer(player)
               return
     ClearPlayer(player)
     if(player == 1)
          byte_1238 |= 40h
          byte_1238 &= ~10h
     else if(player == 2)
          byte_1238 |= 80h
          byte_1238 &= ~20h

*/

#include "emu.h"
#include "spg2xx.h"
#include "machine/nvram.h"

class skannerztv_state : public spg2xx_game_state
{
public:
	skannerztv_state(const machine_config &mconfig, device_type type, const char *tag) :
		spg2xx_game_state(mconfig, type, tag)
	{ }

	void rad_sktv(machine_config& config);

private:
};

static INPUT_PORTS_START( rad_sktv )
	/* how does the Scanner connect? probably some serial port with comms protocol, not IO ports?
	   internal test mode shows 'uart' ports (which currently fail)

	   To access internal test hold DOWN and BUTTON1 together on startup until a coloured screen appears.
	   To cycle through the tests again hold DOWN and press BUTTON1 */

	PORT_START("P1")
	PORT_DIPNAME( 0x0001, 0x0001, "IN0" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P2")
	PORT_DIPNAME( 0x0001, 0x0001, "IN1" )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("P3")
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void skannerztv_state::rad_sktv(machine_config &config)
{
	SPG24X(config, m_maincpu, XTAL(27'000'000), m_screen);
	m_maincpu->set_addrmap(AS_PROGRAM, &skannerztv_state::mem_map_4m);

	spg2xx_base(config);

	m_maincpu->porta_in().set(FUNC(skannerztv_state::base_porta_r));
	m_maincpu->portb_in().set(FUNC(skannerztv_state::base_portb_r));
	m_maincpu->portc_in().set(FUNC(skannerztv_state::base_portc_r));
	//m_maincpu->i2c_w().set(FUNC(skannerztv_state::i2c_w));
	//m_maincpu->i2c_r().set(FUNC(skannerztv_state::i2c_r));
}


ROM_START( rad_sktv )
	ROM_REGION( 0x800000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "skannerztv.bin", 0x000000, 0x200000, CRC(e92278e3) SHA1(eb6bee5e661128d83784960dfff50379c36bfaeb) )

	/* The external scanner MCU is a Winbond from 2000: SA5641
	   the scanner plays sound effects when scanning, without being connected to the main unit, so a way to dump / emulate
	   this MCU is also needed for complete emulation

	   TODO: find details on MCU so that we know capacity etc. */
ROM_END

CONS( 2007, rad_sktv,  0,        0, rad_sktv, rad_sktv,   skannerztv_state, init_crc, "Radica", "Skannerz TV",                 MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING )

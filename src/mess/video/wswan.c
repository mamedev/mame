/***************************************************************************

  wswan.c

  File to handle video emulation of the Bandai WonderSwan.

  Anthony Kruize
  Wilbert Pol

***************************************************************************/

#include "includes/wswan.h"

void wswan_state::wswan_setup_palettes()
{
	int i,j;

	if ( m_vdp.color_mode ) {
		for( i = 0; i < 16; i++ ) {
			for( j = 0; j < 16; j++ ) {
				m_pal[i][j] = ( ( m_vdp.palette_vram[ ( i << 5 ) + j*2 + 1 ] << 8 ) | m_vdp.palette_vram[ ( i << 5 ) + j*2 ] ) & 0x0FFF;
			}
		}
	} else {
		for( i = 0; i < 16; i++ ) {
			m_pal[i][0] = m_ws_portram[ 0x20 + ( i << 1 ) ] & 0x07;
			m_pal[i][1] = ( m_ws_portram[ 0x20 + ( i << 1 ) ] >> 4 ) & 0x07;
			m_pal[i][2] = m_ws_portram[ 0x21 + ( i << 1 ) ] & 0x07;
			m_pal[i][3] = ( m_ws_portram[ 0x21 + ( i << 1 ) ] >> 4 ) & 0x07;
		}
	}
}

void wswan_state::wswan_draw_background()
{
	UINT16  map_addr;
	UINT8   start_column;
	int column;

	map_addr = m_vdp.layer_bg_address + ( ( ( m_vdp.current_line + m_vdp.layer_bg_scroll_y ) & 0xF8 ) << 3 );
	start_column = ( m_vdp.layer_bg_scroll_x >> 3 );
	for( column = 0; column < 29; column++ ) {
		int tile_data, tile_number, tile_palette, tile_line, tile_address;
		UINT32  plane0=0, plane1=0, plane2=0, plane3=0;
		int x, x_offset;

		tile_data = ( m_vdp.vram[ map_addr + ( ( ( start_column + column ) & 0x1F ) << 1 ) + 1 ] << 8 )
					| m_vdp.vram[ map_addr + ( ( ( start_column + column ) & 0x1F ) << 1 ) ];
		tile_number = tile_data & 0x01FF;
		tile_palette = ( tile_data >> 9 ) & 0x0F;

		tile_line = ( m_vdp.current_line + m_vdp.layer_bg_scroll_y ) & 0x07;
		if ( tile_data & 0x8000 ) {
			tile_line = 7 - tile_line;
		}

		if ( m_vdp.colors_16 ) {
			tile_address = ( ( tile_data & 0x2000 ) ? 0x8000 : 0x4000 ) + ( tile_number * 32 ) + ( tile_line << 2 );
			if ( m_vdp.tile_packed ) {
				plane0 = ( m_vdp.vram[ tile_address + 0 ] << 24 ) | ( m_vdp.vram[ tile_address + 1 ] << 16 ) | ( m_vdp.vram[ tile_address + 2 ] << 8 ) | m_vdp.vram[ tile_address + 3 ];
			} else {
				plane0 = m_vdp.vram[ tile_address + 0 ];
				plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
				plane2 = m_vdp.vram[ tile_address + 2 ] << 2;
				plane3 = m_vdp.vram[ tile_address + 3 ] << 3;
			}
		} else {
			tile_address = 0x2000 + ( tile_number * 16 ) + ( tile_line << 1 );
			if ( m_vdp.tile_packed ) {
				plane0 = ( m_vdp.vram[ tile_address + 0 ] << 8 ) | m_vdp.vram[ tile_address + 1 ];
			} else {
				plane0 = m_vdp.vram[ tile_address + 0 ];
				plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
				plane2 = 0;
				plane3 = 0;
			}
		}

		for( x = 0; x < 8; x++ ) {
			int col;
			if ( m_vdp.tile_packed ) {
				if ( m_vdp.colors_16 ) {
					col = plane0 & 0x0F;
					plane0 = plane0 >> 4;
				} else {
					col = plane0 & 0x03;
					plane0 = plane0 >> 2;
				}
			} else {
				col = ( plane3 & 8 ) | ( plane2 & 4 ) | ( plane1 & 2 ) | ( plane0 & 1 );
				plane3 = plane3 >> 1;
				plane2 = plane2 >> 1;
				plane1 = plane1 >> 1;
				plane0 = plane0 >> 1;
			}
			if ( tile_data & 0x4000 ) {
				x_offset = x + ( column << 3 ) - ( m_vdp.layer_bg_scroll_x & 0x07 );
			} else {
				x_offset = 7 - x + ( column << 3 ) - ( m_vdp.layer_bg_scroll_x & 0x07 );
			}
			if ( x_offset >= 0 && x_offset < WSWAN_X_PIXELS ) {
				if ( m_vdp.colors_16 ) {
					if ( col ) {
						if ( m_vdp.color_mode ) {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						} else {
							/* Hmmmm, what should we do here... Is this correct?? */
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						}
					}
				} else {
					if ( col || !(tile_palette & 4 ) ) {
						if ( m_vdp.color_mode ) {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						} else {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_vdp.main_palette[m_pal[tile_palette][col]];
						}
					}
				}
			}
		}
	}
}

void wswan_state::wswan_draw_foreground_0()
{
	UINT16  map_addr;
	UINT8   start_column;
	int column;
	map_addr = m_vdp.layer_fg_address + ( ( ( m_vdp.current_line + m_vdp.layer_fg_scroll_y ) & 0xF8 ) << 3 );
	start_column = ( m_vdp.layer_fg_scroll_x >> 3 );
	for( column = 0; column < 29; column++ ) {
		UINT32  plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		int x, x_offset, tile_line, tile_address;
		int tile_data = ( m_vdp.vram[ map_addr + ( ( ( start_column + column ) & 0x1F ) << 1 ) + 1 ] << 8 )
									| m_vdp.vram[ map_addr + ( ( ( start_column + column ) & 0x1F ) << 1 ) ];
		int tile_number = tile_data & 0x01FF;
		int tile_palette = ( tile_data >> 9 ) & 0x0F;

		tile_line = ( m_vdp.current_line + m_vdp.layer_fg_scroll_y ) & 0x07;
		if ( tile_data & 0x8000 ) {
			tile_line = 7 - tile_line;
		}

		if ( m_vdp.colors_16 ) {
			tile_address = ( ( tile_data & 0x2000 ) ? 0x8000 : 0x4000 ) + ( tile_number * 32 ) + ( tile_line << 2 );
			if ( m_vdp.tile_packed ) {
				plane0 = ( m_vdp.vram[ tile_address + 0 ] << 24 ) | ( m_vdp.vram[ tile_address + 1 ] << 16 ) | ( m_vdp.vram[ tile_address + 2 ] << 8 ) | m_vdp.vram[ tile_address + 3 ];
			} else {
				plane0 = m_vdp.vram[ tile_address + 0 ];
				plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
				plane2 = m_vdp.vram[ tile_address + 2 ] << 2;
				plane3 = m_vdp.vram[ tile_address + 3 ] << 3;
			}
		} else {
			tile_address = 0x2000 + ( tile_number * 16 ) + ( tile_line << 1 );
			if ( m_vdp.tile_packed ) {
				plane0 = ( m_vdp.vram[ tile_address + 0 ] << 8 ) | m_vdp.vram[ tile_address + 1 ];
			} else {
				plane0 = m_vdp.vram[ tile_address + 0 ];
				plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
				plane2 = 0;
				plane3 = 0;
			}
		}

		for( x = 0; x < 8; x++ ) {
			int col;
			if ( m_vdp.tile_packed ) {
				if ( m_vdp.colors_16 ) {
					col = plane0 & 0x0F;
					plane0 = plane0 >> 4;
				} else {
					col = plane0 & 0x03;
					plane0 = plane0 >> 2;
				}
			} else {
				col = ( plane3 & 8 ) | ( plane2 & 4 ) | ( plane1 & 2 ) | ( plane0 & 1 );
				plane3 = plane3 >> 1;
				plane2 = plane2 >> 1;
				plane1 = plane1 >> 1;
				plane0 = plane0 >> 1;
			}
			if ( tile_data & 0x4000 ) {
				x_offset = x + ( column << 3 ) - ( m_vdp.layer_fg_scroll_x & 0x07 );
			} else {
				x_offset = 7 - x + ( column << 3 ) - ( m_vdp.layer_fg_scroll_x & 0x07 );
			}
			if ( x_offset >= 0 && x_offset < WSWAN_X_PIXELS ) {
				if ( m_vdp.colors_16 ) {
					if ( col ) {
//                      if ( m_vdp.color_mode ) {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
//                      } else {
//                          /* Hmmmm, what should we do here... Is this correct?? */
//                          m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
//                      }
					}
				} else {
					if ( col || !(tile_palette & 4 ) ) {
						if ( m_vdp.color_mode ) {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						} else {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_vdp.main_palette[m_pal[tile_palette][col]];
						}
					}
				}
			}
		}
	}
}

void wswan_state::wswan_draw_foreground_2()
{
	UINT16  map_addr;
	UINT8   start_column;
	int column;
	map_addr = m_vdp.layer_fg_address + ( ( ( m_vdp.current_line + m_vdp.layer_fg_scroll_y ) & 0xF8 ) << 3 );
	start_column = ( m_vdp.layer_fg_scroll_x >> 3 );
	for( column = 0; column < 29; column++ ) {
		UINT32  plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		int x, x_offset, tile_line, tile_address;
		int tile_data = ( m_vdp.vram[ map_addr + ( ( ( start_column + column ) & 0x1F ) << 1 ) + 1 ] << 8 )
						| m_vdp.vram[ map_addr + ( ( ( start_column + column ) & 0x1F ) << 1 ) ];
		int tile_number = tile_data & 0x01FF;
		int tile_palette = ( tile_data >> 9 ) & 0x0F;

		tile_line = ( m_vdp.current_line + m_vdp.layer_fg_scroll_y ) & 0x07;
		if ( tile_data & 0x8000 ) {
			tile_line = 7 - tile_line;
		}

		if ( m_vdp.colors_16 ) {
			tile_address = ( ( tile_data & 0x2000 ) ? 0x8000 : 0x4000 ) + ( tile_number * 32 ) + ( tile_line << 2 );
			if ( m_vdp.tile_packed ) {
				plane0 = ( m_vdp.vram[ tile_address + 0 ] << 24 ) | ( m_vdp.vram[ tile_address + 1 ] << 16 ) | ( m_vdp.vram[ tile_address + 2 ] << 8 ) | m_vdp.vram[ tile_address + 3 ];
			} else {
				plane0 = m_vdp.vram[ tile_address + 0 ];
				plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
				plane2 = m_vdp.vram[ tile_address + 2 ] << 2;
				plane3 = m_vdp.vram[ tile_address + 3 ] << 3;
			}
		} else {
			tile_address = 0x2000 + ( tile_number * 16 ) + ( tile_line << 1 );
			if ( m_vdp.tile_packed ) {
				plane0 = ( m_vdp.vram[ tile_address + 0 ] << 8 ) | m_vdp.vram[ tile_address + 1 ];
			} else {
				plane0 = m_vdp.vram[ tile_address + 0 ];
				plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
				plane2 = 0;
				plane3 = 0;
			}
		}

		for( x = 0; x < 8; x++ ) {
			int col;
			if ( m_vdp.tile_packed ) {
				if ( m_vdp.colors_16 ) {
					col = plane0 & 0x0F;
					plane0 = plane0 >> 4;
				} else {
					col = plane0 & 0x03;
					plane0 = plane0 >> 2;
				}
			} else {
				col = ( plane3 & 8 ) | ( plane2 & 4 ) | ( plane1 & 2 ) | ( plane0 & 1 );
				plane3 = plane3 >> 1;
				plane2 = plane2 >> 1;
				plane1 = plane1 >> 1;
				plane0 = plane0 >> 1;
			}
			if ( tile_data & 0x4000 ) {
				x_offset = x + ( column << 3 ) - ( m_vdp.layer_fg_scroll_x & 0x07 );
			} else {
				x_offset = 7 - x + ( column << 3 ) - ( m_vdp.layer_fg_scroll_x & 0x07 );
			}
			if ( x_offset >= 0 && x_offset >= m_vdp.window_fg_left && x_offset < m_vdp.window_fg_right && x_offset < WSWAN_X_PIXELS ) {
				if ( m_vdp.colors_16 ) {
					if ( col ) {
						if ( m_vdp.color_mode ) {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						} else {
							/* Hmmmm, what should we do here... Is this correct?? */
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						}
					}
				} else {
					if ( col || !(tile_palette & 4 ) ) {
						if ( m_vdp.color_mode ) {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						} else {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_vdp.main_palette[m_pal[tile_palette][col]];
						}
					}
				}
			}
		}
	}
}

void wswan_state::wswan_draw_foreground_3()
{
	UINT16  map_addr;
	UINT8   start_column;
	int column;
	map_addr = m_vdp.layer_fg_address + ( ( ( m_vdp.current_line + m_vdp.layer_fg_scroll_y ) & 0xF8 ) << 3 );
	start_column = ( m_vdp.layer_fg_scroll_x >> 3 );
	for( column = 0; column < 29; column++ ) {
		UINT32  plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
		int x, x_offset, tile_line, tile_address;
		int tile_data = ( m_vdp.vram[ map_addr + ( ( ( start_column + column ) & 0x1F ) << 1 ) + 1 ] << 8 )
						| m_vdp.vram[ map_addr + ( ( ( start_column + column ) & 0x1F ) << 1 ) ];
		int tile_number = tile_data & 0x01FF;
		int tile_palette = ( tile_data >> 9 ) & 0x0F;

		tile_line = ( m_vdp.current_line + m_vdp.layer_fg_scroll_y ) & 0x07;
		if ( tile_data & 0x8000 ) { // vflip
			tile_line = 7 - tile_line;
		}

		if ( m_vdp.colors_16 ) {
			tile_address = ( ( tile_data & 0x2000 ) ? 0x8000 : 0x4000 ) + ( tile_number * 32 ) + ( tile_line << 2 );
			if ( m_vdp.tile_packed ) {
				plane0 = ( m_vdp.vram[ tile_address + 0 ] << 24 ) | ( m_vdp.vram[ tile_address + 1 ] << 16 ) | ( m_vdp.vram[ tile_address + 2 ] << 8 ) | m_vdp.vram[ tile_address + 3 ];
			} else {
				plane0 = m_vdp.vram[ tile_address + 0 ];
				plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
				plane2 = m_vdp.vram[ tile_address + 2 ] << 2;
				plane3 = m_vdp.vram[ tile_address + 3 ] << 3;
			}
		} else {
			tile_address = 0x2000 + ( tile_number * 16 ) + ( tile_line << 1 );
			if ( m_vdp.tile_packed ) {
				plane0 = ( m_vdp.vram[ tile_address + 0 ] << 8 ) | m_vdp.vram[ tile_address + 1 ];
			} else {
				plane0 = m_vdp.vram[ tile_address + 0 ];
				plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
				plane2 = 0;
				plane3 = 0;
			}
		}

		for( x = 0; x < 8; x++ ) {
			int col;
			if ( m_vdp.tile_packed ) {
				if ( m_vdp.colors_16 ) {
					col = plane0 & 0x0F;
					plane0 = plane0 >> 4;
				} else {
					col = plane0 & 0x03;
					plane0 = plane0 >> 2;
				}
			} else {
				col = ( plane3 & 8 ) | ( plane2 & 4 ) | ( plane1 & 2 ) | ( plane0 & 1 );
				plane3 = plane3 >> 1;
				plane2 = plane2 >> 1;
				plane1 = plane1 >> 1;
				plane0 = plane0 >> 1;
			}
			if ( tile_data & 0x4000 ) {
				x_offset = x + ( column << 3 ) - ( m_vdp.layer_fg_scroll_x & 0x07 );
			} else {
				x_offset = 7 - x + ( column << 3 ) - ( m_vdp.layer_fg_scroll_x & 0x07 );
			}
			if ( ( x_offset >= 0 && x_offset < m_vdp.window_fg_left ) || ( x_offset >= m_vdp.window_fg_right && x_offset < WSWAN_X_PIXELS ) ) {
				if ( m_vdp.colors_16 ) {
					if ( col ) {
						if ( m_vdp.color_mode ) {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						} else {
							/* Hmmmm, what should we do here... Is this correct?? */
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						}
					}
				} else {
					if ( col || !(tile_palette & 4 ) ) {
						if ( m_vdp.color_mode ) {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
						} else {
							m_bitmap.pix16(m_vdp.current_line, x_offset) = m_vdp.main_palette[m_pal[tile_palette][col]];
						}
					}
				}
			}
		}
	}
}

void wswan_state::wswan_handle_sprites(int mask )
{
	int i;
	if ( m_vdp.sprite_count == 0 )
		return;
	for( i = m_vdp.sprite_first + m_vdp.sprite_count - 1; i >= m_vdp.sprite_first; i-- ) {
		UINT8   x, y;
		UINT16  tile_data;
		int tile_line;

		tile_data = ( m_vdp.sprite_table_buffer[ i * 4 + 1 ] << 8 ) | m_vdp.sprite_table_buffer[ i * 4 ];
		y = m_vdp.sprite_table_buffer[ i * 4 + 2 ];
		x = m_vdp.sprite_table_buffer[ i * 4 + 3 ];
		tile_line = m_vdp.current_line - y;
		tile_line = tile_line & 0xFF;
		if ( ( tile_line >= 0 ) && ( tile_line < 8 ) && ( ( tile_data & 0x2000 ) == mask ) ) {
			UINT32  plane0 = 0, plane1 = 0, plane2 = 0, plane3 = 0;
			int j, x_offset, tile_address;
			int tile_number = tile_data & 0x01FF;
			int tile_palette = 8 + ( ( tile_data >> 9 ) & 0x07 );
			int check_clip = 0;
			if ( tile_data & 0x8000 ) {
				tile_line = 7 - tile_line;
			}

			if ( m_vdp.colors_16 ) {
				tile_address = 0x4000 + ( tile_number * 32 ) + ( tile_line << 2 );
				if ( m_vdp.tile_packed ) {
					plane0 = ( m_vdp.vram[ tile_address + 0 ] << 24 ) | ( m_vdp.vram[ tile_address + 1 ] << 16 ) | ( m_vdp.vram[ tile_address + 2 ] << 8 ) | m_vdp.vram[ tile_address + 3 ];
				} else {
					plane0 = m_vdp.vram[ tile_address + 0 ];
					plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
					plane2 = m_vdp.vram[ tile_address + 2 ] << 2;
					plane3 = m_vdp.vram[ tile_address + 3 ] << 3;
				}
			} else {
				tile_address = 0x2000 + ( tile_number * 16 ) + ( tile_line << 1 );
				if ( m_vdp.tile_packed ) {
					plane0 = ( m_vdp.vram[ tile_address + 0 ] << 8 ) | m_vdp.vram[ tile_address + 1 ];
				} else {
					plane0 = m_vdp.vram[ tile_address + 0 ];
					plane1 = m_vdp.vram[ tile_address + 1 ] << 1;
					plane2 = 0;
					plane3 = 0;
				}
			}

			if ( m_vdp.window_sprites_enable ) {
				if ( tile_data & 0x1000 ) {
					if ( m_vdp.current_line >= m_vdp.window_sprites_top && m_vdp.current_line <= m_vdp.window_sprites_bottom ) {
						check_clip = 1;
					}
				} else {
					if ( m_vdp.current_line < m_vdp.window_sprites_top || m_vdp.current_line > m_vdp.window_sprites_bottom ) {
						continue;
					}
				}
			}

			for ( j = 0; j < 8; j++ ) {
				int col;
				if ( m_vdp.tile_packed ) {
					if ( m_vdp.colors_16 ) {
						col = plane0 & 0x0F;
						plane0 = plane0 >> 4;
					} else {
						col = plane0 & 0x03;
						plane0 = plane0 >> 2;
					}
				} else {
					col = ( plane3 & 8 ) | ( plane2 & 4 ) | ( plane1 & 2 ) | ( plane0 & 1 );
					plane3 = plane3 >> 1;
					plane2 = plane2 >> 1;
					plane1 = plane1 >> 1;
					plane0 = plane0 >> 1;
				}
				if ( tile_data & 0x4000 ) {
					x_offset = x + j;
				} else {
					x_offset = x + 7 - j;
				}
				x_offset = x_offset & 0xFF;
				if ( m_vdp.window_sprites_enable ) {
					if ( tile_data & 0x1000 && check_clip ) {
						if ( x_offset >= m_vdp.window_sprites_left && x_offset <= m_vdp.window_sprites_right ) {
							continue;
						}
					} else {
						if ( x_offset < m_vdp.window_sprites_left || x_offset > m_vdp.window_sprites_right ) {
//                          continue;
						}
					}
				}
				if ( x_offset >= 0 && x_offset < WSWAN_X_PIXELS ) {
					if ( m_vdp.colors_16 ) {
						if ( col ) {
							if ( m_vdp.color_mode ) {
								m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
							} else {
								/* Hmmmm, what should we do here... Is this correct?? */
								m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
							}
						}
					} else {
						if ( col || !(tile_palette & 4 ) ) {
							if ( m_vdp.color_mode ) {
								m_bitmap.pix16(m_vdp.current_line, x_offset) = m_pal[tile_palette][col];
							} else {
								m_bitmap.pix16(m_vdp.current_line, x_offset) = m_vdp.main_palette[m_pal[tile_palette][col]];
							}
						}
					}
				}
			}
		}
	}
}

void wswan_state::wswan_refresh_scanline()
{
	wswan_setup_palettes();

	rectangle rec(0, WSWAN_X_PIXELS, m_vdp.current_line, m_vdp.current_line);
	if ( m_ws_portram[0x14] ) {
		/* Not sure if these background color checks and settings are correct */
		if ( m_vdp.color_mode && m_vdp.colors_16 ) {
			m_bitmap.fill( m_pal[m_ws_portram[0x01]>>4][m_ws_portram[0x01]&0x0F], rec );
		} else {
			m_bitmap.fill( m_vdp.main_palette[m_ws_portram[0x01]&0x07], rec );
		}
	} else {
		m_bitmap.fill( 0, rec );
		return;
	}

	/*
	 * Draw background layer
	 */
	if ( m_vdp.layer_bg_enable ) {
		wswan_draw_background();
	}

	/*
	 * Draw sprites between background and foreground layers
	 */
	if ( m_vdp.sprites_enable ) {
		wswan_handle_sprites(0);
	}

	/*
	 * Draw foreground layer, taking window settings into account
	 */
	if ( m_vdp.layer_fg_enable ) {
		switch( m_vdp.window_fg_mode ) {
		case 0: /* FG inside & outside window area */
			wswan_draw_foreground_0();
			break;
		case 1: /* ??? */
			logerror( "Unknown foreground mode 1 set\n" );
			break;
		case 2: /* FG only inside window area */
			if ( m_vdp.current_line >= m_vdp.window_fg_top && m_vdp.current_line <= m_vdp.window_fg_bottom ) {
				wswan_draw_foreground_2();
			}
			break;
		case 3: /* FG only outside window area */
			if ( m_vdp.current_line < m_vdp.window_fg_top || m_vdp.current_line > m_vdp.window_fg_bottom ) {
				wswan_draw_foreground_0();
			} else {
				wswan_draw_foreground_3();
			}
			break;
		}
	}

	/*
	 * Draw sprites in front of foreground layer
	 */
	if ( m_vdp.sprites_enable ) {
		wswan_handle_sprites(0x2000);
	}
}


void wswan_state::video_start()
{
	machine().first_screen()->register_screen_bitmap(m_bitmap);
	save_item(NAME(m_bitmap));
}

UINT32 wswan_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);
	return 0;
}

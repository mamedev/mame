// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_SHARED_MAHJONG_H
#define MAME_SHARED_MAHJONG_H

#pragma once


INPUT_PORTS_EXTERN(mahjong_matrix_1p);          // letters, start, kan/pon/chi/reach/ron
INPUT_PORTS_EXTERN(mahjong_matrix_1p_ff);       // adds flip flop
INPUT_PORTS_EXTERN(mahjong_matrix_1p_bet);      // adds bet/last chance
INPUT_PORTS_EXTERN(mahjong_matrix_1p_bet_wup);  // adds take score/double up/big/small

INPUT_PORTS_EXTERN(mahjong_matrix_2p);          // letters, start, kan/pon/chi/reach/ron
INPUT_PORTS_EXTERN(mahjong_matrix_2p_ff);       // adds flip flop
INPUT_PORTS_EXTERN(mahjong_matrix_2p_bet);      // adds bet/last chance
INPUT_PORTS_EXTERN(mahjong_matrix_2p_bet_wup);  // adds take score/double up/big/small

#endif // MAME_SHARED_MAHJONG_H

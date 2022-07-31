// license:BSD-3-Clause
// copyright-holders:Robbbert
/********************************************************************************************************************
PINBALL
Created 2022-02-13.
Stern SPIKE/SPIKE2 system.

The current (as of 2022) system for Stern pinball machines. There are a number of boards that take 48 volt
power and regulate it to the voltages needed. The boards communicate via Cat5 cables, and each has a unique
network (node) address. For example the CPU board is Node 0, and other major boards are 8 and 9.

CPU/GPU = IMX 6 ARMS PROCESSOR CORTEX A9 1.2GHz DUAL CORE. Later games will have a Quad-core processor.
Like most modern pinballs, updates are uploaded via a USB stick.

This source is not included in the compilation, as there's no code at this time.

Game                                        NUM    Start game                   End ball
---------------------------------------------------------------------------------------------
Whoa Nelly! Big Juicy Melons                D7
The Pabst Can Crusher                       D9
WrestleMania (Pro)                          G1
Legends of WrestleMania (LE)                G2
Game of Thrones (Pro)                       G4
Game of Thrones (LE)                        G5
Game of Thrones (Premium)                   G6
Kiss (Pro)                                  H1
Kiss (LE)                                   H2
Kiss (Premium)                              H3
Ghostbusters (Pro)                          H5
Ghostbusters (LE)                           H6
Ghostbusters (Premium)                      H7
Batman (LE)                                 I0
Batman (Premium)                            I2
Batman (SLE)                                I3
Batman                                      I4
Aerosmith (Pro)                             I5
Aerosmith (LE)                              I6
Aerosmith (Premium)                         I7
Elvira's House of Horrors (LE)              J1
Elvira's House of Horrors (Premium)         J2
Elvira's House of Horrors (SE)              J3
Star Wars (Pro)                             J5
Star Wars (LE)                              J6
Star Wars (Premium)                         J7
Star Wars (Comic Pro)                       J8
Star Wars (Premium)                         J9 (this game has 2 numbers)
Deadpool (Pro)                              K1
Deadpool (LE)                               K2
Deadpool (Premium)                          K3
The Beatles (Platinum)                      K7
The Beatles (Gold)                          K8
The Beatles (Diamond)                       K9
The Munsters (Pro)                          L1
The Munsters (LE)                           L2
The Munsters (Premium)                      L3
Guardians of the Galaxy (Pro)               L5
Guardians of the Galaxy (LE)                L6
Guardians of the Galaxy (Premium)           L7
Star Wars Pin                               L8
Star Wars Pin (Comic)                       L9
Jurassic Park (Pro)                         M1
Jurassic Park (LE)                          M2
Jurassic Park (Premium)                     M3
Jurassic Park Pin                           M4
Black Knight: Sword of Rage (Pro)           N1
Black Knight: Sword of Rage (LE)            N2
Black Knight: Sword of Rage (Premium)       N3
Iron Maiden: Legacy of the Beast (Pro)      N5
Iron Maiden: Legacy of the Beast (LE)       N6
Iron Maiden: Legacy of the Beast (Premium)  N7
Primus                                      N9
Supreme (Made by Stern for Supreme Co.)     PL
Stranger Things (Pro)                       Q1
Stranger Things (LE)                        Q2
Stranger Things (Premium)                   Q3
Teenage Mutant Ninja Turtles (Pro?)         Q5
Teenage Mutant Ninja Turtles (LE)           Q6
Teenage Mutant Ninja Turtles (Premium)      Q7
The Avengers: Infinity Quest (Pro)          R1
The Avengers: Infinity Quest (LE)           R2
The Avengers: Infinity Quest (Premium)      R3
Led Zeppelin (Pro)                          R5
Led Zeppelin (LE)                           R6
Led Zeppelin (Premium)                      R7
Heavy Metal                                 S1
The Mandalorian (Pro)                       S5
The Mandalorian (LE)                        S6
The Mandalorian (Premium)                   S7
Rush (Pro)                                  T1
Rush (LE)                                   T2
Rush (Premium)                              T3
Godzilla (Pro)                              T5
Godzilla (LE)                               T6
Godzilla (Premium)                          T7

* Notes: LE = Limited Edition; SE = Special Edition; SLE = Special Limited Edition; Pin = Pinball for the home.

Status:
- Skeletons

ToDo:
- Everything

*********************************************************************************************************************/
//#include "emu.h"
// Include info here as it is found.

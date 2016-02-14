/*
 *  Copyright 2014 The Luvit Authors. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include "luv.h"

static int luv_constants(lua_State* L) {
  lua_newtable(L);

  // File open bitwise flags O_*
#ifdef O_RDONLY
  lua_pushinteger(L, O_RDONLY);
  lua_setfield(L, -2, "O_RDONLY");
#endif
#ifdef O_WRONLY
  lua_pushinteger(L, O_WRONLY);
  lua_setfield(L, -2, "O_WRONLY");
#endif
#ifdef O_RDWR
  lua_pushinteger(L, O_RDWR);
  lua_setfield(L, -2, "O_RDWR");
#endif
#ifdef O_APPEND
  lua_pushinteger(L, O_APPEND);
  lua_setfield(L, -2, "O_APPEND");
#endif
#ifdef O_CREAT
  lua_pushinteger(L, O_CREAT);
  lua_setfield(L, -2, "O_CREAT");
#endif
#ifdef O_DSYNC
  lua_pushinteger(L, O_DSYNC);
  lua_setfield(L, -2, "O_DSYNC");
#endif
#ifdef O_EXCL
  lua_pushinteger(L, O_EXCL);
  lua_setfield(L, -2, "O_EXCL");
#endif
#ifdef O_EXLOCK
  lua_pushinteger(L, O_EXLOCK);
  lua_setfield(L, -2, "O_EXLOCK");
#endif
#ifdef O_NOCTTY
  lua_pushinteger(L, O_NOCTTY);
  lua_setfield(L, -2, "O_NOCTTY");
#endif
#ifdef O_NONBLOCK
  lua_pushinteger(L, O_NONBLOCK);
  lua_setfield(L, -2, "O_NONBLOCK");
#endif
#ifdef O_RSYNC
  lua_pushinteger(L, O_RSYNC);
  lua_setfield(L, -2, "O_RSYNC");
#endif
#ifdef O_SYNC
  lua_pushinteger(L, O_SYNC);
  lua_setfield(L, -2, "O_SYNC");
#endif
#ifdef O_TRUNC
  lua_pushinteger(L, O_TRUNC);
  lua_setfield(L, -2, "O_TRUNC");
#endif

  // Socket types SOCK_*
#ifdef SOCK_STREAM
  lua_pushinteger(L, SOCK_STREAM);
  lua_setfield(L, -2, "SOCK_STREAM");
#endif
#ifdef SOCK_DGRAM
  lua_pushinteger(L, SOCK_DGRAM);
  lua_setfield(L, -2, "SOCK_DGRAM");
#endif
#ifdef SOCK_SEQPACKET
  lua_pushinteger(L, SOCK_SEQPACKET);
  lua_setfield(L, -2, "SOCK_SEQPACKET");
#endif
#ifdef SOCK_RAW
  lua_pushinteger(L, SOCK_RAW);
  lua_setfield(L, -2, "SOCK_RAW");
#endif
#ifdef SOCK_RDM
  lua_pushinteger(L, SOCK_RDM);
  lua_setfield(L, -2, "SOCK_RDM");
#endif

  // AF_*
#ifdef AF_UNIX
  lua_pushinteger(L, AF_UNIX);
  lua_setfield(L, -2, "AF_UNIX");
#endif
#ifdef AF_INET
  lua_pushinteger(L, AF_INET);
  lua_setfield(L, -2, "AF_INET");
#endif
#ifdef AF_INET6
  lua_pushinteger(L, AF_INET6);
  lua_setfield(L, -2, "AF_INET6");
#endif
#ifdef AF_IPX
  lua_pushinteger(L, AF_IPX);
  lua_setfield(L, -2, "AF_IPX");
#endif
#ifdef AF_NETLINK
  lua_pushinteger(L, AF_NETLINK);
  lua_setfield(L, -2, "AF_NETLINK");
#endif
#ifdef AF_X25
  lua_pushinteger(L, AF_X25);
  lua_setfield(L, -2, "AF_X25");
#endif
#ifdef AF_AX25
  lua_pushinteger(L, AF_AX25);
  lua_setfield(L, -2, "AF_AX25");
#endif
#ifdef AF_ATMPVC
  lua_pushinteger(L, AF_ATMPVC);
  lua_setfield(L, -2, "AF_ATMPVC");
#endif
#ifdef AF_APPLETALK
  lua_pushinteger(L, AF_APPLETALK);
  lua_setfield(L, -2, "AF_APPLETALK");
#endif
#ifdef AF_PACKET
  lua_pushinteger(L, AF_PACKET);
  lua_setfield(L, -2, "AF_PACKET");
#endif

  // AI_*
#ifdef AI_ADDRCONFIG
  lua_pushinteger(L, AI_ADDRCONFIG);
  lua_setfield(L, -2, "AI_ADDRCONFIG");
#endif
#ifdef AI_V4MAPPED
  lua_pushinteger(L, AI_V4MAPPED);
  lua_setfield(L, -2, "AI_V4MAPPED");
#endif
#ifdef AI_ALL
  lua_pushinteger(L, AI_ALL);
  lua_setfield(L, -2, "AI_ALL");
#endif
#ifdef AI_NUMERICHOST
  lua_pushinteger(L, AI_NUMERICHOST);
  lua_setfield(L, -2, "AI_NUMERICHOST");
#endif
#ifdef AI_PASSIVE
  lua_pushinteger(L, AI_PASSIVE);
  lua_setfield(L, -2, "AI_PASSIVE");
#endif
#ifdef AI_NUMERICSERV
  lua_pushinteger(L, AI_NUMERICSERV);
  lua_setfield(L, -2, "AI_NUMERICSERV");
#endif

  // Signals
#ifdef SIGHUP
  lua_pushinteger(L, SIGHUP);
  lua_setfield(L, -2, "SIGHUP");
#endif
#ifdef SIGINT
  lua_pushinteger(L, SIGINT);
  lua_setfield(L, -2, "SIGINT");
#endif
#ifdef SIGQUIT
  lua_pushinteger(L, SIGQUIT);
  lua_setfield(L, -2, "SIGQUIT");
#endif
#ifdef SIGILL
  lua_pushinteger(L, SIGILL);
  lua_setfield(L, -2, "SIGILL");
#endif
#ifdef SIGTRAP
  lua_pushinteger(L, SIGTRAP);
  lua_setfield(L, -2, "SIGTRAP");
#endif
#ifdef SIGABRT
  lua_pushinteger(L, SIGABRT);
  lua_setfield(L, -2, "SIGABRT");
#endif
#ifdef SIGIOT
  lua_pushinteger(L, SIGIOT);
  lua_setfield(L, -2, "SIGIOT");
#endif
#ifdef SIGBUS
  lua_pushinteger(L, SIGBUS);
  lua_setfield(L, -2, "SIGBUS");
#endif
#ifdef SIGFPE
  lua_pushinteger(L, SIGFPE);
  lua_setfield(L, -2, "SIGFPE");
#endif
#ifdef SIGKILL
  lua_pushinteger(L, SIGKILL);
  lua_setfield(L, -2, "SIGKILL");
#endif
#ifdef SIGUSR1
  lua_pushinteger(L, SIGUSR1);
  lua_setfield(L, -2, "SIGUSR1");
#endif
#ifdef SIGSEGV
  lua_pushinteger(L, SIGSEGV);
  lua_setfield(L, -2, "SIGSEGV");
#endif
#ifdef SIGUSR2
  lua_pushinteger(L, SIGUSR2);
  lua_setfield(L, -2, "SIGUSR2");
#endif
#ifdef SIGPIPE
  lua_pushinteger(L, SIGPIPE);
  lua_setfield(L, -2, "SIGPIPE");
#endif
#ifdef SIGALRM
  lua_pushinteger(L, SIGALRM);
  lua_setfield(L, -2, "SIGALRM");
#endif
#ifdef SIGTERM
  lua_pushinteger(L, SIGTERM);
  lua_setfield(L, -2, "SIGTERM");
#endif
#ifdef SIGCHLD
  lua_pushinteger(L, SIGCHLD);
  lua_setfield(L, -2, "SIGCHLD");
#endif
#ifdef SIGSTKFLT
  lua_pushinteger(L, SIGSTKFLT);
  lua_setfield(L, -2, "SIGSTKFLT");
#endif
#ifdef SIGCONT
  lua_pushinteger(L, SIGCONT);
  lua_setfield(L, -2, "SIGCONT");
#endif
#ifdef SIGSTOP
  lua_pushinteger(L, SIGSTOP);
  lua_setfield(L, -2, "SIGSTOP");
#endif
#ifdef SIGTSTP
  lua_pushinteger(L, SIGTSTP);
  lua_setfield(L, -2, "SIGTSTP");
#endif
#ifdef SIGBREAK
  lua_pushinteger(L, SIGBREAK);
  lua_setfield(L, -2, "SIGBREAK");
#endif
#ifdef SIGTTIN
  lua_pushinteger(L, SIGTTIN);
  lua_setfield(L, -2, "SIGTTIN");
#endif
#ifdef SIGTTOU
  lua_pushinteger(L, SIGTTOU);
  lua_setfield(L, -2, "SIGTTOU");
#endif
#ifdef SIGURG
  lua_pushinteger(L, SIGURG);
  lua_setfield(L, -2, "SIGURG");
#endif
#ifdef SIGXCPU
  lua_pushinteger(L, SIGXCPU);
  lua_setfield(L, -2, "SIGXCPU");
#endif
#ifdef SIGXFSZ
  lua_pushinteger(L, SIGXFSZ);
  lua_setfield(L, -2, "SIGXFSZ");
#endif
#ifdef SIGVTALRM
  lua_pushinteger(L, SIGVTALRM);
  lua_setfield(L, -2, "SIGVTALRM");
#endif
#ifdef SIGPROF
  lua_pushinteger(L, SIGPROF);
  lua_setfield(L, -2, "SIGPROF");
#endif
#ifdef SIGWINCH
  lua_pushinteger(L, SIGWINCH);
  lua_setfield(L, -2, "SIGWINCH");
#endif
#ifdef SIGIO
  lua_pushinteger(L, SIGIO);
  lua_setfield(L, -2, "SIGIO");
#endif
#ifdef SIGPOLL
  lua_pushinteger(L, SIGPOLL);
  lua_setfield(L, -2, "SIGPOLL");
#endif
#ifdef SIGLOST
  lua_pushinteger(L, SIGLOST);
  lua_setfield(L, -2, "SIGLOST");
#endif
#ifdef SIGPWR
  lua_pushinteger(L, SIGPWR);
  lua_setfield(L, -2, "SIGPWR");
#endif
#ifdef SIGSYS
  lua_pushinteger(L, SIGSYS);
  lua_setfield(L, -2, "SIGSYS");
#endif
  return 1;
}

static int luv_af_string_to_num(const char* string) {
  if (!string) return AF_UNSPEC;
#ifdef AF_UNIX
  if (strcmp(string, "unix") == 0) return AF_UNIX;
#endif
#ifdef AF_INET
  if (strcmp(string, "inet") == 0) return AF_INET;
#endif
#ifdef AF_INET6
  if (strcmp(string, "inet6") == 0) return AF_INET6;
#endif
#ifdef AF_IPX
  if (strcmp(string, "ipx") == 0) return AF_IPX;
#endif
#ifdef AF_NETLINK
  if (strcmp(string, "netlink") == 0) return AF_NETLINK;
#endif
#ifdef AF_X25
  if (strcmp(string, "x25") == 0) return AF_X25;
#endif
#ifdef AF_AX25
  if (strcmp(string, "ax25") == 0) return AF_AX25;
#endif
#ifdef AF_ATMPVC
  if (strcmp(string, "atmpvc") == 0) return AF_ATMPVC;
#endif
#ifdef AF_APPLETALK
  if (strcmp(string, "appletalk") == 0) return AF_APPLETALK;
#endif
#ifdef AF_PACKET
  if (strcmp(string, "packet") == 0) return AF_PACKET;
#endif
  return 0;
}

static const char* luv_af_num_to_string(const int num) {
  switch (num) {
#ifdef AF_UNIX
  case AF_UNIX: return "unix";
#endif
#ifdef AF_INET
  case AF_INET: return "inet";
#endif
#ifdef AF_INET6
  case AF_INET6: return "inet6";
#endif
#ifdef AF_IPX
  case AF_IPX: return "ipx";
#endif
#ifdef AF_NETLINK
  case AF_NETLINK: return "netlink";
#endif
#ifdef AF_X25
  case AF_X25: return "x25";
#endif
#ifdef AF_AX25
  case AF_AX25: return "ax25";
#endif
#ifdef AF_ATMPVC
  case AF_ATMPVC: return "atmpvc";
#endif
#ifdef AF_APPLETALK
  case AF_APPLETALK: return "appletalk";
#endif
#ifdef AF_PACKET
  case AF_PACKET: return "packet";
#endif
  }
  return NULL;
}


static int luv_sock_string_to_num(const char* string) {
  if (!string) return 0;
#ifdef SOCK_STREAM
  if (strcmp(string, "stream") == 0) return SOCK_STREAM;
#endif
#ifdef SOCK_DGRAM
  if (strcmp(string, "dgram") == 0) return SOCK_DGRAM;
#endif
#ifdef SOCK_SEQPACKET
  if (strcmp(string, "seqpacket") == 0) return SOCK_SEQPACKET;
#endif
#ifdef SOCK_RAW
  if (strcmp(string, "raw") == 0) return SOCK_RAW;
#endif
#ifdef SOCK_RDM
  if (strcmp(string, "rdm") == 0) return SOCK_RDM;
#endif
  return 0;
}

static const char* luv_sock_num_to_string(const int num) {
  switch (num) {
#ifdef SOCK_STREAM
  case SOCK_STREAM: return "stream";
#endif
#ifdef SOCK_DGRAM
  case SOCK_DGRAM: return "dgram";
#endif
#ifdef SOCK_SEQPACKET
  case SOCK_SEQPACKET: return "seqpacket";
#endif
#ifdef SOCK_RAW
  case SOCK_RAW: return "raw";
#endif
#ifdef SOCK_RDM
  case SOCK_RDM: return "rdm";
#endif
  }
  return NULL;
}

static int luv_sig_string_to_num(const char* string) {
  if (!string) return 0;
#ifdef SIGHUP
  if (strcmp(string, "sighup") == 0) return SIGHUP;
#endif
#ifdef SIGINT
  if (strcmp(string, "sigint") == 0) return SIGINT;
#endif
#ifdef SIGQUIT
  if (strcmp(string, "sigquit") == 0) return SIGQUIT;
#endif
#ifdef SIGILL
  if (strcmp(string, "sigill") == 0) return SIGILL;
#endif
#ifdef SIGTRAP
  if (strcmp(string, "sigtrap") == 0) return SIGTRAP;
#endif
#ifdef SIGABRT
  if (strcmp(string, "sigabrt") == 0) return SIGABRT;
#endif
#ifdef SIGIOT
  if (strcmp(string, "sigiot") == 0) return SIGIOT;
#endif
#ifdef SIGBUS
  if (strcmp(string, "sigbus") == 0) return SIGBUS;
#endif
#ifdef SIGFPE
  if (strcmp(string, "sigfpe") == 0) return SIGFPE;
#endif
#ifdef SIGKILL
  if (strcmp(string, "sigkill") == 0) return SIGKILL;
#endif
#ifdef SIGUSR1
  if (strcmp(string, "sigusr1") == 0) return SIGUSR1;
#endif
#ifdef SIGSEGV
  if (strcmp(string, "sigsegv") == 0) return SIGSEGV;
#endif
#ifdef SIGUSR2
  if (strcmp(string, "sigusr2") == 0) return SIGUSR2;
#endif
#ifdef SIGPIPE
  if (strcmp(string, "sigpipe") == 0) return SIGPIPE;
#endif
#ifdef SIGALRM
  if (strcmp(string, "sigalrm") == 0) return SIGALRM;
#endif
#ifdef SIGTERM
  if (strcmp(string, "sigterm") == 0) return SIGTERM;
#endif
#ifdef SIGCHLD
  if (strcmp(string, "sigchld") == 0) return SIGCHLD;
#endif
#ifdef SIGSTKFLT
  if (strcmp(string, "sigstkflt") == 0) return SIGSTKFLT;
#endif
#ifdef SIGCONT
  if (strcmp(string, "sigcont") == 0) return SIGCONT;
#endif
#ifdef SIGSTOP
  if (strcmp(string, "sigstop") == 0) return SIGSTOP;
#endif
#ifdef SIGTSTP
  if (strcmp(string, "sigtstp") == 0) return SIGTSTP;
#endif
#ifdef SIGBREAK
  if (strcmp(string, "sigbreak") == 0) return SIGBREAK;
#endif
#ifdef SIGTTIN
  if (strcmp(string, "sigttin") == 0) return SIGTTIN;
#endif
#ifdef SIGTTOU
  if (strcmp(string, "sigttou") == 0) return SIGTTOU;
#endif
#ifdef SIGURG
  if (strcmp(string, "sigurg") == 0) return SIGURG;
#endif
#ifdef SIGXCPU
  if (strcmp(string, "sigxcpu") == 0) return SIGXCPU;
#endif
#ifdef SIGXFSZ
  if (strcmp(string, "sigxfsz") == 0) return SIGXFSZ;
#endif
#ifdef SIGVTALRM
  if (strcmp(string, "sigvtalrm") == 0) return SIGVTALRM;
#endif
#ifdef SIGPROF
  if (strcmp(string, "sigprof") == 0) return SIGPROF;
#endif
#ifdef SIGWINCH
  if (strcmp(string, "sigwinch") == 0) return SIGWINCH;
#endif
#ifdef SIGIO
  if (strcmp(string, "sigio") == 0) return SIGIO;
#endif
#ifdef SIGPOLL
  if (strcmp(string, "sigpoll") == 0) return SIGPOLL;
#endif
#ifdef SIGLOST
  if (strcmp(string, "siglost") == 0) return SIGLOST;
#endif
#ifdef SIGPWR
  if (strcmp(string, "sigpwr") == 0) return SIGPWR;
#endif
#ifdef SIGSYS
  if (strcmp(string, "sigsys") == 0) return SIGSYS;
#endif
  return 0;
}

static const char* luv_sig_num_to_string(const int num) {
  switch (num) {
#ifdef SIGHUP
  case SIGHUP: return "sighup";
#endif
#ifdef SIGINT
  case SIGINT: return "sigint";
#endif
#ifdef SIGQUIT
  case SIGQUIT: return "sigquit";
#endif
#ifdef SIGILL
  case SIGILL: return "sigill";
#endif
#ifdef SIGTRAP
  case SIGTRAP: return "sigtrap";
#endif
#ifdef SIGABRT
  case SIGABRT: return "sigabrt";
#endif
#ifdef SIGIOT
# if SIGIOT != SIGABRT
  case SIGIOT: return "sigiot";
# endif
#endif
#ifdef SIGBUS
  case SIGBUS: return "sigbus";
#endif
#ifdef SIGFPE
  case SIGFPE: return "sigfpe";
#endif
#ifdef SIGKILL
  case SIGKILL: return "sigkill";
#endif
#ifdef SIGUSR1
  case SIGUSR1: return "sigusr1";
#endif
#ifdef SIGSEGV
  case SIGSEGV: return "sigsegv";
#endif
#ifdef SIGUSR2
  case SIGUSR2: return "sigusr2";
#endif
#ifdef SIGPIPE
  case SIGPIPE: return "sigpipe";
#endif
#ifdef SIGALRM
  case SIGALRM: return "sigalrm";
#endif
#ifdef SIGTERM
  case SIGTERM: return "sigterm";
#endif
#ifdef SIGCHLD
  case SIGCHLD: return "sigchld";
#endif
#ifdef SIGSTKFLT
  case SIGSTKFLT: return "sigstkflt";
#endif
#ifdef SIGCONT
  case SIGCONT: return "sigcont";
#endif
#ifdef SIGSTOP
  case SIGSTOP: return "sigstop";
#endif
#ifdef SIGTSTP
  case SIGTSTP: return "sigtstp";
#endif
#ifdef SIGBREAK
  case SIGBREAK: return "sigbreak";
#endif
#ifdef SIGTTIN
  case SIGTTIN: return "sigttin";
#endif
#ifdef SIGTTOU
  case SIGTTOU: return "sigttou";
#endif
#ifdef SIGURG
  case SIGURG: return "sigurg";
#endif
#ifdef SIGXCPU
  case SIGXCPU: return "sigxcpu";
#endif
#ifdef SIGXFSZ
  case SIGXFSZ: return "sigxfsz";
#endif
#ifdef SIGVTALRM
  case SIGVTALRM: return "sigvtalrm";
#endif
#ifdef SIGPROF
  case SIGPROF: return "sigprof";
#endif
#ifdef SIGWINCH
  case SIGWINCH: return "sigwinch";
#endif
#ifdef SIGIO
  case SIGIO: return "sigio";
#endif
#ifdef SIGPOLL
# if SIGPOLL != SIGIO
  case SIGPOLL: return "sigpoll";
# endif
#endif
#ifdef SIGLOST
  case SIGLOST: return "siglost";
#endif
#ifdef SIGPWR
# if SIGPWR != SIGLOST
  case SIGPWR: return "sigpwr";
# endif
#endif
#ifdef SIGSYS
  case SIGSYS: return "sigsys";
#endif
  }
  return NULL;
}

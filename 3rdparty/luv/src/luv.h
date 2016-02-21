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
#ifndef LUV_H
#define LUV_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "uv.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#if defined(_WIN32)
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# ifndef __MINGW32__
#   define S_ISREG(x)  (((x) & _S_IFMT) == _S_IFREG)
#   define S_ISDIR(x)  (((x) & _S_IFMT) == _S_IFDIR)
#   define S_ISFIFO(x) (((x) & _S_IFMT) == _S_IFIFO)
#   define S_ISCHR(x)  (((x) & _S_IFMT) == _S_IFCHR)
#   define S_ISBLK(x)  0
# endif
# define S_ISLNK(x)  (((x) & S_IFLNK) == S_IFLNK)
# define S_ISSOCK(x) 0
#else
# include <unistd.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX (8096)
#endif

#ifndef MAX_TITLE_LENGTH
#define MAX_TITLE_LENGTH (8192)
#endif

#if LUA_VERSION_NUM < 502
# define lua_rawlen lua_objlen
/* lua_...uservalue: Something very different, but it should get the job done */
# define lua_getuservalue lua_getfenv
# define lua_setuservalue lua_setfenv
# define luaL_newlib(L,l) (lua_newtable(L), luaL_register(L,NULL,l))
# define luaL_setfuncs(L,l,n) (assert(n==0), luaL_register(L,NULL,l))
# define lua_resume(L,F,n) lua_resume(L,n)
# define lua_pushglobaltable(L) lua_pushvalue(L, LUA_GLOBALSINDEX)
#endif

/* There is a 1-1 relation between a lua_State and a uv_loop_t
   These helpers will give you one if you have the other
   These are exposed for extensions built with luv
   This allows luv to be used in multithreaded applications.
*/
LUALIB_API lua_State* luv_state(uv_loop_t* loop);
/* All libuv callbacks will lua_call directly from this root-per-thread state
*/
LUALIB_API uv_loop_t* luv_loop(lua_State* L);

/* This is the main hook to load the library.
   This can be called multiple times in a process as long
   as you use a different lua_State and thread for each.
*/
LUALIB_API int luaopen_luv (lua_State *L);

#include "util.h"
#include "lhandle.h"
#include "lreq.h"

/* From stream.c */
static uv_stream_t* luv_check_stream(lua_State* L, int index);
static void luv_alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf);
static void luv_check_buf(lua_State *L, int idx, uv_buf_t *pbuf);
static uv_buf_t* luv_prep_bufs(lua_State* L, int index, size_t *count);

/* from tcp.c */
static void parse_sockaddr(lua_State* L, struct sockaddr_storage* address, int addrlen);
static void luv_connect_cb(uv_connect_t* req, int status);

/* From fs.c */
static void luv_push_stats_table(lua_State* L, const uv_stat_t* s);

/* from constants.c */
static int luv_af_string_to_num(const char* string);
static const char* luv_af_num_to_string(const int num);
static int luv_sock_string_to_num(const char* string);
static const char* luv_sock_num_to_string(const int num);
static int luv_sig_string_to_num(const char* string);
static const char* luv_sig_num_to_string(const int num);

typedef lua_State* (*luv_acquire_vm)();
typedef void (*luv_release_vm)(lua_State* L);
LUALIB_API void luv_set_thread_cb(luv_acquire_vm acquire, luv_release_vm release);

#endif

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
#ifndef LUV_LHANDLE_H
#define LUV_LHANDLE_H

#include "luv.h"

/* There are two slots for holding callbacks.  One is for the CLOSED event.
   The other slot is for all others since they never conflict in practice.
*/
#define luv_callback_id int
#define LUV_CLOSED 0
#define LUV_TIMEOUT 1
#define LUV_PREPARE 1
#define LUV_IDLE 1
#define LUV_CHECK 1
#define LUV_ASYNC 1
#define LUV_POLL 1
#define LUV_SIGNAL 1
#define LUV_EXIT 1
#define LUV_CONNECTION 1
#define LUV_READ 1
#define LUV_RECV 1
#define LUV_FS_EVENT 1
#define LUV_FS_POLL 1

/* Ref for userdata and event callbacks */
typedef struct {
  int ref;
  int callbacks[2];
  void* extra;
} luv_handle_t;

/* Setup the handle at the top of the stack */
static luv_handle_t* luv_setup_handle(lua_State* L);

/* Store a lua callback in a luv_handle for future callbacks.
   Either replace an existing callback by id or append a new one at the end.
*/
static void luv_check_callback(lua_State* L, luv_handle_t* data, luv_callback_id id, int index);

/* Lookup a function and call it with nargs
   If there is no such function, pop the args.
*/
static void luv_call_callback(lua_State* L, luv_handle_t* data, luv_callback_id id, int nargs);

/* Push a userdata on the stack from a handle */
static void luv_find_handle(lua_State* L, luv_handle_t* data);

/* Recursivly free the luv_handle and all event handlers */
static void luv_cleanup_handle(lua_State* L, luv_handle_t* data);

#endif

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
#ifndef LUV_LTHREADPOOL_H
#define LUV_LTHREADPOOL_H

#include "luv.h"

#define LUV_THREAD_MAXNUM_ARG 9

typedef struct {
  /* only support LUA_TNIL, LUA_TBOOLEAN, LUA_TLIGHTUSERDATA, LUA_TNUMBER, LUA_TSTRING*/
  int type;
  union
  {
    lua_Number num;
    int boolean;
    void* userdata;
    struct {
      const char* base;
      size_t len;
    } str;
  } val;
} luv_val_t;

typedef struct {
  int argc;
  luv_val_t argv[LUV_THREAD_MAXNUM_ARG];
} luv_thread_arg_t;

static int luv_thread_arg_set(lua_State* L, luv_thread_arg_t* args, int idx, int top, int flag);
static int luv_thread_arg_push(lua_State* L, const luv_thread_arg_t* args);
static void luv_thread_arg_clear(luv_thread_arg_t* args);

#endif //LUV_LTHREADPOOL_H

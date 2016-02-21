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

static uv_req_t* luv_check_req(lua_State* L, int index) {
  uv_req_t* req = luaL_checkudata(L, index, "uv_req");
  luaL_argcheck(L, req->data, index, "Expected uv_req_t");
  return req;
}

static int luv_req_tostring(lua_State* L) {
  uv_req_t* req = luaL_checkudata(L, 1, "uv_req");
  switch (req->type) {
#define XX(uc, lc) case UV_##uc: lua_pushfstring(L, "uv_"#lc"_t: %p", req); break;
  UV_REQ_TYPE_MAP(XX)
#undef XX
    default: lua_pushfstring(L, "uv_req_t: %p", req); break;
  }
  return 1;
}

static void luv_req_init(lua_State* L) {
  luaL_newmetatable (L, "uv_req");
  lua_pushcfunction(L, luv_req_tostring);
  lua_setfield(L, -2, "__tostring");
  lua_pop(L, 1);
}

// Metamethod to allow storing anything in the userdata's environment
static int luv_cancel(lua_State* L) {
  uv_req_t* req = luv_check_req(L, 1);
  int ret = uv_cancel(req);
  if (ret < 0) return luv_error(L, ret);
  luv_cleanup_req(L, req->data);
  req->data = NULL;
  lua_pushinteger(L, ret);
  return 1;
}

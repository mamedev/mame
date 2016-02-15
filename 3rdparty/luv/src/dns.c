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
#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#endif

static void luv_pushaddrinfo(lua_State* L, struct addrinfo* res) {
  char ip[INET6_ADDRSTRLEN];
  int port, i = 0;
  const char *addr;
  struct addrinfo* curr = res;
  lua_newtable(L);
  for (curr = res; curr; curr = curr->ai_next) {
    if (curr->ai_family == AF_INET || curr->ai_family == AF_INET6) {
      lua_newtable(L);
      if (curr->ai_family == AF_INET) {
        addr = (char*) &((struct sockaddr_in*) curr->ai_addr)->sin_addr;
        port = ((struct sockaddr_in*) curr->ai_addr)->sin_port;
      } else {
        addr = (char*) &((struct sockaddr_in6*) curr->ai_addr)->sin6_addr;
        port = ((struct sockaddr_in6*) curr->ai_addr)->sin6_port;
      }
      lua_pushstring(L, luv_af_num_to_string(curr->ai_family));
      lua_setfield(L, -2, "family");
      uv_inet_ntop(curr->ai_family, addr, ip, INET6_ADDRSTRLEN);
      lua_pushstring(L, ip);
      lua_setfield(L, -2, "addr");
      if (ntohs(port)) {
        lua_pushinteger(L, ntohs(port));
        lua_setfield(L, -2, "port");
      }
      lua_pushstring(L, luv_sock_num_to_string(curr->ai_socktype));
      lua_setfield(L, -2, "socktype");
      lua_pushstring(L, luv_af_num_to_string(curr->ai_protocol));
      lua_setfield(L, -2, "protocol");
      if (curr->ai_canonname) {
        lua_pushstring(L, curr->ai_canonname);
        lua_setfield(L, -2, "canonname");
      }
      lua_rawseti(L, -2, ++i);
    }
  }
}

static void luv_getaddrinfo_cb(uv_getaddrinfo_t* req, int status, struct addrinfo* res) {
  lua_State* L = luv_state(req->loop);
  int nargs;

  if (status < 0) {
    luv_status(L, status);
    nargs = 1;
  }
  else {
    lua_pushnil(L);
    luv_pushaddrinfo(L, res);
    nargs = 2;
  }
  luv_fulfill_req(L, req->data, nargs);
  luv_cleanup_req(L, req->data);
  req->data = NULL;
  if (res) uv_freeaddrinfo(res);
}


static int luv_getaddrinfo(lua_State* L) {
  uv_getaddrinfo_t* req;
  const char* node;
  const char* service;
  struct addrinfo hints_s;
  struct addrinfo* hints = &hints_s;
  int ret, ref;
  if (lua_isnoneornil(L, 1)) node = NULL;
  else node = luaL_checkstring(L, 1);
  if (lua_isnoneornil(L, 2)) service = NULL;
  else service = luaL_checkstring(L, 2);
  if (!lua_isnoneornil(L, 3)) luaL_checktype(L, 3, LUA_TTABLE);
  else hints = NULL;
  ref = lua_isnoneornil(L, 4) ? LUA_NOREF : luv_check_continuation(L, 4);
  if (hints) {
    // Initialize the hints
    memset(hints, 0, sizeof(*hints));

    // Process the `family` hint.
    lua_getfield(L, 3, "family");
    if (lua_isnumber(L, -1)) {
      hints->ai_family = lua_tointeger(L, -1);
    }
    else if (lua_isstring(L, -1)) {
      hints->ai_family = luv_af_string_to_num(lua_tostring(L, -1));
    }
    else if (lua_isnil(L, -1)) {
      hints->ai_family = AF_UNSPEC;
    }
    else {
      luaL_argerror(L, 3, "family hint must be string if set");
    }
    lua_pop(L, 1);

    // Process `socktype` hint
    lua_getfield(L, 3, "socktype");
    if (lua_isnumber(L, -1)) {
      hints->ai_socktype = lua_tointeger(L, -1);
    }
    else if (lua_isstring(L, -1)) {
      hints->ai_socktype = luv_sock_string_to_num(lua_tostring(L, -1));
    }
    else if (!lua_isnil(L, -1)) {
      return luaL_argerror(L, 3, "socktype hint must be string if set");
    }
    lua_pop(L, 1);

    // Process the `protocol` hint
    lua_getfield(L, 3, "protocol");
    if (lua_isnumber(L, -1)) {
      hints->ai_protocol = lua_tointeger(L, -1);
    }
    else if (lua_isstring(L, -1)) {
      int protocol = luv_af_string_to_num(lua_tostring(L, -1));
      if (protocol) {
        hints->ai_protocol = protocol;
      }
      else {
        return luaL_argerror(L, 3, "Invalid protocol hint");
      }
    }
    else if (!lua_isnil(L, -1)) {
      return luaL_argerror(L, 3, "protocol hint must be string if set");
    }
    lua_pop(L, 1);

    lua_getfield(L, 3, "addrconfig");
    if (lua_toboolean(L, -1)) hints->ai_flags |=  AI_ADDRCONFIG;
    lua_pop(L, 1);

    lua_getfield(L, 3, "v4mapped");
    if (lua_toboolean(L, -1)) hints->ai_flags |=  AI_V4MAPPED;
    lua_pop(L, 1);

    lua_getfield(L, 3, "all");
    if (lua_toboolean(L, -1)) hints->ai_flags |=  AI_ALL;
    lua_pop(L, 1);

    lua_getfield(L, 3, "numerichost");
    if (lua_toboolean(L, -1)) hints->ai_flags |=  AI_NUMERICHOST;
    lua_pop(L, 1);

    lua_getfield(L, 3, "passive");
    if (lua_toboolean(L, -1)) hints->ai_flags |=  AI_PASSIVE;
    lua_pop(L, 1);

    lua_getfield(L, 3, "numericserv");
    if (lua_toboolean(L, -1)) {
        hints->ai_flags |=  AI_NUMERICSERV;
        /* On OS X upto at least OSX 10.9, getaddrinfo crashes
         * if AI_NUMERICSERV is set and the servname is NULL or "0".
         * This workaround avoids a segfault in libsystem.
         */
        if (NULL == service) service = "00";
    }
    lua_pop(L, 1);

    lua_getfield(L, 3, "canonname");
    if (lua_toboolean(L, -1)) hints->ai_flags |=  AI_CANONNAME;
    lua_pop(L, 1);
  }

  req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);

  ret = uv_getaddrinfo(luv_loop(L), req, ref == LUA_NOREF ? NULL : luv_getaddrinfo_cb, node, service, hints);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  if (ref == LUA_NOREF) {

    lua_pop(L, 1);
    luv_pushaddrinfo(L, req->addrinfo);
    uv_freeaddrinfo(req->addrinfo);
    luv_cleanup_req(L, req->data);
  }
  return 1;
}

static void luv_getnameinfo_cb(uv_getnameinfo_t* req, int status, const char* hostname, const char* service) {
  lua_State* L = luv_state(req->loop);

  int nargs;

  if (status < 0) {
    luv_status(L, status);
    nargs = 1;
  }
  else {
    lua_pushnil(L);
    lua_pushstring(L, hostname);
    lua_pushstring(L, service);
    nargs = 3;
  }

  luv_fulfill_req(L, req->data, nargs);
  luv_cleanup_req(L, req->data);
  req->data = NULL;
}

static int luv_getnameinfo(lua_State* L) {
  uv_getnameinfo_t* req;
  struct sockaddr_storage addr;
  const char* ip = NULL;
  int flags = 0;
  int ret, ref, port = 0;

  luaL_checktype(L, 1, LUA_TTABLE);
  memset(&addr, 0, sizeof(addr));

  lua_getfield(L, 1, "ip");
  if (lua_isstring(L, -1)) {
    ip = lua_tostring(L, -1);
  }
  else if (!lua_isnil(L, -1)) {
    luaL_argerror(L, 1, "ip property must be string if set");
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "port");
  if (lua_isnumber(L, -1)) {
    port = lua_tointeger(L, -1);
  }
  else if (!lua_isnil(L, -1)) {
    luaL_argerror(L, 1, "port property must be integer if set");
  }
  lua_pop(L, 1);

  if (ip || port) {
    if (!ip) ip = "0.0.0.0";
    if (!uv_ip4_addr(ip, port, (struct sockaddr_in*)&addr)) {
      addr.ss_family = AF_INET;
    }
    else if (!uv_ip6_addr(ip, port, (struct sockaddr_in6*)&addr)) {
      addr.ss_family = AF_INET6;
    }
    else {
      return luaL_argerror(L, 1, "Invalid ip address or port");
    }
  }

  lua_getfield(L, 1, "family");
  if (lua_isnumber(L, -1)) {
    addr.ss_family = lua_tointeger(L, -1);
  }
  else if (lua_isstring(L, -1)) {
    addr.ss_family = luv_af_string_to_num(lua_tostring(L, -1));
  }
  else if (!lua_isnil(L, -1)) {
    luaL_argerror(L, 1, "family must be string if set");
  }
  lua_pop(L, 1);

  ref = lua_isnoneornil(L, 2) ? LUA_NOREF : luv_check_continuation(L, 2);

  req = lua_newuserdata(L, sizeof(*req));
  req->data = luv_setup_req(L, ref);

  ret = uv_getnameinfo(luv_loop(L), req, ref == LUA_NOREF ? NULL : luv_getnameinfo_cb, (struct sockaddr*)&addr, flags);
  if (ret < 0) {
    lua_pop(L, 1);
    return luv_error(L, ret);
  }
  if (ref == LUA_NOREF) {
    lua_pop(L, 1);
    lua_pushstring(L, req->host);
    lua_pushstring(L, req->service);
    luv_cleanup_req(L, req->data);
    return 2;
  }
  return 1;
}


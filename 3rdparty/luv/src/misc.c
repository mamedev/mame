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
#ifdef _WIN32
#include <process.h>
#endif

static int luv_guess_handle(lua_State* L) {
  uv_file file = luaL_checkinteger(L, 1);
  switch (uv_guess_handle(file)) {
#define XX(uc, lc) case UV_##uc: lua_pushstring(L, #lc); break;
  UV_HANDLE_TYPE_MAP(XX)
#undef XX
    case UV_FILE: lua_pushstring(L, "file"); break;
    default: return 0;
  }
  return 1;
}

static int luv_version(lua_State* L) {
 lua_pushinteger(L, uv_version());
 return 1;
}

static int luv_version_string(lua_State* L) {
 lua_pushstring(L, uv_version_string());
 return 1;
}

static int luv_get_process_title(lua_State* L) {
  char title[MAX_TITLE_LENGTH];
  int ret = uv_get_process_title(title, MAX_TITLE_LENGTH);
  if (ret < 0) return luv_error(L, ret);
  lua_pushstring(L, title);
  return 1;
}

static int luv_set_process_title(lua_State* L) {
  const char* title = luaL_checkstring(L, 1);
  int ret = uv_set_process_title(title);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_resident_set_memory(lua_State* L) {
  size_t rss;
  int ret = uv_resident_set_memory(&rss);
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, rss);
  return 1;
}

static int luv_uptime(lua_State* L) {
  double uptime;
  int ret = uv_uptime(&uptime);
  if (ret < 0) return luv_error(L, ret);
  lua_pushnumber(L, uptime);
  return 1;
}

static void luv_push_timeval_table(lua_State* L, const uv_timeval_t* t) {
  lua_createtable(L, 0, 2);
  lua_pushinteger(L, t->tv_sec);
  lua_setfield(L, -2, "sec");
  lua_pushinteger(L, t->tv_usec);
  lua_setfield(L, -2, "usec");
}

static int luv_getrusage(lua_State* L) {
  uv_rusage_t rusage;
  int ret = uv_getrusage(&rusage);
  if (ret < 0) return luv_error(L, ret);
  lua_createtable(L, 0, 16);
  // user CPU time used
  luv_push_timeval_table(L, &rusage.ru_utime);
  lua_setfield(L, -2, "utime");
  // system CPU time used
  luv_push_timeval_table(L, &rusage.ru_stime);
  lua_setfield(L, -2, "stime");
  // maximum resident set size
  lua_pushinteger(L, rusage.ru_maxrss);
  lua_setfield(L, -2, "maxrss");
  // integral shared memory size
  lua_pushinteger(L, rusage.ru_ixrss);
  lua_setfield(L, -2, "ixrss");
  // integral unshared data size
  lua_pushinteger(L, rusage.ru_idrss);
  lua_setfield(L, -2, "idrss");
  // integral unshared stack size
  lua_pushinteger(L, rusage.ru_isrss);
  lua_setfield(L, -2, "isrss");
  // page reclaims (soft page faults)
  lua_pushinteger(L, rusage.ru_minflt);
  lua_setfield(L, -2, "minflt");
  // page faults (hard page faults)
  lua_pushinteger(L, rusage.ru_majflt);
  lua_setfield(L, -2, "majflt");
  // swaps
  lua_pushinteger(L, rusage.ru_nswap);
  lua_setfield(L, -2, "nswap");
  // block input operations
  lua_pushinteger(L, rusage.ru_inblock);
  lua_setfield(L, -2, "inblock");
  // block output operations
  lua_pushinteger(L, rusage.ru_oublock);
  lua_setfield(L, -2, "oublock");
  // IPC messages sent
  lua_pushinteger(L, rusage.ru_msgsnd);
  lua_setfield(L, -2, "msgsnd");
  // IPC messages received
  lua_pushinteger(L, rusage.ru_msgrcv);
  lua_setfield(L, -2, "msgrcv");
  // signals received
  lua_pushinteger(L, rusage.ru_nsignals);
  lua_setfield(L, -2, "nsignals");
  // voluntary context switches
  lua_pushinteger(L, rusage.ru_nvcsw);
  lua_setfield(L, -2, "nvcsw");
  // involuntary context switches
  lua_pushinteger(L, rusage.ru_nivcsw);
  lua_setfield(L, -2, "nivcsw");
  return 1;
}

static int luv_cpu_info(lua_State* L) {
  uv_cpu_info_t* cpu_infos;
  int count, i;
  int ret = uv_cpu_info(&cpu_infos, &count);
  if (ret < 0) return luv_error(L, ret);
  lua_newtable(L);

  for (i = 0; i < count; i++) {
    lua_newtable(L);
    lua_pushstring(L, cpu_infos[i].model);
    lua_setfield(L, -2, "model");
    lua_pushnumber(L, cpu_infos[i].speed);
    lua_setfield(L, -2, "speed");
    lua_newtable(L);
    lua_pushnumber(L, cpu_infos[i].cpu_times.user);
    lua_setfield(L, -2, "user");
    lua_pushnumber(L, cpu_infos[i].cpu_times.nice);
    lua_setfield(L, -2, "nice");
    lua_pushnumber(L, cpu_infos[i].cpu_times.sys);
    lua_setfield(L, -2, "sys");
    lua_pushnumber(L, cpu_infos[i].cpu_times.idle);
    lua_setfield(L, -2, "idle");
    lua_pushnumber(L, cpu_infos[i].cpu_times.irq);
    lua_setfield(L, -2, "irq");
    lua_setfield(L, -2, "times");
    lua_rawseti(L, -2, i + 1);
  }

  uv_free_cpu_info(cpu_infos, count);
  return 1;
}

static int luv_interface_addresses(lua_State* L) {
  uv_interface_address_t* interfaces;
  int count, i;
  char ip[INET6_ADDRSTRLEN];
  char netmask[INET6_ADDRSTRLEN];

  uv_interface_addresses(&interfaces, &count);

  lua_newtable(L);

  for (i = 0; i < count; i++) {
    lua_getfield(L, -1, interfaces[i].name);
    if (!lua_istable(L, -1)) {
      lua_pop(L, 1);
      lua_newtable(L);
      lua_pushvalue(L, -1);
      lua_setfield(L, -3, interfaces[i].name);
    }
    lua_newtable(L);
    lua_pushboolean(L, interfaces[i].is_internal);
    lua_setfield(L, -2, "internal");

    lua_pushlstring(L, interfaces[i].phys_addr, sizeof(interfaces[i].phys_addr));
    lua_setfield(L, -2, "mac");

    if (interfaces[i].address.address4.sin_family == AF_INET) {
      uv_ip4_name(&interfaces[i].address.address4, ip, sizeof(ip));
      uv_ip4_name(&interfaces[i].netmask.netmask4, netmask, sizeof(netmask));
    } else if (interfaces[i].address.address4.sin_family == AF_INET6) {
      uv_ip6_name(&interfaces[i].address.address6, ip, sizeof(ip));
      uv_ip6_name(&interfaces[i].netmask.netmask6, netmask, sizeof(netmask));
    } else {
      strncpy(ip, "<unknown sa family>", INET6_ADDRSTRLEN);
      strncpy(netmask, "<unknown sa family>", INET6_ADDRSTRLEN);
    }
    lua_pushstring(L, ip);
    lua_setfield(L, -2, "ip");
    lua_pushstring(L, netmask);
    lua_setfield(L, -2, "netmask");

    lua_pushstring(L, luv_af_num_to_string(interfaces[i].address.address4.sin_family));
    lua_setfield(L, -2, "family");
    lua_rawseti(L, -2, lua_rawlen (L, -2) + 1);
    lua_pop(L, 1);
  }
  uv_free_interface_addresses(interfaces, count);
  return 1;
}

static int luv_loadavg(lua_State* L) {
  double avg[3];
  uv_loadavg(avg);
  lua_pushnumber(L, avg[0]);
  lua_pushnumber(L, avg[1]);
  lua_pushnumber(L, avg[2]);
  return 3;
}

static int luv_exepath(lua_State* L) {
  size_t size = 2*PATH_MAX;
  char exe_path[2*PATH_MAX];
  int ret = uv_exepath(exe_path, &size);
  if (ret < 0) return luv_error(L, ret);
  lua_pushlstring(L, exe_path, size);
  return 1;
}

static int luv_cwd(lua_State* L) {
  size_t size = 2*PATH_MAX;
  char path[2*PATH_MAX];
  int ret = uv_cwd(path, &size);
  if (ret < 0) return luv_error(L, ret);
  lua_pushlstring(L, path, size);
  return 1;
}

static int luv_chdir(lua_State* L) {
  int ret = uv_chdir(luaL_checkstring(L, 1));
  if (ret < 0) return luv_error(L, ret);
  lua_pushinteger(L, ret);
  return 1;
}

static int luv_os_homedir(lua_State* L) {
  size_t size = 2*PATH_MAX;
  char homedir[2*PATH_MAX];
  int ret = uv_os_homedir(homedir, &size);
  if (ret < 0) return luv_error(L, ret);
  lua_pushlstring(L, homedir, size);
  return 1;
}

static int luv_get_total_memory(lua_State* L) {
  lua_pushnumber(L, uv_get_total_memory());
  return 1;
}

static int luv_get_free_memory(lua_State* L) {
  lua_pushnumber(L, uv_get_free_memory());
  return 1;
}

static int luv_hrtime(lua_State* L) {
  lua_pushnumber(L, uv_hrtime());
  return 1;
}

static int luv_getpid(lua_State* L){
  int pid = getpid();
  lua_pushinteger(L, pid);
  return 1;
}

#ifndef _WIN32
static int luv_getuid(lua_State* L){
  int uid = getuid();
  lua_pushinteger(L, uid);
  return 1;
}

static int luv_getgid(lua_State* L){
  int gid = getgid();
  lua_pushinteger(L, gid);
  return 1;
}

static int luv_setuid(lua_State* L){
  int uid = luaL_checkinteger(L, 1);
  int r = setuid(uid);
  if (-1 == r) {
    luaL_error(L, "Error setting UID");
  }
  return 0;
}

static int luv_setgid(lua_State* L){
  int gid = luaL_checkinteger(L, 1);
  int r = setgid(gid);
  if (-1 == r) {
    luaL_error(L, "Error setting GID");
  }
  return 0;
}
#endif

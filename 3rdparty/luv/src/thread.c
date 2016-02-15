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
#include "lthreadpool.h"

typedef struct {
  uv_thread_t handle;
  char* code;
  int len;
  int argc;
  luv_thread_arg_t arg;
} luv_thread_t;

static luv_acquire_vm acquire_vm_cb = NULL;
static luv_release_vm release_vm_cb = NULL;

static lua_State* luv_thread_acquire_vm() {
  lua_State* L = luaL_newstate();

  // Add in the lua standard libraries
  luaL_openlibs(L);

  // Get package.loaded, so we can store uv in it.
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "loaded");
  lua_remove(L, -2); // Remove package

  // Store uv module definition at loaded.uv
  luaopen_luv(L);
  lua_setfield(L, -2, "luv");
  lua_pop(L, 1);

  return L;
}

static void luv_thread_release_vm(lua_State* L) {
  lua_close(L);
}

static int luv_thread_arg_set(lua_State* L, luv_thread_arg_t* args, int idx, int top, int flag)
{
  int i;
  idx = idx > 0 ? idx : 1;
  i = idx;
  while (i <= top && i <= LUV_THREAD_MAXNUM_ARG + idx)
  {
    luv_val_t *arg = args->argv + i - idx;
    arg->type = lua_type(L, i);
    switch (arg->type)
    {
    case LUA_TNIL:
      break;
    case LUA_TBOOLEAN:
      arg->val.boolean = lua_toboolean(L, i);
      break;
    case LUA_TNUMBER:
      arg->val.num = lua_tonumber(L, i);
      break;
    case LUA_TLIGHTUSERDATA:
      arg->val.userdata = lua_touserdata(L, i);
      break;
    case LUA_TSTRING:
    {
      const char* p = lua_tolstring(L, i, &arg->val.str.len);
      arg->val.str.base = malloc(arg->val.str.len);
      if (arg->val.str.base == NULL)
      {
        perror("out of memory");
        return 0;
      }
      memcpy((void*)arg->val.str.base, p, arg->val.str.len);
      break;
    }
    case LUA_TUSERDATA:
      if (flag == 1) {
        arg->val.userdata = luv_check_handle(L, i);
        break;
      }
    default:
      fprintf(stderr, "Error: thread arg not support type '%s' at %d",
        luaL_typename(L, arg->type), i);
      exit(-1);
      break;
    }
    i++;
  }
  args->argc = i - idx;
  return args->argc;
}

static void luv_thread_arg_clear(luv_thread_arg_t* args) {
  int i;
  for (i = 0; i < args->argc; i++)
  {
    if (args->argv[i].type == LUA_TSTRING)
    {
      free((void*)args->argv[i].val.str.base);
    }
  }
  memset(args, 0, sizeof(*args));
  args->argc = 0;
}

static void luv_thread_setup_handle(lua_State* L, uv_handle_t* handle) {
  *(void**) lua_newuserdata(L, sizeof(void*)) = handle;

#define XX(uc, lc) case UV_##uc:    \
    luaL_getmetatable(L, "uv_"#lc); \
    break;
  switch (handle->type) {
    UV_HANDLE_TYPE_MAP(XX)
  default:
    luaL_error(L, "Unknown handle type");
  }
#undef XX

  lua_setmetatable(L, -2);
}

static int luv_thread_arg_push(lua_State* L, const luv_thread_arg_t* args) {
  int i = 0;
  while (i < args->argc)
  {
    const luv_val_t* arg = args->argv + i;
    switch (arg->type)
    {
    case LUA_TNIL:
      lua_pushnil(L);
      break;
    case LUA_TBOOLEAN:
      lua_pushboolean(L, arg->val.boolean);
      break;
    case LUA_TLIGHTUSERDATA:
      lua_pushlightuserdata(L, arg->val.userdata);
      break;
    case LUA_TNUMBER:
      lua_pushnumber(L, arg->val.num);
      break;
    case LUA_TSTRING:
      lua_pushlstring(L, arg->val.str.base, arg->val.str.len);
      break;
    case LUA_TUSERDATA:
      luv_thread_setup_handle(L, arg->val.userdata);
      break;
    default:
      fprintf(stderr, "Error: thread arg not support type %s at %d",
        luaL_typename(L, arg->type), i + 1);
    }
    i++;
  };
  return i;
}

int thread_dump(lua_State* L, const void* p, size_t sz, void* B)
{
  (void)L;
  luaL_addlstring((luaL_Buffer*) B, (const char*) p, sz);
  return 0;
}

static const char* luv_thread_dumped(lua_State* L, int idx, size_t* l) {
  if (lua_isstring(L, idx)) {
    return lua_tolstring(L, idx, l);
  } else {
    const char* buff = NULL;
    int top = lua_gettop(L);
    luaL_Buffer b;
    luaL_checktype(L, idx, LUA_TFUNCTION);
    lua_pushvalue(L, idx);
    luaL_buffinit(L, &b);
#if LUA_VERSION_NUM>=503
    int test_lua_dump = (lua_dump(L, thread_dump, &b, 1) == 0);
#else
    int test_lua_dump = (lua_dump(L, thread_dump, &b) == 0);
#endif
    if (test_lua_dump) {
      luaL_pushresult(&b);
      buff = lua_tolstring(L, -1, l);
    } else
      luaL_error(L, "Error: unable to dump given function");
    lua_settop(L, top);

    return buff;
  }
}

static luv_thread_t* luv_check_thread(lua_State* L, int index)
{
  luv_thread_t* thread = luaL_checkudata(L, index, "uv_thread");
  return thread;
}

static int luv_thread_gc(lua_State* L) {
  luv_thread_t* tid = luv_check_thread(L, 1);
  free(tid->code);
  tid->code = NULL;
  tid->len = 0;
  luv_thread_arg_clear(&tid->arg);
  return 0;
}

static int luv_thread_tostring(lua_State* L)
{
  luv_thread_t* thd = luv_check_thread(L, 1);
  lua_pushfstring(L, "uv_thread_t: %p", thd->handle);
  return 1;
}

static void luv_thread_cb(void* varg) {
  luv_thread_t* thd = (luv_thread_t*)varg;
  lua_State* L = acquire_vm_cb();
  if (luaL_loadbuffer(L, thd->code, thd->len, "=thread") == 0)
  {
    int top = lua_gettop(L);
    int i = luv_thread_arg_push(L, &thd->arg);

    for (i = 0; i < thd->arg.argc; i++) {
      if (thd->arg.argv[i].type == LUA_TUSERDATA) {
        lua_pushlightuserdata(L, thd->arg.argv[i].val.userdata);
        lua_pushvalue(L, top + i + 1);
        lua_rawset(L, LUA_REGISTRYINDEX);
      }
    }

    if (lua_pcall(L, i, 0, 0)) {
      fprintf(stderr, "Uncaught Error in thread: %s\n", lua_tostring(L, -1));
    }

    for (i = 0; i < thd->arg.argc; i++) {
      if (thd->arg.argv[i].type == LUA_TUSERDATA) {
        lua_pushlightuserdata(L, thd->arg.argv[i].val.userdata);
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
        lua_setmetatable(L, -2);
        lua_pop(L, 1);

        lua_pushlightuserdata(L, thd->arg.argv[i].val.userdata);
        lua_pushnil(L);
        lua_rawset(L, LUA_REGISTRYINDEX);
      }
    }

  } else {
    fprintf(stderr, "Uncaught Error: %s\n", lua_tostring(L, -1));
  }
  release_vm_cb(L);
}

static int luv_new_thread(lua_State* L) {
  int ret;
  size_t len;
  const char* buff;
  luv_thread_t* thread;
  thread = lua_newuserdata(L, sizeof(*thread));
  memset(thread, 0, sizeof(*thread));
  luaL_getmetatable(L, "uv_thread");
  lua_setmetatable(L, -2);

  buff = luv_thread_dumped(L, 1, &len);

  thread->argc = luv_thread_arg_set(L, &thread->arg, 2, lua_gettop(L) - 1, 1);
  thread->len = len;
  thread->code = malloc(thread->len);
  memcpy(thread->code, buff, len);

  ret = uv_thread_create(&thread->handle, luv_thread_cb, thread);
  if (ret < 0) return luv_error(L, ret);

  return 1;
}

static int luv_thread_join(lua_State* L) {
  luv_thread_t* tid = luv_check_thread(L, 1);
  int ret = uv_thread_join(&tid->handle);
  if (ret < 0) return luv_error(L, ret);
  lua_pushboolean(L, 1);
  return 1;
}

static int luv_thread_self(lua_State* L)
{
  luv_thread_t* thread;
  uv_thread_t t = uv_thread_self();
  thread = lua_newuserdata(L, sizeof(*thread));
  memset(thread, 0, sizeof(*thread));
  memcpy(&thread->handle, &t, sizeof(t));
  luaL_getmetatable(L, "uv_thread");
  lua_setmetatable(L, -2);
  return 1;
}

static int luv_thread_equal(lua_State* L) {
  luv_thread_t* t1 = luv_check_thread(L, 1);
  luv_thread_t* t2 = luv_check_thread(L, 2);
  int ret = uv_thread_equal(&t1->handle, &t2->handle);
  lua_pushboolean(L, ret);
  return 1;
}

/* Pause the calling thread for a number of milliseconds. */
static int luv_thread_sleep(lua_State* L) {
#ifdef _WIN32
  DWORD msec = luaL_checkinteger(L, 1);
  Sleep(msec);
#else
  lua_Integer msec = luaL_checkinteger(L, 1);
  usleep(msec * 1000);
#endif
  return 0;
}

static const luaL_Reg luv_thread_methods[] = {
  {"equal", luv_thread_equal},
  {"join", luv_thread_join},
  {NULL, NULL}
};

static void luv_thread_init(lua_State* L) {
  luaL_newmetatable(L, "uv_thread");
  lua_pushcfunction(L, luv_thread_tostring);
  lua_setfield(L, -2, "__tostring");
  lua_pushcfunction(L, luv_thread_equal);
  lua_setfield(L, -2, "__eq");
  lua_pushcfunction(L, luv_thread_gc);
  lua_setfield(L, -2, "__gc");
  lua_newtable(L);
  luaL_setfuncs(L, luv_thread_methods, 0);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);

  if (acquire_vm_cb == NULL) acquire_vm_cb = luv_thread_acquire_vm;
  if (release_vm_cb == NULL) release_vm_cb = luv_thread_release_vm;
}

LUALIB_API void luv_set_thread_cb(luv_acquire_vm acquire, luv_release_vm release)
{
  acquire_vm_cb = acquire;
  release_vm_cb = release;
}

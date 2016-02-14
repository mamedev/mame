#!/bin/sh
# Verifies that luv will cleanup libuv process handles correctly even if
# not done by "userspace".
# Details: https://github.com/luvit/luv/issues/193

# This test modifies one of the examples to skip libuv process cleanup,
# purposely making it leave SIGCHLD signal handler.
# 
patch -p1 << "EOF"
diff --git a/examples/talking-to-children.lua b/examples/talking-to-children.lua
index 10a53ef..6c6c53f 100644
--- a/examples/talking-to-children.lua
+++ b/examples/talking-to-children.lua
@@ -41,7 +41,3 @@ uv.read_start(stdout, onread)
 uv.read_start(stderr, onread)
 uv.write(stdin, "Hello World")
 uv.shutdown(stdin, onshutdown)
-
-uv.run()
-uv.walk(uv.close)
-uv.run()
EOF

# It also requires a patched lua standalone interpreter that sends SIGCHLD to
# itself after calling lua_close, which would have freed all memory of the libuv
# event loop associated with the lua state.
(
cd deps/lua
patch -p1 << "EOF"
diff --git a/src/lua.c b/src/lua.c
index 7a47582..4dc19d5 100644
--- a/src/lua.c
+++ b/src/lua.c
@@ -608,6 +608,7 @@ int main (int argc, char **argv) {
   result = lua_toboolean(L, -1);  /* get result */
   report(L, status);
   lua_close(L);
+  kill(0, SIGCHLD);
   return (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
 }
EOF
)

WITH_LUA_ENGINE=Lua make
./build/lua examples/talking-to-children.lua

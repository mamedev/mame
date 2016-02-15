local p = require('lib/utils').prettyPrint
local uv = require('luv')

local function create_server(host, port, on_connection)

  local server = uv.new_tcp()
  p(1, server)
  uv.tcp_bind(server, host, port)

  uv.listen(server, 128, function(err)
    assert(not err, err)
    local client = uv.new_tcp()
    uv.accept(server, client)
    on_connection(client)
  end)

  return server
end

local server = create_server("0.0.0.0", 0, function (client)
  p("new client", client, uv.tcp_getsockname(client), uv.tcp_getpeername(client))
  uv.read_start(client, function (err, chunk)
    p("onread", {err=err,chunk=chunk})

    -- Crash on errors
    assert(not err, err)

    if chunk then
      -- Echo anything heard
      uv.write(client, chunk)
    else
      -- When the stream ends, close the socket
      uv.close(client)
    end
  end)
end)

local address = uv.tcp_getsockname(server)
p("server", server, address)

local client = uv.new_tcp()
uv.tcp_connect(client, "127.0.0.1", address.port, function (err)
  assert(not err, err)

  uv.read_start(client, function (err, chunk)
    p("received at client", {err=err,chunk=chunk})
    assert(not err, err)
    if chunk then
      uv.shutdown(client)
      p("client done shutting down")
    else
      uv.close(client)
      uv.close(server)
    end
  end)

  p("writing from client")
  uv.write(client, "Hello")
  uv.write(client, "World")

end)

-- Start the main event loop
uv.run()
-- Close any stray handles when done
uv.walk(uv.close)
uv.run()
uv.loop_close()

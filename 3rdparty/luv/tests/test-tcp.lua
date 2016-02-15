return require('lib/tap')(function (test)
  test("basic tcp server and client", function (print, p, expect, uv)
    local server = uv.new_tcp()
    uv.tcp_bind(server, "::", 0)
    uv.listen(server, 128, expect(function (err)
      p("server on connection", server)
      assert(not err, err)
      uv.close(server)
    end))

    local address = uv.tcp_getsockname(server)
    p{server=server,address=address}

    local client = uv.new_tcp()
    local req = uv.tcp_connect(client, "::1", address.port, expect(function (err)
      p("client on connect", client, err)
      assert(not err, err)
      uv.shutdown(client, expect(function (err)
        p("client on shutdown", client, err)
        assert(not err, err)
        uv.close(client, expect(function ()
          p("client on close", client)
        end))
      end))
    end))
    p{client=client,req=req}
  end)

  test("tcp echo server and client", function (print, p, expect, uv)
    local server = uv.new_tcp()
    assert(uv.tcp_bind(server, "127.0.0.1", 0))
    assert(uv.listen(server, 1, expect(function ()
      local client = uv.new_tcp()
      assert(uv.accept(server, client))
      assert(uv.read_start(client, expect(function (err, data)
        p("server read", {err=err,data=data})
        assert(not err, err)
        if data then
          assert(uv.write(client, data))
        else
          assert(uv.read_stop(client))
          uv.close(client)
          uv.close(server)
        end
      end, 2)))
    end)))

    local address = uv.tcp_getsockname(server)
    p{server=server,address=address}

    local socket = assert(uv.new_tcp())
    assert(uv.tcp_connect(socket, "127.0.0.1", address.port, expect(function ()
      assert(uv.read_start(socket, expect(function (err, data)
        p("client read", {err=err,data=data})
        assert(not err, err)
        assert(uv.read_stop(socket))
        uv.close(socket)
      end)))
      local req = assert(uv.write(socket, "Hello", function (err)
        p("client onwrite", socket, err)
        assert(not err, err)
      end))
      p{socket=socket,req=req}
    end)))
  end)

  test("tcp echo server and client with methods", function (print, p, expect, uv)
    local server = uv.new_tcp()
    assert(server:bind("127.0.0.1", 0))
    assert(server:listen(1, expect(function ()
      local client = uv.new_tcp()
      assert(server:accept(client))
      assert(client:read_start(expect(function (err, data)
        p("server read", {err=err,data=data})
        assert(not err, err)
        if data then
          assert(client:write(data))
        else
          assert(client:read_stop())
          client:close()
          server:close()
        end
      end, 2)))
    end)))

    local address = server:getsockname()
    p{server=server,address=address}

    local socket = assert(uv.new_tcp())
    assert(socket:connect("127.0.0.1", address.port, expect(function ()
      assert(socket:read_start(expect(function (err, data)
        p("client read", {err=err,data=data})
        assert(not err, err)
        assert(socket:read_stop())
        socket:close()
      end)))
      local req = assert(socket:write("Hello", function (err)
        p("client onwrite", socket, err)
        assert(not err, err)
      end))
      p{socket=socket,req=req}
    end)))
  end)

  test("tcp invalid ip address", function (print, p, expect, uv)
    local ip = '127.0.0.100005'
    local server = uv.new_tcp()
    local status, err = pcall(function() uv.tcp_bind(server, ip, 1000) end)
    assert(not status)
    p(err)
    assert(err:find(ip))
    uv.close(server)
  end)
end)

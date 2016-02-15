local uv = require('luv')


local client = uv.new_tcp()
uv.tcp_connect(client, "127.0.0.1", 1337, function (err)
    assert(not err, err)
    uv.read_start(client, function (err, chunk)
      assert(not err, err)
      if chunk then
        print(chunk)
      else
        uv.close(client)
      end
    end)

    uv.write(client, "Hello")
    uv.write(client, "World")
end)
print('CTRL-C to break')
uv.run('default')
uv.loop_close()

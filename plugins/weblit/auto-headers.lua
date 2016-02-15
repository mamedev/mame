

--[[

Response automatic values:
 - Auto Server header
 - Auto Date Header
 - code defaults to 404 with body "Not Found\n"
 - if there is a string body add Content-Length and ETag if missing
 - if string body and no Content-Type, use text/plain for valid utf-8, application/octet-stream otherwise
 - Auto add "; charset=utf-8" to Content-Type when body is known to be valid utf-8
 - Auto 304 responses for if-none-match requests
 - Auto strip body with HEAD requests
 - Auto chunked encoding if body with unknown length
 - if Connection header set and not keep-alive, set res.keepAlive to false
 - Add Connection Keep-Alive/Close if not found based on res.keepAlive

--TODO: utf8 scanning

]]

--local digest = require('openssl').digest.digest
local date = require('os').date

return function (req, res, go)
  local isHead = false
  if req.method == "HEAD" then
    req.method = "GET"
    isHead = true
  end

  local requested = req.headers["if-none-match"]

  go()

  -- We could use the fancy metatable, but this is much faster
  local lowerHeaders = {}
  local headers = res.headers
  for i = 1, #headers do
    local key, value = unpack(headers[i])
    lowerHeaders[key:lower()] = value
  end


  if not lowerHeaders.server then
    headers[#headers + 1] = {"Server", serverName}
  end
  if not lowerHeaders.date then
    headers[#headers + 1] = {"Date", date("!%a, %d %b %Y %H:%M:%S GMT")}
  end

  if not lowerHeaders.connection then
    if req.keepAlive then
      lowerHeaders.connection = "Keep-Alive"
      headers[#headers + 1] = {"Connection", "Keep-Alive"}
    else
      headers[#headers + 1] = {"Connection", "Close"}
    end
  end
  res.keepAlive = lowerHeaders.connection and lowerHeaders.connection:lower() == "keep-alive"

  local body = res.body
  if body then
    local needLength = not lowerHeaders["content-length"] and not lowerHeaders["transfer-encoding"]
    if type(body) == "string" then
      if needLength then
        headers[#headers + 1] = {"Content-Length", #body}
      end
  --    if not lowerHeaders.etag then
    --    local etag = '"' .. digest("sha1", body) .. '"'
      --  lowerHeaders.etag = etag
        --headers[#headers + 1] = {"ETag", etag}
     -- end
    else
      if needLength then
        headers[#headers + 1] = {"Transfer-Encoding", "chunked"}
      end
    end
    if not lowerHeaders["content-type"] then
      headers[#headers + 1] = {"Content-Type", "text/plain"}
    end
  end

  local etag = lowerHeaders.etag
  if requested and res.code >= 200 and res.code < 300 and requested == etag then
    res.code = 304
    body = nil
  end

  if isHead then body = nil end
  res.body = body
end

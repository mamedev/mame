fs = import('fs')

api_header_deps

foreach header : api_headers
  api_header_deps += fs.copyfile(header[1], header[0])
endforeach

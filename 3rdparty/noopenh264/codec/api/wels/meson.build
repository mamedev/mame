headers = [
  'codec_api.h',
  'codec_app_def.h',
  'codec_def.h',
  'codec_ver.h',
]

foreach header : headers
  api_headers += [[header, files(header)]]
endforeach

install_headers(headers,
  subdir: 'wels')

import io
import os.path
import sys
import tarfile
import time
import zipfile

def read_file(path, use_crlf):
	with open(path, 'rb') as file:
		data = file.read()

	if b'\0' not in data:
		data = data.replace(b'\r', b'')
		if use_crlf:
			data = data.replace(b'\n', b'\r\n')

	return data

def write_zip(target, arcprefix, timestamp, sources):
	with zipfile.ZipFile(target, 'w') as archive:
		for source in sorted(sources):
			data = read_file(source, use_crlf = True)
			path = os.path.join(arcprefix, source)
			info = zipfile.ZipInfo(path)
			info.date_time = time.localtime(timestamp)
			info.compress_type = zipfile.ZIP_DEFLATED
			info.external_attr = 0o644 << 16
			archive.writestr(info, data)

def write_tar(target, arcprefix, timestamp, sources, compression):
	with tarfile.open(target, 'w:' + compression) as archive:
		for source in sorted(sources):
			data = read_file(source, use_crlf = False)
			path = os.path.join(arcprefix, source)
			info = tarfile.TarInfo(path)
			info.size = len(data)
			info.mtime = timestamp
			archive.addfile(info, io.BytesIO(data))

if len(sys.argv) < 5:
	raise RuntimeError('Usage: python archive.py <target> <archive prefix> <timestamp> <source files>')

target, arcprefix, timestamp = sys.argv[1:4]
sources = sys.argv[4:]

# tarfile._Stream._init_write_gz always writes current time to gzip header
time.time = lambda: timestamp

if target.endswith('.zip'):
	write_zip(target, arcprefix, int(timestamp), sources)
elif target.endswith('.tar.gz') or target.endswith('.tar.bz2'):
	write_tar(target, arcprefix, int(timestamp), sources, compression = os.path.splitext(target)[1][1:])
else:
	raise NotImplementedError('File type not supported: ' + target)

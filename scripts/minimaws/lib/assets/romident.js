// license:BSD-3-Clause
// copyright-holders:Vas Crabb

var matched_names = new Array();
var unmatched_names = new Array();
var dump_info = Object.create(null);
var machine_info = Object.create(null);
var crc_table = new Uint32Array(256);

for (var i = 0; i < 256; i++)
{
	var crc = i;
	for (var b = 0; b < 8; b++)
		crc = (crc >>> 1) ^ ((crc & 1) ? 0xedb88320 : 0x00000000);
	crc_table[i] = crc;
}


function get_chd_sha1(header)
{
	if (header.byteLength < 16)
		return null;
	if (String.fromCharCode.apply(null, new Uint8Array(header, 0, 8)) != 'MComprHD')
		return null;
	var view = new DataView(header);
	var headerlen = view.getUint32(8, false);
	var version = view.getUint32(12, false);
	var sha1offs;
	if (version == 3)
	{
		if (headerlen != 120)
			return null;
		sha1offs = 80;
	}
	else if (version == 4)
	{
		if (headerlen != 108)
			return null;
		sha1offs = 48;
	}
	else if (version == 5)
	{
		if (headerlen != 124)
			return null;
		sha1offs = 84;
	}
	else
	{
		return null;
	}
	if (header.byteLength < (sha1offs + 20))
		return null;
	return Array.from(new Uint8Array(header, sha1offs, 20)).map(x => x.toString(16).padStart(2, '0')).join('');
}


function get_sha1_group(sha1)
{
	if (Object.hasOwnProperty.call(dump_info, sha1))
	{
		return dump_info[sha1];
	}
	else
	{
		var result = new Object();
		result.crc = Object.create(null);
		dump_info[sha1] = result;
		return result;
	}
}


function add_matches(table, names, matches)
{
	for (var i = 0; i < names.length; i++)
	{
		var row = table.appendChild(document.createElement('tr'));
		var name = row.appendChild(document.createElement('th'));
		name.textContent = names[i];
		if (matches !== null)
		{
			name.setAttribute('rowspan', matches.length);
			row.appendChild(document.createElement('td')).textContent = matches[0].name;
			var bad = row.appendChild(document.createElement('td'));
			if (matches[0].bad)
				bad.textContent = 'BAD';
			for (var j = 1; j < matches.length; j++)
			{
				row = table.appendChild(document.createElement('tr'));
				row.appendChild(document.createElement('td')).textContent = matches[j].name;
				bad = row.appendChild(document.createElement('td'));
				if (matches[j].bad)
					bad.textContent = 'BAD';
			}
		}
	}
}


function add_unmatched(names, crc, sha1)
{
	var table;
	if (unmatched_names.length > 0)
	{
		table = document.getElementById('table-unmatched');
	}
	else
	{
		var div = document.getElementById('div-machines');
		var heading = document.body.insertBefore(document.createElement('h2'), div);
		heading.textContent = 'Unmatched';
		table = document.body.insertBefore(document.createElement('table'), div);
		table.setAttribute('id', 'table-unmatched');
	}
	var row = table.appendChild(document.createElement('tr'));
	var content;
	if (crc === null)
	{
		content = 'SHA1(' + sha1 + ')';
	}
	else
	{
		if (crc < 0)
			crc = 0xffffffff + crc + 1;
		content = 'CRC(' + crc.toString(16).padStart(8, '0') + ') SHA1(' + sha1 + ')';
	}
	for (var i = 0; i < names.length; i++)
	{
		row.appendChild(document.createElement('th')).textContent = names[i];
		row.appendChild(document.createElement('td')).textContent = content;
		unmatched_names.push(names[i]);
	}
}


function get_machine_table(shortname, description)
{
	if (Object.hasOwnProperty.call(machine_info, shortname))
	{
		return machine_info[shortname];
	}
	else
	{
		var div = document.getElementById('div-machines');
		var heading = div.appendChild(document.createElement('h2'));
		var link = heading.appendChild(document.createElement('a'));
		link.textContent = description;
		link.setAttribute('href', appurl + 'machine/' + encodeURIComponent(shortname));
		var table = div.appendChild(document.createElement('table'));
		machine_info[shortname] = table;
		add_matches(table, matched_names, null);
		return table;
	}
}


function request_dumps(name, group, crc, sha1, url, progress)
{
	var req = new XMLHttpRequest();
	req.responseType = 'json';
	req.onload =
			function ()
			{
				if (req.status == 200)
				{
					var machines = Object.create(null);
					var matched = Object.keys(req.response);
					if (matched.length > 0)
					{
						Object.keys(machine_info).forEach(
								function (shortname)
								{
									var table = machine_info[shortname];
									if (Object.hasOwnProperty.call(req.response, shortname))
									{
										machines[shortname] = req.response[shortname].matches;
										add_matches(table, group.names, req.response[shortname].matches);
									}
									else
									{
										add_matches(table, group.names, null);
									}
								});
						matched.forEach(
								function (shortname)
								{
									if (!Object.hasOwnProperty.call(machine_info, shortname))
									{
										var info = req.response[shortname];
										var table = get_machine_table(shortname, info.description);
										machines[shortname] = req.response[shortname].matches;
										add_matches(table, group.names, info.matches);
									}
								});
						for (var i = 0; i < group.names.length; i++)
							matched_names.push(group.names[i]);
					}
					else
					{
						add_unmatched(group.names, crc, sha1);
					}
					group.machines = machines;
					progress.parentNode.removeChild(progress);
				}
				else
				{
					progress.textContent = 'Error identifying \u201C' + name + '\u201D: HTTP status code ' + req.status.toString(10);
					if (crc !== null)
						delete dump_info[sha1].crc[crc];
					else
						delete dump_info[sha1].disk;
				}
			};
	req.onerror =
			function ()
			{
				progress.textContent = 'Error identifying \u201C' + name + '\u201D';
				if (crc !== null)
					delete dump_info[sha1].crc[crc];
				else
					delete dump_info[sha1].disk;
			};
	req.onabort =
			function ()
			{
				progress.textContent = 'Error identifying \u201C' + name + '\u201D: server request aborted';
				if (crc !== null)
					delete dump_info[sha1].crc[crc];
				else
					delete dump_info[sha1].disk;
			};
	req.open('GET', url);
	req.send();
}


function add_name(group, name, crc, sha1)
{
	if (group.names.indexOf(name) < 0)
	{
		group.names.push(name);
		if (group.hasOwnProperty('machines'))
		{
			var machines = group.machines;
			var names = [ name ];
			if (Object.keys(machines).length > 0)
			{
				Object.keys(machine_info).forEach(
						function (shortname)
						{
							var table = machine_info[shortname];
							if (Object.hasOwnProperty.call(machines, shortname))
								add_matches(table, names, machines[shortname]);
							else
								add_matches(table, names, null);
						});
				matched_names.push(name);
			}
			else
			{
				add_unmatched(names, crc, sha1);
			}
		}
	}
}


function identify_file(file, trychd, progress)
{
	var digested = 0;
	var crc = 0xffffffff;
	var sha1st = new Uint32Array(5);
	var sha1cnt = new Uint32Array(2);
	var sha1buf = new DataView(new ArrayBuffer(64));

	function rol(x, n)
	{
		return ((x << n) | (x >>> (32 - n))) & 0xffffffff;
	}

	function sha1_process(data)
	{
		var d = new Uint32Array(sha1st);

		function b(i)
		{
			var r = data.getUint32(((i + 13) & 15) << 2, false);
			r ^= data.getUint32(((i + 8) & 15) << 2, false);
			r ^= data.getUint32(((i + 2) & 15) << 2, false);
			r ^= data.getUint32((i & 15) << 2, false);
			r = rol(r, 1);
			data.setUint32((i & 15) << 2, r, false);
			return r;
		}
		function r0(i)
		{
			d[i % 5] = 0xffffffff & (d[i % 5] + ((d[(i + 3) % 5] & (d[(i + 2) % 5] ^ d[(i + 1) % 5])) ^ d[(i + 1) % 5]) + data.getUint32(i << 2, false) + 0x5a827999 + rol(d[(i + 4) % 5], 5));
			d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
		}
		function r1(i)
		{
			d[i % 5] = 0xffffffff & (d[i % 5] + ((d[(i + 3) % 5] & (d[(i + 2) % 5] ^ d[(i + 1) % 5])) ^ d[(i + 1) % 5])+ b(i) + 0x5a827999 + rol(d[(i + 4) % 5], 5));
			d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
		}
		function r2(i)
		{
			d[i % 5] = 0xffffffff & (d[i % 5] + (d[(i + 3) % 5] ^ d[(i + 2) % 5] ^ d[(i + 1) % 5]) + b(i) + 0x6ed9eba1 + rol(d[(i + 4) % 5], 5));
			d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
		}
		function r3(i)
		{
			d[i % 5] = 0xffffffff & (d[i % 5] + (((d[(i + 3) % 5] | d[(i + 2) % 5]) & d[(i + 1) % 5]) | (d[(i + 3) % 5] & d[(i + 2) % 5])) + b(i) + 0x8f1bbcdc + rol(d[(i + 4) % 5], 5));
			d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
		}
		function r4(i)
		{
			d[i % 5] = 0xffffffff & (d[i % 5] + (d[(i + 3) % 5] ^ d[(i + 2) % 5] ^ d[(i + 1) % 5]) + b(i) + 0xca62c1d6 + rol(d[(i + 4) % 5], 5));
			d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
		}

		var i = 0;
		while (i < 16)
			r0(i++);
		while (i < 20)
			r1(i++);
		while (i < 40)
			r2(i++);
		while (i < 60)
			r3(i++);
		while (i < 80)
			r4(i++);
		for (i = 0; i < sha1st.length; i++)
			sha1st[i] = (sha1st[i] + d[i]) & 0xffffffff;
	}

	function sha1_digest(data, view)
	{
		var residual = sha1cnt[0];
		sha1cnt[0] = (sha1cnt[0] + (view.length << 3)) & 0xffffffff;
		if (residual > sha1cnt[0])
			sha1cnt[1]++;
		sha1cnt[1] += (view.length >>> 29);
		residual = (residual >>> 3) & 63;
		var offset = 0;
		if ((residual + view.length) >= 64)
		{
			if (residual > 0)
			{
				for (offset = 0; (offset + residual) < 64; offset++)
					sha1buf.setUint8(offset + residual, view[offset]);
				sha1_process(sha1buf);
				residual = 0;
			}
			for ( ; (view.length - offset) >= 64; offset += 64)
				sha1_process(new DataView(data, offset, 64));
		}
		for ( ; offset < view.length; residual++, offset++)
			sha1buf.setUint8(residual, view[offset]);
	}

	function sha1_finalise()
	{
		var lenbuf = new ArrayBuffer(8);
		var lenview = new DataView(lenbuf);
		lenview.setUint32(0, sha1cnt[1], false);
		lenview.setUint32(4, sha1cnt[0], false);
		var padbuf = new ArrayBuffer(64 - (63 & ((sha1cnt[0] >>> 3) + 8)));
		var padview = new Uint8Array(padbuf);
		padview[0] = 0x80;
		for (var i = 1; i < padview.length; i++)
			padview[i] = 0x00;
		sha1_digest(padbuf, padview);
		sha1_digest(lenbuf, new Uint8Array(lenbuf));
		var result = new Array(20);
		for (var i = 0; i < 20; i++)
			result[i] = (sha1st[4 - (i >>> 2)] >> ((3 - (i & 3)) << 3)) & 0x000000ff;
		return result.map(x => x.toString(16).padStart(2, '0')).join('');
	}

	function process_chunk(e)
	{
		if (e.target.error === null)
		{
			var data = e.target.result;
			if ((digested == 0) && trychd)
			{
				var chdsha1 = get_chd_sha1(data);
				if (chdsha1 !== null)
				{
					var sha1grp = get_sha1_group(chdsha1);
					if (!sha1grp.hasOwnProperty('disk'))
					{
						var diskgrp = new Object();
						diskgrp.names = [ file.name ];
						sha1grp.disk = diskgrp;
						request_dumps(file.name, diskgrp, null, chdsha1, appurl + 'rpc/diskdumps?sha1=' + chdsha1, progress);
					}
					else
					{
						add_name(sha1grp.disk, file.name, crc, sha1);
						progress.parentNode.removeChild(progress);
					}
					return;
				}
			}
			var view = new Uint8Array(data);
			for (var i = 0; i < view.length; i++)
				crc = (crc >>> 8) ^ crc_table[(crc & 0x000000ff) ^ view[i]];
			sha1_digest(data, view);
			digested += data.byteLength;
			if (digested < file.size)
			{
				read_block();
			}
			else
			{
				crc ^= 0xffffffff;
				var sha1 = sha1_finalise();
				var sha1grp = get_sha1_group(sha1);
				if (!Object.hasOwnProperty.call(sha1grp.crc, crc))
				{
					var crcgrp = new Object();
					crcgrp.names = [ file.name ];
					sha1grp.crc[crc] = crcgrp;
					var crcstr = ((crc < 0) ? (0xffffffff + crc + 1) : crc).toString(16).padStart(8, '0');
					request_dumps(file.name, crcgrp, crc, sha1, appurl + 'rpc/romdumps?crc=' + crcstr + '&sha1=' + sha1, progress);
				}
				else
				{
					add_name(sha1grp.crc[crc], file.name, crc, sha1);
					progress.parentNode.removeChild(progress);
				}
			}
		}
		else
		{
			progress.textContent = 'Error identifying \u201C' + file.name + '\u201D: ' + e.target.error;
		}
	}

	function read_block()
	{
		var remaining = file.size - digested;
		var chunk = file.slice(digested, digested + Math.min(remaining, 65536));
		var reader = new FileReader();
		reader.onload = process_chunk;
		reader.readAsArrayBuffer(chunk);
	};

	sha1st[0] = 0xc3d2e1f0;
	sha1st[1] = 0x10325476;
	sha1st[2] = 0x98badcfe;
	sha1st[3] = 0xefcdab89;
	sha1st[4] = 0x67452301;
	sha1cnt[0] = 0;
	sha1cnt[1] = 0;
	progress.textContent = 'Identifying \u201C' + file.name + '\u201D\u2026';
	read_block();
}


function add_dump_files(files)
{
	var prog = document.getElementById('div-progress');
	for (var i = 0; i < files.length; i++)
	{
		var name = files[i].name;
		var dot = name.lastIndexOf('.');
		var trychd = (dot > 0) && (name.substring(dot + 1).toLowerCase() == 'chd');
		identify_file(files[i], trychd, prog.appendChild(document.createElement('p')));
	}
}


function div_dropzone_dragover(e)
{
	e.stopPropagation();
	e.preventDefault();
	e.dataTransfer.dropEffect = 'link';
}


function div_dropzone_drop(e)
{
	e.stopPropagation();
	e.preventDefault();
	add_dump_files(e.dataTransfer.files);
}

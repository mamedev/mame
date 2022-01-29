// license:BSD-3-Clause
// copyright-holders:Vas Crabb

var matched_names = new Array();
var unmatched_names = new Array();
var dump_info = Object.create(null);
var machine_info = Object.create(null);
var softwarelist_info = Object.create(null);
var software_info = Object.create(null);


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


function add_software_matches(table, names, part, matches)
{
	for (var i = 0; i < names.length; i++)
	{
		var row = table.appendChild(document.createElement('tr'));
		var name = row.appendChild(document.createElement('th'));
		name.textContent = names[i];
		if (matches !== null)
		{
			name.setAttribute('rowspan', matches.length);
			row.appendChild(document.createElement('td')).textContent = part;
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
		var div = document.getElementById('div-unmatched');
		var heading = div.appendChild(document.createElement('h2'));
		heading.textContent = 'Unmatched';
		table = div.appendChild(document.createElement('table'));
		table.setAttribute('id', 'table-unmatched');
		make_collapsible(heading, table);
	}
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
		var row = table.appendChild(document.createElement('tr'));
		row.appendChild(document.createElement('th')).textContent = names[i];
		row.appendChild(document.createElement('td')).textContent = content;
		unmatched_names.push(names[i]);
	}
}


function add_issues(name)
{
	var div = document.getElementById('div-issues');
	var list;
	if (!div.hasChildNodes())
	{
		var heading = div.appendChild(document.createElement('h2'));
		heading.textContent = 'Potential Issues';
		list = div.appendChild(document.createElement('dl'));
		make_collapsible(heading, list);
	}
	else
	{
		list = div.lastChild;
	}
	list.appendChild(document.createElement('dt')).textContent = name;
	var table = list.appendChild(document.createElement('dd')).appendChild(document.createElement('table'));
	table.setAttribute('class', 'sysinfo');
	return table;
}


function add_stuck_bits(table, stuck)
{
	function format_stuck_bits(bits, mask)
	{
		var result = '';
		for (var i = 0; i < bits.length; i++)
		{
			if (i > 0)
				result += ' ';
			for (var j = 0; j < 8; j++)
			{
				if (!((mask[i] >> (7 - j)) & 0x01))
					result += '-';
				else if ((bits[i] >> (7 - j)) & 0x01)
					result += '1';
				else
					result += '0';
			}
		}
		var cell = document.createElement('td');
		cell.appendChild(document.createElement('tt')).textContent = result;
		return cell;
	}

	var row = table.appendChild(document.createElement('tr'));
	var header = row.appendChild(document.createElement('th'));
	header.textContent = 'Fixed data bits:';
	header.setAttribute('rowspan', stuck.length);
	row.appendChild(format_stuck_bits(stuck[0].bits, stuck[0].mask));
	for (var i = 1; i < stuck.length; i++)
	{
		row = table.appendChild(document.createElement('tr'));
		row.appendChild(format_stuck_bits(stuck[i].bits, stuck[i].mask));
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
		if (!div.hasChildNodes())
			div.appendChild(document.createElement('h2')).textContent = 'Machines';
		var heading = div.appendChild(document.createElement('h3'));
		var link = heading.appendChild(document.createElement('a'));
		link.textContent = description;
		link.setAttribute('href', appurl + 'machine/' + encodeURIComponent(shortname));
		var table = div.appendChild(document.createElement('table'));
		machine_info[shortname] = table;
		add_matches(table, matched_names, null);
		make_collapsible(heading, table);
		return table;
	}
}


function get_softwarelist_div(shortname, description)
{
	if (Object.hasOwnProperty.call(softwarelist_info, shortname))
	{
		return softwarelist_info[shortname];
	}
	else
	{
		software_info[shortname] = Object.create(null)
		var div = document.getElementById('div-software');
		if (!div.hasChildNodes())
			div.appendChild(document.createElement('h2')).textContent = 'Software';
		div = div.appendChild(document.createElement('div'));
		var heading = div.appendChild(document.createElement('h3'));
		var link = heading.appendChild(document.createElement('a'));
		link.textContent = description;
		link.setAttribute('href', appurl + 'softwarelist/' + encodeURIComponent(shortname));
		softwarelist_info[shortname] = div;
		return div;
	}
}


function get_software_table(softwarelist, shortname, description)
{
	if (Object.hasOwnProperty.call(software_info, softwarelist) && Object.hasOwnProperty.call(software_info[softwarelist], shortname))
	{
		return software_info[softwarelist][shortname];
	}
	else
	{
		var div = softwarelist_info[softwarelist];
		var heading = div.appendChild(document.createElement('h4'));
		var link = heading.appendChild(document.createElement('a'));
		link.textContent = description;
		link.setAttribute('href', appurl + 'softwarelist/' + encodeURIComponent(softwarelist) + '/' + encodeURIComponent(shortname));
		var table = div.appendChild(document.createElement('table'));
		software_info[softwarelist][shortname] = table;
		add_software_matches(table, matched_names, null, null);
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
					var software = Object.create(null);
					var matched = Object.keys(req.response.machines);
					var softwarelists = Object.keys(req.response.software);

					if (matched.length > 0)
					{
						Object.keys(machine_info).forEach(
								function (shortname)
								{
									var machine_table = machine_info[shortname];
									if (Object.hasOwnProperty.call(req.response.machines, shortname))
									{
										machines[shortname] = req.response.machines[shortname].matches;
										add_matches(machine_table, group.names, req.response.machines[shortname].matches);
									}
									else
									{
										add_matches(machine_table, group.names, null);
									}
								});
						if (softwarelists.length <= 0)
						{
							Object.keys(software_info).forEach(
									function (listname)
									{
										Object.keys(software_info[listname]).forEach(
												function (softwarename)
												{
													var software_table = software_info[listname][softwarename];
													add_software_matches(software_table, group.names, null, null);
												});
									});
						}
						matched.forEach(
								function (shortname)
								{
									if (!Object.hasOwnProperty.call(machine_info, shortname))
									{
										var machine_details = req.response.machines[shortname];
										var machine_table = get_machine_table(shortname, machine_details.description);
										machines[shortname] = req.response.machines[shortname].matches;
										add_matches(machine_table, group.names, machine_details.matches);
									}
								});
					}

					if (softwarelists.length > 0)
					{
						if (matched.length <= 0)
						{
							Object.keys(machine_info).forEach(
									function (shortname)
									{
										var machine_table = machine_info[shortname];
										add_matches(machine_table, group.names, null);
									});
						}
						Object.keys(software_info).forEach(
								function (listname)
								{
									var haslist = Object.hasOwnProperty.call(req.response.software, listname);
									Object.keys(software_info[listname]).forEach(
											function (softwarename)
											{
												var software_table = software_info[listname][softwarename];
												if (haslist && Object.hasOwnProperty.call(req.response.software[listname].software, softwarename))
												{
													if (!Object.hasOwnProperty.call(software, listname))
														software[listname] = Object.create(null);
													if (!Object.hasOwnProperty.call(software[listname], softwarename))
														software[listname][softwarename] = Object.create(null);
													var software_details = req.response.software[listname].software[softwarename];
													Object.keys(software_details.parts).forEach(
															function (partname)
															{
																var matches = software_details.parts[partname].matches;
																software[listname][softwarename][partname] = matches;
																add_software_matches(software_table, group.names, partname, matches);
															});
												}
												else
												{
													add_software_matches(software_table, group.names, null, null);
												}
											});
								});
						softwarelists.forEach(
								function (listname)
								{
									var haslist = Object.hasOwnProperty.call(software_info, listname);
									var softwarelist_details = req.response.software[listname];
									var softwarelist_div = get_softwarelist_div(listname, softwarelist_details.description);
									Object.keys(softwarelist_details.software).forEach(
											function (softwarename)
											{
												if (!haslist || !Object.hasOwnProperty.call(software_info[listname], softwarename))
												{
													if (!Object.hasOwnProperty.call(software, listname))
														software[listname] = Object.create(null);
													if (!Object.hasOwnProperty.call(software[listname], softwarename))
														software[listname][softwarename] = Object.create(null);
													var software_details = softwarelist_details.software[softwarename];
													var software_table = get_software_table(listname, softwarename, software_details.description);
													Object.keys(software_details.parts).forEach(
															function (partname)
															{
																var matches = software_details.parts[partname].matches;
																software[listname][softwarename][partname] = matches;
																add_software_matches(software_table, group.names, partname, matches);
															});
												}
											});
								});
					}

					if ((matched.length > 0) | (softwarelists.length > 0))
					{
						for (var i = 0; i < group.names.length; i++)
							matched_names.push(group.names[i]);
					}
					else
					{
						add_unmatched(group.names, crc, sha1);
					}

					group.machines = machines;
					group.software = software;
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

		if (group.hasOwnProperty('issues'))
		{
			var issues = group.issues;
			if (issues !== null)
				issues.parentNode.parentNode.insertBefore(document.createElement('dt'), issues.parentNode).textContent = name;
		}

		if (group.hasOwnProperty('machines'))
		{
			var machines = group.machines;
			var software = group.software;
			var names = [ name ];
			if ((Object.keys(machines).length > 0) || (Object.keys(software).length > 0))
			{
				Object.keys(machine_info).forEach(
						function (shortname)
						{
							var machine_table = machine_info[shortname];
							if (Object.hasOwnProperty.call(machines, shortname))
								add_matches(machine_table, names, machines[shortname]);
							else
								add_matches(machine_table, names, null);
						});

				Object.keys(software_info).forEach(
						function (listname)
						{
							var haslist = Object.hasOwnProperty.call(software, listname);
							Object.keys(software_info[listname]).forEach(
									function (softwarename)
									{
										var software_table = software_info[listname][softwarename];
										if (haslist && Object.hasOwnProperty.call(software[listname], softwarename))
										{
											Object.keys(software[listname][softwarename]).forEach(
													function (partname)
													{
														add_software_matches(software_table, names, partname, software[listname][softwarename][partname]);
													});
										}
										else
										{
											add_software_matches(software_table, names, null, null);
										}
									});
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
	var crcproc = new Crc32Digester();
	var sha1proc = new Sha1Digester();
	var stuckbitsproc = new StuckBitsDigester();

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
			crcproc.digest(data, view);
			sha1proc.digest(data, view);
			stuckbitsproc.digest(data, view);
			digested += data.byteLength;
			if (digested < file.size)
			{
				read_block();
			}
			else
			{
				var crc = crcproc.finalise();
				var sha1 = sha1proc.finalise();
				var stuckbits = stuckbitsproc.finalise();

				var sha1grp = get_sha1_group(sha1);
				var crcgrp;
				if (!Object.hasOwnProperty.call(sha1grp.crc, crc))
				{
					crcgrp = new Object();
					crcgrp.names = [ file.name ];
					sha1grp.crc[crc] = crcgrp;
					if (stuckbits.length)
					{
						crcgrp.issues = add_issues(file.name);
						add_stuck_bits(crcgrp.issues, stuckbits);
					}
					else
					{
						crcgrp.issues = null;
					}
					var crcstr = ((crc < 0) ? (0xffffffff + crc + 1) : crc).toString(16).padStart(8, '0');
					request_dumps(file.name, crcgrp, crc, sha1, appurl + 'rpc/romdumps?crc=' + crcstr + '&sha1=' + sha1, progress);
				}
				else
				{
					crcgrp = sha1grp.crc[crc];
					add_name(crcgrp, file.name, crc, sha1);
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

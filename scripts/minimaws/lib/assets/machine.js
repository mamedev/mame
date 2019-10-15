// license:BSD-3-Clause
// copyright-holders:Vas Crabb

var slot_info = Object.create(null);
var bios_sets = Object.create(null);
var machine_flags = Object.create(null);


function make_slot_popup_id(name) { return ('select-slot-choice-' + name).replace(/:/g, '-'); }
function get_slot_popup(name) { return document.getElementById(make_slot_popup_id(name)); }


function update_cmd_preview()
{
	var inifmt = document.getElementById('select-options-format').value == 'ini';
	var result = '';
	var first = true;
	function add_option(flag, value)
	{
		if (first)
			first = false;
		else if (inifmt)
			result += '\n';
		else
			result += ' ';

		if (inifmt)
		{
			result += flag + ' ';
			if (flag.length < 25)
				result += ' '.repeat(25 - flag.length);
			result += value;
		}
		else
		{
			result += '-' + flag + ' ';
			if (value == '')
				result += '""';
			else
				result += value;
		}
	}
	var elide_defaults = !document.getElementById('check-explicit-defaults').checked;

	// add system BIOS if applicable
	var sysbios = document.getElementById('select-system-bios');
	if (sysbios && (!elide_defaults || (sysbios.selectedOptions[0].getAttribute('data-isdefault') != 'yes')))
		add_option('bios', sysbios.value);

	// add RAM option if applicable
	var ramopt = document.getElementById('select-ram-option');
	if (ramopt && (!elide_defaults || (ramopt.selectedOptions[0].getAttribute('data-isdefault') != 'yes')))
		add_option('ramsize', ramopt.value);

	var slotslist = document.getElementById('list-slot-options');
	if (slotslist)
	{
		for (var item = slotslist.firstChild; item; item = item.nextSibling)
		{
			if (item.nodeName == 'DT')
			{
				// need to set the slot option if it has non-default card and/or non-default card BIOS
				var slotname = item.getAttribute('data-slotname');
				var selection = get_slot_popup(slotname).selectedOptions[0];
				var biospopup = document.getElementById(('select-slot-bios-' + slotname).replace(/:/g, '-'));
				var defcard = selection.getAttribute('data-isdefault') == 'yes';
				var defbios = !biospopup || (biospopup.selectedOptions[0].getAttribute('data-isdefault') == 'yes');
				if (!elide_defaults || !defcard || !defbios)
				{
					var card = selection.value;
					add_option(slotname, card + ((biospopup && (!elide_defaults || !defbios)) ? (',bios=' + biospopup.value) : ''));
				}
			}
		}
	}

	// replace the preview with appropriate element
	var target = document.getElementById('para-cmd-preview');
	var replacement = document.createElement(inifmt ? 'pre' : 'tt');
	replacement.setAttribute('id', 'para-cmd-preview');
	replacement.textContent = result;
	target.parentNode.replaceChild(replacement, target);
}


function set_default_system_bios()
{
	// search for an explicit default option
	var sysbios = document.getElementById('select-system-bios');
	var len = sysbios.options.length;
	for (var i = 0; i < len; i++)
	{
		if (sysbios.options[i].getAttribute('data-isdefault') == 'yes')
		{
			// select it and add a button for restoring it
			sysbios.selectedIndex = i;
			var dflt = make_restore_default_button('default', 'btn-def-system-bios', sysbios, i);
			sysbios.onchange = make_slot_bios_change_handler(dflt);
			sysbios.parentNode.appendChild(document.createTextNode(' '));
			sysbios.parentNode.appendChild(dflt);
			break;
		}
	}
	update_cmd_preview();
}


function set_default_ram_option()
{
	// search for an explicit default option
	var ramopt = document.getElementById('select-ram-option');
	var len = ramopt.options.length;
	for (var i = 0; i < len; i++)
	{
		if (ramopt.options[i].getAttribute('data-isdefault') == 'yes')
		{
			// select it and add a button for restoring it
			ramopt.selectedIndex = i;
			var dflt = make_restore_default_button('default', 'btn-def-ram-option', ramopt, i);
			ramopt.onchange = make_slot_bios_change_handler(dflt);
			ramopt.parentNode.appendChild(document.createTextNode(' '));
			ramopt.parentNode.appendChild(dflt);
			break;
		}
	}
	update_cmd_preview();
}


var fetch_bios_sets = (function ()
		{
			var pending = Object.create(null);
			return function (device)
			{
				if (!Object.prototype.hasOwnProperty.call(bios_sets, device) && !Object.prototype.hasOwnProperty.call(pending, device))
				{
					pending[device] = true;
					var req = new XMLHttpRequest();
					req.open('GET', appurl + 'rpc/bios/' + encodeURIComponent(device), true);
					req.responseType = 'json';
					req.onload =
							function ()
							{
								delete pending[device];
								if (req.status == 200)
								{
									bios_sets[device] = req.response;
									var slotslist = document.getElementById('list-slot-options');
									if (slotslist)
									{
										for (var item = slotslist.firstChild; item; item = item.nextSibling)
										{
											if ((item.nodeName == 'DT') && (item.getAttribute('data-slotcard') == device))
												add_bios_row(item.getAttribute('data-slotname'), item.nextSibling.firstChild, device);
										}
									}
								}
							};
					req.send();
				}
			};
		})();


var fetch_machine_flags = (function ()
		{
			var pending = Object.create(null);
			return function (device)
			{
				if (!Object.prototype.hasOwnProperty.call(machine_flags, device) && !Object.prototype.hasOwnProperty.call(pending, device))
				{
					pending[device] = true;
					var req = new XMLHttpRequest();
					req.open('GET', appurl + 'rpc/flags/' + encodeURIComponent(device), true);
					req.responseType = 'json';
					req.onload =
							function ()
							{
								delete pending[device];
								if (req.status == 200)
								{
									machine_flags[device] = req.response;
									var slotslist = document.getElementById('list-slot-options');
									if (slotslist)
									{
										for (var item = slotslist.firstChild; item; item = item.nextSibling)
										{
											if ((item.nodeName == 'DT') && (item.getAttribute('data-slotcard') == device))
												add_flag_rows(item.nextSibling.firstChild, device);
										}
									}
								}
							};
					req.send();
				}
			};
		})();


function add_flag_rows(table, device)
{
	var sorted_features = Object.keys(machine_flags[device].features).sort();
	var imperfect = [], unemulated = [];
	var len = sorted_features.length;
	for (var i = 0; i < len; i++)
		((machine_flags[device].features[sorted_features[i]].overall == 'unemulated') ? unemulated : imperfect).push(sorted_features[i]);

	function add_one(flags, title)
	{
		var len = flags.length;
		if (len > 0)
		{
			var row = document.createElement('tr');
			row.appendChild(document.createElement('th')).textContent = title;
			var cell = row.appendChild(document.createElement('td'));
			cell.textContent = flags[0];
			for (i = 1; i < len; i++)
				cell.textContent += ', ' + flags[i];
			if (table.lastChild.getAttribute('class') == 'devbios')
				table.insertBefore(row, table.lastChild);
			else
				table.appendChild(row);
		}
	}

	add_one(unemulated, 'Unemulated features:');
	add_one(imperfect, 'Imperfect features:');
}


function add_bios_row(slot, table, device)
{
	var sorted_sets = Object.keys(bios_sets[device]).sort();
	var len = sorted_sets.length;
	if (len > 0)
	{
		// create table row, add heading
		var row = document.createElement('tr');
		row.setAttribute('class', 'devbios');
		row.appendChild(document.createElement('th')).textContent = 'BIOS:';
		var cell = document.createElement('td');

		// make the BIOS popul itself
		var popup = document.createElement('select');
		popup.setAttribute('id', ('select-slot-bios-' + slot).replace(/:/g, '-'));
		for (var i = 0; i < len; i++)
		{
			var set = sorted_sets[i];
			var detail = bios_sets[device][set];
			var option = document.createElement('option');
			option.setAttribute('value', set);
			option.setAttribute('data-isdefault', detail.isdefault ? 'yes' : 'no');
			option.textContent = set + ' - ' + detail.description;
			popup.appendChild(option);
			if (detail.isdefault)
				popup.selectedIndex = i;
		}
		cell.appendChild(popup);

		// make a button to restore the default
		var dflt;
		if (popup.selectedOptions[0].getAttribute('data-isdefault') == 'yes')
		{
			dflt = make_restore_default_button('default', ('btn-def-slot-bios-' + slot).replace(/:/g, '-'), popup, popup.selectedIndex);
			cell.appendChild(document.createTextNode(' '))
			cell.appendChild(dflt);
		}

		// drop the controls into a cell, add it to the table, keep the command line preview up-to-date
		popup.onchange = make_slot_bios_change_handler(dflt);
		row.appendChild(cell);
		table.appendChild(row);
		update_cmd_preview();
	}
}


function make_slot_term(name, slot, defaults)
{
	var len, i;

	// see if we can find a default, outer layers take precedence
	var defcard = '';
	len = defaults.length;
	for (i = 0; (i < len) && (defcard == ''); i++)
	{
		if (Object.prototype.hasOwnProperty.call(defaults[i], name))
			defcard = defaults[i][name];
	}

	// create a container with the slot name and popup
	var term = document.createElement('dt');
	term.setAttribute('id', ('item-slot-choice-' + name).replace(/:/g, '-'));
	term.setAttribute('data-slotname', name);
	term.setAttribute('data-slotcard', '');
	term.textContent = name + ': ';
	var popup = document.createElement('select');
	popup.setAttribute('id', make_slot_popup_id(name));
	term.appendChild(popup);

	// add the empty slot option
	var option = document.createElement('option');
	option.setAttribute('value', '');
	option.setAttribute('data-isdefault', ('' == defcard) ? 'yes' : 'no');
	option.textContent = '-';
	popup.appendChild(option);
	popup.selectedIndex = 0;

	// add options for the cards
	var sorted_choices = Object.keys(slot).sort();
	len = sorted_choices.length;
	for (i = 0; i < len; i++)
	{
		var choice = sorted_choices[i];
		var card = slot[choice];
		option = document.createElement('option');
		option.setAttribute('value', choice);
		option.setAttribute('data-isdefault', (choice == defcard) ? 'yes' : 'no');
		option.textContent = choice + ' - ' + card.description;
		popup.appendChild(option);
		if (choice == defcard)
			popup.selectedIndex = i + 1;
	}

	// make a button for restoring the default and hook up events
	var dflt = make_restore_default_button('default', ('btn-def-slot-choice-' + name).replace(/:/g, '-'), popup, popup.selectedIndex);
	term.appendChild(document.createTextNode(' '));
	term.appendChild(dflt);
	popup.onchange = make_slot_change_handler(name, slot, defaults, dflt);
	return term;
}


function add_slot_items(root, device, defaults, slotslist, pos)
{
	// add another layer of defaults for this device
	var defvals = Object.create(null);
	for (var key in slot_info[device].defaults)
		defvals[root + key] = slot_info[device].defaults[key];
	defaults = defaults.slice();
	defaults.push(defvals);
	var defcnt = defaults.length;

	// add controls for each subslot
	var slots = slot_info[device].slots;
	var sorted_slots = Object.keys(slots).sort();
	var len = sorted_slots.length;
	for (var i = 0; i < len; i++)
	{
		var slotname = sorted_slots[i];
		var slotabs = root + slotname;
		var slot = slots[slotname];
		var term = make_slot_term(slotabs, slot, defaults);
		var def = document.createElement('dd');
		def.setAttribute('id', ('item-slot-detail-' + slotabs).replace(/:/g, '-'));
		if (pos)
		{
			slotslist.insertBefore(term, pos);
			slotslist.insertBefore(def, pos);
		}
		else
		{
			slotslist.appendChild(term);
			slotslist.appendChild(def);
		}

		// force a change event to populate subslot controls if the default isn't empty
		get_slot_popup(slotabs).dispatchEvent(new Event('change'));
	}

	update_cmd_preview();
}


function make_slot_change_handler(name, slot, defaults, dfltbtn)
{
	var selection = null;
	return function (event)
	{
		var choice = event.target.value;
		var slotslist = event.target.parentNode.parentNode;
		var def = event.target.parentNode.nextSibling;
		var slotname = event.target.parentNode.getAttribute('data-slotname');
		selection = (choice == '') ? null : slot[choice];
		dfltbtn.disabled = event.target.selectedOptions[0].getAttribute('data-isdefault') == 'yes';

		// clear out any subslots from previous selection
		var prefix = slotname + ':';
		for (var candidate = def.nextSibling; candidate && candidate.getAttribute('data-slotname').startsWith(prefix); )
		{
			var next = candidate.nextSibling;
			slotslist.removeChild(candidate);
			candidate = next.nextSibling;
			slotslist.removeChild(next);
		}

		if (selection === null)
		{
			// no selection, remove the slot card details table
			event.target.parentNode.setAttribute('data-slotcard', '');
			if (def.firstChild)
				def.removeChild(def.firstChild);
		}
		else
		{
			// stash the selected device where it's easy to find
			event.target.parentNode.setAttribute('data-slotcard', selection.device);

			// create details table and add a link to the device's page
			var tbl = document.createElement('table');
			tbl.setAttribute('class', 'sysinfo');
			var row = tbl.appendChild(document.createElement('tr'));
			row.appendChild(document.createElement('th')).textContent = 'Short name:';
			var link = row.appendChild(document.createElement('td')).appendChild(document.createElement('a'));
			link.textContent = selection.device;
			link.setAttribute('href', appurl + 'machine/' + encodeURIComponent(selection.device));

			// if we have emulation flags, populate now, otherwise fetch asynchronously
			if (!Object.prototype.hasOwnProperty.call(machine_flags, selection.device))
				fetch_machine_flags(selection.device);
			else
				add_flag_rows(tbl, selection.device);

			// if we have BIOS details, populate now, otherwise fetch asynchronously
			if (!Object.prototype.hasOwnProperty.call(bios_sets, selection.device))
				fetch_bios_sets(selection.device);
			else
				add_bios_row(slotname, tbl, selection.device);

			// drop the details table into the list
			if (def.firstChild)
				def.replaceChild(tbl, def.firstChild);
			else
				def.appendChild(tbl);

			// create controls for subslots
			add_slot_items(slotname + ':' + choice, selection.device, defaults, slotslist, def.nextSibling);
		}
		update_cmd_preview();
	};
}


function make_slot_bios_change_handler(dflt)
{
	return function (event)
	{
		if (dflt)
			dflt.disabled = event.target.selectedOptions[0].getAttribute('data-isdefault') == 'yes';
		update_cmd_preview();
	}
}


function populate_slots(machine)
{
	var placeholder = document.getElementById('para-slots-placeholder');
	var slotslist = document.createElement('dl');
	slotslist.setAttribute('id', 'list-slot-options');
	placeholder.parentNode.replaceChild(slotslist, placeholder);
	add_slot_items('', machine, [], slotslist, null);
}


function slot_retrieve_error(device)
{
	var errors;
	var placeholder = document.getElementById('para-slots-placeholder');
	if (placeholder)
	{
		errors = document.createElement('div');
		errors.setAttribute('id', 'div-slots-errors');
		placeholder.parentNode.replaceChild(errors, placeholder);
	}
	else
	{
		errors = document.getElementById('div-slots-errors');
	}
	var message = document.createElement('p');
	message.textContent = 'Error retrieving slot information for ' + device + '.';
	errors.appendChild(message);
}


function fetch_slots(machine)
{
	function make_request(device)
	{
		var req = new XMLHttpRequest();
		req.open('GET', appurl + 'rpc/slots/' + encodeURIComponent(device), true);
		req.responseType = 'json';
		req.onload =
				function ()
				{
					if (req.status == 200)
					{
						slot_info[device] = req.response;
						delete pending[device];
						for (var slotname in req.response.slots)
						{
							var slot = req.response.slots[slotname];
							for (var choice in slot)
							{
								var card = slot[choice].device
								if (!Object.prototype.hasOwnProperty.call(slot_info, card) && !Object.prototype.hasOwnProperty.call(pending, card))
								{
									pending[card] = true;
									make_request(card);
								}
							}
						}
						if (!Object.keys(pending).length)
							populate_slots(machine);
					}
					else
					{
						slot_retrieve_error(device);
					}
				};
		req.send();
	}
	var pending = Object.create(null);
	pending[machine] = true;
	make_request(machine);
}

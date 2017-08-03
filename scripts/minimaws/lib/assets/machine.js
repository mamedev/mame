// license:BSD-3-Clause
// copyright-holders:Vas Crabb

var slot_info = Object.create(null);
var machine_flags = Object.create(null);


function update_cmd_preview()
{
	var result = '';
	var first = true;
	var slotslist = document.getElementById('list-slot-options');
	if (slotslist)
	{
		for (var item = slotslist.firstChild; item; item = item.nextSibling)
		{
			if (item.nodeName == 'DT')
			{
				var selection = item.lastChild.selectedOptions[0];
				if (selection.getAttribute('data-isdefault') != 'yes')
				{
					if (first)
						first = false;
					else
						result += ' ';
					var card = selection.value;
					if (card == '')
						card = '""';
					result += '-' + item.getAttribute('data-slotname') + ' ' + card;
				}
			}
		}
	}
	document.getElementById('para-cmd-preview').textContent = result;
}


var fetch_machine_flags = (function ()
		{
			var pending = Object.create(null);
			return function (device)
			{
				if (!Object.prototype.hasOwnProperty.call(machine_flags, device) && !Object.prototype.hasOwnProperty.call(pending, device))
				{
					pending[device] = true;
					var req = new XMLHttpRequest();
					req.open('GET', appurl + 'rpc/flags/' + device, true);
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
	var len, i, row, cell;

	var sorted_features = Object.keys(machine_flags[device].features).sort();
	var imperfect = [], unemulated = [];
	len = sorted_features.length;
	for (i = 0; i < len; i++)
		((machine_flags[device].features[sorted_features[i]].overall == 'unemulated') ? unemulated : imperfect).push(sorted_features[i]);

	len = unemulated.length;
	if (len > 0)
	{
		row = table.appendChild(document.createElement('tr'));
		row.appendChild(document.createElement('th')).textContent = 'Unemulated features:';
		cell = row.appendChild(document.createElement('td'));
		cell.textContent = unemulated[0];
		for (i = 1; i < len; i++)
			cell.textContent += ', ' + unemulated[i];
	}

	len = imperfect.length;
	if (len > 0)
	{
		row = table.appendChild(document.createElement('tr'));
		row.appendChild(document.createElement('th')).textContent = 'Imperfect features:';
		cell = row.appendChild(document.createElement('td'));
		cell.textContent = imperfect[0];
		for (i = 1; i < len; i++)
			cell.textContent += ', ' + unemulated[i];
	}
}


function make_slot_term(name, slot, defaults)
{
	var len, i;

	var defcard = '';
	len = defaults.length;
	for (i = 0; (i < len) && (defcard == ''); i++)
	{
		if (Object.prototype.hasOwnProperty.call(defaults[i], name))
			defcard = defaults[i][name];
	}

	var term = document.createElement('dt');
	term.setAttribute('id', ('item-slot-choice-' + name).replace(/:/g, '-'));
	term.setAttribute('data-slotname', name);
	term.setAttribute('data-slotcard', '');
	term.textContent = name + ': ';
	var popup = document.createElement('select');
	popup.setAttribute('id', ('select-slot-choice-' + name).replace(/:/g, '-'));
	term.appendChild(popup);
	var option = document.createElement('option');
	option.setAttribute('value', '');
	option.setAttribute('data-isdefault', ('' == defcard) ? 'yes' : 'no');
	option.textContent = '-';
	popup.appendChild(option);
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
	}
	popup.selectedIndex = 0;
	popup.onchange = make_slot_change_handler(name, slot, defaults);
	return term;
}


function add_slot_items(root, device, defaults, slotslist, pos)
{
	var defvals = Object.create(null);
	for (var key in slot_info[device].defaults)
		defvals[root + key] = slot_info[device].defaults[key];
	defaults = defaults.slice();
	defaults.push(defvals);
	var defcnt = defaults.length;

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

		for (var j = 0; j < defcnt; j++)
		{
			if (Object.prototype.hasOwnProperty.call(defaults[j], slotabs))
			{
				var card = defaults[j][slotabs];
				var sel = term.lastChild;
				var found = false;
				var choice;
				for (choice in sel.options)
				{
					if (sel.options[choice].value == card)
					{
						found = true;
						break;
					}
				}
				if (found)
				{
					sel.selectedIndex = choice;
					sel.dispatchEvent(new Event('change'));
					break;
				}
			}
		}
	}

	update_cmd_preview();
}


function make_slot_change_handler(name, slot, defaults)
{
	var selection = null;
	return function (event)
	{
		var choice = event.target.value;
		var slotslist = event.target.parentNode.parentNode;
		var def = event.target.parentNode.nextSibling;
		var slotname = event.target.parentNode.getAttribute('data-slotname');
		selection = (choice == '') ? null : slot[choice];

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
			event.target.parentNode.setAttribute('data-slotcard', '');
			if (def.firstChild)
				def.removeChild(def.firstChild);
		}
		else
		{
			event.target.parentNode.setAttribute('data-slotcard', selection.device);
			var tbl = document.createElement('table');
			tbl.setAttribute('class', 'sysinfo');

			var row = tbl.appendChild(document.createElement('tr'));
			row.appendChild(document.createElement('th')).textContent = 'Short name:';
			var link = row.appendChild(document.createElement('td')).appendChild(document.createElement('a'));
			link.textContent = selection.device;
			link.setAttribute('href', appurl + 'machine/' + selection.device);

			if (!Object.prototype.hasOwnProperty.call(machine_flags, selection.device))
				fetch_machine_flags(selection.device);
			else
				add_flag_rows(tbl, selection.device);

			if (def.firstChild)
				def.replaceChild(tbl, def.firstChild);
			else
				def.appendChild(tbl);

			add_slot_items(slotname + ':' + choice, selection.device, defaults, slotslist, def.nextSibling);
		}
		update_cmd_preview();
	};
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
		req.open('GET', appurl + 'rpc/slots/' + device, true);
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

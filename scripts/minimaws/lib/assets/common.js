// license:BSD-3-Clause
// copyright-holders:Vas Crabb

function sort_table(tbl, col, dir, numeric)
{
	var tbody = tbl.tBodies[0];
	var trows = Array.prototype.slice.call(tbody.rows, 0).sort(
			function (x, y)
			{
				if (numeric)
					return dir * (parseInt(x.cells[col].textContent) - parseInt(y.cells[col].textContent));
				else
					return dir * x.cells[col].textContent.localeCompare(y.cells[col].textContent);
			})
	trows.forEach(function (row) { tbody.appendChild(row); });
}


function make_table_sortable(tbl)
{
	var headers = tbl.tHead.rows[0].cells;
	for (var i = 0; i < headers.length; i++)
	{
		(function (col)
		{
			var dir = 1;
			var sorticon = document.createElement('img');
			sorticon.setAttribute('src', assetsurl + '/sortind.png');
			sorticon.style.cssFloat = 'right';
			sorticon.style.marginLeft = '0.5em';
			headers[col].appendChild(sorticon);
			headers[col].addEventListener(
					'click',
					function (event)
					{
						var imgsrc = sorticon.getAttribute('src');
						imgsrc = imgsrc.substr(imgsrc.lastIndexOf('/') + 1);
						if (imgsrc != 'sortind.png')
							dir = -dir;
						if (dir < 0)
							sorticon.setAttribute('src', assetsurl + '/sortdesc.png');
						else
							sorticon.setAttribute('src', assetsurl + '/sortasc.png');
						for (var i = 0; i < headers.length; i++)
						{
							if (i != col)
								headers[i].lastChild.setAttribute('src', assetsurl + '/sortind.png');
						}
						sort_table(tbl, col, dir, headers[col].getAttribute('class') == 'numeric');
					});
		}(i));
	}
}


function make_restore_default_handler(popup, index)
{
	return function (event)
	{
		if (popup.selectedIndex != index)
		{
			popup.selectedIndex = index;
			popup.dispatchEvent(new Event('change'));
		}
	}
}


function make_restore_default_button(title, id, popup, index)
{
	var btn = document.createElement('button');
	btn.setAttribute('id', id);
	btn.setAttribute('type', 'button');
	btn.disabled = popup.selectedIndex == index;
	btn.textContent = title;
	btn.onclick = make_restore_default_handler(popup, index);
	return btn;
}

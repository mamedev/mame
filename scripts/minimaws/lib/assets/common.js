// license:BSD-3-Clause
// copyright-holders:Vas Crabb

function make_table_sortable(tbl)
{
	var sorticons = new Array(tbl.tHead.rows[0].cells.length);

	function TableSortHelper(i)
	{
		this.column = i;
		this.header = tbl.tHead.rows[0].cells[i]
		this.sorted = false;
		this.direction = 1;

		this.icon = document.createElement('img');
		this.icon.setAttribute('src', assetsurl + '/sortind.svg');
		this.icon.addEventListener('click', this.icon_click.bind(this));

		var container = document.createElement('div');
		container.setAttribute('class', 'sorticon');
		container.appendChild(this.icon);
		this.header.appendChild(container);
	}

	TableSortHelper.prototype = (function ()
			{
				function set_unsorted(obj)
				{
					obj.sorted = false;
					obj.icon.setAttribute('src', assetsurl + '/sortind.svg');
				}

				function sort_table(obj)
				{
					var c = obj.header.cellIndex;
					var numeric = obj.header.getAttribute('class') == 'numeric';
					var tbody = tbl.tBodies[0];
					var trows;
					if (numeric)
					{
						trows = Array.prototype.slice.call(tbody.rows, 0).sort(
								function (x, y)
								{
									return obj.direction * (parseFloat(x.cells[c].textContent) - parseFloat(y.cells[c].textContent));
								});
					}
					else
					{
						trows = Array.prototype.slice.call(tbody.rows, 0).sort(
								function (x, y)
								{
									return obj.direction * x.cells[c].textContent.localeCompare(y.cells[c].textContent);
								});
					}
					trows.forEach(function (row) { tbody.appendChild(row); });
				}

				return {
					icon_click : function (event)
					{
						if (this.sorted)
							this.direction = -this.direction;
						if (this.direction < 0)
							this.icon.setAttribute('src', assetsurl + '/sortdesc.svg');
						else
							this.icon.setAttribute('src', assetsurl + '/sortasc.svg');
						this.sorted = true;
						for (var i = 0; i < sorticons.length; i++)
						{
							if (i != this.column)
								set_unsorted(sorticons[i]);
						}
						sort_table(this);
					}
				};
			})();

	for (var i = 0; i < sorticons.length; i++)
		sorticons[i] = new TableSortHelper(i);
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

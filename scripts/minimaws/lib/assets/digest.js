// license:BSD-3-Clause
// copyright-holders:Vas Crabb

function Crc32Digester()
{
	this.crc = 0xffffffff;
}

Crc32Digester.prototype = (function ()
		{
			var table = new Uint32Array(256);
			(function ()
			{
				for (var i = 0; i < 256; i++)
				{
					var crc = i;
					for (var b = 0; b < 8; b++)
						crc = (crc >>> 1) ^ ((crc & 1) ? 0xedb88320 : 0x00000000);
					table[i] = crc;
				}
			})();

			return {
				digest: function (data, view)
				{
					for (var i = 0; i < view.length; i++)
						this.crc = (this.crc >>> 8) ^ table[(this.crc & 0x000000ff) ^ view[i]];
				},

				finalise: function ()
				{
					return this.crc ^ 0xffffffff;
				}
			};
		})();


function Sha1Digester()
{
	this.st = new Uint32Array(5);
	this.st[0] = 0xc3d2e1f0;
	this.st[1] = 0x10325476;
	this.st[2] = 0x98badcfe;
	this.st[3] = 0xefcdab89;
	this.st[4] = 0x67452301;

	this.cnt = new Uint32Array(2);
	this.cnt[0] = 0;
	this.cnt[1] = 0;

	this.buf = new DataView(new ArrayBuffer(64));
}

Sha1Digester.prototype = (function ()
		{
			function rol(x, n)
			{
				return ((x << n) | (x >>> (32 - n))) & 0xffffffff;
			}

			function b(data, i)
			{
				var r = data.getUint32(((i + 13) & 15) << 2, false);
				r ^= data.getUint32(((i + 8) & 15) << 2, false);
				r ^= data.getUint32(((i + 2) & 15) << 2, false);
				r ^= data.getUint32((i & 15) << 2, false);
				r = rol(r, 1);
				data.setUint32((i & 15) << 2, r, false);
				return r;
			}

			function r0(data, d, i)
			{
				d[i % 5] = 0xffffffff & (d[i % 5] + ((d[(i + 3) % 5] & (d[(i + 2) % 5] ^ d[(i + 1) % 5])) ^ d[(i + 1) % 5]) + data.getUint32(i << 2, false) + 0x5a827999 + rol(d[(i + 4) % 5], 5));
				d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
			}

			function r1(data, d, i)
			{
				d[i % 5] = 0xffffffff & (d[i % 5] + ((d[(i + 3) % 5] & (d[(i + 2) % 5] ^ d[(i + 1) % 5])) ^ d[(i + 1) % 5])+ b(data, i) + 0x5a827999 + rol(d[(i + 4) % 5], 5));
				d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
			}

			function r2(data, d, i)
			{
				d[i % 5] = 0xffffffff & (d[i % 5] + (d[(i + 3) % 5] ^ d[(i + 2) % 5] ^ d[(i + 1) % 5]) + b(data, i) + 0x6ed9eba1 + rol(d[(i + 4) % 5], 5));
				d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
			}

			function r3(data, d, i)
			{
				d[i % 5] = 0xffffffff & (d[i % 5] + (((d[(i + 3) % 5] | d[(i + 2) % 5]) & d[(i + 1) % 5]) | (d[(i + 3) % 5] & d[(i + 2) % 5])) + b(data, i) + 0x8f1bbcdc + rol(d[(i + 4) % 5], 5));
				d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
			}

			function r4(data, d, i)
			{
				d[i % 5] = 0xffffffff & (d[i % 5] + (d[(i + 3) % 5] ^ d[(i + 2) % 5] ^ d[(i + 1) % 5]) + b(data, i) + 0xca62c1d6 + rol(d[(i + 4) % 5], 5));
				d[(i + 3) % 5] = rol(d[(i + 3) % 5], 30);
			}

			function process(st, data)
			{
				var d = new Uint32Array(st);
				var i = 0;
				while (i < 16)
					r0(data, d, i++);
				while (i < 20)
					r1(data, d, i++);
				while (i < 40)
					r2(data, d, i++);
				while (i < 60)
					r3(data, d, i++);
				while (i < 80)
					r4(data, d, i++);
				for (i = 0; i < st.length; i++)
					st[i] = (st[i] + d[i]) & 0xffffffff;
			}

			return {
				digest: function (data, view)
				{
					var residual = this.cnt[0];
					this.cnt[0] = (this.cnt[0] + (view.length << 3)) & 0xffffffff;
					if (residual > this.cnt[0])
						this.cnt[1]++;
					this.cnt[1] += (view.length >>> 29);
					residual = (residual >>> 3) & 63;
					var offset = 0;
					if ((residual + view.length) >= 64)
					{
						if (residual > 0)
						{
							for (offset = 0; (offset + residual) < 64; offset++)
								this.buf.setUint8(offset + residual, view[offset]);
							process(this.st, this.buf);
							residual = 0;
						}
						for ( ; (view.length - offset) >= 64; offset += 64)
							process(this.st, new DataView(data, offset, 64));
					}
					for ( ; offset < view.length; residual++, offset++)
						this.buf.setUint8(residual, view[offset]);
				},

				finalise : function ()
				{
					var lenbuf = new ArrayBuffer(8);
					var lenview = new DataView(lenbuf);
					lenview.setUint32(0, this.cnt[1], false);
					lenview.setUint32(4, this.cnt[0], false);
					var padbuf = new ArrayBuffer(64 - (63 & ((this.cnt[0] >>> 3) + 8)));
					var padview = new Uint8Array(padbuf);
					padview[0] = 0x80;
					for (var i = 1; i < padview.length; i++)
						padview[i] = 0x00;
					this.digest(padbuf, padview);
					this.digest(lenbuf, new Uint8Array(lenbuf));
					var result = new Array(20);
					for (var i = 0; i < 20; i++)
						result[i] = (this.st[4 - (i >>> 2)] >> ((3 - (i & 3)) << 3)) & 0x000000ff;
					return result.map(x => x.toString(16).padStart(2, '0')).join('');
				}
			};
		})();


function StuckBitsDigester(max = 4)
{
	this.offset = 0;
	this.state = [ ];
	for (var i = 0; i <= max; i++)
	{
		var bytes = 1 << i;
		this.state.push({ filled: 0, bits: new Uint8Array(bytes), mask: new Uint8Array(bytes) });
	}
}

StuckBitsDigester.prototype = (function ()
		{
			return {
				digest: function (data, view)
				{
					for (var i = 0; i < view.length; i++)
					{
						var data = view[i];
						for (var j = 0; j < this.state.length; j++)
						{
							var detail = this.state[j];
							var bytes = 1 << j;
							var o = this.offset % bytes;
							var f = o == (bytes - 1);
							if (!detail.filled)
							{
								detail.bits[o] = data;
								detail.mask[o] = 0xff;
								if (f)
									detail.filled = 1;
							}
							else
							{
								detail.mask[o] &= ~(data ^ detail.bits[o]);
								detail.bits[o] &= detail.mask[o];
							}
							if (o == (bytes - 1))
								detail.filled += 1;
						}
						this.offset = (this.offset + 1) % (1 << (this.state.length - 1));
					}
				},

				finalise: function ()
				{
					var result = [ ];
					var lastresult = null;
					for (var i = 0; (i < this.state.length) && (this.state[i].filled >= 4); i++)
					{
						var detail = this.state[i];
						var bytes = 1 << i;
						for (var j = 0; j < bytes; j++)
						{
							var mask = detail.mask[j];
							if (mask && ((lastresult === null) || (mask != lastresult.mask[j % (1 << (lastresult.mask.length - 1))])))
							{
								lastresult = { bits: detail.bits, mask: detail.mask };
								result.push(lastresult);
								break;
							}
						}
					}
					return result;
				}
			};
		})();

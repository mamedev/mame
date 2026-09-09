# license:BSD-3-Clause
"""Integration tests using synthetic cartridges and user-supplied console ROMs."""
from pathlib import Path
import argparse, subprocess, zipfile, json, re
from concurrent.futures import ThreadPoolExecutor
p=argparse.ArgumentParser(description='Exercise Gigacart RPK validation and bank reads in a built TI-99 MAME.')
p.add_argument('--mame',type=Path,required=True)
p.add_argument('--rompath',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args()
root=a.mame.resolve().parent
folder=a.output.resolve()
folder.mkdir(parents=True,exist_ok=True)
cases=[]
def case(name, pcb='gigacart-rom', attrs='bank_data_bits="2"', romsize=8192, sockets=None, good=False):
    if sockets is None: sockets=[('rom_socket','rom','rom')]
    resources=[]; links=[]
    for socket,res,kind in sockets:
        resources.append(f'<{kind} id="{res}" file="{res}.bin"'+(' length="8192"' if kind=='ram' else '')+'/>')
        links.append(f'<socket id="{socket}" uses="{res}"/>')
    xml=f'<romset><resources>{"".join(resources)}</resources><configuration><pcb type="{pcb}" {attrs}>{"".join(links)}</pcb></configuration></romset>'
    path=folder/(name+'.rpk')
    with zipfile.ZipFile(path,'w',compression=zipfile.ZIP_DEFLATED) as z:
        z.writestr('layout.xml',xml)
        for res in set(r for _,r,k in sockets if k=='rom'):
            data=bytearray(romsize if res=='rom' else 8192)
            if res=='rom':
                for bank in range(len(data)//8192):
                    offset=bank*8192+8190
                    data[offset:offset+2]=((bank^0x5aa5)&0xffff).to_bytes(2,'big')
            z.writestr(res+'.bin',data)
    width = re.search(r'bank_data_bits="([1-8])"',attrs)
    cases.append((name,path,good,int(width[1]) if width else None,romsize,attrs))
for bits in range(1,9): case(f'width-{bits}',attrs=f'bank_data_bits="{bits}"',good=True)
for value in ['', 'bank_data_bits="0"','bank_data_bits="9"','bank_data_bits="02"','bank_data_bits="2x"']:
    case(f'bad-width-{len(cases)}',attrs=value)
case('bad-initial',attrs='bank_data_bits="2" initial_bank="16384"')
case('empty-initial',attrs='bank_data_bits="2" initial_bank=""')
case('initial-last',attrs='bank_data_bits="8" initial_bank="last"',good=True)
case('initial-number',attrs='bank_data_bits="2" initial_bank="4096"',good=True)
case('short-rom',romsize=8191)
case('empty-rom',romsize=0)
case('not-power-two',romsize=24576)
case('oversize',attrs='bank_data_bits="1"',romsize=128*1024*1024)
case('cpld',pcb='gigacart-cpld',good=True)
case('grom',pcb='gigacart-grom',sockets=[('rom_socket','rom','rom'),('grom_socket','grom','rom')],good=True)
case('missing-grom',pcb='gigacart-grom')
case('unexpected-grom',pcb='gigacart-cpld',sockets=[('rom_socket','rom','rom'),('grom_socket','grom','rom')])
case('unexpected-ram',sockets=[('rom_socket','rom','rom'),('ram_socket','ram','ram')])
case('ram-as-rom',sockets=[('rom_socket','rom','ram')])
case('duplicate-socket',sockets=[('rom_socket','rom','rom'),('rom_socket','other','rom')])
case('unknown-socket',sockets=[('rom_socket','rom','rom'),('wrong_socket','other','rom')])
case('legacy-standard',pcb='standard',attrs='',good=True)
case('legacy-paged',pcb='paged',attrs='',romsize=16384,good=True)
case('legacy-inverted',pcb='paged379i',attrs='',romsize=131072,good=True)
case('legacy-noninverted',pcb='paged377',attrs='',romsize=131072,good=True)
def run(item):
    name,path,good,bits,romsize,attrs=item
    cfg=folder/name
    cfg.mkdir(exist_ok=True)
    cmd=[str(a.mame.resolve()),'ti99_4a','-rompath',str(a.rompath.resolve()),'-cart',str(path),
         '-video','none','-sound','none','-nothrottle','-skip_gameinfo','-seconds_to_run','1','-cfg_directory',str(cfg)]
    if good and bits:
        initial=re.search(r'initial_bank="([^"]*)"',attrs)
        initial=initial[1] if initial else 'first'
        initial=0 if initial=='first' else (1<<(12+bits))-1 if initial=='last' else int(initial)
        lua=cfg/'check.lua'
        lua.write_text(f'''local m=manager.machine
local c=m.devices[':gromport:single:cartridge']
local bank=emu.item(c.items['0/m_rom_page'])
assert(bank:read(0)=={initial}, 'initial bank')
local symbols=emu.symbol_table(m)
local mem={{read_u16=function(self,a) return symbols:memory_value(':maincpu','p',a,2,true) end,write_u16=function(self,a,v) symbols:set_memory_value(':maincpu','p',a,2,v,true) end}}
mem:write_u16(0x7ffe,0xff00)
assert(bank:read(0)=={(1<<(12+bits))-1}, 'configured physical bank width')
assert(mem:read_u16(0x7ffe)=={((romsize//8192-1)^0x5aa5)&0xffff}, 'mirrored ROM read')
print('BANK ASSERTIONS PASSED')
m:exit()
''')
        cmd.extend(['-autoboot_delay','0','-autoboot_script',str(lua)])
    r=subprocess.run(cmd,cwd=root,capture_output=True,text=True,timeout=60)
    (folder/(name+'.log')).write_text(r.stdout+r.stderr)
    result={'case':name,'expected_load':good,'exit':r.returncode,'pass':r.returncode==(0 if good else 4) and (not(good and bits) or 'BANK ASSERTIONS PASSED' in r.stdout)}
    print(json.dumps(result),flush=True)
    return result
with ThreadPoolExecutor(max_workers=2) as pool: results=list(pool.map(run,cases))
(folder/'results.json').write_text(json.dumps(results,indent=2))
assert all(r['pass'] for r in results)
print(f'PASS: {len(results)} RPK load/rejection cases')

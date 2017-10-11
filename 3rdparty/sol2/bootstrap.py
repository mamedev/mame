#!/usr/bin/env python

import ninja_syntax
import os, sys, glob, re
import itertools
import argparse

# utilities
def flags(*args):
    return ' '.join(itertools.chain(*args))

def includes(l):
    return ['-I"{}"'.format(x) for x in l]

def library_includes(l):
    return ['-L"{}"'.format(x) for x in l]

def libraries(l):
    return ['-l{}'.format(x) for x in l]

def dependencies(l):
    return ['-isystem"{}"'.format(x) for x in l]

def object_file(f):
    (root, ext) = os.path.splitext(f)
    return os.path.join(objdir, root + '.o')

def replace_extension(f, e):
    (root, ext) = os.path.splitext(f)
    return root + e

# Default install dir
install_dir = os.path.join('/usr', 'include') if 'linux' in sys.platform else 'include'

# Compiler: Read from environment or defaulted
cxx = os.environ.get('CXX', "g++") 

# command line stuff
parser = argparse.ArgumentParser()
parser.add_argument('--debug', action='store_true', help='compile with debug flags')
parser.add_argument('--cxx', metavar='<compiler>', help='compiler name to use (default: env.CXX=%s)' % cxx, default=cxx)
parser.add_argument('--cxx-flags', help='additional flags passed to the compiler', default='')
parser.add_argument('--ci', action='store_true', help=argparse.SUPPRESS)
parser.add_argument('--testing', action='store_true', help=argparse.SUPPRESS)
parser.add_argument('--lua-version', help='Lua version, e.g. lua53', default='lua53')
parser.add_argument('--lua-lib', help='lua library name (without the lib on *nix).', default='lua')
parser.add_argument('--lua-dir', metavar='<dir>', help='directory lua is in with include and lib subdirectories')
parser.add_argument('--install-dir', metavar='<dir>', help='directory to install the headers to', default=install_dir);
parser.epilog = """In order to install sol, administrative privileges might be required.
Note that installation is done through the 'ninja install' command. To uninstall, the
command used is 'ninja uninstall'. The default installation directory for this
system is {}""".format(install_dir)

args = parser.parse_args()

# general variables
include = [ '.', './include' ]
depends = [os.path.join('Catch', 'include')]
cxxflags = [ '-Wall', '-Wextra', '-Wpedantic', '-pedantic', '-pedantic-errors', '-std=c++14', '-ftemplate-depth=1024' ]
cxxflags.extend([p for p in re.split("( |\\\".*?\\\"|'.*?')", args.cxx_flags) if p.strip()])
example_cxxflags = [ '-Wall', '-Wextra', '-Wpedantic', '-pedantic', '-pedantic-errors', '-std=c++14', '-ftemplate-depth=1024' ]
example_cxxflags.extend([p for p in re.split("( |\\\".*?\\\"|'.*?')", args.cxx_flags) if p.strip()])
ldflags = []
script_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
sol_dir = os.path.join(script_dir, 'sol')
sol_file = os.path.join(script_dir, 'sol.hpp')

copy_command = 'cp -rf {} $in && cp -f {} $in'.format(sol_dir, sol_file)
remove_command = 'rm -rf {} && rm -f {}'.format(os.path.join(args.install_dir, 'sol'), os.path.join(args.install_dir, 'sol.hpp'))
if sys.platform == 'win32':
    copy_command = 'robocopy /COPYALL /E {} $in && robocopy /COPYALL {} $in'.format(sol_dir, sol_file)
    remove_command = 'rmdir /S /Q {} && erase /F /S /Q /A {}'.format(os.path.join(args.install_dir, 'sol'),
                                                                     os.path.join(args.install_dir, 'sol.hpp'))

if not args.lua_lib:
     args.lua_lib = 'lua'

if args.debug:
    cxxflags.extend(['-g', '-O0'])
else:
    cxxflags.extend(['-DNDEBUG', '-O3'])
example_cxxflags.extend(['-g', '-O0'])

if args.lua_dir:
    include.extend([os.path.join(args.lua_dir, 'include')])
    ldflags.extend(library_includes([os.path.join(args.lua_dir, 'lib')]))

if 'linux' in sys.platform:
    lua_version = os.environ.get('LUA_VERSION', args.lua_version)
    if re.match(r'lua5[1-3]', lua_version):
        # Using normal lua
        lua_lib = lua_version[:-1] + '.' + lua_version[-1]
        lua_incl = lua_lib
    elif re.match(r'luajit5[1-3]', lua_version):
        # luajit
        lua_incl = 'luajit-2.0' # I don't get this..
        lua_lib = lua_version[:-2] + '-' + lua_version[-2] + '.' + lua_version[-1]
        include.extend(['/usr/include/luajit-2.0/', '/usr/local/include/luajit-2.0/'])
    else:
        raise Exception('Unknown lua_version={}' % lua_version)


    include.extend(['/usr/include/' + lua_incl, '/usr/local/include/' + lua_incl])
    ldflags.extend(library_includes(['/usr/local/lib']))
    ldflags.extend(libraries([lua_lib]))
elif 'darwin' in sys.platform:
    # OSX 
    lua_version = os.environ.get('LUA_VERSION', args.lua_version)
    if re.match(r'lua5[1-3]', lua_version):
        # Using normal lua
        lua_incl = lua_version[:-1] + '.' + lua_version[-1]
        lua_lib = lua_version[:-2] + '.' +  lua_version[-2] + '.' + lua_version[-1]
    elif re.match(r'luajit', lua_version):
        # luajit
        lua_incl = 'luajit-2.0'
        lua_lib = 'luajit'
        ldflags.extend(['-pagezero_size 10000', '-image_base 100000000'])
    elif re.match(r'luajit5[1-3]', lua_version):
        # luajit
        lua_incl = 'luajit-2.0'
        lua_lib = lua_version[:-2] + '-' + lua_version[-2] + '.' + lua_version[-1]
        ldflags.extend(['-pagezero_size 10000', '-image_base 100000000'])
    else:
        raise Exception('Unknown lua_version={}' % lua_version)

    depends.extend(['/usr/include/' + lua_incl, '/usr/local/include/' + lua_incl])
    ldflags.extend(library_includes(['/usr/local/lib']))
    ldflags.extend(libraries([lua_lib]))
else:
    ldflags.extend(libraries([args.lua_lib]))

if args.testing:
    cxxflags.append('-Wmissing-declarations')

if 'linux' in sys.platform:
    ldflags.extend(libraries(['dl']))

builddir = 'bin'
objdir = 'obj'
if 'win32' in sys.platform:
     tests = os.path.join(builddir, 'tests.exe')
else:
     tests = os.path.join(builddir, 'tests')

tests_inputs = []
tests_object_files = []
for f in glob.glob('test*.cpp'):
    obj = object_file(f)
    tests_inputs.append(f)
    tests_object_files.append(obj)

examples = []
examples_input = []
for f in glob.glob('examples/*.cpp'):
    if 'win32' in sys.platform:
        example = os.path.join(builddir, replace_extension(f, '.exe'))
    else:
        example = os.path.join(builddir, replace_extension(f, ''))
    examples_input.append(f)
    examples.append(example)


# ninja file
ninja = ninja_syntax.Writer(open('build.ninja', 'w'))

# variables
ninja.variable('ninja_required_version', '1.3')
ninja.variable('builddir', 'bin')
ninja.variable('cxx', args.cxx)
ninja.variable('cxxflags', flags(cxxflags + includes(include) + dependencies(depends)))
ninja.variable('example_cxxflags', flags(example_cxxflags + includes(include) + dependencies(depends)))
ninja.variable('ldflags', flags(ldflags))
ninja.newline()

# rules
ninja.rule('bootstrap', command = ' '.join(['python'] + sys.argv), generator = True)
ninja.rule('compile', command = '$cxx -MMD -MF $out.d -c $cxxflags -Werror $in -o $out',
                      deps = 'gcc', depfile = '$out.d',
                      description = 'compiling $in to $out')
ninja.rule('link', command = '$cxx $cxxflags $in -o $out $ldflags', description = 'creating $out')
ninja.rule('tests_runner', command = tests)
ninja.rule('examples_runner', command = 'cmd /c ' + (' && '.join(examples)) if 'win32' in sys.platform else ' && '.join(examples) )
ninja.rule('example', command = '$cxx $example_cxxflags -MMD -MF $out.d $in -o $out $ldflags',
                      deps = 'gcc', depfile = '$out.d',
                      description = 'compiling example $in to $out')
ninja.rule('installer', command = copy_command)
ninja.rule('uninstaller', command = remove_command)
ninja.newline()

# builds
ninja.build('build.ninja', 'bootstrap', implicit = sys.argv[0])

for obj, f in zip(tests_object_files, tests_inputs):
    ninja.build(obj, 'compile', inputs = f)

for example, f in zip(examples, examples_input):
    ninja.build(example, 'example', inputs = f)

ninja.build(tests, 'link', inputs = tests_object_files)
ninja.build('tests', 'phony', inputs = tests)
ninja.build('examples', 'phony', inputs = examples)
ninja.build('install', 'installer', inputs = args.install_dir)
ninja.build('uninstall', 'uninstaller')
ninja.build('run', 'tests_runner', implicit = 'tests')
ninja.build('run_examples', 'examples_runner', implicit = 'examples')
ninja.default('run run_examples')

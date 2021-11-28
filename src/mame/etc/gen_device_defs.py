#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Angelo Salese
"""Simple Python script to generate a new definition from the template_* files
It will create a new device .cpp/.h definition, using template_device.* as a base.
Most arguments are sanitized and validated to match a Jinja2-like {{ foo }} pattern.

This should be run from the root of your mame tree, i.e.
python ./src/mame/etc/gen_device_defs.py <params>
"""
import argparse
import os

# Derived license options over a source grep done on November 2021 source code
# TODO: consider using SPDX patterns instead, derive externally
# https://spdx.org/licenses/
LICENSE_OPTIONS = [
    "BSD-3-Clause",
    "BSD-2-Clause",
    "GPL-2.0+",
    "GPL2+",
    "LGPL-2.1+",
    "MIT",
]

CPP_EXTENSIONS = [".cpp", ".h"]

def generate_device_header(value: str) -> str:
    """Generate the C++ macro variable key, deriving from the given path

    MAME design prefixes MAME_ and suffixes an _H for each include guard.
    This is left by design in the file itself, it also pleases IDEs better
    (e.g. vscode would otherwise believe that everything in the include guard shouldn't be linted)

    Args:
        value (str): FQDN file that needs to be translated

    Raises:
        AssertionError:
            if attempting to generate a device header out of the scope of this.

    Returns:
        str:
            device header sanitized.
            Treats bus type file as a special option, so that it will parse as:
            bus/<bus_family>/<bus_devicefile> -> BUS_<BUS_FAMILY>_<BUS_DEVICE>
            ditto for CPU:
            cpu/<cpu_family>/<cpu_devicefile> -> CPU_<CPU_FAMILY>_<CPU_DEVICE>
            It otherwise treat everything else as:
            <device_type>/<devicefile> -> <DEVICE_TYPE>_<DEVICEFILE>
    """
    _is_bus_file = "bus" + os.path.sep in value or "cpu" + os.path.sep in value
    _fh = value.split(os.path.sep)
    main_category_idx = -3 if _is_bus_file else -2
    file_category = _fh[main_category_idx]
    assert file_category in ["bus", "cpu", "machine", "video", "sound", "imagedev"], (
        f"{generate_device_header.__name__} -> {file_category} invalid for device_handler"
    )
    return "_".join(_fh[main_category_idx:]).upper()

def get_args():
    parser = argparse.ArgumentParser(
        description=__doc__
    )
    parser.add_argument(
        "-device_file",
        required=True,
        type=str,
        help=" ".join([
            "The device .cpp/.h base file path, also derives the header macro directive by",
            "deriving from the path \"src/devices/machine/acorn_vidc20.cpp\"",
            "-> \"MAME_MACHINE_ACORN_VIDC20_H\""
        ])
    )
    # TODO: number of parameters can go out of hand, consider using a data source instead
    parser.add_argument(
        "-license_opt",
        default=LICENSE_OPTIONS[0],
        type=str,
        help=f"License option, valid field options are: {repr(LICENSE_OPTIONS)}"
    )
    parser.add_argument(
        "-device_longname",
        required=True,
        type=str,
        help="Friendly name for a device \"Acorn VIDC20\""
    )
    parser.add_argument(
        "-device_classname",
        required=True,
        type=str,
        help=" ".join([
            "Class declaration i.e. \"acorn_vidc20\",",
            "this usually matches device filename except for structures that needs",
            "to be further break down into sub-classes.",
        ])
    )
    parser.add_argument(
        "-device_typename",
        required=True,
        type=str,
        help="Type handle declaration i.e. \"ACORN_VIDC20\""
    )
    parser.add_argument(
        "-author_name",
        required=True,
        type=str,
        help="Your author handle name in copyright header"
    )
    # TODO: add optional structures like device_memory_interface
    # TODO: dry-run option
    # TODO: interactive mode
    # TODO: bus width

    user_input = vars(parser.parse_args())
    return {
        **user_input,
        "device_header": generate_device_header(user_input["device_file"])
    }

def sanitize(key: str, value: str) -> str:
    rules_fn = {
        "device_typename": lambda v: v.upper().strip(),
        "device_classname": lambda v: v.lower().strip(),
    }
    return rules_fn.get(key, lambda v: v.strip())(value)

def validate(key: str, value: str) -> bool:
    assert_fn = {
        "license_opt": lambda v: v in LICENSE_OPTIONS,
    }
    return assert_fn.get(key, lambda _: True)(value)

if __name__ == '__main__':
    # default place where template_device files lies, given as convenience for future extensions
    DEFAULT_TEMPLATE_PATH = os.path.sep.join(["src", "mame", "etc"])
    SRC_TEMPLATE_PATH = f"{os.getcwd()}{os.path.sep}{DEFAULT_TEMPLATE_PATH}"

    args = get_args()
    DEVICE_FILEPATH = args["device_file"]
    DST_BASE_PATH = f"{os.getcwd()}{os.path.sep}{DEVICE_FILEPATH}"
    assert not any(item in DST_BASE_PATH for item in CPP_EXTENSIONS), (
        f"device_file -> {DST_BASE_PATH} must not have C++ extension"
    )
    del args["device_file"]

    for file_type in CPP_EXTENSIONS:
        SRC_TEMPLATE_FILE = os.path.sep.join([SRC_TEMPLATE_PATH, f"template_device{file_type}"])
        with open(SRC_TEMPLATE_FILE, "r", encoding="utf-8") as template_fh:
            buf_string = template_fh.read()
        for k, v in args.items():
            # TODO: consider fully translating this to a Jinja2 or equivalent library
            buf_string = buf_string.replace("{{ " + k + " }}", sanitize(k, v))
            assert validate(k, v), f"{v} does not honor {k} ruleset"

        DST_DEVICE_FILE = f"{DEVICE_FILEPATH}{file_type}"

        with open(DST_DEVICE_FILE, "w", encoding="utf-8", newline=os.linesep) as result_fh:
            result_fh.write(buf_string)

    # TODO: auto-inject in LUA scripts

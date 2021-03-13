# license: BSD-3-Clause
"""Simple Python script to generate a new definition from the template_* files
"""
import argparse
import os

def get_args():
    parser = argparse.ArgumentParser(description="""
        Create a new device .cpp/.h definition, using template_device.* as a base.
        All arguments are sanitized to match the given patterns
        """
    )
    parser.add_argument(
        "-device_file",
        required=True,
        type=str,
        help="The device .cpp/.h base file name, also the header macro directive \"acorn_vidc20.cpp\" -> \"MAME_MACHINE_ACORN_VIDC20_H\""
    )
    # TODO: directory option, honor with os.getcwd() as default
    parser.add_argument(
        "-license_opt",
        default="BSD-3-Clause",
        type=str,
        help="License option"
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
        help="Class declaration \"acorn_vidc20\", this will also be the filename output"
    )
    parser.add_argument(
        "-device_typename",
        required=True,
        type=str,
        help='Type handle declaration \"ACORN_VIDC20\"'
    )
    parser.add_argument(
        "-author_name",
        required=True,
        type=str,
        help="Your author handle name in copyright header"
    )
    # TODO: add optional structures like device_memory_interface

    user_input = vars(parser.parse_args())
    return {
        **user_input,
        "device_header": user_input["device_file"].upper()
    }

def sanitize(k, v):
    rules_fn = {
        "device_typename": lambda v: v.upper().strip(),
        "device_classname": lambda v: v.lower().strip()
        # TODO: validate license_opt with an enum
    }
    return rules_fn.get(k, lambda v: v.strip())(v)

if __name__ == '__main__':
    args = get_args()
    for file_type in [".cpp", ".h"]:
        with open(".{0}template_device{1}".format(os.path.sep, file_type), "r", encoding="utf-8", newline="\n") as template_fh:
            buf_string = template_fh.read()
        for k, v in args.items():
            if k == "device_file":
                continue
            buf_string = buf_string.replace("<" + k + ">", sanitize(k, v))
        with open(".{0}{1}{2}".format(os.path.sep, args["device_file"], file_type), "w", encoding="utf-8", newline="\n") as result_fh:
            result_fh.write(buf_string)

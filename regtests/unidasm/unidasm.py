#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Angelo Salese
##
import os
import sys
import logging
import subprocess
from typing import Callable

def __assert_valid_filename(filehandle: str, target_arch: str, expected_extension: str):
    """Checkout if derived file handle input is the expected one

    Args:
        filehandle (str): file handle
        target_arch (str): the expected architecture base name
        expected_extension (str): the expected extension that the file handle should have
    """

    assert filehandle.startswith(target_arch), f"{filehandle} {expected_extension} mismatch with arch {target_arch}"
    assert filehandle.endswith(expected_extension), f"{filehandle} mismatch {expected_extension}"

def run_process():
    """Process our static directory, derive and execute tests 

    For unidasm the strategy is to point at the static folder and
    derive unit tests based off what's in static directory itself.
    To compose a new test for a given arch:
    
    1. craft a new binary file and save it in static subfolder, name it <test_arch>.bin,
       where <test_arch> is the name as it is displayed by running unidasm.exe
       with no additional params. Keep it simple & short, more sofisticated programs aren't really 
       the scope of this (we're just testing if the arch disassembles here).
    2. run unidasm $(thisfolder)/static/<test_arch>.bin -arch <test_arch> > $(thisfolder)/static/<test_arch>.asm
       On windows the resulting asm file newlines must be converted to LF with this.
    3. now run make tests and check if the new test gets captured. 

    """
    unidasm_exe = os.path.join(os.getcwd(), "unidasm{0}".format(".exe" if os.name == 'nt' else ""))
    static_tests_folder = os.path.join(os.path.dirname(os.path.realpath(__file__)), "static")
    static_tests = os.listdir(static_tests_folder)
    
    test_pool = {
        asm_filename.split(".")[0]: [asm_filename, bin_filename] for asm_filename, bin_filename in zip(static_tests[0::2], static_tests[1::2])
    }

    logging.info("Detected %s tests", len(test_pool))

    for target_arch, params in test_pool.items():
        assert len(params) == 2, f"{target_arch} Unexpected input test structure {params}"
        asm_filename, bin_filename = params
        __assert_valid_filename(asm_filename, target_arch, ".asm")
        __assert_valid_filename(bin_filename, target_arch, ".bin")

        logging.debug("Test arch %s passed internal validation, launching unidasm", target_arch)

        with open(os.path.join(static_tests_folder, asm_filename), "r", encoding="utf-8", newline="\n") as txt_h:
            expected_asm = txt_h.read()

        # TODO: checkout how unidasm copes with endianness on 16/32-bit based CPUs
        launch_process = subprocess.run(
            [unidasm_exe] + [os.path.join(static_tests_folder, bin_filename), "-arch", target_arch], 
            encoding="utf-8", 
            capture_output=True,
            check=True,
            text=True,
            shell=False
        )
        assert launch_process.returncode == 0, f"{target_arch} return code == {launch_process.returncode}"
        # TODO: eventually use difflib
        assert launch_process.stdout == expected_asm, "{1} expected:{0}{2}{0}actual:{0}{3}".format(
            os.linesep,
            target_arch,
            repr(expected_asm),
            repr(launch_process.stdout)
        )
        logging.info("Target arch test %s passed", target_arch)

if __name__ == "__main__":
    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)
    try:
        run_process()
    except (subprocess.CalledProcessError, AssertionError) as ex:
        logging.error(str(ex))
        sys.exit(1)
        raise
    logging.info("unidasm test successful")
    sys.exit(0)

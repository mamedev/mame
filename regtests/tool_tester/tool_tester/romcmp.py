##
## license:BSD-3-Clause
## copyright-holders:Angelo Salese
##

import os
from subprocess import CompletedProcess
from dataclasses import dataclass
import logging
from tool_tester._selfexe import SelfExeTests
#import difflib

class RomCmpTests(SelfExeTests):
    """Test out romcmp usage.

    Simple tests, just verify output from a dummy binary format.

    Args:
        SelfExeTests ([type]): [description]
    """
    identifier = "romcmp"

    def __init__(self, work_path: str, assets_path: str):
        super().__init__(work_path, assets_path)
        self._logs_folder = os.path.join(assets_path, self.identifier)
        self._bin_test_folder = os.path.join(assets_path, self.identifier, "bin")
        logging.debug(self._bin_test_folder)

    def _collect_tests(self):
        return {
            # FIXME: on at least Windows this causes readback at root drive
            # is it subprocess or romcmp fault?
#           "normal": ["", self._bin_test_folder],
            "normal": [self._bin_test_folder],
            "slower": ["-d", self._bin_test_folder],
            "hash": ["-h", self._bin_test_folder]
        }
    
    def _subprocess_args(self, test_handle, test_params):
        return test_params

    def _assert_test_result(self, launch_result: CompletedProcess, test_handle, test_params):
        assert launch_result.returncode == 0
        # it prints a percentage to stderr
        #assert launch_result.stderr == "", launch_result.stderr
        with open(os.path.join(self._logs_folder,f"{test_handle}.txt"), "r", encoding="utf-8", newline="\n") as txt_h:
            __expected_stdout = txt_h.read()
        assert launch_result.stdout == __expected_stdout, launch_result.stdout

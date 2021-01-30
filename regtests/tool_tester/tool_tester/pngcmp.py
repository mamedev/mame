##
## license:BSD-3-Clause
## copyright-holders:Angelo Salese
##

import os
from subprocess import CompletedProcess
#from dataclasses import dataclass
#import logging
from tool_tester._selfexe import SelfExeTests
from typing import List

class PngCmpTests(SelfExeTests):
    """A very bread and butter test suite against PngCmp tool

    * same_* tests if file A is equal, by checking if A == A
    * divergent_* tests if A != B, and B != A

    We don't need to actually save the output to anywhere else,
    this tool will most likely be reused in other areas anyway. ;)

    Args:
        SelfExeTests ([type]): [description]
    """

    def __init__(self, assets_path: str):
        super().__init__("pngcmp", assets_path)
        self._png_test_folder = os.path.join(assets_path, "png")

    def _collect_tests(self):
        return {
            "same_black": ["black320x240", "black320x240"],
            "same_white": ["white320x240", "white320x240"],
            "divergent_black_on_white": ["black320x240", "white320x240"],
            "divergent_white_on_black": ["white320x240", "black320x240"]
        }

    def _subprocess_args(self, test_handle: str, test_params: List):
        return [
            os.path.join(self._png_test_folder, test_params[0] + ".png"), 
            os.path.join(self._png_test_folder, test_params[1] + ".png"),
            os.devnull
        ]

    def _assert_test_result(self, launch_result: CompletedProcess, test_handle: str, test_params: List):
        assert launch_result.returncode == int(test_handle.startswith("divergent")), f"{test_handle} return code == {launch_result.returncode}"
        assert launch_result.stderr == "", f"{test_handle} non-empty stderr {launch_result.stderr}"
        assert launch_result.stdout == "", f"{test_handle} non-empty stdout {launch_result.stdout}"

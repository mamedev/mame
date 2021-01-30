##
## license:BSD-3-Clause
## copyright-holders:Angelo Salese
##
import os
from subprocess import CompletedProcess
from dataclasses import dataclass
import logging
from tool_tester._selfexe import SelfExeTests

@dataclass(frozen=True)
class UnidasmMapper:
    asm_filename: str
    bin_filename: str

    def __post_init__(self):
        for idx, params in enumerate(zip([self.asm_filename, self.bin_filename], [".asm", ".bin"])):
#           assert item.startswith(test_handle), f"{item} {expected_extension} mismatch with arch {target_arch}"
            assert params[0].endswith(params[1]), f"{params[0]} mismatch extension {params[1]}"


class UnidasmTests(SelfExeTests):
    """Unidasm test collector

    For unidasm the strategy is to point at the static folder and
    derive unit tests based off what's in static directory itself.
    To compose a new test for a given arch:
    
    1. craft a new binary file and save it in static subfolder, name it <test_arch>.bin,
       where <test_arch> is the name as it is displayed by running unidasm.exe
       with no additional params. Keep it simple & short, more sofisticated programs aren't really 
       the scope of this (we're just testing if the arch disassembles here).
    2. run unidasm $(regtests)/static/<test_arch>.bin -arch <test_arch> > $(regtests)/static/<test_arch>.asm
       Note: on windows the resulting asm file newlines must be converted to LF with this
       (otherwise you get a diff error later on).
    3. now run make tests or python $(modulepath)/test_tools.py and check if the new test gets captured. 

    """

    def __init__(self, assets_path: str):
        super().__init__("unidasm", assets_path)
        self._asm_test_folder = os.path.join(assets_path, "asm")
        self._bin_test_folder = os.path.join(assets_path, "bin")
        logging.debug(self._asm_test_folder)
        logging.debug(self._bin_test_folder)

    def _collect_tests(self):
        __static_tests = zip( os.listdir(self._asm_test_folder), os.listdir(self._bin_test_folder))
        return {
            __asm.split(".")[0]: UnidasmMapper(asm_filename=__asm, bin_filename=__bin) for __asm, __bin in __static_tests
        }

    def _subprocess_args(self, test_handle: str, test_params: UnidasmMapper):
        return [os.path.join(self._bin_test_folder, test_params.bin_filename), "-arch", test_handle]

    def _assert_test_result(self, launch_result: CompletedProcess, test_handle: str, test_params: UnidasmMapper):
        __ret_code = launch_result.returncode
        assert __ret_code == 0, f"{test_handle} return code == {__ret_code}"
        __actual_stderr = launch_result.stderr
        assert not __actual_stderr, f"unexpected stderr {__actual_stderr}"

        with open(os.path.join(self._asm_test_folder, test_params.asm_filename), "r", encoding="utf-8", newline="\n") as txt_h:
            __expected_asm = txt_h.read()
        __actual_stdout = launch_result.stdout
        # TODO: eventually use difflib for error printing
        # https://docs.python.org/3/library/difflib.html
        assert __actual_stdout == __expected_asm, "{1} test FAILED{0}expected:{0}{2}{0}actual:{0}{3}".format(
            os.linesep,
            test_handle,
            repr(__expected_asm),
            repr(__actual_stdout)
        )

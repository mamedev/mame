##
## license:BSD-3-Clause
## copyright-holders:Angelo Salese
##
from abc import ABC, abstractmethod
import os
import logging
import subprocess
from typing import Dict, List

class SelfExeTests(ABC):
    identifier: str

    def __init__(self, work_path: str, assets_path: str):
        __id_exe = self.identifier
        self._exec_path = os.path.join(work_path, "{0}{1}".format(__id_exe, ".exe" if os.name == 'nt' else ""))
        self._assets_path = assets_path
        logging.debug("Setup %s executable exe at %s", __id_exe, self._exec_path)

    def compose_tests(self) -> Dict:
        """Compose a list of tests to be later reused.

        Client can override this method and call super on itself
        if it needs finer logging granularity (such as printing with logging.debug)

        Returns:
            Dict: key for test_handle, value is an arbitrary list or dataclass driven object (preferably latter)
        """
        test_pool = self._collect_tests()
        logging.info("Detected %s tests", len(test_pool))
        return test_pool

    @abstractmethod
    def _collect_tests(self) -> Dict:
        """Mapping fn that the client must override

        Returns:
            Dict: key for test_handle, value is an arbitrary list or dataclass driven object (preferably latter)
        """

    def execute_tests(self, test_pool: Dict) -> bool:
        """Test executor pipeline

        Args:
            test_pool (Dict): a mapping defined by self._collect_tests()

        Returns:
            bool: True if all tests passes successfully, False otherwise
        """
        test_result = True
        for test_handle, test_params in test_pool.items():
            __test_name = f"{self.identifier}-{test_handle}"
            logging.debug("Preparing %s test ", __test_name)
            try:
                __launch_fullpath =  [self._exec_path] + self._subprocess_args(test_handle, test_params)
                logging.debug(__launch_fullpath)
                launch_result = subprocess.run(
                    __launch_fullpath, 
                    encoding="utf-8",
                    capture_output=True,
                    # TODO: we need to disable check here cause pngcmp has a returncode of 1 when diverging snapshots occurs (wtf)
                    #check=True,
                    check=False,
                    text=True,
                    shell=False
                )
                logging.debug("Subprocess test %s pass", __test_name)
                self._assert_test_result(launch_result, test_handle, test_params)
                logging.info("Test %s passed", __test_name)
            except (subprocess.CalledProcessError, AssertionError) as ex:
                logging.exception(str(ex)) 
                test_result = False
        return test_result

    @abstractmethod
    def _subprocess_args(self, test_handle: str, test_params: List) -> List:
        """Translator fn for dispatch parameter inputs against a subprocess run

        Args:
            test_handle (str): handle identifier
                Client can breakdown at will for branching with parameters
            test_params (List): parameter List or dataclass object mapper

        Returns:
            List: formatted list for subprocess
        """

    @abstractmethod
    def _assert_test_result(self, launch_result: subprocess.CompletedProcess, test_handle: str, test_params: List):
        """Unit test fn an actual test against the resulting output from subprocess

        A bare minimum is to assert against returncode, stdout and stderr.
        If you need to check against binary formats give extra carefulness that the test
        is always repeatable and non-OS dependant.

        Args:
            launch_result (subprocess.CompletedProcess): result of a given 
            test_handle (str): handle identifier
                Client can breakdown at will for branching with parameters
            test_params (List): parameter List or dataclass object mapper
        """

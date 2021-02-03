#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Angelo Salese
##
import sys
from os.path import join, dirname, realpath
import logging
from tool_tester.pngcmp import PngCmpTests
from tool_tester.romcmp import RomCmpTests
from tool_tester.unidasm import UnidasmTests

if __name__ == "__main__":
    # TODO: proper requirements.txt / setup.py or virtual env management
    # dataclasses aren't supported in anything prior to 3.7 (dacite lib 3.6)
    assert sys.version_info >= (3, 7), f"python version {sys.version_info.major}.{sys.version_info.minor} < 3.7"

    # TODO: add colorized messages
    # TODO: argparse the logging level
    logging.basicConfig(format='%(levelname)s: %(message)s', level=logging.INFO)

    # TODO: for now I'll just use class handlers here to chain test sources
    # In an ideal world you want to collect items thru inspect module instead
    # https://docs.python.org/3/library/inspect.html
    # and isolate by handler name, so that an optional arg can be passed here and launch
    # a given test module on user demand
    chained_results = []
    # TODO: point to $(regtests)\assets, configure if necessary
    assets_folder = join(dirname(dirname(realpath(__file__))), "assets")
    for test_cls in [PngCmpTests, RomCmpTests, UnidasmTests]:
        test_fn = test_cls(assets_folder)
        logging.info("Start test suite: %s", test_fn.identifier)
        chained_results.append(test_fn.execute_tests(test_fn.compose_tests()))
    logging.debug("test results %s", repr(chained_results))
    sys.exit(0 if all(chained_results) else 1)

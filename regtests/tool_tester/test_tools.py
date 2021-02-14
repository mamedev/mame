#!/usr/bin/python
##
## license:BSD-3-Clause
## copyright-holders:Angelo Salese
##
import sys
import os
from os.path import join, dirname, realpath
import argparse
import logging
from tool_tester import (
    ORCHESTRATOR_POOL
)

def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v",
        "--verbose",
        dest="verbose",
        action="store_true",
        help="Enable debug logging messages if enabled"
    )
    parser.add_argument(
        "-work_dir",
        dest="work_dir",
        type=str,
        default=os.getcwd(),
        help="Work directory where tools lies"
    )
    parser.add_argument(
        "-id",
        dest="test_id",
        type=str,
        default=None,
        help="If non-default run this test suite only, supported values: {0}".format(
            repr([item.identifier for item in ORCHESTRATOR_POOL])
        )
    )
    return parser.parse_args()

if __name__ == "__main__":
    args = get_args()

    # TODO: proper requirements.txt / setup.py or virtual env management
    # dataclasses aren't supported in anything prior to 3.7 (dacite lib 3.6)
    assert sys.version_info >= (3, 7), f"python version {sys.version_info.major}.{sys.version_info.minor} < 3.7"
    log_level = logging.DEBUG if args.verbose else logging.INFO

    # TODO: add colorized messages
    # consider either using colorlog or make one that has support for all terminal flavours
    logging.basicConfig(format='%(levelname)s: %(message)s', level=log_level)

    # TODO: currently points to $(regtests)/assets, make it a configurable option?
    assets_folder = join(dirname(dirname(realpath(__file__))), "assets")

    chained_results = []
    __single_test = args.test_id
    if __single_test is None:
        __EXECUTE_TESTS = ORCHESTRATOR_POOL 
    else:
        __EXECUTE_TESTS = [item for item in ORCHESTRATOR_POOL if item.identifier == args.test_id]
    assert __EXECUTE_TESTS, f"{args.test_id} not found in available tests"

    for test_cls in __EXECUTE_TESTS:
        test_fn = test_cls(args.work_dir, assets_folder)
        logging.info("Start test suite: %s", test_fn.identifier)
        chained_results.append(test_fn.execute_tests(test_fn.compose_tests()))
    logging.debug("test results %s", repr(chained_results))
    sys.exit(0 if all(chained_results) else 1)

from tool_tester.pngcmp import PngCmpTests
from tool_tester.romcmp import RomCmpTests
from tool_tester.unidasm import UnidasmTests

# TODO: for now I'll just use class handlers here to chain test sources
# In an ideal world you eventually want to collect these items thru inspect module instead,
# especially if this pool starts to get too big to mantain.
# https://docs.python.org/3/library/inspect.html
ORCHESTRATOR_POOL = [PngCmpTests, RomCmpTests, UnidasmTests]

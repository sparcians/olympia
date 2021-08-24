project(olympia_test)

# Include the handy testing marcros defined in Sparta
include(${SPARTA_BASE}/test/TestingMacros.cmake)

# This line will make sure olympia is built before running the tests
sparta_regress (olympia)

# Tests
sparta_named_test(max_ipc_1M    olympia -i 1M -p top.cpu.core0.fetch.params.fetch_max_ipc true)
sparta_named_test(random_ipc_1M olympia -i 1M -p top.cpu.core0.fetch.params.fetch_max_ipc false)

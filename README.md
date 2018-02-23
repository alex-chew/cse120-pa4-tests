# CSE 120 PA4 Test Suite

A test suite for PA4 (user-level threading library) and test runners.

Relies on a modified version of [acutest] (included in the repo).

PRs welcome. (See **Contributing** below.)

## Usage

Copy the files into your `pa4` directory, then make the test runners with `make
tests`. This builds both `mytest`, which uses your kernel implementation, and
`reftest`, which uses the reference kernel implementation (so you can compare
outputs and ensure that the tests themselves are valid). The examples below can
be run with either.

Running a single test:

```
$ ./reftest                 # print help message (extra options, list of tests)
$ ./reftest -l              # list all tests
$ ./reftest    square_cube  # run the `square_cube` test
$ ./reftest    squ          # same as above; partial names work
$ ./reftest -v squ          # same as above, but print all assertions
```

Running all tests:

```
$ ./runall.sh    reftest    # run all tests in the reftest runner
$ ./runall.sh -v reftest    # same as above, but print all assertions
```

## Contributing

Adding a test just requires adding two things to `tests.c`:

1.  Create a function `tN` which runs the test. Typically this starts with
    `MyInitThreads()`, and so forth. See [acutest] for more info on how to make
    assertions (or just follow the existing tests).

2.  Register the function as a test in `TEST_LIST` at the end of the file. You
    can just follow the existing format, making sure to leave the final `{0}`
    intact.

Please ensure the test is valid by running it with the reference kernel.



[acutest]: https://github.com/mity/acutest

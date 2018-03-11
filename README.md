# CSE 120 PA4 Test Suite

A test suite for PA4 (user-level threading library) and test runners.

Relies on a modified version of [acutest] (included in the repo).

PRs welcome. (See **Contributing** below.)

## Installation

Just run the included installer, and you're set!

```
$ ./install.sh
```

## Usage

Before running any tests, make sure to assimilate first. Make will fail if this
step is not completed. This is done once for you when installed, but if you 
add/change any tests you will need to assimilate again. This can be done with
the following:

```
$ ./assimilate.sh           # assimilate test files into tests.c
$ ./assimilate.sh clean     # clean work done by assimilate.sh
```

Make the test runners with `make
tests`. This builds both `mytest`, which uses *your* kernel implementation, and
`reftest`, which uses the *reference* kernel implementation (so you can compare
outputs and ensure that the tests themselves are valid). The examples below can
be run with either.

To see the expected behavior using the reference kernel, replace `mytest` with
`reftest` in all the commands below.

Running all tests:

```
$ ./runall.sh    mytest     # run all tests in the mytest runner
$ ./runall.sh -v mytest     # same as above, but print all assertions
```

Running a single test:

```
$ ./mytest                  # print help message (extra options, list of tests)
$ ./mytest -l               # list all tests
$ ./mytest    square_cube   # run the `square_cube` test
$ ./mytest    squ           # same as above; partial names work
$ ./mytest -v squ           # same as above, but print all assertions
```

## Contributing

To add a new test, just add a new `.c` source file to the `tests` directory.
There are 2 things to keep in mind when adding a new test: 

1.  There must be a global function with the same name as the file (minus the
    extension). Typically this starts with
    `MyInitThreads()`, and so forth. See [acutest] for more info on how to
    make assertions (or just follow the existing tests).

2.  All other functions/declarations should be static. This is not so much
    a requirement, since there is no guarantee things will go wrong, but it
    is still good to keep things local to the file static whenever possible.

Please ensure the test is valid by running it with the reference kernel.



[acutest]: https://github.com/mity/acutest

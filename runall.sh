#!/usr/bin/env bash

# Runs all tests in an acutest suite.
# USAGE:
# ./runall.sh    reftest  # Test using reference kernel.
# ./runall.sh    mytest   # Test using your kernel.
# ./runall.sh -v reftest  # Test using reference kernel; print each assertion.
# ./runall.sh -v mytest   # Test using your kernel; print each assertion.
usage () {
  echo "Usage: ./runall.sh [ -v ] [ reftest | mytest ]"
  exit 1
}

# Parse options
while getopts ":v" opt; do
  case $opt in
    v) args="$args -v" ;;
    *) usage ;;
  esac
done
shift $((OPTIND - 1))
suite="$1"

if [[ ! -x $suite ]]; then
  echo "*** FATAL: Cannot execute $suite ***"
  usage
fi

$suite --list 2>&1 | awk '/^  / { print $1 }' | while read name; do
  eval "$suite $args $name"
done

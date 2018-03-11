#!/bin/bash
# This function will assimilate the valid tests and make the following changes:
# - Write tests.c to initialize global test array
# - Change tests/Makefile to include the valid tests at compile time

BASE_DIR=~/pa4
TEST_DIR=$BASE_DIR/tests
MAIN_FILE=$BASE_DIR/tests.c

if [ "$1" == "clean" ]; then
  echo "Cleaning tests..."

  cd $TEST_DIR
  rm -f $MAIN_FILE
  sed "s/^SRC.*/SRC     =/" Makefile > _Makefile
  mv _Makefile Makefile

  echo "Cleaning complete."
  exit 0
fi

echo "Assimilating tests..."

cd $TEST_DIR
rm -f $MAIN_FILE
tests=`ls -1 *.c 2> /dev/null | sed 's/\.c$//'`
testArray=($tests)

for test in ${testArray[@]}; do
  findFun=`grep "void.*$test.*\\(.*\\)" "$test.c"`

  if [ -z "$findFun" ]; then
    echo "Warning: File $test.c does not have a function with the same name as the file. Skipping"
    tests=`echo "$tests" | grep -v "^$test$"`
  fi
done

if [ -z "$tests" ]; then
  echo "No valid tests found! Aborting"
  exit 1
fi

echo "$tests"                                       >  _testList
tests2=`paste -d '|' _testList _testList`
rm _testList

echo "#include \"acutest.h\""                       >  $MAIN_FILE
echo                                                >> $MAIN_FILE
echo "$tests" | sed 's/^/extern void /;s/$/();/'    >> $MAIN_FILE
echo                                                >> $MAIN_FILE
echo "TEST_LIST = {"                                >> $MAIN_FILE
echo "$tests2" | sed "s/^/	{\"/;s/|/\", /;s/$/},/" >> $MAIN_FILE
echo "\t{0}"                                        >> $MAIN_FILE
echo "};"                                           >> $MAIN_FILE

tests=`echo "$tests" | sed 's/$/.c/g'`
testsLine=`echo $tests`
sed "s/^SRC.*/SRC     = $testsLine/" Makefile > _Makefile
mv _Makefile Makefile

echo "Assimilation complete."

#!/usr/bin/env bash
echo "Start executing TestSuite..."
mkdir TEST_RESULTS
cd TEST_RESULTS
../cmake-build-debug/CIoTA_TestsSuite suite > test.txt
 if grep -q "ALL TESTS PASSED" test.txt; then
    echo "suite mode PASS the tests"
 else
    echo "suite mode FAILED the tests"
 fi
 ../cmake-build-debug/CIoTA_TestsSuite "sync" > test.txt
 if grep -q "ALL TESTS PASSED" test.txt; then
    echo "sync mode PASS the tests"
 else
    echo "sync mode FAILED the tests"
 fi
 ../cmake-build-debug/CIoTA_TestsSuite "async" > test.txt
 if grep -q "ALL TESTS PASSED" test.txt; then
    echo "async mode PASS the tests"
 else
    echo "async mode FAILED the tests"
 fi
 cd ..
 rm -r TEST_RESULTS

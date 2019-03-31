#!/bin/bash
TEMP_LOG="temp_log_for_test_driver_$$.txt"

for F in "$1"/*.gb; do
    ./cputest "$F" > $TEMP_LOG
    echo -n "Test ROM '$F': "
    >> /dev/null grep "Pass" $TEMP_LOG
    if [ $? -eq 0 ]; then
        echo "PASSED!"
    else
        echo "FAILED!"; echo
        cat $TEMP_LOG
    fi
done

rm -f "$TEMP_LOG"
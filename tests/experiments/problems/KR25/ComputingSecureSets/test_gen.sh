#!/bin/bash

# for i in {1..10}; do
#     for j in 166;do
#         ./generate.py $j 0.8 > g-$j-80-$i.new.asp   
#     done
# done

for i in {1..10}; do
    timeout 120  python "$ROOT/prop_unf/run.py" -e=encoding.asp -i=g-166-80-$i.new.asp -b=test >> out
    if [ $? -eq 124 ]; then
        echo "Execution for g-166-80-$i.new.asp timed out" >> out
    fi
done
METHOD=$1
echo "Checking method: $METHOD"
python $ROOT/tests/correctness/run.py -e=encoding.asp -i=correctness.asp -m=$METHOD all

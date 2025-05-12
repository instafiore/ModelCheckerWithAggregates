EXPERIMENTS_PYTHON=$1
EXPERIMENTS_OUTPUT=$2
rm -f logs/$EXPERIMENTS_OUTPUT/.* 2> /dev/null
python $PYRUNNER -r $EXPERIMENTS_PYTHON -o xml -l results/xml/$EXPERIMENTS_OUTPUT -d logs/$EXPERIMENTS_OUTPUT
# export CPU=95
# ./run_experiments.sh experiments.py 2025-04-05_clingo_unf_kclique
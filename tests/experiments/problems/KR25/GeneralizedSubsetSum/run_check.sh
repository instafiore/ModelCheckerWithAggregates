SAT=10
UNSAT=20
ERROR=1

for f in *.asp; do 
    echo "[bash] running $f"

    timeout 20m time -p python $ROOT/prop_unf/run.py -e=$f -c -b=release
    exit_code=$?  # Store the exit code immediately

    echo "exit code $exit_code"

    # Check for error exit code
    if [ "$exit_code" -eq "$ERROR" ]; then
        echo "[bash] Error with $f"
        exit 1
    fi

    # Check if timeout occurred (exit code 124)
    if [ "$exit_code" -eq 124 ]; then
        echo "[bash] Timeout occurred with $f"

    elif [ "$exit_code" -ne "$SAT" ] && [ "$exit_code" -ne "$UNSAT" ]; then
        # Ensure only valid exit codes pass
        echo "[bash] Unexpected exit code $exit_code for $f (expected $SAT or $UNSAT)"
        exit 1
    fi

done

echo "[bash] Tests passed!"
exit 0

import os
import sys

ROOT= os.getenv("ROOT")
PYRUNLIM= os.getenv("PYRUNLIM")
CPU= os.getenv("CPU",0)

print(f"Using CPU: {CPU}")

sys.path.append(ROOT)

from prop_unf.utils.settings import *
from prop_unf.utils.utility import *
sys.path.append(PROP_UNF)

from tests.correctness.validators import *

self.setPyrunlim([
    f"{PYRUNLIM}",
    "--time=%d" % ( 20 * 60),
    "--memory=%d" % (15 * 1024),
    f"--affinity={CPU}",
    "--output=xml"
])

benchmarks = [
   "ComputingSecureSets",
    "GeneralizedSubsetSum",
    "k-CliqueColoring",
]

problem="KR25"


# self.addCommand(Command("clingo_unf_v2", f"python {ROOT}/prop_unf/run.py -e=$1 -i=$2 -m=unfounded", validator=ExitCodeValidator()))
self.addCommand(Command("clingo_red_pc", f"python {ROOT}/prop_unf/run.py -e=$1 -i=$2 -m=reduct", validator=ExitCodeValidator()))
# self.addCommand(Command("clingo_plain", f"clingo $1 $2", validator=ExitCodeValidator([10,20,30])))

for benchmark in benchmarks:
    files = self.executeAndSplit("cat $DIRNAME/problems/%s/%s/instances.all" % (problem, benchmark))
    filesFull = []
    for file in files:
        filesFull.append(("$DIRNAME/problems/%s/%s/%s" % (problem, benchmark, file),))

    encoding = "$DIRNAME/problems/%s/%s/encoding.asp" % (problem, benchmark)

    self.addBenchmark(
        Benchmark(
            benchmark,
            sharedOptions=[encoding], 
            testcases=sorted(filesFull),
            validator=AnswerSetValidator(encoding)
        )
    )
import os
import subprocess

UTILS = os.path.dirname(os.path.abspath(__file__))
PROP_UNF = os.path.dirname(UTILS)
ROOT = os.path.dirname(PROP_UNF)
PROPAGATOR_C = f"{PROP_UNF}/propagator_c"
PROPAGATOR_BIN = f"{PROPAGATOR_C}/bin/propagator"
LOGS = f"{ROOT}/logs"
GENERIC_REGEX_ANSWERS_SET_ATOM = r"(?<=[\s,{])(([\w_]+[\(]?[\w_,]*[\)]?))"
GENERIC_REGEX_ANSWERS_SET_ATOM_WITH_NEGATIVE = r"(?<=[\s,{])((~?[\w_]+[\(]?[\w_,]*[\)]?))"
REGEX_ANSWERSET = r"^Answer set \d+ {(.+)}"
TIMELIMIT = 5 * 60 #seconds

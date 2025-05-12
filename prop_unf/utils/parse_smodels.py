import sys
import re

def parse_smodels():
    # Read entire input from stdin
    smodels_input = sys.stdin.read().strip()
    lines = smodels_input.split("\n")
    
    # Step 1: Read Header
    header = list(map(int, lines[0].split()))
    num_atoms = header[0]  # First number is total atoms

    rules = []
    atom_mapping = {}

    # Step 2: Extract Rules
    rule_lines = []
    for i, line in enumerate(lines[1:], start=1):
        if line.strip() == "0":
            break  # Stop at separator line
        rule_lines.append(list(map(int, line.split())))

    # Step 3: Extract Atom Names
    for line in lines[i+1:]:  # Start from line after rules
        if line.strip() == "0":
            break
        match = re.match(r"(\d+)\s+(\w+)", line.strip())
        if match:
            atom_mapping[int(match.group(1))] = match.group(2)

    # Step 4: Convert Rules
    asp_program = []
    for rule in rule_lines:
        rule_type, head, body_size, neg_count, *body = rule
        head_atom = atom_mapping.get(head, f"aux_{head}")

        # Separate positive and negative dependencies
        positive_body = [atom_mapping.get(a, f"aux_{a}") for a in body[neg_count:]]
        negative_body = [atom_mapping.get(a, f"aux_{a}") for a in body[:neg_count]]

        # Build ASP rule
        comma_after_positive_body = ", " if len(positive_body) > 0 else ""
        body_str = ", ".join(positive_body) + (f"{comma_after_positive_body}not " + ", not ".join(negative_body) if negative_body else "")
        rule_str = f"{head_atom} :- {body_str}." if body_str else f"{head_atom}."
        asp_program.append(rule_str)

    # Print the formatted ASP program
    print("\n".join(asp_program))

if __name__ == "__main__":
    parse_smodels()

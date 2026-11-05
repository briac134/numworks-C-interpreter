INPUT_FILE = "file_to_convert.c"
OUTPUT_FILE = "converted_file_to_paste.txt"

PREFIX = '  const std::string default_source = '


def escape_c(line: str) -> str:
    """Échappe une ligne pour la mettre dans un littéral de chaîne C++."""
    return (
        line.replace("\\", "\\\\")
            .replace('"', '\\"')
            .replace("\t", "\\t")
    )


def main():
    with open(INPUT_FILE, "r", encoding="utf-8") as f:
        lines = f.read().splitlines()

    indent = " " * len(PREFIX)
    literals = [f'"{escape_c(line)}\\n"' for line in lines]

    out = ["int main() {"]
    if literals:
        out.append(PREFIX + literals[0])
        for lit in literals[1:]:
            out.append(indent + lit)
        out[-1] += ";"
    else:
        out.append(PREFIX + '"";')

    with open(OUTPUT_FILE, "w", encoding="utf-8") as f:
        f.write("\n".join(out) + "\n")


if __name__ == "__main__":
    main()
import argparse
import re
import sys
from typing import TextIO


def search_stream(
    stream: TextIO,
    pattern: re.Pattern[str],
    show_line_numbers: bool,
    filename_prefix = None) -> bool:
    any_match = False
    for lineno, line in enumerate(stream, start=1):
        if not pattern.search(line):
            continue

        any_match = True
        if not line.endswith("\n"):
            line += "\n"

        prefix = ""
        if filename_prefix is not None:
            prefix += f"{filename_prefix}:"
        if show_line_numbers:
            prefix += f"{lineno}: "

        sys.stdout.write(prefix + line)

    return any_match


def main(argv = None) -> int:
    parser = argparse.ArgumentParser(
        prog="mini-grep",
        description="Print lines matching a regex pattern.",
    )
    parser.add_argument(
        "-q",
        action="store_true",
        dest="quiet",
        help="Suppress line numbers in output.",
    )
    parser.add_argument(
        "-e",
        required=True,
        dest="pattern",
        help="Regex pattern to search for.",
    )
    parser.add_argument(
        "files",
        nargs="*",
        help="Files to search. If none given, read from stdin.",
    )
    args = parser.parse_args(argv)

    try:
        pattern = re.compile(args.pattern)
    except re.error as exc:
        print(f"mini-grep: invalid pattern: {exc}", file=sys.stderr)
        return 2

    show_line_numbers = not args.quiet
    any_match = False

    if not args.files:
        any_match = search_stream(sys.stdin, pattern, show_line_numbers)
    else:
        show_filename = len(args.files) > 1
        for filename in args.files:
            try:
                with open(filename, "r", errors="replace") as f:
                    matched = search_stream(
                        f,
                        pattern,
                        show_line_numbers,
                        filename_prefix=filename if show_filename else None,
                    )
            except OSError as exc:
                print(f"mini-grep: {filename}: {exc.strerror}", file=sys.stderr)
                return 2
            any_match = any_match or matched

    return 0 if any_match else 1


if __name__ == "__main__":
    sys.exit(main())

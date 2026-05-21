import argparse
import os
import pwd
import stat
import sys
from datetime import datetime
from pathlib import Path
from typing import TextIO


def format_entry(path: Path, st: os.stat_result) -> str:
    mode_str = stat.filemode(st.st_mode)

    try:
        owner = pwd.getpwuid(st.st_uid).pw_name
    except KeyError:
        owner = str(st.st_uid)

    mtime = datetime.fromtimestamp(st.st_mtime).astimezone()
    mtime_str = mtime.strftime("%d-%m-%Y %H:%M:%S %Z")

    display = str(path)
    if stat.S_ISDIR(st.st_mode):
        display += "/"

    return f"{mode_str} {owner} {mtime_str} {display}"


def list_path(
    path: Path,
    recursive: bool,
    out: TextIO = sys.stdout,
    err: TextIO = sys.stderr,
) -> None:
    try:
        st = path.lstat()
    except OSError as exc:
        print(f"mini-ls: {path}: {exc.strerror}", file=err)
        return

    if not stat.S_ISDIR(st.st_mode):
        print(format_entry(path, st), file=out)
        return

    print(format_entry(path, st), file=out)

    try:
        children = sorted(path.iterdir(), key=lambda p: p.name)
    except OSError as exc:
        print(f"mini-ls: {path}: {exc.strerror}", file=err)
        return

    for child in children:
        try:
            child_st = child.lstat()
        except OSError as exc:
            print(f"mini-ls: {child}: {exc.strerror}", file=err)
            continue

        if recursive and stat.S_ISDIR(child_st.st_mode):
            list_path(child, recursive=True, out=out, err=err)
        else:
            print(format_entry(child, child_st), file=out)


def main(argv = None) -> int:
    parser = argparse.ArgumentParser(
        prog="mini-ls",
        description="List information about files and directories.",
    )
    parser.add_argument(
        "-r",
        action="store_true",
        dest="recursive",
        help="Recurse into directories.",
    )
    parser.add_argument(
        "files",
        nargs="*",
        help="Files or directories to list. Defaults to the current directory.",
    )
    args = parser.parse_args(argv)

    paths = [Path(p) for p in args.files] if args.files else [Path(".")]

    for path in paths:
        list_path(path, recursive=args.recursive)

    return 0


if __name__ == "__main__":
    sys.exit(main())

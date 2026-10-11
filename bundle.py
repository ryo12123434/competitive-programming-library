#!/usr/bin/env python3
from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LIBRARY_DIR = ROOT / "library"

LOCAL_INCLUDE_RE = re.compile(
    r'^(?P<indent>\s*)#\s*include\s*"(?P<path>[^"]+)"'
    r'(?P<suffix>\s*(?:/\*.*?\*/\s*)*(?://.*)?)$'
)
PRAGMA_ONCE_RE = re.compile(r"^\s*#\s*pragma\s+once\s*(?:/\*.*?\*/\s*)*(?://.*)?$")
ACL_INCLUDE_RE = re.compile(
    r'^\s*#\s*include\s*(?:<(?P<angle>atcoder/[^>]+)>|"(?P<quoted>atcoder/[^"]+)")'
    r'\s*(?:/\*.*?\*/\s*)*(?://.*)?$'
)


class BundleError(RuntimeError):
    pass


def is_inside(path: Path, directory: Path) -> bool:
    try:
        path.relative_to(directory)
        return True
    except ValueError:
        return False


def resolve_library_include(include_path: str, source: Path) -> Path | None:
    raw = Path(include_path)

    if raw.is_absolute():
        candidates = [raw]
    else:
        candidates = [
            source.parent / raw,
            ROOT / raw,
            LIBRARY_DIR / raw,
        ]

    checked: set[Path] = set()

    for candidate in candidates:
        resolved = candidate.resolve()

        if resolved in checked:
            continue
        checked.add(resolved)

        if resolved.is_file() and is_inside(resolved, LIBRARY_DIR):
            return resolved

    return None


def looks_like_library_include(include_path: str) -> bool:
    path = include_path.replace("\\", "/")
    return path == "library" or path.startswith("library/")


def find_acl_directory(acl_dir: Path | None) -> Path | None:
    """atcoder/ 自体、またはそれを含む include ディレクトリから ACL を探す。"""
    if acl_dir is not None:
        candidates = [acl_dir]
    else:
        candidates = []
        for variable in ("CPATH", "CPLUS_INCLUDE_PATH"):
            value = os.environ.get(variable)
            if value:
                candidates.extend(Path(entry or ".") for entry in value.split(os.pathsep))
        candidates.extend([ROOT / "ac-library", ROOT, Path("/usr/local/include"), Path("/usr/include")])

    for candidate in candidates:
        directory = (candidate if candidate.name == "atcoder" else candidate / "atcoder").resolve()
        if directory.is_dir():
            return directory
    return None


def resolve_acl_include(include_path: str, acl_directory: Path | None) -> Path:
    if acl_directory is None:
        raise BundleError("cannot find ACL headers; specify --acl-dir or configure CPLUS_INCLUDE_PATH")
    target = (acl_directory / include_path.removeprefix("atcoder/")).resolve()
    if not target.is_file() or not is_inside(target, acl_directory):
        raise BundleError(f'cannot resolve ACL include "{include_path}" from {acl_directory}')
    return target


def bundle_file(
    source: Path,
    included: set[Path],
    *,
    is_library_file: bool,
    expand_acl: bool = False,
    acl_directory: Path | None = None,
) -> list[str]:
    source = source.resolve()

    if is_library_file:
        if source in included:
            return []
        included.add(source)

    try:
        lines = source.read_text(encoding="utf-8").splitlines()
    except OSError as exc:
        raise BundleError(f"failed to read {source}: {exc}") from exc

    output: list[str] = []

    for line in lines:
        if is_library_file and PRAGMA_ONCE_RE.match(line):
            continue

        # ACL の判定・探索はオプション指定時だけ行い、従来の処理を維持する。
        acl_match = ACL_INCLUDE_RE.match(line) if expand_acl else None
        if acl_match is not None:
            include_path = acl_match.group("angle") or acl_match.group("quoted")
            target = resolve_acl_include(include_path, acl_directory)
            relative = "atcoder/" + target.relative_to(acl_directory).as_posix()
        else:
            match = LOCAL_INCLUDE_RE.match(line)
            if match is None:
                output.append(line)
                continue

            include_path = match.group("path")
            target = resolve_library_include(include_path, source)

            if target is None:
                if is_library_file or looks_like_library_include(include_path):
                    raise BundleError(
                        f'cannot resolve library include "{include_path}" from {source}'
                    )

                output.append(line)
                continue

            relative = target.relative_to(ROOT).as_posix()
        if target in included:
            continue

        output.append(f"// BEGIN: {relative}")
        output.extend(
            bundle_file(
                target,
                included,
                is_library_file=True,
                expand_acl=expand_acl,
                acl_directory=acl_directory,
            )
        )
        output.append(f"// END: {relative}")

    return output


def bundle(source: Path, *, expand_acl: bool = False, acl_dir: Path | None = None) -> str:
    source = source.resolve()

    if not source.is_file():
        raise BundleError(f"input file not found: {source}")

    if not LIBRARY_DIR.is_dir():
        raise BundleError(f"library directory not found: {LIBRARY_DIR}")

    acl_directory = find_acl_directory(acl_dir) if expand_acl else None
    lines = bundle_file(
        source, set(), is_library_file=False,
        expand_acl=expand_acl, acl_directory=acl_directory,
    )
    return "\n".join(lines) + "\n"


def default_output_path(source: Path) -> Path:
    return source.with_name(f"{source.stem}.bundled{source.suffix}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Expand includes under this repository's library/ directory "
            "into a single C++ source file."
        )
    )
    parser.add_argument("input", type=Path, help="input C++ source file")
    parser.add_argument("--expand-acl", action="store_true", help="also expand atcoder/ includes")
    parser.add_argument(
        "--acl-dir", type=Path,
        help="ACL checkout/include directory or atcoder/ directory (used with --expand-acl)",
    )
    output_group = parser.add_mutually_exclusive_group()
    output_group.add_argument(
        "-o",
        "--output",
        type=Path,
        help='output file (default: <input>.bundled.cpp); use "-" for stdout',
    )
    output_group.add_argument(
        "--stdout",
        action="store_true",
        help="write bundled source to stdout instead of a file",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    try:
        bundled = bundle(args.input, expand_acl=args.expand_acl, acl_dir=args.acl_dir)

        if args.stdout or args.output == Path("-"):
            sys.stdout.write(bundled)
            return 0

        output_path = args.output or default_output_path(args.input)
        output_path.write_text(bundled, encoding="utf-8", newline="\n")
    except (BundleError, OSError) as exc:
        print(f"bundle.py: error: {exc}", file=sys.stderr)
        return 1

    print(output_path.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

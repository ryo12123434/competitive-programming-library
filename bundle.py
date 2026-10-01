#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent
LIBRARY_DIR = ROOT / "library"

LOCAL_INCLUDE_RE = re.compile(
    r'^(?P<indent>\\s*)#\\s*include\\s*"(?P<path>[^"]+)"(?P<suffix>\\s*(?://.*)?)$'
)
PRAGMA_ONCE_RE = re.compile(r"^\\s*#\\s*pragma\\s+once(?:\\s*(?://.*)?)?$")


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


def bundle_file(
    source: Path,
    included: set[Path],
    *,
    is_library_file: bool,
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

        if target in included:
            continue

        relative = target.relative_to(ROOT).as_posix()
        output.append(f"// BEGIN: {relative}")
        output.extend(
            bundle_file(
                target,
                included,
                is_library_file=True,
            )
        )
        output.append(f"// END: {relative}")

    return output


def bundle(source: Path) -> str:
    source = source.resolve()

    if not source.is_file():
        raise BundleError(f"input file not found: {source}")

    if not LIBRARY_DIR.is_dir():
        raise BundleError(f"library directory not found: {LIBRARY_DIR}")

    lines = bundle_file(source, set(), is_library_file=False)
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
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help="output file (default: <input>.bundled.cpp)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output_path = args.output or default_output_path(args.input)

    try:
        bundled = bundle(args.input)
        output_path.write_text(bundled, encoding="utf-8", newline="\n")
    except (BundleError, OSError) as exc:
        print(f"bundle.py: error: {exc}", file=sys.stderr)
        return 1

    print(output_path.resolve())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

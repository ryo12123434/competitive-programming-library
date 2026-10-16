#!/usr/bin/env python3
"""入力生成器・高速解法・愚直解法を比較する。Python 3.10+、外部 Python 依存なし。"""
from __future__ import annotations

import argparse
from contextlib import ExitStack
from dataclasses import dataclass
import hashlib
import json
import math
import os
from pathlib import Path
import signal
import subprocess
import sys
import tempfile
import time
import zipfile


ROOT = Path(__file__).resolve().parents[1]
FORMAT = "cpl-stress-v1"
CASE_FILES = ("failed.in", "expected.txt", "actual.txt", "meta.json", "original.in",
              "generator.stderr.txt", "fast.stderr.txt", "brute.stderr.txt", "shrinker.stderr.txt")


@dataclass
class Result:
    command: list[str]
    stdout: bytes = b""
    stderr: bytes = b""
    returncode: int | None = None
    error: str | None = None
    elapsed: float = 0

    def metadata(self):
        return {"command": self.command, "returncode": self.returncode,
                "error": self.error, "elapsed_seconds": self.elapsed}


def kill_tree(process):
    """タイムアウトしたプロセスと子プロセスを終了する。"""
    if os.name == "nt":
        try:
            subprocess.run(["taskkill", "/PID", str(process.pid), "/T", "/F"],
                           stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, timeout=5)
        except (OSError, subprocess.TimeoutExpired):
            pass
        if process.poll() is None:
            process.kill()
    else:
        try:
            os.killpg(process.pid, signal.SIGKILL)
        except ProcessLookupError:
            pass


def run(command, data, timeout, limit):
    """シェルを通さず実行。出力は一時ファイルに置き、読み込み量を制限する。"""
    result = Result(list(command))
    start = time.monotonic()
    with tempfile.TemporaryFile() as out, tempfile.TemporaryFile() as err:
        options = {"creationflags": subprocess.CREATE_NEW_PROCESS_GROUP} if os.name == "nt" else {"start_new_session": True}
        try:
            process = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=out, stderr=err, **options)
        except OSError as exc:
            result.error = "launch_error"
            result.stderr = str(exc).encode("utf-8")
            result.elapsed = time.monotonic() - start
            return result
        try:
            process.communicate(data, timeout=timeout)
        except subprocess.TimeoutExpired:
            result.error = "timeout"
            kill_tree(process)
            process.communicate()
        except BaseException:
            kill_tree(process)
            process.wait()
            raise
        result.returncode = process.returncode
        out.seek(0)
        err.seek(0)
        result.stdout = out.read(limit + 1)
        result.stderr = err.read(limit + 1)
        if len(result.stdout) > limit or len(result.stderr) > limit:
            result.error = result.error or "output_limit"
            result.stdout = result.stdout[:limit]
            result.stderr = result.stderr[:limit]
        if result.returncode != 0:
            result.error = result.error or "nonzero_exit"
    result.elapsed = time.monotonic() - start
    return result


def parse_spec(spec):
    """通常はファイル名。追加引数は JSON 配列で渡し、Windows の引用規則への依存を避ける。"""
    if isinstance(spec, list):
        parts = spec
    elif spec.lstrip().startswith("["):
        parts = json.loads(spec)
    else:
        parts = [spec]
    if not isinstance(parts, list) or not parts or not all(isinstance(part, str) and part for part in parts):
        raise ValueError("command must be a path or a nonempty JSON array of strings")
    parts = list(parts)
    path = Path(parts[0])
    if path.is_file():
        parts[0] = str(path.resolve())
    return parts


def prepare(spec, role, args, directory):
    parts = parse_spec(spec)
    path = Path(parts[0])
    if path.suffix.lower() in (".cpp", ".cc", ".cxx"):
        if not path.is_file():
            raise ValueError(f"source not found: {path}")
        binary = directory / "bin" / (role + ".exe")
        binary.parent.mkdir(parents=True, exist_ok=True)
        command = [args.cxx, "-std=gnu++23", "-O2", "-Wall", "-Wextra", "-I", str(ROOT)]
        if args.local:
            command.append("-DLOCAL")
        if args.sanitize:
            command += ["-O1", "-g", "-fno-omit-frame-pointer", "-fsanitize=" + args.sanitize,
                        "-fno-sanitize-recover=all"]
        command += args.cxxflag + [str(path.resolve()), "-o", str(binary)]
        compiled = run(command, b"", args.compile_timeout, args.output_limit)
        if compiled.error:
            raise ValueError(f"compile {role}: {compiled.error}\n" + compiled.stderr.decode("utf-8", "replace"))
        return [str(binary), *parts[1:]], parts
    if path.suffix.lower() == ".py":
        return [sys.executable, str(path.resolve()), *parts[1:]], parts
    return parts, parts


def evaluate(data, commands, args):
    results = {role: run(commands[role], data, args.timeout, args.output_limit) for role in ("fast", "brute")}
    for role in ("fast", "brute"):
        if results[role].error:
            return role + "_" + results[role].error, results
    actual, expected = results["fast"].stdout, results["brute"].stdout
    equal = actual == expected if args.compare == "exact" else actual.split() == expected.split()
    return (None if equal else "wrong_answer"), results


def shrink(data, reason, results, command, commands, args):
    """同じ失敗分類で、(バイト数, 辞書順) が小さい候補だけ採用する。制約は縮小器が保証する。"""
    info = {"accepted_steps": 0, "status": "step_limit", "command": command}
    last = None
    for _ in range(args.shrink_steps):
        last = run(command, data, args.timeout, args.output_limit)
        if last.error:
            info["status"] = "shrinker_" + last.error
            break
        candidate = last.stdout
        if (len(candidate), candidate) >= (len(data), data):
            info["status"] = "no_smaller_candidate"
            break
        candidate_reason, candidate_results = evaluate(candidate, commands, args)
        if candidate_reason != reason:
            info["status"] = "failure_not_preserved"
            break
        data, results = candidate, candidate_results
        info["accepted_steps"] += 1
    return data, results, info, last


def save_case(directory, data, original, results, metadata):
    """固定名だけを書き換える。成功した実行では前回の反例を消さない。"""
    files = {"failed.in": data, "original.in": original,
             "expected.txt": results.get("brute", Result([])).stdout,
             "actual.txt": results.get("fast", Result([])).stdout}
    for role in ("generator", "fast", "brute", "shrinker"):
        files[role + ".stderr.txt"] = results.get(role, Result([])).stderr
    metadata["input_sha256"] = hashlib.sha256(data).hexdigest()
    metadata["results"] = {role: result.metadata() for role, result in results.items()}
    files["meta.json"] = (json.dumps(metadata, ensure_ascii=False, indent=2) + "\n").encode("utf-8")
    for name, content in files.items():
        (directory / name).write_bytes(content)


def export_case(directory, archive, prefix=""):
    for name in CASE_FILES:
        archive.write(directory / name, prefix + name)


def parser():
    result = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    result.add_argument("--gen", help="入力生成器。seed とテスト番号を末尾の引数として渡す")
    result.add_argument("--fast", help="高速解法: C++ / Python / 実行ファイル / JSON argv")
    result.add_argument("--brute", help="愚直解法: C++ / Python / 実行ファイル / JSON argv")
    result.add_argument("-n", "--tests", type=int, help="試行数。通常 1000、固定入力・replay は 1")
    result.add_argument("--seed", type=int, help="基準 seed（uint64、既定値 0）")
    result.add_argument("--case", type=int, help="開始するテスト番号（1-indexed）。失敗番号から直接再生成できる")
    source = result.add_mutually_exclusive_group()
    source.add_argument("--input", type=Path, help="生成器の代わりに読む入力ファイル")
    source.add_argument("--replay", action="store_true", help="out の反例と metadata のプログラム指定を再利用する")
    result.add_argument("--out", type=Path, default=Path(".stress"), help="反例・固定名のビルド出力の保存先")
    result.add_argument("--timeout", type=float, help="各プログラムの実時間制限（秒）、既定値 2、replay では保存値")
    result.add_argument("--compile-timeout", type=float, default=60, help="各コンパイルの制限（秒）")
    result.add_argument("--output-limit", type=int, help="各 stdout / stderr の読み込み上限（bytes）、既定値 8 MiB")
    result.add_argument("--compare", choices=("tokens", "exact"), default=None, help="既定値 tokens: 空白区切りで比較")
    result.add_argument("--collect", type=int, default=1, help="失敗数の上限。2 以上なら collected.zip にも番号付きで保存")
    result.add_argument("--archive", type=Path, help="最終反例を指定した新しい ZIP に保存（明示指定時だけ）")
    result.add_argument("--shrinker", help="stdin の有効入力を縮小し stdout に候補を一つ返すプログラム")
    result.add_argument("--shrink-steps", type=int, default=30, help="反例ごとの縮小器の呼び出し上限")
    result.add_argument("--cxx", help="C++ コンパイラ。既定値 g++、replay では保存値")
    result.add_argument("--cxxflag", action="append", help="追加のコンパイル引数。例: --cxxflag=-Werror")
    result.add_argument("--local", action=argparse.BooleanOptionalAction, default=None, help="C++ を -DLOCAL でコンパイルする")
    result.add_argument("--sanitize", nargs="?", const="undefined", choices=("none", "undefined", "address", "address,undefined"),
                        help="コンパイラが対応する Sanitizer を指定。省略値は undefined")
    return result


def execute(args):
    directory = args.out.resolve()
    saved = {}
    if args.replay:
        saved = json.loads((directory / "meta.json").read_text(encoding="utf-8"))
        if saved.get("format") != FORMAT:
            raise ValueError("unsupported meta.json format")
        args.fast = args.fast or saved["specs"]["fast"]
        args.brute = args.brute or saved["specs"]["brute"]
        args.input = directory / "failed.in"
    args.seed = args.seed if args.seed is not None else saved.get("master_seed", 0)
    args.case = args.case if args.case is not None else saved.get("case_index", 1)
    args.tests = args.tests if args.tests is not None else (1 if args.input else 1000)
    args.compare = args.compare or saved.get("compare", "tokens")
    args.timeout = args.timeout if args.timeout is not None else saved.get("timeout_seconds", 2)
    args.output_limit = args.output_limit if args.output_limit is not None else saved.get("output_limit", 8 * 1024 * 1024)
    args.cxx = args.cxx or saved.get("cxx", "g++")
    args.cxxflag = args.cxxflag if args.cxxflag is not None else saved.get("cxxflags", [])
    args.local = args.local if args.local is not None else saved.get("local", False)
    args.sanitize = args.sanitize if args.sanitize is not None else saved.get("sanitize")
    if args.sanitize == "none":
        args.sanitize = None
    if not args.fast or not args.brute or (not args.input and not args.gen):
        raise ValueError("--fast / --brute と --gen または --input が必要です")
    if not (0 <= args.seed < 2**64) or args.case < 1 or args.tests < 1 or args.collect < 1 or args.shrink_steps < 0:
        raise ValueError("seed は uint64、case / tests / collect は正、shrink-steps は非負にしてください")
    if not all(math.isfinite(value) and value > 0 for value in (args.timeout, args.compile_timeout)) or args.output_limit < 1:
        raise ValueError("timeout と output-limit は正の有限値にしてください")
    if args.archive and args.archive.exists():
        raise ValueError("archive already exists; choose a new filename")
    fixed = None
    if args.input:
        with args.input.open("rb") as source:
            fixed = source.read(args.output_limit + 1)
    if fixed is not None and len(fixed) > args.output_limit:
        raise ValueError("input exceeds --output-limit")
    directory.mkdir(parents=True, exist_ok=True)
    commands, specs = {}, dict(saved.get("specs", {}))
    for role in ("gen", "fast", "brute", "shrinker"):
        spec = getattr(args, role)
        if spec and (role != "gen" or fixed is None):
            commands[role], specs[role] = prepare(spec, role, args, directory)
    failed = 0
    with ExitStack() as stack:
        collection = stack.enter_context(zipfile.ZipFile(directory / "collected.zip", "w")) if args.collect > 1 else None
        for index in range(args.case, args.case + args.tests):
            seed = (args.seed + index - 1) % 2**64
            results = {}
            if fixed is None:
                generated = run(commands["gen"] + [str(seed), str(index)], b"", args.timeout, args.output_limit)
                data = generated.stdout
                results["generator"] = generated
                reason = "generator_" + generated.error if generated.error else None
            else:
                data, reason = fixed, None
            if reason is None:
                reason, answers = evaluate(data, commands, args)
                results.update(answers)
            if reason is None:
                continue
            original = data
            metadata = {"format": FORMAT, "master_seed": args.seed, "case_index": index, "case_seed": seed,
                        "cwd": str(Path.cwd()), "specs": specs, "commands": commands, "reason": reason,
                        "compare": args.compare, "timeout_seconds": args.timeout,
                        "output_limit": args.output_limit,
                        "local": args.local, "sanitize": args.sanitize, "cxx": args.cxx, "cxxflags": args.cxxflag,
                        "original_size": len(original), "input_source": str(args.input) if args.input else "generator"}
            # 縮小器が停止・失敗しても、まず元の反例を確保する。
            save_case(directory, data, original, results, metadata)
            if "shrinker" in commands and not reason.startswith("generator_"):
                data, answers, info, last = shrink(data, reason, {r: results[r] for r in ("fast", "brute")},
                                                 commands["shrinker"], commands, args)
                results.update(answers)
                if last is not None:
                    results["shrinker"] = last
                metadata["shrink"] = info
                save_case(directory, data, original, results, metadata)
            failed += 1
            print(f"FAIL case={index} seed={seed} master_seed={args.seed} reason={reason}")
            print(data[:4000].decode("utf-8", "replace"))
            if len(data) > 4000:
                print("... (input display truncated)")
            print(f"saved: {directory / 'failed.in'}")
            if fixed is None:
                print(f"regenerate: --seed {args.seed} --case {index} --tests 1")
            if collection:
                export_case(directory, collection, f"{failed:04d}/")
            if failed >= args.collect:
                break
    if failed and args.archive:
        args.archive.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(args.archive, "x") as archive:
            export_case(directory, archive)
    if not failed:
        print(f"OK: {args.tests} cases (master_seed={args.seed}, first_case={args.case})")
    else:
        print(f"FAILED: {failed} cases saved")
    return int(failed > 0)


def main(argv=None):
    args = parser().parse_args(argv)
    try:
        return execute(args)
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f"stress: {exc}", file=sys.stderr)
        return 2
    except KeyboardInterrupt:
        print("stress: interrupted", file=sys.stderr)
        return 130


if __name__ == "__main__":
    sys.exit(main())

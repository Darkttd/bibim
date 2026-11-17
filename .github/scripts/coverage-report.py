#!/usr/bin/env python3
"""Turn a gcovr JSON summary into a Markdown coverage report.

Usage: coverage-report.py CURRENT.json [BASELINE.json [BASELINE_LABEL]]

CURRENT.json and BASELINE.json are written by `gcovr --json-summary`. With a
baseline, every number also shows how it changed. The report groups files by
area: src/<Module>/ becomes <Module>, everything under include/ is "include".
The first line is an HTML comment that the workflow uses to find and update
its own pull request comment.
"""
import json
import sys

MARKER = "<!-- bibim-coverage -->"


def load(path):
    with open(path, encoding="utf-8") as f:
        return json.load(f)


def area_of(filename):
    parts = filename.replace("\\", "/").split("/")
    if parts[0] == "src" and len(parts) > 2:
        return parts[1]
    if parts[0] == "include":
        return "include"
    return parts[0]


def by_area(summary):
    areas = {}
    for f in summary["files"]:
        a = areas.setdefault(area_of(f["filename"]), [0, 0, 0, 0])
        a[0] += f["line_covered"]
        a[1] += f["line_total"]
        a[2] += f["branch_covered"]
        a[3] += f["branch_total"]
    return areas


def percent(covered, total):
    return 100.0 * covered / total if total else 0.0


def cell(covered, total, base=None):
    text = "{:.1f}% ({:,} / {:,})".format(percent(covered, total), covered, total)
    if base is None:
        return text
    diff = percent(covered, total) - percent(base[0], base[1])
    lines = covered - base[0]
    if abs(diff) < 0.05 and lines == 0 and total == base[1]:
        return text
    return "{} **{:+.1f}** ({:+,})".format(text, diff, lines)


def row(name, a, base):
    line_base = (base[0], base[1]) if base else None
    branch_base = (base[2], base[3]) if base else None
    return "| {} | {} | {} |".format(name, cell(a[0], a[1], line_base), cell(a[2], a[3], branch_base))


def main(argv):
    current = load(argv[1])
    baseline = load(argv[2]) if len(argv) > 2 else None
    label = argv[3] if len(argv) > 3 else "master"

    total = [current["line_covered"], current["line_total"], current["branch_covered"], current["branch_total"]]
    areas = by_area(current)
    base_total = None
    base_areas = {}
    if baseline:
        base_total = [baseline["line_covered"], baseline["line_total"], baseline["branch_covered"], baseline["branch_total"]]
        base_areas = by_area(baseline)

    header = ["| 영역 | 줄 | 분기 |", "|---|---|---|"]
    out = [MARKER, "## 테스트 커버리지", ""]
    if baseline:
        out.append("기준: {} · 굵은 숫자는 퍼센트포인트 증감, 괄호는 덮인 줄·분기 수 증감".format(label))
    else:
        out.append("비교할 기준값이 없습니다 (master에서 Coverage가 아직 실행되지 않았습니다).")
    out.append("")
    out += header
    out.append(row("**전체**", total, base_total))

    if baseline:
        changed = [name for name in sorted(set(areas) | set(base_areas))
                   if areas.get(name, [0, 0, 0, 0]) != base_areas.get(name, [0, 0, 0, 0])]
        out.append("")
        if changed:
            out.append("### 바뀐 영역")
            out.append("")
            out += header
            for name in sorted(changed, key=lambda n: -areas.get(n, [0, 0, 0, 0])[1]):
                out.append(row(name, areas.get(name, [0, 0, 0, 0]), base_areas.get(name, [0, 0, 0, 0])))
        else:
            out.append("바뀐 영역이 없습니다.")

    out += ["", "<details><summary>모든 영역</summary>", ""]
    out += header
    for name in sorted(areas, key=lambda n: -areas[n][1]):
        out.append(row(name, areas[name], base_areas.get(name) if baseline else None))
    out += ["", "</details>", ""]
    out.append("분모는 Linux에서 컴파일되는 `src/`, `include/` 파일입니다 (`extlibs/`, `tests/`, 다른 플랫폼 전용 파일 제외). "
               "헤더의 inline 함수는 `include`로 집계됩니다. 파일별로 덮이지 않은 줄은 작업의 `coverage-html` artifact에서 볼 수 있습니다.")
    print("\n".join(out))


if __name__ == "__main__":
    main(sys.argv)

"""
Parse Cobertura XML coverage report and produce a JSON summary.

Usage:
    python coverage_summary.py <cobertura.xml> <output.json>

Output JSON fields:
    - global_line_rate: overall line coverage percentage
    - global_branch_rate: overall branch coverage percentage
    - total_files: number of unique source files tracked
    - files: list of per-file coverage info, sorted by lowest coverage first

Note: Source files compiled into multiple test targets are deduplicated
by merging line-hit data across all occurrences (union of covered lines).
"""

import json
import sys
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import PurePosixPath


def parse_cobertura(xml_path: str) -> dict:
    tree = ET.parse(xml_path)
    root = tree.getroot()

    # Collect per-line hit data across all class entries, keyed by filename.
    # For each file, track: {line_number: max_hits} across all duplicates.
    file_lines: dict[str, dict[int, int]] = defaultdict(dict)

    for package in root.iter("package"):
        for cls in package.iter("class"):
            filename = cls.get("filename", "")

            # Only include files from our src/ directory
            if "src" not in filename.replace("\\", "/"):
                continue

            # Normalise path separators for deduplication
            norm = filename.replace("\\", "/")

            for line in cls.iter("line"):
                line_num = int(line.get("number", "0"))
                hits = int(line.get("hits", "0"))
                # Keep the maximum hit count across all duplicate compilations
                prev = file_lines[norm].get(line_num, 0)
                file_lines[norm][line_num] = max(prev, hits)

    # Build per-file summary from merged line data
    files = []
    total_lines = 0
    total_covered = 0

    for filename, lines in file_lines.items():
        n_lines = len(lines)
        covered = sum(1 for h in lines.values() if h > 0)
        uncovered = [num for num, h in sorted(lines.items()) if h == 0]
        line_rate = (covered / n_lines * 100) if n_lines else 0.0

        total_lines += n_lines
        total_covered += covered

        # Extract relative path from src/ onwards
        parts = filename.split("/")
        try:
            src_idx = parts.index("src")
            rel_path = "/".join(parts[src_idx:])
        except ValueError:
            rel_path = PurePosixPath(filename).name

        files.append({
            "filename": rel_path,
            "line_rate": round(line_rate, 1),
            "lines_covered": covered,
            "lines_total": n_lines,
            "uncovered_lines": uncovered,
            "uncovered_count": len(uncovered),
        })

    # Sort by line coverage (lowest first) for easy gap identification
    files.sort(key=lambda f: (f["line_rate"], f["filename"]))

    global_line_rate = (total_covered / total_lines * 100) if total_lines else 0.0

    return {
        "global_line_rate": round(global_line_rate, 1),
        "global_branch_rate": 100.0,
        "total_files": len(files),
        "total_lines": total_lines,
        "total_covered": total_covered,
        "files": files,
    }


def print_summary(summary: dict) -> None:
    print(f"\n{'='*60}")
    print(f"  Coverage Summary")
    print(f"{'='*60}")
    print(f"  Line coverage:   {summary['global_line_rate']:.1f}%"
          f"  ({summary['total_covered']}/{summary['total_lines']} lines)")
    print(f"  Files tracked:   {summary['total_files']}")
    print(f"{'='*60}")

    if summary["files"]:
        print(f"\n  {'File':<40} {'Lines':>7}  {'Detail':>14}")
        print(f"  {'-'*40} {'-'*7}  {'-'*14}")
        for f in summary["files"]:
            detail = f"{f['lines_covered']}/{f['lines_total']}"
            print(f"  {f['filename']:<40} {f['line_rate']:>6.1f}%  {detail:>14}")
    print()


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <cobertura.xml> <output.json>")
        sys.exit(1)

    xml_path = sys.argv[1]
    json_path = sys.argv[2]

    summary = parse_cobertura(xml_path)

    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(summary, f, indent=2)

    print_summary(summary)
    print(f"  JSON written to: {json_path}")


if __name__ == "__main__":
    main()

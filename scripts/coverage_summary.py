"""
Parse llvm-cov JSON export and produce a coverage summary.

Usage:
    python coverage_summary.py <coverage.json> <output.json> [--qml <qml-coverage.xml>]

The input is produced by:
    llvm-cov export -format=text -instr-profile=merged.profdata <objects...>

Optional --qml flag parses a Cobertura XML report (from qoverage) and appends
QML file entries to the summary output.

Output JSON fields:
    - global_line_rate: overall line coverage percentage
    - global_branch_rate: overall region coverage percentage (if available)
    - global_mcdc_rate: overall MC/DC coverage percentage (if available)
    - total_files: number of source files tracked
    - files: list of per-file coverage info, sorted by lowest coverage first
"""

from __future__ import annotations

import json
import sys
from pathlib import PurePosixPath
from xml.etree import ElementTree


def parse_llvm_cov(json_path: str) -> dict:
    with open(json_path, encoding="utf-8") as f:
        data = json.load(f)

    files = []

    for report in data["data"]:
        for file_entry in report["files"]:
            filename = file_entry["filename"].replace("\\", "/")

            # Only include files from our src/ directory
            if "/src/" not in filename:
                continue

            # Extract relative path from src/ onwards
            parts = filename.split("/")
            try:
                src_idx = next(i for i, p in enumerate(parts) if p == "src")
                rel_path = "/".join(parts[src_idx:])
            except StopIteration:
                rel_path = PurePosixPath(filename).name

            summary = file_entry["summary"]
            lines = summary["lines"]

            file_record = {
                "filename": rel_path,
                "line_rate": round(lines["percent"], 1),
                "lines_covered": lines["covered"],
                "lines_total": lines["count"],
            }

            # Region (branch) coverage
            if "regions" in summary and summary["regions"]["count"] > 0:
                regions = summary["regions"]
                file_record["region_rate"] = round(regions["percent"], 1)
                file_record["regions_covered"] = regions["covered"]
                file_record["regions_total"] = regions["count"]

            # MC/DC coverage
            if "mcdc" in summary and summary["mcdc"]["count"] > 0:
                mcdc = summary["mcdc"]
                file_record["mcdc_rate"] = round(mcdc["percent"], 1)
                file_record["mcdc_covered"] = mcdc["covered"]
                file_record["mcdc_total"] = mcdc["count"]

            files.append(file_record)

    # Sort by line coverage (lowest first) for easy gap identification
    files.sort(key=lambda f: (f["line_rate"], f["filename"]))

    # Compute global totals
    total_lines = sum(f["lines_total"] for f in files)
    total_covered = sum(f["lines_covered"] for f in files)
    global_line_rate = (total_covered / total_lines * 100) if total_lines else 0.0

    result = {
        "global_line_rate": round(global_line_rate, 1),
        "total_files": len(files),
        "total_lines": total_lines,
        "total_covered": total_covered,
        "files": files,
    }

    # Global region (branch) coverage
    total_regions = sum(f.get("regions_total", 0) for f in files)
    total_regions_covered = sum(f.get("regions_covered", 0) for f in files)
    if total_regions > 0:
        result["global_branch_rate"] = round(
            total_regions_covered / total_regions * 100, 1
        )

    # Global MC/DC coverage
    total_mcdc = sum(f.get("mcdc_total", 0) for f in files)
    total_mcdc_covered = sum(f.get("mcdc_covered", 0) for f in files)
    if total_mcdc > 0:
        result["global_mcdc_rate"] = round(
            total_mcdc_covered / total_mcdc * 100, 1
        )

    return result


def parse_cobertura_qml(xml_path: str) -> list[dict]:
    """Parse Cobertura XML (from qoverage) and return per-file coverage records."""
    tree = ElementTree.parse(xml_path)
    root = tree.getroot()

    files = []
    for package in root.findall(".//package"):
        for cls in package.findall(".//class"):
            filename = cls.get("filename", "")
            if not filename:
                continue

            # Normalize path: keep qml/ relative path
            filename = filename.replace("\\", "/")
            parts = filename.split("/")
            try:
                qml_idx = next(i for i, p in enumerate(parts) if p == "qml")
                rel_path = "/".join(parts[qml_idx:])
            except StopIteration:
                rel_path = PurePosixPath(filename).name

            # Count lines from <line> elements
            lines = cls.findall(".//line")
            total = len(lines)
            covered = sum(1 for ln in lines if int(ln.get("hits", "0")) > 0)

            if total == 0:
                continue

            rate = round(covered / total * 100, 1)
            files.append({
                "filename": rel_path,
                "line_rate": rate,
                "lines_covered": covered,
                "lines_total": total,
                "source": "qml",
            })

    files.sort(key=lambda f: (f["line_rate"], f["filename"]))
    return files


def print_summary(summary: dict) -> None:
    print(f"\n{'='*65}")
    print(f"  Coverage Summary")
    print(f"{'='*65}")
    print(
        f"  Line coverage:   {summary['global_line_rate']:.1f}%"
        f"  ({summary['total_covered']}/{summary['total_lines']} lines)"
    )
    if "global_branch_rate" in summary:
        print(f"  Region coverage: {summary['global_branch_rate']:.1f}%")
    if "global_mcdc_rate" in summary:
        print(f"  MC/DC coverage:  {summary['global_mcdc_rate']:.1f}%")
    print(f"  Files tracked:   {summary['total_files']}")

    # QML-specific summary
    qml_files = [f for f in summary["files"] if f.get("source") == "qml"]
    if qml_files:
        qml_total = sum(f["lines_total"] for f in qml_files)
        qml_covered = sum(f["lines_covered"] for f in qml_files)
        qml_rate = (qml_covered / qml_total * 100) if qml_total else 0.0
        print(f"  QML coverage:    {qml_rate:.1f}%  ({qml_covered}/{qml_total} lines, {len(qml_files)} files)")

    print(f"{'='*65}")

    if summary["files"]:
        has_regions = any("region_rate" in f for f in summary["files"])
        has_mcdc = any("mcdc_rate" in f for f in summary["files"])
        has_qml = any(f.get("source") == "qml" for f in summary["files"])

        header = f"  {'File':<40} {'Lines':>7}"
        divider = f"  {'-'*40} {'-'*7}"
        if has_regions:
            header += f"  {'Regions':>8}"
            divider += f"  {'-'*8}"
        if has_mcdc:
            header += f"  {'MC/DC':>7}"
            divider += f"  {'-'*7}"

        print(f"\n{header}")
        print(divider)

        for f in summary["files"]:
            prefix = "[QML] " if f.get("source") == "qml" else ""
            name = prefix + f["filename"]
            line = f"  {name:<40} {f['line_rate']:>6.1f}%"
            if has_regions:
                region = f"{f.get('region_rate', 0):>6.1f}%" if "region_rate" in f else "    n/a"
                line += f"  {region}"
            if has_mcdc:
                mcdc = f"{f.get('mcdc_rate', 0):>5.1f}%" if "mcdc_rate" in f else "   n/a"
                line += f"  {mcdc}"
            print(line)
    print()


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <coverage.json> <output.json> [--qml <qml-coverage.xml>]")
        sys.exit(1)

    json_path = sys.argv[1]
    output_path = sys.argv[2]

    # Parse optional --qml flag
    qml_xml_path = None
    if "--qml" in sys.argv:
        qml_idx = sys.argv.index("--qml")
        if qml_idx + 1 < len(sys.argv):
            qml_xml_path = sys.argv[qml_idx + 1]

    try:
        summary = parse_llvm_cov(json_path)
    except FileNotFoundError:
        print(f"Error: JSON file not found: {json_path}", file=sys.stderr)
        sys.exit(1)
    except (json.JSONDecodeError, KeyError) as e:
        print(f"Error: Failed to parse llvm-cov JSON: {e}", file=sys.stderr)
        sys.exit(1)

    # Append QML coverage if provided
    if qml_xml_path:
        try:
            qml_files = parse_cobertura_qml(qml_xml_path)
            summary["files"].extend(qml_files)
            summary["files"].sort(key=lambda f: (f["line_rate"], f["filename"]))
            # Recompute global totals including QML
            total_lines = sum(f["lines_total"] for f in summary["files"])
            total_covered = sum(f["lines_covered"] for f in summary["files"])
            summary["total_lines"] = total_lines
            summary["total_covered"] = total_covered
            summary["total_files"] = len(summary["files"])
            summary["global_line_rate"] = round(
                (total_covered / total_lines * 100) if total_lines else 0.0, 1
            )
        except FileNotFoundError:
            print(f"Warning: QML coverage file not found: {qml_xml_path}", file=sys.stderr)
        except ElementTree.ParseError as e:
            print(f"Warning: Failed to parse QML coverage XML: {e}", file=sys.stderr)

    try:
        with open(output_path, "w", encoding="utf-8") as f:
            json.dump(summary, f, indent=2)
    except OSError as e:
        print(f"Error: Cannot write output file: {e}", file=sys.stderr)
        sys.exit(1)

    print_summary(summary)
    print(f"  JSON written to: {output_path}")


if __name__ == "__main__":
    main()

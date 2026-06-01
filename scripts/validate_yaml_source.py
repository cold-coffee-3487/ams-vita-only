#!/usr/bin/env python3
"""Validate the AMS VITA YAML source of truth."""

from pathlib import Path
import re
import sys
from typing import Any, Optional

import yaml

YAML_PATH = Path("spec/ams_vita_49-2_tailoring.yaml")
EXACT_TOKEN_RE = re.compile(r"(?:0|[1-9][0-9]*|[01]+|0x[0-9A-Fa-f]+)")
RANGE_TOKEN_RE = re.compile(r"(?:0|[1-9][0-9]*|[01]+|0x[0-9A-Fa-f]+):(?:0|[1-9][0-9]*|[01]+|0x[0-9A-Fa-f]+|2\^[1-9][0-9]*-1)")
RANGE_GRAMMAR_RE = re.compile(
    rf"^(?:{EXACT_TOKEN_RE.pattern}|{RANGE_TOKEN_RE.pattern})(?:,\s*(?:{EXACT_TOKEN_RE.pattern}|{RANGE_TOKEN_RE.pattern}))*$"
)
DESIGNATION_RE = re.compile(r"^\.[A-Za-z_][A-Za-z0-9_]*(?:\.[A-Za-z_][A-Za-z0-9_]*)*$")
APPROVED_BIT_USED_VALUES = {"B", "E", "N"}
BIT_GRAMMAR_RE = re.compile(r"^(?:N/A|0|[1-9][0-9]*|(?:0|[1-9][0-9]*):(?:0|[1-9][0-9]*))$")
GENERATED_CIF_GROUP_RE = re.compile(r"^\.cif[0-7](?:\s|$)")
DEFAULT_VALUE_RE = re.compile(rf"^(?:N/A|{EXACT_TOKEN_RE.pattern})$")
FIELD_SIZE_WORDS_RE = re.compile(r"^(?:-1|0|[1-9][0-9]*)$")
INDICATOR_GROUP_PREFIXES = (".cif", ".eif", ".wif")
STRUCTURAL_INDICATOR_DESIGNATIONS = {
    ".cfci",
    ".cif1enable",
    ".cif2enable",
    ".cif3enable",
    ".cif4enable",
    ".eif1enable",
    ".eif2enable",
    ".eif3enable",
    ".eif4enable",
}


# Checks that the YAML source of truth can be parsed before any schema-specific
# validations inspect packet sections, groups, fields, or row tables.
def load_yaml() -> tuple[Optional[Any], list[str]]:
    try:
        with YAML_PATH.open("r", encoding="utf-8") as handle:
            return yaml.safe_load(handle), []
    except yaml.YAMLError as exc:
        return None, [f"{YAML_PATH}: YAML parse failed: {exc}"]
    except OSError as exc:
        return None, [f"{YAML_PATH}: could not read file: {exc}"]


def field_location(section: dict[str, Any], group: dict[str, Any], index: int, field: dict[str, Any]) -> str:
    section_title = section.get("title", "<unknown section>")
    group_name = group.get("group_name", group.get("cif_bit", "<unknown group>"))
    designation = field.get("Designation", "<missing designation>")
    return f"{section_title} / {group_name} / field {index} / {designation}"


def iter_groups(data: dict[str, Any]):
    for section in data.get("sections", []):
        for index, group in enumerate(section.get("groups", []), start=1):
            yield section, index, group


def iter_group_fields(data: dict[str, Any]):
    for section, _group_index, group in iter_groups(data):
        for index, field in enumerate(group.get("fields", []), start=1):
            yield section, group, index, field


def section_title(section: dict[str, Any]) -> str:
    return str(section.get("title", "<unknown section>"))


# Checks row-table sections. Headers must be non-empty and unique, and every
# key used by a row must be declared in the section headers so generated tables
# do not silently drop ad hoc row data.
def validate_row_section_headers(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section in data.get("sections", []):
        rows = section.get("rows", [])
        if not rows:
            continue

        headers = section.get("headers", [])
        seen_headers: set[str] = set()
        for index, header in enumerate(headers, start=1):
            header_text = str(header)
            if not header_text:
                errors.append(f"{section_title(section)} / header {index}: empty header")
            if header_text in seen_headers:
                errors.append(f"{section_title(section)} / header {index}: duplicate header {header_text!r}")
            seen_headers.add(header_text)

        for row_index, row in enumerate(rows, start=1):
            for key in row:
                if key not in seen_headers:
                    errors.append(
                        f"{section_title(section)} / row {row_index}: "
                        f"row key {key!r} is not declared in headers"
                    )
    return errors


# Checks row-table Designation values when a row provides one. Row sections may
# contain rows without designations, but any Designation value must be a
# dot-prefixed path rather than prose or display text.
def validate_row_designations(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section in data.get("sections", []):
        for row_index, row in enumerate(section.get("rows", []), start=1):
            if "Designation" not in row:
                continue

            designation = str(row["Designation"])
            if not DESIGNATION_RE.fullmatch(designation):
                errors.append(
                    f"{section_title(section)} / row {row_index}: "
                    f"Designation {designation!r} does not match approved path grammar"
                )
    return errors


# Checks row-table Bits Used values when a row provides one. The row-section
# column uses the same approved tailoring markers as packet group Bit Used:
# B for Base Set, E for Extension Set, or N for Not Allowed.
def validate_row_bits_used_values(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section in data.get("sections", []):
        for row_index, row in enumerate(section.get("rows", []), start=1):
            if "Bits Used" not in row:
                continue

            bits_used = str(row["Bits Used"])
            if bits_used not in APPROVED_BIT_USED_VALUES:
                errors.append(
                    f"{section_title(section)} / row {row_index}: "
                    f"Bits Used {bits_used!r} is not one of {sorted(APPROVED_BIT_USED_VALUES)}"
                )
    return errors


# Checks row-table Bit values when a row provides one. Bit coordinates must be
# a decimal bit/index or an inclusive decimal bit range; prose belongs in the
# section text, Format, Function, or Notes.
def validate_row_bit_grammar(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section in data.get("sections", []):
        for row_index, row in enumerate(section.get("rows", []), start=1):
            if "Bit" not in row:
                continue

            bit = str(row["Bit"])
            normalized = bit.replace("\n", " ").strip()
            if not BIT_GRAMMAR_RE.fullmatch(normalized) or normalized == "N/A":
                errors.append(
                    f"{section_title(section)} / row {row_index}: "
                    f"Bit {bit!r} does not match approved row bit grammar"
                )
    return errors


# Checks row-table Bit Number values when a row provides one. Bit Number values
# must be decimal bit/index values, inclusive decimal ranges, or the EEI label
# row used to introduce the Error Enumeration Index lookup table.
def validate_row_bit_number_grammar(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section in data.get("sections", []):
        for row_index, row in enumerate(section.get("rows", []), start=1):
            if "Bit Number" not in row:
                continue

            bit_number = str(row["Bit Number"])
            normalized = bit_number.replace("\n", " ").strip()
            if normalized == "EEI":
                continue
            if not BIT_GRAMMAR_RE.fullmatch(normalized) or normalized == "N/A":
                errors.append(
                    f"{section_title(section)} / row {row_index}: "
                    f"Bit Number {bit_number!r} does not match approved bit-number grammar"
                )
    return errors


# Checks packet group names. A group name must carry a dot-prefixed source path
# before any descriptive suffix, or use cif_bit metadata when no group_name is
# present, so generated sections have a stable source identifier. The C++
# generator currently supports only CIF indexes 0 through 7.
def validate_group_names(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section, index, group in iter_groups(data):
        group_name = group.get("group_name")
        if group_name is None:
            if "cif_bit" not in group:
                section_title = section.get("title", "<unknown section>")
                errors.append(f"{section_title} / group {index}: missing group_name or cif_bit")
            continue

        group_name_text = str(group_name)
        source_path = group_name_text.split(maxsplit=1)[0]
        if not DESIGNATION_RE.fullmatch(source_path):
            section_title = section.get("title", "<unknown section>")
            errors.append(
                f"{section_title} / group {index}: "
                f"group_name {group_name!r} does not start with an approved dot-path"
            )
            continue

        if source_path.startswith(".cif") and not GENERATED_CIF_GROUP_RE.match(group_name_text):
            section_title = section.get("title", "<unknown section>")
            errors.append(
                f"{section_title} / group {index}: "
                f"group_name {group_name!r} is not a supported generated CIF group (.cif0 through .cif7)"
            )
    return errors


# Checks packet group field Bit values. Bit coordinates must be N/A, a decimal
# bit/index, or an inclusive decimal bit range; prose belongs in Function or
# Notes.
def validate_bit_grammar(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section, group, index, field in iter_group_fields(data):
        if "Bit" not in field:
            continue

        bit = str(field["Bit"])
        normalized = bit.replace("\n", " ").strip()
        if not BIT_GRAMMAR_RE.fullmatch(normalized):
            errors.append(
                f"{field_location(section, group, index, field)}: "
                f"Bit {bit!r} does not match approved bit grammar"
            )
    return errors


# Checks packet group field Range values. Ranges must be numeric/binary/hex
# tokens, inclusive token ranges, power-of-two upper-bound ranges, or comma-
# separated lists of those forms; prose belongs in Function or Notes.
def validate_range_grammar(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section, group, index, field in iter_group_fields(data):
        if "Range" not in field:
            continue

        value = str(field["Range"])
        normalized = value.replace("\n", " ").strip()
        if not RANGE_GRAMMAR_RE.fullmatch(normalized):
            errors.append(
                f"{field_location(section, group, index, field)}: "
                f"Range {value!r} does not match approved range grammar"
            )
    return errors


# Checks packet group field Default Value values when a field provides one.
# Defaults must be a decimal, binary, or hex token, or N/A for fields whose
# default is not applicable; prose belongs in Function or Notes.
def validate_default_values(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section, group, index, field in iter_group_fields(data):
        if "Default Value" not in field:
            continue

        default_value = str(field["Default Value"])
        normalized = default_value.replace("\n", " ").strip()
        if not DEFAULT_VALUE_RE.fullmatch(normalized):
            errors.append(
                f"{field_location(section, group, index, field)}: "
                f"Default Value {default_value!r} does not match approved default grammar"
            )
    return errors


# Checks packet group field Field Size (words) values when a field provides
# one. Field sizes must be decimal word counts, including -1 for dynamic
# structures handled by generated C++ mapping code.
def validate_field_size_words(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section, group, index, field in iter_group_fields(data):
        if "Field Size (words)" not in field:
            continue

        field_size = str(field["Field Size (words)"])
        normalized = field_size.replace("\n", " ").strip()
        if not FIELD_SIZE_WORDS_RE.fullmatch(normalized):
            errors.append(
                f"{field_location(section, group, index, field)}: "
                f"Field Size (words) {field_size!r} does not match approved word-count grammar"
            )
    return errors


def is_indicator_group(group: dict[str, Any]) -> bool:
    group_name = str(group.get("group_name", ""))
    return group_name.startswith(INDICATOR_GROUP_PREFIXES)


def indicator_group_prefix(group: dict[str, Any]) -> str:
    group_name = str(group.get("group_name", ""))
    return group_name.split(maxsplit=1)[0]


# Checks allowed CIF/EIF/WIF indicator fields. In VITA 49.2, a set indicator
# bit means the corresponding field or response payload is present. Every
# allowed non-structural single-bit indicator therefore needs an explicit word
# count, including 0 when a tailoring intentionally treats the bit as
# indicator-only.
def validate_indicator_field_sizes(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section, group, index, field in iter_group_fields(data):
        if not is_indicator_group(group):
            continue

        bit_used = str(field.get("Bit Used", ""))
        if bit_used not in {"B", "E"}:
            continue

        designation = str(field.get("Designation", ""))
        if designation in STRUCTURAL_INDICATOR_DESIGNATIONS:
            continue

        bit = str(field.get("Bit", ""))
        if not bit or ":" in bit:
            continue

        if "Field Size (words)" not in field:
            errors.append(
                f"{field_location(section, group, index, field)}: "
                "allowed indicator field is missing Field Size (words)"
            )
    return errors


# Checks the global generated mask naming invariant. Individual generated mask
# constants are keyed by indicator-group index and designation, not by packet
# class. Reusing the same key at different bit coordinates would make generated
# code reference the wrong mask for at least one packet.
def validate_indicator_mask_name_collisions(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    seen: dict[tuple[str, str], tuple[str, str]] = {}
    for section, group, index, field in iter_group_fields(data):
        if not is_indicator_group(group):
            continue

        bit_used = str(field.get("Bit Used", ""))
        if bit_used not in {"B", "E"}:
            continue

        designation = str(field.get("Designation", ""))
        if designation in STRUCTURAL_INDICATOR_DESIGNATIONS:
            continue

        bit = str(field.get("Bit", ""))
        if not bit or ":" in bit:
            continue

        key = (indicator_group_prefix(group), designation)
        location = field_location(section, group, index, field)
        previous = seen.get(key)
        if previous is None:
            seen[key] = (bit, location)
            continue

        previous_bit, previous_location = previous
        if previous_bit != bit:
            errors.append(
                f"{location}: generated mask key {key[0]} {key[1]} uses bit {bit}, "
                f"but {previous_location} uses bit {previous_bit}"
            )
    return errors


# Checks packet group field Designation values. Every field must have a
# dot-prefixed path made from identifier tokens so generated names and source
# references are unambiguous.
def validate_designations(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section, group, index, field in iter_group_fields(data):
        if "Designation" not in field:
            errors.append(f"{field_location(section, group, index, field)}: missing Designation")
            continue

        designation = str(field["Designation"])
        if not DESIGNATION_RE.fullmatch(designation):
            errors.append(
                f"{field_location(section, group, index, field)}: "
                f"Designation {designation!r} does not match approved path grammar"
            )
    return errors


# Checks each packet group for duplicate Designation values. A designation may
# appear in different groups, but within one group it must identify exactly one
# field so source paths and generated names are not ambiguous.
def validate_unique_group_designations(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section in data.get("sections", []):
        for group in section.get("groups", []):
            seen: dict[str, int] = {}
            for index, field in enumerate(group.get("fields", []), start=1):
                designation = field.get("Designation")
                if designation is None:
                    continue

                designation = str(designation)
                if designation in seen:
                    errors.append(
                        f"{field_location(section, group, index, field)}: "
                        f"duplicate Designation {designation!r}; first seen at field {seen[designation]}"
                    )
                else:
                    seen[designation] = index
    return errors


# Checks packet group field Bit Used values. When present, Bit Used must be one
# of the approved tailoring markers: B for Base Set, E for Extension Set, or N
# for Not Allowed.
def validate_bit_used_values(data: dict[str, Any]) -> list[str]:
    errors: list[str] = []
    for section, group, index, field in iter_group_fields(data):
        if "Bit Used" not in field:
            continue

        bit_used = str(field["Bit Used"])
        if bit_used not in APPROVED_BIT_USED_VALUES:
            errors.append(
                f"{field_location(section, group, index, field)}: "
                f"Bit Used {bit_used!r} is not one of {sorted(APPROVED_BIT_USED_VALUES)}"
            )
    return errors


def main() -> int:
    data, errors = load_yaml()
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: YAML parses successfully")

    errors = validate_group_names(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Group names match approved source-path grammar")

    errors = validate_bit_grammar(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Bit values match approved grammar")

    errors = validate_range_grammar(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Range values match approved grammar")

    errors = validate_default_values(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Default Value values match approved grammar")

    errors = validate_field_size_words(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Field Size (words) values match approved grammar")

    errors = validate_indicator_field_sizes(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Allowed indicator fields declare Field Size (words)")

    errors = validate_indicator_mask_name_collisions(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Generated indicator mask names are unambiguous")

    errors = validate_designations(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Designations match approved path grammar")

    errors = validate_unique_group_designations(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Group designations are unique")

    errors = validate_bit_used_values(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Bit Used values are approved")

    errors = validate_row_section_headers(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Row section headers match row keys")

    errors = validate_row_designations(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Row designations match approved path grammar")

    errors = validate_row_bits_used_values(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Row Bits Used values are approved")

    errors = validate_row_bit_grammar(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Row Bit values match approved grammar")

    errors = validate_row_bit_number_grammar(data)
    if errors:
        for error in errors:
            print(error, file=sys.stderr)
        return 1
    print(f"{YAML_PATH}: Row Bit Number values match approved grammar")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

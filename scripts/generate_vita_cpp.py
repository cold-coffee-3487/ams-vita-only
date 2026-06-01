from pathlib import Path
import re
import sys

import yaml

YAML_PATH = Path('spec/ams_vita_49-2_tailoring.yaml')
OUTPUT_DIR = Path('include/ams/iface/vita')
CIF_GROUP_RE = re.compile(r'^\.cif([0-7])(?:\s|$)')
UNSUPPORTED_CIF_GROUP_RE = re.compile(r'^\.cif')

with YAML_PATH.open() as f:
    data = yaml.safe_load(f)

def clean_name(designation):
    n = designation.strip('.')
    return n[0].upper() + n[1:]


def parse_bit_mask(bit_str):
    if not bit_str or bit_str == 'N/A':
        return None, None
    if ':' in bit_str:
        high_str, low_str = bit_str.split(':', 1)
        high = int(high_str)
        low = int(low_str)
        first = min(high, low)
        last = max(high, low)
        mask = 0
        for bit in range(first, last + 1):
            mask |= (1 << bit)
        return None, mask

    bit_num = int(bit_str)
    return bit_num, (1 << bit_num)


def is_fixed_zero_range(value):
    return str(value or '').replace('\n', ' ').strip().strip("'\"") == '0'


def is_unsupported_fixed_zero_extension(field):
    return field.get('Bit Used') == 'E' and is_fixed_zero_range(field.get('Range'))


packets = {}

for section in data.get('sections', []):
    title = section['title']
    if 'Packet' not in title:
        continue
    
    clean_title = title.replace('(', '').replace(')', '').replace(' ', '')
        
    cifs = {}
    for group in section.get('groups', []):
        name = group['group_name']
        cif_match = CIF_GROUP_RE.match(name)
        if cif_match:
            cif_idx = int(cif_match.group(1))
            cifs[cif_idx] = cifs.get(cif_idx, [])
            
            for field in group.get('fields', []):
                designation = field.get('Designation', '')
                if not designation: continue
                raw_name = designation.strip('.')
                
                bit_used = field.get('Bit Used')
                if bit_used not in ['B', 'E', 'N']:
                    continue
                
                bit_str = str(field.get('Bit'))
                bit_num, bit_mask = parse_bit_mask(bit_str)
                if bit_mask is None: continue
                if bit_num is None and bit_used != 'N': continue
                
                # CFCI and CIF enables are structural header bits for CIFs, not payload mappings.
                is_structural = raw_name in ['cfci', 'cif1enable', 'cif2enable', 'cif3enable', 'cif4enable', 'cif7enable']
                
                size_str = str(field.get('Field Size (words)', '0'))
                try:
                    size = int(size_str)
                except ValueError:
                    size = 0
                
                is_dynamic_struct = (size == -1)
                
                cifs[cif_idx].append({
                    'name': clean_name(designation),
                    'raw_name': raw_name,
                    'bit': bit_num,
                    'bit_mask': bit_mask,
                    'size': size,
                    'is_dynamic_struct': is_dynamic_struct,
                    'bit_used': bit_used,
                    'is_structural': is_structural,
                    'is_fixed_zero_extension': is_unsupported_fixed_zero_extension(field)
                })
        elif UNSUPPORTED_CIF_GROUP_RE.match(name):
            print(f"Unsupported generated CIF group {name!r} in section {title!r}; expected .cif0 through .cif7", file=sys.stderr)
            raise SystemExit(1)
    
    disabled_cif_indexes = set()
    for field in cifs.get(0, []):
        match = re.fullmatch(r'cif([1-7])enable', field['raw_name'])
        if match and (field['bit_used'] == 'N' or field['is_fixed_zero_extension']):
            disabled_cif_indexes.add(int(match.group(1)))

    for disabled_idx in disabled_cif_indexes:
        if disabled_idx in cifs:
            cifs[disabled_idx] = [{
                'name': 'Disabled',
                'raw_name': f'cif{disabled_idx}',
                'bit': None,
                'bit_mask': 0xFFFFFFFF,
                'size': 0,
                'is_dynamic_struct': False,
                'bit_used': 'N',
                'is_structural': False,
                'is_fixed_zero_extension': False
            }]

    for k in cifs:
        cifs[k].sort(key=lambda x: x['bit'] if x['bit'] is not None else -1, reverse=True)
        
    packets[clean_title] = cifs

def write_generated_masks(packets):
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    with (OUTPUT_DIR / 'GeneratedMasks.h').open('w') as f:
        f.write("#pragma once\n\n#include <cstdint>\n\nnamespace ams::iface::vita::masks {\n")
        f.write("    // CIF Enables\n")
        f.write("    constexpr uint32_t CIF0_CIF7_ENABLE = 1U << 7;\n")
        f.write("    constexpr uint32_t CIF0_CIF4_ENABLE = 1U << 4;\n")
        f.write("    constexpr uint32_t CIF0_CIF3_ENABLE = 1U << 3;\n")
        f.write("    constexpr uint32_t CIF0_CIF2_ENABLE = 1U << 2;\n")
        f.write("    constexpr uint32_t CIF0_CIF1_ENABLE = 1U << 1;\n\n")
        
        written = set()
        for pkt, cifs in packets.items():
            for cif_idx, fields in cifs.items():
                if not fields: continue
                b_mask = 0
                n_mask = 0
                for field in fields:
                    if field['bit_used'] == 'B':
                        b_mask |= field['bit_mask']
                    elif field['bit_used'] == 'N' or field['is_fixed_zero_extension']:
                        n_mask |= field['bit_mask']
                        
                    if field['bit_used'] == 'N' or field['is_fixed_zero_extension'] or field['is_structural']:
                        continue
                    
                    mask_name = f"CIF{cif_idx}_{field['name'].upper()}_MASK"
                    if mask_name not in written:
                        f.write(f"    constexpr uint32_t {mask_name:<30} = 1U << {field['bit']};\n")
                        written.add(mask_name)
                        
                f.write(f"    constexpr uint32_t {pkt.upper()}_CIF{cif_idx}_B_MASK = 0x{b_mask:08X};\n")
                f.write(f"    constexpr uint32_t {pkt.upper()}_CIF{cif_idx}_N_MASK = 0x{n_mask:08X};\n")
        f.write("} // namespace ams::iface::vita::masks\n")

def write_base_class(packet_name, cifs):
    base_name = f"{packet_name}Base"
    header = OUTPUT_DIR / f"{base_name}.h"
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    with header.open('w') as f:
        f.write(f"#pragma once\n\n#include <cstdint>\n#include <optional>\n#include <stdexcept>\n#include <arpa/inet.h>\n#include <span>\n#include <array>\n")
        f.write("#include \"ams/iface/vita/GeneratedMasks.h\"\n")
        f.write("#include \"ams/iface/vita/Primitives.h\"\n\n")
        f.write("namespace ams::iface::vita {\n\n")
        
        # Base View
        f.write(f"class {base_name}View {{\nprotected:\n")
        cif_decls = ", ".join([f"cif{c} = 0" for c, fields in cifs.items() if fields])
        if not cif_decls:
            cif_decls = "cif0 = 0"
        f.write(f"    uint32_t {cif_decls};\n")
        for cif_idx, fields in cifs.items():
            for field in fields:
                if field['bit_used'] == 'N' or field['is_fixed_zero_extension'] or field['is_structural'] or field['size'] == 0: continue
                if field['is_dynamic_struct']:
                    f.write(f"    const uint32_t* ptr_{field['name']} = nullptr;\n")
                    f.write(f"    size_t size_{field['name']} = 0;\n")
                elif field['size'] == 2:
                    f.write(f"    const uint32_t* ptr_{field['name']}_hi = nullptr;\n")
                    f.write(f"    const uint32_t* ptr_{field['name']}_lo = nullptr;\n")
                elif field['size'] > 2:
                    f.write(f"    const uint32_t* ptr_{field['name']} = nullptr;\n")
                    f.write(f"    size_t size_{field['name']} = {field['size']};\n")
                else:
                    f.write(f"    const uint32_t* ptr_{field['name']} = nullptr;\n")
        
        f.write("    const uint32_t* mapGeneratedFields(const uint32_t* current, [[maybe_unused]] const uint32_t* end) {\n")
        for cif_idx, fields in cifs.items():
            if not fields: continue
            f.write(f"        // CIF{cif_idx} Mapping\n")
            f.write(f"        if ((cif{cif_idx} & masks::{packet_name.upper()}_CIF{cif_idx}_N_MASK) != 0) return nullptr;\n")
            
            has_b_fields = any(field['bit_used'] == 'B' for field in fields)
            if packet_name == 'ControlScheduleRequestPacket' and cif_idx == 1:
                f.write("        constexpr uint32_t required_without_pointing3d = masks::CONTROLSCHEDULEREQUESTPACKET_CIF1_B_MASK & ~masks::CIF1_POINTING3D_MASK;\n")
                f.write("        if ((cif1 & required_without_pointing3d) != required_without_pointing3d) return nullptr;\n")
                f.write("        const bool has_pointing3d = (cif1 & masks::CIF1_POINTING3D_MASK) != 0;\n")
                f.write("        const bool has_pointing3d_struct = (cif1 & masks::CIF1_POINTING3DSTRUCT_MASK) != 0;\n")
                f.write("        if (has_pointing3d == has_pointing3d_struct) return nullptr;\n")
            elif has_b_fields:
                f.write(f"        if ((cif{cif_idx} & masks::{packet_name.upper()}_CIF{cif_idx}_B_MASK) != masks::{packet_name.upper()}_CIF{cif_idx}_B_MASK) return nullptr;\n")
            
            for field in fields:
                if field['bit_used'] == 'N' or field['is_fixed_zero_extension'] or field['is_structural'] or field['size'] == 0: continue
                mask_name = f"masks::CIF{cif_idx}_{field['name'].upper()}_MASK"
                if field['is_dynamic_struct']:
                    f.write(f"        if (cif{cif_idx} & {mask_name}) {{\n")
                    f.write(f"            if (current >= end) return nullptr;\n")
                    f.write(f"            uint32_t total_words = ntohl(*current);\n")
                    f.write(f"            size_t remaining = static_cast<size_t>(std::distance(current, end));\n")
                    f.write(f"            if (total_words == 0 || remaining < total_words) return nullptr;\n")
                    if field['name'] == 'DataAddressStructure':
                        f.write(f"            if (!DataAddressStructureView(std::span<const uint32_t>(current, total_words)).isValid()) return nullptr;\n")
                    elif field['name'] == 'Pointing3dStruct':
                        f.write(f"            if (!Pointing3dStructureView(std::span<const uint32_t>(current, total_words)).isValid()) return nullptr;\n")

                    f.write(f"            ptr_{field['name']} = current;\n")
                    f.write(f"            size_{field['name']} = total_words;\n")
                    f.write(f"            current += total_words;\n")
                    f.write(f"        }}\n")
                    continue

                if field['size'] == 2:
                    if field['name'] == 'DataFormat':
                        f.write(f"        if (cif{cif_idx} & {mask_name}) {{\n")
                        f.write(f"            if (end - current < 2) return nullptr;\n")
                        f.write(f"            ptr_{field['name']}_hi = current++; ptr_{field['name']}_lo = current++;\n")
                        f.write(f"            const uint64_t data_format = (static_cast<uint64_t>(ntohl(*ptr_{field['name']}_hi)) << 32) | ntohl(*ptr_{field['name']}_lo);\n")
                        f.write(f"            if (!isAmsDataFormat(data_format)) return nullptr;\n")
                        f.write(f"        }}\n")
                    else:
                        f.write(f"        if (cif{cif_idx} & {mask_name}) {{ if (end - current < 2) return nullptr; ptr_{field['name']}_hi = current++; ptr_{field['name']}_lo = current++; }}\n")
                elif field['size'] > 2:
                    f.write(f"        if (cif{cif_idx} & {mask_name}) {{ if (end - current < {field['size']}) return nullptr; ptr_{field['name']} = current; current += {field['size']}; }}\n")
                else:
                    f.write(f"        if (cif{cif_idx} & {mask_name}) {{ if (current >= end) return nullptr; ptr_{field['name']} = current++; }}\n")
        f.write("        return current;\n    }\n\npublic:\n")
        
        for cif_idx, fields in cifs.items():
            for field in fields:
                if field['bit_used'] == 'N' or field['is_fixed_zero_extension'] or field['is_structural'] or field['size'] == 0: continue
                
                if field['is_dynamic_struct']:
                    f.write(f"    inline std::span<const uint32_t> get{field['name']}Raw() const {{\n")
                    f.write(f"        if (!ptr_{field['name']}) return {{}};\n")
                    f.write(f"        return {{ptr_{field['name']}, size_{field['name']}}};\n")
                    f.write("    }\n")
                    continue
                    
                ret_type = "uint64_t" if field['size'] == 2 else "uint32_t"
                if field['size'] > 2:
                    f.write(f"    inline std::optional<std::array<uint32_t, {field['size']}>> get{field['name']}() const {{\n")
                    f.write(f"        if (!ptr_{field['name']}) return std::nullopt;\n")
                    f.write(f"        std::array<uint32_t, {field['size']}> arr;\n")
                    f.write(f"        for (size_t i = 0; i < {field['size']}; ++i) arr[i] = ntohl(ptr_{field['name']}[i]);\n")
                    f.write(f"        return arr;\n")
                    f.write("    }\n")
                else:
                    if field['name'] == 'DataFormat':
                        f.write(f"    inline std::optional<DataFormat> get{field['name']}() const {{\n")
                        f.write(f"        if (!ptr_{field['name']}_hi || !ptr_{field['name']}_lo) return std::nullopt;\n")
                        f.write(f"        return parseAmsDataFormat((static_cast<uint64_t>(ntohl(*ptr_{field['name']}_hi)) << 32) | ntohl(*ptr_{field['name']}_lo));\n")
                        f.write("    }\n")
                    else:
                        f.write(f"    inline std::optional<{ret_type}> get{field['name']}() const {{\n")
                        if field['size'] == 2:
                            f.write(f"        if (!ptr_{field['name']}_hi || !ptr_{field['name']}_lo) return std::nullopt;\n")
                            f.write(f"        return (static_cast<uint64_t>(ntohl(*ptr_{field['name']}_hi)) << 32) | ntohl(*ptr_{field['name']}_lo);\n")
                        else:
                            f.write(f"        if (!ptr_{field['name']}) return std::nullopt;\n")
                            f.write(f"        return ntohl(*ptr_{field['name']});\n")
                        f.write("    }\n")
        
        f.write("};\n\n")
        
        # Base Builder
        f.write(f"class {base_name}Builder {{\nprotected:\n")
        for cif_idx, fields in cifs.items():
            if not fields: continue
            f.write(f"    uint32_t cif{cif_idx} = masks::{packet_name.upper()}_CIF{cif_idx}_B_MASK;\n")
        
        for cif_idx, fields in cifs.items():
            for field in fields:
                if field['bit_used'] == 'N' or field['is_fixed_zero_extension'] or field['is_structural'] or field['size'] == 0: continue
                if field['is_dynamic_struct']:
                    f.write(f"    std::span<const uint32_t> p_{field['name']};\n")
                    continue
                if field['size'] == 2:
                    f.write(f"    uint32_t p_{field['name']}_hi = 0, p_{field['name']}_lo = 0;\n")
                    if field['name'] == 'DataFormat':
                        f.write(f"    bool p_{field['name']}_set = false;\n")
                elif field['size'] > 2:
                    f.write(f"    std::array<uint32_t, {field['size']}> p_{field['name']} = {{0}};\n")
                else:
                    f.write(f"    uint32_t p_{field['name']} = 0;\n")
        
        f.write("    size_t writeGeneratedFields([[maybe_unused]] uint32_t* buffer, size_t offset, [[maybe_unused]] size_t max_words) const {\n")
        for cif_idx, fields in cifs.items():
            if not fields: continue
            f.write(f"        // CIF{cif_idx} Mapping\n")
            for field in fields:
                if field['bit_used'] == 'N' or field['is_fixed_zero_extension'] or field['is_structural'] or field['size'] == 0: continue
                
                mask_name = f"masks::CIF{cif_idx}_{field['name'].upper()}_MASK"
                if field['is_dynamic_struct']:
                    f.write(f"        if (cif{cif_idx} & {mask_name}) {{\n")
                    f.write(f"            if (offset > max_words || p_{field['name']}.size() > max_words - offset) return 0;\n")
                    f.write(f"            for (uint32_t word : p_{field['name']}) buffer[offset++] = word;\n")
                    f.write(f"        }}\n")
                    continue
                
                if field['size'] == 2:
                    if field['name'] == 'DataFormat':
                        f.write(f"        if (cif{cif_idx} & {mask_name}) {{ if (!p_{field['name']}_set || offset + 2 > max_words) return 0; buffer[offset++] = p_{field['name']}_hi; buffer[offset++] = p_{field['name']}_lo; }}\n")
                    else:
                        f.write(f"        if (cif{cif_idx} & {mask_name}) {{ if (offset + 2 > max_words) return 0; buffer[offset++] = p_{field['name']}_hi; buffer[offset++] = p_{field['name']}_lo; }}\n")
                elif field['size'] > 2:
                    f.write(f"        if (cif{cif_idx} & {mask_name}) {{\n")
                    f.write(f"            if (offset + {field['size']} > max_words) return 0;\n")
                    f.write(f"            for (size_t i = 0; i < {field['size']}; ++i) buffer[offset++] = p_{field['name']}[i];\n")
                    f.write(f"        }}\n")
                else:
                    f.write(f"        if (cif{cif_idx} & {mask_name}) {{ if (offset + 1 > max_words) return 0; buffer[offset++] = p_{field['name']}; }}\n")
        f.write("        return offset;\n    }\n")

        for cif_idx, fields in cifs.items():
            for field in fields:
                if field['bit_used'] == 'N' or field['is_fixed_zero_extension'] or field['is_structural'] or not field['is_dynamic_struct']:
                    continue
                mask_name = f"masks::CIF{cif_idx}_{field['name'].upper()}_MASK"
                f.write(f"    void add{field['name']}Raw(std::span<const uint32_t> val) {{\n")
                f.write("        if (val.empty()) {\n")
                f.write(f"            throw std::invalid_argument(\"{field['name']} dynamic span cannot be empty\");\n")
                f.write("        }\n")
                if field['name'] == 'DataAddressStructure':
                    f.write("        if (!DataAddressStructureView(val).isValid()) throw std::invalid_argument(\"DataAddressStructure dynamic span is invalid\");\n")
                elif field['name'] == 'Pointing3dStruct':
                    f.write("        if (!Pointing3dStructureView(val).isValid()) throw std::invalid_argument(\"Pointing3dStruct dynamic span is invalid\");\n")

                if packet_name == 'ControlScheduleRequestPacket' and field['name'] == 'Pointing3dStruct':
                    f.write("        cif1 &= ~masks::CIF1_POINTING3D_MASK;\n")
                f.write(f"        cif{cif_idx} |= {mask_name};\n")
                f.write(f"        p_{field['name']} = val;\n")
                f.write("    }\n")

        for cif_idx, fields in cifs.items():
            for field in fields:
                if field['bit_used'] == 'N' or field['is_fixed_zero_extension'] or field['is_structural'] or field['name'] != 'DataFormat':
                    continue
                mask_name = f"masks::CIF{cif_idx}_{field['name'].upper()}_MASK"
                f.write(f"    void set{field['name']}Raw(DataFormat val) {{\n")
                f.write(f"        cif{cif_idx} |= {mask_name};\n")
                f.write("        const uint64_t raw = static_cast<uint64_t>(val);\n")
                f.write(f"        p_{field['name']}_hi = htonl(static_cast<uint32_t>(raw >> 32));\n")
                f.write(f"        p_{field['name']}_lo = htonl(static_cast<uint32_t>(raw & 0xFFFFFFFF));\n")
                f.write(f"        p_{field['name']}_set = true;\n")
                f.write("    }\n")

        f.write("\npublic:\n")
        
        for cif_idx, fields in cifs.items():
            for field in fields:
                if field['bit_used'] == 'N' or field['is_fixed_zero_extension'] or field['is_structural'] or field['name'] == 'DataFormat': continue
                mask_name = f"masks::CIF{cif_idx}_{field['name'].upper()}_MASK"
                if field['size'] == 0:
                    f.write(f"    void add{field['name']}() {{\n")
                    f.write(f"        cif{cif_idx} |= {mask_name};\n")
                    f.write("    }\n")
                elif field['is_dynamic_struct']:
                    continue
                elif field['size'] > 2:
                    f.write(f"    void add{field['name']}(std::span<const uint32_t> val) {{\n")
                    f.write(f"        if (val.size() != {field['size']}) {{\n")
                    f.write(f"            throw std::invalid_argument(\"{field['name']} must be exactly {field['size']} words\");\n")
                    f.write("        }\n")
                    f.write(f"        cif{cif_idx} |= {mask_name};\n")
                    f.write(f"        for (size_t i = 0; i < {field['size']}; ++i) p_{field['name']}[i] = htonl(val[i]);\n")
                    f.write("    }\n")
                else:
                    arg_type = "uint64_t" if field['size'] == 2 else "uint32_t"
                    f.write(f"    void add{field['name']}({arg_type} val) {{\n")
                    if packet_name == 'ControlScheduleRequestPacket' and field['name'] == 'Pointing3d':
                        f.write("        cif1 &= ~masks::CIF1_POINTING3DSTRUCT_MASK;\n")
                        f.write("        p_Pointing3dStruct = {};\n")
                    f.write(f"        cif{cif_idx} |= {mask_name};\n")
                    if field['size'] == 2:
                        f.write(f"        p_{field['name']}_hi = htonl(static_cast<uint32_t>(val >> 32));\n")
                        f.write(f"        p_{field['name']}_lo = htonl(static_cast<uint32_t>(val & 0xFFFFFFFF));\n")
                    else:
                        f.write(f"        p_{field['name']} = htonl(val);\n")
                    f.write("    }\n")
        
        f.write("};\n\n")
        f.write("} // namespace ams::iface::vita\n")

write_generated_masks(packets)
for k, v in packets.items():
    write_base_class(k, v)

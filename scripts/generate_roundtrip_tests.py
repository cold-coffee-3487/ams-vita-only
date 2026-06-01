import yaml
import struct
import random

def clean_name(designation):
    n = designation.strip('.')
    return n[0].upper() + n[1:]

AMS_DATA_FORMATS = [
    ('Complex16BitSigned', 0x200003CF, 0x00000000),
    ('Complex8BitSigned', 0xA00001C7, 0x00000000),
]


def data_format_enum_for_words(hi, lo):
    for enum_name, fmt_hi, fmt_lo in AMS_DATA_FORMATS:
        if hi == fmt_hi and lo == fmt_lo:
            return enum_name
    raise RuntimeError(f"unsupported AMS data format words: {hi:08x} {lo:08x}")


def is_fixed_zero_range(value):
    return str(value or '').replace('\n', ' ').strip().strip("'\"") == '0'

ACKX_ERROR_SUBJECT_BY_EIF_BIT = {
    (0, 31): 'NoCorrespondingPayloadField',
    (0, 29): 'Bandwidth',
    (0, 27): 'RfRefFreq',
    (0, 23): 'Gain',
    (0, 21): 'SampleRate',
    (0, 15): 'DataFormat',
    (1, 30): 'Polarization',
    (1, 29): 'Pointing3d',
    (1, 25): 'BeamWidth',
    (2, 6): 'FuncPriorityId',
    (3, 21): 'Dwell',
    (4, 29): 'RfFigureOfMerit',
    (4, 24): 'AddressGroupIndex',
    (4, 21): 'TxDigitalInputPower',
}

def ackx_error_subject_for_payload(payload_word):
    eif_idx = (payload_word >> 15) & 0x7
    eif_bit = (payload_word >> 10) & 0x1F
    return ACKX_ERROR_SUBJECT_BY_EIF_BIT[(eif_idx, eif_bit)]

class BitBuilder:
    def __init__(self):
        self.words = []
    def add_word(self, val):
        self.words.append(val & 0xFFFFFFFF)
    def to_hex(self):
        return b''.join(struct.pack('>I', w) for w in self.words).hex()

def generate():
    random.seed(42)
    
    with open('spec/ams_vita_49-2_tailoring.yaml') as f:
        data = yaml.safe_load(f)

    packets = {}
    for section in data.get('sections', []):
        title = section['title']
        if 'Packet' not in title:
            continue
        clean_title = title.replace('(', '').replace(')', '').replace(' ', '')
            
        cifs = {}
        for group in section.get('groups', []):
            name = group['group_name']
            if name.startswith('.cif') or name.startswith('.eif'):
                cif_idx = int(name[4])
                cifs[cif_idx] = cifs.get(cif_idx, [])
                for field in group.get('fields', []):
                    designation = field.get('Designation', '')
                    if not designation: continue
                    raw_name = designation.strip('.')
                    bit_used = field.get('Bit Used')
                    if bit_used not in ['B', 'E']: continue
                    if bit_used == 'E' and is_fixed_zero_range(field.get('Range')): continue
                    bit_str = str(field.get('Bit'))
                    if not bit_str or ':' in bit_str: continue
                    bit_num = int(bit_str)
                    
                    if raw_name in ['cfci', 'cif1enable', 'cif2enable', 'cif3enable', 'cif4enable']: continue 
                    
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
                        'size': size,
                        'is_dynamic_struct': is_dynamic_struct,
                        'bit_used': bit_used
                    })
        
        disabled_cif_indexes = set()
        for group in section.get('groups', []):
            if group.get('group_name', '').startswith('.cif0'):
                for field in group.get('fields', []):
                    designation = field.get('Designation', '').strip('.')
                    if designation.startswith('cif') and designation.endswith('enable'):
                        try:
                            cif_idx = int(designation[3])
                        except (ValueError, IndexError):
                            continue
                        bit_used = field.get('Bit Used')
                        if bit_used == 'N' or (bit_used == 'E' and is_fixed_zero_range(field.get('Range'))):
                            disabled_cif_indexes.add(cif_idx)
        for disabled_idx in disabled_cif_indexes:
            cifs.pop(disabled_idx, None)

        for k in cifs:
            cifs[k].sort(key=lambda x: x['bit'], reverse=True)
            
        if cifs:
            packets[clean_title] = cifs
            
    pkt_map = {
        'ControlScheduleRequestPacket': ('ControlScheduleRequestView', 'ControlScheduleRequestBuilder', 'ControlScheduleRequestPacket.h'),
        'ExtensionDataContextPacket': ('ExtensionDataContextView', 'ExtensionDataContextBuilder', 'ExtensionDataContextPacket.h'),
        'ScheduleAckAckRPacket': ('AckRView', 'AckRBuilder', 'AckRPacket.h'),
        'ExecutionAckAckXPacket': ('AckXView', 'AckXBuilder', 'AckRPacket.h'),
    }
            
    cpp = """// AUTO-GENERATED BY generate_roundtrip_tests.py
#include "ams/iface/vita/ControlScheduleRequestPacket.h"
#include "ams/iface/vita/ExtensionDataContextPacket.h"
#include "ams/iface/vita/DataPacket.h"
#include "ams/iface/vita/AckRPacket.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <initializer_list>

using namespace ams::iface::vita;

auto hex_to_words(const std::string& hex) -> std::vector<uint32_t> {
    std::vector<uint32_t> words;
    for (size_t i = 0; i < hex.length(); i += 8) {
        uint32_t word = static_cast<uint32_t>(std::stoul(hex.substr(i, 8), nullptr, 16));
        words.push_back(htonl(word));
    }
    return words;
}

auto host_to_network_words(std::initializer_list<uint32_t> words) -> std::vector<uint32_t> {
    std::vector<uint32_t> network_words(words);
    std::transform(network_words.begin(), network_words.end(), network_words.begin(),
                   [](uint32_t word) { return htonl(word); });
    return network_words;
}
"""

    for pkt_name, cifs in packets.items():
        if pkt_name not in pkt_map: continue
        
        view_name, bldr_name, header_file = pkt_map[pkt_name]
        
        cpp += f"void test_{pkt_name}() {{\n"
        cpp += f"    std::cout << \"Testing {pkt_name}...\\n\";\n"
        
        for i in range(5): # 5 test cases per packet
            bb = BitBuilder()
            
            # Prologue
            if pkt_name == "ControlScheduleRequestPacket" or pkt_name == "ScheduleAckAckRPacket" or pkt_name == "ExecutionAckAckXPacket":
                prologue_words = 9
                word1_base = (7 << 28) | (1 << 27) | (3 << 22) | (2 << 20)
                if pkt_name == "ScheduleAckAckRPacket" or pkt_name == "ExecutionAckAckXPacket":
                    word1_base |= (1 << 26) # isAck = 1
            elif pkt_name == "ExtensionDataContextPacket":
                prologue_words = 7
                word1_base = (5 << 28) | (1 << 27) | (3 << 22) | (2 << 20)
            else:
                prologue_words = 7
                word1_base = (1 << 28) | (1 << 27) | (3 << 22) | (2 << 20)
                
            stream_id = random.randint(0, 0xFFFFFFFF)
            oui = 0xAAAAAA
            if pkt_name == "ControlScheduleRequestPacket":
                codes = 0x04000A05
            elif pkt_name == "ScheduleAckAckRPacket":
                codes = 0x04000B05
            elif pkt_name == "ExecutionAckAckXPacket":
                codes = 0x04000C05
            elif pkt_name == "ExtensionDataContextPacket":
                codes = 0x05000E05
            else:
                codes = 0x04000D05
            tsi = random.randint(0, 0xFFFFFFFF)
            tsf_hi = random.randint(0, 0xFFFFFFFF)
            tsf_lo = random.randint(0, 0xFFFFFFFF)
            if pkt_name == "ControlScheduleRequestPacket":
                schedule_request_type = random.randint(0, 0xF)
                cam = 0x01098000 | (schedule_request_type << 4) # action=execute, ReqX, ReqEr, ReqR, SchReqType=0..15
            elif pkt_name == "ScheduleAckAckRPacket":
                cam = 0x01008410 # action=execute, AckR, SchX, SchReqType=1
            elif pkt_name == "ExecutionAckAckXPacket":
                schedule_request_type = random.randint(0, 0xF)
                schx_bit = 0x00000400 if random.choice([True, False]) else 0
                cam = 0x01080000 | schx_bit | (schedule_request_type << 4) # action=execute, AckX, SchX dynamic, SchReqType=0..15
            else:
                cam = 0
            msg_id = random.randint(0, 0xFFFFFFFF)
            
            bb.add_word(word1_base) # Will update size later
            bb.add_word(stream_id)
            bb.add_word(oui)
            bb.add_word(codes)
            bb.add_word(tsi)
            bb.add_word(tsf_hi)
            bb.add_word(tsf_lo)
            if prologue_words == 9:
                bb.add_word(cam)
                bb.add_word(msg_id)
                
            if pkt_name == "ScheduleAckAckRPacket":
                cif_vals = {0: (1<<4)|(1<<2), 1:0, 2:0, 3:0, 4:0}
            elif pkt_name == "ExecutionAckAckXPacket":
                cif_vals = {0: 0, 1:0, 2:0, 3:0, 4:0}
                # AckX EIFs contain 1-word flat payloads for every set bit.
                # Override the sizing logic during roundtrip generation since
                # the YAML may define them as structs or 2-word values but AckX errors
                # only include the 1-word indicator payload for each set bit
                for cif_idx in range(5):
                    if cif_idx not in cifs: continue
                    for i, field in enumerate(cifs[cif_idx]):
                        cifs[cif_idx][i]['size'] = 1
                        cifs[cif_idx][i]['is_dynamic_struct'] = False
                        
                        # EIF0 Bit 0 and Bits 4:1 (Enables) and Bits 31:5 (Payloads)
                        # Bits 4:1 are reserved for Enables. Do not randomly generate payloads for them in EIF0.
                        # For all other EIFs, Bit 0 is reserved.
                        if cif_idx == 0:
                            if cifs[cif_idx][i]['bit'] < 5:
                                cifs[cif_idx][i]['size'] = 0
                            else:
                                cifs[cif_idx][i]['size'] = 1
                        else:
                            if cifs[cif_idx][i]['bit'] == 0:
                                cifs[cif_idx][i]['size'] = 0
                            else:
                                cifs[cif_idx][i]['size'] = 1
            else:
                cif_vals = {0: (1<<31)|(1<<4)|(1<<3)|(1<<2)|(1<<1), 1:0, 2:0, 3:0, 4:0}
            
            payloads = []
            
            active_fields = []
            
            for cif_idx in range(5):
                if cif_idx not in cifs: continue
                for field in cifs[cif_idx]:
                    is_b = field.get('bit_used', '') == 'B'
                    is_n = field.get('bit_used', '') == 'N'
                    is_e = field.get('bit_used', '') == 'E'
                    
                    if is_n:
                        continue
                    
                    # Force include B fields, randomly include E fields
                    include_field = is_b or (is_e and random.choice([True, False]))
                    
                    if include_field:
                        if field['name'] == 'Pointing3dStruct' and any(x['field']['name'] == 'Pointing3d' for x in active_fields): continue
                        if field['name'] == 'Pointing3d' and any(x['field']['name'] == 'Pointing3dStruct' for x in active_fields): continue
                        if field['name'] == 'DataAddressStructure' and any(x['field']['name'] == 'DataAddressIndex' for x in active_fields): continue
                        if field['name'] == 'DataAddressIndex' and any(x['field']['name'] == 'DataAddressStructure' for x in active_fields): continue
                        
                        if pkt_name == "ExecutionAckAckXPacket" and field['size'] == 0:
                            continue
                            
                        cif_vals[cif_idx] |= (1 << field['bit'])
                        
                        field_data = []
                        if field['size'] == 0:
                            pass
                        elif field['size'] == 1:
                            if pkt_name == "ExecutionAckAckXPacket":
                                err_enum = random.randint(0, 0x3FF)
                                payload_word = (1 << 31) | (cif_idx << 15) | (field['bit'] << 10) | err_enum
                                field_data.append(payload_word)
                            else:
                                field_data.append(random.randint(0, 0xFFFFFFFF))
                        elif field['size'] == 2:
                            if field['name'] == 'DataFormat':
                                _, fmt_hi, fmt_lo = random.choice(AMS_DATA_FORMATS)
                                field_data.append(fmt_hi)
                                field_data.append(fmt_lo)
                            else:
                                field_data.append(random.randint(0, 0xFFFFFFFF))
                                field_data.append(random.randint(0, 0xFFFFFFFF))
                        elif field['size'] > 2:
                            for _ in range(field['size']):
                                field_data.append(random.randint(0, 0xFFFFFFFF))
                        elif field['is_dynamic_struct']:
                            if field['name'] == 'DataAddressStructure':
                                sid = random.randint(0, 0xFFFFFFFF)
                                addr = random.randint(0, 0xFFFFFFFF)
                                field_data.extend([5, (2 << 12) | 1, 0xC0000000, sid, addr])
                            elif field['name'] == 'Pointing3dStruct':
                                pointing = random.randint(0, 0xFFFFFFFF)
                                field_data.extend([4, (3 << 24) | (1 << 12) | 1, 0x40000000, pointing])
                            else:
                                raise RuntimeError(f"Unsupported dynamic structure {field['name']}")
                            
                        payloads.extend(field_data)
                        active_fields.append({'field': field, 'cif_idx': cif_idx, 'data': field_data})
                        
            if pkt_name == "ScheduleAckAckRPacket":
                if cif_vals[1] != 0: cif_vals[0] |= (1<<1)
                if cif_vals[3] != 0: cif_vals[0] |= (1<<3)
            elif pkt_name == "ExecutionAckAckXPacket":
                if cif_vals[1] != 0: cif_vals[0] |= (1<<1)
                if cif_vals[2] != 0: cif_vals[0] |= (1<<2)
                if cif_vals[3] != 0: cif_vals[0] |= (1<<3)
                if cif_vals[4] != 0: cif_vals[0] |= (1<<4)
                if payloads:
                    cam |= 0x00010000 # CAM_ACKER_MASK; SchX remains independent of error reporting
                else:
                    cam &= ~0x00010000 # clear CAM_ACKER_MASK if no payloads
                    cif_vals[0] = 0
                    
            if pkt_name == "ExecutionAckAckXPacket":
                bb.words[7] = cam
                
            if pkt_name == "ExecutionAckAckXPacket" and payloads:
                bb.add_word(cif_vals[0])
                cif_count = 1
                for j in range(1, 5):
                    if cif_vals[0] & (1 << j):
                        bb.add_word(cif_vals[j])
                        cif_count += 1
            elif pkt_name != "ExecutionAckAckXPacket":
                bb.add_word(cif_vals[0])
                cif_count = 1
                for j in range(1, 5):
                    if cif_vals[0] & (1 << j):
                        bb.add_word(cif_vals[j])
                        cif_count += 1
            else:
                cif_count = 0
                
            for p in payloads:
                bb.add_word(p)
                
            pkt_size = prologue_words + cif_count + len(payloads)
            
            # Re-apply AckX isAck forced 1 bit because words[0] |= might lose context if originally zeroed properly
            if pkt_name == "ScheduleAckAckRPacket" or pkt_name == "ExecutionAckAckXPacket":
                bb.words[0] |= (1 << 26)
            
            # Recalculate the 16-bit packet size field for generated roundtrip packets.
            word1_final = (bb.words[0] & ~0xFFFF) | (pkt_size & 0xFFFF)
            bb.words[0] = word1_final
            
            hex_str = bb.to_hex()
            
            cpp += f"    {{\n"
            cpp += f"        std::string hex = \"{hex_str}\";\n"
            cpp += f"        auto words = hex_to_words(hex);\n"
            cpp += f"        std::span<const uint32_t> span_data(words);\n"
            cpp += f"        {view_name} view(span_data);\n"
            cpp += f"        if (!view.isValid()) {{\n"
            cpp += f"            std::cerr << \"View Invalid. Word1: \" << std::hex << ntohl(words[0]) << \" Words Size: \" << words.size() << \"\\n\";\n"
            cpp += f"            std::cerr << \"OUI: \" << std::hex << ntohl(words[2]) << \" Codes: \" << ntohl(words[3]) << \"\\n\";\n"
            cpp += f"            std::cerr << \"CIF0: \" << std::hex << ntohl(words[9]) << \"\\n\";\n"
            cpp += f"            throw std::runtime_error(\"Invalid View for {view_name}\");\n"
            cpp += f"        }}\n"
            cpp += f"        if (view.getStreamId() != {stream_id}U) {{ throw std::runtime_error(\"Stream ID mismatch\"); }}\n"
            
            for item in active_fields:
                f = item['field']
                d = item['data']
                name = f['name']
                if pkt_name == "ExecutionAckAckXPacket":
                    pass # AckX views extract flat error payloads via getErrorPayloads(), we skip field getters
                elif f['size'] == 0:
                    pass 
                elif f['size'] == 1:
                    cpp += f"        if (view.get{name}().value_or(0) != {d[0]}U) {{ throw std::runtime_error(\"{name} mismatch\"); }}\n"
                elif f['size'] == 2:
                    val64 = (d[0] << 32) | d[1]
                    if name == 'DataFormat':
                        enum_name = data_format_enum_for_words(d[0], d[1])
                        cpp += f"        auto data_format = view.getParsedDataFormat();\n"
                        cpp += f"        if (!data_format || *data_format != DataFormat::{enum_name}) {{ throw std::runtime_error(\"DataFormat mismatch\"); }}\n"
                    else:
                        cpp += f"        if (view.get{name}().value_or(0) != {val64}ULL) {{ throw std::runtime_error(\"{name} mismatch\"); }}\n"
                elif f['size'] > 2:
                    cpp += f"        auto val_{name} = view.get{name}();\n"
                    cpp += f"        if (!val_{name}) {{ throw std::runtime_error(\"{name} missing\"); }}\n"
                    cpp += f"        if (val_{name}->size() != {f['size']}) {{ throw std::runtime_error(\"{name} size mismatch\"); }}\n"
                    cpp += f"        if (val_{name}->at(0) != {d[0]}U) {{ throw std::runtime_error(\"{name} data mismatch\"); }}\n"
                elif f['is_dynamic_struct']:
                    cpp += f"        auto val_{name} = view.get{name}Raw();\n"
                    cpp += f"        if (val_{name}.empty()) {{ throw std::runtime_error(\"{name} empty\"); }}\n"
                    cpp += f"        if (ntohl(val_{name}[0]) != {d[0]}U) {{ throw std::runtime_error(\"{name} data mismatch\"); }}\n"
            
            if pkt_name == "ExecutionAckAckXPacket":
                cpp += f"        auto ackx_errs = view.getParsedErrorPayloads();\n"
                cpp += f"        if (ackx_errs.size() != {len(payloads)}) {{\n"
                cpp += f"            std::cerr << \"Expected {len(payloads)} error payloads, got \" << ackx_errs.size() << \" hex: \" << hex << \"\\n\";\n"
                cpp += f"            std::cerr << \"eif0: \" << std::hex << view.getEif0() << \" eif1: \" << view.getEif1() << \" eif2: \" << view.getEif2() << \" eif3: \" << view.getEif3() << \" eif4: \" << view.getEif4() << \"\\n\";\n"
                cpp += f"            std::cerr << \"view.getPacketSize(): \" << (ntohl(words[0]) & 0xFFFF) << \"\\n\";\n"
                cpp += f"            throw std::runtime_error(\"AckX error payloads len mismatch\");\n"
                cpp += f"        }}\n"
                # For roundtrip generation, AckX errors might get reordered slightly since they are generated across all fields uniformly
                # but read out sequentially EIF0 -> EIF4. To not overcomplicate the C++ assert, we just check they exist if order shifts.
                if payloads:
                    cpp += f"        for (uint32_t val : {{" + ",".join(f"{x}U" for x in payloads) + f"}}) {{\n"
                    cpp += f"            bool found = std::any_of(ackx_errs.begin(), ackx_errs.end(),\n"
                    cpp += f"                                     [val](const ams::iface::vita::AckXView::ErrorPayload& err) {{\n"
                    cpp += f"                                         uint32_t rec = ackx_detail::pack_error_payload(err.subject, err.error_flags, err.error_enum_index);\n"
                    cpp += f"                                         return rec == val; }});\n"
                    cpp += f"            if (!found) {{ throw std::runtime_error(\"AckX error payload missing\"); }}\n"
                    cpp += f"        }}\n"
            
            cpp += f"        std::vector<uint32_t> buf(2048, 0);\n"
            cpp += f"        {bldr_name} builder(buf.data(), buf.size());\n"
            cpp += f"        builder.setStreamId({stream_id}U);\n"
            cpp += f"        builder.setTimestampInt({tsi}U);\n"
            cpp += f"        builder.setTimestampFracHigh({tsf_hi}U);\n"
            cpp += f"        builder.setTimestampFracLow({tsf_lo}U);\n"
            if prologue_words == 9:
                cpp += f"        builder.setMessageId({msg_id}U);\n"
                if pkt_name == "ScheduleAckAckRPacket":
                    cpp += f"        builder.setSchX({str((cam & 0x00000400) != 0).lower()});\n"
                    cpp += f"        builder.setHasErrors({str((cam & 0x00010000) != 0).lower()});\n"
                elif pkt_name == "ControlScheduleRequestPacket":
                    cpp += f"        builder.setScheduleRequestType({(cam >> 4) & 0xF}U);\n"
                elif pkt_name == "ExecutionAckAckXPacket":
                    cpp += f"        builder.setSchX({str((cam & 0x00000400) != 0).lower()});\n"
                    cpp += f"        builder.setScheduleRequestType({(cam >> 4) & 0xF}U);\n"
                
            if pkt_name == "ExecutionAckAckXPacket":
                for p in payloads:
                    subject = ackx_error_subject_for_payload(p)
                    cpp += f"        builder.addError(AckXErrorSubject::{subject}, {p}U & 0xFFF80000U, {p}U & 0x3FF);\n"
            else:
                for item in active_fields:
                    f = item['field']
                    d = item['data']
                    name = f['name']
                    if f['size'] == 0:
                        cpp += f"        builder.add{name}();\n"
                    elif f['size'] == 1:
                        cpp += f"        builder.add{name}({d[0]}U);\n"
                    elif f['size'] == 2:
                        val64 = (d[0] << 32) | d[1]
                        if name == 'DataFormat':
                            enum_name = data_format_enum_for_words(d[0], d[1])
                            cpp += f"        builder.setDataFormat(DataFormat::{enum_name});\n"
                        else:
                            cpp += f"        builder.add{name}({val64}ULL);\n"
                    elif f['size'] > 2:
                        cpp += f"        std::vector<uint32_t> d_{name} = {{" + ",".join(f"{x}U" for x in d) + f"}};\n"
                        cpp += f"        builder.add{name}(d_{name});\n"
                    elif f['is_dynamic_struct']:
                        cpp += f"        std::vector<uint32_t> d_{name} = host_to_network_words({{" + ",".join(f"{x}U" for x in d) + f"}});\n"
                        if name == 'DataAddressStructure':
                            cpp += f"        DataAddressStructureView data_address_view_{name}(d_{name});\n"
                            cpp += f"        std::vector<DataAddressRecord> records_{name}(data_address_view_{name}.begin(), data_address_view_{name}.end());\n"
                            cpp += f"        builder.addDataAddressStructure(records_{name});\n"
                        elif name == 'Pointing3dStruct':
                            cpp += f"        Pointing3dStructureView pointing_view_{name}(d_{name});\n"
                            cpp += f"        std::vector<Pointing3dRecord> records_{name}(pointing_view_{name}.begin(), pointing_view_{name}.end());\n"
                            cpp += f"        builder.addPointing3dStructure(pointing_view_{name}.getGlobalIndexRefBeam(), records_{name});\n"

                    
            cpp += f"        size_t builder_size = builder.finalize();\n"
            cpp += f"        if (builder_size != words.size()) {{ throw std::runtime_error(\"Builder Size mismatch: expected \" + std::to_string(words.size()) + \" got \" + std::to_string(builder_size)); }}\n"
            cpp += f"        if (std::memcmp(buf.data(), words.data(), builder_size * 4) != 0) {{\n"
            cpp += f"            for(size_t j=0; j<builder_size; j++) {{\n"
            cpp += f"                if (buf[j] != words[j]) {{ std::cerr << \"Mismatch at word \" << j << \": expected \" << std::hex << ntohl(words[j]) << \" got \" << ntohl(buf[j]) << \"\\n\"; }}\n"
            cpp += f"            }}\n"
            cpp += f"            throw std::runtime_error(\"Builder Bytes mismatch\");\n"
            cpp += f"        }}\n"
            cpp += f"    }}\n"
        cpp += "}\n\n"

    cpp += "void test_DataPacket() {\n"
    cpp += "    std::cout << \"Testing DataPacket...\\n\";\n"
    for i in range(5):
        bb = BitBuilder()
        prologue_words = 7
        word1_base = (1 << 28) | (1 << 27) | (3 << 22) | (2 << 20)
        stream_id = random.randint(0, 0xFFFFFFFF)
        oui = 0xAAAAAA
        codes = 0x04000D05
        tsi = random.randint(0, 0xFFFFFFFF)
        tsf_hi = random.randint(0, 0xFFFFFFFF)
        tsf_lo = random.randint(0, 0xFFFFFFFF)
        
        payload_len = random.randint(0, 10)
        payload = [random.randint(0, 0xFFFFFFFF) for _ in range(payload_len)]
        
        pkt_size = prologue_words + payload_len + 1
        word1_base |= pkt_size | (1<<26)
        
        bb.add_word(word1_base)
        bb.add_word(stream_id)
        bb.add_word(oui)
        bb.add_word(codes)
        bb.add_word(tsi)
        bb.add_word(tsf_hi)
        bb.add_word(tsf_lo)
        for p in payload:
            bb.add_word(p)
        bb.add_word(0x00C00000)
        
        hex_str = bb.to_hex()
        cpp += f"    {{\n"
        cpp += f"        std::string hex = \"{hex_str}\";\n"
        cpp += f"        auto words = hex_to_words(hex);\n"
        cpp += f"        std::span<const uint32_t> span_data(words);\n"
        cpp += f"        DataPacketView view(span_data);\n"
        cpp += f"        if (!view.isValid()) {{ throw std::runtime_error(\"Invalid Data View\"); }}\n"
        cpp += f"        if (view.getStreamId() != {stream_id}U) {{ throw std::runtime_error(\"Data Stream ID mismatch\"); }}\n"
        cpp += f"        auto payload_span = view.getPayload();\n"
        cpp += f"        if (payload_span.size() != {payload_len}U) {{ throw std::runtime_error(\"Data Payload Len mismatch\"); }}\n"
        
        cpp += f"        std::vector<uint32_t> buf(1024, 0);\n"
        cpp += f"        DataPacketBuilder builder(buf.data(), buf.size());\n"
        cpp += f"        builder.setStreamId({stream_id}U);\n"
        cpp += f"        builder.setTimestampInt({tsi}U);\n"
        cpp += f"        builder.setTimestampFracHigh({tsf_hi}U);\n"
        cpp += f"        builder.setTimestampFracLow({tsf_lo}U);\n"
        if payload:
            cpp += f"        std::vector<uint32_t> p_buf = host_to_network_words({{" + ",".join(f"{x}U" for x in payload) + f"}});\n"
            cpp += f"        builder.setPayload(p_buf);\n"
        cpp += f"        size_t builder_size = builder.finalize();\n"
        cpp += f"        if (builder_size != words.size()) {{ throw std::runtime_error(\"Data Builder Size mismatch\"); }}\n"
        cpp += f"        if (std::memcmp(buf.data(), words.data(), builder_size * 4) != 0) {{\n"
        cpp += f"            for(size_t j=0; j<builder_size; j++) {{\n"
        cpp += f"                if (buf[j] != words[j]) {{ std::cerr << \"Mismatch at word \" << j << \": expected \" << std::hex << ntohl(words[j]) << \" got \" << ntohl(buf[j]) << \"\\n\"; }}\n"
        cpp += f"            }}\n"
        cpp += f"            throw std::runtime_error(\"Data Builder Bytes mismatch\");\n"
        cpp += f"        }}\n"
        cpp += f"    }}\n"
    cpp += "}\n\n"

    cpp += "auto main() -> int {\n"
    cpp += "    try {\n"
    for pkt_name in pkt_map.keys():
        cpp += f"        test_{pkt_name}();\n"
    cpp += "        test_DataPacket();\n"
    cpp += "        std::cout << \"All generated roundtrip tests passed.\\n\";\n"
    cpp += "    } catch (const std::exception& e) {\n"
    cpp += "        std::cerr << \"Test failed: \" << e.what() << \"\\n\";\n"
    cpp += "        return 1;\n"
    cpp += "    }\n"
    cpp += "    return 0;\n"
    cpp += "}\n"

    with open('tests/test_generated_roundtrip.cpp', 'w') as f:
        f.write(cpp)

if __name__ == '__main__':
    generate()

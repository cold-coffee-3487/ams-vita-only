#pragma once

#include "ams/iface/vita/Primitives.h"
#include "ams/iface/vita/GeneratedMasks.h"
#include "ams/iface/vita/ScheduleAckAckRPacketBase.h"
#include "ams/iface/vita/ExecutionAckAckXPacketBase.h"
#include <cstdint>
#include <cstddef>
#include <optional>
#include <arpa/inet.h>
#include <span>
#include <vector>
#include <bit>
#include <algorithm>
#include <stdexcept>

namespace ams {
namespace iface {
namespace vita {

namespace ackx_detail {
constexpr uint32_t EIF0_ENABLE_MASK = masks::CIF0_CIF1_ENABLE | masks::CIF0_CIF2_ENABLE |
                                      masks::CIF0_CIF3_ENABLE | masks::CIF0_CIF4_ENABLE;
constexpr uint32_t EIF0_PAYLOAD_MASK = (1U << 31) | (1U << 29) | (1U << 27) |
                                       (1U << 23) | (1U << 21) | (1U << 15);
constexpr uint32_t EIF1_PAYLOAD_MASK = (1U << 30) | (1U << 29) | (1U << 25);
constexpr uint32_t EIF2_PAYLOAD_MASK = (1U << 6);
constexpr uint32_t EIF3_PAYLOAD_MASK = (1U << 21);
constexpr uint32_t EIF4_PAYLOAD_MASK = (1U << 29) | (1U << 24) | (1U << 21);
constexpr uint32_t EIF0_ALLOWED_MASK = EIF0_ENABLE_MASK | EIF0_PAYLOAD_MASK;

constexpr uint32_t ERROR_FLAGS_MASK = 0xFFF80000U;
constexpr uint32_t ERROR_RESERVED_MASK = 1U << 18;
constexpr uint32_t ERROR_EIF_INDEX_MASK = 0x00038000U;
constexpr uint32_t ERROR_EIF_INDEX_SHIFT = 15;
constexpr uint32_t ERROR_EIF_BIT_MASK = 0x00007C00U;
constexpr uint32_t ERROR_EIF_BIT_SHIFT = 10;
constexpr uint32_t ERROR_ENUM_INDEX_MASK = 0x000003FFU;

constexpr uint32_t ACK_ACTION_EXECUTE = 2U << masks::CAM_ACTION_SHIFT;
constexpr uint32_t ACKX_CAM_ALLOWED_MASK = masks::CAM_ACTION_MASK | masks::CAM_ACKX_MASK |
                                           masks::CAM_ACKER_MASK | masks::CAM_SCHX_MASK |
                                           masks::CAM_SCH_REQ_TYP_MASK;
constexpr uint32_t ACKX_CAM_REQUIRED_MASK = ACK_ACTION_EXECUTE | masks::CAM_ACKX_MASK;
constexpr uint32_t ACKR_CAM_ALLOWED_MASK = masks::CAM_ACTION_MASK | masks::CAM_ACKER_MASK |
                                           masks::CAM_ACKR_MASK | masks::CAM_SCHX_MASK |
                                           masks::CAM_SCH_REQ_TYP_MASK;
constexpr uint32_t ACKR_CAM_REQUIRED_MASK = ACK_ACTION_EXECUTE | masks::CAM_ACKR_MASK |
                                           (1U << masks::CAM_SCH_REQ_TYP_SHIFT);

constexpr uint32_t ERROR_NOT_EXECUTED = ackx_error_flags::NotExecuted;
constexpr uint32_t ERROR_DEVICE_FAILURE = ackx_error_flags::DeviceFailure;
constexpr uint32_t ERROR_ERRONEOUS_FIELD = ackx_error_flags::ErroneousField;
constexpr uint32_t ERROR_OUT_OF_RANGE = ackx_error_flags::OutOfRange;
constexpr uint32_t ERROR_UNSUPPORTED_PRECISION = ackx_error_flags::UnsupportedPrecision;
constexpr uint32_t ERROR_INVALID_VALUE = ackx_error_flags::InvalidValue;
constexpr uint32_t ERROR_BAD_TIMESTAMP = ackx_error_flags::BadTimestamp;
constexpr uint32_t ERROR_HAZARDOUS_POWER_LEVELS = ackx_error_flags::HazardousPowerLevels;
constexpr uint32_t ERROR_DISTORTION = ackx_error_flags::Distortion;
constexpr uint32_t ERROR_IN_BAND_POWER_COMPLIANCE = ackx_error_flags::InBandPowerCompliance;
constexpr uint32_t ERROR_OUT_OF_BAND_POWER_COMPLIANCE = ackx_error_flags::OutOfBandPowerCompliance;
constexpr uint32_t ERROR_CO_SITE_INTERFERENCE = ackx_error_flags::CoSiteInterference;
constexpr uint32_t ERROR_REGIONAL_INTERFERENCE = ackx_error_flags::RegionalInterference;

struct SubjectEncoding {
    AckXErrorSubject subject;
    uint8_t eif_index;
    uint8_t eif_bit;
    uint8_t payload_eif_index;
    uint8_t payload_bit;
};

inline SubjectEncoding encoding_for_subject(AckXErrorSubject subject) {
    switch (subject) {
        case AckXErrorSubject::NoCorrespondingPayloadField: return {subject, 0, 31, 7, 0};
        case AckXErrorSubject::Bandwidth: return {subject, 0, 29, 0, 29};
        case AckXErrorSubject::RfRefFreq: return {subject, 0, 27, 0, 27};
        case AckXErrorSubject::Gain: return {subject, 0, 23, 0, 23};
        case AckXErrorSubject::SampleRate: return {subject, 0, 21, 0, 21};
        case AckXErrorSubject::DataFormat: return {subject, 0, 15, 0, 15};
        case AckXErrorSubject::Polarization: return {subject, 1, 30, 1, 30};
        case AckXErrorSubject::Pointing3d: return {subject, 1, 29, 1, 29};
        case AckXErrorSubject::BeamWidth: return {subject, 1, 25, 1, 25};
        case AckXErrorSubject::FuncPriorityId: return {subject, 2, 6, 2, 6};
        case AckXErrorSubject::Dwell: return {subject, 3, 21, 3, 21};
        case AckXErrorSubject::RfFigureOfMerit: return {subject, 4, 29, 4, 29};
        case AckXErrorSubject::AddressGroupIndex: return {subject, 4, 24, 4, 24};
        case AckXErrorSubject::TxDigitalInputPower: return {subject, 4, 21, 4, 21};
    }
    throw std::invalid_argument("Unknown AckX error subject");
}

inline AckXErrorSubject subject_for_eif_bit(uint8_t eif_index, uint8_t bit_index) {
    if (eif_index == 0 && bit_index == 31) return AckXErrorSubject::NoCorrespondingPayloadField;
    if (eif_index == 0 && bit_index == 29) return AckXErrorSubject::Bandwidth;
    if (eif_index == 0 && bit_index == 27) return AckXErrorSubject::RfRefFreq;
    if (eif_index == 0 && bit_index == 23) return AckXErrorSubject::Gain;
    if (eif_index == 0 && bit_index == 21) return AckXErrorSubject::SampleRate;
    if (eif_index == 0 && bit_index == 15) return AckXErrorSubject::DataFormat;
    if (eif_index == 1 && bit_index == 30) return AckXErrorSubject::Polarization;
    if (eif_index == 1 && bit_index == 29) return AckXErrorSubject::Pointing3d;
    if (eif_index == 1 && bit_index == 25) return AckXErrorSubject::BeamWidth;
    if (eif_index == 2 && bit_index == 6) return AckXErrorSubject::FuncPriorityId;
    if (eif_index == 3 && bit_index == 21) return AckXErrorSubject::Dwell;
    if (eif_index == 4 && bit_index == 29) return AckXErrorSubject::RfFigureOfMerit;
    if (eif_index == 4 && bit_index == 24) return AckXErrorSubject::AddressGroupIndex;
    if (eif_index == 4 && bit_index == 21) return AckXErrorSubject::TxDigitalInputPower;
    throw std::invalid_argument("Unsupported AckX EIF bit");
}

inline uint32_t eif_payload_mask(uint8_t eif_index) {
    switch (eif_index) {
        case 0: return EIF0_PAYLOAD_MASK;
        case 1: return EIF1_PAYLOAD_MASK;
        case 2: return EIF2_PAYLOAD_MASK;
        case 3: return EIF3_PAYLOAD_MASK;
        case 4: return EIF4_PAYLOAD_MASK;
        default: return 0;
    }
}

inline bool payload_identifier_matches(AckXErrorSubject subject, uint8_t payload_eif_index, uint8_t payload_bit) {
    const SubjectEncoding encoding = encoding_for_subject(subject);
    return payload_eif_index == encoding.payload_eif_index && payload_bit == encoding.payload_bit;
}

inline bool is_valid_ackx_cam(uint32_t cam) {
    return (cam & ~ACKX_CAM_ALLOWED_MASK) == 0U &&
           (cam & (masks::CAM_ACTION_MASK | masks::CAM_ACKX_MASK)) == ACKX_CAM_REQUIRED_MASK;
}

inline bool is_valid_ackr_cam(uint32_t cam) {
    return (cam & ~ACKR_CAM_ALLOWED_MASK) == 0U &&
           (cam & (masks::CAM_ACTION_MASK | masks::CAM_ACKR_MASK | masks::CAM_SCH_REQ_TYP_MASK)) == ACKR_CAM_REQUIRED_MASK;
}

inline size_t count_payload_indicators(uint32_t eif0, uint32_t eif1, uint32_t eif2, uint32_t eif3, uint32_t eif4) {
    size_t count = static_cast<size_t>(std::popcount(eif0 & EIF0_PAYLOAD_MASK));
    if (eif0 & masks::CIF0_CIF1_ENABLE) count += static_cast<size_t>(std::popcount(eif1 & EIF1_PAYLOAD_MASK));
    if (eif0 & masks::CIF0_CIF2_ENABLE) count += static_cast<size_t>(std::popcount(eif2 & EIF2_PAYLOAD_MASK));
    if (eif0 & masks::CIF0_CIF3_ENABLE) count += static_cast<size_t>(std::popcount(eif3 & EIF3_PAYLOAD_MASK));
    if (eif0 & masks::CIF0_CIF4_ENABLE) count += static_cast<size_t>(std::popcount(eif4 & EIF4_PAYLOAD_MASK));
    return count;
}

inline bool eif_words_are_supported(uint32_t eif0, uint32_t eif1, uint32_t eif2, uint32_t eif3, uint32_t eif4) {
    if ((eif0 & ~EIF0_ALLOWED_MASK) != 0) return false;
    if ((eif0 & masks::CIF0_CIF1_ENABLE) != 0 && (eif1 == 0 || (eif1 & ~EIF1_PAYLOAD_MASK) != 0)) return false;
    if ((eif0 & masks::CIF0_CIF2_ENABLE) != 0 && (eif2 == 0 || (eif2 & ~EIF2_PAYLOAD_MASK) != 0)) return false;
    if ((eif0 & masks::CIF0_CIF3_ENABLE) != 0 && (eif3 == 0 || (eif3 & ~EIF3_PAYLOAD_MASK) != 0)) return false;
    if ((eif0 & masks::CIF0_CIF4_ENABLE) != 0 && (eif4 == 0 || (eif4 & ~EIF4_PAYLOAD_MASK) != 0)) return false;
    return true;
}

inline uint32_t pack_error_payload(AckXErrorSubject subject, uint32_t error_flags, uint16_t error_enum_index) {
    const SubjectEncoding encoding = encoding_for_subject(subject);
    return error_flags |
           (static_cast<uint32_t>(encoding.payload_eif_index) << ERROR_EIF_INDEX_SHIFT) |
           (static_cast<uint32_t>(encoding.payload_bit) << ERROR_EIF_BIT_SHIFT) |
           static_cast<uint32_t>(error_enum_index);
}
} // namespace ackx_detail


class AckRView : public ScheduleAckAckRPacketBaseView {
private:
    const uint32_t* buffer;

public:
    explicit AckRView(std::span<const uint32_t> raw_buffer) {
        if (raw_buffer.size() < getPrologueWords() + 3) {
            buffer = nullptr;
            return;
        }

        uint32_t word1 = ntohl(raw_buffer[0]);
        // Validate Packet Type (0111)
        if (((word1 & masks::PKT_TYPE_MASK) >> masks::PKT_TYPE_SHIFT) != 7U) {
            buffer = nullptr;
            return;
        }
        // Validate C-Bit
        if ((word1 & masks::CBIT_MASK) == 0) {
            buffer = nullptr;
            return;
        }
        // Validate TSI (11)
        if (((word1 & masks::TSI_MASK) >> masks::TSI_SHIFT) != 3U) {
            buffer = nullptr;
            return;
        }
        // Validate TSF (10)
        if (((word1 & masks::TSF_MASK) >> masks::TSF_SHIFT) != 2U) {
            buffer = nullptr;
            return;
        }
        if ((word1 & masks::CMD_ACK_MASK) == 0U || (word1 & ((1U << 25) | (1U << 24))) != 0U) {
            buffer = nullptr;
            return;
        }

        // Validate Fixed Metadata Class ID
        if (ntohl(raw_buffer[2]) != 0xAAAAAA) { // OUI
            buffer = nullptr;
            return;
        }
        if (!isAmsClassIdCodes(ntohl(raw_buffer[3]), 0x0BU)) { // Info & Pkt Class
            buffer = nullptr;
            return;
        }

        uint32_t pkt_size = word1 & masks::PKT_SIZE_MASK;
        if (raw_buffer.size() < pkt_size || pkt_size < getPrologueWords() + 3) {
            buffer = nullptr;
            return;
        }

        buffer = raw_buffer.data();
        if (!ackx_detail::is_valid_ackr_cam(getCam())) {
            buffer = nullptr;
            return;
        }

        const uint32_t* current = buffer + getPrologueWords();
        const uint32_t* end = buffer + pkt_size;

        if (current >= end) { buffer = nullptr; return; }
        cif0 = ntohl(*current++);
        if ((cif0 & masks::CIF0_CIF2_ENABLE) == 0 ||
            (cif0 & masks::CIF0_CIF4_ENABLE) == 0) {
            buffer = nullptr;
            return;
        }

        if (cif0 & masks::CIF0_CIF1_ENABLE) { if (current >= end) { buffer = nullptr; return; } cif1 = ntohl(*current++); }
        if (cif0 & masks::CIF0_CIF2_ENABLE) { if (current >= end) { buffer = nullptr; return; } cif2 = ntohl(*current++); }
        if (cif0 & masks::CIF0_CIF3_ENABLE) { if (current >= end) { buffer = nullptr; return; } cif3 = ntohl(*current++); }
        if (cif0 & masks::CIF0_CIF4_ENABLE) { if (current >= end) { buffer = nullptr; return; } cif4 = ntohl(*current++); }
        if (cif0 & masks::CIF0_CIF7_ENABLE) { if (current >= end) { buffer = nullptr; return; } cif7 = ntohl(*current++); }

        current = mapGeneratedFields(current, end);
        if (!current || current != end) {
            buffer = nullptr;
            return;
        }
    }

    inline bool isValid() const { return buffer != nullptr; }

    // Static prologue getters
    inline uint32_t getWord1() const { return buffer ? ntohl(buffer[0]) : 0; }
    inline uint32_t getStreamId() const { return buffer ? ntohl(buffer[1]) : 0; }
    inline uint32_t getClassIdOui() const { return buffer ? ntohl(buffer[2]) : 0; }
    inline uint32_t getClassIdCodes() const { return buffer ? ntohl(buffer[3]) : 0; }
    inline uint32_t getTimestampInt() const { return buffer ? ntohl(buffer[4]) : 0; }
    inline uint32_t getTimestampFracHigh() const { return buffer ? ntohl(buffer[5]) : 0; }
    inline uint32_t getTimestampFracLow() const { return buffer ? ntohl(buffer[6]) : 0; }
    inline uint8_t getPacketCount() const { return buffer ? ((ntohl(buffer[0]) >> 16) & 0xF) : 0; }

    inline bool hasClassId() const { return (getWord1() & masks::CBIT_MASK) != 0; }
    inline bool hasTsi() const { return ((getWord1() & masks::TSI_MASK) >> masks::TSI_SHIFT) != 0; }
    inline bool hasTsf() const { return ((getWord1() & masks::TSF_MASK) >> masks::TSF_SHIFT) != 0; }

    static inline size_t getPrologueWords() {
        return 9; // AMS GRA Tailored Ack packets have a strict 9-word prologue
    }

    inline uint32_t getCam() const { return buffer ? ntohl(buffer[getPrologueWords() - 2]) : 0; }
    inline uint32_t getMessageId() const { return buffer ? ntohl(buffer[getPrologueWords() - 1]) : 0; }
    
    inline bool isAckR() const { return (getCam() & masks::CAM_ACKR_MASK) != 0; }
    inline bool hasErrors() const { return (getCam() & masks::CAM_ACKER_MASK) != 0; }
    inline bool isSchX() const { return (getCam() & masks::CAM_SCHX_MASK) != 0; }

};

class AckXView : public ExecutionAckAckXPacketBaseView {
private:
    const uint32_t* buffer;
    
    uint32_t eif0 = 0;
    uint32_t eif1 = 0;
    uint32_t eif2 = 0;
    uint32_t eif3 = 0;
    uint32_t eif4 = 0;

    const uint32_t* error_payloads_start = nullptr;
    size_t error_payload_count = 0;

public:
    inline bool hasClassId() const { return (getWord1() & masks::CBIT_MASK) != 0; }
    inline bool hasTsi() const { return ((getWord1() & masks::TSI_MASK) >> masks::TSI_SHIFT) != 0; }
    inline bool hasTsf() const { return ((getWord1() & masks::TSF_MASK) >> masks::TSF_SHIFT) != 0; }

    static inline size_t getPrologueWords() {
        return 9; // AMS GRA Tailored Ack packets have a strict 9-word prologue
    }

    explicit AckXView(std::span<const uint32_t> raw_buffer) {
        if (raw_buffer.size() < getPrologueWords()) {
            buffer = nullptr;
            return;
        }

        uint32_t word1 = ntohl(raw_buffer[0]);
        // Validate Packet Type (0111)
        if (((word1 & masks::PKT_TYPE_MASK) >> masks::PKT_TYPE_SHIFT) != 7U) {
            buffer = nullptr;
            return;
        }
        // Validate C-Bit
        if ((word1 & masks::CBIT_MASK) == 0) {
            buffer = nullptr;
            return;
        }
        // Validate TSI (11)
        if (((word1 & masks::TSI_MASK) >> masks::TSI_SHIFT) != 3U) {
            buffer = nullptr;
            return;
        }
        // Validate TSF (10)
        if (((word1 & masks::TSF_MASK) >> masks::TSF_SHIFT) != 2U) {
            buffer = nullptr;
            return;
        }
        if ((word1 & masks::CMD_ACK_MASK) == 0U || (word1 & ((1U << 25) | (1U << 24))) != 0U) {
            buffer = nullptr;
            return;
        }

        // Validate Fixed Metadata Class ID
        if (ntohl(raw_buffer[2]) != 0xAAAAAA) { // OUI
            buffer = nullptr;
            return;
        }
        if (!isAmsClassIdCodes(ntohl(raw_buffer[3]), 0x0CU)) { // Info & Pkt Class
            buffer = nullptr;
            return;
        }

        buffer = raw_buffer.data();
        if (!ackx_detail::is_valid_ackx_cam(getCam())) {
            buffer = nullptr;
            return;
        }
        if (!hasErrors()) {
            uint32_t pkt_size = word1 & masks::PKT_SIZE_MASK;
            if (raw_buffer.size() < pkt_size || pkt_size != getPrologueWords()) {
                buffer = nullptr;
            }
            return;
        }

        if (raw_buffer.size() < getPrologueWords() + 1) {
            buffer = nullptr;
            return;
        }

        uint32_t pkt_size = word1 & masks::PKT_SIZE_MASK;
        if (raw_buffer.size() < pkt_size || pkt_size < getPrologueWords() + 1) {
            buffer = nullptr;
            return;
        }

        const uint32_t* current = raw_buffer.data() + getPrologueWords();
        const uint32_t* end = raw_buffer.data() + pkt_size;

        if (current >= end) { buffer = nullptr; return; }
        eif0 = ntohl(*current++);
        
        if (eif0 & masks::CIF0_CIF1_ENABLE) { if (current >= end) { buffer = nullptr; return; } eif1 = ntohl(*current++); }
        if (eif0 & masks::CIF0_CIF2_ENABLE) { if (current >= end) { buffer = nullptr; return; } eif2 = ntohl(*current++); }
        if (eif0 & masks::CIF0_CIF3_ENABLE) { if (current >= end) { buffer = nullptr; return; } eif3 = ntohl(*current++); }
        if (eif0 & masks::CIF0_CIF4_ENABLE) { if (current >= end) { buffer = nullptr; return; } eif4 = ntohl(*current++); }

        error_payloads_start = current;

        if (!ackx_detail::eif_words_are_supported(eif0, eif1, eif2, eif3, eif4)) {
            buffer = nullptr;
            return;
        }

        error_payload_count = ackx_detail::count_payload_indicators(eif0, eif1, eif2, eif3, eif4);
        if (error_payload_count == 0 || error_payloads_start + error_payload_count != end) {
            buffer = nullptr;
            return;
        }

        const uint32_t eifs[] = {eif0, eif1, eif2, eif3, eif4};
        size_t payload_idx = 0;
        for (uint8_t eif_idx = 0; eif_idx < 5U; ++eif_idx) {
            if (eif_idx > 0 && (eif0 & (1U << eif_idx)) == 0) continue;
            uint32_t mask = ackx_detail::eif_payload_mask(eif_idx);
            for (int bit = 31; bit >= 0; --bit) {
                if ((eifs[eif_idx] & mask & (1U << bit)) == 0) continue;
                uint32_t payload = ntohl(error_payloads_start[payload_idx++]);
                if ((payload & ackx_detail::ERROR_RESERVED_MASK) != 0) { buffer = nullptr; return; }
                uint8_t payload_eif = static_cast<uint8_t>((payload & ackx_detail::ERROR_EIF_INDEX_MASK) >> ackx_detail::ERROR_EIF_INDEX_SHIFT);
                uint8_t payload_bit = static_cast<uint8_t>((payload & ackx_detail::ERROR_EIF_BIT_MASK) >> ackx_detail::ERROR_EIF_BIT_SHIFT);
                AckXErrorSubject subject;
                try {
                    subject = ackx_detail::subject_for_eif_bit(eif_idx, static_cast<uint8_t>(bit));
                } catch (const std::invalid_argument&) {
                    buffer = nullptr;
                    return;
                }
                if (!ackx_detail::payload_identifier_matches(subject, payload_eif, payload_bit)) { buffer = nullptr; return; }
            }
        }
    }

    inline bool isValid() const { return buffer != nullptr; }

    // Static prologue getters
    inline uint32_t getWord1() const { return buffer ? ntohl(buffer[0]) : 0; }
    inline uint32_t getStreamId() const { return buffer ? ntohl(buffer[1]) : 0; }
    inline uint32_t getClassIdOui() const { return buffer ? ntohl(buffer[2]) : 0; }
    inline uint32_t getClassIdCodes() const { return buffer ? ntohl(buffer[3]) : 0; }
    inline uint32_t getTimestampInt() const { return buffer ? ntohl(buffer[4]) : 0; }
    inline uint32_t getTimestampFracHigh() const { return buffer ? ntohl(buffer[5]) : 0; }
    inline uint32_t getTimestampFracLow() const { return buffer ? ntohl(buffer[6]) : 0; }
    inline uint8_t getPacketCount() const { return buffer ? ((ntohl(buffer[0]) >> 16) & 0xF) : 0; }
    inline uint32_t getCam() const { return buffer ? ntohl(buffer[getPrologueWords() - 2]) : 0; }
    inline uint32_t getMessageId() const { return buffer ? ntohl(buffer[getPrologueWords() - 1]) : 0; }
    inline uint8_t getScheduleRequestType() const {
        return static_cast<uint8_t>((getCam() & masks::CAM_SCH_REQ_TYP_MASK) >> masks::CAM_SCH_REQ_TYP_SHIFT);
    }
    
    inline bool hasErrors() const { 
        return (getCam() & masks::CAM_ACKER_MASK) != 0; 
    }

    inline bool isAckX() const { return (getCam() & masks::CAM_ACKX_MASK) != 0; }
    inline bool isSchX() const { return (getCam() & masks::CAM_SCHX_MASK) != 0; }

    inline uint32_t getEif0() const { return eif0; }
    inline uint32_t getEif1() const { return eif1; }
    inline uint32_t getEif2() const { return eif2; }
    inline uint32_t getEif3() const { return eif3; }
    inline uint32_t getEif4() const { return eif4; }

    using ErrorPayload = AckXErrorPayload;

    std::vector<ErrorPayload> getParsedErrorPayloads() const {
        std::vector<ErrorPayload> payloads;
        if (!buffer || !error_payloads_start) return payloads;
        
        payloads.reserve(error_payload_count);
        const uint32_t eifs[] = {eif0, eif1, eif2, eif3, eif4};
        size_t payload_idx = 0;
        for (uint8_t eif_idx = 0; eif_idx < 5U; ++eif_idx) {
            if (eif_idx > 0 && (eif0 & (1U << eif_idx)) == 0) continue;
            uint32_t mask = ackx_detail::eif_payload_mask(eif_idx);
            for (int bit = 31; bit >= 0; --bit) {
                if ((eifs[eif_idx] & mask & (1U << bit)) == 0) continue;
                uint32_t payload = ntohl(error_payloads_start[payload_idx++]);
                payloads.push_back({
                    ackx_detail::subject_for_eif_bit(eif_idx, static_cast<uint8_t>(bit)),
                    payload & ackx_detail::ERROR_FLAGS_MASK,
                    static_cast<uint16_t>(payload & ackx_detail::ERROR_ENUM_INDEX_MASK)
                });
            }
        }
        
        return payloads;
    }
};


class AckRBuilder : public ScheduleAckAckRPacketBaseBuilder {
private:
    uint32_t* buffer;
    size_t max_words;

    uint32_t word1 = 0;
    uint32_t stream_id = 0;
    uint32_t class_id_oui = htonl(0xAAAAAA);
    uint32_t class_id_codes = htonl(makeAmsClassIdCodes(InfoClassCodeType::TxCommBaseSet, 0x0BU));
    uint32_t ts_int = 0;
    uint32_t ts_frac_hi = 0;
    uint32_t ts_frac_lo = 0;
    uint32_t message_id = 0;
    bool has_errors = false;
    bool schx = true;


public:
    explicit AckRBuilder(uint32_t* target_buffer, size_t max_words_in) 
        : buffer(target_buffer), max_words(max_words_in) {}

    void setStreamId(uint32_t val) { stream_id = htonl(val); }
    void setTimestampInt(uint32_t val) { ts_int = htonl(val); }
    void setTimestampFracHigh(uint32_t val) { ts_frac_hi = htonl(val); }
    void setTimestampFracLow(uint32_t val) { ts_frac_lo = htonl(val); }
    void setMessageId(uint32_t val) { message_id = htonl(val); }
    void setInfoClassType(InfoClassCodeType val) { class_id_codes = htonl(makeAmsClassIdCodes(val, 0x0BU)); }
    void setHasErrors(bool val) { has_errors = val; }
    void setSchX(bool val) { schx = val; }
    void setPacketCount(uint8_t val) {
        word1 &= ~(0xFU << 16);
        word1 |= ((static_cast<uint32_t>(val) & 0xF) << 16);
    }

    size_t finalize() {
        if (cif1) cif0 |= masks::CIF0_CIF1_ENABLE;
        cif0 |= masks::CIF0_CIF2_ENABLE;
        if (cif3) cif0 |= masks::CIF0_CIF3_ENABLE;
        cif0 |= masks::CIF0_CIF4_ENABLE;

        size_t offset = 1;
        // Calculate exact number of CIF words needed
        size_t cif_words = 1; // CIF0 is always written
        if (cif0 & masks::CIF0_CIF1_ENABLE) cif_words++;
        if (cif0 & masks::CIF0_CIF2_ENABLE) cif_words++;
        if (cif0 & masks::CIF0_CIF3_ENABLE) cif_words++;
        if (cif0 & masks::CIF0_CIF4_ENABLE) cif_words++;

        if (offset + 8 + cif_words > max_words) return 0; // 8 remaining prologue + variable CIFs
        
        buffer[offset++] = stream_id;
        buffer[offset++] = class_id_oui;
        buffer[offset++] = class_id_codes;
        buffer[offset++] = ts_int;
        buffer[offset++] = ts_frac_hi;
        buffer[offset++] = ts_frac_lo;
        const uint32_t cam_host = ackx_detail::ACKR_CAM_REQUIRED_MASK |
                                  (has_errors ? masks::CAM_ACKER_MASK : 0U) |
                                  (schx ? masks::CAM_SCHX_MASK : 0U);
        buffer[offset++] = htonl(cam_host);
        buffer[offset++] = message_id;

        buffer[offset++] = htonl(cif0);
        if (cif0 & masks::CIF0_CIF1_ENABLE) buffer[offset++] = htonl(cif1);
        if (cif0 & masks::CIF0_CIF2_ENABLE) buffer[offset++] = htonl(cif2);
        if (cif0 & masks::CIF0_CIF3_ENABLE) buffer[offset++] = htonl(cif3);
        if (cif0 & masks::CIF0_CIF4_ENABLE) buffer[offset++] = htonl(cif4);

        offset = writeGeneratedFields(buffer, offset, max_words);
        if (offset == 0 || offset > 65535U) return 0;



        // Force ams_vita_49-2_tailoring word1 prologue constraints for AckR
        word1 &= ~masks::PKT_TYPE_MASK;
        word1 |= (7U << masks::PKT_TYPE_SHIFT); // 0111
        word1 |= masks::CBIT_MASK; // 1
        word1 |= (1U << 26); // .isAck = 1
        word1 &= ~(1U << 25); // .reservedBit25 = 0
        word1 &= ~(1U << 24); // .reservedBit24 = 0
        word1 |= masks::TSI_MASK; // 11
        word1 &= ~masks::TSF_MASK;
        word1 |= (2 << masks::TSF_SHIFT); // 10
        word1 = (word1 & ~masks::PKT_SIZE_MASK) | (offset & masks::PKT_SIZE_MASK);
        buffer[0] = htonl(word1);

        return offset;
    }
};

class AckXBuilder : public ExecutionAckAckXPacketBaseBuilder {
private:
    uint32_t* buffer;
    size_t max_words;

    uint32_t word1 = 0;
    uint32_t stream_id = 0;
    uint32_t class_id_oui = htonl(0xAAAAAA);
    uint32_t class_id_codes = htonl(makeAmsClassIdCodes(InfoClassCodeType::TxCommBaseSet, 0x0CU));
    uint32_t ts_int = 0;
    uint32_t ts_frac_hi = 0;
    uint32_t ts_frac_lo = 0;
    uint32_t message_id = 0;
    bool schx = true;
    uint8_t schedule_request_type = 0;

    struct PendingError {
        AckXErrorSubject subject;
        uint32_t error_flags;
        uint16_t error_enum_index;
    };

    std::vector<PendingError> error_payloads;

public:
    explicit AckXBuilder(uint32_t* target_buffer, size_t max_words_in) 
        : buffer(target_buffer), max_words(max_words_in) {}

    void setStreamId(uint32_t val) { stream_id = htonl(val); }
    void setTimestampInt(uint32_t val) { ts_int = htonl(val); }
    void setTimestampFracHigh(uint32_t val) { ts_frac_hi = htonl(val); }
    void setTimestampFracLow(uint32_t val) { ts_frac_lo = htonl(val); }
    void setMessageId(uint32_t val) { message_id = htonl(val); }
    void setInfoClassType(InfoClassCodeType val) { class_id_codes = htonl(makeAmsClassIdCodes(val, 0x0CU)); }
    void setPacketCount(uint8_t val) {
        word1 &= ~(0xFU << 16);
        word1 |= ((static_cast<uint32_t>(val) & 0xF) << 16);
    }
    void setSchX(bool val) { schx = val; }
    void setScheduleRequestType(uint8_t val) {
        if (val > 0x0FU) {
            throw std::invalid_argument("AckX schedule request type must fit in 4 bits");
        }
        schedule_request_type = val;
    }

    void addError(AckXErrorSubject subject, uint32_t error_flags, uint16_t error_enum_index) {
        if ((error_flags & ~ackx_detail::ERROR_FLAGS_MASK) != 0) {
            throw std::invalid_argument("AckX error flags must use bits 31:19");
        }
        if ((error_enum_index & ~static_cast<uint16_t>(ackx_detail::ERROR_ENUM_INDEX_MASK)) != 0) {
            throw std::invalid_argument("AckX error enumeration index must fit in 10 bits");
        }
        if (std::any_of(error_payloads.begin(), error_payloads.end(), [subject](const PendingError& pending) {
                return pending.subject == subject;
            })) {
            throw std::invalid_argument("AckX error payload subject duplicated");
        }

        error_payloads.push_back({subject, error_flags, error_enum_index});
    }

    size_t finalize() {
        const bool has_error_payloads = !error_payloads.empty();
        uint32_t cam_host = ackx_detail::ACKX_CAM_REQUIRED_MASK |
                            (schx ? masks::CAM_SCHX_MASK : 0U) |
                            (static_cast<uint32_t>(schedule_request_type) << masks::CAM_SCH_REQ_TYP_SHIFT);

        uint32_t eif0 = 0;
        uint32_t eif1 = 0;
        uint32_t eif2 = 0;
        uint32_t eif3 = 0;
        uint32_t eif4 = 0;

        for (const PendingError& pending : error_payloads) {
            const ackx_detail::SubjectEncoding encoding = ackx_detail::encoding_for_subject(pending.subject);
            const uint32_t indicator_bit = 1U << encoding.eif_bit;
            if (encoding.eif_index == 0) {
                eif0 |= indicator_bit;
            } else if (encoding.eif_index == 1) {
                eif1 |= indicator_bit;
            } else if (encoding.eif_index == 2) {
                eif2 |= indicator_bit;
            } else if (encoding.eif_index == 3) {
                eif3 |= indicator_bit;
            } else if (encoding.eif_index == 4) {
                eif4 |= indicator_bit;
            } else {
                return 0;
            }
        }

        if (eif1) eif0 |= masks::CIF0_CIF1_ENABLE;
        if (eif2) eif0 |= masks::CIF0_CIF2_ENABLE;
        if (eif3) eif0 |= masks::CIF0_CIF3_ENABLE;
        if (eif4) eif0 |= masks::CIF0_CIF4_ENABLE;

        size_t offset = 1;
        if (offset + 8 > max_words) return 0; // 8 remaining prologue

        buffer[offset++] = stream_id;
        buffer[offset++] = class_id_oui;
        buffer[offset++] = class_id_codes;
        buffer[offset++] = ts_int;
        buffer[offset++] = ts_frac_hi;
        buffer[offset++] = ts_frac_lo;
        buffer[offset++] = htonl(cam_host);
        buffer[offset++] = message_id;

        if (has_error_payloads) {
            cam_host |= masks::CAM_ACKER_MASK;
            
            size_t eif_words = 1; // EIF0
            if (eif0 & masks::CIF0_CIF1_ENABLE) eif_words++;
            if (eif0 & masks::CIF0_CIF2_ENABLE) eif_words++;
            if (eif0 & masks::CIF0_CIF3_ENABLE) eif_words++;
            if (eif0 & masks::CIF0_CIF4_ENABLE) eif_words++;

            if (!ackx_detail::eif_words_are_supported(eif0, eif1, eif2, eif3, eif4)) {
                return 0;
            }

            size_t expected_payload_count = ackx_detail::count_payload_indicators(eif0, eif1, eif2, eif3, eif4);
            if (expected_payload_count == 0 || expected_payload_count != error_payloads.size()) {
                return 0; // The number of error payloads must exactly match the number of set EIF payload indicators
            }

            std::vector<uint32_t> ordered_payloads;
            ordered_payloads.reserve(error_payloads.size());
            const uint32_t eifs[] = {eif0, eif1, eif2, eif3, eif4};
            for (uint8_t eif_idx = 0; eif_idx < 5U; ++eif_idx) {
                if (eif_idx > 0 && (eif0 & (1U << eif_idx)) == 0) continue;
                uint32_t mask = ackx_detail::eif_payload_mask(eif_idx);
                for (int bit = 31; bit >= 0; --bit) {
                    if ((eifs[eif_idx] & mask & (1U << bit)) == 0) continue;
                    const AckXErrorSubject subject = ackx_detail::subject_for_eif_bit(eif_idx, static_cast<uint8_t>(bit));
                    const auto pending = std::find_if(error_payloads.begin(), error_payloads.end(), [subject](const PendingError& candidate) {
                        return candidate.subject == subject;
                    });
                    if (pending == error_payloads.end()) return 0;
                    const uint32_t payload = ackx_detail::pack_error_payload(subject, pending->error_flags, pending->error_enum_index);
                    if ((payload & ackx_detail::ERROR_RESERVED_MASK) != 0) return 0;
                    ordered_payloads.push_back(payload);
                }
            }

            if (ordered_payloads.size() != error_payloads.size()) return 0;

            if (offset + eif_words > max_words) return 0;
            buffer[offset++] = htonl(eif0);
            if (eif0 & masks::CIF0_CIF1_ENABLE) buffer[offset++] = htonl(eif1);
            if (eif0 & masks::CIF0_CIF2_ENABLE) buffer[offset++] = htonl(eif2);
            if (eif0 & masks::CIF0_CIF3_ENABLE) buffer[offset++] = htonl(eif3);
            if (eif0 & masks::CIF0_CIF4_ENABLE) buffer[offset++] = htonl(eif4);

            for (uint32_t err : ordered_payloads) {
                if (offset + 1 > max_words) return 0; // bounds check error payloads
                buffer[offset++] = htonl(err);
            }
            
            // Recalculate CAM since we might have modified it
            buffer[7] = htonl(cam_host);
        }

        if (offset > 65535U) return 0;

        // Force ams_vita_49-2_tailoring word1 prologue constraints for AckX
        word1 &= ~masks::PKT_TYPE_MASK;
        word1 |= (7U << masks::PKT_TYPE_SHIFT); // 0111
        word1 |= masks::CBIT_MASK; // 1
        word1 |= (1U << 26); // .isAck = 1
        word1 &= ~(1U << 25); // .reservedBit25 = 0
        word1 &= ~(1U << 24); // .reservedBit24 = 0
        word1 |= masks::TSI_MASK; // 11
        word1 &= ~masks::TSF_MASK;
        word1 |= (2 << masks::TSF_SHIFT); // 10
        
        if ((cam_host & masks::CAM_ACKER_MASK) == 0U) {
            offset = 9;
        }

        word1 = (word1 & ~masks::PKT_SIZE_MASK) | (offset & masks::PKT_SIZE_MASK);
        buffer[0] = htonl(word1);

        return offset;
    }
};


} // namespace vita
} // namespace iface
} // namespace ams

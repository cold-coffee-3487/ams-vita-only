#pragma once

#include "ams/iface/vita/Primitives.h"
#include "ams/iface/vita/ControlScheduleRequestPacketBase.h"
#include <cstdint>
#include <cstddef>
#include <optional>
#include <algorithm>
#include <arpa/inet.h>
#include <span>
#include <stdexcept>
#include <vector>

namespace ams::iface::vita {

namespace control_schedule_request_detail {
constexpr uint32_t CONTROL_ACTION_EXECUTE = 2U << masks::CAM_ACTION_SHIFT;
constexpr uint32_t CONTROL_CAM_ALLOWED_MASK = masks::CAM_W_MASK | masks::CAM_ACTION_MASK |
                                             masks::CAM_REQX_MASK | masks::CAM_REQER_MASK |
                                             masks::CAM_REQR_MASK | masks::CAM_TIMING_CTRL_MASK |
                                             masks::CAM_SCH_REQ_TYP_MASK;
constexpr uint32_t CONTROL_CAM_REQUIRED_MASK = CONTROL_ACTION_EXECUTE | masks::CAM_REQX_MASK |
                                              masks::CAM_REQER_MASK | masks::CAM_REQR_MASK;
constexpr uint32_t CONTROL_CAM_FIXED_MASK = masks::CAM_ACTION_MASK | masks::CAM_REQX_MASK |
                                           masks::CAM_REQER_MASK | masks::CAM_REQR_MASK;
constexpr uint32_t CONTROL_HEADER_ACK_MASK = 1U << 26;          // .isAck
constexpr uint32_t CONTROL_HEADER_RESERVED_BIT25_MASK = 1U << 25; // .reservedBit25
constexpr uint32_t CONTROL_HEADER_CANCELLATION_MASK = 1U << 24; // .isCancellation
constexpr uint32_t CONTROL_HEADER_FIXED_ZERO_MASK = CONTROL_HEADER_ACK_MASK |
                                                  CONTROL_HEADER_RESERVED_BIT25_MASK |
                                                  CONTROL_HEADER_CANCELLATION_MASK;
constexpr size_t CONTROL_MIN_PACKET_WORDS = 32U;
constexpr size_t CONTROL_MAX_PACKET_WORDS = masks::PKT_SIZE_MASK;

inline bool is_valid_control_cam(uint32_t cam) {
    return (cam & ~CONTROL_CAM_ALLOWED_MASK) == 0U &&
           (cam & CONTROL_CAM_FIXED_MASK) == CONTROL_CAM_REQUIRED_MASK;
}

inline bool is_valid_control_header(uint32_t word1) {
    return (word1 & CONTROL_HEADER_FIXED_ZERO_MASK) == 0U;
}
} // namespace control_schedule_request_detail

class ControlScheduleRequestView : public ControlScheduleRequestPacketBaseView {
private:
    const uint32_t* buffer;

public:
    inline uint32_t getWord1() const { return buffer ? ntohl(buffer[0]) : 0; }
    inline bool hasClassId() const { return (getWord1() & ams::iface::vita::masks::CBIT_MASK) != 0; }
    inline bool hasTsi() const { return ((getWord1() & ams::iface::vita::masks::TSI_MASK) >> ams::iface::vita::masks::TSI_SHIFT) != 0; }
    inline bool hasTsf() const { return ((getWord1() & ams::iface::vita::masks::TSF_MASK) >> ams::iface::vita::masks::TSF_SHIFT) != 0; }

    static inline size_t getPrologueWords() { return 9; }

    explicit ControlScheduleRequestView(std::span<const uint32_t> view_buffer) {
        if (view_buffer.size() < getPrologueWords() + 5) {
            buffer = nullptr;
            return;
        }

        uint32_t word1 = ntohl(view_buffer[0]);
        // Validate Packet Type (0111)
        if (((word1 & ams::iface::vita::masks::PKT_TYPE_MASK) >> ams::iface::vita::masks::PKT_TYPE_SHIFT) != 7U) {
            buffer = nullptr;
            return;
        }
        // Validate C-Bit
        if ((word1 & ams::iface::vita::masks::CBIT_MASK) == 0) {
            buffer = nullptr;
            return;
        }
        // Validate TSI (11)
        if (((word1 & ams::iface::vita::masks::TSI_MASK) >> ams::iface::vita::masks::TSI_SHIFT) != 3U) {
            buffer = nullptr;
            return;
        }
        // Validate TSF (10)
        if (((word1 & ams::iface::vita::masks::TSF_MASK) >> ams::iface::vita::masks::TSF_SHIFT) != 2U) {
            buffer = nullptr;
            return;
        }
        // Validate fixed-zero command header subtype/reserved bits.
        if (!control_schedule_request_detail::is_valid_control_header(word1)) {
            buffer = nullptr;
            return;
        }

        // Validate Fixed Metadata Class ID
        if (ntohl(view_buffer[2]) != 0xAAAAAA) { // OUI
            buffer = nullptr;
            return;
        }
        if (!isAmsClassIdCodes(ntohl(view_buffer[3]), 0x0AU)) { // Info & Pkt Class
            buffer = nullptr;
            return;
        }
        uint32_t pkt_size = word1 & ams::iface::vita::masks::PKT_SIZE_MASK;
        if (view_buffer.size() < pkt_size ||
            pkt_size < control_schedule_request_detail::CONTROL_MIN_PACKET_WORDS ||
            pkt_size > control_schedule_request_detail::CONTROL_MAX_PACKET_WORDS) {
            buffer = nullptr;
            return;
        }

        if (ntohl(view_buffer[getPrologueWords() - 1]) == 0U) {
            buffer = nullptr;
            return;
        }

        buffer = view_buffer.data();
        if (!control_schedule_request_detail::is_valid_control_cam(getCam())) {
            buffer = nullptr;
            return;
        }

        const uint32_t* current = buffer + getPrologueWords();
        const uint32_t* end = buffer + pkt_size;

        if (current >= end) { buffer = nullptr; return; }
        cif0 = ntohl(*current++);
        if ((cif0 & masks::CIF0_CIF1_ENABLE) == 0 ||
            (cif0 & masks::CIF0_CIF2_ENABLE) == 0 ||
            (cif0 & masks::CIF0_CIF3_ENABLE) == 0 ||
            (cif0 & masks::CIF0_CIF4_ENABLE) == 0 ||
            (cif0 & (1U << 31)) == 0) { // .cfci
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
    inline uint32_t getStreamId() const { return buffer ? ntohl(buffer[1]) : 0; }
    inline uint8_t getPacketCount() const { return buffer ? ((ntohl(buffer[0]) >> 16) & 0xF) : 0; }

    inline uint32_t getClassIdOui() const { return buffer ? ntohl(buffer[2]) : 0; }
    inline uint32_t getClassIdCodes() const { return buffer ? ntohl(buffer[3]) : 0; }
    inline uint32_t getTimestampInt() const { return buffer ? ntohl(buffer[4]) : 0; }
    inline uint32_t getTimestampFracHigh() const { return buffer ? ntohl(buffer[5]) : 0; }
    inline uint32_t getTimestampFracLow() const { return buffer ? ntohl(buffer[6]) : 0; }

    inline uint32_t getMessageId() const { return buffer ? ntohl(buffer[getPrologueWords() - 1]) : 0; }
    inline uint32_t getCam() const { return buffer ? ntohl(buffer[getPrologueWords() - 2]) : 0; }
    inline uint8_t getScheduleRequestType() const {
        return static_cast<uint8_t>((getCam() & masks::CAM_SCH_REQ_TYP_MASK) >> masks::CAM_SCH_REQ_TYP_SHIFT);
    }

    inline ams::iface::vita::Pointing3dStructureView getPointing3dStructure() const {
        return ams::iface::vita::Pointing3dStructureView(getPointing3dStructRaw());
    }

    // Expose 16-bit packed halves of 32-bit payloads as defined by AMS VITA 49.2 tailoring
    inline std::optional<uint16_t> getBeamWidthHorizontal() const {
        auto val = getBeamWidth();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>((*val >> 16) & 0xFFFF);
    }
    inline std::optional<uint16_t> getBeamWidthVertical() const {
        auto val = getBeamWidth();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>(*val & 0xFFFF);
    }

    inline std::optional<uint16_t> getGainStage2() const {
        auto val = getGain();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>((*val >> 16) & 0xFFFF);
    }
    inline std::optional<uint16_t> getGainStage1() const {
        auto val = getGain();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>(*val & 0xFFFF);
    }

    inline std::optional<uint16_t> getPolarizationTiltAngle() const {
        auto val = getPolarization();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>((*val >> 16) & 0xFFFF);
    }
    inline std::optional<uint16_t> getPolarizationEllipticity() const {
        auto val = getPolarization();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>(*val & 0xFFFF);
    }

    inline std::optional<uint16_t> getPointing3dElevation() const {
        auto val = getPointing3d();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>((*val >> 16) & 0xFFFF);
    }
    inline std::optional<uint16_t> getPointing3dAzimuth() const {
        auto val = getPointing3d();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>(*val & 0xFFFF);
    }

    inline std::optional<uint16_t> getRfFigureOfMeritLowerBound() const {
        auto val = getRfFigureOfMerit();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>((*val >> 16) & 0xFFFF);
    }
    inline std::optional<uint16_t> getRfFigureOfMeritRequested() const {
        auto val = getRfFigureOfMerit();
        if (!val) return std::nullopt;
        return static_cast<uint16_t>(*val & 0xFFFF);
    }

    inline std::optional<ams::iface::vita::DataFormat> getParsedDataFormat() const {
        return getDataFormat();
    }
};

class ControlScheduleRequestBuilder : public ControlScheduleRequestPacketBaseBuilder {
private:
    uint32_t* buffer;
    size_t max_words;

    uint32_t word1 = 0;
    uint32_t stream_id = 0;
    uint32_t class_id_oui = htonl(0xAAAAAA);
    uint32_t class_id_codes = htonl(makeAmsClassIdCodes(InfoClassCodeType::TxCommBaseSet, 0x0AU));
    uint32_t ts_int = 0;
    uint32_t ts_frac_hi = 0;
    uint32_t ts_frac_lo = 0;
    uint32_t cam = 0;
    uint32_t message_id = 0;
    uint8_t schedule_request_type = 1;


    // Local caches for 16-bit halves of split payloads
    uint16_t p_beam_horiz = 0;
    uint16_t p_beam_vert = 0;
    uint16_t p_gain_stage2 = 0;
    uint16_t p_gain_stage1 = 0;
    uint16_t p_pol_tilt = 0;
    uint16_t p_pol_ellip = 0;
    uint16_t p_p3d_elev = 0;
    uint16_t p_p3d_azim = 0;
    uint16_t p_rffom_lower = 0;
    uint16_t p_rffom_req = 0;

    // Internal buffer managed by wrapper since base class only takes a span
    std::vector<uint32_t> m_dynamic_p3d_buffer;

public:
    explicit ControlScheduleRequestBuilder(uint32_t* target_buffer, size_t max_words_in) 
        : buffer(target_buffer), max_words(max_words_in) {}

    ControlScheduleRequestBuilder(const ControlScheduleRequestBuilder&) = delete;
    ControlScheduleRequestBuilder& operator=(const ControlScheduleRequestBuilder&) = delete;
    ControlScheduleRequestBuilder(ControlScheduleRequestBuilder&&) = delete;
    ControlScheduleRequestBuilder& operator=(ControlScheduleRequestBuilder&&) = delete;

    void setStreamId(uint32_t val) { stream_id = htonl(val); }
    void setTimestampInt(uint32_t val) { ts_int = htonl(val); }
    void setTimestampFracHigh(uint32_t val) { ts_frac_hi = htonl(val); }
    void setTimestampFracLow(uint32_t val) { ts_frac_lo = htonl(val); }
    void setMessageId(uint32_t val) { message_id = htonl(val); }
    void setInfoClassType(InfoClassCodeType val) { class_id_codes = htonl(makeAmsClassIdCodes(val, 0x0AU)); }
    void setPacketCount(uint8_t val) {
        word1 &= ~(0xFU << 16);
        word1 |= ((static_cast<uint32_t>(val) & 0xF) << 16);
    }
    void setScheduleRequestType(uint8_t val) {
        if (val > 0x0FU) {
            throw std::invalid_argument("Control schedule request type must fit in 4 bits");
        }
        schedule_request_type = val;
    }

    void addPointing3dStructure(std::optional<uint32_t> global_irb, std::span<const ams::iface::vita::Pointing3dRecord> records) {
        if (records.size() > 4095) {
            throw std::invalid_argument("Pointing3dStructure cannot contain more than 4095 records");
        }
        bool has_irb = std::any_of(records.begin(), records.end(), 
                                    [](const auto& rec) { return rec.index_ref_beam.has_value(); });

        size_t n_records = records.size();
        size_t words_per_rec = has_irb ? 2 : 1;
        uint32_t header_words = global_irb.has_value() ? 4 : 3;
        uint32_t array_total_words = static_cast<uint32_t>(header_words + (n_records * words_per_rec));
        
        m_dynamic_p3d_buffer.clear();
        m_dynamic_p3d_buffer.reserve(array_total_words);
        
        m_dynamic_p3d_buffer.push_back(htonl(array_total_words));
        
        uint32_t word2 = (header_words << ams::iface::vita::masks::P3D_WORD2_HEADER_SIZE_SHIFT) |
                         (static_cast<uint32_t>(words_per_rec) << ams::iface::vita::masks::P3D_WORD2_RECORD_SIZE_SHIFT) |
                         (static_cast<uint32_t>(n_records) & ams::iface::vita::masks::P3D_WORD2_NUM_RECORDS_MASK);
        m_dynamic_p3d_buffer.push_back(htonl(word2));

        uint32_t word3 = has_irb ? ams::iface::vita::masks::P3D_WORD3_HAS_IRB_MASK : 0;
        word3 |= ams::iface::vita::masks::P3D_WORD3_HAS_VECTOR_MASK;
        m_dynamic_p3d_buffer.push_back(htonl(word3));

        if (global_irb.has_value()) {
            m_dynamic_p3d_buffer.push_back(htonl(global_irb.value()));
        }

        for (const auto& rec : records) {
            if (has_irb) {
                m_dynamic_p3d_buffer.push_back(htonl(rec.index_ref_beam.value_or(0)));
            }
            m_dynamic_p3d_buffer.push_back(htonl(rec.pointing3d));
        }
        
        addPointing3dStructRaw(std::span<const uint32_t>(m_dynamic_p3d_buffer));
    }

    void setBeamWidthHorizontal(uint16_t val) {
        p_beam_horiz = val;
        addBeamWidth((static_cast<uint32_t>(p_beam_horiz) << 16) | p_beam_vert);
    }
    void setBeamWidthVertical(uint16_t val) {
        p_beam_vert = val;
        addBeamWidth((static_cast<uint32_t>(p_beam_horiz) << 16) | p_beam_vert);
    }

    void setGainStage2(uint16_t val) {
        p_gain_stage2 = val;
        addGain((static_cast<uint32_t>(p_gain_stage2) << 16) | p_gain_stage1);
    }
    void setGainStage1(uint16_t val) {
        p_gain_stage1 = val;
        addGain((static_cast<uint32_t>(p_gain_stage2) << 16) | p_gain_stage1);
    }

    void setPolarizationTiltAngle(uint16_t val) {
        p_pol_tilt = val;
        addPolarization((static_cast<uint32_t>(p_pol_tilt) << 16) | p_pol_ellip);
    }
    void setPolarizationEllipticity(uint16_t val) {
        p_pol_ellip = val;
        addPolarization((static_cast<uint32_t>(p_pol_tilt) << 16) | p_pol_ellip);
    }

    void setPointing3dElevation(uint16_t val) {
        p_p3d_elev = val;
        addPointing3d((static_cast<uint32_t>(p_p3d_elev) << 16) | p_p3d_azim);
    }
    void setPointing3dAzimuth(uint16_t val) {
        p_p3d_azim = val;
        addPointing3d((static_cast<uint32_t>(p_p3d_elev) << 16) | p_p3d_azim);
    }

    void setRfFigureOfMeritLowerBound(uint16_t val) {
        p_rffom_lower = val;
        addRfFigureOfMerit((static_cast<uint32_t>(p_rffom_lower) << 16) | p_rffom_req);
    }
    void setRfFigureOfMeritRequested(uint16_t val) {
        p_rffom_req = val;
        addRfFigureOfMerit((static_cast<uint32_t>(p_rffom_lower) << 16) | p_rffom_req);
    }

    void setDataFormat(ams::iface::vita::DataFormat format) {
        setDataFormatRaw(format);
    }

    size_t finalize() {
        if (message_id == 0U) return 0;

        cif0 |= masks::CIF0_CIF1_ENABLE;
        cif0 |= masks::CIF0_CIF2_ENABLE;
        cif0 |= masks::CIF0_CIF3_ENABLE;
        cif0 |= masks::CIF0_CIF4_ENABLE;
        cif0 |= (1U << 31); // .cfci

        const size_t write_limit = std::min(max_words, control_schedule_request_detail::CONTROL_MAX_PACKET_WORDS);

        size_t offset = 1;
        if (offset + 8 + 5 > write_limit) return 0; // 8 prologue + 5 CIFs

        buffer[offset++] = stream_id;
        buffer[offset++] = class_id_oui;
        buffer[offset++] = class_id_codes;
        buffer[offset++] = ts_int;
        buffer[offset++] = ts_frac_hi;
        buffer[offset++] = ts_frac_lo;
        buffer[offset++] = cam;
        buffer[offset++] = message_id;

        buffer[offset++] = htonl(cif0);
        if (cif0 & masks::CIF0_CIF1_ENABLE) buffer[offset++] = htonl(cif1);
        if (cif0 & masks::CIF0_CIF2_ENABLE) buffer[offset++] = htonl(cif2);
        if (cif0 & masks::CIF0_CIF3_ENABLE) buffer[offset++] = htonl(cif3);
        if (cif0 & masks::CIF0_CIF4_ENABLE) buffer[offset++] = htonl(cif4);

        offset = writeGeneratedFields(buffer, offset, write_limit);
        if (offset == 0 ||
            offset < control_schedule_request_detail::CONTROL_MIN_PACKET_WORDS ||
            offset > control_schedule_request_detail::CONTROL_MAX_PACKET_WORDS) return 0;

        cam = htonl((2U << masks::CAM_ACTION_SHIFT) | masks::CAM_REQX_MASK | masks::CAM_REQER_MASK |
                   masks::CAM_REQR_MASK |
                   (static_cast<uint32_t>(schedule_request_type) << masks::CAM_SCH_REQ_TYP_SHIFT));
        buffer[7] = cam;

        // Force ams_vita_49-2_tailoring word1 prologue constraints
        word1 &= ~ams::iface::vita::masks::PKT_TYPE_MASK;
        word1 |= (7U << ams::iface::vita::masks::PKT_TYPE_SHIFT); // 0111
        word1 |= ams::iface::vita::masks::CBIT_MASK; // 1
        word1 &= ~(1U << 26); // .isAck = 0
        word1 &= ~(1U << 25); // .reservedBit25 = 0
        word1 &= ~(1U << 24); // .isCancellation = 0
        word1 |= ams::iface::vita::masks::TSI_MASK; // 11
        word1 &= ~ams::iface::vita::masks::TSF_MASK;
        word1 |= (2 << ams::iface::vita::masks::TSF_SHIFT); // 10
        word1 = (word1 & ~ams::iface::vita::masks::PKT_SIZE_MASK) | (offset & ams::iface::vita::masks::PKT_SIZE_MASK);
        buffer[0] = htonl(word1);

        return offset;
    }
};


} // namespace ams::iface::vita

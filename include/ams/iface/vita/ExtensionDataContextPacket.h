#pragma once

#include "ams/iface/vita/Primitives.h"
#include "ams/iface/vita/ExtensionDataContextPacketBase.h"
#include <cstdint>
#include <cstddef>
#include <optional>
#include <algorithm>
#include <arpa/inet.h>
#include <span>
#include <vector>

namespace ams::iface::vita {

namespace extension_data_context_detail {
constexpr uint32_t HEADER_FIXED_ZERO_MASK = (1U << 26) | (1U << 25) | (1U << 24);
} // namespace extension_data_context_detail

class ExtensionDataContextView : public ExtensionDataContextPacketBaseView {
private:
    const uint32_t* buffer;

public:
    inline bool isValid() const { return buffer != nullptr; }
    inline uint32_t getWord1() const { return buffer ? ntohl(buffer[0]) : 0; }
    inline bool hasClassId() const { return (getWord1() & ams::iface::vita::masks::CBIT_MASK) != 0; }
    inline bool hasTsi() const { return ((getWord1() & ams::iface::vita::masks::TSI_MASK) >> ams::iface::vita::masks::TSI_SHIFT) != 0; }
    inline bool hasTsf() const { return ((getWord1() & ams::iface::vita::masks::TSF_MASK) >> ams::iface::vita::masks::TSF_SHIFT) != 0; }

    static inline size_t getPrologueWords() { return 7; }

    explicit ExtensionDataContextView(std::span<const uint32_t> view_buffer) {
        if (view_buffer.size() < getPrologueWords() + 5) {
            buffer = nullptr;
            return;
        }

        uint32_t word1 = ntohl(view_buffer[0]);
        // Validate Packet Type (0101)
        if (((word1 & ams::iface::vita::masks::PKT_TYPE_MASK) >> ams::iface::vita::masks::PKT_TYPE_SHIFT) != 5U) {
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
        // AMS extension context packets fix reservedBit26, notV49p0Packet, and timeStampMode to zero.
        if ((word1 & extension_data_context_detail::HEADER_FIXED_ZERO_MASK) != 0U) {
            buffer = nullptr;
            return;
        }

        // Validate Fixed Metadata Class ID
        if (ntohl(view_buffer[2]) != 0xAAAAAA) { // OUI
            buffer = nullptr;
            return;
        }
        if (!isAmsClassIdCodes(ntohl(view_buffer[3]), 0x0EU)) { // Info & Pkt Class
            buffer = nullptr;
            return;
        }

        uint32_t pkt_size = word1 & ams::iface::vita::masks::PKT_SIZE_MASK;
        if (view_buffer.size() < pkt_size || pkt_size < getPrologueWords() + 5) {
            buffer = nullptr;
            return;
        }

        buffer = view_buffer.data();
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

    inline uint32_t getStreamId() const { return buffer ? ntohl(buffer[1]) : 0; }
    inline uint32_t getClassIdOui() const { return buffer ? ntohl(buffer[2]) : 0; }
    inline uint32_t getClassIdCodes() const { return buffer ? ntohl(buffer[3]) : 0; }
    inline uint32_t getTimestampInt() const { return buffer ? ntohl(buffer[4]) : 0; }
    inline uint32_t getTimestampFracHigh() const { return buffer ? ntohl(buffer[5]) : 0; }
    inline uint32_t getTimestampFracLow() const { return buffer ? ntohl(buffer[6]) : 0; }
    inline uint8_t getPacketCount() const { return buffer ? ((ntohl(buffer[0]) >> 16) & 0xF) : 0; }
    
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

class ExtensionDataContextBuilder : public ExtensionDataContextPacketBaseBuilder {
private:
    uint32_t* buffer;
    size_t max_words;

    uint32_t word1 = 0;
    uint32_t stream_id = 0;
    uint32_t class_id_oui = htonl(0xAAAAAA);
    uint32_t class_id_codes = htonl(makeAmsClassIdCodes(InfoClassCodeType::RxCommBaseSet, 0x0EU));
    uint32_t ts_int = 0;
    uint32_t ts_frac_hi = 0;
    uint32_t ts_frac_lo = 0;

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

public:
    explicit ExtensionDataContextBuilder(uint32_t* target_buffer, size_t max_words_in) 
        : buffer(target_buffer), max_words(max_words_in) {}

    void setStreamId(uint32_t val) { stream_id = htonl(val); }
    void setTimestampInt(uint32_t val) { ts_int = htonl(val); }
    void setTimestampFracHigh(uint32_t val) { ts_frac_hi = htonl(val); }
    void setTimestampFracLow(uint32_t val) { ts_frac_lo = htonl(val); }
    void setInfoClassType(InfoClassCodeType val) { class_id_codes = htonl(makeAmsClassIdCodes(val, 0x0EU)); }
    void setPacketCount(uint8_t val) {
        word1 &= ~(0xFU << 16);
        word1 |= ((static_cast<uint32_t>(val) & 0xF) << 16);
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
        cif0 |= masks::CIF0_CIF1_ENABLE;
        cif0 |= masks::CIF0_CIF2_ENABLE;
        cif0 |= masks::CIF0_CIF3_ENABLE;
        cif0 |= masks::CIF0_CIF4_ENABLE;
        cif0 |= (1U << 31); // .cfci

        size_t offset = 1;
        if (offset + 6 + 5 > max_words) return 0; // 6 remaining prologue + 5 CIFs
        
        buffer[offset++] = stream_id;
        buffer[offset++] = class_id_oui;
        buffer[offset++] = class_id_codes;
        buffer[offset++] = ts_int;
        buffer[offset++] = ts_frac_hi;
        buffer[offset++] = ts_frac_lo;

        buffer[offset++] = htonl(cif0);
        if (cif0 & masks::CIF0_CIF1_ENABLE) buffer[offset++] = htonl(cif1);
        if (cif0 & masks::CIF0_CIF2_ENABLE) buffer[offset++] = htonl(cif2);
        if (cif0 & masks::CIF0_CIF3_ENABLE) buffer[offset++] = htonl(cif3);
        if (cif0 & masks::CIF0_CIF4_ENABLE) buffer[offset++] = htonl(cif4);

        offset = writeGeneratedFields(buffer, offset, max_words);
        if (offset == 0 || offset > 65535U) return 0;



        // Force ams_vita_49-2_tailoring word1 prologue constraints for Extension Context
        word1 &= ~ams::iface::vita::masks::PKT_TYPE_MASK;
        word1 |= (5U << ams::iface::vita::masks::PKT_TYPE_SHIFT); // 0101
        word1 |= ams::iface::vita::masks::CBIT_MASK; // 1
        word1 &= ~(1U << 26); // .reservedBit26 = 0
        word1 &= ~(1U << 25); // .notV49p0Packet = 0
        word1 &= ~(1U << 24); // .timeStampMode = 0
        word1 |= ams::iface::vita::masks::TSI_MASK; // 11
        word1 &= ~ams::iface::vita::masks::TSF_MASK;
        word1 |= (2 << ams::iface::vita::masks::TSF_SHIFT); // 10
        word1 = (word1 & ~ams::iface::vita::masks::PKT_SIZE_MASK) | (offset & ams::iface::vita::masks::PKT_SIZE_MASK);
        buffer[0] = htonl(word1);

        return offset;
    }
};


} // namespace ams::iface::vita

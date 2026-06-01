#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <arpa/inet.h>
#include <span>
#include <array>
#include "ams/iface/vita/GeneratedMasks.h"
#include "ams/iface/vita/Primitives.h"

namespace ams::iface::vita {

class ControlScheduleRequestPacketBaseView {
protected:
    uint32_t cif0 = 0, cif1 = 0, cif2 = 0, cif3 = 0, cif4 = 0, cif7 = 0;
    const uint32_t* ptr_Bandwidth_hi = nullptr;
    const uint32_t* ptr_Bandwidth_lo = nullptr;
    const uint32_t* ptr_RfRefFreq_hi = nullptr;
    const uint32_t* ptr_RfRefFreq_lo = nullptr;
    const uint32_t* ptr_Gain = nullptr;
    const uint32_t* ptr_SampleRate_hi = nullptr;
    const uint32_t* ptr_SampleRate_lo = nullptr;
    const uint32_t* ptr_DataFormat_hi = nullptr;
    const uint32_t* ptr_DataFormat_lo = nullptr;
    const uint32_t* ptr_Polarization = nullptr;
    const uint32_t* ptr_Pointing3d = nullptr;
    const uint32_t* ptr_Pointing3dStruct = nullptr;
    size_t size_Pointing3dStruct = 0;
    const uint32_t* ptr_BeamWidth = nullptr;
    const uint32_t* ptr_CitedMsgId = nullptr;
    const uint32_t* ptr_InfoSource = nullptr;
    const uint32_t* ptr_ModeId = nullptr;
    const uint32_t* ptr_FuncPriorityId = nullptr;
    const uint32_t* ptr_Dwell_hi = nullptr;
    const uint32_t* ptr_Dwell_lo = nullptr;
    const uint32_t* ptr_Jitter_hi = nullptr;
    const uint32_t* ptr_Jitter_lo = nullptr;
    const uint32_t* ptr_RfFigureOfMerit = nullptr;
    const uint32_t* ptr_AddressGroupIndex = nullptr;
    const uint32_t* ptr_TxDigitalInputPower = nullptr;
    const uint32_t* mapGeneratedFields(const uint32_t* current, [[maybe_unused]] const uint32_t* end) {
        // CIF0 Mapping
        if ((cif0 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF0_N_MASK) != 0) return nullptr;
        if ((cif0 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF0_B_MASK) != masks::CONTROLSCHEDULEREQUESTPACKET_CIF0_B_MASK) return nullptr;
        if (cif0 & masks::CIF0_BANDWIDTH_MASK) { if (end - current < 2) return nullptr; ptr_Bandwidth_hi = current++; ptr_Bandwidth_lo = current++; }
        if (cif0 & masks::CIF0_RFREFFREQ_MASK) { if (end - current < 2) return nullptr; ptr_RfRefFreq_hi = current++; ptr_RfRefFreq_lo = current++; }
        if (cif0 & masks::CIF0_GAIN_MASK) { if (current >= end) return nullptr; ptr_Gain = current++; }
        if (cif0 & masks::CIF0_SAMPLERATE_MASK) { if (end - current < 2) return nullptr; ptr_SampleRate_hi = current++; ptr_SampleRate_lo = current++; }
        if (cif0 & masks::CIF0_DATAFORMAT_MASK) {
            if (end - current < 2) return nullptr;
            ptr_DataFormat_hi = current++; ptr_DataFormat_lo = current++;
            const uint64_t data_format = (static_cast<uint64_t>(ntohl(*ptr_DataFormat_hi)) << 32) | ntohl(*ptr_DataFormat_lo);
            if (!isAmsDataFormat(data_format)) return nullptr;
        }
        // CIF1 Mapping
        if ((cif1 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF1_N_MASK) != 0) return nullptr;
        constexpr uint32_t required_without_pointing3d = masks::CONTROLSCHEDULEREQUESTPACKET_CIF1_B_MASK & ~masks::CIF1_POINTING3D_MASK;
        if ((cif1 & required_without_pointing3d) != required_without_pointing3d) return nullptr;
        const bool has_pointing3d = (cif1 & masks::CIF1_POINTING3D_MASK) != 0;
        const bool has_pointing3d_struct = (cif1 & masks::CIF1_POINTING3DSTRUCT_MASK) != 0;
        if (has_pointing3d == has_pointing3d_struct) return nullptr;
        if (cif1 & masks::CIF1_POLARIZATION_MASK) { if (current >= end) return nullptr; ptr_Polarization = current++; }
        if (cif1 & masks::CIF1_POINTING3D_MASK) { if (current >= end) return nullptr; ptr_Pointing3d = current++; }
        if (cif1 & masks::CIF1_POINTING3DSTRUCT_MASK) {
            if (current >= end) return nullptr;
            uint32_t total_words = ntohl(*current);
            size_t remaining = static_cast<size_t>(std::distance(current, end));
            if (total_words == 0 || remaining < total_words) return nullptr;
            if (!Pointing3dStructureView(std::span<const uint32_t>(current, total_words)).isValid()) return nullptr;
            ptr_Pointing3dStruct = current;
            size_Pointing3dStruct = total_words;
            current += total_words;
        }
        if (cif1 & masks::CIF1_BEAMWIDTH_MASK) { if (current >= end) return nullptr; ptr_BeamWidth = current++; }
        // CIF2 Mapping
        if ((cif2 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF2_N_MASK) != 0) return nullptr;
        if ((cif2 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF2_B_MASK) != masks::CONTROLSCHEDULEREQUESTPACKET_CIF2_B_MASK) return nullptr;
        if (cif2 & masks::CIF2_CITEDMSGID_MASK) { if (current >= end) return nullptr; ptr_CitedMsgId = current++; }
        if (cif2 & masks::CIF2_INFOSOURCE_MASK) { if (current >= end) return nullptr; ptr_InfoSource = current++; }
        if (cif2 & masks::CIF2_MODEID_MASK) { if (current >= end) return nullptr; ptr_ModeId = current++; }
        if (cif2 & masks::CIF2_FUNCPRIORITYID_MASK) { if (current >= end) return nullptr; ptr_FuncPriorityId = current++; }
        // CIF3 Mapping
        if ((cif3 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF3_N_MASK) != 0) return nullptr;
        if ((cif3 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF3_B_MASK) != masks::CONTROLSCHEDULEREQUESTPACKET_CIF3_B_MASK) return nullptr;
        if (cif3 & masks::CIF3_DWELL_MASK) { if (end - current < 2) return nullptr; ptr_Dwell_hi = current++; ptr_Dwell_lo = current++; }
        if (cif3 & masks::CIF3_JITTER_MASK) { if (end - current < 2) return nullptr; ptr_Jitter_hi = current++; ptr_Jitter_lo = current++; }
        // CIF4 Mapping
        if ((cif4 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF4_N_MASK) != 0) return nullptr;
        if ((cif4 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF4_B_MASK) != masks::CONTROLSCHEDULEREQUESTPACKET_CIF4_B_MASK) return nullptr;
        if (cif4 & masks::CIF4_RFFIGUREOFMERIT_MASK) { if (current >= end) return nullptr; ptr_RfFigureOfMerit = current++; }
        if (cif4 & masks::CIF4_ADDRESSGROUPINDEX_MASK) { if (current >= end) return nullptr; ptr_AddressGroupIndex = current++; }
        if (cif4 & masks::CIF4_TXDIGITALINPUTPOWER_MASK) { if (current >= end) return nullptr; ptr_TxDigitalInputPower = current++; }
        // CIF7 Mapping
        if ((cif7 & masks::CONTROLSCHEDULEREQUESTPACKET_CIF7_N_MASK) != 0) return nullptr;
        return current;
    }

public:
    inline std::optional<uint64_t> getBandwidth() const {
        if (!ptr_Bandwidth_hi || !ptr_Bandwidth_lo) return std::nullopt;
        return (static_cast<uint64_t>(ntohl(*ptr_Bandwidth_hi)) << 32) | ntohl(*ptr_Bandwidth_lo);
    }
    inline std::optional<uint64_t> getRfRefFreq() const {
        if (!ptr_RfRefFreq_hi || !ptr_RfRefFreq_lo) return std::nullopt;
        return (static_cast<uint64_t>(ntohl(*ptr_RfRefFreq_hi)) << 32) | ntohl(*ptr_RfRefFreq_lo);
    }
    inline std::optional<uint32_t> getGain() const {
        if (!ptr_Gain) return std::nullopt;
        return ntohl(*ptr_Gain);
    }
    inline std::optional<uint64_t> getSampleRate() const {
        if (!ptr_SampleRate_hi || !ptr_SampleRate_lo) return std::nullopt;
        return (static_cast<uint64_t>(ntohl(*ptr_SampleRate_hi)) << 32) | ntohl(*ptr_SampleRate_lo);
    }
    inline std::optional<DataFormat> getDataFormat() const {
        if (!ptr_DataFormat_hi || !ptr_DataFormat_lo) return std::nullopt;
        return parseAmsDataFormat((static_cast<uint64_t>(ntohl(*ptr_DataFormat_hi)) << 32) | ntohl(*ptr_DataFormat_lo));
    }
    inline std::optional<uint32_t> getPolarization() const {
        if (!ptr_Polarization) return std::nullopt;
        return ntohl(*ptr_Polarization);
    }
    inline std::optional<uint32_t> getPointing3d() const {
        if (!ptr_Pointing3d) return std::nullopt;
        return ntohl(*ptr_Pointing3d);
    }
    inline std::span<const uint32_t> getPointing3dStructRaw() const {
        if (!ptr_Pointing3dStruct) return {};
        return {ptr_Pointing3dStruct, size_Pointing3dStruct};
    }
    inline std::optional<uint32_t> getBeamWidth() const {
        if (!ptr_BeamWidth) return std::nullopt;
        return ntohl(*ptr_BeamWidth);
    }
    inline std::optional<uint32_t> getCitedMsgId() const {
        if (!ptr_CitedMsgId) return std::nullopt;
        return ntohl(*ptr_CitedMsgId);
    }
    inline std::optional<uint32_t> getInfoSource() const {
        if (!ptr_InfoSource) return std::nullopt;
        return ntohl(*ptr_InfoSource);
    }
    inline std::optional<uint32_t> getModeId() const {
        if (!ptr_ModeId) return std::nullopt;
        return ntohl(*ptr_ModeId);
    }
    inline std::optional<uint32_t> getFuncPriorityId() const {
        if (!ptr_FuncPriorityId) return std::nullopt;
        return ntohl(*ptr_FuncPriorityId);
    }
    inline std::optional<uint64_t> getDwell() const {
        if (!ptr_Dwell_hi || !ptr_Dwell_lo) return std::nullopt;
        return (static_cast<uint64_t>(ntohl(*ptr_Dwell_hi)) << 32) | ntohl(*ptr_Dwell_lo);
    }
    inline std::optional<uint64_t> getJitter() const {
        if (!ptr_Jitter_hi || !ptr_Jitter_lo) return std::nullopt;
        return (static_cast<uint64_t>(ntohl(*ptr_Jitter_hi)) << 32) | ntohl(*ptr_Jitter_lo);
    }
    inline std::optional<uint32_t> getRfFigureOfMerit() const {
        if (!ptr_RfFigureOfMerit) return std::nullopt;
        return ntohl(*ptr_RfFigureOfMerit);
    }
    inline std::optional<uint32_t> getAddressGroupIndex() const {
        if (!ptr_AddressGroupIndex) return std::nullopt;
        return ntohl(*ptr_AddressGroupIndex);
    }
    inline std::optional<uint32_t> getTxDigitalInputPower() const {
        if (!ptr_TxDigitalInputPower) return std::nullopt;
        return ntohl(*ptr_TxDigitalInputPower);
    }
};

class ControlScheduleRequestPacketBaseBuilder {
protected:
    uint32_t cif0 = masks::CONTROLSCHEDULEREQUESTPACKET_CIF0_B_MASK;
    uint32_t cif1 = masks::CONTROLSCHEDULEREQUESTPACKET_CIF1_B_MASK;
    uint32_t cif2 = masks::CONTROLSCHEDULEREQUESTPACKET_CIF2_B_MASK;
    uint32_t cif3 = masks::CONTROLSCHEDULEREQUESTPACKET_CIF3_B_MASK;
    uint32_t cif4 = masks::CONTROLSCHEDULEREQUESTPACKET_CIF4_B_MASK;
    uint32_t cif7 = masks::CONTROLSCHEDULEREQUESTPACKET_CIF7_B_MASK;
    uint32_t p_Bandwidth_hi = 0, p_Bandwidth_lo = 0;
    uint32_t p_RfRefFreq_hi = 0, p_RfRefFreq_lo = 0;
    uint32_t p_Gain = 0;
    uint32_t p_SampleRate_hi = 0, p_SampleRate_lo = 0;
    uint32_t p_DataFormat_hi = 0, p_DataFormat_lo = 0;
    bool p_DataFormat_set = false;
    uint32_t p_Polarization = 0;
    uint32_t p_Pointing3d = 0;
    std::span<const uint32_t> p_Pointing3dStruct;
    uint32_t p_BeamWidth = 0;
    uint32_t p_CitedMsgId = 0;
    uint32_t p_InfoSource = 0;
    uint32_t p_ModeId = 0;
    uint32_t p_FuncPriorityId = 0;
    uint32_t p_Dwell_hi = 0, p_Dwell_lo = 0;
    uint32_t p_Jitter_hi = 0, p_Jitter_lo = 0;
    uint32_t p_RfFigureOfMerit = 0;
    uint32_t p_AddressGroupIndex = 0;
    uint32_t p_TxDigitalInputPower = 0;
    size_t writeGeneratedFields([[maybe_unused]] uint32_t* buffer, size_t offset, [[maybe_unused]] size_t max_words) const {
        // CIF0 Mapping
        if (cif0 & masks::CIF0_BANDWIDTH_MASK) { if (offset + 2 > max_words) return 0; buffer[offset++] = p_Bandwidth_hi; buffer[offset++] = p_Bandwidth_lo; }
        if (cif0 & masks::CIF0_RFREFFREQ_MASK) { if (offset + 2 > max_words) return 0; buffer[offset++] = p_RfRefFreq_hi; buffer[offset++] = p_RfRefFreq_lo; }
        if (cif0 & masks::CIF0_GAIN_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_Gain; }
        if (cif0 & masks::CIF0_SAMPLERATE_MASK) { if (offset + 2 > max_words) return 0; buffer[offset++] = p_SampleRate_hi; buffer[offset++] = p_SampleRate_lo; }
        if (cif0 & masks::CIF0_DATAFORMAT_MASK) { if (!p_DataFormat_set || offset + 2 > max_words) return 0; buffer[offset++] = p_DataFormat_hi; buffer[offset++] = p_DataFormat_lo; }
        // CIF1 Mapping
        if (cif1 & masks::CIF1_POLARIZATION_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_Polarization; }
        if (cif1 & masks::CIF1_POINTING3D_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_Pointing3d; }
        if (cif1 & masks::CIF1_POINTING3DSTRUCT_MASK) {
            if (offset > max_words || p_Pointing3dStruct.size() > max_words - offset) return 0;
            for (uint32_t word : p_Pointing3dStruct) buffer[offset++] = word;
        }
        if (cif1 & masks::CIF1_BEAMWIDTH_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_BeamWidth; }
        // CIF2 Mapping
        if (cif2 & masks::CIF2_CITEDMSGID_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_CitedMsgId; }
        if (cif2 & masks::CIF2_INFOSOURCE_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_InfoSource; }
        if (cif2 & masks::CIF2_MODEID_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_ModeId; }
        if (cif2 & masks::CIF2_FUNCPRIORITYID_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_FuncPriorityId; }
        // CIF3 Mapping
        if (cif3 & masks::CIF3_DWELL_MASK) { if (offset + 2 > max_words) return 0; buffer[offset++] = p_Dwell_hi; buffer[offset++] = p_Dwell_lo; }
        if (cif3 & masks::CIF3_JITTER_MASK) { if (offset + 2 > max_words) return 0; buffer[offset++] = p_Jitter_hi; buffer[offset++] = p_Jitter_lo; }
        // CIF4 Mapping
        if (cif4 & masks::CIF4_RFFIGUREOFMERIT_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_RfFigureOfMerit; }
        if (cif4 & masks::CIF4_ADDRESSGROUPINDEX_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_AddressGroupIndex; }
        if (cif4 & masks::CIF4_TXDIGITALINPUTPOWER_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_TxDigitalInputPower; }
        // CIF7 Mapping
        return offset;
    }
    void addPointing3dStructRaw(std::span<const uint32_t> val) {
        if (val.empty()) {
            throw std::invalid_argument("Pointing3dStruct dynamic span cannot be empty");
        }
        if (!Pointing3dStructureView(val).isValid()) throw std::invalid_argument("Pointing3dStruct dynamic span is invalid");
        cif1 &= ~masks::CIF1_POINTING3D_MASK;
        cif1 |= masks::CIF1_POINTING3DSTRUCT_MASK;
        p_Pointing3dStruct = val;
    }
    void setDataFormatRaw(DataFormat val) {
        cif0 |= masks::CIF0_DATAFORMAT_MASK;
        const uint64_t raw = static_cast<uint64_t>(val);
        p_DataFormat_hi = htonl(static_cast<uint32_t>(raw >> 32));
        p_DataFormat_lo = htonl(static_cast<uint32_t>(raw & 0xFFFFFFFF));
        p_DataFormat_set = true;
    }

public:
    void addBandwidth(uint64_t val) {
        cif0 |= masks::CIF0_BANDWIDTH_MASK;
        p_Bandwidth_hi = htonl(static_cast<uint32_t>(val >> 32));
        p_Bandwidth_lo = htonl(static_cast<uint32_t>(val & 0xFFFFFFFF));
    }
    void addRfRefFreq(uint64_t val) {
        cif0 |= masks::CIF0_RFREFFREQ_MASK;
        p_RfRefFreq_hi = htonl(static_cast<uint32_t>(val >> 32));
        p_RfRefFreq_lo = htonl(static_cast<uint32_t>(val & 0xFFFFFFFF));
    }
    void addGain(uint32_t val) {
        cif0 |= masks::CIF0_GAIN_MASK;
        p_Gain = htonl(val);
    }
    void addSampleRate(uint64_t val) {
        cif0 |= masks::CIF0_SAMPLERATE_MASK;
        p_SampleRate_hi = htonl(static_cast<uint32_t>(val >> 32));
        p_SampleRate_lo = htonl(static_cast<uint32_t>(val & 0xFFFFFFFF));
    }
    void addPolarization(uint32_t val) {
        cif1 |= masks::CIF1_POLARIZATION_MASK;
        p_Polarization = htonl(val);
    }
    void addPointing3d(uint32_t val) {
        cif1 &= ~masks::CIF1_POINTING3DSTRUCT_MASK;
        p_Pointing3dStruct = {};
        cif1 |= masks::CIF1_POINTING3D_MASK;
        p_Pointing3d = htonl(val);
    }
    void addBeamWidth(uint32_t val) {
        cif1 |= masks::CIF1_BEAMWIDTH_MASK;
        p_BeamWidth = htonl(val);
    }
    void addCitedMsgId(uint32_t val) {
        cif2 |= masks::CIF2_CITEDMSGID_MASK;
        p_CitedMsgId = htonl(val);
    }
    void addInfoSource(uint32_t val) {
        cif2 |= masks::CIF2_INFOSOURCE_MASK;
        p_InfoSource = htonl(val);
    }
    void addModeId(uint32_t val) {
        cif2 |= masks::CIF2_MODEID_MASK;
        p_ModeId = htonl(val);
    }
    void addFuncPriorityId(uint32_t val) {
        cif2 |= masks::CIF2_FUNCPRIORITYID_MASK;
        p_FuncPriorityId = htonl(val);
    }
    void addDwell(uint64_t val) {
        cif3 |= masks::CIF3_DWELL_MASK;
        p_Dwell_hi = htonl(static_cast<uint32_t>(val >> 32));
        p_Dwell_lo = htonl(static_cast<uint32_t>(val & 0xFFFFFFFF));
    }
    void addJitter(uint64_t val) {
        cif3 |= masks::CIF3_JITTER_MASK;
        p_Jitter_hi = htonl(static_cast<uint32_t>(val >> 32));
        p_Jitter_lo = htonl(static_cast<uint32_t>(val & 0xFFFFFFFF));
    }
    void addRfFigureOfMerit(uint32_t val) {
        cif4 |= masks::CIF4_RFFIGUREOFMERIT_MASK;
        p_RfFigureOfMerit = htonl(val);
    }
    void addAddressGroupIndex(uint32_t val) {
        cif4 |= masks::CIF4_ADDRESSGROUPINDEX_MASK;
        p_AddressGroupIndex = htonl(val);
    }
    void addTxDigitalInputPower(uint32_t val) {
        cif4 |= masks::CIF4_TXDIGITALINPUTPOWER_MASK;
        p_TxDigitalInputPower = htonl(val);
    }
};

} // namespace ams::iface::vita

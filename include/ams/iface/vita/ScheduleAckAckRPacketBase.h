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

class ScheduleAckAckRPacketBaseView {
protected:
    uint32_t cif0 = 0, cif1 = 0, cif2 = 0, cif3 = 0, cif4 = 0, cif7 = 0;
    const uint32_t* ptr_CitedSid = nullptr;
    const uint32_t* ptr_RejectReason = nullptr;
    const uint32_t* ptr_AddressGroupIndex = nullptr;
    const uint32_t* mapGeneratedFields(const uint32_t* current, [[maybe_unused]] const uint32_t* end) {
        // CIF0 Mapping
        if ((cif0 & masks::SCHEDULEACKACKRPACKET_CIF0_N_MASK) != 0) return nullptr;
        if ((cif0 & masks::SCHEDULEACKACKRPACKET_CIF0_B_MASK) != masks::SCHEDULEACKACKRPACKET_CIF0_B_MASK) return nullptr;
        // CIF1 Mapping
        if ((cif1 & masks::SCHEDULEACKACKRPACKET_CIF1_N_MASK) != 0) return nullptr;
        // CIF2 Mapping
        if ((cif2 & masks::SCHEDULEACKACKRPACKET_CIF2_N_MASK) != 0) return nullptr;
        if ((cif2 & masks::SCHEDULEACKACKRPACKET_CIF2_B_MASK) != masks::SCHEDULEACKACKRPACKET_CIF2_B_MASK) return nullptr;
        if (cif2 & masks::CIF2_CITEDSID_MASK) { if (current >= end) return nullptr; ptr_CitedSid = current++; }
        // CIF3 Mapping
        if ((cif3 & masks::SCHEDULEACKACKRPACKET_CIF3_N_MASK) != 0) return nullptr;
        // CIF4 Mapping
        if ((cif4 & masks::SCHEDULEACKACKRPACKET_CIF4_N_MASK) != 0) return nullptr;
        if ((cif4 & masks::SCHEDULEACKACKRPACKET_CIF4_B_MASK) != masks::SCHEDULEACKACKRPACKET_CIF4_B_MASK) return nullptr;
        if (cif4 & masks::CIF4_REJECTREASON_MASK) { if (current >= end) return nullptr; ptr_RejectReason = current++; }
        if (cif4 & masks::CIF4_ADDRESSGROUPINDEX_MASK) { if (current >= end) return nullptr; ptr_AddressGroupIndex = current++; }
        // CIF7 Mapping
        if ((cif7 & masks::SCHEDULEACKACKRPACKET_CIF7_N_MASK) != 0) return nullptr;
        return current;
    }

public:
    inline std::optional<uint32_t> getCitedSid() const {
        if (!ptr_CitedSid) return std::nullopt;
        return ntohl(*ptr_CitedSid);
    }
    inline std::optional<uint32_t> getRejectReason() const {
        if (!ptr_RejectReason) return std::nullopt;
        return ntohl(*ptr_RejectReason);
    }
    inline std::optional<uint32_t> getAddressGroupIndex() const {
        if (!ptr_AddressGroupIndex) return std::nullopt;
        return ntohl(*ptr_AddressGroupIndex);
    }
};

class ScheduleAckAckRPacketBaseBuilder {
protected:
    uint32_t cif0 = masks::SCHEDULEACKACKRPACKET_CIF0_B_MASK;
    uint32_t cif1 = masks::SCHEDULEACKACKRPACKET_CIF1_B_MASK;
    uint32_t cif2 = masks::SCHEDULEACKACKRPACKET_CIF2_B_MASK;
    uint32_t cif3 = masks::SCHEDULEACKACKRPACKET_CIF3_B_MASK;
    uint32_t cif4 = masks::SCHEDULEACKACKRPACKET_CIF4_B_MASK;
    uint32_t cif7 = masks::SCHEDULEACKACKRPACKET_CIF7_B_MASK;
    uint32_t p_CitedSid = 0;
    uint32_t p_RejectReason = 0;
    uint32_t p_AddressGroupIndex = 0;
    size_t writeGeneratedFields([[maybe_unused]] uint32_t* buffer, size_t offset, [[maybe_unused]] size_t max_words) const {
        // CIF0 Mapping
        // CIF1 Mapping
        // CIF2 Mapping
        if (cif2 & masks::CIF2_CITEDSID_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_CitedSid; }
        // CIF3 Mapping
        // CIF4 Mapping
        if (cif4 & masks::CIF4_REJECTREASON_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_RejectReason; }
        if (cif4 & masks::CIF4_ADDRESSGROUPINDEX_MASK) { if (offset + 1 > max_words) return 0; buffer[offset++] = p_AddressGroupIndex; }
        // CIF7 Mapping
        return offset;
    }

public:
    void addCitedSid(uint32_t val) {
        cif2 |= masks::CIF2_CITEDSID_MASK;
        p_CitedSid = htonl(val);
    }
    void addRejectReason(uint32_t val) {
        cif4 |= masks::CIF4_REJECTREASON_MASK;
        p_RejectReason = htonl(val);
    }
    void addAddressGroupIndex(uint32_t val) {
        cif4 |= masks::CIF4_ADDRESSGROUPINDEX_MASK;
        p_AddressGroupIndex = htonl(val);
    }
};

} // namespace ams::iface::vita

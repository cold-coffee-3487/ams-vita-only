#include "ams/iface/vita/AckRPacket.h"
#include "ams/iface/vita/ControlScheduleRequestPacket.h"
#include "ams/iface/vita/ExtensionDataContextPacket.h"

#include <arpa/inet.h>

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <span>
#include <vector>

using namespace ams::iface::vita;

namespace {

constexpr uint32_t bit_mask(unsigned bit) {
    return 1U << bit;
}

std::vector<uint32_t> packet_words(const std::array<uint32_t, 1024>& buffer, size_t written) {
    return std::vector<uint32_t>(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(written));
}

void set_packet_size(std::vector<uint32_t>& packet) {
    uint32_t word1 = ntohl(packet[0]);
    word1 &= ~masks::PKT_SIZE_MASK;
    word1 |= static_cast<uint32_t>(packet.size()) & masks::PKT_SIZE_MASK;
    packet[0] = htonl(word1);
}

std::vector<uint32_t> make_control_packet() {
    std::array<uint32_t, 1024> buffer{};
    ControlScheduleRequestBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x10101010U);
    builder.setMessageId(0x1111U);
    builder.addBandwidth(5000000ULL);
    builder.addGain(42U);
    builder.setDataFormat(DataFormat::Complex16BitSigned);
    builder.addBeamWidth(100U);
    builder.addDwell(0x123456789ABCDEF0ULL);

    const size_t written = builder.finalize();
    assert(written > 0);
    std::vector<uint32_t> packet = packet_words(buffer, written);
    assert(ControlScheduleRequestView(std::span<const uint32_t>(packet.data(), packet.size())).isValid());
    return packet;
}

std::vector<uint32_t> make_ackr_packet() {
    std::array<uint32_t, 1024> buffer{};
    AckRBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x20202020U);

    const size_t written = builder.finalize();
    assert(written > 0);
    std::vector<uint32_t> packet = packet_words(buffer, written);
    assert(AckRView(std::span<const uint32_t>(packet.data(), packet.size())).isValid());
    return packet;
}

std::vector<uint32_t> make_context_packet() {
    std::array<uint32_t, 1024> buffer{};
    ExtensionDataContextBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x30303030U);
    builder.setPacketCount(3U);
    builder.addBandwidth(5000000ULL);
    builder.addGain(0x12345678U);
    builder.setDataFormat(DataFormat::Complex16BitSigned);

    const size_t written = builder.finalize();
    assert(written > 0);
    std::vector<uint32_t> packet = packet_words(buffer, written);
    assert(ExtensionDataContextView(std::span<const uint32_t>(packet.data(), packet.size())).isValid());
    return packet;
}

template <typename View>
void expect_rejected(const std::vector<uint32_t>& packet) {
    const View view(std::span<const uint32_t>(packet.data(), packet.size()));
    assert(!view.isValid());
}

template <typename View>
void expect_rejects_each_mask_bit(const std::vector<uint32_t>& good_packet, size_t word_index, uint32_t mask) {
    for (unsigned bit = 0; bit < 32U; ++bit) {
        const uint32_t single_bit = bit_mask(bit);
        if ((mask & single_bit) == 0U) {
            continue;
        }

        std::vector<uint32_t> packet = good_packet;
        packet[word_index] = htonl(ntohl(packet[word_index]) | single_bit);
        expect_rejected<View>(packet);
    }
}

template <typename View>
void expect_rejects_each_inserted_cif7_bit(
    const std::vector<uint32_t>& good_packet,
    size_t cif0_index,
    size_t cif7_insert_index,
    uint32_t cif7_mask) {
    for (unsigned bit = 0; bit < 32U; ++bit) {
        const uint32_t single_bit = bit_mask(bit);
        if ((cif7_mask & single_bit) == 0U) {
            continue;
        }

        std::vector<uint32_t> packet = good_packet;
        packet[cif0_index] = htonl(ntohl(packet[cif0_index]) | masks::CIF0_CIF7_ENABLE);
        packet.insert(packet.begin() + static_cast<std::ptrdiff_t>(cif7_insert_index), htonl(single_bit));
        set_packet_size(packet);
        expect_rejected<View>(packet);
    }
}

void test_happy_paths() {
    const auto control = make_control_packet();
    assert(ControlScheduleRequestView(std::span<const uint32_t>(control.data(), control.size())).isValid());

    const auto ackr = make_ackr_packet();
    assert(AckRView(std::span<const uint32_t>(ackr.data(), ackr.size())).isValid());

    const auto context = make_context_packet();
    assert(ExtensionDataContextView(std::span<const uint32_t>(context.data(), context.size())).isValid());

    std::cout << "test_happy_paths passed\n";
}

void test_control_forbidden_cif_ranges() {
    const auto packet = make_control_packet();

    expect_rejects_each_mask_bit<ControlScheduleRequestView>(packet, 9U, masks::CONTROLSCHEDULEREQUESTPACKET_CIF0_N_MASK);
    expect_rejects_each_mask_bit<ControlScheduleRequestView>(packet, 10U, masks::CONTROLSCHEDULEREQUESTPACKET_CIF1_N_MASK);
    expect_rejects_each_mask_bit<ControlScheduleRequestView>(packet, 11U, masks::CONTROLSCHEDULEREQUESTPACKET_CIF2_N_MASK);
    expect_rejects_each_mask_bit<ControlScheduleRequestView>(packet, 12U, masks::CONTROLSCHEDULEREQUESTPACKET_CIF3_N_MASK);
    expect_rejects_each_mask_bit<ControlScheduleRequestView>(packet, 13U, masks::CONTROLSCHEDULEREQUESTPACKET_CIF4_N_MASK);
    expect_rejects_each_inserted_cif7_bit<ControlScheduleRequestView>(packet, 9U, 14U, masks::CONTROLSCHEDULEREQUESTPACKET_CIF7_N_MASK);

    std::cout << "test_control_forbidden_cif_ranges passed\n";
}

void test_ackr_forbidden_cif_ranges() {
    const auto packet = make_ackr_packet();

    expect_rejects_each_mask_bit<AckRView>(packet, 9U, masks::SCHEDULEACKACKRPACKET_CIF0_N_MASK);
    expect_rejects_each_mask_bit<AckRView>(packet, 10U, masks::SCHEDULEACKACKRPACKET_CIF2_N_MASK);
    expect_rejects_each_mask_bit<AckRView>(packet, 11U, masks::SCHEDULEACKACKRPACKET_CIF4_N_MASK);

    expect_rejects_each_inserted_cif7_bit<AckRView>(packet, 9U, 12U, masks::SCHEDULEACKACKRPACKET_CIF7_N_MASK);

    std::cout << "test_ackr_forbidden_cif_ranges passed\n";
}

void test_context_forbidden_cif_ranges() {
    const auto packet = make_context_packet();

    expect_rejects_each_mask_bit<ExtensionDataContextView>(packet, 7U, masks::EXTENSIONDATACONTEXTPACKET_CIF0_N_MASK);
    expect_rejects_each_mask_bit<ExtensionDataContextView>(packet, 8U, masks::EXTENSIONDATACONTEXTPACKET_CIF1_N_MASK);
    expect_rejects_each_mask_bit<ExtensionDataContextView>(packet, 9U, masks::EXTENSIONDATACONTEXTPACKET_CIF2_N_MASK);
    expect_rejects_each_mask_bit<ExtensionDataContextView>(packet, 10U, masks::EXTENSIONDATACONTEXTPACKET_CIF3_N_MASK);
    expect_rejects_each_mask_bit<ExtensionDataContextView>(packet, 11U, masks::EXTENSIONDATACONTEXTPACKET_CIF4_N_MASK);
    expect_rejects_each_inserted_cif7_bit<ExtensionDataContextView>(packet, 7U, 12U, masks::EXTENSIONDATACONTEXTPACKET_CIF7_N_MASK);

    std::cout << "test_context_forbidden_cif_ranges passed\n";
}

} // namespace

auto main() -> int {
    test_happy_paths();
    test_control_forbidden_cif_ranges();
    test_ackr_forbidden_cif_ranges();
    test_context_forbidden_cif_ranges();
    std::cout << "All forbidden indicator range tests passed!\n";
    return 0;
}

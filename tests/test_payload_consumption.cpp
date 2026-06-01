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

// Copy the written packet words out of the fixed test buffer. Views receive a
// span sized to the packet under test, not the builder's full scratch buffer.
std::vector<uint32_t> packet_words(const std::array<uint32_t, 1024>& buffer, size_t written) {
    return std::vector<uint32_t>(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(written));
}

// Keep word 1 packet-size bits consistent after a test mutates packet length.
void set_packet_size(std::vector<uint32_t>& packet) {
    uint32_t word1 = ntohl(packet[0]);
    word1 &= ~masks::PKT_SIZE_MASK;
    word1 |= static_cast<uint32_t>(packet.size()) & masks::PKT_SIZE_MASK;
    packet[0] = htonl(word1);
}

// Append one unexplained payload word and update packetSize. The extra word is
// inside the declared packet, but no AMS tailoring indicator accounts for it.
std::vector<uint32_t> with_extra_payload_word(std::vector<uint32_t> packet) {
    packet.push_back(htonl(0xA5A5A5A5U));
    set_packet_size(packet);
    return packet;
}

std::vector<uint32_t> make_control_packet() {
    std::array<uint32_t, 1024> buffer{};
    ControlScheduleRequestBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x11111111U);
    builder.setMessageId(0x2222U);
    builder.addBandwidth(5000000ULL);
    builder.addGain(42U);
    builder.setDataFormat(DataFormat::Complex16BitSigned);
    builder.addBeamWidth(100U);
    builder.addDwell(0x123456789ABCDEF0ULL);

    const size_t written = builder.finalize();
    assert(written > 0);
    return packet_words(buffer, written);
}

std::vector<uint32_t> make_ackr_packet() {
    std::array<uint32_t, 1024> buffer{};
    AckRBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x22222222U);

    const size_t written = builder.finalize();
    assert(written > 0);
    return packet_words(buffer, written);
}

std::vector<uint32_t> make_ackx_with_error_packet() {
    std::array<uint32_t, 1024> buffer{};
    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x33333333U);
    builder.addError(AckXErrorSubject::Bandwidth, ackx_detail::ERROR_NOT_EXECUTED, 1U);

    const size_t written = builder.finalize();
    assert(written > 0);
    return packet_words(buffer, written);
}

std::vector<uint32_t> make_ackx_without_error_packet() {
    std::array<uint32_t, 1024> buffer{};
    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x44444444U);

    const size_t written = builder.finalize();
    assert(written > 0);
    return packet_words(buffer, written);
}

std::vector<uint32_t> make_context_packet() {
    std::array<uint32_t, 1024> buffer{};
    ExtensionDataContextBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x55555555U);
    builder.setPacketCount(5U);
    builder.addBandwidth(5000000ULL);
    builder.addGain(0x12345678U);
    builder.setDataFormat(DataFormat::Complex16BitSigned);

    const size_t written = builder.finalize();
    assert(written > 0);
    return packet_words(buffer, written);
}

// Baseline packets from each builder must still validate when every declared
// packet word is consumed by the prologue, indicator words, and indicated
// payload fields.
void test_happy_paths_consume_exact_payloads() {
    const auto control = make_control_packet();
    assert(ControlScheduleRequestView(std::span<const uint32_t>(control.data(), control.size())).isValid());

    const auto ackr = make_ackr_packet();
    assert(AckRView(std::span<const uint32_t>(ackr.data(), ackr.size())).isValid());

    const auto ackx_with_error = make_ackx_with_error_packet();
    assert(AckXView(std::span<const uint32_t>(ackx_with_error.data(), ackx_with_error.size())).isValid());

    const auto ackx_without_error = make_ackx_without_error_packet();
    assert(AckXView(std::span<const uint32_t>(ackx_without_error.data(), ackx_without_error.size())).isValid());

    const auto context = make_context_packet();
    assert(ExtensionDataContextView(std::span<const uint32_t>(context.data(), context.size())).isValid());

    std::cout << "test_happy_paths_consume_exact_payloads passed\n";
}

// Each view must reject a packet whose packetSize includes trailing words after
// all valid fields have been parsed. This prevents silently accepting payload
// data that is not described by CIF/EIF indicators in the AMS tailoring.
void test_extra_payload_words_are_rejected() {
    const auto control = with_extra_payload_word(make_control_packet());
    assert(!ControlScheduleRequestView(std::span<const uint32_t>(control.data(), control.size())).isValid());

    const auto ackr = with_extra_payload_word(make_ackr_packet());
    assert(!AckRView(std::span<const uint32_t>(ackr.data(), ackr.size())).isValid());

    const auto ackx_with_error = with_extra_payload_word(make_ackx_with_error_packet());
    assert(!AckXView(std::span<const uint32_t>(ackx_with_error.data(), ackx_with_error.size())).isValid());

    const auto ackx_without_error = with_extra_payload_word(make_ackx_without_error_packet());
    assert(!AckXView(std::span<const uint32_t>(ackx_without_error.data(), ackx_without_error.size())).isValid());

    const auto context = with_extra_payload_word(make_context_packet());
    assert(!ExtensionDataContextView(std::span<const uint32_t>(context.data(), context.size())).isValid());

    std::cout << "test_extra_payload_words_are_rejected passed\n";
}

} // namespace

auto main() -> int {
    test_happy_paths_consume_exact_payloads();
    test_extra_payload_words_are_rejected();
    std::cout << "All payload consumption tests passed!\n";
    return 0;
}

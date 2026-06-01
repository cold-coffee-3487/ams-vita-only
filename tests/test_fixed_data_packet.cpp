#include "ams/iface/vita/FixedDataPacket.h"

#include <arpa/inet.h>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <span>
#include <vector>

using namespace ams::iface::vita;

namespace {

template <std::size_t Bytes>
std::size_t write_valid_packet(FixedDataPacket<Bytes>& packet) {
    auto builder = packet.builder();
    builder.setStreamId(0x12345678U);
    builder.setTimestampInt(0x10203040U);
    builder.setTimestampFracHigh(0x50607080U);
    builder.setTimestampFracLow(0x90A0B0C0U);

    const std::array<uint32_t, 2> payload = {htonl(0x01020304U), htonl(0xA0B0C0D0U)};
    builder.setPayload(payload);
    return builder.finalize();
}

template <std::size_t Bytes>
void test_capacity_and_header_length() {
    FixedDataPacket<Bytes> packet;
    static_assert(FixedDataPacket<Bytes>::byte_capacity == Bytes);
    static_assert(FixedDataPacket<Bytes>::word_capacity == Bytes / sizeof(uint32_t));
    static_assert(sizeof(FixedDataPacket<Bytes>) == Bytes);

    auto word_span = packet.words();
    auto byte_span = packet.bytes();
    const auto* data_ptr = packet.data();
    assert(data_ptr == word_span.data());
    assert(word_span.size() == FixedDataPacket<Bytes>::word_capacity);
    assert(byte_span.size() == FixedDataPacket<Bytes>::byte_capacity);

    const std::size_t written = write_valid_packet(packet);
    assert(written == DataPacketView::getPrologueWords() + 2U + 1U);

    const DataPacketView view = packet.view();
    assert(view.isValid());
    assert(static_cast<std::size_t>(view.getPacketSize()) == written);
    const auto post_write_words = packet.words();
    const auto post_write_bytes = packet.bytes();
    assert(static_cast<std::size_t>(view.getPacketSize()) < post_write_words.size());
    assert(static_cast<std::size_t>(view.getPacketSize()) * sizeof(uint32_t) < post_write_bytes.size());

    const auto payload = view.getPayload();
    assert(payload.size() == 2U);
    assert(payload.data() == post_write_words.data() + DataPacketView::getPrologueWords());
    assert(payload[0] == htonl(0x01020304U));
    assert(ntohl(payload[0]) == 0x01020304U);
}

void test_fixed_capacity_set() {
    test_capacity_and_header_length<512>();
    test_capacity_and_header_length<1024>();
    test_capacity_and_header_length<8192>();
    test_capacity_and_header_length<32768>();
    std::cout << "test_fixed_capacity_set passed\n";
}

void test_default_storage_is_not_a_valid_packet() {
    const FixedDataPacket<512> packet;
    assert(!packet.view().isValid());
    std::cout << "test_default_storage_is_not_a_valid_packet passed\n";
}

void test_malformed_headers_are_rejected() {
    {
        FixedDataPacket<512> packet;
        const std::size_t written = write_valid_packet(packet);
        assert(written > 0U);
        assert(packet.view().isValid());
        auto words = packet.words();
        const uint32_t original_word1 = ntohl(words[0]);
        words[0] = htonl((original_word1 & ~masks::PKT_TYPE_MASK) | (2U << masks::PKT_TYPE_SHIFT));
        assert(!packet.view().isValid());
    }

    {
        FixedDataPacket<512> packet;
        const std::size_t written = write_valid_packet(packet);
        assert(written > 0U);
        auto words = packet.words();
        const uint32_t original_word1 = ntohl(words[0]);
        words[0] = htonl(original_word1 & ~masks::DATA_TRAILER_MASK);
        assert(!packet.view().isValid());
    }

    {
        FixedDataPacket<512> packet;
        const std::size_t written = write_valid_packet(packet);
        assert(written > 0U);
        auto words = packet.words();
        words[2] = htonl(0xBBBBBBU);
        assert(!packet.view().isValid());
    }

    {
        FixedDataPacket<512> packet;
        const std::size_t written = write_valid_packet(packet);
        assert(written > 0U);
        auto words = packet.words();
        words[3] = htonl(0x04000D06U);
        assert(!packet.view().isValid());
    }

    {
        FixedDataPacket<512> packet;
        const std::size_t written = write_valid_packet(packet);
        assert(written > 0U);
        auto words = packet.words();
        words[written - 1U] = htonl(ntohl(words[written - 1U]) | 0x1U);
        assert(!packet.view().isValid());
    }

    std::cout << "test_malformed_headers_are_rejected passed\n";
}

void test_max_capacity_payload() {
    FixedDataPacket<512> packet;
    constexpr std::size_t max_payload_words = FixedDataPacket<512>::word_capacity - 7U - 1U;
    const std::vector<uint32_t> max_payload(max_payload_words, htonl(0x11223344U));

    auto builder = packet.builder();
    builder.setPayload(std::span<const uint32_t>(max_payload.data(), max_payload.size()));
    const std::size_t written = builder.finalize();
    assert(written == FixedDataPacket<512>::word_capacity);

    const DataPacketView view = packet.view();
    assert(view.isValid());
    assert(static_cast<std::size_t>(view.getPacketSize()) == FixedDataPacket<512>::word_capacity);
    assert(view.getPayload().size() == max_payload_words);
    assert(view.getPayload().front() == htonl(0x11223344U));

    const std::vector<uint32_t> too_large_payload(max_payload_words + 1U, htonl(0x55667788U));
    auto oversized_builder = packet.builder();
    oversized_builder.setPayload(std::span<const uint32_t>(too_large_payload.data(), too_large_payload.size()));
    const std::size_t oversized_written = oversized_builder.finalize();
    assert(oversized_written == 0U);

    std::cout << "test_max_capacity_payload passed\n";
}

} // namespace

auto main() -> int {
    test_fixed_capacity_set();
    test_default_storage_is_not_a_valid_packet();
    test_malformed_headers_are_rejected();
    test_max_capacity_payload();
    std::cout << "All FixedDataPacket tests passed!\n";
    return 0;
}

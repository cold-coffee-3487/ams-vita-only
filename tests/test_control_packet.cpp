#include "ams/iface/vita/ControlScheduleRequestPacket.h"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <array>
#include <optional>
#include <stdexcept>
#include <vector>
#include <type_traits>

using namespace ams::iface::vita;

void test_control_packet() {
    constexpr size_t BUFFER_SIZE = 1024;
    constexpr uint32_t STREAM_ID = 0x11223344;
    constexpr uint32_t MESSAGE_ID = 12345;
    constexpr uint32_t GAIN_VAL = 42;
    constexpr uint64_t BW_VAL = 5000000ULL;
    constexpr uint32_t BEAM_WIDTH = 100;
    constexpr uint64_t DWELL_VAL = 0x123456789ABCDEF0;
    
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    ControlScheduleRequestBuilder builder(buffer.data(), buffer.size());
    
    builder.setStreamId(STREAM_ID);
    builder.setMessageId(MESSAGE_ID);

    // Out-of-order setters
    builder.addGain(GAIN_VAL);
    builder.addBandwidth(BW_VAL); // 5MHz
    builder.addBeamWidth(BEAM_WIDTH);
    builder.addDwell(DWELL_VAL);
    builder.setDataFormat(ams::iface::vita::DataFormat::Complex16BitSigned);
    
    size_t written = builder.finalize();

    // Verify word1 packet size updated
    assert((ntohl(buffer[0]) & masks::PKT_SIZE_MASK) == written);

    // Read back via View
    ControlScheduleRequestView view(buffer);
    
    assert(view.getStreamId() == STREAM_ID);
    assert(view.getMessageId() == MESSAGE_ID);

    // Verify Payloads
    assert(view.getBandwidth().value_or(0) == BW_VAL);
    assert(view.getGain().value_or(0) == GAIN_VAL);
    assert(view.getBeamWidth().value_or(0) == BEAM_WIDTH);
    assert(view.getDwell().value_or(0) == DWELL_VAL);
    auto parsed_data_format = view.getParsedDataFormat();
    assert(parsed_data_format && *parsed_data_format == DataFormat::Complex16BitSigned);

    bool replaced_data_format = false;
    const uint32_t supported_format_hi = static_cast<uint32_t>(static_cast<uint64_t>(DataFormat::Complex16BitSigned) >> 32);
    const uint32_t supported_format_lo = static_cast<uint32_t>(static_cast<uint64_t>(DataFormat::Complex16BitSigned));
    for (size_t i = 0; i + 1 < written; ++i) {
        if (ntohl(buffer[i]) == supported_format_hi && ntohl(buffer[i + 1]) == supported_format_lo) {
            buffer[i] = htonl(0x90000000U);
            buffer[i + 1] = htonl(0U);
            replaced_data_format = true;
            break;
        }
    }
    assert(replaced_data_format && "test packet should contain a dataFormat payload");
    ControlScheduleRequestView unsupported_format_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!unsupported_format_view.isValid() && "Control must reject unsupported AMS data formats");

    std::cout << "test_control_packet passed\n";
}

void test_control_pointing3d_structure_excludes_single_word_pointing3d() {
    std::array<uint32_t, 1024> buffer{};
    ControlScheduleRequestBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x55667788U);
    builder.setMessageId(12346U);
    builder.setDataFormat(DataFormat::Complex16BitSigned);

    const std::array<Pointing3dRecord, 1> records{{Pointing3dRecord{std::nullopt, 0x01020304U}}};
    builder.addPointing3dStructure(std::nullopt, records);

    const size_t written = builder.finalize();
    assert(written > 0);

    ControlScheduleRequestView view(std::span<const uint32_t>(buffer.data(), written));
    assert(view.isValid());
    assert(!view.getPointing3d().has_value());
    assert(!view.getPointing3dStructRaw().empty());
    assert(view.getPointing3dStructure().isValid());

    std::array<uint32_t, 1024> invalid_buffer = buffer;
    invalid_buffer[10] = htonl(ntohl(invalid_buffer[10]) | masks::CIF1_POINTING3D_MASK);
    ControlScheduleRequestView invalid_view(std::span<const uint32_t>(invalid_buffer.data(), written));
    assert(!invalid_view.isValid() && "Control must reject simultaneous pointing3d and pointing3dStruct fields");

    std::cout << "test_control_pointing3d_structure_excludes_single_word_pointing3d passed\n";
}

void test_control_packet_size_bounds() {
    constexpr std::size_t MIN_WORDS = control_schedule_request_detail::CONTROL_MIN_PACKET_WORDS;
    constexpr std::size_t MAX_WORDS = control_schedule_request_detail::CONTROL_MAX_PACKET_WORDS;
    static_assert(MIN_WORDS == 32U);
    static_assert(MAX_WORDS == 65535U);

    std::array<uint32_t, MIN_WORDS - 1U> undersized_buffer{};
    ControlScheduleRequestBuilder undersized_builder(undersized_buffer.data(), undersized_buffer.size());
    undersized_builder.setMessageId(1U);
    undersized_builder.setDataFormat(DataFormat::Complex16BitSigned);
    const std::size_t undersized_written = undersized_builder.finalize();
    assert(undersized_written == 0U && "Control builder must reject capacity below the 32-word minimum");

    std::array<uint32_t, MIN_WORDS> minimal_buffer{};
    ControlScheduleRequestBuilder minimal_builder(minimal_buffer.data(), minimal_buffer.size());
    minimal_builder.setMessageId(1U);
    minimal_builder.setDataFormat(DataFormat::Complex16BitSigned);
    const std::size_t minimal_written = minimal_builder.finalize();
    assert(minimal_written == MIN_WORDS);
    assert((ntohl(minimal_buffer[0]) & masks::PKT_SIZE_MASK) == MIN_WORDS);
    assert(ControlScheduleRequestView(std::span<const uint32_t>(minimal_buffer.data(), minimal_written)).isValid());

    std::array<uint32_t, MIN_WORDS> size_22_buffer = minimal_buffer;
    size_22_buffer[0] = htonl((ntohl(size_22_buffer[0]) & ~masks::PKT_SIZE_MASK) | 22U);
    assert(!ControlScheduleRequestView(std::span<const uint32_t>(size_22_buffer.data(), size_22_buffer.size())).isValid() &&
           "Control view must reject declared packet sizes below 32 words");

    std::vector<Pointing3dRecord> max_records(4095U, Pointing3dRecord{0U, 0x01020304U});
    std::vector<uint32_t> large_buffer(MAX_WORDS);
    ControlScheduleRequestBuilder large_builder(large_buffer.data(), large_buffer.size());
    large_builder.setMessageId(2U);
    large_builder.setDataFormat(DataFormat::Complex16BitSigned);
    large_builder.addPointing3dStructure(7U, std::span<const Pointing3dRecord>(max_records.data(), max_records.size()));
    const std::size_t large_written = large_builder.finalize();
    assert(large_written > MIN_WORDS);
    assert(large_written <= MAX_WORDS);
    assert(ControlScheduleRequestView(std::span<const uint32_t>(large_buffer.data(), large_written)).isValid());

    std::vector<uint32_t> overdeclared_buffer(MAX_WORDS);
    std::copy(minimal_buffer.begin(), minimal_buffer.end(), overdeclared_buffer.begin());
    overdeclared_buffer[0] = htonl((ntohl(overdeclared_buffer[0]) & ~masks::PKT_SIZE_MASK) |
                                   static_cast<uint32_t>(MAX_WORDS));
    assert(!ControlScheduleRequestView(std::span<const uint32_t>(overdeclared_buffer.data(), overdeclared_buffer.size())).isValid() &&
           "Control view must reject declared max-size packets unless the payload structure consumes exactly that size");

    class OversizeControlScheduleRequestBuilder : public ControlScheduleRequestBuilder {
    public:
        using ControlScheduleRequestBuilder::ControlScheduleRequestBuilder;

        void forceOversizePointing3dStruct(std::span<const uint32_t> words) {
            cif1 &= ~masks::CIF1_POINTING3D_MASK;
            cif1 |= masks::CIF1_POINTING3DSTRUCT_MASK;
            p_Pointing3dStruct = words;
        }
    };

    std::vector<uint32_t> oversize_storage(MAX_WORDS + 1U);
    std::vector<uint32_t> oversize_dynamic_field(MAX_WORDS, htonl(0U));
    OversizeControlScheduleRequestBuilder oversize_builder(oversize_storage.data(), oversize_storage.size());
    oversize_builder.setMessageId(3U);
    oversize_builder.setDataFormat(DataFormat::Complex16BitSigned);
    oversize_builder.forceOversizePointing3dStruct(std::span<const uint32_t>(oversize_dynamic_field.data(), oversize_dynamic_field.size()));
    const std::size_t oversize_written = oversize_builder.finalize();
    assert(oversize_written == 0U && "Control builder must reject packets that would exceed the 16-bit size field");

    std::cout << "test_control_packet_size_bounds passed\n";
}

void test_control_zero_allocation() {
    ControlScheduleRequestBuilder builder(nullptr, 0);
    builder.setStreamId(0x1234);
    builder.setPacketCount(5); // Should modify local word1 caching var, not crash
    size_t written = builder.finalize();
    assert(written == 0);

    std::cout << "test_control_zero_allocation passed\n";
}

void test_control_negative() {
    constexpr size_t BUFFER_SIZE = 1024;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    ControlScheduleRequestBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.setMessageId(12345);
    
    // Add required payload fields to satisfy B masks
    builder.addBandwidth(5000000);
    builder.setDataFormat(DataFormat::Complex16BitSigned);
    // ControlScheduleRequest has several B fields. finalize() will automatically add the correct B-masks.
    
    size_t written = builder.finalize();
    assert(written > 0 && "Builder should produce a valid packet");

    ControlScheduleRequestView valid_view(std::span<const uint32_t>(buffer.data(), written));
    assert(valid_view.isValid() && "Base Control packet should be valid");

    const uint32_t original_message_id = buffer[8];
    buffer[8] = htonl(0U);
    ControlScheduleRequestView zero_message_id_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!zero_message_id_view.isValid() && "Control messageId must start at 1");
    buffer[8] = original_message_id;

    std::array<uint32_t, BUFFER_SIZE> zero_message_id_buffer{};
    ControlScheduleRequestBuilder zero_message_id_builder(zero_message_id_buffer.data(), zero_message_id_buffer.size());
    zero_message_id_builder.setStreamId(0x1234);
    zero_message_id_builder.setDataFormat(DataFormat::Complex16BitSigned);
    zero_message_id_builder.addBandwidth(5000000);
    const size_t zero_message_id_written = zero_message_id_builder.finalize();
    assert(zero_message_id_written == 0 && "Control builder must reject messageId zero");

    const uint32_t original_word1 = ntohl(buffer[0]);
    const uint32_t invalid_header_bits[] = {
        1U << 26, // .isAck
        1U << 25, // .reservedBit25
        1U << 24  // .isCancellation
    };
    for (uint32_t invalid_bit : invalid_header_bits) {
        buffer[0] = htonl(original_word1 | invalid_bit);
        ControlScheduleRequestView invalid_header_view(std::span<const uint32_t>(buffer.data(), written));
        assert(!invalid_header_view.isValid() && "Control header with fixed-zero bit set should be rejected");
    }
    buffer[0] = htonl(original_word1);

    const uint32_t original_cam = ntohl(buffer[7]);
    const uint32_t invalid_cam_bits[] = {
        masks::CAM_CE_MASK,
        masks::CAM_IE_MASK,
        masks::CAM_CR_MASK,
        masks::CAM_IR_MASK,
        masks::CAM_P_MASK,
        masks::CAM_ER_MASK,
        masks::CAM_NACK_MASK,
        masks::CAM_REQV_MASK,
        masks::CAM_REQS_MASK,
        masks::CAM_REQW_MASK,
        masks::CAM_ACK_BITS_MASK,
        masks::CAM_REQ_STAT_CH_MASK,
        0x7U
    };
    for (uint32_t invalid_bit : invalid_cam_bits) {
        buffer[7] = htonl(original_cam | invalid_bit);
        ControlScheduleRequestView invalid_cam_view(std::span<const uint32_t>(buffer.data(), written));
        assert(!invalid_cam_view.isValid() && "Control CAM with unsupported bit set should be rejected");
    }

    buffer[7] = htonl((original_cam & ~masks::CAM_ACTION_MASK) | masks::CAM_REQX_MASK | masks::CAM_REQER_MASK |
                      masks::CAM_REQR_MASK | (1U << masks::CAM_SCH_REQ_TYP_SHIFT));
    ControlScheduleRequestView invalid_action_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!invalid_action_view.isValid() && "Control CAM must require execute action");

    buffer[7] = htonl(original_cam & ~masks::CAM_SCH_REQ_TYP_MASK);
    ControlScheduleRequestView informational_schedule_type_view(std::span<const uint32_t>(buffer.data(), written));
    assert(informational_schedule_type_view.isValid() && "Control CAM must allow AMS schedule-request type 0");

    buffer[7] = htonl((original_cam & ~masks::CAM_SCH_REQ_TYP_MASK) | (15U << masks::CAM_SCH_REQ_TYP_SHIFT));
    ControlScheduleRequestView cancellation_schedule_type_view(std::span<const uint32_t>(buffer.data(), written));
    assert(cancellation_schedule_type_view.isValid() && "Control CAM must allow AMS schedule-request type 15");
    buffer[7] = htonl(original_cam);

    std::array<uint32_t, BUFFER_SIZE> typed_schedule_buffer{};
    ControlScheduleRequestBuilder typed_schedule_builder(typed_schedule_buffer.data(), typed_schedule_buffer.size());
    typed_schedule_builder.setStreamId(0x1234);
    typed_schedule_builder.setMessageId(12345);
    typed_schedule_builder.addBandwidth(5000000);
    typed_schedule_builder.setDataFormat(DataFormat::Complex16BitSigned);
    typed_schedule_builder.setScheduleRequestType(15U);
    const size_t typed_schedule_written = typed_schedule_builder.finalize();
    assert(typed_schedule_written > 0);
    ControlScheduleRequestView typed_schedule_view(std::span<const uint32_t>(typed_schedule_buffer.data(), typed_schedule_written));
    assert(typed_schedule_view.isValid());
    assert(typed_schedule_view.getScheduleRequestType() == 15U);

    bool rejected_bad_schedule_type = false;
    try {
        typed_schedule_builder.setScheduleRequestType(16U);
    } catch (const std::invalid_argument&) {
        rejected_bad_schedule_type = true;
    }
    assert(rejected_bad_schedule_type && "Control builder must reject schedule request types wider than 4 bits");

    // Test Invalid OUI
    buffer[2] = htonl(0xBBBBBB);
    ControlScheduleRequestView invalid_oui_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!invalid_oui_view.isValid() && "Control with invalid OUI should be rejected");
    buffer[2] = htonl(0xAAAAAA);

    // Test Missing B Field in CIF0
    uint32_t orig_cif0 = ntohl(buffer[9]);
    buffer[9] = htonl(orig_cif0 & ~(1U << 29)); // Remove Bandwidth bit (assuming bit 29 is a B bit)
    ControlScheduleRequestView missing_b_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!missing_b_view.isValid() && "Control missing a B field should be rejected");

    // Test Present N Field in CIF0
    buffer[9] = htonl(orig_cif0 | masks::CONTROLSCHEDULEREQUESTPACKET_CIF0_N_MASK);
    ControlScheduleRequestView present_n_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!present_n_view.isValid() && "Control with an N field should be rejected");

    std::cout << "test_control_negative passed\n";
}

void test_control_builder_uncopyable() {
    static_assert(!std::is_copy_constructible_v<ControlScheduleRequestBuilder>, "ControlScheduleRequestBuilder should not be copyable");
    static_assert(!std::is_move_constructible_v<ControlScheduleRequestBuilder>, "ControlScheduleRequestBuilder should not be movable");
}

auto main() -> int {
    test_control_packet();
    test_control_pointing3d_structure_excludes_single_word_pointing3d();
    test_control_packet_size_bounds();
    test_control_zero_allocation();
    test_control_negative();
    test_control_builder_uncopyable();
    std::cout << "All Control Packet tests passed!\n";
    return 0;
}


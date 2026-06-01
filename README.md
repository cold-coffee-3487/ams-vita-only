# AMS VITA

> Header-only C++20 packet views and builders for the AMS GRA tailoring of VITA 49.2 VITA Radio Transport packets.

## Contents

- [Features](#features)
- [Assumptions](#assumptions)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Basic Usage](#basic-usage)
- [CMake Vendoring](#cmake-vendoring)
- [Validation](#validation)
- [Generation](#generation)
- [Tailoring Source](#tailoring-source)
- [License](#license)

## Features

- Control schedule request packet builders and views
- Schedule acknowledgment and execution acknowledgment packet builders and views
- Data packet builders and views with payload and trailer handling
- Fixed-size owning data packet storage for APIs that need concrete packet buffers
- Extension data context packet builders and views
- AMS GRA tailoring checks for packet type, timestamps, class ID presence, and packet size
- Word-oriented serialization and deserialization with explicit network byte order handling

## Assumptions

This library is generated for the AMS GRA tailoring of VITA 49.2 and is used on both the sender and receiver sides of that tailored exchange. Generated builders do not expose forbidden fields and write forbidden bits as zero by default. Packets with extra payload words or forbidden bits set are outside the expected sender contract.

## Prerequisites

- C++20 compiler
- CMake 3.15+
- Python 3.12+ with PyYAML for code, test-vector, and documentation generation
- cppcheck and clang-tidy for `make check`
- Docker or another compatible container runtime for container validation

## Installation

This is a header-only library. Add `include/` to your compiler include path and include the packet headers you need. Payload words passed to `DataPacketBuilder::setPayload()` must already be in packet/network byte order. `DataPacketView::getPayload()` returns packet/network-order words without copying.

```bash
c++ -std=c++20 -I include your_program.cpp -o your_program
```

## Basic Usage

```cpp
#include "ams/iface/vita/FixedDataPacket.h"

#include <arpa/inet.h>
#include <array>
#include <cstddef>
#include <cstdint>

int main() {
    ams::iface::vita::FixedDataPacket<1024> packet{};

    auto builder = packet.builder();
    builder.setStreamId(0x12345678U);

    std::array<uint32_t, 2> payload = {htonl(0x11111111U), htonl(0x22222222U)};
    builder.setPayload(payload);

    const std::size_t written_words = builder.finalize();
    if (written_words == 0) {
        return 1;
    }

    auto view = packet.view();
    if (!view.isValid() || static_cast<std::size_t>(view.getPacketSize()) != written_words) {
        return 1;
    }

    return view.getPayload().size() == payload.size() ? 0 : 1;
}
```

`FixedDataPacket<N>` owns `N` bytes of packet storage. The valid serialized packet length is the packet-size field parsed by `DataPacketView::getPacketSize()`, not the storage capacity.

## CMake Vendoring

Add this repository with `add_subdirectory()` and link the `ams_vita` interface target:

```cmake
add_subdirectory(ams-vita)
target_link_libraries(your_target PRIVATE ams_vita)
```

The target requires C++20 and exports the `include/` directory. Tests are built by default only when AMS VITA is configured as the top-level project. Set `AMS_VITA_BUILD_TESTS=ON` to build them when vendored.

## Validation

Build and run the test suite with host tools:

```bash
make test
```

Run release validation, including tests, cppcheck, and clang-tidy:

```bash
make check
```

Run the same validation in the project container:

```bash
docker build -t ams-vita-test -f Containerfile .
```

Or with Podman:

```bash
podman build -t ams-vita-test -f Containerfile .
```

## Generation

Regenerate all generated files from the tailoring source:

```bash
make generate
```

Set up a local Python virtual environment for generation:

```bash
python3 -m venv venv
. venv/bin/activate
python -m pip install -r scripts/requirements.txt
```

Generation targets are also available separately:

```bash
make generate-docs
make generate-cpp
make generate-roundtrip-tests
```

## Tailoring Source

`spec/ams_vita_49-2_tailoring.yaml` is the source file for AMS GRA tailoring data. Human-readable terminology and field-size notes live in `spec/ams_vita_49-2_tailoring.md`. Generated Markdown and CSV tables live under `spec/generated/`, and generation scripts live under `scripts/`.

## License

This project is licensed under the Apache License 2.0 - see the [LICENSE](LICENSE) and [INTENT.md](INTENT.md) files for details.

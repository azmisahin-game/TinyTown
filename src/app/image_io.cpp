#include "app/image_io.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>

#if defined(TINYTOWN_HAVE_ZLIB)
#include <zlib.h>
#endif

namespace tt::image {
namespace {

// glReadPixels returns rows bottom-up; image formats want top-down.
std::vector<unsigned char> flipVertically(const std::vector<unsigned char>& rgb, int width,
                                          int height) {
    const size_t stride = static_cast<size_t>(width) * 3u;
    std::vector<unsigned char> out(rgb.size());
    for (int y = 0; y < height; ++y) {
        const size_t src = static_cast<size_t>(height - 1 - y) * stride;
        const size_t dst = static_cast<size_t>(y) * stride;
        std::copy(rgb.begin() + static_cast<std::ptrdiff_t>(src),
                  rgb.begin() + static_cast<std::ptrdiff_t>(src + stride),
                  out.begin() + static_cast<std::ptrdiff_t>(dst));
    }
    return out;
}

bool writePpm(const std::string& path, const std::vector<unsigned char>& topDown, int width,
              int height, std::string& error) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        error = "cannot open file for writing: " + path;
        return false;
    }
    out << "P6\n" << width << " " << height << "\n255\n";
    out.write(reinterpret_cast<const char*>(topDown.data()),
              static_cast<std::streamsize>(topDown.size()));
    if (!out) {
        error = "failed while writing PPM data";
        return false;
    }
    return true;
}

#if defined(TINYTOWN_HAVE_ZLIB)

void appendBigEndian32(std::vector<unsigned char>& out, uint32_t value) {
    out.push_back(static_cast<unsigned char>((value >> 24) & 0xFF));
    out.push_back(static_cast<unsigned char>((value >> 16) & 0xFF));
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
    out.push_back(static_cast<unsigned char>(value & 0xFF));
}

uint32_t crcOf(const unsigned char* data, size_t length) {
    return static_cast<uint32_t>(crc32(0L, data, static_cast<uInt>(length)));
}

void appendChunk(std::vector<unsigned char>& out, const char type[4],
                 const std::vector<unsigned char>& payload) {
    appendBigEndian32(out, static_cast<uint32_t>(payload.size()));
    const size_t typeStart = out.size();
    out.insert(out.end(), type, type + 4);
    out.insert(out.end(), payload.begin(), payload.end());
    const uint32_t crc = crcOf(out.data() + typeStart, 4 + payload.size());
    appendBigEndian32(out, crc);
}

bool writePng(const std::string& path, const std::vector<unsigned char>& topDown, int width,
              int height, std::string& error) {
    // Build the raw scanlines with a filter byte of 0 (None) per row.
    const size_t stride = static_cast<size_t>(width) * 3u;
    std::vector<unsigned char> raw;
    raw.reserve((stride + 1) * static_cast<size_t>(height));
    for (int y = 0; y < height; ++y) {
        raw.push_back(0);
        const size_t offset = static_cast<size_t>(y) * stride;
        raw.insert(raw.end(), topDown.begin() + static_cast<std::ptrdiff_t>(offset),
                   topDown.begin() + static_cast<std::ptrdiff_t>(offset + stride));
    }

    uLongf compressedSize = compressBound(static_cast<uLong>(raw.size()));
    std::vector<unsigned char> compressed(compressedSize);
    const int level = Z_BEST_SPEED;
    if (compress2(compressed.data(), &compressedSize, raw.data(),
                  static_cast<uLong>(raw.size()), level) != Z_OK) {
        error = "zlib compression failed";
        return false;
    }
    compressed.resize(compressedSize);

    std::vector<unsigned char> png;
    const unsigned char signature[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    png.insert(png.end(), signature, signature + 8);

    std::vector<unsigned char> ihdr;
    appendBigEndian32(ihdr, static_cast<uint32_t>(width));
    appendBigEndian32(ihdr, static_cast<uint32_t>(height));
    ihdr.push_back(8);   // bit depth
    ihdr.push_back(2);   // colour type: truecolour RGB
    ihdr.push_back(0);   // compression method
    ihdr.push_back(0);   // filter method
    ihdr.push_back(0);   // interlace
    appendChunk(png, "IHDR", ihdr);
    appendChunk(png, "IDAT", compressed);
    appendChunk(png, "IEND", {});

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        error = "cannot open file for writing: " + path;
        return false;
    }
    out.write(reinterpret_cast<const char*>(png.data()),
              static_cast<std::streamsize>(png.size()));
    if (!out) {
        error = "failed while writing PNG data";
        return false;
    }
    return true;
}

#endif  // TINYTOWN_HAVE_ZLIB

}  // namespace

std::string preferredExtension() {
#if defined(TINYTOWN_HAVE_ZLIB)
    return "png";
#else
    return "ppm";
#endif
}

bool writeScreenshot(const std::string& path, const std::vector<unsigned char>& rgb, int width,
                     int height, std::string& error) {
    if (width <= 0 || height <= 0) {
        error = "invalid image dimensions";
        return false;
    }
    const size_t expected = static_cast<size_t>(width) * static_cast<size_t>(height) * 3u;
    if (rgb.size() < expected) {
        error = "pixel buffer is smaller than the declared dimensions";
        return false;
    }

    const std::vector<unsigned char> topDown = flipVertically(rgb, width, height);

    const bool isPng = path.size() >= 4 && path.compare(path.size() - 4, 4, ".png") == 0;
    if (isPng) {
#if defined(TINYTOWN_HAVE_ZLIB)
        return writePng(path, topDown, width, height, error);
#else
        error = "this build has no PNG support (zlib was missing at configure time)";
        return false;
#endif
    }
    return writePpm(path, topDown, width, height, error);
}

}  // namespace tt::image

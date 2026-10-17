#pragma once

// Screenshot writing. PNG is written when zlib is available at build time;
// otherwise a plain PPM is written, which every image tool can read. There is
// no image library dependency either way.

#include <string>
#include <vector>

namespace tt::image {

// Write an RGB8, bottom-up buffer (as produced by glReadPixels) to `path`.
// Returns true on success and fills `error` on failure.
bool writeScreenshot(const std::string& path, const std::vector<unsigned char>& rgb,
                     int width, int height, std::string& error);

// "png" when the build has zlib, otherwise "ppm".
std::string preferredExtension();

}  // namespace tt::image

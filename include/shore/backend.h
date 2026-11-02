#pragma once

#include <memory>
#include "frame_peer.h"

namespace shore {

class SHApp;
class SHFrame;
class SHBackend {
public:
    virtual ~SHBackend() = default;
    virtual std::unique_ptr<SHFramePeer> CreateFramePeer(SHFrame* frame) = 0;
    virtual int Run(SHApp* app, int argc, char** argv) = 0;
    virtual void Quit() = 0;
    virtual const char* Name() const = 0;
};

}  // namespace shore

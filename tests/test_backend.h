#pragma once

#include <memory>
#include <shore/backend.h>

std::unique_ptr<shore::SHBackend> MakeTestBackend();

#pragma once

#include "ir.hpp"

namespace backend {
struct interp {
    void exec(ir::ssa *inst, size_t len);
};
}
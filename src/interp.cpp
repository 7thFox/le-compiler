#include "interp.hpp"

#include "log.hpp"

namespace backend {

void interp::exec(ir::ssa *inst, size_t len) {
    for (size_t i = 0; i < len; i++) {
        log::tracef("op: %.4s", reinterpret_cast<const char *>(&inst[i].op));
    }
}

}
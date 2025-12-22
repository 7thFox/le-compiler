#pragma once

#include "ir.hpp"

struct bb {
    ir::ssa *head;
    ir::ssa *tail;
};
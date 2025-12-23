#pragma once

#include "ast.hpp"
#include "basic-block.hpp"
#include "ir-builder.hpp"

bb *ast2cfg(ir::builder *b, ast::stmt *tree);
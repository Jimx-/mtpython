#ifndef _EXPR_CONTEXT_H_
#define _EXPR_CONTEXT_H_

#include <cstddef>
#include <string>

namespace mtpython {
namespace tree {

typedef enum {
    EC_LOAD,
    EC_STORE,
    EC_DEL,
    EC_AUGLOAD,
    EC_AUGSTORE,
    EC_PARAM,
} ExprContext;

static std::string exprctx2str(ExprContext ctx)
{
    switch (ctx) {
    case EC_LOAD:
        return "Load";
    case EC_STORE:
        return "Store";
    case EC_DEL:
        return "Del";
    case EC_PARAM:
        return "Param";
    }

    return "Invalid";
}

} // namespace tree
} // namespace mtpython

#endif /* _EXPR_CONTEXT_H_ */

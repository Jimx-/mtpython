#include "parse/code_builder.h"

using namespace mtpython::parse;

static std::unordered_map<unsigned char, int> static_opcode_stack_effect = {
    {POP_TOP, -1},
    {ROT_TWO, 0},
    {ROT_THREE, 0},
    {DUP_TOP, 1},
    {DUP_TOP_TWO, 2},
    {NOP, 0},
    {UNARY_POSITIVE, 0},
    {UNARY_NEGATIVE, 0},
    {UNARY_NOT, 0},
    {UNARY_INVERT, 0},
    {BINARY_POWER, -1},
    {BINARY_MULTIPLY, -1},
    {BINARY_MODULO, -1},
    {BINARY_ADD, -1},
    {BINARY_SUBTRACT, -1},
    {BINARY_SUBSCR, -1},
    {BINARY_FLOOR_DIVIDE, -1},
    {BINARY_TRUE_DIVIDE, -1},
    {INPLACE_FLOOR_DIVIDE, -1},
    {INPLACE_TRUE_DIVIDE, -1},
    {STORE_MAP, -2},
    {INPLACE_ADD, -1},
    {INPLACE_SUBTRACT, -1},
    {INPLACE_MULTIPLY, -1},
    {INPLACE_MODULO, -1},
    {STORE_SUBSCR, -3},
    {DELETE_SUBSCR, -2},
    {BINARY_LSHIFT, -1},
    {BINARY_RSHIFT, -1},
    {BINARY_AND, -1},
    {BINARY_XOR, -1},
    {BINARY_OR, -1},
    {INPLACE_POWER, -1},
    {GET_ITER, 0},
    {PRINT_EXPR, -1},
    {LOAD_BUILD_CLASS, 1},
    {YIELD_FROM, -1},
    {INPLACE_LSHIFT, -1},
    {INPLACE_RSHIFT, -1},
    {INPLACE_AND, -1},
    {INPLACE_XOR, -1},
    {INPLACE_OR, -1},
    {BREAK_LOOP, 0},
    {WITH_CLEANUP, -1},
    {RETURN_VALUE, -1},
    {IMPORT_STAR, -1},
    {YIELD_VALUE, -1},
    {POP_BLOCK, 0},
    {END_FINALLY, -3},
    {POP_EXCEPT, 0},
    {STORE_NAME, 1},
    {DELETE_NAME, 0},
    {FOR_ITER, 1},
    {STORE_ATTR, -1},
    {DELETE_ATTR, 0},
    {STORE_GLOBAL, -1},
    {DELETE_GLOBAL, 0},
    {LOAD_CONST, 1},
    {LOAD_NAME, 1},
    {LOAD_ATTR, 1},
    {COMPARE_OP, -1},
    {IMPORT_NAME, -1},
    {IMPORT_FROM, 1},
    {JUMP_FORWARD, 0},
    {JUMP_IF_FALSE_OR_POP, 0},
    {JUMP_IF_TRUE_OR_POP, 0},
    {JUMP_ABSOLUTE, 0},
    {POP_JUMP_IF_FALSE, -1},
    {POP_JUMP_IF_TRUE, -1},
    {LOAD_GLOBAL, 1},
    {CONTINUE_LOOP, 0},
    {SETUP_LOOP, 0},
    {SETUP_EXCEPT, 0},
    {SETUP_FINALLY, 0},
    {LOAD_FAST, 1},
    {STORE_FAST, -1},
    {DELETE_FAST, 0},
    {LOAD_CLOSURE, 1},
    {LOAD_DEREF, 1},
    {STORE_DEREF, -1},
    {DELETE_DEREF, 0},
    {SETUP_WITH, 1},
    {EXTENDED_ARG, 0},
    {LIST_APPEND, -1},
    {SET_ADD, -1},
    {MAP_ADD, -2},
    {LOAD_CLASSDEREF, 1},
};

static std::unordered_map<unsigned char, int (*)(int)> dyn_opcode_stack_effect =
    {
        {UNPACK_SEQUENCE, [](int arg) { return arg - 1; }},
};

void CodeBlock::get_code(std::vector<unsigned char>& code)
{
    for (Instruction* inst : instructions) {
        unsigned char opcode = inst->get_op();

        if (opcode >= HAVE_ARGUMENT) {
            int arg = inst->get_arg();
            if (arg > 0xffff) {
                int ext = arg >> 16;
                code.push_back((unsigned char)EXTENDED_ARG);
                code.push_back((unsigned char)(ext & 0xff));
                code.push_back((unsigned char)(ext >> 8));
                arg &= 0xff;
            }
            code.push_back(opcode);
            code.push_back((unsigned char)(arg & 0xff));
            code.push_back((unsigned char)(arg >> 8));
        } else {
            code.push_back(opcode);
        }
    }
}

void CodeBlock::dfs(std::vector<CodeBlock*>& blocks)
{
    if (seen) return;
    seen = true;

    if (next) next->dfs(blocks);

    for (Instruction* inst : instructions) {
        if (inst->has_jump()) inst->get_target()->dfs(blocks);
    }

    blocks.push_back(this);
}

void CodeBlock::get_block_list(std::vector<CodeBlock*>& blocks)
{
    blocks.clear();

    dfs(blocks);
    std::reverse(blocks.begin(), blocks.end());
}

template <typename T>
static void vector2map(std::vector<T>& vec, std::unordered_map<T, int>& map,
                       int offset = 0)
{
    for (unsigned int i = 0; i < vec.size(); i++) {
        map[vec[i]] = i + offset;
    }
}

CodeBuilder::CodeBuilder(const std::string& name,
                         mtpython::vm::ThreadContext* context, Scope* scope,
                         int first_lineno, CompileInfo* info)
    : name(name)
{
    this->first_lineno = first_lineno;
    this->context = context;
    this->space = context->get_space();
    compile_info = info;
    this->first_block = new_block();
    use_block(this->first_block);
    lineno = 0;
    lineno_set = false;
    argcount = 0;
    kwonlyargcount = 0;

    qualname = space->wrap_None();

    vector2map<std::string>(scope->get_varnames(), varnames);

    std::unordered_map<std::string, int>& symbols = scope->get_symbols();
    int cell_index = 0;
    for (auto iter = symbols.begin(); iter != symbols.end(); iter++) {
        if (iter->second == SCOPE_CELL) {
            cellvars[iter->first] = cell_index;
            cell_index++;
        }
    }

    vector2map<std::string>(scope->get_free_vars(), freevars, cellvars.size());
}

void CodeBuilder::set_lineno(int lineno)
{
    this->lineno = lineno;
    lineno_set = false;
}

CodeBlock* CodeBuilder::use_next_block(CodeBlock* block)
{
    if (!block) block = new_block();

    current_block->set_next(block);
    use_block(block);

    return block;
}

void CodeBuilder::append_instruction(Instruction* inst)
{
    current_block->append_instruction(inst);
}

int CodeBuilder::add_name(std::unordered_map<std::string, int>& container,
                          const std::string& id)
{
    auto got = container.find(id);

    if (got == container.end()) {
        int index = container.size();
        container[id] = index;
        return index;
    } else {
        return got->second;
    }

    return -1;
}

Instruction* CodeBuilder::emit_op(unsigned char op)
{
    Instruction* inst = new Instruction(op);
    if (!lineno_set) {
        inst->set_lineno(lineno);
        lineno_set = true;
    }

    if (current_block->has_return()) return inst;

    append_instruction(inst);
    if (op == RETURN_VALUE) current_block->set_has_return(true);

    return inst;
}

Instruction* CodeBuilder::emit_op_arg(unsigned char op, int arg)
{
    Instruction* inst = new Instruction(op, arg);
    if (!lineno_set) {
        inst->set_lineno(lineno);
        lineno_set = true;
    }

    append_instruction(inst);

    return inst;
}

void CodeBuilder::emit_jump(unsigned char op, CodeBlock* target, bool absolute)
{
    emit_op(op)->jump_to(target, absolute);
}

int CodeBuilder::add_const(mtpython::objects::M_BaseObject* obj)
{
    auto got = consts.find(obj);

    if (got == consts.end()) {
        int index = consts.size();
        consts[obj] = index;
        return index;
    } else {
        return got->second;
    }
}

int CodeBuilder::expr_constant(mtpython::tree::ASTNode* node)
{
    mtpython::tree::NodeType tag = node->get_tag();
    switch (tag) {
    case mtpython::tree::NodeType::NT_NUMBER: {
        mtpython::tree::NumberNode* number_node =
            dynamic_cast<mtpython::tree::NumberNode*>(node);
        return space->is_true(number_node->get_value()) ? 1 : 0;
    }
    case mtpython::tree::NodeType::NT_CONST: {
        mtpython::tree::ConstNode* const_node =
            dynamic_cast<mtpython::tree::ConstNode*>(node);
        return space->is_true(const_node->get_value()) ? 1 : 0;
    }
    }

    return -1;
}

void CodeBuilder::load_const(mtpython::objects::M_BaseObject* obj)
{
    int index = add_const(obj);

    emit_op_arg(LOAD_CONST, index);
}

int CodeBuilder::opcode_stack_effect(unsigned char op, int arg)
{
    auto got = static_opcode_stack_effect.find(op);

    if (got != static_opcode_stack_effect.end()) {
        return got->second;
    }

    auto got2 = dyn_opcode_stack_effect.find(op);
    if (got2 == dyn_opcode_stack_effect.end()) {
        return 0;
    }

    return got2->second(arg);
}

void CodeBuilder::patch_jump(std::vector<CodeBlock*>& blocks)
{
    int ext_arg_count, last_ext_arg_count = 0;

    while (true) {
        ext_arg_count = 0;
        int block_off = 0;
        for (CodeBlock* block : blocks) {
            block->set_offset(block_off);
            block_off += block->code_size();
        }

        for (CodeBlock* block : blocks) {
            int offset = block->get_offset();
            for (Instruction* inst : block->get_instructions()) {
                offset += inst->size();

                if (inst->has_jump()) {
                    CodeBlock* target = inst->get_target();

                    int jump_arg = target->get_offset() -
                                   (inst->is_absolute() ? 0 : offset);
                    inst->set_arg(jump_arg);
                    if (jump_arg > 0xffff) ext_arg_count++;
                }
            }
        }

        if (ext_arg_count == last_ext_arg_count) /* code size didn't change */
            break;
        else
            last_ext_arg_count = ext_arg_count;
    }
}

int CodeBuilder::get_stacksize(std::vector<CodeBlock*>& blocks)
{
    int max_depth = 0;

    for (auto block : blocks) {
        block->set_depth(0);
    }

    for (CodeBlock* block : blocks) {
        int depth = block->get_depth();
        for (Instruction* inst : block->get_instructions()) {
            depth += opcode_stack_effect(inst->get_op(), inst->get_arg());

            if (depth > max_depth) max_depth = depth;
        }

        CodeBlock* next = block->get_next();
        if (next && next->get_depth() < depth) {
            next->set_depth(depth);
        }
    }

    return max_depth;
}

void CodeBuilder::build_lnotab(std::vector<CodeBlock*>& blocks,
                               std::vector<unsigned char>& lnotab)
{
    lnotab.clear();
    int current_line = first_lineno;
    int current_off = 0;

    for (CodeBlock* block : blocks) {
        int offset = block->get_offset();

        for (Instruction* inst : block->get_instructions()) {
            int lineno = inst->get_lineno();
            if (lineno > 0) {
                int line = lineno - current_line;
                if (line < 0) continue;

                int addr = offset - current_off;

                if (line > 0 || addr > 0) {
                    while (addr > 255) {
                        lnotab.push_back((unsigned char)255);
                        lnotab.push_back((unsigned char)0);
                        addr -= 255;
                    }

                    while (line > 255) {
                        lnotab.push_back((unsigned char)addr);
                        lnotab.push_back((unsigned char)255);
                        line -= 255;
                        addr = 0;
                    }
                }
                lnotab.push_back((unsigned char)addr);
                lnotab.push_back((unsigned char)line);

                current_line = lineno;
                current_off = offset;
            }

            offset += inst->size();
        }
    }
}

template <typename T>
static void map2vector(std::unordered_map<T, int>& map, std::vector<T>& vec,
                       int offset = 0)
{
    vec.resize(map.size());
    for (auto it = map.begin(); it != map.end(); it++) {
        vec[it->second - offset] = it->first;
    }
}

mtpython::interpreter::PyCode* CodeBuilder::build()
{
    /* add return if there is not */
    if (!current_block->has_return()) {
        load_const(space->wrap_None());
        emit_op(RETURN_VALUE);
    }

    /* update first lineno */
    if (first_lineno == -1) {
        std::vector<Instruction*> insts = first_block->get_instructions();
        if (insts.size() > 0)
            first_lineno = insts[0]->get_lineno();
        else
            first_lineno = 1;
    }

    /* build code */
    std::vector<CodeBlock*> blocks;
    first_block->get_block_list(blocks);
    patch_jump(blocks);

    /* build lnotab */
    std::vector<unsigned char> lnotab;
    build_lnotab(blocks, lnotab);

    int stacksize = get_stacksize(blocks);

    std::vector<mtpython::objects::M_BaseObject*> consts_array;
    map2vector<mtpython::objects::M_BaseObject*>(this->consts, consts_array);

    std::vector<std::string> names_array;
    map2vector<std::string>(this->names, names_array);

    std::vector<std::string> varnames_array;
    map2vector<std::string>(this->varnames, varnames_array);

    std::vector<std::string> freevars_array;
    map2vector<std::string>(this->freevars, freevars_array,
                            this->cellvars.size());

    std::vector<std::string> cellvars_array;
    map2vector<std::string>(this->cellvars, cellvars_array);

    std::vector<unsigned char> code;
    for (CodeBlock* block : blocks) {
        block->get_code(code);
    }

    return new (vm::ThreadContext::current_thread())
        mtpython::interpreter::PyCode(
            space, argcount, kwonlyargcount, varnames.size(), stacksize,
            get_code_flags(), code, consts_array, names_array, varnames_array,
            freevars_array, cellvars_array, compile_info->get_filename(), name,
            first_lineno, lnotab);
}

#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <deque>
#include <utility>

#include "parse/code_builder.h"
#include "parse/symtable.h"

namespace mtpython {
namespace parse {

typedef enum {
    F_LOOP, F_EXCEPT, F_FINALLY, F_FINALLY_END,
} FrameType;

class BaseCodeGenerator : public CodeBuilder {
private:
	virtual void compile(mtpython::tree::ASTNode* node) { }
protected:
	SymtableVisitor* symtab;
	Scope* scope;
    std::deque<std::pair<FrameType, CodeBlock*> > frame_block;

	unsigned char _binop(BinaryOper op);
    unsigned char _unaryop(UnaryOper op);

	void gen_name(const std::string& name, mtpython::tree::ExprContext ctx);

    void make_call(int n, const std::vector<mtpython::tree::ASTNode*>& args, const std::vector<mtpython::tree::KeywordNode*>& keywords);
    void make_closure(mtpython::interpreter::PyCode* code, int args, mtpython::objects::M_BaseObject* qualname);

    virtual int get_code_flags() { return 0; }
public:
	BaseCodeGenerator(const std::string& name, vm::ThreadContext* context, mtpython::tree::ASTNode* module, SymtableVisitor* symtab, int lineno, CompileInfo* info);

    void push_frame_block(FrameType type, CodeBlock* block) { frame_block.push_back(std::make_pair(type, block)); }
    void pop_frame_block() { frame_block.pop_back(); }

	/*ASTNode* visit_module(ModuleNode* node); */
    //mtpython::tree::ASTNode* visit_alias(mtpython::tree::AliasNode* node);
    mtpython::tree::ASTNode* visit_attribute(mtpython::tree::AttributeNode* node);
    //mtpython::tree::ASTNode* visit_arguments(mtpython::tree::ArgumentsNode* node);
    mtpython::tree::ASTNode* visit_assign(mtpython::tree::AssignNode* node);
    /*ASTNode* visit_augassign(AugAssignNode* node);*/
    mtpython::tree::ASTNode* visit_binop(mtpython::tree::BinOpNode* node);
    mtpython::tree::ASTNode* visit_break(mtpython::tree::BreakNode* node);
    mtpython::tree::ASTNode* visit_call(mtpython::tree::CallNode* node);
    mtpython::tree::ASTNode* visit_classdef(mtpython::tree::ClassDefNode* node);
    mtpython::tree::ASTNode* visit_compare(mtpython::tree::CompareNode* node);
    mtpython::tree::ASTNode* visit_const(mtpython::tree::ConstNode* node);
    /*ASTNode* visit_continue(ContinueNode* node);
    ASTNode* visit_delete(DeleteNode* node); */
	mtpython::tree::ASTNode* visit_expr(mtpython::tree::ExprNode* node);
    mtpython::tree::ASTNode* visit_for(mtpython::tree::ForNode* node);
    mtpython::tree::ASTNode* visit_functiondef(mtpython::tree::FunctionDefNode* node);
    mtpython::tree::ASTNode* visit_if(mtpython::tree::IfNode* node);
    mtpython::tree::ASTNode* visit_ifexp(mtpython::tree::IfExpNode* node);
    mtpython::tree::ASTNode* visit_import(mtpython::tree::ImportNode* node);
    mtpython::tree::ASTNode* visit_importfrom(mtpython::tree::ImportFromNode* node);
    void import_as(mtpython::tree::AliasNode* node);
    mtpython::tree::ASTNode* visit_keyword(mtpython::tree::KeywordNode* node);
    mtpython::tree::ASTNode* visit_list(mtpython::tree::ListNode* node);
    mtpython::tree::ASTNode* visit_name(mtpython::tree::NameNode* node);
    mtpython::tree::ASTNode* visit_number(mtpython::tree::NumberNode* node);
    mtpython::tree::ASTNode* visit_string(mtpython::tree::StringNode* node);
    mtpython::tree::ASTNode* visit_pass(mtpython::tree::PassNode* node);
    /*ASTNode* visit_raise(RaiseNode* node);*/
    mtpython::tree::ASTNode* visit_return(mtpython::tree::ReturnNode* node);
    mtpython::tree::ASTNode* visit_set(mtpython::tree::SetNode* node);
    mtpython::tree::ASTNode* visit_subscript(mtpython::tree::SubscriptNode* node);
    mtpython::tree::ASTNode* visit_try(mtpython::tree::TryNode* node);
    mtpython::tree::ASTNode* visit_try_except(mtpython::tree::TryNode* node);
    mtpython::tree::ASTNode* visit_try_finally(mtpython::tree::TryNode* node);
    mtpython::tree::ASTNode* visit_tuple(mtpython::tree::TupleNode* node);
    mtpython::tree::ASTNode* visit_unaryop(mtpython::tree::UnaryOpNode* node);
    mtpython::tree::ASTNode* visit_while(mtpython::tree::WhileNode* node);
    /*ASTNode* visit_yield(YieldNode* node);
    ASTNode* visit_yieldfrom(YieldFromNode* node);*/
};

class ModuleCodeGenerator : public BaseCodeGenerator {
private:
	void compile(mtpython::tree::ASTNode* node);
public:
	ModuleCodeGenerator(mtpython::vm::ThreadContext* context, mtpython::tree::ASTNode* module, SymtableVisitor* symtab, CompileInfo* info);
};

class FunctionCodeGenerator : public BaseCodeGenerator {
private:
    void compile(mtpython::tree::ASTNode* tree);
public:
    FunctionCodeGenerator(const std::string& name, mtpython::vm::ThreadContext* context, mtpython::tree::ASTNode* tree, SymtableVisitor* symtab, int lineno, CompileInfo* info);
};

class ClassCodeGenerator : public BaseCodeGenerator {
private:
    void compile(mtpython::tree::ASTNode* tree);
public:
    ClassCodeGenerator(const std::string& name, mtpython::vm::ThreadContext* context, mtpython::tree::ASTNode* tree, SymtableVisitor* symtab, int lineno, CompileInfo* info);
};

}
}

#endif

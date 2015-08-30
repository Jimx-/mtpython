#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include <stack>
#include <utility>

#include "parse/code_builder.h"
#include "parse/symtable.h"

namespace mtpython {
namespace parse {

typedef enum {
    F_LOOP,
} FrameType;

class BaseCodeGenerator : public CodeBuilder {
private:
	virtual void compile(mtpython::tree::ASTNode* node) { }
protected:
	SymtableVisitor* symtab;
	Scope* scope;
    std::stack<std::pair<FrameType, CodeBlock*> > frame_block;

	char _binop(BinaryOper op);
	void gen_name(const std::string& name, mtpython::tree::ExprContext ctx);

    void make_closure(mtpython::interpreter::PyCode* code, int args, mtpython::objects::M_BaseObject* qualname);

    virtual int get_code_flags() { return 0; }
public:
	BaseCodeGenerator(const std::string& name, mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* module, SymtableVisitor* symtab, int lineno, CompileInfo* info);

    void push_frame_block(FrameType type, CodeBlock* block) { frame_block.push(std::make_pair(type, block)); }
    void pop_frame_block() { frame_block.pop(); }

	/*virtual ASTNode* visit_module(ModuleNode* node); */
    //virtual mtpython::tree::ASTNode* visit_arguments(mtpython::tree::ArgumentsNode* node);
    virtual mtpython::tree::ASTNode* visit_assign(mtpython::tree::AssignNode* node);
    /*virtual ASTNode* visit_augassign(AugAssignNode* node);*/
    virtual mtpython::tree::ASTNode* visit_binop(mtpython::tree::BinOpNode* node);
    /*virtual ASTNode* visit_break(BreakNode* node); */
    virtual mtpython::tree::ASTNode* visit_call(mtpython::tree::CallNode* node);
    virtual mtpython::tree::ASTNode* visit_compare(mtpython::tree::CompareNode* node);
    virtual mtpython::tree::ASTNode* visit_const(mtpython::tree::ConstNode* node);
    /*virtual ASTNode* visit_continue(ContinueNode* node);
    virtual ASTNode* visit_delete(DeleteNode* node); */
	virtual mtpython::tree::ASTNode* visit_expr(mtpython::tree::ExprNode* node);
    virtual mtpython::tree::ASTNode* visit_for(mtpython::tree::ForNode* node);
    virtual mtpython::tree::ASTNode* visit_functiondef(mtpython::tree::FunctionDefNode* node);
    virtual mtpython::tree::ASTNode* visit_if(mtpython::tree::IfNode* node);
    virtual mtpython::tree::ASTNode* visit_ifexp(mtpython::tree::IfExpNode* node);
    virtual mtpython::tree::ASTNode* visit_keyword(mtpython::tree::KeywordNode* node);
    virtual mtpython::tree::ASTNode* visit_name(mtpython::tree::NameNode* node);
    virtual mtpython::tree::ASTNode* visit_number(mtpython::tree::NumberNode* node);
    virtual mtpython::tree::ASTNode* visit_string(mtpython::tree::StringNode* node);
    virtual mtpython::tree::ASTNode* visit_pass(mtpython::tree::PassNode* node);
    /*virtual ASTNode* visit_raise(RaiseNode* node);*/
    virtual mtpython::tree::ASTNode* visit_return(mtpython::tree::ReturnNode* node);
    virtual mtpython::tree::ASTNode* visit_tuple(mtpython::tree::TupleNode* node);
    /*virtual ASTNode* visit_unaryop(UnaryOpNode* node); */
    virtual mtpython::tree::ASTNode* visit_while(mtpython::tree::WhileNode* node);
    /*virtual ASTNode* visit_yield(YieldNode* node);
    virtual ASTNode* visit_yieldfrom(YieldFromNode* node);*/
};

class ModuleCodeGenerator : public BaseCodeGenerator {
private:
	virtual void compile(mtpython::tree::ASTNode* node);
public:
	ModuleCodeGenerator(mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* module, SymtableVisitor* symtab, CompileInfo* info);
};

class FunctionCodeGenerator : public BaseCodeGenerator {
private:
    int argcount;
    virtual void compile(mtpython::tree::ASTNode* tree);
public:
    FunctionCodeGenerator(const std::string& name, mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* tree, SymtableVisitor* symtab, int lineno, CompileInfo* info);
};

}
}

#endif

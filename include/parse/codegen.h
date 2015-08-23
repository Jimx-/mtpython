#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "parse/code_builder.h"
#include "parse/symtable.h"

namespace mtpython {
namespace parse {

class BaseCodeGenerator : public CodeBuilder {
private:
	virtual void compile(mtpython::tree::ASTNode* node) { }
protected:
	SymtableVisitor* symtab;
	Scope* scope;

	char _binop(BinaryOper op);
	void gen_name(std::string& name, mtpython::tree::ExprContext ctx);

    virtual int get_code_flags() { return 0; }
public:
	BaseCodeGenerator(std::string& name, mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* module, SymtableVisitor* symtab, int lineno, CompileInfo* info);

	/*virtual ASTNode* visit_module(ModuleNode* node); */
    //virtual mtpython::tree::ASTNode* visit_arguments(mtpython::tree::ArgumentsNode* node);
    virtual mtpython::tree::ASTNode* visit_assign(mtpython::tree::AssignNode* node);
    /*virtual ASTNode* visit_augassign(AugAssignNode* node);*/
    virtual mtpython::tree::ASTNode* visit_binop(mtpython::tree::BinOpNode* node);
    /*virtual ASTNode* visit_break(BreakNode* node); */
    virtual mtpython::tree::ASTNode* visit_call(mtpython::tree::CallNode* node);
    /*virtual ASTNode* visit_compare(CompareNode* node);
    virtual ASTNode* visit_continue(ContinueNode* node);
    virtual ASTNode* visit_delete(DeleteNode* node);
    virtual ASTNode* visit_for(ForNode* node);*/
    //virtual mtpython::tree::ASTNode* visit_functiondef(mtpython::tree::FunctionDefNode* node);
    /*virtual ASTNode* visit_if(IfNode* node);
    virtual ASTNode* visit_ifexp(IfExpNode* node);*/
    virtual mtpython::tree::ASTNode* visit_name(mtpython::tree::NameNode* node);
    virtual mtpython::tree::ASTNode* visit_number(mtpython::tree::NumberNode* node);
    /*virtual ASTNode* visit_pass(PassNode* node);
    virtual ASTNode* visit_raise(RaiseNode* node);
    virtual ASTNode* visit_return(ReturnNode* node);
    virtual ASTNode* visit_tuple(TupleNode* node);
    virtual ASTNode* visit_unaryop(UnaryOpNode* node);
    virtual ASTNode* visit_while(WhileNode* node);
    virtual ASTNode* visit_yield(YieldNode* node);
    virtual ASTNode* visit_yieldfrom(YieldFromNode* node);*/
};

class ModuleCodeGenerator : public BaseCodeGenerator {
private:
	virtual void compile(mtpython::tree::ASTNode* node);
public:
	ModuleCodeGenerator(mtpython::objects::ObjSpace* space, mtpython::tree::ASTNode* module, SymtableVisitor* symtab, CompileInfo* info);
};

}
}

#endif

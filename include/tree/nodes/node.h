#ifndef _NODE_H_
#define _NODE_H_

#include <cstddef>
#include "exceptions.h"

namespace mtpython {
namespace tree {

#define error_tree nullptr

typedef enum {
    NT_EMPTY,
    NT_IF,
    NT_FOR,
    NT_ASSIGN,
    NT_AUGASSIGN,
    NT_WHILE,
    NT_TRY,
    NT_CATCH,
    NT_FINALLY,
    NT_RETURN,
    NT_BREAK,
    NT_CONTINUE,
    NT_IDENT,
    NT_UNARY,
    NT_ARGUMENTS,
    NT_CALL,
    NT_MODULE,
    NT_FUNCDEF,
    NT_NUMBER,
    NT_BINOP,
    NT_COMPARE,
    NT_IFEXP,
    NT_TUPLE,
    NT_DELETE,
    NT_YIELD,
    NT_YIELDFROM,
    NT_RAISE,
    NT_PASS,
    NT_KEYWORD,
    NT_STRING,
    NT_EXPR,
    NT_CONST,
    NT_EXCEPTHANDLER,
    NT_ATTRIBUTE,
    NT_ALIAS,
    NT_IMPORT,
    NT_COMPREHENSION,
    NT_GENERATOREXP,
    NT_SUBSCRIPT,
    NT_INDEX,
    NT_LIST,
    NT_IMPORTFROM,
    NT_DICT,
    NT_SET,
    NT_STARRED,
    NT_WITH,
    NT_WITHITEM,
    NT_CLASSDEF,
    NT_LAMBDA,
    NT_SLICE,
    NT_GLOBAL,
    NT_ASSERT,
} NodeType;

class ASTVisitor;

class ASTNode {
protected:
    int line; // line nr

    ASTNode* sibling; // sibling node

public:
    ASTNode(const int line_nr) : line(line_nr) { sibling = nullptr; }
    ~ASTNode() {}

    int get_line() { return line; }
    ASTNode* get_sibling() { return this->sibling; }
    void set_sibling(ASTNode* sbl) { this->sibling = sbl; }
    virtual void set_context(ExprContext ctx) {}
    virtual NodeType get_tag() { return NT_EMPTY; }
    virtual void print(const int padding) {}

    virtual void visit(ASTVisitor* visitor) {}

    int length()
    {
        int ret = 0;
        ASTNode* tree = this;
        while (tree) {
            ret++;
            tree = tree->sibling;
        }
        return ret;
    }
};

class ModuleNode;
class AliasNode;
class AttributeNode;
class ArgumentsNode;
class AssertNode;
class AssignNode;
class AugAssignNode;
class BinOpNode;
class BoolOpNode;
class BreakNode;
class CallNode;
class ClassDefNode;
class CompareNode;
class ComprehensionNode;
class ConstNode;
class ContinueNode;
class DeleteNode;
class DictNode;
class ExceptHandlerNode;
class ExprNode;
class ForNode;
class FunctionDefNode;
class GeneratorExpNode;
class GlobalNode;
class IfNode;
class IfExpNode;
class ImportNode;
class ImportFromNode;
class IndexNode;
class KeywordNode;
class LambdaNode;
class ListNode;
class NameNode;
class NumberNode;
class StringNode;
class SubscriptNode;
class PassNode;
class RaiseNode;
class ReturnNode;
class SetNode;
class SliceNode;
class StarredNode;
class TryNode;
class TupleNode;
class UnaryOpNode;
class WhileNode;
class WithNode;
class WithItemNode;
class YieldNode;
class YieldFromNode;

class ASTVisitor {
public:
    virtual void visit_sequence(ASTNode* seq)
    {
        ASTNode* node = seq;
        while (node) {
            node->visit(this);
            node = node->get_sibling();
        }
    }

    virtual ASTNode* visit_module(ModuleNode* node) { return nullptr; }
    virtual ASTNode* visit_alias(AliasNode* node) { return nullptr; }
    virtual ASTNode* visit_attribute(AttributeNode* node) { return nullptr; }
    virtual ASTNode* visit_arguments(ArgumentsNode* node) { return nullptr; }
    virtual ASTNode* visit_assert(AssertNode* node) { return nullptr; }
    virtual ASTNode* visit_assign(AssignNode* node) { return nullptr; }
    virtual ASTNode* visit_augassign(AugAssignNode* node) { return nullptr; }
    virtual ASTNode* visit_binop(BinOpNode* node) { return nullptr; }
    virtual ASTNode* visit_boolop(BoolOpNode* node) { return nullptr; }
    virtual ASTNode* visit_break(BreakNode* node) { return nullptr; }
    virtual ASTNode* visit_call(CallNode* node) { return nullptr; }
    virtual ASTNode* visit_classdef(ClassDefNode* node) { return nullptr; }
    virtual ASTNode* visit_compare(CompareNode* node) { return nullptr; }
    virtual ASTNode* visit_comprehension(ComprehensionNode* node)
    {
        return nullptr;
    }
    virtual ASTNode* visit_const(ConstNode* node) { return nullptr; }
    virtual ASTNode* visit_continue(ContinueNode* node) { return nullptr; }
    virtual ASTNode* visit_delete(DeleteNode* node) { return nullptr; }
    virtual ASTNode* visit_dict(DictNode* node) { return nullptr; }
    virtual ASTNode* visit_excepthandler(ExceptHandlerNode* node)
    {
        return nullptr;
    }
    virtual ASTNode* visit_expr(ExprNode* node) { return nullptr; }
    virtual ASTNode* visit_for(ForNode* node) { return nullptr; }
    virtual ASTNode* visit_functiondef(FunctionDefNode* node)
    {
        return nullptr;
    }
    virtual ASTNode* visit_generatorexp(GeneratorExpNode* node)
    {
        return nullptr;
    }
    virtual ASTNode* visit_global(GlobalNode* node) { return nullptr; }
    virtual ASTNode* visit_if(IfNode* node) { return nullptr; }
    virtual ASTNode* visit_ifexp(IfExpNode* node) { return nullptr; }
    virtual ASTNode* visit_index(IndexNode* node) { return nullptr; }
    virtual ASTNode* visit_import(ImportNode* node) { return nullptr; }
    virtual ASTNode* visit_importfrom(ImportFromNode* node) { return nullptr; }
    virtual ASTNode* visit_keyword(KeywordNode* node) { return nullptr; }
    virtual ASTNode* visit_lambda(LambdaNode* node) { return nullptr; }
    virtual ASTNode* visit_list(ListNode* node) { return nullptr; }
    virtual ASTNode* visit_name(NameNode* node) { return nullptr; }
    virtual ASTNode* visit_string(StringNode* node) { return nullptr; }
    virtual ASTNode* visit_number(NumberNode* node) { return nullptr; }
    virtual ASTNode* visit_pass(PassNode* node) { return nullptr; }
    virtual ASTNode* visit_raise(RaiseNode* node) { return nullptr; }
    virtual ASTNode* visit_return(ReturnNode* node) { return nullptr; }
    virtual ASTNode* visit_set(SetNode* node) { return nullptr; }
    virtual ASTNode* visit_slice(SliceNode* node) { return nullptr; }
    virtual ASTNode* visit_starred(StarredNode* node) { return nullptr; }
    virtual ASTNode* visit_subscript(SubscriptNode* node) { return nullptr; }
    virtual ASTNode* visit_try(TryNode* node) { return nullptr; }
    virtual ASTNode* visit_tuple(TupleNode* node) { return nullptr; }
    virtual ASTNode* visit_unaryop(UnaryOpNode* node) { return nullptr; }
    virtual ASTNode* visit_while(WhileNode* node) { return nullptr; }
    virtual ASTNode* visit_with(WithNode* node) { return nullptr; }
    virtual ASTNode* visit_withitem(WithItemNode* node) { return nullptr; }
    virtual ASTNode* visit_yield(YieldNode* node) { return nullptr; }
    virtual ASTNode* visit_yieldfrom(YieldFromNode* node) { return nullptr; }
};

} // namespace tree
} // namespace mtpython

#endif /* _NODE_H_ */

#ifndef EXP_HPP
#define EXP_HPP

#include <string>
#include <vector>

namespace EXP
{

#define EXP_SYM_OPEN '('
#define EXP_SYM_CLOSE ')'
#define EXP_SYM_CONST0 '0'
#define EXP_SYM_CONST1 '1'
#define EXP_SYM_NEG '!'
#define EXP_SYM_NEGAFT '\''
#define EXP_SYM_AND '&'
#define EXP_SYM_AND2 '*'
#define EXP_SYM_XOR '^'
#define EXP_SYM_OR '|'
#define EXP_SYM_OR2 '+'

    const int EXPR_ITEM_MAX = 8;

    enum Unate
    {
        VACUOUS = 0,
        POS,
        NEG,
        MIX
    };

    class ExpItem;
    class Expression;

    class ExpItem
    {
        // friend void strExprToExpTree(const std::string& strExpr, ExpItem*& exp, std::vector<std::string>& items);
    public:
        ExpItem(unsigned int item);
        ExpItem(unsigned int item, ExpItem *left, ExpItem *right);
        ~ExpItem();

        ExpItem *getLeft() const { return m_left; }
        ExpItem *getRight() const { return m_right; }

        std::string text(std::vector<std::string> names) const;

    private:
        ExpItem *m_left = nullptr;
        ExpItem *m_right = nullptr;

        bool m_is_operator = false;
        unsigned int m_content;

        Expression *m_expr = nullptr;
    };

    class Expression
    {
    public:
        static Expression *buildExpression(const std::string &strExpr);
        std::string text() const { return m_root->text(m_leaf_names); }

    private:
        ExpItem *m_root = nullptr;
        std::vector<std::string> m_leaf_names;
    };

}
#endif // EXP_HPP
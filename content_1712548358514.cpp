#include "expression.h"
#include <map>
#include <stack>
#include <vector>
#include <iostream>
#include <string>

namespace EXP
{
    ExpItem::ExpItem(unsigned int item)
    {
        if (item > 32)
        {
            if (item != EXP_SYM_CONST0 && item != EXP_SYM_CONST1)
                m_is_operator = true;
        }
    }

    ExpItem::ExpItem(unsigned int item, ExpItem *left, ExpItem *right)
    {
        m_left = left;
        m_right = right;
        if (item > 32)
        {
            if (item != EXP_SYM_CONST0 && item != EXP_SYM_CONST1)
                m_is_operator = true;
        }
    }

    ExpItem::~ExpItem()
    {
        if (m_left != NULL)
        {
            delete m_left;
            m_left = NULL;
        }
        if (m_right != NULL)
        {
            delete m_right;
            m_right = NULL;
        }
    }

    std::string ExpItem::text(std::vector<std::string> names) const
    {
        std::string strText;
        if (m_is_operator)
        {
            strText = std::string(1, m_content);
            std::string left = m_left ? m_left->text(names) : "";
            std::string right = m_right ? m_right->text(names) : "";
            if (m_content == EXP_SYM_NEG)
                strText = "(" + strText + left + ")";
            else
                strText = "(" + left + strText + right + ")";
        }
        else if (m_content == EXP_SYM_CONST0 || m_content == EXP_SYM_CONST1)
        {
            strText = std::string(1, m_content);
        }
        else
        {
            strText = names[m_content];
        }
        return strText;
    }

    void strExprToExpTree(const std::string &strExpr, ExpItem *&root, std::vector<std::string> &itemNames)
    {
        int EXP_FLAG_START = 1; // after the opening parenthesis
        int EXP_FLAG_VAR = 2;   // after operation is received
        int EXP_FLAG_OPER = 3;  // after operation symbol is received
        int EXP_FLAG_ERROR = 4; // when error is detected

        std::map<char, int> priority = {{EXP_SYM_OR, 1}, {EXP_SYM_OR2, 1}, {EXP_SYM_XOR, 2}, {EXP_SYM_AND, 3}, {EXP_SYM_AND2, 3}, {EXP_SYM_NEG, 4}, {EXP_SYM_NEGAFT, 4}};
        std::stack<char> operStack;
        std::stack<ExpItem *> itemStack;
        std::string str = "(" + strExpr + ")";

        auto popOper = [&]()
        {
            bool bPopSucc = true;
            if (!operStack.empty())
            {
                char oper = operStack.top();
                switch (oper)
                {
                case EXP_SYM_NEG:
                case EXP_SYM_NEGAFT:
                {
                    if (!itemStack.empty())
                    {
                        operStack.pop();
                        ExpItem *lChild = itemStack.top();
                        itemStack.pop();
                        ExpItem *pOper = new ExpItem(oper, lChild, nullptr);
                        itemStack.push(pOper);
                    }
                    else
                    {
                        bPopSucc = false;
                        std::cout << "error: " << oper << " has no left child" << std::endl;
                    }
                }
                break;
                case EXP_SYM_AND:
                case EXP_SYM_AND2:
                case EXP_SYM_XOR:
                case EXP_SYM_OR:
                case EXP_SYM_OR2:
                {
                    if (itemStack.size() >= 2)
                    {
                        operStack.pop();
                        ExpItem *rChild = itemStack.top();
                        itemStack.pop();
                        ExpItem *lChild = itemStack.top();
                        itemStack.pop();
                        ExpItem *pOper = new ExpItem(oper, lChild, rChild);
                        itemStack.push(pOper);
                    }
                    else
                    {
                        bPopSucc = false;
                        std::cout << "error: " << oper << " missing variable" << std::endl;
                    }
                }
                break;
                default:
                {
                    bPopSucc = false;
                    std::cout << "error: got invalid operator " << oper << std::endl;
                }
                break;
                }
            }
            return bPopSucc;
        };

        auto pushOper = [&](char oper)
        {
            bool bPushSucc = true;
            if (priority.find(oper) != priority.end())
            {
                while (!operStack.empty() && operStack.top() != EXP_SYM_OPEN && priority[operStack.top()] >= priority[oper])
                {
                    if (!popOper())
                    {
                        bPushSucc = false;
                        break;
                    }
                }
                if (bPushSucc)
                    operStack.push(oper);
            }
            else
            {
                bPushSucc = false;
                std::cout << "error: got invalid operator " << oper << std::endl;
            }
            return bPushSucc;
        };

        int flag = EXP_FLAG_START;
        for (size_t idx = 0; idx < str.size(); ++idx)
        {
            switch (str[idx])
            {
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                continue;
            case EXP_SYM_CONST0:
                if (flag == EXP_FLAG_VAR)
                {
                    flag = EXP_FLAG_ERROR;
                    break;
                }
                flag = EXP_FLAG_VAR;
                itemStack.push(new ExpItem(EXP_SYM_CONST0));
                break;
            case EXP_SYM_CONST1:
                if (flag == EXP_FLAG_VAR)
                {
                    flag = EXP_FLAG_ERROR;
                    break;
                }
                flag = EXP_FLAG_VAR;
                itemStack.push(new ExpItem(EXP_SYM_CONST1));
                break;
            case EXP_SYM_NEG:
                if (flag == EXP_FLAG_VAR)
                { // if NEGBEF follows a variable, AND is assumed
                    if (pushOper(EXP_SYM_AND))
                        flag = EXP_FLAG_OPER;
                    else
                    {
                        flag = EXP_FLAG_ERROR;
                        break;
                    }
                }
                flag = pushOper(EXP_SYM_NEG) ? flag : EXP_FLAG_ERROR;
                break;
            case EXP_SYM_NEGAFT:
                if (flag != EXP_FLAG_VAR)
                {
                    flag = EXP_FLAG_ERROR;
                    break;
                }
                else
                {
                    flag = pushOper(EXP_SYM_NEGAFT) ? flag : EXP_FLAG_ERROR;
                }
                break;
            case EXP_SYM_AND:
            case EXP_SYM_AND2:
            case EXP_SYM_OR:
            case EXP_SYM_OR2:
            case EXP_SYM_XOR:
                if (flag != EXP_FLAG_VAR)
                {
                    flag = EXP_FLAG_ERROR;
                    break;
                }
                else
                {
                    if (!pushOper(str[idx]))
                    {
                        flag = EXP_FLAG_ERROR;
                        break;
                    }
                }
                flag = EXP_FLAG_OPER;
                break;
            case EXP_SYM_OPEN:
                if (flag == EXP_FLAG_VAR)
                {
                    if (!pushOper(EXP_SYM_AND))
                    {
                        flag = EXP_FLAG_ERROR;
                        break;
                    }
                }
                operStack.push(EXP_SYM_OPEN);
                flag = EXP_FLAG_START;
                break;
            case EXP_SYM_CLOSE:
                while (!operStack.empty() && operStack.top() != EXP_SYM_OPEN)
                {
                    if (!popOper())
                        break;
                }
                if (operStack.empty() ||
                    operStack.top() != EXP_SYM_OPEN)
                {
                    flag = EXP_FLAG_ERROR;
                }
                else
                {
                    operStack.pop();
                    flag = EXP_FLAG_VAR;
                }
                break;
            default:
                size_t i = idx;
                std::string itemName = "";
                for (; i < str.size() &&
                       str[i] != ' ' && str[i] != '\t' && str[i] != '\r' && str[i] != '\n' &&
                       str[i] != EXP_SYM_AND && str[i] != EXP_SYM_AND2 && str[i] != EXP_SYM_OR && str[i] != EXP_SYM_OR2 &&
                       str[i] != EXP_SYM_XOR && str[i] != EXP_SYM_NEGAFT && str[i] != EXP_SYM_CLOSE;
                     i++)
                {
                    if (str[i] == EXP_SYM_NEG || str[i] == EXP_SYM_OPEN)
                    {
                        flag = EXP_FLAG_ERROR;
                        break;
                    }
                    itemName += str[i];
                }
                if (flag != EXP_FLAG_ERROR)
                    idx = i - 1;
                if (flag == EXP_FLAG_VAR)
                {
                    if (!pushOper(EXP_SYM_AND))
                    {
                        flag = EXP_FLAG_ERROR;
                        break;
                    }
                }
                unsigned int varIdx = itemName.size();
                for (size_t nameIdx = 0; nameIdx < itemName.size(); nameIdx++)
                {
                    if (itemNames[nameIdx] == itemName)
                    {
                        varIdx = nameIdx;
                        break;
                    }
                }
                if (varIdx < itemNames.size())
                    itemStack.push(new ExpItem(varIdx));
                else
                {
                    itemNames.emplace_back(itemName);
                    itemStack.push(new ExpItem(varIdx));
                }
                flag = EXP_FLAG_VAR;
                break;
            }
            if (flag == EXP_FLAG_ERROR)
                break;
        }
        if (flag != EXP_FLAG_ERROR && itemStack.size() == 1)
            root = itemStack.top();
        else
        {
            if (root)
                root = nullptr;
            while (!itemStack.empty())
            {
                delete itemStack.top();
                itemStack.pop();
            }
            std::cout << "Error: Invalid expression " << str << std::endl;
        }
    }

    Expression *Expression::buildExpression(const std::string &strExpr)
    {
        Expression *expr = nullptr;

        ExpItem *root = nullptr;
        std::vector<std::string> itemNames;
        strExprToExpTree(strExpr, root, itemNames);
        if (root && itemNames.size() < 32)
        {
            expr = new Expression();
            expr->m_root = root;
            expr->m_leaf_names = itemNames;
        }
        else
        {
            delete root;
        }

        return expr;
    }
}
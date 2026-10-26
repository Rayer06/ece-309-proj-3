// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include <stack>

static int precedence(CalcToken::Kind const op) {
    if (op == CalcToken::Kind::Times) {return 2;}
    if (op == CalcToken::Kind::Divide) {return 2;}
    if (op == CalcToken::Kind::Plus) {return 1;}
    if (op == CalcToken::Kind::Minus) {return 1;}
    return 0;
}

static int apply_op(std::stack<double>& values, std::stack<CalcToken::Kind>& ops) {
    CalcToken::Kind op = ops.top(); ops.pop();
    double rhs = values.top(); values.pop();
    double lhs = values.top(); values.pop();
    if (op == CalcToken::Kind::Plus) {values.push(lhs+rhs);}
    if (op == CalcToken::Kind::Minus) {values.push(lhs-rhs);}
    if (op == CalcToken::Kind::Times) {values.push(lhs*rhs);}
    if (op == CalcToken::Kind::Divide) {
        if (rhs == 0) {
            return 0;
        }
        values.push(lhs/rhs);
    }
    return 1;
}

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {
    std::stack<double> values;
    std::stack<CalcToken::Kind> ops;
    for (CalcToken c : tokens) {
        if (c.kind == CalcToken::Kind::Number) {
            values.push(c.value);
        }
        else if (c.kind == CalcToken::Kind::LParen) {
            ops.push(c.kind);
        }
        else if (c.kind == CalcToken::Kind::RParen) {
            while (!ops.empty() && (ops.top() != CalcToken::Kind::LParen)) {
                if (!apply_op(values, ops)) {
                    *error = "division by zero";
                    return false;
                }
            }
            ops.pop(); // pop the (
        }
        else {
            if (!ops.empty() && (ops.top() != CalcToken::Kind::LParen)) {
                if (precedence(ops.top()) >= precedence(c.kind)) {
                    if (!apply_op(values, ops)) {
                        *error = "division by zero";
                        return false;
                    }
                }
            }
            ops.push(c.kind);
        }
    }
    while (!ops.empty()) {
        if (!apply_op(values, ops)) {
            *error = "division by zero";
            return false;
        }
    }
    *out = values.top();
    return true;
}
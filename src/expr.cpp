#include <iostream>
#include "expr.h"

namespace cassm
{

// ----------------------------------------------------------------------------
//      ExprNode
// ----------------------------------------------------------------------------

class ExprNode
{
public:
  virtual ~ExprNode() = default;

  virtual int value() const { return -1; }
  virtual int checkValue() const { return value(); }
  virtual OwnedExprNode eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined) { return nullptr; }
  virtual void dump(std::ostream& s, int level = 0) const noexcept = 0;

protected:
  void indent(std::ostream& s, int level) const;
};

void ExprNode::indent(std::ostream& s, int level) const
{
  while (level > 0)
  {
    s << ' ';
    -- level;
  }
}

// ----------------------------------------------------------------------------
//      ConstantNode
// ----------------------------------------------------------------------------

class ConstantNode : public ExprNode
{
public:
  ConstantNode(int value) : value_(value) { }
  int value() const override { return value_; }
  int checkValue() const override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;

private:
  int value_;
};

int ConstantNode::checkValue() const
{
  int value = this->value();
  if (value < 0 || value > 0xffff)
    throwSourceError("Invalid expression result (%d); expected a number between 0 and 65535", value);
  return value;
}

void ConstantNode::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Constant: " << value_ << std::endl;
}

// ----------------------------------------------------------------------------
//      SymbolNode
// ----------------------------------------------------------------------------

class SymbolNode : public ExprNode
{
public:
  SymbolNode(const std::string& name) : name_(name) { }

  OwnedExprNode eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined) override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;

private:
  std::string name_;
};

OwnedExprNode SymbolNode::eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined)
{
  auto *value = symbols.get(name_);
  if (! value)
  {
    if (! throwUndefined)
      return nullptr;
    throwSourceError("Undefined symbol '%s'", name_.c_str());
  }
  return std::make_unique<ConstantNode>(*value);
}

void SymbolNode::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Symbol: " << name_ << std::endl;
}

// ----------------------------------------------------------------------------
//      ProgramCounterNode
// ----------------------------------------------------------------------------

class ProgramCounterNode : public ExprNode
{
public:
  OwnedExprNode eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined) override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;
};

OwnedExprNode ProgramCounterNode::eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined)
{
  return std::make_unique<ConstantNode>(pc);
}

void ProgramCounterNode::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Program Counter" << std::endl;
}

// ----------------------------------------------------------------------------
//      ArithmeticNode
// ----------------------------------------------------------------------------

class ArithmeticNode : public ExprNode
{
public:
  using Handler = int(*)(int, int);

  ArithmeticNode(OwnedExprNode left, OwnedExprNode right, char op, Handler handler)
    : left_(std::move(left)), right_(std::move(right)), op_(op), handler_(handler) { }

  OwnedExprNode eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined) override;
  void dump(std::ostream& s, int indent = 0) const noexcept override;

private:
  OwnedExprNode left_;
  OwnedExprNode right_;
  char op_;
  Handler handler_;
};

OwnedExprNode ArithmeticNode::eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined)
{
  auto left = left_->eval(pc, symbols, throwUndefined);
  auto right = right_->eval(pc, symbols, throwUndefined);
  if (left)
    left_ = std::move(left);
  if (right)
    right_ = std::move(right);

  auto a = left_->value(), b = right_->value();
  if (a != -1 && b != -1)
    return std::make_unique<ConstantNode>(handler_(a, b));

  return nullptr;
}

void ArithmeticNode::dump(std::ostream& s, int level) const noexcept
{
  indent(s, level);
  s << "Operator: " << op_ << std::endl;
  left_->dump(s, level + 2);
  right_->dump(s, level + 2);
}

// ----------------------------------------------------------------------------
//      Expression
// ----------------------------------------------------------------------------

Expression::~Expression()
{
}

Expression::Expression(LineReader& reader)
{
  root_ = parseOperand(reader);
  Token token;
  while ((token = reader.nextToken()).type == TokenType::Punctuator)
  {
    auto op = token.punctuator;
    switch (op)
    {
      case '+':
        root_ = std::make_unique<ArithmeticNode>(std::move(root_), parseOperand(reader), op, [](int a, int b) { return a + b; });
        break;

      case '-':
        root_ = std::make_unique<ArithmeticNode>(std::move(root_), parseOperand(reader), op, [](int a, int b) { return a - b; });
        break;

      case '*':
        root_ = std::make_unique<ArithmeticNode>(std::move(root_), parseOperand(reader), op, [](int a, int b) { return a * b; });
        break;

      case '/':
        // TODO: check for divide by zero in the handler
        root_ = std::make_unique<ArithmeticNode>(std::move(root_), parseOperand(reader), op, [](int a, int b) { return a / b; });
        break;

      default:
        return;                   // TODO:
    }
  }
}

int Expression::eval(Address pc, const SymbolTable<uint16_t>& symbols, bool throwUndefined)
{
  auto root = root_->eval(pc, symbols, throwUndefined);
  if (root)
    root_ = std::move(root);
  return root_->checkValue();
}

void Expression::dump(std::ostream& s, int level)
{
  root_->dump(s, level);
}

OwnedExprNode Expression::parseOperand(LineReader& reader)
{
  auto token = reader.nextToken();
  if (token.type == TokenType::Number)
    return std::make_unique<ConstantNode>(token.number);
  if (token.type == TokenType::Identifier)
    return std::make_unique<SymbolNode>(token.text);
  if (token.type == TokenType::Literal)
  {
    // TODO: Expect a single character and return its PETSCII value
    return std::make_unique<ConstantNode>(0);
  }
  if (token.type == TokenType::Punctuator)
  {
    switch (token.punctuator)
    {
      case '*':
        return std::make_unique<ProgramCounterNode>();

      case '@':
        // TODO: screen code
        return std::make_unique<ConstantNode>(0);

      default:
        throwSourceError("Unexpected character ('%c')", token.punctuator);
    }
  }

  throwSourceError("Expected a valid operand");
}

}

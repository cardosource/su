#include "stmt.hpp"
#include "visitor.hpp"
#include <any>
#include <memory>

namespace Statement{
  Block::Block(std::vector<std::shared_ptr<Stmt>> statements): statements(statements){}

  std::any Block::accept(StmtVisitor &visitor){
    return visitor.visitBlockStmt(shared_from_this());
  }

  If::If(std::shared_ptr<Expr> condition, std::shared_ptr<Stmt> thenBranch, std::shared_ptr<Stmt> elseBranch)
    : condition{std::move(condition)}, thenBranch{std::move(thenBranch)}, elseBranch{std::move(elseBranch)} {}

  std::any If::accept(StmtVisitor &visitor){
    return visitor.visitIfStmt(shared_from_this());
  }

While::While(std::shared_ptr<Expr> condition,std::shared_ptr<Stmt> body):
condition{std::move(condition)}, body{std::move(body)}{}
 std::any While::accept(StmtVisitor &visitor){
 return visitor.visitWhileStmt(shared_from_this());
  }
}

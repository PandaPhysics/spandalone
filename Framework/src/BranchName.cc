#include "../interface/BranchName.h"

panda::utils::BranchName&
panda::utils::BranchName::operator=(BranchName const& _rhs)
{
  obj_ = _rhs.obj_;
  attr_ = _rhs.attr_;
  isVeto_ = _rhs.isVeto_;
  return *this;
}

bool
panda::utils::BranchName::match(BranchName const& _rhs) const
{
  // Either of the names is null. -> no match
  if (this->isNull() || _rhs.isNull())
    return false;

  if (this->obj_.Length() == 0 && _rhs.obj_.Length() == 0) {
    // Neither names an object. Simple match of attribute names
    return (this->attr_ == _rhs.attr_ || this->attr_ == "*" || _rhs.attr_ == "*");
  }
  else {
    // Object names don't match and neither is a wildcard (including cases where either obj is empty) -> no match
    if (this->obj_ != _rhs.obj_ && this->obj_ != "*" && _rhs.obj_ != "*")
      return false;

    // Either this or RHS only names the object -> match
    if (this->attr_.Length() == 0 || _rhs.attr_.Length() == 0)
      return true;

    // Match attribute names
    return (this->attr_ == _rhs.attr_ || this->attr_ == "*" || _rhs.attr_ == "*");
      return true;
  }
}

std::ostream&
operator<<(std::ostream& os, panda::utils::BranchName const& bn)
{
  return (os << (bn.isVeto() ? "!" : "") << bn.toString());
}

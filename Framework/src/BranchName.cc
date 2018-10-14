#include "../interface/BranchName.h"

panda::utils::BranchName&
panda::utils::BranchName::operator=(BranchName const& _rhs)
{
  core_type::operator=(_rhs);
  isVeto_ = _rhs.isVeto_;
  return *this;
}

bool
panda::utils::BranchName::match(BranchName const& _rhs) const
{
  // case 1. Either of the names is null. -> no match
  if (this->first.Length() == 0 || _rhs.first.Length() == 0)
    return false;

  // case 2. Object names don't match and neither is a wildcard -> no match
  if (this->first != _rhs.first && this->first != "*" && _rhs.first != "*")
    return false;

  // case 3. Either this or RHS only names the object -> match
  if (this->second.Length() == 0 || _rhs.second.Length() == 0)
    return true;

  std::cout << "Checking match between " << toString() << " and " << _rhs.toString() << std::endl;

  // case 4. Either this or RHS has a wildcard branch name -> match
  if (this->second == "*" || _rhs.second == "*")
    return true;

  // case 5. Full comparison
  return this->second == _rhs.second;
}

std::ostream&
operator<<(std::ostream& os, panda::utils::BranchName const& bn)
{
  return (os << bn.toString());
}

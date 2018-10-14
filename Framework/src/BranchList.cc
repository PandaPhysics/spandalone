#include "../interface/BranchList.h"

#include <stdexcept>

panda::utils::BranchList::BranchList(std::initializer_list<TString> _il)
{
  if (_il.size() != 1)
    throw std::runtime_error("BranchList initialization only allowed for with match-all expression");

  TString arg(*_il.begin());
  if (arg != "*" && arg != "!*")
    throw std::runtime_error("BranchList initialization only allowed for with match-all expression");

  bool isVeto(arg[0] == '!');
  matchAll_ = BranchName(isVeto);
  matchAll_.first = "*";
  
  nameRefs_.push_back(&matchAll_);
}    

bool
panda::utils::BranchList::matchesAny(BranchList const& _list) const
{
  // loop over given branch names
  for (auto& bname : _list) {
    // find a non-vetoed branch that is in my list
    if (!bname.isVeto() && this->includes(bname))
      return true;
  }

  return false;
}


bool
panda::utils::BranchList::includes(BranchName const& _name) const
{
  // last match determines the result
    
  bool included(false);
  for (auto* bname : nameRefs_) {
    if (_name.match(*bname))
      included = !bname->isVeto();
  }
  return included;
}

bool
panda::utils::BranchList::vetoes(BranchName const& _name) const
{
  // last match determines the result
    
  bool vetoed(false);
  for (auto* bname : nameRefs_) {
    if (_name.match(*bname))
      vetoed = bname->isVeto();
  }
  return vetoed;
}

panda::utils::BranchSubList
panda::utils::BranchList::subList(TString const& _objName) const
{
  BranchSubList sublist;
  sublist.setVerbosity(getVerbosity());
    
  // loop over my branch names
  for (auto* bname : nameRefs_) {
    // if the object name is not * and not objName, skip
    if (bname->first == "*")
      sublist.emplace_back(_objName, bname->second);
    else if (bname->first == _objName)
      sublist.emplace_back(bname->first, bname->second);
  }
    
  return sublist;
}

std::ostream& operator<<(std::ostream& _os, panda::utils::BranchList const& _bl)
{
  for (unsigned iN(0); iN != _bl.size(); ++iN) {
    _os << _bl.at(iN).toString();
    if (iN != _bl.size() - 1)
      _os << " ";
  }

  return _os;
}


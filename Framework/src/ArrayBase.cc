#include "../interface/ArrayBase.h"

void
panda::ArrayBase::setStatus(TTree& _tree, utils::BranchList const& _branches)
{
  getData().setStatus(_tree, name_, _branches);
}

panda::utils::BranchList
panda::ArrayBase::getStatus(TTree& _tree) const
{
  return getData().getStatus(_tree, name_);
}

panda::utils::BranchList
panda::ArrayBase::getBranchNames(Bool_t _fullName/* = kTRUE*/, Bool_t/* = kFALSE*/) const
{
  if (_fullName)
    return getData().getBranchNames(name_);
  else
    return getData().getBranchNames("");
}

UInt_t
panda::ArrayBase::setAddress(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/, Bool_t _setStatus/* = kTRUE*/)
{
  getData().setAddress(_tree, name_, _branches, _setStatus);

  return registerInput_(_tree);
}

void
panda::ArrayBase::book(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/)
{
  getData().book(_tree, name_, _branches, false);
}

void
panda::ArrayBase::init()
{
  for (unsigned iP(0); iP != getData().nmax(); ++iP)
    elemAt(iP).init();
}

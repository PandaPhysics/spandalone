#include "../interface/Singlet.h"

void
panda::Singlet::setStatus(TTree& _tree, utils::BranchList const& _branches)
{
  doSetStatus_(_tree, _branches);
}

panda::utils::BranchList
panda::Singlet::getStatus(TTree& _tree) const
{
  return doGetStatus_(_tree);
}

panda::utils::BranchList
panda::Singlet::getBranchNames(Bool_t _fullName/* = kTRUE*/, Bool_t/* = kFALSE*/) const
{
  return doGetBranchNames_(_fullName);
}

UInt_t
panda::Singlet::setAddress(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/, Bool_t _setStatus/* = kTRUE*/)
{
  doSetAddress_(_tree, _branches, _setStatus);

  return registerInput_(_tree);
}

void
panda::Singlet::book(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/)
{
  doBook_(_tree, _branches);
}

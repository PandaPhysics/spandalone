#include "../interface/ArrayBase.h"

void
panda::ArrayBase::setStatus(TTree& _tree, utils::BranchList const& _branches)
{
  getData().setStatus(_tree, _branches);
}

UInt_t
panda::ArrayBase::setAddress(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/, Bool_t _setStatus/* = kTRUE*/)
{
  getData().setAddress(_tree, _branches, _setStatus);

  return registerInput_(_tree);
}

void
panda::ArrayBase::book(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/)
{
  getData().book(_tree, _branches, false);
}

void
panda::ArrayBase::init()
{
  for (unsigned iP(0); iP != getData().nmax(); ++iP)
    elemAt(iP).init();
}

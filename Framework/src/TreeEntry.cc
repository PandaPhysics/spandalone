#include "../interface/TreeEntry.h"
#include "../interface/CollectionBase.h"

void
panda::TreeEntry::setStatus(TTree& _tree, utils::BranchList const& _branches)
{
  for (auto* obj : objects_)
    obj->setStatus(_tree, _branches.subList(obj->getName()));

  doSetStatus_(_tree, _branches);
}

panda::utils::BranchList
panda::TreeEntry::getStatus(TTree& _tree) const
{
  utils::BranchList blist;

  for (auto* obj : objects_)
    blist += obj->getStatus(_tree);

  blist += doGetStatus_(_tree);

  return blist;
}

panda::utils::BranchList
panda::TreeEntry::getBranchNames(Bool_t/* = kTRUE*/, Bool_t _direct/* = kFALSE*/) const
{
  utils::BranchList blist;

  if (!_direct) {
    for (auto* obj : objects_)
      blist += obj->getBranchNames(true);
  }

  blist += doGetBranchNames_();

  return blist;
}

UInt_t
panda::TreeEntry::setAddress(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/, Bool_t _setStatus/* = kTRUE*/)
{
  for (auto* obj : objects_)
    obj->setAddress(_tree, _branches.subList(obj->getName()), _setStatus);

  doSetAddress_(_tree, _branches, _setStatus);

  return registerInput_(_tree);
}

void
panda::TreeEntry::book(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/)
{
  for (auto* obj : objects_)
    obj->book(_tree, _branches.subList(obj->getName()));

  doBook_(_tree, _branches);
}

void
panda::TreeEntry::init()
{
  for (auto* obj : objects_)
    obj->init();

  doInit_();
}

Int_t
panda::TreeEntry::getEntry(UInt_t _treeId, Long64_t _entry, Bool_t _localEntry/* = kFALSE*/)
{
  init();

  if (!_localEntry)
    _entry = inputBranches_[_treeId].first->LoadTree(_entry);

  int bytes(0);

  // This breaks if setAddress is called on individual collections separately
  // (tree id must be synched between all objects and this tree entry)
  for (auto* obj : objects_)
    bytes += obj->getEntry(_treeId, _entry, true);

  // Get data for my branches
  for (auto* branch : inputBranches_[_treeId].second) {
    if (branch && !branch->TestBit(kDoNotProcess))
      bytes += branch->GetEntry(_entry);
  }

  if (bytes > 0)
    doGetEntry_(*inputBranches_[_treeId].first);

  return bytes;
}

Int_t
panda::TreeEntry::fill(TTree& _tree)
{
  for (auto* col : collections_)
    col->prepareFill(_tree);

  return _tree.Fill();
}

void
panda::TreeEntry::unlink(TTree& _tree)
{
  ReaderObject::unlink(_tree);

  doUnlink_(_tree);
}

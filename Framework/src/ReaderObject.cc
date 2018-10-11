#include "../interface/ReaderObject.h"

#include "TTree.h"
#include "TChain.h"
#include "TList.h"

#include <stdexcept>

panda::ReaderObject::~ReaderObject()
{
  unsigned iT(0);
  while (iT != inputBranches_.size()) {
    if (utils::removeBranchArrayUpdator(*this, *inputBranches_[iT].first))
      continue; // function call reduced the inputBranches_ vector size

    ++iT; // if for some reason the object was already deregistered from the tree
  }
}

Int_t
panda::ReaderObject::getEntry(TTree& _tree, Long64_t _entry, Bool_t _localEntry/* = kFALSE*/)
{
  for (unsigned iT(0); iT != inputBranches_.size(); ++iT) {
    if (inputBranches_[iT].first == &_tree)
      return getEntry(iT, _entry, _localEntry);
  }

  return 0;
}

Int_t
panda::ReaderObject::getEntry(UInt_t _treeId, Long64_t _entry, Bool_t _localEntry/* = kFALSE*/)
{
  init();

  if (!_localEntry)
    _entry = inputBranches_[_treeId].first->LoadTree(_entry);

  int bytes(0);

  for (auto* branch : inputBranches_[_treeId].second) {
    if (branch && !branch->TestBit(kDoNotProcess))
      bytes += branch->GetEntry(_entry);
  }

  return bytes;
}

UInt_t
panda::ReaderObject::registerInput_(TTree& _tree)
{
  // First see if the tree is already registered
  for (unsigned iT(0); iT != inputBranches_.size(); ++iT) {
    if (inputBranches_[iT].first == &_tree)
      return iT;
  }

  // We don't know the tree.

  unsigned treeId(inputBranches_.size());
  inputBranches_.emplace_back(&_tree, BranchArray());

  auto* updator(new utils::BranchArrayUpdator(*this, _tree));
  _tree.GetUserInfo()->Add(updator);

  if (_tree.InheritsFrom(TChain::Class())) {
    // See if it already has a Notify object other than TNotify
    utils::TNotify* notify(0);
    auto* notifyObj(_tree.GetNotify());
    if (notifyObj) {
      notify = dynamic_cast<utils::TNotify*>(notifyObj);

      if (!notify)
        throw std::runtime_error("Input chain cannot handle Notify() method");
    }
    else {
      notify = new utils::TNotify();
      _tree.SetNotify(notify);

      // Make the tree automatically delete this object
      _tree.GetUserInfo()->Add(notify);
    }

    notify->Add(updator);
  }

  // If the tree is a TChain and is not loaded yet, BranchArrayUpdator will
  // fill the BranchArray at the first Notify. If the tree is already loaded
  // or is a TTree, we need to manually call updateBranchArray().
  if (_tree.GetTreeNumber() >= 0)
    updateBranchArray(_tree);

  return treeId;
}

void
panda::ReaderObject::unlink(TTree& _tree)
{
  for (auto itr(inputBranches_.begin()); itr != inputBranches_.end(); ++itr) {
    if (itr->first == &_tree) {
      inputBranches_.erase(itr);
      return;
    }
  }
}

void
panda::ReaderObject::updateBranchArray(TTree& _tree)
{
  for (unsigned iT(0); iT != inputBranches_.size(); ++iT) {
    if (inputBranches_[iT].first == &_tree) {
      auto& branchArray(inputBranches_[iT].second);

      branchArray.clear();
      for (auto& bname : getBranchNames(true, true))
        branchArray.push_back(_tree.GetBranch(bname.fullName())); // allow nullptrs
    }
  }
}


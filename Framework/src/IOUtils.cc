#include "../interface/IOUtils.h"
#include "../interface/ReaderObject.h"

#include "TChain.h"
#include "TChainElement.h"
#include "TList.h"

#include <stdexcept>

Int_t
panda::utils::checkStatus(TTree& _tree, BranchName const& _bName, Bool_t _status)
{
  // If _tree is a TChain, paths may not be added yet. Then GetBranch returns 0
  // but that does not necessarily mean that the branch does not exist in the trees
  // to be added. Check GetBranch, but fail only if there is a tree loaded (GetBranch
  // loads the first tree if it isn't yet).

  TString fullName(_bName.toString());

  if (!_tree.GetBranch(fullName) && _tree.GetTreeNumber() >= 0)
    return -1;

  if (_tree.GetBranchStatus(fullName) == _status)
    return 0;
  else
    return 1;
}

Int_t
panda::utils::setStatus(TTree& _tree, BranchName const& _bName, BranchList const& _bList)
{
  TString fullName(_bName.toString());

  bool status(false);
  if (_bList.includes(_bName))
    status = true;
  else if (_bList.vetoes(_bName))
    status = false;
  else {
    if (_bList.getVerbosity() > 1)
      std::cout << "Branch " << fullName << " status change not requested" << std::endl;

    return -2;
  }

  // -1 -> branch does not exist; 0 -> status is already set; 1 -> status is different
  Int_t returnCode(checkStatus(_tree, _bName, status));
  if (returnCode != 1) {
    if (_bList.getVerbosity() > 0)
      std::cout << "Branch " << fullName << " status unchanged at " << status << std::endl;

    return returnCode;
  }

  if (_bList.getVerbosity() > 0)
    std::cout << "Setting branch " << fullName << " status to " << status << std::endl;

  _tree.SetBranchStatus(fullName, status);
  return 1;
}

Int_t
panda::utils::setAddress(TTree& _tree, BranchName const& _bName, void* _bPtr, BranchList const& _bList, Bool_t _setStatus)
{
  Int_t returnCode(0);

  TString fullName(_bName.toString());

  if (_setStatus) {
    returnCode = setStatus(_tree, _bName, _bList);

    if (returnCode < 0) { // branch does not exist or is not in the list (includes vetoed case)
      // diagnose the failure for dump
      if (_bList.getVerbosity() > 0) {
        if (returnCode == -2) {
          if (_bList.getVerbosity() > 1) 
            std::cout << "Branch " << fullName.Data() << " was not requested" << std::endl;
        }
        else if (returnCode == -1) {
          std::cout << "Branch " << fullName.Data() << " does not exist" << std::endl;
        }
        else if (_bList.vetoes(_bName)) {
          std::cout << "Branch " << fullName.Data() << " was vetoed" << std::endl;
        }
      }

      return returnCode;
    }
  }
  else {
    if (!_bList.includes(_bName))
      return -2;
    
    // -1 -> branch does not exist; 0 -> status is true; 1 -> status is false
    returnCode = checkStatus(_tree, _bName, true);
    if (returnCode != 0)
      return returnCode;
  }
  
  if (_bList.getVerbosity() > 0) {
    if (_bList.vetoes(_bName)) 
      std::cout << "Branch " << fullName.Data() << " was vetoed" << std::endl;
    else
      std::cout << "Branch " << fullName.Data() << " will be read" << std::endl;
  }

  _tree.SetBranchAddress(fullName, _bPtr);
  return 1;
}

Int_t 
panda::utils::book(TTree& _tree, BranchName const& _bName, TString const& _size, char _lType, void* _bPtr, BranchList const& _bList)
{
  // bName: electrons.pt
  // size: [electrons.size]
  // lType: F

  if (!_bList.includes(_bName))
    return -2;

  TString fullName(_bName.toString());

  if (_tree.GetBranch(fullName))
    throw std::runtime_error(("Branch " + fullName + " booked twice").Data());

  TString lExpr(_bName.second + _size);
  lExpr += "/";
  lExpr += _lType;

  _tree.Branch(fullName, _bPtr, lExpr);

  return 1;
}

Int_t 
panda::utils::resetAddress(TTree& _tree, BranchName const& _bName)
{
  // bName: electrons.pt

  TString fullName(_bName.toString());

  auto* branch(_tree.GetBranch(fullName));
  if (branch)
    branch->ResetAddress();

  if (_tree.InheritsFrom(TChain::Class())) {
    auto& chain(static_cast<TChain&>(_tree));
    auto* elem(static_cast<TChainElement*>(chain.GetStatus()->FindObject(fullName)));
    if (elem)
      elem->SetBaddress(0);
  }

  return 0;
}

TTree*
panda::utils::makeDocTree(TString const& _treeName, TString _names[], UInt_t _size)
{
  auto* tree(new TTree(_treeName, _treeName));
  TString* name(new TString);
  tree->Branch("name", "TString", &name);
  for (unsigned iN(0); iN != _size; ++iN) {
    *name = _names[iN];
    tree->Fill();
  }
  tree->ResetBranchAddresses();
  delete name;
  return tree;
}

panda::utils::TNotify::TNotify()
{
  SetBit(kIsOnHeap); // this object is added to tree UserInfo, which deletes all IsOnHeap objects at the end
}

Bool_t
panda::utils::TNotify::Notify()
{
  for (int iN(0); iN != GetEntriesFast(); ++iN)
    UncheckedAt(iN)->Notify();

  return true;
}

panda::utils::BranchArrayUpdator::BranchArrayUpdator(ReaderObject& _obj, TTree& _tree) :
  obj_(_obj),
  tree_(_tree)
{
  SetBit(kIsOnHeap);
}

panda::utils::BranchArrayUpdator::~BranchArrayUpdator()
{
  obj_.unlink(tree_);
}

char const*
panda::utils::BranchArrayUpdator::GetName() const
{
  return obj_.getName();
}

Bool_t
panda::utils::BranchArrayUpdator::Notify()
{
  obj_.updateBranchArray(tree_);

  return true;
}

Bool_t
panda::utils::removeBranchArrayUpdator(ReaderObject& _obj, TTree& _tree)
{
  auto* uinfo(_tree.GetUserInfo());
  for (auto* uobj : *uinfo) {
    if (uobj->GetName() != _obj.getName())
      continue;

    auto* cleaner(dynamic_cast<BranchArrayUpdator*>(uobj));
    if (cleaner && &cleaner->getObject() == &_obj) {
      uinfo->Remove(uobj);
      delete uobj; // calls _obj.unlink(_tree)
      return true;
    }
  }

  return false;
}

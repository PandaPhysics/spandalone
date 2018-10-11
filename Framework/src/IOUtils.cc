#include "../interface/IOUtils.h"
#include "../interface/ReaderObject.h"

#include "TObjArray.h"
#include "TList.h"
#include "TChain.h"
#include "TChainElement.h"

#include <stdexcept>

panda::utils::BranchName::BranchName(BranchName const& _src) :
  std::vector<TString>(_src),
  isVeto_(_src.isVeto_)
{
}

panda::utils::BranchName::BranchName(char const* _name)
{
  TString name(_name);
  if (name[0] == '!') {
    isVeto_ = true;
    name = name(1, name.Length());
  }

  auto* parts(name.Tokenize("."));

  for (auto* s : *parts)
    emplace_back(s->GetName());

  delete parts;
}

panda::utils::BranchName::operator TString() const
{
  TString name;

  if (isVeto_)
    name = "!";

  for (unsigned iN(0); iN != size() - 1; ++iN)
    name += (*this)[iN] + ".";

  name += back();

  return name;
}

TString
panda::utils::BranchName::fullName(TString const& _objName/* = ""*/) const
{
  TString bFullName(*this);

  if (_objName.Length() != 0) {
    if (isVeto_) {
      // remove the leading '!'
      bFullName = bFullName(1, bFullName.Length());
    }
    
    bFullName.Prepend(_objName + ".");

    if (isVeto_) {
      // and add it to the front
      bFullName.Prepend("!");
    }
  }

  return bFullName;
}

bool
panda::utils::BranchName::match(BranchName const& _rhs) const
{
  // case 1. Both names are null. -> no match
  if (size() == 0 || _rhs.size() == 0)
    return false;

  // now match the names word by word
  for (unsigned iP(0); iP != size(); ++iP) {
    // case 2. RHS has fewer words, but agreeds up to the last word. -> match
    if (iP == _rhs.size())
      return true;

    // case 3. A word does not agree. -> no match
    if ((*this)[iP] != "*" && _rhs[iP] != "*" && (*this)[iP] != _rhs[iP])
      return false;
  }
        
  // case 4. A perfect match.
  // case 5. RHS has more words.
  // -> both considered a match
  return true;
}

bool
panda::utils::BranchName::in(BranchList const& _list) const
{
  // last match determines the result

  bool included(false);
  for (auto& bname : _list) {
    if (match(bname)) {
      included = !bname.isVeto();
    }
  }
  return included;
}

bool
panda::utils::BranchName::vetoed(BranchList const& _list) const
{
  // last match determines the result

  bool vetoed(false);
  for (auto& bname : _list) {
    if (match(bname))
      vetoed = bname.isVeto();
  }
  return vetoed;
}

panda::utils::BranchList
panda::utils::BranchList::subList(TString const& _objName) const
{
  BranchList list;
  list.setVerbosity(getVerbosity());

  // loop over my branch names
  for (auto& b : *this) {
    // if the first name is not * and not objName, skip
    if (b.size() == 0 || (b[0] != "*" && b[0] != _objName))
      continue;

    list.emplace_back(b);
    auto& bname(list.back());

    // if the branch name is just objName, consider it as objName.*
    if (bname.size() == 1)
      bname.emplace_back("*");

    // remove the objName. part
    bname.erase(bname.begin());
  }

  return list;
}

bool
panda::utils::BranchList::matchesAny(BranchList const& _list) const
{
  // loop over given branch names
  for (auto& b : _list) {
    // find a non-vetoed branch that is in my list
    if (!b.isVeto() && b.in(*this))
      return true;
  }

  return false;
}

panda::utils::BranchList&
panda::utils::BranchList::operator+=(BranchList const& _rhs)
{
  insert(end(), _rhs.begin(), _rhs.end());
  return *this;
}

panda::utils::BranchList
panda::utils::BranchList::fullNames(TString const& _objName/* = ""*/) const
{
  BranchList blist;
  for (auto& name : *this)
    blist.emplace_back(name.fullName(_objName));

  return blist;
}

/*static*/
panda::utils::BranchList
panda::utils::BranchList::makeList(TTree& _tree)
{
  BranchList blist;

  auto* branches(_tree.GetListOfBranches());
  for (auto* br : *branches)
    blist.emplace_back(br->GetName());

  return blist;
}

Int_t
panda::utils::checkStatus(TTree& _tree, TString const& _fullName, Bool_t _status)
{
  // If _tree is a TChain, paths may not be added yet. Then GetBranch returns 0
  // but that does not necessarily mean that the branch does not exist in the trees
  // to be added. Check GetBranch, but fail only if there is a tree loaded (GetBranch
  // loads the first tree if it isn't yet).
  if (!_tree.GetBranch(_fullName) && _tree.GetTreeNumber() >= 0)
    return -1;

  if (_tree.GetBranchStatus(_fullName) == _status)
    return 0;
  else
    return 1;
}

Int_t
panda::utils::setStatus(TTree& _tree, TString const& _objName, BranchName const& _bName, BranchList const& _bList)
{
  TString fullName(_bName.fullName(_objName));

  bool status;
  if (_bName.in(_bList))
    status = true;
  else if (_bName.vetoed(_bList))
    status = false;
  else {
    if (_bList.getVerbosity() > 1)
      std::cout << "Branch " << fullName << " status change not requested" << std::endl;

    return -2;
  }

  // -1 -> branch does not exist; 0 -> status is already set; 1 -> status is different
  Int_t returnCode(checkStatus(_tree, fullName, status));
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

panda::utils::BranchName
panda::utils::getStatus(TTree& _tree, TString const& _objName, BranchName const& _bName)
{
  TString fullName(_bName.fullName(_objName));

  // -1 -> branch does not exist; 0 -> status is already set; 1 -> status is different
  Int_t returnCode(checkStatus(_tree, fullName, true));
  if (returnCode == 0)
    return BranchName(fullName);
  else
    return BranchName("!" + fullName);
}

Int_t
panda::utils::setAddress(TTree& _tree, TString const& _objName, BranchName const& _bName, void* _bPtr, BranchList const& _bList, Bool_t _setStatus)
{
  Int_t returnCode(0);

  TString fullName(_bName.fullName(_objName));

  if (_setStatus) {
    returnCode = setStatus(_tree, _objName, _bName, _bList);

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
        else if (_bName.vetoed(_bList)) {
          std::cout << "Branch " << fullName.Data() << " was vetoed" << std::endl;
        }
      }

      return returnCode;
    }
  }
  else {
    if (!_bName.in(_bList))
      return -2;
    
    // -1 -> branch does not exist; 0 -> status is true; 1 -> status is false
    returnCode = checkStatus(_tree, fullName, true);
    if (returnCode != 0)
      return returnCode;
  }
  
  if (_bList.getVerbosity() > 0) {
    if (_bName.vetoed(_bList)) 
      std::cout << "Branch " << fullName.Data() << " was vetoed" << std::endl;
    else
      std::cout << "Branch " << fullName.Data() << " will be read" << std::endl;
  }

  _tree.SetBranchAddress(fullName, _bPtr);
  return 1;
}

Int_t 
panda::utils::book(TTree& _tree, TString const& _objName, BranchName const& _bName, TString const& _size, char _lType, void* _bPtr, BranchList const& _bList)
{
  // objName: electrons
  // bName: pt
  // size: [electrons.size]
  // lType: F

  if (!_bName.in(_bList))
    return -2;

  TString fullName(_bName.fullName(_objName));
  if (_tree.GetBranch(fullName))
    throw std::runtime_error(("Branch " + fullName + " booked twice").Data());

  TString lExpr(_bName + _size);

  lExpr += "/";
  lExpr += _lType;

  _tree.Branch(_bName.fullName(_objName), _bPtr, lExpr);

  return 1;
}

Int_t 
panda::utils::resetAddress(TTree& _tree, TString const& _objName, BranchName const& _bName)
{
  // bName: electrons.pt
  auto* branch(_tree.GetBranch(_bName.fullName(_objName)));
  if (branch)
    branch->ResetAddress();

  if (_tree.InheritsFrom(TChain::Class())) {
    auto& chain(static_cast<TChain&>(_tree));
    auto* elem(static_cast<TChainElement*>(chain.GetStatus()->FindObject(_bName.fullName(_objName))));
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

std::ostream&
operator<<(std::ostream& os, panda::utils::BranchName const& bn)
{
  return (os << TString(bn));
}

std::ostream&
operator<<(std::ostream& os, panda::utils::BranchList const& bl)
{
  for (unsigned iN(0); iN != bl.size(); ++iN) {
    os << bl[iN].fullName();
    if (iN != bl.size() - 1)
      os << " ";
  }

  return os;
}

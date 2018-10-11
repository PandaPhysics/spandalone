#include "../interface/Element.h"
#include "../interface/ArrayBase.h"

panda::Element::StoreManager panda::Element::gStore;

/*protected*/
panda::Element::Element(ArrayBase* _array)
{
  gStore.emplace(this, _array);
}

panda::Element::~Element()
{
  delete &gStore.getArray(this);
  gStore.erase(this);
}

char const*
panda::Element::getName() const
{
  // Must return an empty string when called for a non-singlet instance (i.e. a member of a Container)
  auto oItr(gStore.find(this));
  if (oItr == gStore.end())
    return "";
  else
    return oItr->second->getName();
}

void
panda::Element::setName(char const* _name)
{
  gStore.getArray(this).setName(_name);
}

void
panda::Element::setStatus(TTree& _tree, utils::BranchList const& _branches)
{
  auto& array(gStore.getArray(this));
  array.getData().setStatus(_tree, array.getName(), _branches);
}

panda::utils::BranchList
panda::Element::getStatus(TTree& _tree) const
{
  auto& array(gStore.getArray(this));
  return array.getData().getStatus(_tree, array.getName());
}

panda::utils::BranchList
panda::Element::getBranchNames(Bool_t _fullName/* = kTRUE*/, Bool_t/* = kFALSE*/) const
{
  auto& array(gStore.getArray(this));
  if (_fullName)
    return array.getData().getBranchNames(array.getName());
  else
    return array.getData().getBranchNames();
}

UInt_t
panda::Element::setAddress(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/, Bool_t _setStatus/* = kTRUE*/)
{
  return gStore.getArray(this).setAddress(_tree, _branches, _setStatus);
}

void
panda::Element::book(TTree& _tree, utils::BranchList const& _branches/* = {"*"}*/)
{
  doBook_(_tree, gStore.getName(this), _branches);
}

Int_t
panda::Element::getEntry(TTree& _tree, Long64_t _entry, Bool_t _localEntry/* = kFALSE*/)
{
  return gStore.getArray(this).getEntry(_tree, _entry, _localEntry);
}

Int_t
panda::Element::getEntry(UInt_t _treeId, Long64_t _entry, Bool_t _localEntry/* = kFALSE*/)
{
  return gStore.getArray(this).getEntry(_treeId, _entry, _localEntry);
}

panda::ArrayBase&
panda::Element::StoreManager::getArray(Element const* _obj) const
{
  try {
    return *at(_obj);
  }
  catch (std::out_of_range&) {
    std::cerr << "!!! EXCEPTION !!!" << std::endl;
    std::cerr << "Data was requested for a non-singlet " << _obj->typeName() << " instance." << std::endl;
    std::cerr << "This should not happen under normal circumstances and indicates a bug in" << std::endl;
    std::cerr << "the framework. Contact a Panda expert to fix the issue." << std::endl;
    throw;
  }
}

char const*
panda::Element::StoreManager::getName(Element const* _obj) const
{
  return getArray(_obj).getName();
}

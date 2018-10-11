#ifndef PandaTree_Interface_TreeEntry_h
#define PandaTree_Interface_TreeEntry_h

#include "ReaderObject.h"
#include "IOUtils.h"

#include "TTree.h"

#include <vector>

namespace panda {

  class CollectionBase;

  //! Base class for objects representing one full entry of a tree (e.g. Event).
  /*!
   * A derived class of TreeEntry typically owns multiple Singlets, Containers, as well as its own primitive branches.
   */
  class TreeEntry : public ReaderObject {
  public:
    TreeEntry() : ReaderObject() {}
    TreeEntry(TreeEntry const& src) : ReaderObject (src) {}
    virtual ~TreeEntry() {}
    // Important to have this implemented - otherwise the TreeEntries will start copying the objects_ vectors!
    TreeEntry& operator=(TreeEntry const&) { return *this; }

    void setStatus(TTree&, utils::BranchList const& blist) final;
    utils::BranchList getStatus(TTree&) const final;
    utils::BranchList getBranchNames(Bool_t = kTRUE, Bool_t direct = kFALSE) const final;
    UInt_t setAddress(TTree&, utils::BranchList const& blist = {"*"}, Bool_t setStatus = kTRUE) final;
    void book(TTree&, utils::BranchList const& blist = {"*"}) final;
    using ReaderObject::getEntry;
    Int_t getEntry(UInt_t, Long64_t entry, Bool_t localEntry = kFALSE) final;
    Int_t fill(TTree&) final;
    void init() final;
    char const* getName() const final { return ""; }
    void setName(char const*) final {}
    void unlink(TTree&) final;

  protected:
    virtual void doSetStatus_(TTree&, utils::BranchList const&) = 0;
    virtual utils::BranchList doGetStatus_(TTree&) const = 0;
    virtual utils::BranchList doGetBranchNames_() const = 0;
    virtual void doSetAddress_(TTree& tree, utils::BranchList const&, Bool_t setStatus) = 0;
    virtual void doBook_(TTree&, utils::BranchList const&) = 0;
    virtual void doGetEntry_(TTree&) = 0;
    virtual void doInit_() = 0;
    virtual void doUnlink_(TTree&) = 0;

    std::vector<Object*> objects_{}; //!< list of object branches (containers and singlets)
    std::vector<CollectionBase*> collections_{}; //!< list of collections (overlaps with objects_)
  };

}

#endif

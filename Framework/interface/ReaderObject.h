#ifndef PandaTree_Framework_ReaderObject_h
#define PandaTree_Framework_ReaderObject_h

#include "Object.h"
#include "IOUtils.h"

namespace panda {

  //! Base class for objects that can be linked to an input tree directly.
  class ReaderObject : public Object {
  public:
    ReaderObject() : Object() {}
    ReaderObject(ReaderObject const& src) : Object(src) {}
    ~ReaderObject();
    ReaderObject& operator=(ReaderObject const&) { return *this; }

    Int_t getEntry(TTree& tree, Long64_t entry, Bool_t localEntry = kFALSE) final;
    Int_t getEntry(UInt_t treeId, Long64_t entry, Bool_t localEntry = kFALSE) override;

    //! Unlink from a tree.
    virtual void unlink(TTree&);
    
    void updateBranchArray(TTree&);

  protected:
    UInt_t registerInput_(TTree&);

    typedef std::vector<TBranch*> BranchArray;

    //! List of branches linked with this object in the tree
    typedef std::pair<TTree*, BranchArray> BranchMapping;

    //! List of list of input branches
    /*!
     * With vector<BranchMapping> implementation, we will be serially searching for a tree whenever
     * getEntry(tree, entry) is called. Alternative would be to use a map<TTree*, BranchArray>
     * to make use of binary-tree lookup. However most of the time we will be dealing with one
     * input tree, and at most a few input trees, that the map implementation actually won't
     * make sense.
     */
    std::vector<BranchMapping> inputBranches_;
  };

}

#endif

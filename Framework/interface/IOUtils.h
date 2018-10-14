#ifndef PandaTree_Interface_IOUtils_h
#define PandaTree_Interface_IOUtils_h

#include "BranchName.h"
#include "BranchList.h"

#include "TString.h"
#include "TTree.h"

#include <vector>
#include <utility>
#include <iostream>

namespace panda {
  class ReaderObject;

  namespace utils {
    class BranchName;
    class BranchList;

    //! Check status of a branch
    /*!
     * Return values:
     * -1 branch does not exist
     * 0  status is current
     * 1  status is different
     */
    Int_t checkStatus(TTree&, BranchName const&, Bool_t status);
    //! Set the status of a branch
    /*!
     * Return values:
     * -2 branch is not in given list
     * -1 branch does not exist
     * 0  status is already set
     * 1  status is changed
     */
    Int_t setStatus(TTree&, BranchName const&, BranchList const&);
    //! Set address
    /*!
     * Return values:
     * -2 branch is not in given list
     * -1 branch does not exist
     * 0  status is false and address is not set
     * 1  status is true and address is set
     */
    Int_t setAddress(TTree&, BranchName const&, void* bPtr, BranchList const&, Bool_t setStatus);
    //! Book the branch
    /*!
     * Return values:
     * -2 branch is not in given list
     * 1  branch is booked
     * Throws in case of double-booking
     */
    Int_t book(TTree&, BranchName const&, TString const& size, char lType, void* bPtr, BranchList const&);

    Int_t resetAddress(TTree&, BranchName const&);

    template<class O>
    Int_t
    book(TTree& _tree, BranchName const& _bName, TString const& _objType, O** _bPtr, BranchList const& _bList)
    {
      // bName: jet.tags
      // objType: std::vector<int>

      if (!_bList.includes(_bName))
        return -1;

      _tree.Branch(_bName.toString(), _objType, _bPtr);

      return 0;
    }

    //! Make a tree from a TString array
    /*!
     * Used to document enum contents.
     */
    TTree*
    makeDocTree(TString const& treeName, TString names[], UInt_t size);

    //! One big Notify() manager for all objects
    /*!
     * TNotify is a simple array that calls Notify() of all elements sequentially.
     * Whoever is the first one to register a Notify() object to an input tree must create
     * an instance of this object. The instance must then be passed to UserInfo of the tree
     * for automatic deletion.
     */
    class TNotify : public TObjArray {
    public:
      TNotify();
      ~TNotify() {}
      Bool_t Notify() override;
    };

    //! Automated branch list update for ReaderObjects.
    /*!
     * This class serves two purposes:
     * 1. When reading from a TChain, use the ROOT built-in Notify() mechanism to update
     *    the list of branches the ReaderObject holds.
     * 2. When the tree is deleted, make use of the fact that contents of the
     *    TTree::fUserInfo container is automatically deleted (if IsOnHeap is true) to
     *    call unlink() on the ReaderObject.
     */
    class BranchArrayUpdator : public TObject {
    public:
      BranchArrayUpdator(ReaderObject&, TTree&);
      ~BranchArrayUpdator();

      char const* GetName() const override;
      Bool_t Notify() override;

      ReaderObject const& getObject() const { return obj_; }

    private:
      ReaderObject& obj_;
      TTree& tree_;
    };

    //! Called when the ReaderObject is deleted before the tree.
    /*!
      \param obj   Object that is being deleted and therefore must be deregistered from the Tree
      \param tree  Tree to be cleaned
      \return true if obj is found in the tree's UserInfo
     */
    Bool_t removeBranchArrayUpdator(ReaderObject& obj, TTree& tree);

  }
}

#endif

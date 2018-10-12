#ifndef PandaTree_Interface_IOUtils_h
#define PandaTree_Interface_IOUtils_h

#include "TString.h"
#include "TTree.h"

#include <vector>
#include <iostream>

namespace panda {
  class ReaderObject;

  namespace utils {

    class BranchList;

    //! Tokenized branch name
    /*!
     * Represents a branch name given in formats like muons.pt by a vector {"muons", "pt"}.
     * The name can be preceded by '!' to indicate a veto for the branch.
     * Wild card (*) is also allowed, and will match to either one section of the name or to the
     * entire remainder.
     */
    class BranchName : public std::vector<TString> {
    public:
      static char const* separator;

      BranchName() {}
      BranchName(BranchName const&);
      BranchName(char const*);
      BranchName(std::string const& s) : BranchName(s.c_str()) {}
      BranchName(TString const& s) : BranchName(s.Data()) {}
      //! Concatenate the words with '.'
      operator TString() const;
      //! Prepend the branch name with <objName.>.
      virtual TString fullName(TString const& objName = "") const;
      //! Did the name start with a '!'?
      bool isVeto() const { return isVeto_; }
      //! Does the name match with the given name?
      /*!
       * Only considers the name part and not the veto-ness.
       */
      bool match(BranchName const&) const;
      //! Is the name included and not vetoed?
      /*!
       * Does not take the veto on the parent object into account. Simply asks the question
       * "is the name in the given list and not vetoed in the list?"
       */
      bool in(BranchList const&) const;
      //! Is the name included and vetoed?
      /*!
       * Does not take the veto on the parent object into account. Simply asks the question
       * "is the name in the given list and vetoed in the list?"
       */
      bool vetoed(BranchList const&) const;

    private:
      bool isVeto_{false};
    };

    //! List of branch names
    /*!
     * Basically a vector of BranchNames with a few facilities.
     */
    class BranchList : public std::vector<BranchName> {
    public:
      BranchList() {}
      BranchList(std::initializer_list<value_type> il, const allocator_type& alloc = allocator_type()) : std::vector<BranchName>(il, alloc) {}
      //! Return a new list of branches that starts with the objName, removing <objName.> from each.
      BranchList subList(TString const& objName) const;
      //! Returns true if any of the branch in the list is not vetoed in my list.
      bool matchesAny(BranchList const&) const;
      //! Extend the list
      BranchList& operator+=(BranchList const&);
      //! Prepend the branch names with <objName.>.
      BranchList fullNames(TString const& objName = "") const;
      //! Create a branchlist object from the branches in the tree
      static BranchList makeList(TTree&);
      //! Set the verbosity level
      /*!
       * Reports individual branches when reading from tree:
       * 0 = silent [default]
       * 1 = report requested, vetoed, missing branches
       * 2 = report all branches
       */
      void setVerbosity(int i) { verbosity = i; }
      //! Get the verbosity level
      int getVerbosity() const { return verbosity; }
    private:
      int verbosity{0};
    };

    //! Check status of a branch
    /*!
     * Return values:
     * -1 branch does not exist
     * 0  status is current
     * 1  status is different
     */
    Int_t checkStatus(TTree&, TString const& fullName, Bool_t status);
    //! Set the status of a branch
    /*!
     * Return values:
     * -2 branch is not in given list
     * -1 branch does not exist
     * 0  status is already set
     * 1  status is changed
     */
    Int_t setStatus(TTree&, TString const& objName, BranchName const& bName, BranchList const&);
    //! Get the status of a branch
    /*!
     * Returned branch name is vetoed if the branch does not exist or is not enabled.
     */
    BranchName getStatus(TTree&, TString const& objName, BranchName const& bName);
    //! Set address
    /*!
     * Return values:
     * -2 branch is not in given list
     * -1 branch does not exist
     * 0  status is false and address is not set
     * 1  status is true and address is set
     */
    Int_t setAddress(TTree&, TString const& objName, BranchName const& bName, void* bPtr, BranchList const&, Bool_t setStatus);
    //! Book the branch
    /*!
     * Return values:
     * -2 branch is not in given list
     * 1  branch is booked
     * Throws in case of double-booking
     */
    Int_t book(TTree&, TString const& objName, BranchName const& bName, TString const& size, char lType, void* bPtr, BranchList const&);

    Int_t resetAddress(TTree&, TString const& objName, BranchName const& bName);

    template<class O>
    Int_t
    book(TTree& _tree, TString const& _objName, BranchName const& _bName, TString const& _objType, O** _bPtr, BranchList const& _bList)
    {
      // objName: electrons
      // bName: tags
      // objType: std::vector<int>

      if (!_bName.in(_bList))
        return -1;

      _tree.Branch(_bName.fullName(_objName), _objType, _bPtr);

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

//! Print BranchList
std::ostream& operator<<(std::ostream&, panda::utils::BranchName const&);
std::ostream& operator<<(std::ostream&, panda::utils::BranchList const&);

#endif

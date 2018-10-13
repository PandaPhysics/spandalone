#ifndef PandaTree_Interface_BranchList_h
#define PandaTree_Interface_BranchList_h

#include "TString.h"
#include "TTree.h"

#include <vector>
#include <iostream>
#include <memory>
#include <functional>

namespace panda {
  namespace utils {
    class BranchName;
    class NullNameSyntax;
    template<class T> class BranchListImpl;

    typedef BranchListImpl<NullNameSyntax> BranchSubList;

    //! List of branch names
    /*!
     * This base class is mostly defined for accessors returning refernces,
     * but a special-case construction with {"*"} or {"!*"} is defined.
     */
    class BranchList {
    public:
      typedef std::reference_wrapper<BranchName> ref_wrapper;
      typedef std::vector<ref_wrapper> ref_vector;
      typedef BranchName& reference;
      typedef BranchName const& const_reference;
      typedef ref_vector::iterator iterator;
      typedef ref_vector::const_iterator const_iterator;

      BranchList(std::initializer_list<TString>);

      reference at(unsigned i) { return nameRefs_.at(i); }
      const_reference at(unsigned i) const { return nameRefs_.at(i); }
      iterator begin() { return nameRefs_.begin(); }
      const_iterator begin() const { nameRefs_.begin(); }
      iterator end() { nameRefs_.end(); }
      const_iterator end() const { nameRefs_.end(); }

      //! Returns true if any of the branch in the list is not vetoed in my list.
      bool matchesAny(BranchList const&) const;
      //! Returns true if the name is included and not vetoed
      /*!
       * Does not take the veto on the parent object into account. Simply asks the question
       * "is the name in the given list and not vetoed in the list?"
       */
      bool includes(BranchName const&) const;
      //! Returns true if the name is included and vetoed?
      /*!
       * Does not take the veto on the parent object into account. Simply asks the question
       * "is the name in the given list and vetoed in the list?"
       */
      bool vetoes(BranchName const&) const;
      //! Return a sublist of branches that starts with the objName.
      BranchSubList subList(TString const& objName) const;

      //! Set the verbosity level
      /*!
       * Reports individual branches when reading from tree:
       * 0 = silent [default]
       * 1 = report requested, vetoed, missing branches
       * 2 = report all branches
       */
      void setVerbosity(int i) { verbosity_ = i; }
      //! Get the verbosity level
      int getVerbosity() const { return verbosity_; }

    protected:
      ref_vector nameRefs_;
      BranchName matchAll_;
      int verbosity_{0};
    };

    template<class T>
    class BranchListImpl : public BranchList {
    public:
      typedef T NameSyntax;
      typedef BranchNameImpl<T> name_type;
      typedef BranchListImpl<T> self_type;

      BranchListImpl(std::initializer_list<TString>);

      void clear() { nameRefs_.clear(); names_.clear(); }

      //! Extend the list
      template<class... Args>
      void emplace_back(Args&&... args);
      //! Extend the list
      self_type& operator+=(self_type const&);
      //! Create a branchlist object from the branches in the tree
      static self_type makeList(TTree&);

    private:
      std::vector<name_type> names_;
    };

    template<class T>
    BranchListImpl<T>::BranchListImpl(std::initializer_list<TString> il)
    {
      for (auto& name : il)
        names_.emplace_back(name);

      for (auto& name : names_)
        nameRefs_.emplace_back(name);
    }

    template<class T>
    template<class... Args>
    void
    BranchListImpl<T>::emplace_back(Args&&... args)
    {
      names_.emplace_back(&args...);
      
      // names_ may have been reallocated
      nameRefs_.clear();
      for (auto& name : names_)
        nameRefs_.emplace_back(name);
    }

    template<class T>
    BranchListImpl<T>&
    BranchListImpl<T>::operator+=(BranchListImpl<T> const& _rhs)
    {
      names_.insert(names_.end(), _rhs.names_.begin(), _rhs.names_.end());
      
      // names_ may have been reallocated
      nameRefs_.clear();
      for (auto& name : names_)
        nameRefs_.emplace_back(name);

      return *this;
    }

    /*static*/
    template<class T>
    BranchListImpl<T>
    BranchListImpl<T>::makeList(TTree& _tree)
    {
      BranchListImpl<T> blist;
    
      auto* branches(_tree.GetListOfBranches());
      for (auto* br : *branches)
        blist.emplace_back(br->GetName());
    
      return blist;
    }

  }
}

//! Print BranchList
std::ostream& operator<<(std::ostream&, panda::utils::BranchList const&);

#endif

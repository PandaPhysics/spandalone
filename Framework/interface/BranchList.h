#ifndef PandaTree_Interface_BranchList_h
#define PandaTree_Interface_BranchList_h

#include "BranchName.h"

#include "TString.h"
#include "TTree.h"

#include <vector>
#include <iostream>
#include <memory>

namespace panda {
  namespace utils {

    //! List of branch names
    /*!
     * Basically a vector of BranchNames with a few facilities.
     */
    class BranchList {
    public:
      //! Return sublist of branches that starts with the objName.
      virtual std::unique_ptr<BranchList> subList(TString const& objName) const = 0;

      //! Returns true if any of the branch in the list is not vetoed in my list.
      virtual bool matchesAny(BranchList const&) const = 0;

      //! Returns true if the name is included and not vetoed
      /*!
       * Does not take the veto on the parent object into account. Simply asks the question
       * "is the name in the given list and not vetoed in the list?"
       */
      virtual bool includes(BranchName const&) const = 0;
      //! Returns true if the name is included and vetoed?
      /*!
       * Does not take the veto on the parent object into account. Simply asks the question
       * "is the name in the given list and vetoed in the list?"
       */
      virtual bool vetoes(BranchName const&) const = 0;

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

    private:
      int verbosity_{0};
    };

    template<class T>
    class BranchListImpl : public BranchList {
    public:
      typedef T NameSyntax;
      typedef BranchNameImpl<T> name_type;
      typedef BranchListImpl<T> self_type;
      typedef std::vector<name_type> list_type;

      BranchListImpl(std::initializer_list<list_type::value_type>, const list_type::allocator_type& = list_type::allocator_type());

      std::unique_ptr<BranchList> subList(TString const& objName) const final;
      bool matchesAny(BranchList const&) const final;
      bool includes(BranchName const&) const final;
      bool vetoes(BranchName const&) const final;

      //! Size of the list
      unsigned size() const { return list_.size(); }
      //! Element access
      name_type& at(unsigned i) { return list_.at(i); }
      name_type const& at(unsigned i) const { return list_.at(i); }
      name_type& operator[](unsigned i) { return list_[i]; }
      name_type const& operator[](unsigned i) const { return list_[i]; }
      //! Extend the list
      self_type& operator+=(self_type const&);
      //! Create a branchlist object from the branches in the tree
      static self_type makeList(TTree&);

    private:
      list_type list_;
    };

    template<class T>
    BranchListImpl<T>::BranchListImpl(std::initializer_list<list_type::value_type> il, const list_type::allocator_type& alloc = list_type::allocator_type()) :
      list_(il, alloc)
    {
    }

    template<class T>
    std::unique_ptr<BranchList>
    BranchListImpl<T>::subList(TString const& _objName) const
    {
      BranchListImpl<T>* sublist(new BranchListImpl<T>());
      sublist->setVerbosity(getVerbosity());
    
      // loop over my branch names
      for (auto& b : this->list_) {
        // if the object name is not * and not objName, skip
        if (b.first == "*")
          sublist.emplace_back(_objName, b.second);
        else if (b.first == _objName)
          sublist.emplace_back(b)
      }
    
      return std::unique_ptr<BranchList>(sublist);
    }

    template<class T>
    bool
    BranchListImpl<T>::matchesAny(BranchList const& _list) const
    {
      // loop over given branch names
      for (auto& b : _list) {
        // find a non-vetoed branch that is in my list
        if (!b.isVeto() && b.in(*this))
          return true;
      }

      return false;
    }

    template<class T>
    BranchListImpl<T>&
    BranchListImpl<T>::operator+=(BranchListImpl<T> const& _rhs)
    {
      this->list_.insert(this->list_.end(), _rhs.list_.begin(), _rhs.list_.end());
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
        blist.list_.emplace_back(br->GetName());
    
      return blist;
    }

    template<class T>
    bool
    BranchNameListImpl<T>::includes(BranchName const& _name) const
    {
      // last match determines the result
    
      bool included(false);
      for (auto& bname : this->list_) {
        if (_name.match(bname))
          included = !bname.isVeto();
      }
      return included;
    }

    template<class T>
    bool
    BranchNameListImpl<T>::vetoes(BranchName const& _name) const
    {
      // last match determines the result
    
      bool vetoed(false);
      for (auto& bname : this->list_) {
        if (_name.match(bname))
          vetoed = bname.isVeto();
      }
      return vetoed;
    }

  }
}

//! Print BranchList
template<class T>
std::ostream& operator<<(std::ostream& _os, panda::utils::BranchListImpl<T> const& _bl)
{
  for (unsigned iN(0); iN != _bl.size(); ++iN) {
    _os << _bl[iN].toString();
    if (iN != _bl.size() - 1)
      _os << " ";
  }

  return _os;
}

#endif

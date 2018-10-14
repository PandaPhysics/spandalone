#ifndef PandaTree_Interface_BranchList_h
#define PandaTree_Interface_BranchList_h

#include "BranchName.h"

#include "TString.h"
#include "TTree.h"

#include <vector>
#include <iostream>
#include <memory>
#include <functional>

namespace panda {
  namespace utils {
    class BaseNameSyntax;
    template<class T, class S> class BranchListImpl;

    typedef BranchListImpl<BaseNameSyntax, BaseNameSyntax> BranchSubList;

    //! List of branch names
    /*!
     * This base class is mostly defined for accessors returning refernces,
     * but a special-case construction with {"*"} or {"!*"} is defined.
     */
    class BranchList {
      typedef std::vector<BranchName const*> container_type;

    public:
      class Iterator {
        friend BranchList;

      public:
        typedef BranchName const value_type;
        typedef int difference_type;
        typedef BranchName const& reference;
        typedef BranchName const* pointer;
        typedef std::forward_iterator_tag iterator_category;

        Iterator(Iterator const& src) : baseItr_(src.baseItr_) {}
        Iterator& operator=(Iterator const& rhs) { baseItr_ = rhs.baseItr_; return *this; }
        ~Iterator() {}
        void swap(Iterator& other) { std::swap(baseItr_, other.baseItr_); }
        reference operator*() { return **baseItr_; }
        Iterator& operator++() { ++baseItr_; return *this; }
        bool operator==(Iterator const& rhs) const { return baseItr_ == rhs.baseItr_; }
        bool operator!=(Iterator const& rhs) const { return baseItr_ != rhs.baseItr_; }

      private:
        typedef typename container_type::const_iterator base_type;

        Iterator(base_type const& _itr) : baseItr_(_itr) {}

        base_type baseItr_;
      };

      typedef BranchName const& const_reference;
      typedef Iterator const_iterator;

      BranchList() {}
      BranchList(std::initializer_list<TString>);

      unsigned size() const { return nameRefs_.size(); }
      const_reference at(unsigned i) const { return *nameRefs_.at(i); }
      const_iterator begin() const { return const_iterator(nameRefs_.begin()); }
      const_iterator end() const { return const_iterator(nameRefs_.end()); }

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
      container_type nameRefs_;
      BranchName matchAll_;
      int verbosity_{0};
    };

    template<class T, class S = T>
    class BranchListImpl : public BranchList {
    public:
      typedef T NameSyntax;
      typedef S SizeNameSyntax;
      typedef BranchNameImpl<T, S> name_type;
      typedef BranchListImpl<T, S> self_type;

      BranchListImpl() {}
      BranchListImpl(std::initializer_list<TString>);
      BranchListImpl(TString const& obj, std::initializer_list<TString>);

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

    template<class T, class S/* = T*/>
    BranchListImpl<T, S>::BranchListImpl(std::initializer_list<TString> _il)
    {
      for (auto& name : _il)
        names_.emplace_back(name);

      for (auto& name : names_)
        nameRefs_.push_back(&name);
    }

    template<class T, class S/* = T*/>
    BranchListImpl<T, S>::BranchListImpl(TString const& _obj, std::initializer_list<TString> _il)
    {
      for (auto& name : _il)
        names_.emplace_back(_obj, name);

      for (auto& name : names_)
        nameRefs_.push_back(&name);
    }

    template<class T, class S/* = T*/>
    template<class... Args>
    void
    BranchListImpl<T, S>::emplace_back(Args&&... args)
    {
      names_.emplace_back(args...);
      
      // names_ may have been reallocated
      nameRefs_.clear();
      for (auto& name : names_)
        nameRefs_.push_back(&name);
    }

    template<class T, class S/* = T*/>
    BranchListImpl<T, S>&
    BranchListImpl<T, S>::operator+=(BranchListImpl<T, S> const& _rhs)
    {
      names_.insert(names_.end(), _rhs.names_.begin(), _rhs.names_.end());
      
      // names_ may have been reallocated
      nameRefs_.clear();
      for (auto& name : names_)
        nameRefs_.push_back(&name);

      return *this;
    }

    /*static*/
    template<class T, class S/* = T*/>
    BranchListImpl<T, S>
    BranchListImpl<T, S>::makeList(TTree& _tree)
    {
      BranchListImpl<T, S> blist;
    
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

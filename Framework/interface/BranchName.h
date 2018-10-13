#ifndef PandaTree_Interface_BranchName_h
#define PandaTree_Interface_BranchName_h

#include "TString.h"

#include <utility>
#include <iostream>

namespace panda {
  namespace utils {

    //! Tokenized branch name
    /*!
     * Represents a branch name given in formats like muons.pt by a pair ("muons", "pt").
     * The name can be preceded by '!' to indicate a veto for the branch.
     * Wild card (*) is also allowed in both the object name and branch name.
     * An instance is created either by a full branch name ("muons.pt") or the object name ("muons")
     * or an object-branch name pair ("muons", "pt").
     */
    class BranchName : public std::pair<TString, TString> {
    public:
      typedef std::pair<TString, TString> core_type;

      BranchName() {}
      BranchName(BranchName const& src) : core_type(src), isVeto_(src.isVeto_) {}
      BranchName& operator=(BranchName const&);

      //! Generate a string representation of the branch name (without the veto "!")
      virtual TString toString() const { return ""; }
      operator TString() const { return toString(); }

      //! Did the name start with a '!'?
      bool isVeto() const { return isVeto_; }
      //! Does the name match with the given name?
      /*!
       * Only considers the name part and not the veto-ness.
       */
      bool match(BranchName const&) const;

    private:
      bool isVeto_{false};
    };

    template<class T>
    class BranchNameImpl : public BranchName {
    public:
      typedef T NameSyntax;

      BranchNameImpl(TString const& s, TString const& b = "", bool isVeto = false);
      BranchNameImpl(std::string const& s, std::string const& b = "", bool isVeto = false) : BranchNameImpl(s.c_str(), b.c_str(), isVeto) {}

      TString toString() const final { NameSyntax::generate(*this); }
    };

    template<class T>
    BranchNameImpl<T>::BranchNameImpl(TString const& _name, TString const& _bname/* = ""*/, bool _isVeto/* = false*/) :
      isVeto_(_isVeto)
    {
      TString name(_name);

      if (name(0) == "!") {
        isVeto_ = true; // regardless of _isVeto
        name = name(1, _name.Length());
      }

      core_type::operator=(NameSyntax::parse(name));

      if (this->second == "")
        this->second = _bname;
    }
  }
}

std::ostream& operator<<(std::ostream&, panda::utils::BranchName const&);

#endif

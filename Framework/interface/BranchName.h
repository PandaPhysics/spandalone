#ifndef PandaTree_Interface_BranchName_h
#define PandaTree_Interface_BranchName_h

#include "TString.h"

#include <utility>
#include <iostream>
#include <cstring>

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
      BranchName(bool _isVeto) : isVeto_(_isVeto) {}
      BranchName(BranchName const& src) : core_type(src), isVeto_(src.isVeto_) {}
      BranchName& operator=(BranchName const&);

      //! Generate a string representation of the branch name (without the veto "!")
      virtual TString toString(TString const& = "") const { return ""; }
      operator TString() const { return toString(); }

      //! Did the name start with a '!'?
      bool isVeto() const { return isVeto_; }
      //! Does the name match with the given name?
      /*!
       * Only considers the name part and not the veto-ness.
       */
      bool match(BranchName const&) const;

    protected:
      bool isVeto_{false};
    };

    template<class T, class S = T>
    class BranchNameImpl : public BranchName {
    public:
      typedef T NameSyntax;
      typedef S SizeNameSyntax;

      BranchNameImpl(char const* s, char const* b = "", bool isVeto = false);

      TString toString(TString const& objName = "") const final;
    };

    template<class T, class S/* = T*/>
    BranchNameImpl<T, S>::BranchNameImpl(char const* _name, char const* _bname/* = ""*/, bool _isVeto/* = false*/) :
      BranchName(_isVeto)
    {
      TString name(_name);

      if (name(0) == '!') {
        isVeto_ = true; // regardless of _isVeto
        name = name(1, name.Length());
      }

      if (_bname && std::strlen(_bname) != 0) {
        this->first = _name;
        this->second = _bname;
      }
      else {
        auto parsed(NameSyntax::parse(name));
        if (parsed.first.Length() == 0) {
          // maybe this was a size branch?
          parsed = SizeNameSyntax::parse(name);
        }
        if (parsed.first.Length() == 0)
          throw std::runtime_error(TString::Format("Invalid branch name %s", name.Data()).Data());

        core_type::operator=(parsed);
      }
    }

    template<class T, class S/* = T*/>
    TString
    BranchNameImpl<T, S>::toString(TString const& _objName/* = ""*/) const
    {
      if (this->second.Length() == 0 && _objName.Length() != 0) {
        if (this->first == "size")
          return SizeNameSyntax::generate(_objName, this->first);
        else
          return NameSyntax::generate(_objName, this->first);
      }
      else {
        if (this->second == "size")
          return SizeNameSyntax::generate(this->first, this->second);
        else
          return NameSyntax::generate(this->first, this->second);
      }
    }

    class BaseNameSyntax {
    public:
      static TString generate(TString const& o, TString const&) { return o; }
      static std::pair<TString, TString> parse(TString const& n) { return std::pair<TString, TString>(n, ""); }
    };

    typedef BranchNameImpl<BaseNameSyntax> BaseBranchName;

  }
}

std::ostream& operator<<(std::ostream&, panda::utils::BranchName const&);

#endif

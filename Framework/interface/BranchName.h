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
    class BranchName {
    public:
      enum ParseAs {
        kFullName,
        kObjName,
        kAttrName
      };

      BranchName() {}
      BranchName(char const* obj, char const* attr, bool isVeto) : obj_(obj), attr_(attr), isVeto_(isVeto) {}
      BranchName(BranchName const& src) : obj_(src.obj_), attr_(src.attr_), isVeto_(src.isVeto_) {}
      BranchName& operator=(BranchName const&);

      //! Generate a string representation of the branch name (without the veto "!")
      virtual TString toString(TString const& = "") const { return ""; }
      operator TString() const { return toString(); }

      TString const& obj() const { return obj_; }
      TString const& attr() const { return attr_; }

      void setObj(char const* obj) { obj_ = obj; }

      //! Did the name start with a '!'?
      bool isVeto() const { return isVeto_; }
      //! Is the name empty?
      bool isNull() const { return obj_.Length() == 0 && attr_.Length() == 0; }
      //! Does the name match with the given name?
      /*!
       * Only considers the name part and not the veto-ness.
       */
      bool match(BranchName const&) const;

    protected:
      TString obj_{""};
      TString attr_{""};
      bool isVeto_{false};
    };

    template<class T, class S = T>
    class BranchNameImpl : public BranchName {
    public:
      typedef T NameSyntax;
      typedef S SizeNameSyntax;

      BranchNameImpl(char const* name, bool isVeto = false, ParseAs = kFullName);
      BranchNameImpl(char const* _obj, char const* _attr, bool _isVeto = false) : BranchName(_obj, _attr, _isVeto) {}

      TString toString(TString const& objName = "") const final;
    };

    template<class T, class S/* = T*/>
    BranchNameImpl<T, S>::BranchNameImpl(char const* _name, bool _isVeto/* = false*/, ParseAs _parseAs/* = kFullName*/) :
    BranchName("", "", _isVeto)
    {
      TString name(_name);

      if (name(0) == '!') {
        isVeto_ = true; // regardless of _isVeto
        name = name(1, name.Length());
      }

      if (_parseAs == kFullName) {
        auto parsed(NameSyntax::parse(name));
        if (parsed.first.Length() == 0 && parsed.second.Length() == 0) {
          // maybe this was a size branch?
          parsed = SizeNameSyntax::parse(name);
        }
        if (parsed.first.Length() == 0 && parsed.second.Length() == 0)
          throw std::runtime_error(TString::Format("Invalid branch name %s", name.Data()).Data());

        obj_ = parsed.first;
        attr_ = parsed.second;
      }
      else if (_parseAs == kObjName) 
        obj_ = name;
      else if (_parseAs == kAttrName)
        attr_ = name;
    }

    template<class T, class S/* = T*/>
    TString
    BranchNameImpl<T, S>::toString(TString const& _objName/* = ""*/) const
    {
      if (this->obj_.Length() == 0) {
        if (_objName.Length() != 0) {
          if (this->attr_ == "size")
            return SizeNameSyntax::generate(_objName, this->attr_);
          else
            return NameSyntax::generate(_objName, this->attr_);
        }
        else
          return this->attr_;
      }
      else {
        if (this->attr_ == "size")
          return SizeNameSyntax::generate(this->obj_, this->attr_);
        else
          return NameSyntax::generate(this->obj_, this->attr_);
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

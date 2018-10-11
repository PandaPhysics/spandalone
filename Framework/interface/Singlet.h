#ifndef PandaTree_Framework_Singlet_h
#define PandaTree_Framework_Singlet_h

#include "ReaderObject.h"

#include "TString.h"

#include <vector>

namespace panda {

  //! Base class for singlet objects.
  /*!
   * Singlet objects are owners of their data, in contrast to Element.
   */
  class Singlet : public ReaderObject {
  public:
    Singlet(char const* name = "") : ReaderObject(), name_(name) {}
    Singlet(Singlet const& src) : ReaderObject(src), name_(src.name_) {}
    ~Singlet() {}
    Singlet& operator=(Singlet const& _src) { name_ = _src.name_; return *this; }

    void setStatus(TTree&, utils::BranchList const& blist) final;
    utils::BranchList getStatus(TTree&) const final;
    utils::BranchList getBranchNames(Bool_t fullName = kTRUE, Bool_t = kFALSE) const final;
    UInt_t setAddress(TTree&, utils::BranchList const& blist = {"*"}, Bool_t setStatus = kTRUE) final;
    void book(TTree&, utils::BranchList const& blist = {"*"}) final;
    void init() final { doInit_(); }
    char const* getName() const final { return name_; }
    void setName(char const* n) final { name_ = n; }

  protected:
    virtual void doSetStatus_(TTree&, utils::BranchList const&) = 0;
    virtual utils::BranchList doGetStatus_(TTree&) const = 0;
    virtual utils::BranchList doGetBranchNames_(Bool_t) const = 0;
    virtual void doSetAddress_(TTree&, utils::BranchList const&, Bool_t setStatus) = 0;
    virtual void doBook_(TTree&, utils::BranchList const&) = 0;
    virtual void doInit_() = 0;

    TString name_{};
  };

}

#endif

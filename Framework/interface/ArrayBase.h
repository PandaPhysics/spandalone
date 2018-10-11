#ifndef PandaTree_Framework_ArrayBase_h
#define PandaTree_Framework_ArrayBase_h

#include "ContainerBase.h"

namespace panda {

  //! Base class for fixed-size containers.
  /*!
   * Panda containers can be fixed-size (Array) or dynamic-size (Collection). Fixed-size containers
   * are similar to C arrays, and are more light weight and faster to use.
   */
  class ArrayBase : public ContainerBase {
    friend class Element;

  public:
    ArrayBase() = delete;
    ArrayBase(ArrayBase const& src) = delete;
    ~ArrayBase() {}
    ArrayBase& operator=(ArrayBase const&) = delete;

    void setStatus(TTree&, utils::BranchList const& blist) final;
    utils::BranchList getStatus(TTree&) const final;
    utils::BranchList getBranchNames(Bool_t fullName = kTRUE, Bool_t = kFALSE) const final;
    UInt_t setAddress(TTree&, utils::BranchList const& blist = {"*"}, Bool_t setStatus = kTRUE) final;
    void book(TTree&, utils::BranchList const& blist = {"*"}) final;
    void init() final;
    UInt_t size() const final { return getData().nmax(); }

  protected:
    ArrayBase(char const* name, UInt_t unitSize, Bool_t dummy) : ContainerBase(name, unitSize) {}

  private:
    //! To be called only from Element::setAddress
    UInt_t addInput_(TTree&);
  };

}

#endif

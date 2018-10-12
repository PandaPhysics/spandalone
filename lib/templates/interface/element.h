#ifndef @NAMESPACE@_Objects_@NAME@_h
#define @NAMESPACE@_Objects_@NAME@_h

#include "Constants.h"
@INCLUDES@
@IF[PHYS_PARENT]@
@ELSE@
#include "../interface/SizeBranchName.h"
@ENDIF@
#include "../../Framework/interface/Array.h"
#include "../../Framework/interface/Collection.h"
#include "../../Framework/interface/Ref.h"
#include "../../Framework/interface/RefVector.h"

namespace @NAMESPACE@ {

  class @NAME@ : public @PARENT@ {
  public:
    @ENUMS@
    @CONSTANTS@
    struct datastore : public @PARENT@::datastore {
      datastore() : @PARENT@::datastore() {}
      ~datastore() { deallocate(); }
  
      @DATASTORE_MEMBERS@

      void allocate(UInt_t n) override;
      void deallocate() override;
      void setStatus(TTree&, TString const&, panda::utils::BranchList const&) override;
      panda::utils::BranchList getStatus(TTree&, TString const&) const override;
      panda::utils::BranchList getBranchNames(TString const& = "") const override;
      void setAddress(TTree&, TString const&, panda::utils::BranchList const& = {"*"}, Bool_t setStatus = kTRUE) override;
      void book(TTree&, TString const&, panda::utils::BranchList const& = {"*"}, Bool_t dynamic = kTRUE) override;
      void releaseTree(TTree&, TString const&) override;
      void resizeVectors_(UInt_t) override;

      @IF[PHYS_PARENT]@
      @ELSE@
      typedef @NAMESPACE@::SizeBranchName size_name_type;
      @ENDIF@
    };

    typedef panda::Array<@NAME@> array_type;
    typedef panda::Collection<@NAME@> collection_type;

    typedef @PARENT@ base_type;

    @IF[INSTANTIABLE]@
    @NAME@(char const* name = "");
    @NAME@(@NAME@ const&);
    @ENDIF@
    @NAME@(datastore&, UInt_t idx);
    ~@NAME@();
    @NAME@& operator=(@NAME@ const&);
    void destructor(Bool_t recursive = kFALSE);

    static char const* typeName() { return "@NAME@"; }
    static panda::utils::BranchList getListOfBranches();

    void print(std::ostream& = std::cout, UInt_t level = 1) const override;
    void dump(std::ostream& = std::cout) const override;

    @FUNCTIONS@

    @PUBLIC_MEMBERS@

    /* BEGIN CUSTOM classdef */
    /* END CUSTOM */

  protected:
    @PROTECTED_MEMBERS@

    @NAME@(panda::ArrayBase*);

    void doBook_(TTree&, TString const&, panda::utils::BranchList const& = {"*"}) override;
    void doInit_() override;
  };

  typedef panda::Array<@NAME@> @NAME@Array;
  typedef panda::Collection<@NAME@> @NAME@Collection;
  typedef panda::Ref<@NAME@> @NAME@Ref;
  typedef panda::RefVector<@NAME@> @NAME@RefVector;

  /* BEGIN CUSTOM global */
  /* END CUSTOM */

}

#endif

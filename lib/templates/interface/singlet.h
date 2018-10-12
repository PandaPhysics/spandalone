#ifndef @NAMESPACE@_Objects_@NAME@_h
#define @NAMESPACE@_Objects_@NAME@_h

#include "Constants.h"
@INCLUDES@
#include "../../Framework/interface/Array.h"
#include "../../Framework/interface/Collection.h"
#include "../../Framework/interface/Ref.h"
#include "../../Framework/interface/RefVector.h"

namespace @NAMESPACE@ {

  class @NAME@ : public @PARENT@ {
  public:
    @ENUMS@
    @CONSTANTS@

    typedef @PARENT@ base_type;

    @IF[INSTANTIABLE]@
    @NAME@(char const* name = "");
    @NAME@(@NAME@ const&);
    @ENDIF@
    ~@NAME@();
    @NAME@& operator=(@NAME@ const&);

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

    void doSetStatus_(TTree&, panda::utils::BranchList const&) override;
    panda::utils::BranchList doGetStatus_(TTree&) const override;
    panda::utils::BranchList doGetBranchNames_(Bool_t) const override;
    void doSetAddress_(TTree&, panda::utils::BranchList const& = {"*"}, Bool_t setStatus = kTRUE) override;
    void doBook_(TTree&, panda::utils::BranchList const& = {"*"}) override;
    void doInit_() override;
  };

  /* BEGIN CUSTOM global */
  /* END CUSTOM */

}

#endif

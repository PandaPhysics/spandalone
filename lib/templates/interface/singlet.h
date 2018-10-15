#ifndef @NAMESPACE@_@NAME@_h
#define @NAMESPACE@_@NAME@_h

#include "Constants.h"
#include "BranchName.h"
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

    panda::utils::BranchList const& getBranchNames() const override { return @NAME@::branchNames; }

    static char const* typeName() { return "@NAME@"; }

    void print(std::ostream& = std::cout, UInt_t level = 1) const override;
    void dump(std::ostream& = std::cout, UInt_t indent = 0) const override;

    @FUNCTIONS@

    @PUBLIC_MEMBERS@

    /* BEGIN CUSTOM classdef */
    /* END CUSTOM */

    static @NAMESPACE@::BranchList const branchNames;

  protected:
    @PROTECTED_MEMBERS@

    void doSetStatus_(TTree&, panda::utils::BranchList const&) override;
    void doSetAddress_(TTree&, panda::utils::BranchList const& = {"*"}, Bool_t setStatus = kTRUE) override;
    void doBook_(TTree&, panda::utils::BranchList const& = {"*"}) override;
    void doInit_() override;
  };

  /* BEGIN CUSTOM global */
  /* END CUSTOM */

}

#endif

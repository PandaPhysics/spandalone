#ifndef @NAMESPACE@_@NAME@_h
#define @NAMESPACE@_@NAME@_h

#include "Constants.h"
#include "BranchName.h"
@INCLUDES@

namespace @NAMESPACE@ {

  class @NAME@ : public @PARENT@ {
  public:
    @NAME@();
    @NAME@(@NAME@ const&);
    ~@NAME@();
    @NAME@& operator=(@NAME@ const&);

    panda::utils::BranchList const& getBranchNames() const override { return @NAME@::branchNames; }
    
    void print(std::ostream& = std::cout, UInt_t level = 1) const override;
    void dump(std::ostream& = std::cout) const override;

    @FUNCTIONS@

    @OBJBRANCHES@

    @BRANCHES@

    static @NAMESPACE@::BranchList const branchNames;

  protected:
    void doSetStatus_(TTree&, panda::utils::BranchList const&) override;
    void doSetAddress_(TTree&, panda::utils::BranchList const&, Bool_t setStatus) override;
    void doBook_(TTree&, panda::utils::BranchList const&) override;
    void doGetEntry_(TTree&) override;
    void doInit_() override;
    void doUnlink_(TTree&) override;

  public:
    /* BEGIN CUSTOM classdef */
    /* END CUSTOM */
  };

  /* BEGIN CUSTOM global */
  /* END CUSTOM */

}

#endif

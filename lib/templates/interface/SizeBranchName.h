#ifndef @NAMESPACE@_Objects_SizeBranchName_h
#define @NAMESPACE@_Objects_SizeBranchName_h

#include "../../Framework/interface/IOUtils.h"

namespace @NAMESPACE@ {

  //! BranchName specialized for collection size branch.
  /*!
   * SizeBranchName must be instantiable both from the full branch name and just the object name.
   * User-provided parse function must return the name of the object or an empty string.
   */
  class SizeBranchName : public panda::utils::BranchName {
  public:
    SizeBranchName() {}
    SizeBranchName(SizeBranchName const& src) : BranchName(src) {}
    SizeBranchName(char const*);
    SizeBranchName(std::string const& s) : SizeBranchName(s.c_str()) {}
    SizeBranchName(TString const& s) : SizeBranchName(s.Data()) {}

    TString fullName(TString const& = "") const override { return generate(front()); }
    TString internalName() const { return panda::utils::BranchName::fullName(); }
    
    static TString parse(TString const&);
    static TString generate(TString const&);
  };

}

#endif

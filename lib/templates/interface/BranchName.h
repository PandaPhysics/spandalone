#ifndef @NAMESPACE@_BranchName_h
#define @NAMESPACE@_BranchName_h

#include "../../Framework/interface/BranchName.h"
#include "../../Framework/interface/BranchList.h"

namespace @NAMESPACE@ {

  class BranchNameSyntax {
  public:
    static TString generate(TString const&, TString const&);
    static std::pair<TString, TString> parse(TString const&);
  };

  @IF[CUSTOM_SIZEBRANCH]@
  class SizeBranchNameSyntax {
  public:
    static TString generate(TString const&, TString const&);
    static std::pair<TString, TString> parse(TString const&);
  };

  typedef panda::utils::BranchNameImpl<BranchNameSyntax, SizeBranchNameSyntax> BranchName;
  typedef panda::utils::BranchListImpl<BranchNameSyntax, SizeBranchNameSyntax> BranchList;
  @ELSE@
  typedef panda::utils::BranchNameImpl<BranchNameSyntax> BranchName;
  typedef panda::utils::BranchListImpl<BranchNameSyntax> BranchList;
  @ENDIF@

}

#endif

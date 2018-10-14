#ifndef @NAMESPACE@_BranchName_h
#define @NAMESPACE@_BranchName_h

#include "../../Framework/interface/BranchName.h"
#include "../../Framework/interface/BranchList.h"

namespace @NAMESPACE@ {

  class BranchNameSyntax {
  public:
    static TString generate(BranchName const&);
    static std::pair<TString, TString> parse(TString const&);
  };

  typedef panda::utils::BranchNameImpl<BranchNameSyntax> BranchName;
  typedef panda::utils::BranchListImpl<BranchNameSyntax> BranchList;

  @IF[CUSTOM_SIZEBRANCH]@
  class SizeBranchNameSyntax {
  public:
    static TString generate(BranchName const&);
    static std::pair<TString, TString> parse(TString const&);
  };
  @ELSE@
  typedef BranchNameSyntax SizeBranchNameSyntax;
  @ENDIF@

  class SizeBranchName : public panda::utils::BranchNameImpl<SizeBranchNameSyntax> {
    typedef panda::utils::BranchNameImpl<SizeBranchNameSyntax> base_type;
  public:
    SizeBranchName(TString const& obj, bool isVeto = false) : base_type(obj, "size", isVeto) {}
  };

}

#endif

#include "../interface/BranchName.h"

namespace @NAMESPACE@ {

  /*static*/
  TString
  BranchNameSyntax::generate(TString const& _obj, TString const& _br)
  {
    @IF[CUSTOM_BRANCHNAME]@
    @BRANCHNAME_GENERATE@
    @ELSE@
    return _obj + "." + _br;
    @ENDIF@
  }

  /*static*/
  std::pair<TString, TString>
  BranchNameSyntax::parse(TString const& _str)
  {
    @IF[CUSTOM_BRANCHNAME]@
    @BRANCHNAME_PARSE@
    @ELSE@
    auto* parts(_str.Tokenize("."));
    if (parts->GetEntries() != 2) {
      delete parts;
      return "";
    }
  
    std::pair<TString, TString> bname(parts->At(0)->GetName(), parts->At(1)->GetName());
    delete parts;
    return bname;
    @ENDIF@
  }

  @IF[CUSTOM_SIZEBRANCH]@
  /*static*/
  TString
  SizeBranchNameSyntax::generate(TString const& _obj, TString const& _br)
  {
    @SIZEBRANCH_GENERATE@
  }

  /*static*/
  std::pair<TString, TString>
  SizeBranchNameSyntax::parse(TString const& _str)
  {
    @SIZEBRANCH_PARSE@
  }
  @ENDIF@

}

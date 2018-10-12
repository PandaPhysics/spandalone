#include "../interface/SizeBranchName.h"

namespace @NAMESPACE@ {

  SizeBranchName::SizeBranchName(char const* _name)
  {
    auto parsed(parse(_name));
  
    if (parsed.Length() == 0)
      emplace_back(_name);
    else
      emplace_back(parsed);
  
    emplace_back("size");
  }
  
  /*static*/
  TString
  SizeBranchName::parse(TString const& _bname)
  {
    @IF[CUSTOM_SIZEBRANCH]@
    @SIZEBRANCH_PARSE@
    @ELSE@
    auto* parts(_bname.Tokenize(separator));
    if (parts->GetEntries() != 2) {
      delete parts;
      return "";
    }
  
    TString objName(parts->At(0)->GetName());
    delete parts;
    return objName;
    @ENDIF@
  }

  /*static*/
  TString
  SizeBranchName::generate(TString const& _objName)
  {
    @IF[CUSTOM_SIZEBRANCH]@
    @SIZEBRANCH_GENERATE@
    @ELSE@
    return _objName + ".size";
    @ENDIF@
  }

}

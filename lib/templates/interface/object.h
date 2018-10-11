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

  };
}

#endif

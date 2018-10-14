#include "@OUTDIR@/Framework/interface/Object.h"
#include "@OUTDIR@/Framework/interface/ReaderObject.h"
#include "@OUTDIR@/Framework/interface/Singlet.h"
#include "@OUTDIR@/Framework/interface/Element.h"
#include "@OUTDIR@/Framework/interface/ContainerBase.h"
#include "@OUTDIR@/Framework/interface/ArrayBase.h"
#include "@OUTDIR@/Framework/interface/CollectionBase.h"
#include "@OUTDIR@/Framework/interface/TreeEntry.h"
#include "@OUTDIR@/Framework/interface/RRNG.h"
@INCLUDES@

#ifdef __CLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;

#pragma link C++ namespace panda;

#pragma link C++ class panda::Object;
#pragma link C++ class panda::ReaderObject;
#pragma link C++ class panda::Singlet;
#pragma link C++ class panda::Element;
#pragma link C++ class panda::ContainerBase;
#pragma link C++ class panda::ArrayBase;
#pragma link C++ class panda::CollectionBase;
#pragma link C++ class panda::TreeEntry;
#pragma link C++ class panda::RRNG;

@IF[CUSTOM_NAMESPACE]@
#pragma link C++ namespace @NAMESPACE@;
@ENDIF@

@ENUMS@
@OBJECTS@
@GENERICS@
@CONTAINERS@
@TREES@

#endif

@INCLUDES@

#ifdef __CLING__
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ nestedclass;
#pragma link C++ nestedtypedef;
#pragma link C++ namespace @NAMESPACE@;

@ENUMS@
@OBJECTS@
@GENERICS@
@CONTAINERS@
@TREES@

#endif

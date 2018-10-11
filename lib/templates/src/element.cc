#include "../interface/@NAME@.h"

namespace @NAMESPACE@ {

  @ENUM_STRS@
  @CONSTANTS@
  
  /*static*/
  panda::utils::BranchList
  @NAME@::getListOfBranches()
  {
    @IF[PHYS_PARENT]@
    panda::utils::BranchList blist(@PARENT@::getListOfBranches());
    @ELSE@
    panda::utils::BranchList blist;
    @ENDIF@

    blist += {@BNAMES@};

    return blist;
  }

  void
  @NAME@::datastore::allocate(UInt_t _nmax)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::datastore::allocate(_nmax);

    @ENDIF@
    @ALLOCATE@
  }

  void
  @NAME@::datastore::deallocate()
  {
    @IF[PHYS_PARENT]@
    @PARENT@::datastore::deallocate();

    @ENDIF@
    @DEALLOCATE@
  }

  void
  @NAME@::datastore::setStatus(TTree& _tree, TString const& _name, panda::utils::BranchList const& _branches)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::datastore::setStatus(_tree, _name, _branches);

    @ENDIF@
    @SET_STATUS@
  }

  panda::utils::BranchList
  @NAME@::datastore::getStatus(TTree& _tree, TString const& _name) const
  {
    @IF[PHYS_PARENT]@
    panda::utils::BranchList(@PARENT@::datastore::getStatus(_tree, _name));
    @ELSE@
    panda::utils::BranchList;
    @ENDIF@

    @GET_STATUS@

    return blist;
  }

  void
  @NAME@::datastore::setAddress(TTree& _tree, TString const& _name, panda::utils::BranchList const& _branches/* = {"*"}*/, Bool_t _setStatus/* = kTRUE*/)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::datastore::setAddress(_tree, _name, _branches, _setStatus);

    @ENDIF@
    @SET_ADDRESS@
  }

  void
  @NAME@::datastore::book(TTree& _tree, TString const& _name, panda::utils::BranchList const& _branches/* = {"*"}*/, Bool_t _dynamic/* = kTRUE*/)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::datastore::book(_tree, _name, _branches, _dynamic);

    @ENDIF@
    TString size(_dynamic ? "[" + _name + ".size]" : TString::Format("[%d]", nmax_));

    @DSBOOK@
  }

  void
  @NAME@::datastore::releaseTree(TTree& _tree, TString const& _name)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::datastore::releaseTree(_tree, _name);

    @ENDIF@
    @RELEASE_TREE@
  }

  void
  @NAME@::datastore::resizeVectors_(UInt_t _size)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::datastore::resizeVectors_(_size);

    @ENDIF@
    @RESIZE_VECTORS@
  }

  panda::utils::BranchList
  @NAME@::datastore::getBranchNames(TString const& _name/* = ""*/) const
  {
    return @NAME@::getListOfBranches().fullNames(_name);
  }

  @IF[INSTANTIABLE]@
  @NAME@::@NAME@(char const* _name/* = ""*/) :
    @INITIALIZERS_DEFAULT@
  {
    @DEFAULT_CTOR@
  }

  @NAME@::@NAME@(@NAME@ const& _src) :
    @INITIALIZERS_COPY@
  {
    @PARENT@::operator=(_src);

    @COPY_CTOR@
  }
  @ENDIF@

  @NAME@::@NAME@(datastore& _data, UInt_t _idx) :
    @INITIALIZERS_STANDARD@
  {
    @STANDARD_CTOR@
  }

  /*protected*/
  @NAME@::@NAME@(ArrayBase* _array) :
    @INITIALIZERS_PRIVATE@
  {
    @DEFAULT_CTOR@
  }

  @NAME@::~@NAME@()
  {
    destructor();
  }

  @NAME@&
  @NAME@::operator=(@NAME@ const& _src)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::operator=(_src);

    @ENDIF@
    @ASSIGNMENT@

    /* BEGIN CUSTOM assignment */
    /* END CUSTOM */
  }

  void
  @NAME@::destructor(Bool_t _recursive/* = kFALSE*/)
  {
    /* BEGIN CUSTOM destructor */
    /* END CUSTOM */

    @DESTRUCTOR@
  }

  /*protected*/
  void
  @NAME@::doBook_(TTree& _tree, TString const& _name, panda::utils::BranchList const& _branches/* = {"*"}*/)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::doBook_(_tree, _name, _branches);

    @ENDIF@
    @BOOK@
  }

  /*protected*/
  void
  @NAME@::doInit_()
  {
    @IF[PHYS_PARENT]@
    @PARENT@::doInit_();

    @ENDIF@
    @INIT@

    /* BEGIN CUSTOM init */
    /* END CUSTOM */
  }

  void
  @NAME@::print(std::ostream& _out/* = std::cout*/, UInt_t _level/* = 1*/) const
  {
    /* BEGIN CUSTOM print */
    dump(_out);
    /* END CUSTOM */
  }

  void
  @NAME@::dump(std::ostream& _out/* = std::cout*/) const
  {
    @IF[PHYS_PARENT]@
    @PARENT@::dump(_out);

    @ENDIF@
    @DUMP@
  }

  @FUNCTIONS@

  /* BEGIN CUSTOM global */
  /* END CUSTOM */

}

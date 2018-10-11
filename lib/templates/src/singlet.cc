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
    
  @IF[INSTANTIABLE]@
  @NAME@::@NAME@(char const* _name/* = ""*/) :
    @INITIALIZERS_DEFAULT@
  {
    @DEFAULT_CTOR@
  }

  @NAME@::@NAME@(@NAME@ const& _src) :
    @INITIALIZERS_COPY@
  {
    @COPY_CTOR@
  }
  @ENDIF@

  @NAME@::~@NAME@()
  {
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

    return *this;
  }

  /*protected*/
  void
  @NAME@::doSetStatus_(TTree& _tree, panda::utils::BranchList const& _branches)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::doSetStatus_(_tree, _branches);

    @ENDIF@
    @SET_STATUS@
  }

  /*protected*/
  panda::utils::BranchList
  @NAME@::doGetStatus_(TTree& _tree) const
  {
    @IF[PHYS_PARENT]@
    panda::utils::BranchList blist(@PARENT@::doGetStatus_(_tree));
    @ELSE@
    panda::utils::BranchList blist;
    @ENDIF@

    @GET_STATUS@

    return blist;
  }

  /*protected*/
  void
  @NAME@::doSetAddress_(TTree& _tree, panda::utils::BranchList const& _branches/* = {"*"}*/, Bool_t _setStatus/* = kTRUE*/)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::doSetAddress_(_tree, _branches, _setStatus);
    
    @ENDIF@
    @SET_ADDRESS@
  }

  /*protected*/
  void
  @NAME@::doBook_(TTree& _tree, panda::utils::BranchList const& _branches/* = {"*"}*/)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::doBook_(_tree, _branches);

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

  /*protected*/
  panda::utils::BranchList
  @NAME@::doGetBranchNames_(Bool_t _fullName) const
  {
    if (_fullName)
      return getListOfBranches().fullNames(name_);
    else
      return getListOfBranches().fullNames();
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

    @ENDIF
    @DUMP@
  }

  @FUNCTIONS@

  /* BEGIN CUSTOM global */
  /* END CUSTOM */

}

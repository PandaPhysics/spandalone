#include "../interface/@NAME@.h"

namespace @NAMESPACE@ {

  @NAME@::@NAME@() :
    @PARENT@()
  {
    @INIT_OBJPTRS@
    @INIT_COLLPTRS@
    @INIT_REFS@

    /* BEGIN CUSTOM default_ctor */
    /* END CUSTOM */
  }

  @NAME@::@NAME@(@NAME@ const& _src) :
    @INITIALIZERS_COPY@
  {
    @INIT_OBJPTRS@
    @INIT_COLLPTRS@
    @INIT_REFS@

    /* BEGIN CUSTOM copy_ctor */
    /* END CUSTOM */
  }

  @NAME@::~@NAME@()
  {
    /* BEGIN CUSTOM dtor */
    /* END CUSTOM */
  }

  @NAME@&
  @NAME@::operator=(@NAME@ const& _src)
  {
    @PARENT@::operator=(_src);

    /* BEGIN CUSTOM assignment */
    /* END CUSTOM */

    @ASSIGNMENTS@

    return *this;
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

  /*static*/
  panda::utils::BranchList
  @NAME@::getListOfBranches(Bool_t _direct/* = kFALSE*/)
  {
    panda::utils::BranchList blist;
    
    @IF[PHYS_PARENT]@
    blist += @PARENT@::getListOfBranches(_direct);
    @ENDIF@

    blist += {@BNAMES@};

    if (!_direct) {
      @OBJ_BLIST@
    }

    /* BEGIN CUSTOM getListOfBranches */
    /* END CUSTOM */

    return blist;
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
    panda::utils::BranchList blist(@PARENT@::doGetStatus(_tree));
    @ELSE@
    panda::utils::BranchList blist;
    @ENDIF@

    @GET_STATUS@

    return blist;
  }
  
  /*protected*/
  panda::utils::BranchList
  @NAME@::doGetBranchNames_() const
  {
    return getListOfBranches(true);
  }
  
  /*protected*/
  void
  @NAME@::doSetAddress_(TTree& _tree, panda::utils::BranchList const& _branches, Bool_t _setStatus)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::doSetAddress_(_tree, _branches, _setStatus);

    @ENDIF@
    @SET_ADDRESS@
  }
  
  /*protected*/
  void
  @NAME@::doBook_(TTree& _tree, panda::utils::BranchList const& _branches)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::doBook_(_tree, _branches);

    @ENDIF@
    @BOOK@
  }
  
  /*protected*/
  void
  @NAME@::doGetEntry_(TTree& _tree)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::doGetEntry_(_tree);

    @ENDIF@
  
    /* BEGIN CUSTOM get_entry */
    /* END CUSTOM */
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
  void
  @NAME@::doUnlink_(TTree& _tree)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::doUnlink_(_tree);

    @ENDIF@
    /* BEGIN CUSTOM unlink */
    /* END CUSTOM */
  }
  
  @FUNCTIONS@
  
  /* BEGIN CUSTOM global */
  /* END CUSTOM */

}

#include "../interface/@NAME@.h"
#include "../../Framework/interface/IOUtils.h"

namespace @NAMESPACE@ {

  /*static*/
  BranchList const @NAME@::branchNames{std::initializer_list<TString>{@BNAMES@}};

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
  @NAME@::dump(std::ostream& _out/* = std::cout*/, UInt_t _indent/* = 0*/) const
  {
    std::string indentation(_indent * 2, ' ');

    @PARENT@::dump(_out, _indent);
    @DUMP@
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

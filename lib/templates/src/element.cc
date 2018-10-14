#include "../interface/@NAME@.h"
#include "../../Framework/interface/IOUtils.h"

namespace panda {
  /*static*/
  template<>
  @NAMESPACE@::BranchList const Collection<@NAMESPACE@::@NAME@>::branchNames{std::initializer_list<TString>{@BNAMES_WITH_SIZE@}};
}

namespace @NAMESPACE@ {

  @ENUM_STRS@
  @CONSTANTS@

  /*static*/
  BranchList const @NAME@::branchNames{std::initializer_list<TString>{@BNAMES@}};
 
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

  void
  @NAME@::datastore::setAddress(TTree& _tree, TString const& _name, panda::utils::BranchList const& _branches/* = {"*"}*/, Bool_t _setStatus/* = kTRUE*/)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::datastore::setAddress(_tree, _name, _branches, _setStatus);

    @ENDIF@
    @SET_ADDRESS@
  }

  void
  @NAME@::datastore::book(TTree& _tree, TString const& _name, panda::utils::BranchList const& _branches/* = {"*"}*/, BookAs _bookAs/* = aCollection*/)
  {
    @IF[PHYS_PARENT]@
    @PARENT@::datastore::book(_tree, _name, _branches, _bookAs);

    @ENDIF@
    @IF[STD_VECTOR_BRANCHES]@
    @ELSE@
    TString size;

    switch (_bookAs) {
    case aCollection:
      size = TString::Format("[%s]", BranchName(_name, "size").toString());
      break;
    case aArray:
      size = TString::Format("[%d]", nmax_);
      break;
    case aSinglet:
      size = "";
      break;
    }
    @ENDIF@

    @BOOK@
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
  @NAME@::@NAME@(panda::ArrayBase* _array) :
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

    return *this;
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

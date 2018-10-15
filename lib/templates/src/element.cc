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
    @PARENT@::datastore::allocate(_nmax);

    @ALLOCATE@
  }

  void
  @NAME@::datastore::deallocate()
  {
    @PARENT@::datastore::deallocate();

    @DEALLOCATE@
  }

  void
  @NAME@::datastore::setStatus(TTree& _tree, TString const& _name, panda::utils::BranchList const& _branches)
  {
    @PARENT@::datastore::setStatus(_tree, _name, _branches);

    @SET_STATUS@
  }

  void
  @NAME@::datastore::setAddress(TTree& _tree, TString const& _name, panda::utils::BranchList const& _branches/* = {"*"}*/, Bool_t _setStatus/* = kTRUE*/)
  {
    @PARENT@::datastore::setAddress(_tree, _name, _branches, _setStatus);

    @SET_ADDRESS@
  }

  void
  @NAME@::datastore::book(TTree& _tree, TString const& _name, panda::utils::BranchList const& _branches/* = {"*"}*/, BookAs _bookAs/* = aCollection*/)
  {
    @PARENT@::datastore::book(_tree, _name, _branches, _bookAs);

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
    @PARENT@::datastore::releaseTree(_tree, _name);

    @RELEASE_TREE@
  }

  void
  @NAME@::datastore::resizeVectors_(UInt_t _size)
  {
    @PARENT@::datastore::resizeVectors_(_size);

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
    @PARENT@::operator=(_src);

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
  @NAME@::dump(std::ostream& _out/* = std::cout*/, UInt_t _indent/* = 0*/) const
  {
    @PARENT@::dump(_out, _indent);
    std::string indentation(_indent * 2, ' ');
    @DUMP@
  }

  @FUNCTIONS@

  /* BEGIN CUSTOM global */
  /* END CUSTOM */

}

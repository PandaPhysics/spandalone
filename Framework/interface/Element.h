#ifndef PandaTree_Framework_Element_h
#define PandaTree_Framework_Element_h

#include "Object.h"
#include "IOUtils.h"

#include <map>

namespace panda {

  class ContainerBase;
  class ArrayBase;
  class CollectionBase;

  //! Base class for elements of containers.
  /*!
   * Element is the base class of objects that are elements of containers (Array = fixed size and Collection = dynamic size).
   * All deriving class of Element must have a member class named datastore (which derives from Element::datastore) where
   * arrays of plain-old-data types and vectors of objects are held. This big chunk of memory is in turn owned by a Container, which
   * also holds an array of Elements. Individual "data members" of a Element-derived class are references to the
   * elements of their associated datastore, linked by the Container.
   * By construction, the standard usage of the Element object is therefore to define a container first and to fetch from it
   * as an element. However, it is also possible to use a Element class as a singlet. This operation is rather expensive as
   * every singlet instantiation of a Element will create a one-element Array in memory in the back end.
   *
   * Note on destructors:
   * The actual destructor of the is called only if the Element is instantiated as a singlet.
   * In the Container environment, the memory allocated for this object is directly deallocated without
   * calling ~(Object). If subclasses need to make some calls at the destruction time of the object, they
   * must be implemented in the destructor() function.
   */
  class Element : public Object {
  public:
    /*!
     * Actual arrays and vectors written to the tree are members of datastore. For example, given a tree with branches
     * particle.size/i
     * particle.pt[particle.size]/F
     * the Particle datastore will have
     * Float_t pt[N];
     * which gets written to / read from the tree.
     * The i-th Particle instance on the other hand has
     * Float_t& pt;
     * which is a reference to datastore::pt[i].
     */
    struct datastore {
      datastore() {}
      virtual ~datastore() { deallocate(); }

      virtual void allocate(UInt_t n) { nmax_ = n; }
      virtual void deallocate() {}
      virtual void setStatus(TTree&, TString const&, utils::BranchList const&) {}
      virtual utils::BranchList getStatus(TTree&, TString const&) const { return utils::BranchList(); }
      virtual utils::BranchList getBranchNames(TString const& = "") const { return utils::BranchList(); }
      virtual void setAddress(TTree&, TString const&, utils::BranchList const& = {"*"}, Bool_t setStatus = kTRUE) {}
      virtual void book(TTree&, TString const&, utils::BranchList const& = {"*"}, Bool_t dynamic = kTRUE) {}
      virtual void releaseTree(TTree&, TString const&) {}

      UInt_t nmax() const { return nmax_; }

      //! Override when there are vector members
      /*!
       * This method should only be used from Collections
       */
      virtual void resizeVectors_(UInt_t) {}

    protected:
      UInt_t nmax_;
    };

    typedef ArrayBase array_type;
    typedef CollectionBase collection_type;

    //! Disabled default constructor.
    Element() = delete;
    //! Standard constructor.
    Element(datastore&, UInt_t) {}
    //! Copy constructor.
    /*!
     * Copy construction is similar to default construction, and involves a creation of a one-element Array.
     */
    Element(Element const& src) : Object(src) {}
    ~Element();
    Element& operator=(Element const&) { return *this; }

    static char const* typeName() { return "Element"; }

    void setStatus(TTree&, utils::BranchList const& blist) final;
    utils::BranchList getStatus(TTree&) const final;
    utils::BranchList getBranchNames(Bool_t fullName = kTRUE, Bool_t = kFALSE) const final;
    UInt_t setAddress(TTree&, utils::BranchList const& blist = {"*"}, Bool_t setStatus = kTRUE) final;
    void book(TTree&, utils::BranchList const& blist = {"*"}) final;
    Int_t getEntry(TTree& tree, Long64_t entry, Bool_t localEntry = kFALSE) final;
    Int_t getEntry(UInt_t treeId, Long64_t entry, Bool_t localEntry = kFALSE) final;
    void init() final { doInit_(); }
    char const* getName() const final;
    void setName(char const*) final;

  protected:
    //! Ctor for singlet instantiation
    /*!
     * When a derived class is instantiated as a singlet, the object must create a new one-element Array
     * and set its references to the element of the array. This array is controlled by a global StoreManager
     * instance.
     */
    Element(ArrayBase*);

    virtual void doBook_(TTree&, TString const&, utils::BranchList const&) = 0;
    virtual void doInit_() = 0;

    //! Singleton class for bookkeeping of Elements constructed as singlets.
    /*!
     * Element objects are internally collections of references to elements of multiple arrays and are
     * not naturally constructible as singlets. StoreManager keeps a global list of arrays which are
     * referenced by singlet Elements. When a singlet Element is destructed, the slots in StoraManager
     * it referred to are freed and is recycled for the next instantiation of an Element object.
     */
    class StoreManager : public std::map<Element const*, ArrayBase*> {
    public:
      ArrayBase& getArray(Element const*) const;
      template<class O> typename O::datastore& getData(O const*) const;
      char const* getName(Element const*) const;
    };

    static StoreManager gStore;
  };

  template<class O>
  typename O::datastore&
  Element::StoreManager::getData(O const* _obj) const
  {
    return static_cast<typename O::datastore&>(getArray(_obj).getData());
  }
}

#endif

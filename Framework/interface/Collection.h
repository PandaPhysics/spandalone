#ifndef PandaTree_Framework_Collection_h
#define PandaTree_Framework_Collection_h

#include "CollectionBase.h"
#include "Iterator.h"

#include <stdexcept>
#include <memory>

namespace panda {

  //! Template class for dynamic-size container implementations. Inherits from base_type::collection_type of the element class.
  /*!
   * For a physics object class E that inherits from B, Panda containers inherit as
   * Object <- ContainerBase <- ArrayBase <- Array<B> <- Array<E>
   * Object <- ContainerBase <- CollectionBase <- Collection<B> <- Collection<E>
   */
  template<class E>
  class Collection : public E::base_type::collection_type {
  public:
    typedef Collection<E> self_type;
    typedef typename E::base_type::collection_type base_type;
    typedef E value_type;
    typedef typename value_type::datastore data_type;
    typedef value_type& reference;
    typedef value_type const& const_reference;
    typedef value_type* pointer;
    typedef value_type const* const_pointer;
    typedef utils::Iterator<self_type, false> iterator;
    typedef utils::Iterator<self_type, true> const_iterator;

    Collection(char const* name = "", UInt_t initialMax = 64) : base_type(name, sizeof(E), kFALSE) { allocate_(initialMax); }
    Collection(self_type const& src) : base_type(src.ContainerBase::name_, sizeof(E), kFALSE) { allocate_(src.data.nmax()); copy(src); }
    ~Collection();
    self_type& operator=(self_type const& rhs) { copy(rhs); return *this; }

    //! Element accessor with range check
    reference at(UInt_t idx);
    //! Element accessor with range check
    const_reference at(UInt_t idx) const;
    //! Element accessor with no range check
    reference operator[](UInt_t idx) { return *addr_(idx); }
    //! Element accessor with no range check
    const_reference operator[](UInt_t idx) const { return *const_addr_(idx); }
    //! Return an iterator pointing to the first element
    iterator begin() { return iterator(addr_(), ContainerBase::unitSize_); }
    //! Return an iterator pointing to the first element
    const_iterator begin() const { return const_iterator(const_addr_(), ContainerBase::unitSize_); }
    //! Return an iterator pointing to the end of the array (invalid address)
    iterator end() { return iterator(addr_(CollectionBase::size()), ContainerBase::unitSize_); }
    //! Return an iterator pointing to the end of the array (invalid address)
    const_iterator end() const { return const_iterator(const_addr_(CollectionBase::size()), ContainerBase::unitSize_); }
    //! Reference to the first element
    reference front() { return *addr_(); }
    //! Reference to the first element
    const_reference front() const { return *const_addr_(); }
    //! Reference to the last element
    reference back() { return *addr_(CollectionBase::size() - 1); }
    //! Reference to the last element
    const_reference back() const { return *const_addr_(CollectionBase::size() - 1); }
    //! Copy the array contents.
    /*!
     * If the new size is greater than nmax, data will be reallocated, making all references invalid.
     */
    void copy(self_type const&);
    //! Append an element to the back and resize by 1.
    /*!
     * If the new size is greater than nmax, data will be reallocated, making all references invalid.
     * Note that this method is not virtual; if calling from a base class, the new element will be copied
     * only partially.
     */
    void push_back(const_reference);
    //! Create an element at the end of the collection and return a reference.
    /*!
     * If the new size is greater than nmax, data will be reallocated, making all references invalid.
     */
    reference create_back();

    Element::datastore& getData() override { return data; }
    Element::datastore const& getData() const override { return data; }
    Element& elemAt(UInt_t idx) override { return at(idx); }
    Element const& elemAt(UInt_t idx) const override { return at(idx); }

    std::vector<UInt_t> sort(ContainerBase::Comparison const&) override;

    void print(std::ostream& = std::cout, UInt_t level = 1) const override;
    void dump(std::ostream& = std::cout) const override;

    data_type data{};

  protected:
    Collection(char const* name, UInt_t unitSize, Bool_t dummy) : base_type(name, unitSize, kFALSE) {}

    void reallocate_(UInt_t) override;

  private:
    value_type* addr_(unsigned idx = 0);
    value_type const* const_addr_(unsigned idx = 0) const;

    template<class T = E>
    typename std::enable_if<std::is_constructible<T>::value>::type allocate_(UInt_t);
    template<class T = E>
    typename std::enable_if<!std::is_constructible<T>::value>::type allocate_(UInt_t);

    template<class T = E>
    typename std::enable_if<std::is_constructible<T>::value>::type
    deallocate_(value_type* addr, data_type&);
    template<class T = E>
    typename std::enable_if<!std::is_constructible<T>::value>::type
    deallocate_(value_type* addr, data_type&);
  };

  template<class E>
  Collection<E>::~Collection()
  {
    if (ContainerBase::array_) {
      deallocate_(addr_(), data);

      // set the pointer to null so that the parent class destructors don't deallocate again
      ContainerBase::array_ = 0;
    }
  }

  template<class E>
  typename Collection<E>::reference
  Collection<E>::at(UInt_t _idx)
  {
    if (_idx >= CollectionBase::size())
      throw std::out_of_range((ContainerBase::name_ + "::at").Data());

    return *addr_(_idx);
  }

  template<class E>
  typename Collection<E>::const_reference
  Collection<E>::at(UInt_t _idx) const
  {
    if (_idx >= CollectionBase::size())
      throw std::out_of_range((ContainerBase::name_ + "::at").Data());
  
    return *const_addr_(_idx);
  }

  template<class E>
  void
  Collection<E>::copy(self_type const& _src)
  {
    if (_src.ContainerBase::unitSize_ != ContainerBase::unitSize_)
      throw std::runtime_error((ContainerBase::name_ + "::copy incompatible array").Data());

    CollectionBase::resize(_src.size());
    
    for (UInt_t iP(0); iP != CollectionBase::size(); ++iP)
      (*this)[iP] = _src[iP];

    ContainerBase::setName(_src.name_);
  }

  template<class E>
  void
  Collection<E>::push_back(const_reference _elem)
  {
    CollectionBase::resize(CollectionBase::size() + 1);
    back() = _elem;
  }

  template<class E>
  typename panda::Collection<E>::reference
  Collection<E>::create_back()
  {
    CollectionBase::resize(CollectionBase::size() + 1);
    auto& e(back());
    e.init();
    return e;
  }

  template<class E>
  std::vector<UInt_t>
  Collection<E>::sort(ContainerBase::Comparison const& _c)
  {
    // define an index comparison using the element comparison
    auto indexComp([this, &_c](UInt_t i1, UInt_t i2)->Bool_t {
        return _c((*this)[i1], (*this)[i2]);
      });

    std::vector<UInt_t> sortedIndices(CollectionBase::size());
    for (UInt_t iP(0); iP != CollectionBase::size(); ++iP)
      sortedIndices[iP] = iP;

    std::sort(sortedIndices.begin(), sortedIndices.end(), indexComp);

    self_type tmpCollection(*this);
    for (UInt_t iP(0); iP != CollectionBase::size(); ++iP)
      (*this)[iP] = tmpCollection[sortedIndices[iP]];

    return sortedIndices;
  }

  template<class E>
  void
  Collection<E>::print(std::ostream& _out/* = std::cout*/, UInt_t _level/* = 1*/) const
  {
    _out << E::typeName() << "Collection" << std::endl;
    CollectionBase::print(_out, _level);
  }

  template<class E>
  void
  Collection<E>::dump(std::ostream& _out/* = std::cout*/) const
  {
    _out << E::typeName() << "Collection" << std::endl;
    CollectionBase::dump(_out);
  }

  /*protected*/
  template<class E>
  void
  Collection<E>::reallocate_(UInt_t _nmax)
  {
    if (_nmax <= data.nmax())
      return;

    // keep the copy of the pointers temporarily
    // tmpStore is itself a bunch of pointers (no values are copied here)
    data_type tmpStore(data);
    value_type* tmpArray(reinterpret_cast<value_type*>(ContainerBase::array_));

    // allocate a new chunk of memory in heap
    allocate_(_nmax);

    // copy old values
    // CollectionBase::size() still has the old value
    for (UInt_t iP(0); iP != CollectionBase::size(); ++iP)
      (*this)[iP] = tmpArray[iP];

    // deallocate old space
    deallocate_(tmpArray, tmpStore);
  }

  /*private*/
  template<class E>
  typename Collection<E>::value_type*
  Collection<E>::addr_(unsigned _idx/* = 0*/)
  {
    // Here we may be calling from a base class (when E0 <- E1, (Collection<E0>*)(&t1_array)->addr_(i) should properly point to the second *E1*, not E0),
    // which means we must shift by _idx * unitSize_ instead of _idx * sizeof(value_type).

    return reinterpret_cast<value_type*>(ContainerBase::array_ + _idx * ContainerBase::unitSize_);
  }

  /*private*/
  template<class E>
  typename Collection<E>::value_type const*
  Collection<E>::const_addr_(unsigned _idx/* = 0*/) const
  {
    // Here we may be calling from a base class (when E0 <- E1, (Collection<E0>*)(&t1_array)->addr_(i) should properly point to the second *E1*, not E0),
    // which means we must shift by _idx * unitSize_ instead of _idx * sizeof(value_type).

    return reinterpret_cast<value_type const*>(ContainerBase::array_ + _idx * ContainerBase::unitSize_);
  }

  /*private*/
  template<class E>
  template<class T/* = E*/>
  typename std::enable_if<std::is_constructible<T>::value>::type
  Collection<E>::allocate_(UInt_t _nmax)
  {
    data.allocate(_nmax);

    ContainerBase::array_ = reinterpret_cast<Char_t*>(std::allocator<value_type>().allocate(data.nmax()));

    value_type* p(addr_());
    for (UInt_t iP(0); iP != data.nmax(); ++iP, ++p)
      new (p) value_type(data, iP);
  }

  /*private*/
  template<class E>
  template<class T/* = E*/>
  typename std::enable_if<!std::is_constructible<T>::value>::type
  Collection<E>::allocate_(UInt_t)
  {
    throw std::runtime_error(("Cannot create a Collection of pure-virtual elements (" + ContainerBase::name_ + ")").Data());
  }

  /*private*/
  template<class E>
  template<class T/* = E*/>
  typename std::enable_if<std::is_constructible<T>::value>::type
  Collection<E>::deallocate_(value_type* _addr, data_type& _data)
  {
    // Call the destructor of the array elements
    value_type* p(_addr);
    for (UInt_t iP(0); iP != _data.nmax(); ++iP, ++p)
      p->destructor(true);

    std::allocator<value_type>().deallocate(_addr, _data.nmax());
    _data.deallocate();
  }

  /*private*/
  template<class E>
  template<class T/* = E*/>
  typename std::enable_if<!std::is_constructible<T>::value>::type
  Collection<E>::deallocate_(value_type*, data_type&)
  {
    throw std::runtime_error(("Cannot deallocate pure-virtual elements (" + ContainerBase::name_ + ")").Data());
  }

}

#endif

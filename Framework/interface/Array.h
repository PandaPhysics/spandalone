#ifndef PandaTree_Framework_Array_h
#define PandaTree_Framework_Array_h

#include "ArrayBase.h"
#include "Iterator.h"

#include <stdexcept>
#include <memory>
#include <type_traits>

namespace panda {

  //! Template class for fixed-size container implementations. Inherits from base_type::array_type of the element class.
  /*!
   * For a physics object class E that inherits from B, Panda containers inherit as
   * Object <- ContainerBase <- ArrayBase <- Array<B> <- Array<E>
   * Object <- ContainerBase <- CollectionBase <- Collection<B> <- Collection<E>
   */
  template<class E>
  class Array : public E::base_type::array_type {
  public:
    typedef Array<E> self_type;
    typedef typename E::base_type::array_type base_type;
    typedef E value_type;
    typedef value_type& reference;
    typedef value_type const& const_reference;
    typedef value_type* pointer;
    typedef value_type const* const_pointer;
    typedef utils::Iterator<self_type, false> iterator;
    typedef utils::Iterator<self_type, true> const_iterator;

    Array(UInt_t size, char const* name = "") : base_type(name, sizeof(E), kFALSE) { allocate_(size); }
    Array(self_type const& src) : base_type(src.name_, sizeof(E), kFALSE) { allocate_(src.data.nmax()); copy(src); }
    ~Array();
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
    iterator end() { return iterator(addr_(data.nmax()), ArrayBase::ContainerBase::unitSize_); }
    //! Return an iterator pointing to the end of the array (invalid address)
    const_iterator end() const { return const_iterator(const_addr_(data.nmax()), ContainerBase::unitSize_); }
    //! Reference to the first element
    reference front() { return *addr_(); }
    //! Reference to the first element
    const_reference front() const { return *const_addr_(); }
    //! Reference to the last element
    reference back() { return *addr_(data.nmax() - 1); }
    //! Reference to the last element
    const_reference back() const { return *const_addr_(data.nmax() - 1); }
    //! Copy the array contents.
    void copy(self_type const&);

    Element::datastore& getData() override { return data; }
    Element::datastore const& getData() const override { return data; }
    Element& elemAt(UInt_t idx) override { return at(idx); }
    Element const& elemAt(UInt_t idx) const override { return at(idx); }

    std::vector<UInt_t> sort(ContainerBase::Comparison const&) override;

    void print(std::ostream& = std::cout, UInt_t level = 1) const override;
    void dump(std::ostream& = std::cout) const override;

    typename value_type::datastore data{};

  protected:
    Array(char const* name, UInt_t unitSize, Bool_t dummy) : base_type(name, unitSize, kFALSE) {}

  private:
    value_type* addr_(UInt_t idx = 0);
    value_type const* const_addr_(UInt_t idx = 0) const;

    template<class T = E>
    typename std::enable_if<std::is_constructible<T>::value>::type allocate_(UInt_t);
    template<class T = E>
    typename std::enable_if<!std::is_constructible<T>::value>::type allocate_(UInt_t);
  };

  template<class E>
  Array<E>::~Array()
  {
    if (ContainerBase::array_) {
      // Call the destructor of the array elements
      auto* p(addr_());
      for (UInt_t iP(0); iP != data.nmax(); ++iP, ++p)
        p->destructor(true);

      std::allocator<value_type>().deallocate(addr_(), data.nmax());
      data.deallocate();

      // set the pointer to null so that the parent class destructors don't deallocate again
      ContainerBase::array_ = 0;
    }
  }

  template<class E>
  typename Array<E>::reference
  Array<E>::at(UInt_t _idx)
  {
    if (_idx >= data.nmax())
      throw std::out_of_range((ContainerBase::name_ + "::at").Data());

    return *addr_(_idx);
  }

  template<class E>
  typename Array<E>::const_reference
  Array<E>::at(UInt_t _idx) const
  {
    if (_idx >= data.nmax())
      throw std::out_of_range((ContainerBase::name_ + "::at").Data());
  
    return *const_addr_(_idx);
  }

  template<class E>
  void
  Array<E>::copy(self_type const& _src)
  {
    if (_src.ContainerBase::unitSize_ != ContainerBase::unitSize_)
      throw std::runtime_error((ContainerBase::name_ + "::copy incompatible array").Data());

    if (_src.data.nmax() != data.nmax())
      throw std::runtime_error((ContainerBase::name_ + "::copy array size does not match").Data());
    
    for (UInt_t iP(0); iP != data.nmax(); ++iP)
      (*this)[iP] = _src[iP];

    ContainerBase::setName(_src.name_);
  }

  template<class E>
  std::vector<UInt_t>
  Array<E>::sort(ContainerBase::Comparison const& _c)
  {
    // define an index comparison using the element comparison
    auto indexComp([this, &_c](UInt_t i1, UInt_t i2)->Bool_t {
        return _c((*this)[i1], (*this)[i2]);
      });

    std::vector<UInt_t> sortedIndices(data.nmax());
    for (UInt_t iP(0); iP != data.nmax(); ++iP)
      sortedIndices[iP] = iP;

    std::sort(sortedIndices.begin(), sortedIndices.end(), indexComp);

    self_type tmpCollection(*this);
    for (UInt_t iP(0); iP != data.nmax(); ++iP)
      (*this)[iP] = tmpCollection[sortedIndices[iP]];

    return sortedIndices;
  }

  template<class E>
  void
  Array<E>::print(std::ostream& _out/* = std::cout*/, UInt_t _level/* = 1*/) const
  {
    _out << E::typeName() << "Array(" << ArrayBase::size() << ")" << std::endl;
    ContainerBase::print(_out, _level);
  }

  template<class E>
  void
  Array<E>::dump(std::ostream& _out/* = std::cout*/) const
  {
    _out << E::typeName() << "Array(" << ArrayBase::size() << ")" << std::endl;
    ContainerBase::dump(_out);
  }

  /*private*/
  template<class E>
  typename Array<E>::value_type*
  Array<E>::addr_(unsigned _idx/* = 0*/)
  {
    // Here we may be calling from a base class (when E0 <- E1, (Array<E0>*)(&t1_array)->addr_(i) should properly point to the second *E1*, not E0),
    // which means we must shift by _idx * unitSize_ instead of _idx * sizeof(value_type).

    return reinterpret_cast<value_type*>(ContainerBase::array_ + _idx * ContainerBase::unitSize_);
  }

  /*private*/
  template<class E>
  typename Array<E>::value_type const*
  Array<E>::const_addr_(unsigned _idx/* = 0*/) const
  {
    // Here we may be calling from a base class (when E0 <- E1, (Array<E0>*)(&t1_array)->addr_(i) should properly point to the second *E1*, not E0),
    // which means we must shift by _idx * unitSize_ instead of _idx * sizeof(value_type).

    return reinterpret_cast<value_type const*>(ContainerBase::array_ + _idx * ContainerBase::unitSize_);
  }

  /*private*/
  template<class E>
  template<class T/* = E*/>
  typename std::enable_if<std::is_constructible<T>::value>::type
  Array<E>::allocate_(UInt_t _nmax)
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
  Array<E>::allocate_(UInt_t)
  {
    throw std::runtime_error(("Cannot create an Array of pure-virtual elements (" + ContainerBase::name_ + ")").Data());
  }

}

#endif

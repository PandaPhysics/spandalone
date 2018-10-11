#ifndef PandaTree_Framework_RefVector_h
#define PandaTree_Framework_RefVector_h

#include "ContainerBase.h"
#include "Ref.h"

#include <stdexcept>
#include <vector>

namespace panda {

  //! A vector of references to elements in a container.
  /*!
   * The vector version of Ref, where the container is shared between the elements (individual refs).
   * A vector of index integers, instead of a single integer, is written to the tree.
   */
  template<class E>
  class RefVector {
  public:
    typedef RefVector<E> self_type;
    typedef E element_type;
    typedef Ref<element_type> ref_type;
    typedef ConstRef<element_type> const_ref_type;
    typedef std::vector<Short_t> Indices;

    //! Default constructor.
    RefVector() {}
    //! Standard constructor.
    /*!
     * The container must be a pointer to a derived class of Array<E> or Collection<E>. Proper assignment
     * is only done at runtime and thus there no protection against assigning a wrong type of container.
     */
    RefVector(ContainerBase const*& c, Indices& indices) : container_(&c), indices_(&indices) {}
    //! Copy constructor.
    RefVector(self_type const& orig) : container_(orig.container_), indices_(orig.indices_) {}
    //! Set the index.
    void setIndices(Indices& indices) { indices_ = &indices; }
    //! Set the container (an Array or a Collection).
    /*!
     * The container must be a derived class of Array<E> or Collection<E>. There is no protection against
     * assigning a wrong type of container.
     */
    void setContainer(ContainerBase const*& c) { container_ = &c; }
    //! Size of the vector.
    UShort_t size() const;
    //! Element (ref) accessor.
    ref_type at(UShort_t);
    //! Element (ref) accessor.
    const_ref_type at(UShort_t) const;
    //! Object accessor.
    element_type const& objAt(UShort_t) const;
    //! Assignment operator
    self_type& operator=(self_type const&);
    //! Setter function
    /*!
     * Pass a element_type object after the container is set.
     * If the object is found in the collector, pushes the index value.
     */
    void addRef(element_type const*);
    //! Setter function
    /*!
     * Pushes back a ref if the container is common.
     */
    void push_back(ref_type const&);
    //! Clear operation.
    void clear() { if (indices_) indices_->clear(); }
    //! Initializer
    /*!
     * Invalidates the index by setting it to 0xffffffff.
     */
    void init() { clear(); }
    //! Validity check. Both container and idx must be valid, and idx must not be 0xffffffff.
    Bool_t isValid() const { return container_ && *container_ && indices_; }
    //! Accessor to indices
    /*!
     * Throws a runtime_error if indices is NULL.
     */
    Indices*& indices();
    //! Accessor to container
    /*!
     * Throws a runtime_error if container is not valid.
     */
    ContainerBase const* container() const;
    //! Sort the ref vector
    /*!
     * The internal indices are re-ordered, but the underlying data is untouched
     */
    void sort(ContainerBase::Comparison const&);

    //! "pointer" wrapper for Ref
    template<Bool_t is_const>
    class RefHolder {
    public:
      typedef typename std::conditional<is_const, Short_t const, Short_t>::type index_type;
      typedef typename std::conditional<is_const, const_ref_type, ref_type>::type value_type;

      RefHolder(ContainerBase const*& c, index_type& idx) : ref_(c, idx) {}
      value_type const& operator*() const { return ref_; }
      value_type const* operator->() const { return &ref_; }
    private:
      value_type ref_;
    };

    //! const iterator
    template<Bool_t is_const>
    class RefVectorIterator {
      friend class RefVector<element_type>;
    public:
      typedef RefVector<element_type> refvector_type;
      typedef RefVectorIterator<is_const> self_type;
      typedef typename std::conditional<is_const, const_ref_type, ref_type>::type value_type;
      typedef refvector_type::RefHolder<is_const> ptr_type;
      typedef typename std::conditional<is_const, Indices::const_iterator, Indices::iterator>::type internal_type;
      typedef int difference_type;

      RefVectorIterator() {}
      RefVectorIterator(self_type const& it) : container_(it.container_), itr_(it.itr_) {}

      bool operator==(self_type const& rhs) const { return itr_ == rhs.itr_; }
      bool operator!=(self_type const& rhs) const { return itr_ != rhs.itr_; }

      value_type operator*() const { return value_type(*container_, *itr_); }
      ptr_type operator->() const { return ptr_type(*container_, *itr_); }

      self_type& operator++() { ++itr_; return *this; }
      self_type operator++(int) { auto copy(*this); ++itr_; return copy; }
      self_type& operator--() { --itr_; return *this; }
      self_type operator--(int) { auto copy(*this); --itr_; return copy; }

      self_type& operator+=(int n) { itr_ += n; return *this; }
      self_type& operator-=(int n) { itr_ -= n; return *this; }

      self_type operator+(int n) const { auto copy(*this); return (copy += n); }
      self_type operator-(int n) const { auto copy(*this); return (copy -= n); }

      int operator-(self_type const& rhs) const { 
        return this->operator*().operator->() - (*rhs).operator->(); 
      }

      self_type& operator[](int n) { auto copy(*this); return (copy += n); }
      bool operator<(self_type const& rhs) const { return this->operator-(rhs) < 0; }
      bool operator>(self_type const& rhs) const { return this->operator-(rhs) > 0; }
      bool operator<=(self_type const& rhs) const { return !(this->operator>(rhs)); }
      bool operator>=(self_type const& rhs) const { return !(this->operator<(rhs)); }

    private:
      RefVectorIterator(ContainerBase const*& v, internal_type const& i) : container_(&v), itr_(i) {}

      ContainerBase const** container_{0};
      internal_type itr_{};
    };

    typedef RefVectorIterator<kTRUE> const_iterator;
    typedef RefVectorIterator<kFALSE> iterator;

    const_iterator begin() const { return const_iterator(*container_, indices_->begin()); }
    iterator begin() { return iterator(*container_, indices_->begin()); }
    const_iterator end() const { return const_iterator(*container_, indices_->end()); }
    iterator end() { return iterator(*container_, indices_->end()); }

  private:
    ContainerBase const** container_{0};
    Indices* indices_{0};
  };

  template<class E>
  void
  RefVector<E>::sort(ContainerBase::Comparison const& _c)
  {
    // define an index comparison using the element comparison
    auto indexComp([this, &_c](short i1, short i2)->Bool_t {
        return _c(static_cast<Element const&>((*container_)->elemAt(i1)),
                  static_cast<Element const&>((*container_)->elemAt(i2)));
      });

    std::sort(indices_->begin(), indices_->end(), indexComp);
  }

  template<class E>
  UShort_t
  RefVector<E>::size() const
  {
    if (!indices_)
      return 0;
    return indices_->size();
  }

  template<class E>
  Ref<E>
  RefVector<E>::at(UShort_t _i)
  {
    if (!container_ || !indices_)
      throw std::runtime_error("at() called on an invalid RefVector");

    return Ref<element_type>(*container_, indices_->at(_i));
  }

  template<class E>
  ConstRef<E>
  RefVector<E>::at(UShort_t _i) const
  {
    if (!container_ || !indices_)
      throw std::runtime_error("at() called on an invalid RefVector");

    return ConstRef<element_type>(*container_, indices_->at(_i));
  }

  template<class E>
  E const&
  RefVector<E>::objAt(UShort_t _i) const
  {
    if (!container_ || !(*container_) || !indices_)
      throw std::runtime_error("at() called on an invalid RefVector");

    return static_cast<element_type const&>((*container_)->elemAt(indices_->at(_i)));
  }

  template<class E>
  RefVector<E>&
  RefVector<E>::operator=(self_type const& _rhs)
  {
    container_ = _rhs.container_;
    if (indices_) {
      if (_rhs.indices_)
        indices_->assign(_rhs.indices_->begin(), _rhs.indices_->end());
      else
        indices_ = 0;
    }
    else if (_rhs.indices_)
      throw std::runtime_error("Cannot copy a valid RefVector to an invalid RefVector");

    return *this;
  }

  template<class E>
  void
  RefVector<E>::addRef(element_type const* _obj)
  {
    if (!_obj)
      return;

    if (!container_ || !(*container_) || !indices_)
      throw std::runtime_error("Cannot push to an invalid RefVector");

    for (UShort_t idx(0); idx != (*container_)->size(); ++idx) {
      if (&(*container_)->elemAt(idx) == _obj) {
        indices_->push_back(Short_t(idx));
        return;
      }
    }

    throw std::runtime_error("Pushing an object not in referenced collection");
  }
  
  template<class E>
  void
  RefVector<E>::push_back(Ref<element_type> const& _ref)
  {
    if (!container_ || !indices_)
      throw std::runtime_error("Cannot push to an invalid RefVector");

    if (_ref.container() != *container_)
      throw std::runtime_error("Pushing a ref of a wrong object");

    indices_->push_back(_ref.idx());
  }

  template<class E>
  std::vector<Short_t>*&
  RefVector<E>::indices()
  {
    if (!indices_)
      throw std::runtime_error("Invalid indices ref");

    return indices_;
  }

  template<class E>
  ContainerBase const*
  RefVector<E>::container() const
  {
    if (!container_)
      throw std::runtime_error("Invalid container ref");

    return *container_;
  }

}

template<class E>
std::ostream& operator<<(std::ostream& _out, panda::RefVector<E> const& _ref)
{
  _out << "RefVector<" << E::typeName() << ">";
  if (_ref.isValid())
    _out << " " << _ref.container()->getName();
  else
    _out << " <null>";

  return _out;
}

#endif

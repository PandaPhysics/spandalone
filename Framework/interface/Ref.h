#ifndef PandaTree_Framework_Ref_h
#define PandaTree_Framework_Ref_h

#include "ContainerBase.h"

#include <stdexcept>

namespace panda {

  //! A reference to an element in a container.
  /*!
   * A Ref consists of a pointer reference to a collection and an integer that acts as the index to the element referred to.
   * The references are supplied by the holder of the Ref during construction. At read time, the holder of the
   * Ref is responsible for instructing the Ref to point to a specific index in the given collection. At write time,
   * the Ref can be passed a pointer to the element to point to, using the operator=. The Ref object will find the
   * proper index internally.
   * The class has pointer-like operators (-> and *) to make the implementation details transparent to users.
   */
  template<class E>
  class Ref {
  public:
    typedef Ref<E> self_type;
    typedef E value_type;
    typedef Short_t index_type;

    //! Default constructor.
    Ref() {}
    //! Standard constructor.
    /*!
     * The container must be a derived class of Array<E> or Collection<E>. There is no protection against
     * assigning a wrong type of container.
     */
    Ref(ContainerBase const*& c, index_type& idx) : container_(&c), idx_(&idx) {}
    //! Copy constructor.
    Ref(self_type const& orig) : container_(orig.container_), idx_(orig.idx_) {}
    //! Set the index.
    void setIndex(index_type& idx) { idx_ = &idx; }
    //! Set the container.
    /*!
     * The container must be a derived class of Array<E> or Collection<E>. There is no protection against
     * assigning a wrong type of container.
     */
    void setContainer(ContainerBase const*& c) { container_ = &c; }
    //! The arrow operator.
    /*!
     * Returns a null pointer if the container is not set or the index points to an invalid location.
     */
    value_type const* operator->() const
    {
      if (!container_ || !(*container_) || !idx_ || (*idx_) < 0)
        return 0;
  
      return &static_cast<E const&>((*container_)->elemAt(*idx_));
    }
    //! The dereference operator.
    /*!
     * Throws an invalid_argument exception if the reference is invalid.
     */
    value_type const& operator*() const
    {
      if (!container_ || !(*container_) || !idx_ || (*idx_) < 0)
        throw std::runtime_error("Dereferencing an invalid Ref");
  
      return static_cast<E const&>((*container_)->elemAt(*idx_));
    }
    //! Copy assignment
    /*!
     * idx is copied by value. Change in the right-hand-side index value after the assignment
     * is not reflected to the left-hand-side index.
     */
    self_type& operator=(self_type const& rhs)
    {
      container_ = rhs.container_;
      if (idx_) {
        if (rhs.idx_)
          (*idx_) = (*rhs.idx_);
        else
          (*idx_) = -1;
      }
      return *this;
    }
    //! Setter function
    /*!
     * Pass a pointer to a value_type object on the right hand side after the container is set.
     * If the object is found in the collector, sets the index value.
     * Returns true on successful set.
     */
    Bool_t setRef(value_type const* rhs)
    {
      if (!rhs) {
        container_ = 0;
        idx_ = 0;
      }
  
      if (!container_ || !(*container_) || !idx_)
        return false;
  
      // loop through the container and return when a match is found
      for ((*idx_) = 0; (*idx_) != Short_t((*container_)->size()); ++(*idx_)) {
        if (&(*container_)->elemAt(*idx_) == rhs)
          return true;
      }
  
      // otherwise set to -1 and return false
  
      (*idx_) = -1;
      return true;
    }
    //! Validity check. Both container and idx must be valid, and idx must not be 0xffffffff.
    Bool_t isValid() const { return container_ && *container_ && idx_ && unsigned(*idx_) < (*container_)->size(); }
    //! Initializer
    /*!
     * Invalidates the index by setting it to 0xffffffff.
     */
    void init() { if (idx_) (*idx_) = -1; }
    //! Accessor to idx
    /*!
     * Throws a runtime_error if idx is not valid.
     */
    index_type& idx()
    {
      if (!idx_)
        throw std::runtime_error("Invalid index ref");
  
      return *idx_;
    }
    //! Accessor to idx
    /*!
     * Throws a runtime_error if idx is not valid.
     */
    index_type idx() const
    {
      if (!idx_)
        throw std::runtime_error("Invalid index ref");
  
      return *idx_;
    }
    //! Accessor to container
    /*!
     * Throws a runtime_error if container is not valid.
     */
    ContainerBase const* container() const
    {
      if (!container_)
        throw std::runtime_error("Invalid container ref");
  
      return *container_;
    }
    //! Pointer to object
    value_type const* get() const { return operator->(); }

  private:
    ContainerBase const** container_{0};
    index_type* idx_{0};
  };


  //! A const version of Ref.
  /*!
   * Generating Ref and ConstRef from a single template has been attempted, but it was impossible to delete the assignment operator from
   * ConstRef while enabling it in Ref (using e.g. enable_if) because the default assignment operator will take precedence.
   */
  template<class E>
  class ConstRef {
  public:
    typedef ConstRef<E> self_type;
    typedef E value_type;
    typedef Short_t index_type;

    //! Default constructor.
    ConstRef() {}
    //! Standard constructor.
    /*!
     * The container must be a derived class of Array<E> or Collection<E>. There is no protection against
     * assigning a wrong type of container.
     */
    ConstRef(ContainerBase const*& c, index_type const& idx) : container_(&c), idx_(&idx) {}
    //! Copy constructor.
    ConstRef(self_type const& orig) : container_(orig.container_), idx_(orig.idx_) {}

    self_type& operator=(self_type const&) = delete;

    //! Set the index.
    void setIndex(index_type const& idx) { idx_ = &idx; }
    //! Set the container.
    /*!
     * The container must be a derived class of Array<E> or Collection<E>. There is no protection against
     * assigning a wrong type of container.
     */
    void setContainer(ContainerBase const*& c) { container_ = &c; }
    //! The arrow operator.
    /*!
     * Returns a null pointer if the container is not set or the index points to an invalid location.
     */
    value_type const* operator->() const
    {
      if (!container_ || !(*container_) || !idx_ || (*idx_) < 0)
        return 0;
  
      return &static_cast<E const&>((*container_)->elemAt(*idx_));
    }
    //! The dereference operator.
    /*!
     * Throws an invalid_argument exception if the reference is invalid.
     */
    value_type const& operator*() const
    {
      if (!container_ || !(*container_) || !idx_ || (*idx_) < 0)
        throw std::runtime_error("Dereferencing an invalid ConstRef");
  
      return static_cast<E const&>((*container_)->elemAt(*idx_));
    }
    //! Validity check. Both container and idx must be valid, and idx must not be 0xffffffff.
    Bool_t isValid() const { return container_ && *container_ && idx_ && unsigned(*idx_) < (*container_)->size(); }
    //! Accessor to idx
    /*!
     * Throws a runtime_error if idx is not valid.
     */
    index_type const& idx() const
    {
      if (!idx_)
        throw std::runtime_error("Invalid index ref");
  
      return *idx_;
    }
    //! Accessor to container
    /*!
     * Throws a runtime_error if container is not valid.
     */
    ContainerBase const* container() const
    {
      if (!container_)
        throw std::runtime_error("Invalid container ref");
  
      return *container_;
    }
    //! Pointer to object
    value_type const* get() const { return operator->(); }

  private:
    ContainerBase const** container_{0};
    index_type const* idx_{0};
  };

}

template<class E>
std::ostream& operator<<(std::ostream& _out, panda::Ref<E> const& _ref)
{
  _out << "Ref<" << E::typeName() << ">";
  if (_ref.isValid())
    _out << " " << _ref.container()->getName() << "(" << _ref.idx() << ")";
  else
    _out << " (null)";

  return _out;
}

template<class E>
std::ostream& operator<<(std::ostream& _out, panda::ConstRef<E> const& _ref)
{
  _out << "Ref<" << E::typeName() << ">";
  if (_ref.isValid())
    _out << " " << _ref.container()->getName() << "(" << _ref.idx() << ")";
  else
    _out << " (null)";

  return _out;
}

#endif

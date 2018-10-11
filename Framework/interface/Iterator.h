#ifndef PandaTree_Framework_Iterator_h
#define PandaTree_Framework_Iterator_h

#include <type_traits>

namespace panda {
  namespace utils {

    //! Iterator class for containers.
    template<class C, bool is_const>
    class Iterator {
      typedef C container_type;
      typedef Iterator<container_type, false> mutable_type;
      typedef Iterator<container_type, true> const_type;

      friend container_type;
      friend const_type;

    public:
      typedef int difference_type;
      typedef Iterator<C, is_const> self_type;
      typedef typename C::value_type value_type;
      typedef typename std::conditional<is_const, typename C::const_reference, typename C::reference>::type reference;
      typedef typename std::conditional<is_const, typename C::const_pointer, typename C::pointer>::type pointer;
      typedef std::random_access_iterator_tag iterator_category;

      Iterator() {}
      Iterator(mutable_type const& it) : unitSize_(it.unitSize_) { ptr_.obj = it.ptr_.obj; }

      // implicitly assumes unitSize_ is the same
      self_type& operator=(const_type const& rhs) { ptr_.obj = rhs.ptr_.obj; return *this; } 
      self_type& operator=(mutable_type const& rhs) { ptr_.obj = rhs.ptr_.obj; return *this; }

      bool operator==(mutable_type const& rhs) const { return ptr_.obj == rhs.ptr_.obj; }
      bool operator==(const_type const& rhs) const { return ptr_.obj == rhs.ptr_.obj; }
      bool operator!=(mutable_type const& rhs) const { return ptr_.obj != rhs.ptr_.obj; }
      bool operator!=(const_type const& rhs) const { return ptr_.obj != rhs.ptr_.obj; }

      self_type& operator++() { shiftPtr_(1); return *this; }
      self_type operator++(int) { auto itr(*this); shiftPtr_(1); return itr; }
      self_type& operator--() { shiftPtr_(-1); return *this; }
      self_type operator--(int) { auto itr(*this); shiftPtr_(-1); return itr; }

      self_type& operator+=(int n) { shiftPtr_(n); return *this; }
      self_type& operator-=(int n) { shiftPtr_(-n); return *this; }

      self_type operator+(int n) const { auto itr(*this); return (itr += n); }
      self_type operator-(int n) const { auto itr(*this); return (itr -= n); }

      reference operator*() const { return *ptr_.obj; }
      pointer operator->() const { return ptr_.obj; }

      int operator-(self_type const& rhs) const { return ptr_.obj - rhs.operator->(); }
      reference operator[](int n) const { auto itr(*this); return *(itr += n); }

      bool operator<(self_type const& rhs) const { return this->operator-(rhs) < 0; }
      bool operator>(self_type const& rhs) const { return this->operator-(rhs) > 0; }
      bool operator<=(self_type const& rhs) const { return !(this->operator>(rhs)); }
      bool operator>=(self_type const& rhs) const { return !(this->operator<(rhs)); }

    private:
      Iterator(pointer p, unsigned unitSize) : unitSize_(unitSize) { ptr_.obj = p; }

      void shiftPtr_(int shift) { ptr_.ch += unitSize_ * shift; }

      typedef typename std::conditional<is_const, char const*, char*>::type char_pointer;

      union Ptr {
        pointer obj;
        char_pointer ch;
      } ptr_;
      unsigned const unitSize_{0};
    };

  }
}

#endif

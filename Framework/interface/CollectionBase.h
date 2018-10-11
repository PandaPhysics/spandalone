#ifndef PandaTree_Framework_CollectionBase_h
#define PandaTree_Framework_CollectionBase_h

#include "ContainerBase.h"

#include <vector>
#include <utility>

namespace panda {

  //! Base class for dynamic-size containers.
  /*!
   * Panda containers can be fixed-size (Array) or dynamic-size (Collection). Dynaic-size containers
   * are similar to std::vectors.
   */
  class CollectionBase : public ContainerBase {
  public:
    CollectionBase() = delete;
    CollectionBase(CollectionBase const& src) = delete;
    ~CollectionBase();

    void setStatus(TTree&, utils::BranchList const& blist) final;
    utils::BranchList getStatus(TTree&) const final;
    utils::BranchList getBranchNames(Bool_t fullName = kTRUE, Bool_t = kFALSE) const final;
    UInt_t setAddress(TTree&, utils::BranchList const& blist = {"*"}, Bool_t setStatus = kTRUE) final;
    void book(TTree&, utils::BranchList const& blist = {"*"}) final;
    using ReaderObject::getEntry;
    Int_t getEntry(UInt_t, Long64_t entry, Bool_t localEntry = kFALSE) final;
    Int_t fill(TTree&) final;
    void init() final { clear(); }
    void print(std::ostream& = std::cout, UInt_t level = 1) const override;
    void dump(std::ostream& = std::cout) const override;
    void unlink(TTree&) final;
    UInt_t size() const final { return size_; }

    Bool_t empty() const { return size_ == 0; }

    //! Resize the container.
    /*!
     * If expanding, init() is invoked on all new elements.
     * If the new size is greater than nmax, data will be reallocated, making all references invalid.
     * 
     * \param size   New collection size.
     */
    virtual void resize(UInt_t size);

    //! Pre-allocate space for the container.
    /*!
     * If the new size is greater than nmax, data will be reallocated, making all references invalid.
     * 
     * \param size   New collection allocation size.
     */
    virtual void reserve(UInt_t size);

    //! Clear the container (set size = 0)
    virtual void clear() { size_ = 0; getData().resizeVectors_(0); }

    //! Check for address change before fill.
    virtual void prepareFill(TTree&);

  protected:
    CollectionBase(char const* name, UInt_t unitSize, Bool_t dummy) : ContainerBase(name, unitSize) {}

    virtual void reallocate_(UInt_t) = 0;

  private:
    void doSetAddress_(TTree&, utils::BranchList const&, Bool_t setStatus, Bool_t asInput);

    //! Collection size
    UInt_t size_{0};

    //! Size information lookahead
    /*!
     * To read a collection of N objects, we must have more than N slots allocated in memory.
     * Since ROOT cannot automatically take care of resizing the array, we need to load the "size"
     * branch first and reallocate memory if necessary.
     */
    UInt_t sizeIn_{0};

    //! Flags indicating branch re-linking when container is reallocated. Synchronized with ReaderObject::inputBranches_ vector.
    std::vector<Bool_t> inputInSynch_{};

    //! List of outputs
    /*!
     * When resize & reallocation happens, we need to update the addresses at the output trees too.
     * Using a vector of pairs instead of a map for the same reason as described in ReaderObject.
     */
    std::vector<std::pair<TTree*, Bool_t>> outputs_{};
  };

}

#endif

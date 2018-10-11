#include "../interface/ContainerBase.h"

#include <algorithm>
#include <stdexcept>

void
panda::ContainerBase::print(std::ostream& _out/* = std::cout*/, UInt_t _level/* = 1*/) const
{
  if (name_.Length() != 0)
    _out << "{" << name_ << "}" << std::endl;

  for (unsigned iE(0); iE != size(); ++iE) {
    _out << "[" << iE << "]";
    if (_level > 1)
      _out << std::endl;
    else
      _out << " ";
    elemAt(iE).print(_out, _level);
  }
}

void
panda::ContainerBase::dump(std::ostream& _out/* = std::cout*/) const
{
  _out << "name_ = " << name_ << std::endl;
  _out << "unitSize_ = " << unitSize_ << std::endl;
  _out << "array_ (" << (void*)(array_) << "): " << std::endl;
  for (unsigned iE(0); iE != size(); ++iE) {
    _out << "[" << iE << "]" << std::endl;
    elemAt(iE).dump(_out);
  }
  _out << std::endl;
}

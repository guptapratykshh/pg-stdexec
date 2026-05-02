#pragma once

#include "../../relacy/relacy_std.hpp"

#include <iosfwd>
#include <thread>

namespace std
{
  template <class>
  struct atomic_ref;

  inline std::ostream& operator<<(std::ostream& os, std::thread::id const & id)
  {
    return os << "thread::id{" << id.id_ << "}";
  }
}  // namespace std

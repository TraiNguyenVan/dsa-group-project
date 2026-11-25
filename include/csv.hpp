#ifndef PHONEBOOK_CSV_HPP
#define PHONEBOOK_CSV_HPP

#include <string>

namespace csv {

// Quote fields containing a comma, quote or line break.
std::string escapeField(const std::string& field);

}  // namespace csv

#endif

#include "source.h"

#include <vector>

int main()
{
  jlst::source s();
  std::vector<jlst::source> v;
  v.push_back( jlst::source("coucou.txt" ) );
  return 0;
}

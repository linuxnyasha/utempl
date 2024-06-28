#include <utempl/attributes.hpp>
#include <string>

template <typename T>
struct AttributeData {
  T value;
};


#define MY_ATTRIBUTE(type, ...) GENERIC_ATTRIBUTE(AttributeData<type>{__VA_ARGS__})


ATTRIBUTE_STRUCT(SomeStruct, 
  MY_ATTRIBUTE(int, .value = 2);
  int field;
  SKIP_ATTRIBUTE();
  int field2;
  MY_ATTRIBUTE(std::string, .value = "HEY!")
  std::string field3;
);

static_assert(SomeStruct::GetAttribute<0>().value == 2);
static_assert(SomeStruct::GetAttribute<2>().value == "HEY!");



auto main() -> int {};

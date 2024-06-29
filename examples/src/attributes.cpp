#include <utempl/attributes.hpp>
#include <string>

template <typename T>
struct AttributeData {
  T value;
  constexpr auto operator==(const AttributeData<T>&) const -> bool = default;
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

static_assert(utempl::GetAttributes<SomeStruct>() 
  == utempl::Tuple{
      AttributeData<int>{.value = 2}, 
      utempl::NoInfo{},
      AttributeData<std::string>{.value = "HEY!"}});


struct SomeOtherStruct {
  static_assert(utempl::OpenStruct<SomeOtherStruct>());
  utempl::FieldAttribute<utempl::FieldType<int>, int> field1;
  utempl::FieldAttribute<utempl::FieldType<int>, void> field2;
  static_assert(utempl::CloseStruct());
};

static_assert(utempl::GetAttributes<SomeOtherStruct>() 
  == utempl::Tuple{
      utempl::kTypeList<int>,
      utempl::kTypeList<void>});




auto main() -> int {};

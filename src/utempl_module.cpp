#pragma once

#ifdef UTEMPL_MODULE

export module utempl;
export import utempl.type_list;
export import utempl.tuple;
export import utempl.string;
export import utempl.loopholes;
export import utempl.attributes;
export import utempl.meta_info;
export import utempl.overloaded;
export import utempl.utils;
export import utempl.go_interface;
export import utempl.optional;

#else
#include <utempl/attributes.hpp>
#include <utempl/constexpr_string.hpp>
#include <utempl/go_interface.hpp>
#include <utempl/loopholes/counter.hpp>
#include <utempl/macro.hpp>
#include <utempl/meta_info.hpp>
#include <utempl/optional.hpp>
#include <utempl/overloaded.hpp>
#include <utempl/tuple.hpp>
#include <utempl/type_list.hpp>

#endif

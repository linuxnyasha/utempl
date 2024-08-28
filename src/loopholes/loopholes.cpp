#pragma once
#ifdef UTEMPL_MODULE

export module utempl.loopholes;
export import utempl.loopholes.core;
export import utempl.loopholes.counter;

#else
#include <utempl/loopholes/core.hpp>
#include <utempl/loopholes/counter.hpp>
#endif

#pragma once

#ifdef UTEMPL_MODULE

#define UTEMPL_EXPORT export

#define UTEMPL_EXPORT_BEGIN export {
#define UTEMPL_EXPORT_END }
#define UTEMPL_GLOBAL_MODULE module;
#define UTEMPL_IMPORT(modules, headers) modules

#else

#define UTEMPL_EXPORT
#define UTEMPL_EXPORT_BEGIN
#define UTEMPL_EXPORT_END
#define UTEMPL_GLOBAL_MODULE
#define UTEMPL_IMPORT(modules, headers) headers

#endif

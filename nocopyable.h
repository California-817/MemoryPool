#ifndef __XTEN_NO_COPYABLE_H__
#define __XTEN_NO_COPYABLE_H__
namespace Xten
{
#define NO_COPYABLE_MACRO(CLASS_NAME)                   \
    CLASS_NAME(const CLASS_NAME &) = delete;            \
    CLASS_NAME(CLASS_NAME &&) = delete;                 \
    CLASS_NAME &operator=(const CLASS_NAME &) = delete; \
    CLASS_NAME &operator=(CLASS_NAME &&) = delete
} // namespace Xten

#endif
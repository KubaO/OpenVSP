//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
//
//////////////////////////////////////////////////////////////////////

#ifndef UTIL_VSPOBJ_H
#define UTIL_VSPOBJ_H

template <typename T> static inline T *vGetPtrHelper(T *ptr) { return ptr; }
template <typename Wrapper> static inline typename Wrapper::pointer vGetPtrHelper(const Wrapper &p) { return p.data(); }

/// Use in a class to declare a a private PIMPL class. Works with multiply-inheriting
/// private classes. Use instead of Q_DECLARE_PRIVATE to maintain independence from Qt.
#define VSP_DECLARE_PRIVATE( Class ) \
    friend class Class##Private; \
    Class##Private* d_func(); \
    const Class##Private* d_func() const;

/// Use in the implementation file, in the global scope, to define the implementation's
/// pimpl access methods.
#define VSP_DEFINE_PRIVATE( Class ) \
    inline Class##Private* Class::d_func() { return static_cast<Class##Private *>(vGetPtrHelper(d_ptr)); } \
    inline const Class##Private* Class::d_func() const { return static_cast<const Class##Private *>(vGetPtrHelper(d_ptr)); }

#define VSP_DISABLE_COPY( Class ) \
    Class( const Class & ); \
    Class& operator=( const Class & ); \

#endif // UTIL_VSPOBJ_H

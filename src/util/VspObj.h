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
/// private classes. Use instead of Q_DECLARE_PRIVATE to maintain independence from Qt
/// and to reduce the amount of boilerplate. The PIMPL class is a protected member class.
#define VSP_DECLARE_PRIVATE( Class ) \
    protected: \
        class Private; \
    private: \
        friend class Private; \
        Private* d_func(); \
        const Private* d_func() const;

/// Use in the implementation file, in the global scope, to define the implementation's
/// pimpl access methods.
#define VSP_DEFINE_PRIVATE( Class ) \
    inline Class::Private* Class::d_func() { return static_cast<Class::Private *>(vGetPtrHelper(d_ptr)); } \
    inline const Class::Private* Class::d_func() const { return static_cast<const Class::Private *>(vGetPtrHelper(d_ptr)); }

/// Use in the PIMPL to provide access to the public instance.
#define VSP_DECLARE_PUBLIC(Class)                                    \
    inline Class* q_func() { return static_cast<class Class *>(q_ptr); } \
    inline const Class* q_func() const { return static_cast<const class Class *>(q_ptr); } \
    friend class Class;

/// Use in a public method body to access the PIMPL through d->
#define V_D(Class) Class::Private * const d = d_func()

/// Use in a PIMPL method body to access the public instance through q->
#define V_Q(Class) Class * const q = q_func()

#define VSP_DISABLE_COPY( Class ) \
    Class( const Class & ); \
    Class& operator=( const Class & ); \

#endif // UTIL_VSPOBJ_H

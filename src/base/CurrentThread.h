#ifndef BASE_CURRENTTHREAD_H
#define BASE_CURRENTTHREAD_H

#include <stdint.h>

namespace CurrentThread
{
    extern __thread int t_cachedtid;
    extern __thread const char* t_threadname;

    void cachetid();

    inline int tid()
    {
        if(__builtin_expect(t_cachedtid == 0,0))
        {
            cachetid();
        }
        return t_cachedtid;
    }

    inline const char*name()
    {
        return t_threadname;
    }
};
#endif
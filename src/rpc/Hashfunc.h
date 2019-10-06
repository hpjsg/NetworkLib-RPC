#ifndef RPC_HASHFUNC_H
#define RPC_HASHFUNC_H

#include<openssl/md5.h>
#include"rpc/Consistent_hash.h"

class MD5hashfunc:public Hashfunc
{
    public:
        virtual uint32_t Hash(std::string key)
        {
            const unsigned char*d = reinterpret_cast<const unsigned char*>(key.c_str());
            unsigned long n = key.size();
            unsigned char md[16];
            MD5(d,n,md);
            uint32_t hash = 0;
            for(int i=0;i<4;i++)
            {
                hash  += ((uint32_t)(md[i*4 + 3]&0xFF) << 24)
                    | ((uint32_t)(md[i*4 + 2]&0xFF) << 16)
                    | ((uint32_t)(md[i*4 + 1]&0xFF) <<  8)
                    | ((uint32_t)(md[i*4 + 0]&0xFF));

            }
            return hash;
        }
};

#endif
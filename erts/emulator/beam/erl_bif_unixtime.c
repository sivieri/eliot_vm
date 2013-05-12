#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "sys.h"
#include "global.h"
#include "bif.h"
#include "erl_term.h"
#include "big.h"

BIF_RETTYPE unixtime_clock_0(BIF_ALIST_0)
{
    clock_t res;
    Eterm* hp;

    res = clock();
    hp = HAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
    *hp = make_pos_bignum_header(1);
    BIG_DIGIT(hp, 0) = res;
    
    BIF_RET(make_big(hp));
}

BIF_RETTYPE unixtime_gettimeofday_0(BIF_ALIST_0)
{
    struct timeval res;
    Eterm* hp;
    Eterm* hp2;
    Eterm* hp3;
    
    gettimeofday(&res, NULL);
    hp = HAlloc(BIF_P, 3);
    hp2 = HAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
    *hp2 = make_pos_bignum_header(1);
    BIG_DIGIT(hp2, 0) = res.tv_sec;
    hp3 = HAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
    *hp3 = make_pos_bignum_header(1);
    BIG_DIGIT(hp3, 0) = res.tv_usec;
    
    BIF_RET(TUPLE2(hp, make_big(hp2), make_big(hp3)));
}

BIF_RETTYPE unixtime_clock_gettime_0(BIF_ALIST_0)
{
    struct timespec res;
    Eterm* hp;
    Eterm* hp2;
    Eterm* hp3;
    
#ifdef __mips__
    clock_gettime(CLOCK_MONOTONIC, &res);
#else
    clock_gettime(CLOCK_MONOTONIC_RAW, &res);
#endif
    hp = HAlloc(BIF_P, 3);
    hp2 = HAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
    *hp2 = make_pos_bignum_header(1);
    BIG_DIGIT(hp2, 0) = res.tv_sec;
    hp3 = HAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
    *hp3 = make_pos_bignum_header(1);
    BIG_DIGIT(hp3, 0) = res.tv_nsec;
    
    BIF_RET(TUPLE2(hp, make_big(hp2), make_big(hp3)));
}

BIF_RETTYPE unixtime_times_0(BIF_ALIST_0)
{
    struct tms res;
    Eterm* hp;
    Eterm* hp2;
    Eterm* hp3;
    
    times(&res);
    hp = HAlloc(BIF_P, 3);
    hp2 = HAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
    *hp2 = make_pos_bignum_header(1);
    BIG_DIGIT(hp2, 0) = res.tms_utime;
    hp3 = HAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
    *hp3 = make_pos_bignum_header(1);
    BIG_DIGIT(hp3, 0) = res.tms_stime;
    
    BIF_RET(TUPLE2(hp, make_big(hp2), make_big(hp3)));
}

BIF_RETTYPE unixtime_getrusage_0(BIF_ALIST_0)
{
    struct rusage usg;
    Eterm* hp;
    Eterm* hp2;
    Eterm* hp3;
    uint64_t t1, t2;
    
    getrusage(RUSAGE_SELF, &usg);
    t1 = usg.ru_utime.tv_sec * 1000000 + usg.ru_utime.tv_usec;
    t2 = usg.ru_stime.tv_sec * 1000000 + usg.ru_stime.tv_usec;
    hp = HAlloc(BIF_P, 3);
    hp2 = HAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
    *hp2 = make_pos_bignum_header(1);
    BIG_DIGIT(hp2, 0) = t1;
    hp3 = HAlloc(BIF_P, BIG_UINT_HEAP_SIZE);
    *hp3 = make_pos_bignum_header(1);
    BIG_DIGIT(hp3, 0) = t2;
    
    BIF_RET(TUPLE2(hp, make_big(hp2), make_big(hp3)));
}

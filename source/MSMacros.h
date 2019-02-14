//
//  MSMacros.h
//  ios_example
//
//  Created by 胡校明 on 2019/1/16.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSMacros_h
#define MSMacros_h

#ifdef __APPLE__

#define MSNullable _Nullable
#define MSNonnull _Nonnull

#elif

#define MSNullable
#define MSNonnull

#endif

#define intervale(rate) microseconds(1000000LL/rate)

#if DEBUG

#define ErrorLocationLog(reson) \
printf("\n"\
"-----------------------------ERROR-----------------------------\n"\
"| filepath:    %s\n"\
"| linenumber:  %d\n"\
"| funcname:    %s\n"\
"| reson:       %s\n"\
"---------------------------------------------------------------\n",__FILE__,__LINE__,__func__,reson)

#else

#define ErrorLocationLog(reason)

#endif

#endif /* MSMacros_h */

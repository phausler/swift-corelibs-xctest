// This source file is part of the Swift.org open source project
//
// Copyright (c) 2016 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See http://swift.org/LICENSE.txt for license information
// See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//
//  SwiftXCTest-Bridging-Header.h
//  External C API hooks to provide functionality not available directly in Swift
//  for validating behavior like assertions etc.
//

#ifndef SwiftXCTest_Bridging_Header_h
#define SwiftXCTest_Bridging_Header_h

#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>

_Pragma("clang assume_nonnull begin")

__thread int __caught_signal = 0;
__thread sigjmp_buf __signal_mark = { 0 };

static void __handle_ill(int sig) {
    __caught_signal = sig;
    siglongjmp(__signal_mark, -1);
}

static _Bool __try(int sig, void (^ __attribute__((noescape)) work)(), void (^ __attribute__((noescape)) _Nullable handler)(int)) {
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, sig);
    sigprocmask(SIG_SETMASK, &sigset, NULL);
    switch (sigsetjmp(__signal_mark, 1)) {
        case 0: {
            struct sigaction sigact;
            
            sigemptyset(&sigact.sa_mask);
            sigact.sa_flags = 0;
            sigact.sa_handler = __handle_ill;
            sigaction(sig, &sigact, NULL);
            
            sigdelset(&sigset, sig);
            sigprocmask(SIG_SETMASK, &sigset, NULL);
            
            __caught_signal = 0;
            
            work();
            
            sigprocmask(SIG_SETMASK, NULL, &sigset);
            
            return 1;
        }
        case -1:
            sigprocmask(SIG_SETMASK, NULL, &sigset);
            
            if (sigismember(&sigset, sig)) {
                if (handler) handler(sig);
            }
            return 0;
        default:
            return 0;
    }
}

_Pragma("clang assume_nonnull end")


#endif /* SwiftXCTest_Bridging_Header_h */

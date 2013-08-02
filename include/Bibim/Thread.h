﻿#pragma once
#ifndef __BIBIM_THREAD_H__
#define __BIBIM_THREAD_H__

#   include <Bibim/Foundation.h>

#   if (defined(BIBIM_PLATFORM_WINDOWS))
#       include <Bibim/Thread.Windows.h>
#   elif (defined(BIBIM_PLATFORM_UNIX))
#       include <Bibim/Thread.Unix.h>
#   endif

#endif
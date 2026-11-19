#pragma once
#ifndef __BIBIM_IME_SDL3_H__
#define __BIBIM_IME_SDL3_H__

#include <Bibim/FWD.h>
#if (defined(BIBIM_USE_SDL3))

#include <Bibim/IMEBase.h>

namespace Bibim
{
    class IME : public IMEBase
    {
        BBModuleClass(IME, IMEBase, 'I', 'M', 'E', '_');
        public:
            IME();
            IME(Window* window);
            virtual ~IME();

        private:
            virtual void OnRequest(const Request& request);
    };
}

#endif
#endif

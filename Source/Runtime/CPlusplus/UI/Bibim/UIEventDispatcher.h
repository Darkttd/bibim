#pragma once
#ifndef __BIBIM_UIEVENTDISPATCHER_H__
#define __BIBIM_UIEVENTDISPATCHER_H__

#   include <Bibim/FWD.h>
#   include <Bibim/GameComponent.h>

    namespace Bibim
    {
        class UIEventDispatcher : public GameComponent
        {
            BBClassFOURCC('U', 'I', 'E', 'D');
            public:
                UIEventDispatcher();
                virtual ~UIEventDispatcher();

            private:
        };
    }

#   include <Bibim/UIEventDispatcher.inl>

#endif
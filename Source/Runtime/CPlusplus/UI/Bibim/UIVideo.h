#pragma once
#ifndef __BIBIM_UIVIDEO_H__
#define __BIBIM_UIVIDEO_H__

#   include <Bibim/FWD.h>
#   include <Bibim/UIVisual.h>

    namespace Bibim
    {
        class UIVideo : public UIVisual
        {
            BBReadableObjectClass(UIVideo, UIVisual, UIElement, 'U', 'V', 'D', 'O');
            public:
                UIVideo();
                virtual ~UIVideo();

            private:
        };
    }

#   include <Bibim/UIVideo.inl>

#endif
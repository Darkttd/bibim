#pragma once
#ifndef __BIBIM_DISPLAYSWAPCHAIN_H__
#define __BIBIM_DISPLAYSWAPCHAIN_H__

#   include <Bibim/FWD.h>
#   include <Bibim/GameModule.h>
#   include <Bibim/Point.h>
#   include <Bibim/Rect.h>
#   include <d3d9.h>

    namespace Bibim
    {
        class DisplaySwapChain : public GameModule
        {
            BBObjectClass(DisplaySwapChain, GameModule, 'S', 'W', 'C', 'H');
            public:
                DisplaySwapChain(GraphicsDevice* graphicsDevice, Window* window);
                DisplaySwapChain(GraphicsDevice* graphicsDevice, Window* window, Point size);
                virtual ~DisplaySwapChain();

                void BeginDraw();
                void EndDraw();
                void Present();

                inline GraphicsDevice* GetGraphicsDevice() const;
                inline Window* GetWindow() const;

                inline Point GetBackbufferSize() const;

                inline const Rect& GetViewport() const;
                inline void SetViewport(const Rect& value);

            protected:
                DisplaySwapChain(GraphicsDevice* graphicsDevice, Window* window, void* swapChainInterface);

                void Construct(Window* window, Point size);

            private:
                GraphicsDevice* graphicsDevice;
                Window* window;
                Point backbufferSize;
                Rect viewport;

                IDirect3DSwapChain9* swapChain;
                IDirect3DSurface9* backBuffer;

                friend class GraphicsDevice;
        };
    }

#   include <Bibim/DisplaySwapChain.inl>

#endif
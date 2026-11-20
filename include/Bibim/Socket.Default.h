#pragma once
#ifndef __BIBIM_SOCKET_DEFAULT_H__
#define __BIBIM_SOCKET_DEFAULT_H__

#include <Bibim/FWD.h>
#include <Bibim/SharedObject.h>
#include <Bibim/IPEndPoint.h>
#include <cstdint>

namespace Bibim
{
    class Socket : public SharedObject
    {
        public:
            enum Status
            {
                Disconnected,
                Connected,
            };
            
        public:
            explicit Socket(IPEndPoint endPoint);
            ~Socket();

            bool Connect();
            void Disconnect();

            bool TryConnect();

            int Send(const void* buffer, int size);
            int Receive(void* buffer, int size);

            inline bool IsConnected() const;
            bool CanSend() const;
            bool CanReceive() const;

        private:
            bool Preconnect();
            bool ConnectActually();
            bool SetNonBlockingIOMode(bool value);

        private:
#           if (defined(BIBIM_PLATFORM_WINDOWS))
                typedef std::uintptr_t HANDLE; // SOCKET is UINT_PTR
#           else
                typedef int HANDLE;
                static const int INVALID_SOCKET = -1;
#           endif

            HANDLE handle;
            IPEndPoint endPoint;
            Status status;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool Socket::IsConnected() const
    {
        return status == Socket::Connected;
    }
}

#endif

using System;
using System.Collections.Concurrent;

namespace Bibim.Bab.Consoles
{
    /// <summary>
    /// 실행 중인 콘솔 프로그램에 사용자가 입력한 명령을 전달하는 큐입니다.
    /// WPF ConsoleWindow와 CLI 호스트(Bibim.Baaaaaab.Cli)가 함께 사용하므로 UI에 의존하지 않습니다.
    /// </summary>
    public static class ConsoleCommandQueue
    {
        public static readonly ConcurrentQueue<string> Commands = new ConcurrentQueue<string>();
    }
}

using System;

namespace Louron
{
    public class Debug
    {
        public Debug() {
            Log("Hello From C#!");
        }

        public enum LogType
        {
            Info,
            Warning,
            Error,
            Critical
        }

        public static void Log(String message, LogType type = LogType.Info) {

            EngineCallbacks.Debug_LogMessage(type, message);
        }

        public static void OnStart()
        {
            Console.WriteLine("OnStart Called");
        }

    }
}

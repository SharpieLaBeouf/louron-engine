using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Louron
{
    public class Input
    {

        public static bool GetKey(KeyCode keyCode)
        {
            return EngineCallbacks.Input_GetKey(keyCode);
        }

        public static bool GetKeyDown(KeyCode keyCode)
        {
            return EngineCallbacks.Input_GetKeyDown(keyCode);
        }

        public static bool GetKeyUp(KeyCode keyCode)
        {
            return EngineCallbacks.Input_GetKeyUp(keyCode);
        }

        public static bool GetMouseButton(MouseButton buttonCode)
        {
            return EngineCallbacks.Input_GetMouseButton(buttonCode);
        }

        public static bool GetMouseButtonDown(MouseButton buttonCode)
        {
            return EngineCallbacks.Input_GetMouseButtonDown(buttonCode);
        }

        public static bool GetMouseButtonUp(MouseButton buttonCode)
        {
            return EngineCallbacks.Input_GetMouseButtonUp(buttonCode);
        }

        public static float GetMouseX()
        {
            return GetMousePosition().X;
        }

        public static float GetMouseY()
        {
            return GetMousePosition().X;
        }

        public static Vector2 GetMousePosition() {
            EngineCallbacks.Input_GetMousePosition(out Vector2 result);
            return result;
        }


    }
}

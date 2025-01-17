using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Louron
{

    public struct Time
    {

        public static float deltaTime
        {
            get
            {
                EngineCallbacks.Time_GetDeltaTime(out float result);
                return result;
            }
        }

        public static float currentTime
        {
            get
            {
                EngineCallbacks.Time_GetCurrentTime(out float result);
                return result;
            }
        }

    }
}

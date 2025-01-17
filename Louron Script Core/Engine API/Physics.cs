using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Louron
{
    public enum ColliderType : byte
    {
        None = 0,
        Box_Collider = 1,
        Sphere_Collider = 2,
        Capsule_Collider = 3,
        Mesh_Collider = 4
    }

    public struct Collider
    {
        public ColliderType type;
        private readonly uint entity_uuid;

        public Entity entity
        {
            get
            {
                return new Entity(entity_uuid);
            }
        }

    }
}

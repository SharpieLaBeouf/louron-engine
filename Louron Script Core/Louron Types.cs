using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using static Louron.Entity;

namespace Louron
{

    public class Transform
    {
        public Transform() { ID = 0; }

        internal Transform(uint id) { 
            ID = id;
            _position = position;
            _rotation = rotation;
            _scale = scale;
        }

        public readonly uint ID;

        public Vector3 position
        {
            get
            {
                EngineCallbacks.TransformComponent_GetPosition(ID, ref _position);
                return _position; 
            }
            set
            {
                _position = value;
                EngineCallbacks.TransformComponent_SetPosition(ID, ref _position);
            }
        }

        public Vector3 rotation
        {
            get
            {
                Vector3 reference = new Vector3();
                EngineCallbacks.TransformComponent_GetRotation(ID, ref reference);

                _rotation = reference;

                return _rotation;
            }
            set
            {
                _rotation = value;
                EngineCallbacks.TransformComponent_SetRotation(ID, ref _rotation);
            }
        }
        public Vector3 scale
        {
            get
            {
                Vector3 reference = new Vector3();
                EngineCallbacks.TransformComponent_GetScale(ID, ref reference);

                _scale = reference;

                return _scale;
            }
            set
            {
                _scale = value;
                EngineCallbacks.TransformComponent_SetScale(ID, ref _scale);
            }
        }

        private Vector3 _position = new Vector3();
        private Vector3 _rotation = new Vector3();
        private Vector3 _scale = new Vector3();

        public Vector3 up
        {
            get
            {
                Vector3 reference = new Vector3();
                EngineCallbacks.TransformComponent_GetUp(ID, ref reference);
                return reference;
            }
        }

        public Vector3 front
        {
            get
            {
                Vector3 reference = new Vector3();
                EngineCallbacks.TransformComponent_GetFront(ID, ref reference);
                return reference;
            }
        }

        public Vector3 right
        {
            get
            {
                Vector3 reference = new Vector3();
                EngineCallbacks.TransformComponent_GetRight(ID, ref reference);
                return reference;
            }
        }
    }

    public class Prefab
    {
        internal Prefab(uint asset_handle) { Asset_Handle = asset_handle; }

        internal uint Asset_Handle;

        public uint ID
        {
            get { return Asset_Handle; }
        }
    }

    public enum ForceMode : uint
    {
        eFORCE,            
        eIMPULSE,          
        eVELOCITY_CHANGE,  
        eACCELERATION      
    };

    public enum ShadowTypeFlag : byte
    {
        NoShadows = 0,
        HardShadows = 1,
        SoftShadows = 2,
    }

    public struct PhysicsMaterial
    {
        public float DynamicFriction;
        public float StaticFriction;
        public float Bounciness;
    }

}

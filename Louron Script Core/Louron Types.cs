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

        internal Transform(uint id) { ID = id; }

        public readonly uint ID;

        public Vector3 position
        {
            get
            {
                Vector3 reference = new Vector3();
                EngineCallbacks.TransformComponent_GetPosition(ID, ref reference);

                _position = reference;

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

        private Vector3 _position;
        private Vector3 _rotation;
        private Vector3 _scale;

        // Helper function to convert degrees to radians
        private static float DegreesToRadians(float degrees)
        {
            return degrees * (MathF.PI / 180f);
        }

        public Vector3 up
        {
            get
            {
                return Vector3.Normalize(Vector3.Transform(Vector3.Up, GetRotationQuaternion()));
            }
        }

        public Vector3 front
        {
            get
            {
                return Vector3.Normalize(Vector3.Transform(Vector3.Front, GetRotationQuaternion()));
            }
        }

        public Vector3 right
        {
            get
            {
                return Vector3.Normalize(Vector3.Transform(Vector3.Right, GetRotationQuaternion()));
            }
        }

        // Generate a rotation quaternion from Euler angles
        private Quaternion GetRotationQuaternion()
        {
            float pitch = DegreesToRadians(rotation.X); // Rotation around X-axis
            float yaw = DegreesToRadians(rotation.Y);   // Rotation around Y-axis
            float roll = DegreesToRadians(rotation.Z);  // Rotation around Z-axis

            Quaternion qPitch = Quaternion.CreateFromAxisAngle(Vector3.Right, pitch);
            Quaternion qYaw = Quaternion.CreateFromAxisAngle(Vector3.Up, yaw);
            Quaternion qRoll = Quaternion.CreateFromAxisAngle(Vector3.Front, roll);

            // Combine rotations in Yaw -> Pitch -> Roll order
            return qYaw * qPitch * qRoll;
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

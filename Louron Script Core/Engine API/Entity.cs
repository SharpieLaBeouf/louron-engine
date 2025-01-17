using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using static System.Runtime.CompilerServices.RuntimeHelpers;

namespace Louron
{
    public class Entity
    {

        public readonly uint ID;

        #region Entity Properties

        public string tag
        {
            get
            {
                return EngineCallbacks.TagComponent_GetTag(ID);
            }
            set
            {
                EngineCallbacks.TagComponent_SetTag(ID, value);
            }
        }

        public Transform transform
        {
            get // Create a new reference for a transform, pass it to C++ and store the returned values
            {
                _Transform reference = new _Transform();
                EngineCallbacks.TransformComponent_GetTransform(ID, ref reference);

                _transform.position = reference.position;
                _transform.rotation = reference.rotation;
                _transform.scale = reference.scale;

                return _transform;
            }
            set // Create a new temporary transform proxy to pass to C++ to set the transform
            {
                _Transform temp;
                temp.position = value.position;
                temp.rotation = value.rotation;
                temp.scale = value.scale;

                EngineCallbacks.TransformComponent_SetTransform(ID, ref temp);
            }
        }

        #endregion

        #region Entity Public Methods

        public T AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
                return new T() { Entity = this };

            Type componentType = typeof(T);
            EngineCallbacks.Entity_AddComponent(ID, componentType);

            T component = new T() { Entity = this };
            return component;
        }

        public void RemoveComponent<T>() where T : Component, new ()
        {
            if (!HasComponent<T>())
                return;

            Type componentType = typeof(T);
            EngineCallbacks.Entity_RemoveComponent(ID, componentType);
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            Type componentType = typeof(T);
            return EngineCallbacks.Entity_HasComponent(ID, componentType);
        }

        public T? GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                return null;

            T component = new T() { Entity = this };
            return component;
        }

        #endregion

        #region Entity Internal and Private Variables

        protected Entity()
        {
            ID = 0;
            _transform = new Transform();
        }

        internal Entity(uint id)
        {
            ID = id;
            _transform = new Transform(ID);
        }

        internal Transform _transform;

        internal struct _Transform
        {
            public Vector3 position;
            public Vector3 rotation;
            public Vector3 scale;
        }

        #endregion
    }
}

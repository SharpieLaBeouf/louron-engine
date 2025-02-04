using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Xml.Linq;

namespace Louron
{
    public class ComputeShader
    {
        internal uint Asset_Handle;

        // Users cannot create Compute Shaders on the fly, C++ will
        // call this constructor with the appropriate asset point
        // when the script is instantiated.
        internal ComputeShader(uint asset_handle)
        {
            Asset_Handle = asset_handle; 
        } 

        public void SetBuffer(ComputeBuffer buffer, uint binding_index)
        {
            EngineCallbacks.ComputeShader_SetBuffer(Asset_Handle, buffer.NativePtr, binding_index);
        }

        public void Dispatch(uint x, uint y = 1, uint z = 1)
        {
            EngineCallbacks.ComputeShader_Dispatch(Asset_Handle, x, y, z);
        }

        // Shader Uniforms 
        public void SetBool(string name, bool value)
        {
            EngineCallbacks.ComputeShader_SetBool(Asset_Handle, name, value);
        }

        public void SetInt(string name, int value)
        {
            EngineCallbacks.ComputeShader_SetInt(Asset_Handle, name, value);
        }

        public void SetUInt(string name, uint value)
        {
            EngineCallbacks.ComputeShader_SetUInt(Asset_Handle, name, value);
        }

        public void SetFloat(string name, float value)
        {
            EngineCallbacks.ComputeShader_SetFloat(Asset_Handle, name, value);
        }

        public void SetVector2(string name, Vector2 value)
        {
            EngineCallbacks.ComputeShader_SetVector2(Asset_Handle, name, value);
        }

        public void SetVector3(string name, Vector3 value)
        {
            EngineCallbacks.ComputeShader_SetVector3(Asset_Handle, name, value);
        }

        public void SetVector4(string name, Vector4 value)
        {
            EngineCallbacks.ComputeShader_SetVector4(Asset_Handle, name, value);
        }

    }

    public class ComputeBuffer : IDisposable
    {
        internal IntPtr NativePtr;
        private bool IsReleased = false;

        public ComputeBuffer(int element_count, int element_size)
        {
            NativePtr = EngineCallbacks.ComputeBuffer_Create(element_count, element_size);
            if (NativePtr == IntPtr.Zero)
            {
                IsReleased = true;
                throw new ObjectDisposedException("ComputeBuffer Invalid Pointer.");
            }
        }

        ~ComputeBuffer()
        {
            Release();
        }

        /// <summary>
        /// Writes the Data array into the GPU.
        /// </summary>
        /// <typeparam name="T">The type of the data array.</typeparam>
        /// <param name="output">The object the data will be copied from.</param>
        public void SetData<T>(T[] data) where T : struct
        {
            if (data == null)
                throw new ObjectDisposedException("ComputeBuffer Cannot Set Null Data.");

            if (IsReleased) 
                throw new ObjectDisposedException("ComputeBuffer Invalid Pointer.");
            
            GCHandle handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                EngineCallbacks.ComputeBuffer_SetData(NativePtr, ptr, data.Length, Marshal.SizeOf<T>());
            }
            finally
            {
                handle.Free();
            }
        }

        /// <summary>
        /// Reads the Data back from the GPU and maps it to the output data T[] provided.
        /// </summary>
        /// <typeparam name="T">The type of the data array.</typeparam>
        /// <param name="output">The object the data will be copied into.</param>
        public void GetData<T>(T[] output) where T : struct
        {
            if (IsReleased) 
                throw new ObjectDisposedException("ComputeBuffer Invalid Pointer.");

            GCHandle handle = GCHandle.Alloc(output, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                EngineCallbacks.ComputeBuffer_GetData(NativePtr, ptr, output.Length, Marshal.SizeOf<T>());
            }
            finally
            {
                handle.Free();
            }
        }

        /// <summary>
        /// Releases the CPU and GPU data of this object.
        /// </summary>
        public void Release()
        {
            if (!IsReleased)
            {
                EngineCallbacks.ComputeBuffer_Release(NativePtr);
                IsReleased = true;
            }

            if (NativePtr != IntPtr.Zero)
                NativePtr = IntPtr.Zero;
        }

        /// <summary>
        /// Alternative to Release, but does the same thing.
        /// </summary>
        public void Dispose()
        {
            Release();
            GC.SuppressFinalize(this);
        }

    }
}

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Louron
{

    public class Material
    {
        internal uint Asset_Handle;

        public Material()
        {
            Asset_Handle = EngineCallbacks.Material_Create("New Material");
        }

        public Material(string material_name)
        {
            Asset_Handle = EngineCallbacks.Material_Create(material_name);
        }

        public void SetShader(uint shader_handle)
        {
            EngineCallbacks.Material_SetShader(Asset_Handle, shader_handle);
        }

        ~Material()
        {
            EngineCallbacks.Material_Destroy(Asset_Handle);
        }

        internal Material(uint asset_handle)
        {
            Asset_Handle = asset_handle;
        }
    }

    public class MaterialUniformBlock
    {
        internal IntPtr NativePtr;

        internal MaterialUniformBlock(IntPtr ptr)
        {
            NativePtr = ptr;
        }

        // Bool
        public void SetBool(string uniform_name, bool value)
        {
            unsafe
            {
                byte boolValue = value ? (byte)1 : (byte)0; // Ensure it's marshaled correctly
                EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.Bool, (IntPtr)(&boolValue));
            }
        }

        public void SetBoolVec2(string uniform_name, ref BVector2 value)
        {
            unsafe
            {
                fixed (BVector2* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.BVec2, (IntPtr)ptr);
                }
            }
        }

        public void SetBoolVec3(string uniform_name, ref BVector3 value)
        {
            unsafe
            {
                fixed (BVector3* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.BVec3, (IntPtr)ptr);
                }
            }
        }

        public void SetBoolVec4(string uniform_name, ref BVector4 value)
        {
            unsafe
            {
                fixed (BVector4* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.BVec4, (IntPtr)ptr);
                }
            }
        }

        // Int
        public void SetInt(string uniform_name, int value)
        {
            unsafe
            {
                EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.Int, (IntPtr)(&value));
            }
        }

        public void SetIntVec2(string uniform_name, ref IVector2 value)
        {
            unsafe
            {
                fixed (IVector2* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.IVec2, (IntPtr)ptr);
                }
            }
        }

        public void SetIntVec3(string uniform_name, ref IVector3 value)
        {
            unsafe
            {
                fixed (IVector3* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.IVec3, (IntPtr)ptr);
                }
            }
        }

        public void SetIntVec4(string uniform_name, ref IVector4 value)
        {
            unsafe
            {
                fixed (IVector4* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.IVec4, (IntPtr)ptr);
                }
            }
        }

        // Unsigned Int
        public void SetUInt(string uniform_name, uint value)
        {
            unsafe
            {
                EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.Uint, (IntPtr)(&value));
            }
        }

        public void SetUVec2(string uniform_name, ref UVector2 value)
        {
            unsafe
            {
                fixed (UVector2* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.UVec2, (IntPtr)ptr);
                }
            }
        }

        public void SetUVec3(string uniform_name, ref UVector3 value)
        {
            unsafe
            {
                fixed (UVector3* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.UVec3, (IntPtr)ptr);
                }
            }
        }

        public void SetUVec4(string uniform_name, ref UVector4 value)
        {
            unsafe
            {
                fixed (UVector4* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.UVec4, (IntPtr)ptr);
                }
            }
        }

        // Float
        public void SetFloat(string uniform_name, float value)
        {
            unsafe
            {
                EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.Float, (IntPtr)(&value));
            }
        }

        public void SetVec2(string uniform_name, ref Vector2 value)
        {
            unsafe
            {
                fixed (Vector2* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.Vec2, (IntPtr)ptr);
                }
            }
        }

        public void SetVec3(string uniform_name, ref Vector3 value)
        {
            unsafe
            {
                fixed (Vector3* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.Vec3, (IntPtr)ptr);
                }
            }
        }

        public void SetVec4(string uniform_name, ref Vector4 value)
        {
            unsafe
            {
                fixed (Vector4* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.Vec4, (IntPtr)ptr);
                }
            }
        }

        // Double
        public void SetDouble(string uniform_name, double value)
        {
            unsafe
            {
                EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.Double, (IntPtr)(&value));
            }
        }

        public void SetDVec2(string uniform_name, ref DVector2 value)
        {
            unsafe
            {
                fixed (DVector2* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.DVec2, (IntPtr)ptr);
                }
            }
        }

        public void SetDVec3(string uniform_name, ref DVector3 value)
        {
            unsafe
            {
                fixed (DVector3* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.DVec3, (IntPtr)ptr);
                }
            }
        }

        public void SetUintVec4(string uniform_name, ref DVector4 value)
        {
            unsafe
            {
                fixed (DVector4* ptr = &value)
                {
                    EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)GLSLType.DVec4, (IntPtr)ptr);
                }
            }
        }

        // Sampler Textures
        public void SetSampler(string uniform_name, uint asset_handle, GLSLType samplerType)
        {
            unsafe
            {
                EngineCallbacks.MaterialUniformBlock_SetUniform(NativePtr, uniform_name, (uint)samplerType, (IntPtr)(&asset_handle));
            }
        }

    }

    public enum GLSLType
    {
        Unknown,

		// Bool
		Bool, BVec2, BVec3, BVec4,

		// Int
		Int, IVec2, IVec3, IVec4,

		// Unsigned Int
		Uint, UVec2, UVec3, UVec4,

		// Float
		Float, Vec2, Vec3, Vec4,

		// Double
		Double, DVec2, DVec3, DVec4,

		// Matrix
		Mat2, Mat3, Mat4,

		// Textures
		Sampler1D, Sampler1DArray, Sampler1DShadow, Sampler1DArrayShadow, 
		Sampler2D, Sampler2DArray, Sampler2DShadow, Sampler2DArrayShadow,
		Sampler3D,
		SamplerCube, SamplerCubeArray, SamplerCubeShadow, SamplerCubeArrayShadow
    };
}

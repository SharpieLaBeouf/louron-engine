using Microsoft.VisualBasic;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Louron
{
    public class Mesh
    {
        internal uint Asset_Handle;

        public Mesh()
        {
            Asset_Handle = EngineCallbacks.Mesh_CreateNewMesh();
        }

        public void SubmitChanges(bool clear_cpu_data)
        {
            EngineCallbacks.Mesh_SubmitChanges(Asset_Handle, clear_cpu_data);
        }

        public void CopyBufferDataToCPU()
        {
            EngineCallbacks.Mesh_CopyBufferDataToCPU(Asset_Handle);
        }
        public void ClearBufferDataFromCPU()
        {
            EngineCallbacks.Mesh_ClearBufferDataFromCPU(Asset_Handle);
        }

        public uint[] GetTriangles()
        {
            EngineCallbacks.Mesh_GetTriangles(Asset_Handle, out IntPtr ptr, out int count);

            if (ptr == IntPtr.Zero || count == 0)
                return Array.Empty<uint>();
            
            uint[] output = new uint[count];
            for (int i = 0; i < count; i++)
                output[i] = (uint)Marshal.ReadInt32(ptr, i * sizeof(int));

            return output;
        }

        public void SetTriangles(uint[] triangles)
        {
            if (triangles == null)
                throw new ObjectDisposedException("Mesh Cannot Set Null Data (vertices).");

            GCHandle handle = GCHandle.Alloc(triangles, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                EngineCallbacks.Mesh_SetTriangles(Asset_Handle, ptr, (uint)triangles.Length);
            }
            finally
            {
                handle.Free();
            }
        }

        public float[] GetVertices()
        {
            EngineCallbacks.Mesh_GetVertices(Asset_Handle, out IntPtr ptr, out int count);

            if (ptr == IntPtr.Zero || count == 0)
                return Array.Empty<float>();

            float[] output = new float[count];
            Marshal.Copy(ptr, output, 0, count);

            return output;
        }

        public float[] GetNormals()
        {
            EngineCallbacks.Mesh_GetNormals(Asset_Handle, out IntPtr ptr, out int count);

            if (ptr == IntPtr.Zero || count == 0)
                return Array.Empty<float>();

            float[] output = new float[count];
            Marshal.Copy(ptr, output, 0, count);

            return output;
        }

        public float[] GetTextureCoords()
        {
            EngineCallbacks.Mesh_GetTextureCoords(Asset_Handle, out IntPtr ptr, out int count);

            if (ptr == IntPtr.Zero || count == 0)
                return Array.Empty<float>();

            float[] output = new float[count];
            Marshal.Copy(ptr, output, 0, count);

            return output;
        }

        public float[] GetTangents()
        {
            EngineCallbacks.Mesh_GetTangents(Asset_Handle, out IntPtr ptr, out int count);

            if (ptr == IntPtr.Zero || count == 0)
                return Array.Empty<float>();

            float[] output = new float[count];
            Marshal.Copy(ptr, output, 0, count);

            return output;
        }

        public float[] GetBitangents()
        {
            EngineCallbacks.Mesh_GetBitangents(Asset_Handle, out IntPtr ptr, out int count);

            if (ptr == IntPtr.Zero || count == 0)
                return Array.Empty<float>();

            float[] output = new float[count];
            Marshal.Copy(ptr, output, 0, count);

            return output;
        }

        public void SetVertices(float[] vertices)
        {
            if (vertices == null)
                throw new ObjectDisposedException("Mesh Cannot Set Null Data (vertices).");

            GCHandle handle = GCHandle.Alloc(vertices, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                EngineCallbacks.Mesh_SetVertices(Asset_Handle, ptr, (uint)vertices.Length);
            }
            finally
            {
                handle.Free();
            }
        }

        public void SetNormals(float[] normals)
        {
            if (normals == null)
                throw new ObjectDisposedException("Mesh Cannot Set Null Data (normals).");

            GCHandle handle = GCHandle.Alloc(normals, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                EngineCallbacks.Mesh_SetNormals(Asset_Handle, ptr, (uint)normals.Length);
            }
            finally
            {
                handle.Free();
            }
        }

        public void SetTextureCoords(float[] texture_coords)
        {
            if (texture_coords == null)
                throw new ObjectDisposedException("Mesh Cannot Set Null Data (texture_coords).");

            GCHandle handle = GCHandle.Alloc(texture_coords, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                EngineCallbacks.Mesh_SetTextureCoords(Asset_Handle, ptr, (uint)texture_coords.Length);
            }
            finally
            {
                handle.Free();
            }
        }

        public void SetTangents(float[] tangents)
        {
            if (tangents == null)
                throw new ObjectDisposedException("Mesh Cannot Set Null Data (tangents).");

            GCHandle handle = GCHandle.Alloc(tangents, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                EngineCallbacks.Mesh_SetTangents(Asset_Handle, ptr, (uint)tangents.Length);
            }
            finally
            {
                handle.Free();
            }
        }

        public void SetBitangents(float[] bitangents)
        {
            if (bitangents == null)
                throw new ObjectDisposedException("Mesh Cannot Set Null Data (bitangents).");

            GCHandle handle = GCHandle.Alloc(bitangents, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                EngineCallbacks.Mesh_SetBitangents(Asset_Handle, ptr, (uint)bitangents.Length);
            }
            finally
            {
                handle.Free();
            }
        }

        public void RecalculateNormals()
        {
            EngineCallbacks.Mesh_RecalculateNormals(Asset_Handle);
        }

        internal Mesh(uint asset_handle)
        {
            Asset_Handle = asset_handle;
        }
    }

    public class MeshFilterComponent : Component
    {
        private Mesh? _Mesh;
        public Mesh mesh
        {
            get
            {
                if (_Mesh == null) 
                    _Mesh = new Mesh(EngineCallbacks.MeshFilterComponent_CopyMesh(Entity.ID));
                
                return _Mesh;
            }
            set
            {
                _Mesh = value;
                EngineCallbacks.MeshFilterComponent_SetMesh(Entity.ID, _Mesh.Asset_Handle);
            }
        }

        public Mesh shared_mesh
        {
            get
            {
                if (_Mesh == null) 
                    _Mesh = new Mesh(EngineCallbacks.MeshFilterComponent_SharedMesh(Entity.ID));
                
                return _Mesh;
            }
            set
            {
                _Mesh = value;
                EngineCallbacks.MeshFilterComponent_SetMesh(Entity.ID, _Mesh.Asset_Handle);
            }
        }


        internal override void SetEntity(UInt32 entity_uuid)
        {
            base.SetEntity(entity_uuid);
        }
    }
}

using Microsoft.VisualBasic;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Text;

namespace Louron
{

    public enum TextureFormat : byte
    {
        RED_8 = 0,
        RED_GREEN_BLUE_8,
        RED_GREEN_BLUE_ALPHA_8

    }

    public class Texture2D
    {
        internal uint Asset_Handle;

        public Texture2D(int width, int height, TextureFormat texture_format = TextureFormat.RED_GREEN_BLUE_ALPHA_8)
        {
            Asset_Handle = EngineCallbacks.Texture2D_Create(width, height, (byte)texture_format);
        }

        public Texture2D(byte[] data, int width, int height, TextureFormat internal_format, TextureFormat data_format)
        {
            if (data == null)
                throw new ObjectDisposedException("Texture2D Cannot Set Null Data.");

            GCHandle handle = GCHandle.Alloc(data, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                Asset_Handle = EngineCallbacks.Texture2D_CreateWithData(ptr, width, height, (byte)internal_format, (byte)data_format);

            }
            finally
            {
                handle.Free();
            }
        }

        internal Texture2D(uint asset_handle)
        {
            Asset_Handle = asset_handle;
        }

        ~Texture2D()
        {
            Destroy();
        }

        public void SetPixel(Vector4 colour, IVector2 pixel_coord)
        {
            EngineCallbacks.Texture2D_SetPixel(Asset_Handle, colour, pixel_coord);
        }

        public void SetPixelData(byte[] pixel_data, TextureFormat pixel_data_format)
        {
            if (pixel_data == null)
                throw new ObjectDisposedException("Texture2D Cannot Set Null Data.");

            GCHandle handle = GCHandle.Alloc(pixel_data, GCHandleType.Pinned);
            IntPtr ptr = handle.AddrOfPinnedObject();

            try
            {
                EngineCallbacks.Texture2D_SetPixelData(Asset_Handle, ptr, (uint)pixel_data.Length, (byte)pixel_data_format);

            }
            finally
            {
                handle.Free();
            }
        }

        public void SubmitTextureChanges()
        {
            EngineCallbacks.Texture2D_SubmitTextureChanges(Asset_Handle);
        }

        public void Destroy()
        {
            EngineCallbacks.Texture2D_Destroy(Asset_Handle);

        }
    }
}

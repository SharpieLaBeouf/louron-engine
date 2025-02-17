using System;
using System.Runtime.InteropServices;

using Louron;

namespace SandboxProject
{

    public class OverrideMaterialTest : Entity
    {
        MeshRendererComponent mr;
        MaterialUniformBlock mub;

        Texture2D texture = null;

        public float scale = 1.0f;
        public Vector2 offset = new Vector2(0.0f);
        
        FastNoiseLite noise;

        byte[] texture_data = new byte[1024*1024];
        
        public void OnStart()
        {
            mr = GetComponent<MeshRendererComponent>();
			mr.EnableUniformBlock();
			mub = mr.GetUniformBlock();

            noise = new FastNoiseLite();
            noise.SetNoiseType(FastNoiseLite.NoiseType.Perlin);

            for (int y = 0; y < 1024; y++)
            {
                for (int x = 0; x < 1024; x++)
                {
                    float xCoord = x * scale + offset.X * 1024;
                    float yCoord = y * scale + offset.Y * 1024;

                    float noiseValue = noise.GetNoise(xCoord, yCoord);
                    texture_data[y * 1024 + x] = (byte)((noiseValue + 1) * 0.5f * 255);
                }
            }

            texture = new Texture2D(texture_data, 1024, 1024, TextureFormat.RED_8, TextureFormat.RED_8);

            mub.OverrideAlbedoTexture(texture);
        }

        public void OnUpdate()
        {
            if(!Input.GetKeyDown(KeyCode.Enter))
                return;
            
            for (int y = 0; y < 1024; y++)
            {
                for (int x = 0; x < 1024; x++)
                {
                    float xCoord = x * scale + offset.X * 1024;
                    float yCoord = y * scale + offset.Y * 1024;

                    float noiseValue = noise.GetNoise(xCoord, yCoord);
                    texture_data[y * 1024 + x] = (byte)((noiseValue + 1) * 0.5f * 255);
                }
            }

            texture.SetPixelData(texture_data, TextureFormat.RED_8);
            texture.SubmitTextureChanges();
        }

    }
}
using System;
using System.Runtime.InteropServices;

using Louron;

namespace SandboxProject
{

    public class GenerateMeshTest : Entity
    {
        // Public variables to adjust the noise functions and parameters
        public FastNoiseLite.NoiseType noiseType = FastNoiseLite.NoiseType.Perlin;
        public int fractalOctaves = 6;
        public float frequency = 0.05f;
        public float fractalGain = 0.36f;
        public float baseNoiseAmplitude = 15.0f;
        public float detailNoiseAmplitude = 135.0f;
        public float detailNoiseFrequency = 0.18f;

        public float Scale = 2.0f;

        public int SizeX = 100;
        public int SizeZ = 100;

        private MeshFilterComponent meshfilter;
        FastNoiseLite noise;

        Random rand;
        public void OnStart()
        {
            AddComponent<MeshRendererComponent>();
            meshfilter = AddComponent<MeshFilterComponent>();

            rand = new Random();
            noise = new FastNoiseLite();

            CreateMesh();
        }

        public void CreateMesh()
        {
            Debug.Log("Creating Mesh?");

            meshfilter.mesh = new Mesh();

            noise.SetNoiseType(noiseType); // Use public variable for noise type
            noise.SetFractalType(FastNoiseLite.FractalType.Ridged); // Use public variable for octaves
            noise.SetFractalOctaves(fractalOctaves); // Use public variable for octaves
            noise.SetFrequency(frequency); // Use public variable for frequency
            noise.SetFractalGain(fractalGain); // Use public variable for fractal gain

            float[] vertices = new float[(SizeX + 1) * (SizeZ + 1) * 3];

            float halfSizeX = SizeX * 0.5f;
            float halfSizeZ = SizeZ * 0.5f;

            Vector3 pos = transform.position;

            for (int index = 0, z = 0; z <= SizeZ; z++)
            {
                for (int x = 0; x <= SizeX; x++)
                {
                    // Scale down the x and z positions for larger terrain features
                    float nx = (x + pos.X - halfSizeX) * Scale * .01f; // Larger scale for smoother, open terrain
                    float nz = (z + pos.Z - halfSizeZ) * Scale * .01f; // Larger scale for smoother, open terrain

                    // Base noise for large terrain features
                    float baseHeight = noise.GetNoise(nx, nz) * baseNoiseAmplitude; // Base noise amplitude

                    // Add detail noise (higher frequency, smaller variation)
                    noise.SetFrequency(detailNoiseFrequency); // Higher frequency for small details
                    float detailHeight = noise.GetNoise(nx, nz) * detailNoiseAmplitude; // Smaller amplitude for detail noise

                    // Combine both base terrain and detail noise
                    vertices[index] = x - halfSizeX;
                    vertices[index + 1] = baseHeight + detailHeight; // Combine for more realistic terrain
                    vertices[index + 2] = z - halfSizeZ;

                    index += 3;
                }
            }

            meshfilter.mesh.SetVertices(vertices);

            uint[] triangles = new uint[SizeX * SizeZ * 6];

            uint vert = 0;
            int tris = 0;
            for (int z = 0; z < SizeZ; z++)
            {
                for (int x = 0; x < SizeX; x++)
                {
                    triangles[tris + 0] = vert + 0;
                    triangles[tris + 1] = vert + (uint)SizeX + 1;
                    triangles[tris + 2] = vert + 1;

                    triangles[tris + 3] = vert + 1;
                    triangles[tris + 4] = vert + (uint)SizeX + 1;
                    triangles[tris + 5] = vert + (uint)SizeX + 2;

                    vert++;
                    tris += 6;
                }
                vert++;
            }

            meshfilter.mesh.SetTriangles(triangles);

            meshfilter.mesh.RecalculateNormals();

            meshfilter.mesh.SubmitChanges(true);
        }

        public void OnUpdate()
        {
            if (Input.GetKeyDown(KeyCode.Enter))
                CreateMesh();
        }

        public void OnFixedUpdate() 
        {
        }

        public void OnDestroy() 
        {
        }

        // Collider Functions
        public void OnCollideEnter(Collider other) 
        {
        }

        public void OnCollideStay(Collider other) 
        {
        }

        public void OnCollideLeave(Collider other) 
        {
        }

        // Collider Trigger Functions
        public void OnTriggerEnter(Collider other) 
        {
        }

        public void OnTriggerStay(Collider other) 
        {
        }

        public void OnTriggerLeave(Collider other) 
        {
        }
    }
}
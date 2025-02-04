using System;
using System.Runtime.InteropServices;

using Louron;

namespace SandboxProject
{
	public struct MyData
	{
		public float myFloat;
		public uint myBool;

		public Vector2 myVec;
	};

	public class ComputeShaderTest : Entity
	{
		public ComputeShader CompShader;

		public int Iterations = 1;
		public Vector2 Addition = new Vector2(1.0f,1.0f);
		
		Random rand = new Random();
		
		public void OnStart()
		{
		}
		
		public void OnUpdate()
		{
			if (Input.GetKeyDown(KeyCode.Enter))
			{
				MyData[] data_array = new MyData[1000];
				for (int i = 0; i < data_array.Length; i++)
				{
					data_array[i].myFloat = (float)rand.NextDouble();
					data_array[i].myBool = (uint)rand.Next(2); // Generates 0 or 1
					data_array[i].myVec = new Vector2((float)rand.NextDouble(), (float)rand.NextDouble()); // Generates 0 or 1
				}
				
				ComputeBuffer buffer = new ComputeBuffer(data_array.Length, Marshal.SizeOf<MyData>());
				
				buffer.SetData(data_array);

				CompShader.SetBuffer(buffer, 0);
				CompShader.SetInt("u_Iterations", Iterations);
				CompShader.SetVector2("u_Addition", Addition);
				CompShader.Dispatch((uint)data_array.Length / 1024);

				buffer.GetData(data_array);

				Debug.Log("After");
				foreach (MyData data in data_array)
				{
					Debug.Log($"	Float Value: {data.myFloat}, Bool Value: {data.myBool}, Vector2: {data.myVec}");
				}

				buffer.Release();
			}
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
using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class MaterialUniformsTest : Entity
	{
		MeshRendererComponent mr;
		MaterialUniformBlock mub;

		float time;
		public float Speed = 1.0f;

		public Vector3 Colour = new Vector3(1.0f, 0.0f, 0.0f);

		public void OnStart()
		{
			mr = GetComponent<MeshRendererComponent>();
			mr.EnableUniformBlock();
			mub = mr.GetUniformBlock();

		}

		public void OnUpdate()
		{
			time += Time.deltaTime * Speed;

			float pingPongValue = LMath.PingPong(time, 1.5f) - 0.5f;

			mub.SetFloat("Time", pingPongValue);
			mub.SetVec3("GlowColour", ref Colour);
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
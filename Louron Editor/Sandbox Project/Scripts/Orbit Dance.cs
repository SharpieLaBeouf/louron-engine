using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class OrbitDance : Entity
	{
		
		public float Radius = 1.0f;
		public float Height = 1.0f;
		public float Speed = 1.0f;

		public void OnStart()
		{
			
		}
		
		public void OnUpdate()
		{
			Vector3 position = new Vector3();

			position.Z = Radius * MathF.Sin(Time.currentTime * Speed);
			position.Y = Height + Height * MathF.Sin(Time.currentTime * Speed * 2.0f);
			position.X = Radius * MathF.Cos(Time.currentTime * Speed);

			transform.position = position;
			transform.rotation = new Vector3(0.0f, MathF.Sin(Time.currentTime * Speed) * 360.0f, 0.0f);
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
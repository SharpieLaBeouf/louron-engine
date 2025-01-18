using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class BoxSinWaveY : Entity
	{
		public float speed = 0.5f;
		public float height = 10.0f;

		private float elapsedTime  = 0.0f;
		
		public void OnStart()
		{

		}
		
		public void OnUpdate()
		{
			elapsedTime  += Time.deltaTime;
			transform.position = new Vector3(transform.position.X, MathF.Sin(elapsedTime  * speed) * height, transform.position.Z);
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
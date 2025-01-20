using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class BulletScript : Entity
	{
		
		public float LifeTimer = 5.0f;

		public void OnStart()
		{
		}
		
		public void OnUpdate()
		{
			LifeTimer -= Time.deltaTime;

			if(LifeTimer <= 0.0f)
				Destroy(ID);
		}
		
		public void OnFixedUpdate() 
		{
			
		}
		
		public void OnDestroy() 
		{
			Debug.Log($"Destroyed: {tag}");
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
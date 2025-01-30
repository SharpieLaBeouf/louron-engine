using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class SunRotate : Entity
	{
		public Vector3 RotateAxis = new Vector3(0.0f, 1.0f, 0.0f);
		public float Speed = 10.0f;

		private Vector3 _StartRotation;

		public void OnStart()
		{
			_StartRotation = transform.rotation;
		}
		
		public void OnUpdate()
		{
			transform.rotation = _StartRotation + RotateAxis * (Speed * Time.currentTime);
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
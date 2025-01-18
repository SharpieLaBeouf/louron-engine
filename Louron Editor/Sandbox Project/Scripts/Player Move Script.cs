using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class PlayerMoveScript : Entity
	{
		public float speed = 5.0f;

		public void OnStart()
		{
			
		}
		
		public void OnUpdate()
		{
			Vector3 new_position = new Vector3();
			if(Input.GetKey(KeyCode.W)){
				new_position.X += Time.deltaTime * speed;
			}
			if(Input.GetKey(KeyCode.S)){
				new_position.X -= Time.deltaTime * speed;
			}
			if(Input.GetKey(KeyCode.A)){
				new_position.Z -= Time.deltaTime * speed;
			}
			if(Input.GetKey(KeyCode.D)){
				new_position.Z += Time.deltaTime * speed;
			}
			
			if(Input.GetKey(KeyCode.Space)){
				new_position.Y += Time.deltaTime * speed;
			}
			if(Input.GetKey(KeyCode.LeftShift)){
				new_position.Y -= Time.deltaTime * speed;
			}

			transform.position += new_position;
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
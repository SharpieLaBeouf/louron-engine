using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class RandomiseCubeLocations : Entity
	{
		public Entity cube1;
		public Entity cube2;
		public Entity cube3;
		public Entity cube4;
		public Entity cube5;
		public Entity cube6;
		
		public void OnStart()
		{
			Random rand = new Random();

			cube1.transform.position = new Vector3(
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 10.0f,
				(float)rand.NextDouble() * 50.0f + 20.0f,
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 20.0f
			);
			cube1.transform.rotation = new Vector3(
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f
			);
			cube1.AddComponent<RigidbodyComponent>();
			cube1.AddComponent<BoxColliderComponent>();

			cube2.transform.position = new Vector3(
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 10.0f,
				(float)rand.NextDouble() * 50.0f + 20.0f,
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 20.0f
			);
			cube2.transform.rotation = new Vector3(
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f
			);
			cube2.AddComponent<RigidbodyComponent>();
			cube2.AddComponent<BoxColliderComponent>();

			cube3.transform.position = new Vector3(
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 10.0f,
				(float)rand.NextDouble() * 50.0f + 20.0f,
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 20.0f
			);
			cube3.transform.rotation = new Vector3(
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f
			);
			cube3.AddComponent<RigidbodyComponent>();
			cube3.AddComponent<BoxColliderComponent>();

			cube4.transform.position = new Vector3(
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 10.0f,
				(float)rand.NextDouble() * 50.0f + 20.0f,
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 20.0f
			);
			cube4.transform.rotation = new Vector3(
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f
			);
			cube4.AddComponent<RigidbodyComponent>();
			cube4.AddComponent<BoxColliderComponent>();

			cube5.transform.position = new Vector3(
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 10.0f,
				(float)rand.NextDouble() * 50.0f + 20.0f,
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 20.0f
			);
			cube5.transform.rotation = new Vector3(
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f,
				(float)rand.NextDouble() * 360.0f
			);
			cube5.AddComponent<RigidbodyComponent>();
			cube5.AddComponent<BoxColliderComponent>();

			cube6.transform.position = new Vector3(
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 10.0f,
				(float)rand.NextDouble() * 50.0f + 20.0f,
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 20.0f
			);
			cube6.transform.rotation = new Vector3(
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 360.0f,
				(float)rand.NextDouble() * 360.0f,
				(float)((rand.NextDouble() * 2.0f) - 1.0f) * 360.0f
			);
			cube6.AddComponent<RigidbodyComponent>();
			cube6.AddComponent<BoxColliderComponent>();
		}
		
		public void OnUpdate()
		{
			
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
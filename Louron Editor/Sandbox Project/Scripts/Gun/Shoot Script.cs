using System;

using Louron;

namespace SandboxProject
{

	public class ShootScript : Entity
	{

		public Prefab bullet_prefab;
		public float speed = 1000.0f;
		
		public void OnStart()
		{
				Debug.Log($"SHOOT Rotation: {transform.rotation.ToString()}");
		}
		
		public void OnUpdate()
		{
			if(Input.GetMouseButtonDown(MouseButton.Mouse_Button_Left)){
				Entity clone = Instantiate(bullet_prefab, transform.position);
				clone.GetComponent<RigidbodyComponent>().ApplyForce(transform.front * speed, ForceMode.eIMPULSE);
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
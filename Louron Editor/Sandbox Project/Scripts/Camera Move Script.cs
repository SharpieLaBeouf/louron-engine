using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class CameraMoveScript : Entity
	{
		public float CameraSpeed = 10.0f;
		
		// Persistent rotation variables to store pitch and yaw
		private float pitch = 0.0f; // Rotation around the X-axis
		private float yaw = 0.0f;   // Rotation around the Y-axis
		
		private Vector2 last_mouse;
		private bool first_mouse = true;

		public void OnStart()
		{
			
		}
		
		public void OnUpdate()
		{
			Vector3 position = new Vector3();

			if (Input.GetKey(KeyCode.W))
				position += transform.front * CameraSpeed * Time.deltaTime;
			if (Input.GetKey(KeyCode.S))
				position -= transform.front * CameraSpeed * Time.deltaTime;
			if (Input.GetKey(KeyCode.A))
				position -= transform.right * CameraSpeed * Time.deltaTime;
			if (Input.GetKey(KeyCode.D))
				position += transform.right * CameraSpeed * Time.deltaTime;
			if (Input.GetKey(KeyCode.LeftShift))
				position -= transform.up * CameraSpeed * Time.deltaTime;
			if (Input.GetKey(KeyCode.Space))
				position += transform.up * CameraSpeed * Time.deltaTime;

			transform.position += position;

			if (first_mouse){
				last_mouse = new Vector2(Input.GetMouseX(), Input.GetMouseY());
				first_mouse = false;
			}

			// Mouse input for camera rotation
			float mouseSensitivity = 0.1f; // Adjust for sensitivity
			
			Vector2 new_mouse = new Vector2(Input.GetMouseX(), Input.GetMouseY());

			float xoffset  = (new_mouse.X -last_mouse.X) * mouseSensitivity;
			float yoffset  = (last_mouse.Y - new_mouse.Y) * mouseSensitivity;

			last_mouse = new Vector2(Input.GetMouseX(), Input.GetMouseY());

			// Update yaw (horizontal rotation) and pitch (vertical rotation)
			yaw -= xoffset;
			pitch += yoffset;

			// Clamp pitch to prevent flipping
			pitch = Math.Clamp(pitch, -89.0f, 89.0f);

			transform.rotation = new Vector3(pitch, yaw, 0.0f);
		}
		
		private float Radians(float degrees){
			return (MathF.PI / 180.0f) * degrees;
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
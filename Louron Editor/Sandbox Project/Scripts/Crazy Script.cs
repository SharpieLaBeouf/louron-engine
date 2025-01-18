using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class CrazyScript : Entity
	{
		public Vector3 MaxRegion = new Vector3(20.0f, 20.0f, 20.0f);

		public float Speed = 10.0f; // Per Second

		float StartTime = 0.0f;
		float JourneyLength = 0.0f;
		Vector3 RandomPoint;
		Vector3 OldPoint;
		readonly Random rand = new Random();
		
		public void OnStart()
		{
			StartTime = Time.currentTime;
			OldPoint = transform.position;
			RandomPoint = new Vector3(
				(float)((rand.NextDouble() * 2) - 1) * MaxRegion.X,
				(float)((rand.NextDouble() * 2) - 1) * MaxRegion.Y,
				(float)((rand.NextDouble() * 2) - 1) * MaxRegion.Z
			);
			
			JourneyLength = Vector3.Distance(OldPoint, RandomPoint);
		}
		
		public void OnUpdate()
		{
			// Distance moved equals elapsed time times speed..
			float distCovered = (Time.currentTime - StartTime) * Speed;

			// Fraction of journey completed equals current distance divided by total distance.
			float fractionOfJourney = distCovered / JourneyLength;
			transform.position = Vector3.Lerp(fractionOfJourney, OldPoint, RandomPoint);

			if (fractionOfJourney >= 1.0f) {
				
				StartTime = Time.currentTime;
				OldPoint = transform.position;
				RandomPoint = new Vector3(
					(float)((rand.NextDouble() * 2) - 1) * MaxRegion.X,
					(float)((rand.NextDouble() * 2) - 1) * MaxRegion.Y,
					(float)((rand.NextDouble() * 2) - 1) * MaxRegion.Z
				);

				JourneyLength = Vector3.Distance(OldPoint, RandomPoint);

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
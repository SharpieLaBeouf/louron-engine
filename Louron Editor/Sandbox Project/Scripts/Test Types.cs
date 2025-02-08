using System;
using System.Collections.Generic;
using System.Text;

using Louron;

namespace SandboxProject
{

	public class TestTypes : Entity
	{
		
		// Primitive Types
		public bool BoolField = true;
		public byte ByteField = 255;
		public sbyte SbyteField = -128;
		public char CharField = 'A';
		public decimal DecimalField = 123.456m;
		public double DoubleField = 123.456;
		public float FloatField = 123.45f;
		public int IntField = 12345;
		public uint UintField = 12345;
		public long LongField = 1234567890;
		public ulong UlongField = 1234567890;
		public short ShortField = 1234;
		public ushort UshortField = 1234;

		// Louron Custom Types
		public Vector2 Vector2Field = new Vector2(1.0f, 2.0f);
		public Vector3 Vector3Field = new Vector3(1.0f, 2.0f, 3.0f);
		public Vector4 Vector4Field = new Vector4(1.0f, 2.0f, 3.0f, 4.0f);

		public Entity EntityField;
		public Prefab PrefabField;
		public TransformComponent TransformComponentField;
		public TagComponent TagComponentField;
		public ScriptComponent ScriptComponentField;
		public PointLightComponent PointLightComponentField;
		public SpotLightComponent SpotLightComponentField;
		public DirectionalLightComponent DirectionalLightComponentField;
		public RigidbodyComponent RigidbodyComponentField;
		public BoxColliderComponent BoxColliderComponentField;
		public SphereColliderComponent SphereColliderComponentField;
		public MeshRendererComponent MeshRendererComponentField;

		public Component ComponentField;

		public void OnStart()
		{

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
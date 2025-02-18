#pragma once

// Louron Core Headers
#include "../Scene/Scene.h"

// C++ Standard Library Headers
#include <string>
#include <map>

// External Vendor Library Headers
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace Louron {

	class Entity;

#pragma region Script Fields

	enum class ScriptFieldType
	{
		None = 0,
		Bool,
		Byte,
		Sbyte,
		Char,
		Decimal,
		Double,
		Float,
		Int,
		Uint,
		Long,
		Ulong,
		Short,
		Ushort,
		Vector2,
		Vector3,
		Vector4,
		Entity,
		TransformComponent,
		TagComponent,
		ScriptComponent,
		PointLightComponent,
		SpotLightComponent,
		DirectionalLightComponent,
		RigidbodyComponent,
		BoxColliderComponent,
		SphereColliderComponent,
		MeshFilterComponent,
		MeshRendererComponent,
		Component,
		Prefab,
		ComputeShader
	};

	struct ScriptField
	{

	public:

		ScriptFieldType Type;
		std::string Name;

		MonoClassField* ClassField;

		ScriptField() : Type(ScriptFieldType::None), Name(""), ClassField(nullptr) { }

		ScriptField(ScriptFieldType type, std::string name, MonoClassField* classField) :Type(type), Name(name), ClassField(classField) { }

		~ScriptField() {
			if (m_Buffer)
				delete[] m_Buffer;
		}

		template<typename T>
		T GetInitialValue() const
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			if(!m_Buffer)
				return T();
			return *(T*)m_Buffer;
		}
		
	private:

		char* m_Buffer = nullptr;

		void SetInitialValue(const char* value);

		friend class ScriptManager;

	};

	struct ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}

		template<typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			return *(T*)m_Buffer;
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			memcpy(m_Buffer, &value, sizeof(T));
		}

	private:
		uint8_t m_Buffer[16];

		friend class ScriptManager;
		friend class ScriptInstance;
	};
	// Key is name of field - value is ScriptFieldInstance
	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

#pragma endregion

#pragma region Script Class and Instances

	class ScriptClass {

	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool core_assembly = false);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr, const UUID& entity_uuid = NULL_UUID);

		const std::unordered_map<std::string, ScriptField>& GetFields() const { return m_Fields; }

		MonoClass* GetMonoClass() { return m_MonoClass; }

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;

		std::unordered_map<std::string, ScriptField> m_Fields;

		friend class ScriptManager;

	};

	enum class _Collider_Type : uint8_t {
		None = 0,
		Box_Collider,
		Sphere_Collider,
		Capsule_Collider,
		Mesh_Collider
	};

	enum class _Collision_Type : uint8_t {
		None = 0,
		CollideEnter,
		CollideStay,
		CollideLeave,
		TriggerEnter,
		TriggerStay,
		TriggerLeave
	};

	struct _Collider {
		_Collider_Type type;
		uint32_t other_collider_uuid;
	};

	class ScriptInstance {

	public:

		ScriptInstance(std::shared_ptr<ScriptClass> class_ref, Entity entity);

		void InvokeOnCreate();
		void InvokeOnUpdate();
		void InvokeOnFixedUpdate();
		void InvokeOnDestroy();

		void InvokeOnCollideEnter(void** other_collider_param);
		void InvokeOnCollideStay(void** other_collider_param);
		void InvokeOnCollideLeave(void** other_collider_param);

		void InvokeOnTriggerEnter(void** other_collider_param);
		void InvokeOnTriggerStay(void** other_collider_param);
		void InvokeOnTriggerLeave(void** other_collider_param);

		std::shared_ptr<ScriptClass> GetScriptClass() { return m_ScriptClass; }
		MonoObject* GetManagedObject() { return m_Instance; }

		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			bool success = GetFieldValueInternal(name, s_FieldValueBuffer);
			if (!success)
				return T();

			return *(T*)s_FieldValueBuffer;
		}

		template<typename T>
		void SetFieldValue(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			SetFieldValueInternal(name, &value);
		}

		void SetFieldEntityValue(const ScriptFieldInstance& field_instance, UUID value);
		UUID GetFieldEntityValue(const std::string& field_name);

		void SetFieldComponentPropertyValue(const ScriptFieldInstance& field_instance, UUID value);
		UUID GetFieldComponentPropertyValue(const std::string& field_name);

		void SetFieldPrefabValue(const ScriptFieldInstance& field_instance, AssetHandle value);
		AssetHandle GetFieldPrefabValue(const std::string& field_name);

		void SetFieldComputeShaderValue(const ScriptFieldInstance& field_instance, AssetHandle value);
		AssetHandle GetFieldComputeShaderValue(const std::string& field_name);

	private:

		bool GetFieldValueInternal(const std::string& name, void* buffer);
		bool SetFieldValueInternal(const std::string& name, const void* value);

	private:

		std::shared_ptr<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;

		MonoMethod* m_Constructor = nullptr;

		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
		MonoMethod* m_OnFixedUpdateMethod = nullptr;
		MonoMethod* m_OnDestroyMethod = nullptr;

		MonoMethod* m_OnCollideEnterMethod = nullptr;
		MonoMethod* m_OnCollideStayMethod = nullptr;
		MonoMethod* m_OnCollideLeaveMethod = nullptr;

		MonoMethod* m_OnTriggerEnterMethod = nullptr;
		MonoMethod* m_OnTriggerStayMethod = nullptr;
		MonoMethod* m_OnTriggerLeaveMethod = nullptr;

		UUID m_EntityUUID = NULL_UUID;

		inline static char s_FieldValueBuffer[16];

		friend class ScriptManager;
		friend struct ScriptFieldInstance;
	};

#pragma endregion

	class ScriptManager {

	public:

		static void Init();
		static void Shutdown();
		static ScriptManager& Get();

		static bool LoadAssembly(const std::filesystem::path& filepath);
		static bool LoadAppAssembly(const std::filesystem::path& filepath);

		static void ReloadAssembly();

		static void OnRuntimeStart(std::shared_ptr<Scene> scene);
		static void OnRuntimeStop();

		static Scene* GetSceneContext();

		static bool EntityClassExists(const std::string& fullClassName);
		static ScriptFieldMap& GetScriptFieldMap(UUID entity_uuid, const std::string& script_name);

		static void OnCreateEntity(Entity entity);
		static void OnCreateEntityScript(Entity entity, const std::string& script_name);
		static void OnDestroyEntity(Entity entity);

		static void OnUpdateEntity(Entity entity);
		static void OnFixedUpdateEntity(Entity entity);
		static void OnCollideEntity(Entity entity, Entity other_entity, _Collision_Type collision_type);

		static void CheckInactiveScriptsOnEntities();

		static std::shared_ptr<ScriptClass> GetEntityClass(const std::string& name);
		static const std::unordered_map<std::string, std::shared_ptr<ScriptClass>>& GetEntityClasses();
		static void AddEntityClass(const std::string& entity_class_full_name, std::shared_ptr<ScriptClass> script_class); // Must be "{NameSpace}.{ClassName}"

		static MonoImage* GetCoreAssemblyImage();

		static std::shared_ptr<ScriptInstance> GetEntityScriptInstance(UUID entityID, const std::string& script_name);

		static void SetAppAssemblyPath(const std::filesystem::path& file_path);

	private:

		static void LoadAssemblyClasses(MonoAssembly* assembly);

		ScriptManager();
		~ScriptManager();

		// Delete copy assignment and move assignment constructors
		ScriptManager(const ScriptManager&) = delete;
		ScriptManager(ScriptManager&&) = delete;

		// Delete copy assignment and move assignment operators
		ScriptManager& operator=(const ScriptManager&) = delete;
		ScriptManager& operator=(ScriptManager&&) = delete;



	};

	namespace ScriptingUtils {

		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
				case ScriptFieldType::None:							return "None";
				case ScriptFieldType::Bool:							return "Bool";
				case ScriptFieldType::Byte:							return "Byte";
				case ScriptFieldType::Sbyte:						return "Sbyte";
				case ScriptFieldType::Char:							return "Char";
				case ScriptFieldType::Decimal:						return "Decimal";
				case ScriptFieldType::Double:						return "Double";
				case ScriptFieldType::Float:						return "Float";
				case ScriptFieldType::Int:							return "Int";
				case ScriptFieldType::Uint:							return "Uint";
				case ScriptFieldType::Long:							return "Long";
				case ScriptFieldType::Ulong:						return "Ulong";
				case ScriptFieldType::Short:						return "Short";
				case ScriptFieldType::Ushort:						return "Ushort";
				case ScriptFieldType::Vector2:						return "Vector2";
				case ScriptFieldType::Vector3:						return "Vector3";
				case ScriptFieldType::Vector4:						return "Vector4";
				case ScriptFieldType::Entity:						return "Entity";
				case ScriptFieldType::TransformComponent:			return "TransformComponent";
				case ScriptFieldType::TagComponent:					return "TagComponent";
				case ScriptFieldType::ScriptComponent:				return "ScriptComponent";
				case ScriptFieldType::PointLightComponent:			return "PointLightComponent";
				case ScriptFieldType::SpotLightComponent:			return "SpotLightComponent";
				case ScriptFieldType::DirectionalLightComponent:	return "DirectionalLightComponent";
				case ScriptFieldType::RigidbodyComponent:			return "RigidbodyComponent";
				case ScriptFieldType::BoxColliderComponent:			return "BoxColliderComponent";
				case ScriptFieldType::SphereColliderComponent:		return "SphereColliderComponent";
				case ScriptFieldType::MeshFilterComponent:			return "MeshFilterComponent";
				case ScriptFieldType::MeshRendererComponent:		return "MeshRendererComponent";
				case ScriptFieldType::Component:					return "Component";
				case ScriptFieldType::Prefab:						return "Prefab";
				case ScriptFieldType::ComputeShader:				return "ComputeShader";
			}
			L_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return "None";
		}



		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")						return ScriptFieldType::None;
			if (fieldType == "Bool")						return ScriptFieldType::Bool;
			if (fieldType == "Byte")						return ScriptFieldType::Byte;
			if (fieldType == "Sbyte")						return ScriptFieldType::Sbyte;
			if (fieldType == "Char")						return ScriptFieldType::Char;
			if (fieldType == "Decimal")						return ScriptFieldType::Decimal;
			if (fieldType == "Double")						return ScriptFieldType::Double;
			if (fieldType == "Float")						return ScriptFieldType::Float;
			if (fieldType == "Int")							return ScriptFieldType::Int;
			if (fieldType == "Uint")						return ScriptFieldType::Uint;
			if (fieldType == "Long")						return ScriptFieldType::Long;
			if (fieldType == "Ulong")						return ScriptFieldType::Ulong;
			if (fieldType == "Short")						return ScriptFieldType::Short;
			if (fieldType == "Ushort")						return ScriptFieldType::Ushort;
			if (fieldType == "Vector2")						return ScriptFieldType::Vector2;
			if (fieldType == "Vector3")						return ScriptFieldType::Vector3;
			if (fieldType == "Vector4")						return ScriptFieldType::Vector4;
			if (fieldType == "Entity")						return ScriptFieldType::Entity;
			if (fieldType == "TransformComponent")			return ScriptFieldType::TransformComponent;
			if (fieldType == "TagComponent")				return ScriptFieldType::TagComponent;
			if (fieldType == "ScriptComponent")				return ScriptFieldType::ScriptComponent;
			if (fieldType == "PointLightComponent")			return ScriptFieldType::PointLightComponent;
			if (fieldType == "SpotLightComponent")			return ScriptFieldType::SpotLightComponent;
			if (fieldType == "DirectionalLightComponent")	return ScriptFieldType::DirectionalLightComponent;
			if (fieldType == "RigidbodyComponent")			return ScriptFieldType::RigidbodyComponent;
			if (fieldType == "BoxColliderComponent")		return ScriptFieldType::BoxColliderComponent;
			if (fieldType == "SphereColliderComponent")		return ScriptFieldType::SphereColliderComponent;
			if (fieldType == "MeshFilterComponent")			return ScriptFieldType::MeshFilterComponent;
			if (fieldType == "MeshRendererComponent")		return ScriptFieldType::MeshRendererComponent;
			if (fieldType == "Component")					return ScriptFieldType::Component;
			if (fieldType == "Prefab")						return ScriptFieldType::Prefab;
			if (fieldType == "ComputeShader")				return ScriptFieldType::ComputeShader;

			L_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return ScriptFieldType::None;
		}


	}

}
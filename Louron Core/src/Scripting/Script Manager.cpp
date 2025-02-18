#include "Script Manager.h"


// Louron Core Headers
#include "Script Connector.h"
#include "../Core/Logging.h"
#include "../Debug/Assert.h"
#include "../OpenGL/Compute Shader Asset.h"
#include "../Project/Project.h"
#include "../Scene/Entity.h"

// C++ Standard Library Headers
#include <fstream>
#include <iostream>

// External Vendor Library Headers
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/exception.h"
#include "mono/metadata/threads.h"
#include "mono/metadata/mono-gc.h"


namespace Louron {

	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Boolean",						ScriptFieldType::Bool },
		{ "System.Byte",						ScriptFieldType::Byte },
		{ "System.SByte",						ScriptFieldType::Sbyte },
		{ "System.Char",						ScriptFieldType::Char },
		{ "System.Decimal",						ScriptFieldType::Decimal },
		{ "System.Double",						ScriptFieldType::Double },
		{ "System.Single",						ScriptFieldType::Float },
		{ "System.Int32",						ScriptFieldType::Int },
		{ "System.UInt32",						ScriptFieldType::Uint },
		{ "System.Int64",						ScriptFieldType::Long },
		{ "System.UInt64",						ScriptFieldType::Ulong },
		{ "System.Int16",						ScriptFieldType::Short },
		{ "System.UInt16",						ScriptFieldType::Ushort },
												
		{ "Louron.Vector2",						ScriptFieldType::Vector2 },
		{ "Louron.Vector3",						ScriptFieldType::Vector3 },
		{ "Louron.Vector4",						ScriptFieldType::Vector4 },

		{ "Louron.Entity",						ScriptFieldType::Entity },
		{ "Louron.TransformComponent",			ScriptFieldType::TransformComponent },
		{ "Louron.TagComponent",				ScriptFieldType::TagComponent },
		{ "Louron.ScriptComponent",				ScriptFieldType::ScriptComponent },
		{ "Louron.PointLightComponent",			ScriptFieldType::PointLightComponent },
		{ "Louron.SpotLightComponent",			ScriptFieldType::SpotLightComponent },
		{ "Louron.DirectionalLightComponent",	ScriptFieldType::DirectionalLightComponent },
		{ "Louron.RigidbodyComponent",			ScriptFieldType::RigidbodyComponent },
		{ "Louron.BoxColliderComponent",		ScriptFieldType::BoxColliderComponent },
		{ "Louron.SphereColliderComponent",		ScriptFieldType::SphereColliderComponent },
		{ "Louron.MeshFilterComponent",			ScriptFieldType::MeshFilterComponent },
		{ "Louron.MeshRendererComponent",		ScriptFieldType::MeshRendererComponent },
		{ "Louron.Component",					ScriptFieldType::Component },
		{ "Louron.Prefab",						ScriptFieldType::Prefab },
		{ "Louron.ComputeShader",				ScriptFieldType::ComputeShader }

	};

	struct ScriptManagerData {
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilepath = "";
		std::filesystem::path AppAssemblyFilepath = "";

		bool EnableDebugging = true;

		ScriptClass EntityClass;
		ScriptClass PrefabClass;
		ScriptClass ComputeShaderClass;
		// Key is {Namespace}.{ClassName} - Value is shared_ptr to ScriptClass
		std::unordered_map<std::string, std::shared_ptr<ScriptClass>> EntityClasses;

		// Key is (UUID + Script Name) - value is shared_ptr to ScriptInstance
		std::unordered_map<std::string, std::shared_ptr<ScriptInstance>> EntityInstances;

		// Key is (UUID + Script Name) - value is map of ScriptFieldInstances
		std::unordered_map<std::string, ScriptFieldMap> EntityScriptFields;

		std::weak_ptr<Scene> SceneContext;

		// Key = Entity
		// Value = Vector of ScriptNames on this Entity that are inactive
		std::unordered_map<UUID, std::vector<std::string>> InactiveEntityScripts;
	};

	static ScriptManager* s_Instance = nullptr;
	static ScriptManagerData* s_Data = nullptr;

	namespace ScriptingUtils {

		static char* ReadBytes(const std::string& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				// Failed to open the file
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = static_cast<uint32_t>(end - stream.tellg());

			if (size == 0)
			{
				// File is empty
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = size;
			return buffer;
		}

		static MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath, bool enable_debugging)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			if (enable_debugging)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");

				if (std::filesystem::exists(pdbPath))
				{
					uint32_t pdbFileSize = 0;
					char* pdbFileData = ReadBytes(assemblyPath, &fileSize);
					mono_debug_open_image_from_memory(image, (const mono_byte*)(pdbFileData), pdbFileSize);
					L_CORE_INFO("Loaded PDB {}", pdbPath.string());
					//delete[] pdbFileData;
				}
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
			mono_image_close(image);

			// Don't forget to free the file data
			delete[] fileData;

			return assembly;
		}

		static void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			L_CORE_INFO("Mono Has Found The Following Script Classes: ");
			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
				
				L_CORE_INFO("{}.{}", nameSpace, name);
			}
		}

		static MonoClass* GetClassInAssembly(MonoImage* image, const char* namespaceName, const char* className)
		{
			MonoClass* class_ref = mono_class_from_name(image, namespaceName, className);

			L_CORE_ASSERT(class_ref, "Could Not Get Mono Class.");

			return class_ref;
		}
		
		static MonoObject* InstantiateClass(MonoClass* mono_class)
		{
			// Allocate an instance of our class
			MonoObject* class_instance = mono_object_new(s_Data->AppDomain, mono_class);
			L_CORE_ASSERT(class_instance, "Could Not Instantiate Mono Class.");

			// Call the parameterless (default) constructor
			mono_runtime_object_init(class_instance);

			return class_instance;
		}

		static ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_ScriptFieldTypeMap.find(typeName);
			if (it == s_ScriptFieldTypeMap.end())
			{
				L_CORE_ERROR("Unknown type: {}", typeName);
				return ScriptFieldType::None;
			}

			return it->second;
		}

	}

#pragma region ScriptClass

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool core_assembly)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = ScriptingUtils::GetClassInAssembly(core_assembly ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, m_ClassNamespace.c_str(), m_ClassName.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptingUtils::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params, const UUID& entity_uuid)
	{
		MonoObject* exception = nullptr;
		auto object = mono_runtime_invoke(method, instance, params, &exception);
		if (exception) {

			MonoClass* exceptionClass = mono_object_get_class(exception); // Get the class of the exception
			MonoProperty* messageProp = mono_class_get_property_from_name(exceptionClass, "Message"); // Get the Exception::Message property
			if (messageProp) {
				MonoObject* messageObj = mono_property_get_value(messageProp, exception, nullptr, nullptr); // Get the value of the property
				MonoString* exceptionString = mono_object_to_string(messageObj, nullptr); // Convert the value to a string
				const char* message = mono_string_to_utf8((MonoString*)messageObj);

				L_CORE_INFO("Entity \"{1}\", Script \"{0}\" - Runtime Exception: \"{2}\".", m_ClassName, s_Data->SceneContext.lock()->FindEntityByUUID(entity_uuid).GetName(), message);
			}
		}
		return object;
	}


#pragma endregion

#pragma region ScriptManager


	ScriptManager::ScriptManager() {

		L_CORE_INFO("Loading Script Manager...");

		// 1. Load Mono
		mono_set_assemblies_path("mono/lib");

		// Enable Debugging
		if (s_Data->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=mono/MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		s_Data->RootDomain = mono_jit_init("LouronScriptRuntime_ROOT");
		L_CORE_ASSERT(s_Data->RootDomain, "Could Not Initialise Mono Root Domain.");

		if (s_Data->EnableDebugging)
			mono_debug_domain_create(s_Data->RootDomain);

		mono_thread_set_main(mono_thread_current());

		// 2. Bridge Script and C++ Function Connections
		ScriptConnector::RegisterFunctions();

		// 3. Load Core Assembly Library (Louron Scripting Core)
		bool status = LoadAssembly("Resources/Scripting/Louron Script Core.dll");
		if (!status)
		{
			L_CORE_ERROR("ScriptManager could not load Louron Script Core.dll assembly.");
			return;
		}

		// 4. Load App Assembly Library (Project DLL)
		auto scriptModulePath = Project::GetActiveProject()->GetProjectDirectory() / Project::GetActiveProject()->GetConfig().AppScriptAssemblyPath;
		status = LoadAppAssembly(scriptModulePath);
		if (!status)
		{
			L_CORE_ERROR("ScriptManager could not load app dll assembly.");
			return;
		}

		LoadAssemblyClasses(s_Data->CoreAssembly);

		// 5. Register Components
		ScriptConnector::RegisterComponents();

		// 6. Setup EntityClass ScriptClass
		s_Data->EntityClass = ScriptClass("Louron", "Entity", true);
		s_Data->PrefabClass = ScriptClass("Louron", "Prefab", true);
		s_Data->ComputeShaderClass = ScriptClass("Louron", "ComputeShader", true);

	}

	ScriptManager::~ScriptManager() {

		if(s_Data) {
			Shutdown();
		}

	}

	void ScriptManager::Init() {

		L_CORE_ASSERT(!s_Instance, "Script Manager Already Initialised!");

		s_Data = new ScriptManagerData();
		s_Instance = new ScriptManager();
	}

	void ScriptManager::Shutdown() {

		if (s_Data)
		{
			mono_domain_set(mono_get_root_domain(), false);

			mono_domain_unload(s_Data->AppDomain);
			s_Data->AppDomain = nullptr;

			mono_jit_cleanup(s_Data->RootDomain);
			s_Data->RootDomain = nullptr;

			delete s_Data;
			s_Data = nullptr;
		}
	}

	ScriptManager& ScriptManager::Get() {

		L_CORE_ASSERT(s_Instance, "Script Manager Not Initialised!");

		return *s_Instance;
	}

	void ScriptManager::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* type_def_table = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t num_types = mono_table_info_get_rows(type_def_table);
		MonoClass* entity_class = mono_class_from_name(s_Data->CoreAssemblyImage, "Louron", "Entity");


		for (int32_t i = 0; i < num_types; i++) {

			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(type_def_table, i, cols, MONO_TYPEDEF_SIZE);

			const char* name_space = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);

			std::string full_name = (strlen(name_space) != 0) ? fmt::format("{}.{}", name_space, name) : name;

			MonoClass* mono_class = mono_class_from_name(s_Data->AppAssemblyImage, name_space, name);

			if (!mono_class || mono_class == entity_class)
				continue;

			bool is_entity = mono_class_is_subclass_of(mono_class, entity_class, false);
			if (!is_entity)
				continue;

			std::shared_ptr<ScriptClass> script_class = std::make_shared<ScriptClass>(name_space, name);
			s_Data->EntityClasses[full_name] = script_class;

			// Allocate an instance of our class
			MonoObject* temporary_instance = mono_object_new(s_Data->AppDomain, mono_class);
			L_CORE_ASSERT(temporary_instance, "Could Not Instantiate Mono Class.");

			// Call the parameterless (default) constructor
			mono_runtime_object_init(temporary_instance);

			// Fields
			{
				int fieldCount = mono_class_num_fields(mono_class);
				L_CORE_INFO("{} has {} fields:", name, fieldCount);
				void* iterator = nullptr;
				while (MonoClassField* field = mono_class_get_fields(mono_class, &iterator))
				{
					const char* fieldName = mono_field_get_name(field);
					uint32_t flags = mono_field_get_flags(field);
					if (flags & FIELD_ATTRIBUTE_PUBLIC)
					{
						MonoType* type = mono_field_get_type(field);
						ScriptFieldType fieldType = ScriptingUtils::MonoTypeToScriptFieldType(type);

						script_class->m_Fields[fieldName] = { fieldType, fieldName, field };

						char data[16]{};
						mono_field_get_value(temporary_instance, field, &data);
						script_class->m_Fields[fieldName].SetInitialValue(data);

						L_CORE_INFO(" - {} ({})", fieldName, ScriptingUtils::ScriptFieldTypeToString(fieldType));

					}
				}
			}

			temporary_instance = nullptr;
			mono_gc_collect(mono_gc_max_generation());
		}
		
	}

	bool ScriptManager::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		s_Data->AppDomain = mono_domain_create_appdomain((char*)"LouronScriptRuntime_APP_DOMAIN", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		// Load CoreAssembly
		s_Data->CoreAssemblyFilepath = filepath;
		s_Data->CoreAssembly = ScriptingUtils::LoadCSharpAssembly(filepath.string(), s_Data->EnableDebugging);
		ScriptingUtils::PrintAssemblyTypes(s_Data->CoreAssembly);

		// Check if Loaded Correctly
		if (s_Data->CoreAssembly == nullptr)
			return false;

		// Set CoreAsssemblyImage in ScriptManagerData
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);

		return true;
	}

	bool ScriptManager::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		// Load CoreAssembly
		s_Data->AppAssemblyFilepath = filepath;
		s_Data->AppAssembly = ScriptingUtils::LoadCSharpAssembly(filepath.string(), s_Data->EnableDebugging);
		ScriptingUtils::PrintAssemblyTypes(s_Data->AppAssembly);

		// Check if Loaded Correctly
		if (s_Data->AppAssembly == nullptr)
			return false;

		// Set CoreAsssemblyImage in ScriptManagerData
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);

		return true;
	}

	void ScriptManager::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);

		LoadAssembly(s_Data->CoreAssemblyFilepath);
		LoadAppAssembly(s_Data->AppAssemblyFilepath);
		LoadAssemblyClasses(s_Data->CoreAssembly);

		ScriptConnector::RegisterComponents();

		// Retrieve and instantiate class
		s_Data->EntityClass = ScriptClass("Louron", "Entity", true);
		s_Data->PrefabClass = ScriptClass("Louron", "Prefab", true);
		s_Data->ComputeShaderClass = ScriptClass("Louron", "ComputeShader", true);
	}

	void ScriptManager::OnRuntimeStart(std::shared_ptr<Scene> scene)
	{
		s_Data->SceneContext = scene;

		auto view = scene->GetAllEntitiesWith<ScriptComponent>();
		for (auto entity_handle : view) {

			Entity entity = { entity_handle, scene.get() };

			if (!entity.HasComponent<ScriptComponent>())
				continue;

			auto& component = entity.GetComponent<ScriptComponent>();

			for (auto& [script_name, active] : component.Scripts) {
				if (!active)
					s_Data->InactiveEntityScripts[entity.GetUUID()].push_back(script_name);
			}
		}

	}

	void ScriptManager::OnRuntimeStop()
	{
		s_Data->SceneContext.reset();
		s_Data->EntityInstances.clear();
		s_Data->InactiveEntityScripts.clear();
	}

	Scene* ScriptManager::GetSceneContext()
	{
		return s_Data->SceneContext.lock().get();
	}

	bool ScriptManager::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}

	ScriptFieldMap& ScriptManager::GetScriptFieldMap(UUID entity_uuid, const std::string& script_name)
	{
		std::string script_map_key = std::to_string(entity_uuid) + script_name;
		return s_Data->EntityScriptFields[script_map_key];
	}

	void ScriptManager::OnCreateEntity(Entity entity) {

		const auto& sc = entity.GetComponent<ScriptComponent>();

		if (sc.Scripts.empty())
			return;

		for (auto& [script_name, active] : sc.Scripts) {

			if (!active)
				continue;

			if (ScriptManager::EntityClassExists(script_name)) {

				UUID entityID = entity.GetUUID();
				std::string script_map_key = std::to_string(entityID) + script_name;

				std::shared_ptr<ScriptInstance> instance = std::make_shared<ScriptInstance>(s_Data->EntityClasses[script_name], entity);
				s_Data->EntityInstances[script_map_key] = instance;

				// Copy field values
				if (s_Data->EntityScriptFields.find(script_map_key) != s_Data->EntityScriptFields.end())
				{
					const ScriptFieldMap& fieldMap = s_Data->EntityScriptFields.at(script_map_key);
					
					for (const auto& [name, fieldInstance] : fieldMap) {
						
						const char* field_type_string = ScriptingUtils::ScriptFieldTypeToString(fieldInstance.Field.Type);
						if (std::strstr(field_type_string, "Component")) { // Set Internal Property Values for Components
							instance->SetFieldComponentPropertyValue(fieldInstance, *(UUID*)fieldInstance.m_Buffer);
						}
						else if (fieldInstance.Field.Type == ScriptFieldType::Entity) { // Set Entity value
							instance->SetFieldEntityValue(fieldInstance, *(UUID*)fieldInstance.m_Buffer);
						}
						else if (fieldInstance.Field.Type == ScriptFieldType::Prefab) { // Set Prefab value
							instance->SetFieldPrefabValue(fieldInstance, *(AssetHandle*)fieldInstance.m_Buffer);
						}
						else if (fieldInstance.Field.Type == ScriptFieldType::ComputeShader) { // Set Prefab value
							instance->SetFieldComputeShaderValue(fieldInstance, *(AssetHandle*)fieldInstance.m_Buffer);
						}
						else { // Set values for other fields that are not components
							instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
						}
					}
				}

				instance->InvokeOnCreate();
			}
		}
	}

	void ScriptManager::OnCreateEntityScript(Entity entity, const std::string& script_name) {

		const auto& sc = entity.GetComponent<ScriptComponent>();

		if (sc.Scripts.empty())
			return;

		for (auto& [name, active] : sc.Scripts) {

			if (!active || name != script_name)
				continue;

			if (ScriptManager::EntityClassExists(script_name))
			{
				UUID entityID = entity.GetUUID();
				std::string script_map_key = std::to_string(entityID) + script_name;

				std::shared_ptr<ScriptInstance> instance = std::make_shared<ScriptInstance>(s_Data->EntityClasses[script_name], entity);
				s_Data->EntityInstances[script_map_key] = instance;

				// Copy field values
				bool found_field_value = s_Data->EntityScriptFields.find(script_map_key) != s_Data->EntityScriptFields.end();
				if (found_field_value)
				{
					const ScriptFieldMap& fieldMap = s_Data->EntityScriptFields.at(script_map_key);
					for (const auto& [name, fieldInstance] : fieldMap)
					{
						const char* field_type_string = ScriptingUtils::ScriptFieldTypeToString(fieldInstance.Field.Type);
						if (std::strstr(field_type_string, "Component")) { // Set Internal Property Values for Components
							instance->SetFieldComponentPropertyValue(fieldInstance, *(UUID*)fieldInstance.m_Buffer);
						}
						else if (fieldInstance.Field.Type == ScriptFieldType::Entity) { // Set Entity value
							instance->SetFieldEntityValue(fieldInstance, *(UUID*)fieldInstance.m_Buffer);
						}
						else if (fieldInstance.Field.Type == ScriptFieldType::Prefab) { // Set Prefab value
							instance->SetFieldPrefabValue(fieldInstance, *(AssetHandle*)fieldInstance.m_Buffer);
						}
						else if (fieldInstance.Field.Type == ScriptFieldType::ComputeShader) { // Set Prefab value
							instance->SetFieldComputeShaderValue(fieldInstance, *(AssetHandle*)fieldInstance.m_Buffer);
						}
						else { // Set values for other fields that are not components
							instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
						}
					}
				}

				instance->InvokeOnCreate();
			}
		}
	}

	void ScriptManager::OnDestroyEntity(Entity entity) {

		const auto& sc = entity.GetComponent<ScriptComponent>();

		if (sc.Scripts.empty())
			return;

		for (auto& [script_name, active] : sc.Scripts) {

			if (!active)
				continue;

			if (ScriptManager::EntityClassExists(script_name)) {

				UUID entityID = entity.GetUUID();
				std::string script_map_key = std::to_string(entityID) + script_name;

				auto it = s_Data->EntityInstances.find(script_map_key);
				if (it != s_Data->EntityInstances.end())
				{
					// Call InvokeOnDestroy on the instance before erasing it.
					if (it->second)
						it->second->InvokeOnDestroy();
					
					// Erase the element using the key.
					s_Data->EntityInstances.erase(it);
				}
			}
		}
	}

	void ScriptManager::OnUpdateEntity(Entity entity) {

		auto& component = entity.GetComponent<ScriptComponent>();

		for (int i = 0; i < component.Scripts.size(); i++) {

			if (!component.Scripts.at(i).second) // If Script Not Active!
				continue;

			if (component.Scripts.at(i).first == "") {
				component.Scripts.at(i).second = false;
				continue;
			}

			std::string script_instance_name = std::to_string(entity.GetUUID()) + component.Scripts.at(i).first;

			if (s_Data->EntityInstances.find(script_instance_name) != s_Data->EntityInstances.end())
			{
				std::shared_ptr<ScriptInstance> instance = s_Data->EntityInstances[script_instance_name];
				instance->InvokeOnUpdate();
			}
			else
			{
				L_CORE_ERROR("Could not find ScriptInstance for entity {}::{}", entity.GetUUID(), component.Scripts.at(i).first);
			}
		}
	}

	void ScriptManager::OnFixedUpdateEntity(Entity entity) {

		auto& component = entity.GetComponent<ScriptComponent>();

		for (int i = 0; i < component.Scripts.size(); i++) {

			if (!component.Scripts.at(i).second) // If Script Not Active!
				continue;

			std::string script_instance_name = std::to_string(entity.GetUUID()) + component.Scripts.at(i).first;

			if (s_Data->EntityInstances.find(script_instance_name) != s_Data->EntityInstances.end())
			{
				std::shared_ptr<ScriptInstance> instance = s_Data->EntityInstances[script_instance_name];
				instance->InvokeOnFixedUpdate();
			}
			else
			{
				L_CORE_ERROR("Could not find ScriptInstance for entity {}::{}", entity.GetUUID(), component.Scripts.at(i).first);
			}
		}
	}

	void ScriptManager::OnCollideEntity(Entity entity, Entity other_entity, _Collision_Type collision_type)
	{
		auto& component = entity.GetComponent<ScriptComponent>();

		for (int i = 0; i < component.Scripts.size(); i++) {

			if (!component.Scripts.at(i).second) // If Script Not Active!
				continue;

			std::string script_instance_name = std::to_string(entity.GetUUID()) + component.Scripts.at(i).first;

			if (s_Data->EntityInstances.find(script_instance_name) != s_Data->EntityInstances.end()) {

				std::shared_ptr<ScriptInstance> instance = s_Data->EntityInstances[script_instance_name];

				_Collider other_collider{};
				other_collider.other_collider_uuid = other_entity.GetUUID();

				if (other_entity.HasComponent<BoxColliderComponent>())
					other_collider.type = _Collider_Type::Box_Collider;

				if (other_entity.HasComponent<SphereColliderComponent>())
					other_collider.type = _Collider_Type::Sphere_Collider;

				// TODO
				//if (other_entity.HasComponent<CapsuleCollider>())
				//	other_collider.type = _Collider_Type::Capsule_Collider;
				//if (other_entity.HasComponent<MeshCollider>())
				//	other_collider.type = _Collider_Type::Mesh_Collider;

				void* param_other_collider = &other_collider;

				switch (collision_type) {

					// COLLIDE
					case _Collision_Type::CollideEnter: {
						instance->InvokeOnCollideEnter(&param_other_collider);
						break;
					}

					case _Collision_Type::CollideStay: {
						instance->InvokeOnCollideStay(&param_other_collider);
						break;
					}

					case _Collision_Type::CollideLeave: {
						instance->InvokeOnCollideLeave(&param_other_collider);
						break;
					}

					// TRIGGER
					case _Collision_Type::TriggerEnter: {
						instance->InvokeOnTriggerEnter(&param_other_collider);
						break;
					}

					case _Collision_Type::TriggerStay: {
						instance->InvokeOnTriggerStay(&param_other_collider);
						break;
					}

					case _Collision_Type::TriggerLeave: {
						instance->InvokeOnTriggerLeave(&param_other_collider);
						break;
					}

					case _Collision_Type::None:
					default: L_CORE_WARN("Invalid Collision Type!");
				}
			}
			else
			{
				L_CORE_WARN("Could not find ScriptInstance for entity {}::{}", entity.GetUUID(), component.Scripts.at(i).first);
			}
		}
	}

	void ScriptManager::CheckInactiveScriptsOnEntities()
	{
		auto scene_ref = s_Data->SceneContext.lock();

		if (!scene_ref)
			return;

		auto view = scene_ref->GetAllEntitiesWith<ScriptComponent>();
		for (auto& entity_handle : view) {
			Entity entity = { entity_handle, scene_ref.get() };
			ScriptComponent& component = entity.GetComponent<ScriptComponent>();

			for (auto& [script_name, active] : component.Scripts) {
				if(script_name != "")
					if (
							s_Data->EntityInstances.find(std::to_string(entity.GetUUID()) + script_name) == s_Data->EntityInstances.end() && // Is there an instantiated instance on this object already?
							std::find(s_Data->InactiveEntityScripts[entity.GetUUID()].begin(), s_Data->InactiveEntityScripts[entity.GetUUID()].end(), script_name) == s_Data->InactiveEntityScripts[entity.GetUUID()].end() // Has this already been added to the InactiveEntityScripts?
						)
						s_Data->InactiveEntityScripts[entity.GetUUID()].push_back(script_name);
			}
		}

		// Per Entity
		for (auto it = s_Data->InactiveEntityScripts.begin(); it != s_Data->InactiveEntityScripts.end(); ) {

			Entity entity = scene_ref->FindEntityByUUID(it->first);

			if (!scene_ref->HasEntity(entity))
			{
				it = s_Data->InactiveEntityScripts.erase(it);
				continue;
			}

			// Check if Scene has Entity first
			if (!scene_ref->HasEntity(entity)) {
				it = s_Data->InactiveEntityScripts.erase(it);
				continue;
			}

			// Check if entity still has ScriptComponent
			if (!entity.HasComponent<ScriptComponent>()) {
				it = s_Data->InactiveEntityScripts.erase(it);
				continue;
			}

			ScriptComponent& component = entity.GetComponent<ScriptComponent>();

			// Loop through inactive scripts stored in script manager
			// String Vector Per Entity
			for (int i = 0; i < it->second.size(); ) {

				bool still_has_script = false;
				int script_index = -1;
				// Loop through components available scripts
				for (int i = 0; i < component.Scripts.size(); i++) {
					if (component.Scripts.at(i).first == it->second[i]) {
						still_has_script = true;
						script_index = i;
						break;
					}
				}

				// Remove entry in scriptmanager vector if script not found
				if (!still_has_script) {
					it->second.erase(std::remove(it->second.begin(), it->second.end(), it->second[i]));
					continue;
				}

				if (component.Scripts.at(script_index).second) {
					// SCRIPT HAS BECOME ACTIVE

					OnCreateEntityScript(entity, component.Scripts.at(script_index).first);

					it->second.erase(std::remove(it->second.begin(), it->second.end(), it->second[i]));
					continue;
				}

				i++;
			}

			++it;			
		}
	}

	std::shared_ptr<ScriptClass> ScriptManager::GetEntityClass(const std::string& name) {

		if (!s_Data) return nullptr;

		auto result = s_Data->EntityClasses.find(name);
		return result != s_Data->EntityClasses.end() ? result->second : nullptr;
	}

	const std::unordered_map<std::string, std::shared_ptr<ScriptClass>>& ScriptManager::GetEntityClasses() {

		L_CORE_ASSERT(s_Data, "Script Data Not Initialised.");

		return s_Data->EntityClasses;
	}

	void ScriptManager::AddEntityClass(const std::string& entity_class_full_name, std::shared_ptr<ScriptClass> script_class)
	{
		if (!s_Data) return;

		s_Data->EntityClasses[entity_class_full_name] = script_class;
	}

	MonoImage* ScriptManager::GetCoreAssemblyImage()
	{
		if (!s_Data) return nullptr;

		return s_Data->CoreAssemblyImage;
	}

	std::shared_ptr<ScriptInstance> ScriptManager::GetEntityScriptInstance(UUID entityID, const std::string& script_name)
	{
		if (!s_Data) return nullptr;

		std::string script_map_key = std::to_string(entityID) + script_name;
		auto it = s_Data->EntityInstances.find(script_map_key);
		if (it == s_Data->EntityInstances.end())
			return nullptr;

		return it->second;
	}

	void ScriptManager::SetAppAssemblyPath(const std::filesystem::path& file_path)
	{
		if (!s_Data) return;

		s_Data->AppAssemblyFilepath = file_path;
	}

#pragma endregion

#pragma region Script Instance

	ScriptInstance::ScriptInstance(std::shared_ptr<ScriptClass> class_ref, Entity entity)
		: m_ScriptClass(class_ref), m_EntityUUID(entity.GetUUID())
	{
		m_Instance = class_ref->Instantiate();

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);

		m_OnCreateMethod = class_ref->GetMethod("OnStart", 0);
		m_OnUpdateMethod = class_ref->GetMethod("OnUpdate", 0);
		m_OnFixedUpdateMethod = class_ref->GetMethod("OnFixedUpdate", 0);
		m_OnDestroyMethod = class_ref->GetMethod("OnDestroy", 0);

		m_OnCollideEnterMethod = class_ref->GetMethod("OnCollideEnter", 1);
		m_OnCollideStayMethod = class_ref->GetMethod("OnCollideStay", 1);
		m_OnCollideLeaveMethod = class_ref->GetMethod("OnCollideLeave", 1);

		m_OnTriggerEnterMethod = class_ref->GetMethod("OnTriggerEnter", 1);
		m_OnTriggerStayMethod = class_ref->GetMethod("OnTriggerStay", 1);
		m_OnTriggerLeaveMethod = class_ref->GetMethod("OnTriggerLeave", 1);

		// Call Entity constructor
		{
			void* param = &m_EntityUUID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::SetFieldEntityValue(const ScriptFieldInstance& field_instance, UUID value)
	{
		ScriptClass& entity_class = s_Data->EntityClass;

		MonoClassField* entity_field_in_script = mono_class_get_field_from_name(m_ScriptClass->GetMonoClass(), field_instance.Field.Name.c_str());

		if (!entity_field_in_script)
			return;

		MonoObject* entity_instance = mono_object_new(s_Data->AppDomain, entity_class.GetMonoClass()); // CREATE - create a new object

		if (!entity_instance)
			return;

		MonoMethod* entity_constructor = entity_class.GetMethod(".ctor", 1);

		if (!entity_constructor)
			return;

		void* param = &value;
		entity_class.InvokeMethod(entity_instance, entity_constructor, &param); // INITIALISE - Call the parametered constructor to pass the UUID

		mono_field_set_value(m_Instance, entity_field_in_script, (void*)entity_instance); // ASSIGN - set the object to the field in our Script Instance
	}

	UUID ScriptInstance::GetFieldEntityValue(const std::string& field_name)
	{
		// Get the MonoClassField corresponding to the field_name
		MonoClassField* entity_field_in_script = mono_class_get_field_from_name(m_ScriptClass->GetMonoClass(), field_name.c_str());

		if (!entity_field_in_script)
			return NULL_UUID; // Return a default UUID if the field is not found

		// Retrieve the MonoObject stored in the field
		MonoObject* entity_instance = nullptr;
		mono_field_get_value(m_Instance, entity_field_in_script, &entity_instance);

		if (!entity_instance)
			return NULL_UUID; // Return a default UUID if the field has no value

		// Get the Entity::ID field from the Entity class
		ScriptClass& entity_class = s_Data->EntityClass;
		MonoClassField* id_field_in_entity = mono_class_get_field_from_name(entity_class.GetMonoClass(), "ID");

		if (!id_field_in_entity)
			return NULL_UUID; // Return a default UUID if the ID field is not found

		// Retrieve the value of the ID field
		mono_field_get_value(entity_instance, id_field_in_entity, s_FieldValueBuffer);

		return *(UUID*)s_FieldValueBuffer;
	}

	void ScriptInstance::SetFieldPrefabValue(const ScriptFieldInstance& field_instance, AssetHandle value)
	{
		ScriptClass& prefab_class = s_Data->PrefabClass;

		MonoClassField* prefab_field_in_script = mono_class_get_field_from_name(m_ScriptClass->GetMonoClass(), field_instance.Field.Name.c_str());

		if (!prefab_field_in_script)
			return;

		MonoObject* prefab_instance = mono_object_new(s_Data->AppDomain, prefab_class.GetMonoClass()); // CREATE - create a new object

		if (!prefab_instance)
			return;

		MonoMethod* prefab_constructor = prefab_class.GetMethod(".ctor", 1);

		if (!prefab_constructor)
			return;

		void* param = &value;
		prefab_class.InvokeMethod(prefab_instance, prefab_constructor, &param); // INITIALISE - Call the parametered constructor to pass the UUID

		mono_field_set_value(m_Instance, prefab_field_in_script, (void*)prefab_instance); // ASSIGN - set the object to the field in our Script Instance
	}

	AssetHandle ScriptInstance::GetFieldPrefabValue(const std::string& field_name)
	{
		// Get the MonoClassField corresponding to the field_name
		MonoClassField* prefab_field_in_script = mono_class_get_field_from_name(m_ScriptClass->GetMonoClass(), field_name.c_str());

		if (!prefab_field_in_script)
			return NULL_UUID; // Return a default UUID if the field is not found

		// Retrieve the MonoObject stored in the field
		MonoObject* prefab_instance = nullptr;
		mono_field_get_value(m_Instance, prefab_field_in_script, &prefab_instance);

		if (!prefab_instance)
			return NULL_UUID; // Return a default UUID if the field has no value

		// Get the Entity::ID field from the Entity class
		ScriptClass& entity_class = s_Data->PrefabClass;
		MonoClassField* handle_field_in_prefab_class = mono_class_get_field_from_name(entity_class.GetMonoClass(), "Asset_Handle");

		if (!handle_field_in_prefab_class)
			return NULL_UUID; // Return a default UUID if the ID field is not found

		// Retrieve the value of the AssetHandle field
		mono_field_get_value(prefab_instance, handle_field_in_prefab_class, s_FieldValueBuffer);

		return *(AssetHandle*)s_FieldValueBuffer;
	}

	void ScriptInstance::SetFieldComputeShaderValue(const ScriptFieldInstance& field_instance, AssetHandle value)
	{
		ScriptClass& compute_class = s_Data->ComputeShaderClass;

		MonoClassField* compute_field_in_script = mono_class_get_field_from_name(m_ScriptClass->GetMonoClass(), field_instance.Field.Name.c_str());

		if (!compute_field_in_script)
			return;

		MonoObject* compute_instance = mono_object_new(s_Data->AppDomain, compute_class.GetMonoClass()); // CREATE - create a new object

		if (!compute_instance)
			return;

		MonoMethod* compute_constructor = compute_class.GetMethod(".ctor", 1);

		if (!compute_constructor)
			return;

		void* param = &value;
		compute_class.InvokeMethod(compute_instance, compute_constructor, &param); // INITIALISE - Call the parametered constructor to pass the Asset Handle

		mono_field_set_value(m_Instance, compute_field_in_script, (void*)compute_instance); // ASSIGN - set the object to the field in our Script Instance
	}

	AssetHandle ScriptInstance::GetFieldComputeShaderValue(const std::string& field_name)
	{
		// Get the MonoClassField corresponding to the field_name
		MonoClassField* compute_field_in_script = mono_class_get_field_from_name(m_ScriptClass->GetMonoClass(), field_name.c_str());

		if (!compute_field_in_script)
			return NULL_UUID; // Return a default UUID if the field is not found

		// Retrieve the MonoObject stored in the field
		MonoObject* compute_instance = nullptr;
		mono_field_get_value(m_Instance, compute_field_in_script, &compute_instance);

		if (!compute_instance)
			return NULL_UUID; // Return a default UUID if the field has no value

		// Get the ComputeShader::Asset_Handle field from the ComputeShader class
		ScriptClass& compute_class = s_Data->ComputeShaderClass;
		MonoClassField* handle_field_in_compute_class = mono_class_get_field_from_name(compute_class.GetMonoClass(), "Asset_Handle");

		if (!handle_field_in_compute_class)
			return NULL_UUID; // Return a default UUID if the handle field is not found

		// Retrieve the value of the AssetHandle field
		mono_field_get_value(compute_instance, handle_field_in_compute_class, s_FieldValueBuffer);

		return *(AssetHandle*)s_FieldValueBuffer;
	}

	void ScriptInstance::SetFieldComponentPropertyValue(const ScriptFieldInstance& field_instance, UUID value)
	{
		// Retrieve the MonoClass of the field in our ScriptInstance.
		MonoClass* component_class = mono_class_from_name(s_Data->CoreAssemblyImage, "Louron", ScriptingUtils::ScriptFieldTypeToString(field_instance.Field.Type));

		// Retrieve the field of the Script Class we are currently in E.g., "PointLightComponent light;"
		MonoClassField* component_field_in_script = mono_class_get_field_from_name(m_ScriptClass->GetMonoClass(), field_instance.Field.Name.c_str());

		// Check if our Script Instance's field of MonoClass type is valid.
		if (!component_field_in_script)
			return;

		// CREATE New Instance of this class. Equivalent code in C# may just be PointLightComponent light = SOME VALUE;
		MonoObject* component_instance = mono_object_new(s_Data->AppDomain, component_class); // Create new Object
		mono_runtime_object_init(component_instance); // Initialise the new Object
		mono_field_set_value(m_Instance, component_field_in_script, (void*)component_instance); // Assign the field in our Script Instance to this new Object

		// Now we need to call the Component.SetEntity method which takes the UUID of the Entity that this component is attached to.
		component_class = mono_object_get_class(component_instance);
		MonoMethod* method = mono_class_get_method_from_name(component_class, "SetEntity", 1);
		if (method) {
			void* args[1] = { &value };
			mono_runtime_invoke(method, component_instance, args, nullptr);
		}

	}

	UUID ScriptInstance::GetFieldComponentPropertyValue(const std::string& name)
	{
		// Retrieve the MonoClassField for the property on the current script class
		MonoClassField* componentField = mono_class_get_field_from_name(m_ScriptClass->GetMonoClass(), name.c_str());
		if (!componentField)
		{
			// Field not found, return a default UUID or handle error
			return NULL_UUID;
		}

		// Retrieve the value of the field (component instance) from the script instance
		MonoObject* componentInstance = nullptr;
		mono_field_get_value(m_Instance, componentField, &componentInstance);

		if (!componentInstance)
		{
			// If the component instance is null, return a default UUID
			return NULL_UUID;
		}

		// Retrieve the property "Entity" from the component class
		MonoClass* componentClass = mono_object_get_class(componentInstance);
		MonoProperty* entityProperty = mono_class_get_property_from_name(componentClass, "Entity");
		if (!entityProperty)
		{
			// Property "Entity" not found, return a default UUID
			return NULL_UUID;
		}

		// Get the value of the "Entity" property
		MonoObject* entityInstance = mono_property_get_value(entityProperty, componentInstance, nullptr, nullptr);
		if (!entityInstance)
		{
			// If the Entity object is null, return a default UUID
			return NULL_UUID;
		}

		// Use s_Data->EntityClass to retrieve the "m_UUID" field
		MonoClassField* uuidField = mono_class_get_field_from_name(s_Data->EntityClass.GetMonoClass(), "ID");
		if (!uuidField)
		{
			// UUID field not found in the Entity class, return a default UUID
			return NULL_UUID;
		}

		// Retrieve the UUID value from the Entity instance
		uint32_t uuidValue = 0;
		mono_field_get_value(entityInstance, uuidField, &uuidValue);

		return UUID(uuidValue); // Assuming UUID can be constructed from a uint32_t
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (!m_OnCreateMethod)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod, nullptr, m_EntityUUID);
	}

	void ScriptInstance::InvokeOnUpdate()
	{
		if (!m_OnUpdateMethod)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, nullptr, m_EntityUUID);
	}

	void ScriptInstance::InvokeOnFixedUpdate()
	{
		if (!m_OnFixedUpdateMethod)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnFixedUpdateMethod, nullptr, m_EntityUUID);
	}

	void ScriptInstance::InvokeOnDestroy()
	{
		if (!m_OnDestroyMethod)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnDestroyMethod, nullptr, m_EntityUUID);
	}

	void ScriptInstance::InvokeOnCollideEnter(void** other_collider_param)
	{
		if (!m_OnCollideEnterMethod, nullptr, m_EntityUUID)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnCollideEnterMethod, other_collider_param, m_EntityUUID);
	}

	void ScriptInstance::InvokeOnCollideStay(void** other_collider_param)
	{
		if (!m_OnCollideStayMethod, nullptr, m_EntityUUID)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnCollideStayMethod, other_collider_param, m_EntityUUID);
	}

	void ScriptInstance::InvokeOnCollideLeave(void** other_collider_param)
	{
		if (!m_OnCollideLeaveMethod)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnCollideLeaveMethod, other_collider_param, m_EntityUUID);
	}

	void ScriptInstance::InvokeOnTriggerEnter(void** other_collider_param)
	{
		if (!m_OnTriggerEnterMethod)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnTriggerEnterMethod, other_collider_param, m_EntityUUID);
	}

	void ScriptInstance::InvokeOnTriggerStay(void** other_collider_param)
	{
		if (!m_OnTriggerStayMethod)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnTriggerStayMethod, other_collider_param, m_EntityUUID);
	}

	void ScriptInstance::InvokeOnTriggerLeave(void** other_collider_param)
	{
		if (!m_OnTriggerLeaveMethod)
			return;

		m_ScriptClass->InvokeMethod(m_Instance, m_OnTriggerLeaveMethod, other_collider_param, m_EntityUUID);
	}

#pragma endregion

	void ScriptField::SetInitialValue(const char* value) {
		if (!m_Buffer) {
			m_Buffer = new char[16](0);
		}
		// Ensures that we're only copying 16 bytes.
		memcpy(m_Buffer, value, 16);
	}

}

#include "Light.h"

#ifndef YAML_CPP_STATIC_DEFINE
#define YAML_CPP_STATIC_DEFINE
#endif
#include <yaml-cpp/yaml.h>

namespace Louron {


	void PointLightComponent::Serialize(YAML::Emitter& out) {

		out << YAML::Key << "PointLightComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "Active" << YAML::Value << Active;

		out << YAML::Key << "Colour" << YAML::Value << YAML::Flow
			<< YAML::BeginSeq
			<< Colour.r
			<< Colour.g
			<< Colour.b
			<< Colour.a
			<< YAML::EndSeq;

		out << YAML::Key << "Radius" << YAML::Value << Radius;
		out << YAML::Key << "Intensity" << YAML::Value << Intensity;

		out << YAML::EndMap;

	}

	bool PointLightComponent::Deserialize(const YAML::Node data)
	{
		if (!data)
			return false;

		if (data["Active"]) {
			Active = data["Active"].as<bool>();
		}

		if (data["Colour"]) {
			auto positionSeq = data["Colour"];
			if (positionSeq.IsSequence() && positionSeq.size() == 4) {
				Colour.r = positionSeq[0].as<float>();
				Colour.g = positionSeq[1].as<float>();
				Colour.b = positionSeq[2].as<float>();
				Colour.a = positionSeq[3].as<float>();
			}
		}

		if (data["Radius"]) {
			Radius = data["Radius"].as<GLfloat>();
		}

		if (data["Intensity"]) {
			Intensity = data["Intensity"].as<GLfloat>();
		}

		return true;
	}

	void SpotLightComponent::Serialize(YAML::Emitter& out)
	{
		out << YAML::Key << "SpotLightComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "Active" << YAML::Value << Active;

		out << YAML::Key << "Colour" << YAML::Value << YAML::Flow
			<< YAML::BeginSeq
			<< Colour.r
			<< Colour.g
			<< Colour.b
			<< Colour.a
			<< YAML::EndSeq;

		out << YAML::Key << "Range" << YAML::Value << Range;
		out << YAML::Key << "Angle" << YAML::Value << Angle;
		out << YAML::Key << "Intensity" << YAML::Value << Intensity;

		out << YAML::EndMap;
	}

	bool SpotLightComponent::Deserialize(const YAML::Node data) {
		if (!data)
			return false;

		if (data["Active"]) {
			Active = data["Active"].as<bool>();
		}

		if (data["Colour"]) {
			auto positionSeq = data["Colour"];
			if (positionSeq.IsSequence() && positionSeq.size() == 4) {
				Colour.r = positionSeq[0].as<float>();
				Colour.g = positionSeq[1].as<float>();
				Colour.b = positionSeq[2].as<float>();
				Colour.a = positionSeq[3].as<float>();
			}
		}

		if (data["Range"]) {
			Range = data["Range"].as<GLfloat>();
		}

		if (data["Angle"]) {
			Angle = data["Angle"].as<GLfloat>();
		}

		if (data["Intensity"]) {
			Intensity = data["Intensity"].as<GLfloat>();
		}

		return true;
	}

	void DirectionalLightComponent::Serialize(YAML::Emitter& out) {

		out << YAML::Key << "DirectionalLightComponent";
		out << YAML::BeginMap;

		out << YAML::Key << "Active" << YAML::Value << Active;

		out << YAML::Key << "Colour" << YAML::Value << YAML::Flow
			<< YAML::BeginSeq
			<< Colour.r
			<< Colour.g
			<< Colour.b
			<< Colour.a
			<< YAML::EndSeq;

		out << YAML::Key << "Intensity" << YAML::Value << Intensity;

		out << YAML::EndMap;
	}

	bool DirectionalLightComponent::Deserialize(const YAML::Node data) {
		if (!data)
			return false;

		if (data["Active"]) {
			Active = data["Active"].as<bool>();
		}

		if (data["Colour"]) {
			auto positionSeq = data["Colour"];
			if (positionSeq.IsSequence() && positionSeq.size() == 4) {
				Colour.r = positionSeq[0].as<float>();
				Colour.g = positionSeq[1].as<float>();
				Colour.b = positionSeq[2].as<float>();
				Colour.a = positionSeq[3].as<float>();
			}
		}

		if (data["Intensity"]) {
			Intensity = data["Intensity"].as<GLfloat>();
		}

		return true;
	}

}

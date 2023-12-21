#pragma once
#include <string>

namespace Louron {

	class Layer {
	public:
		Layer(const std::string& layerName = "Generic Layer") { }
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnGuiRender() {}
	};
}
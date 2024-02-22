#pragma once

// Louron Core Headers
#include "Layer.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace Louron {

	class GuiLayer : public Layer {

	public:
		GuiLayer();
		~GuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();

		uint32_t GetActiveWidgetID() const;
	private:
		bool m_BlockEvents = true;
	};
}
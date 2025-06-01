#include "Layout.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace ks {
	auto ks::Layout::dock_space() -> void {
		m_dockspaceID = ImGui::GetID("MyDockSpace");
		ImGui::DockSpaceOverViewport(m_dockspaceID, 0, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoTabBar);
		if (m_firstTime) {
			m_firstTime = 0;
			ImGui::DockBuilderRemoveNode(m_dockspaceID);
			ImGui::DockBuilderAddNode(m_dockspaceID, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(m_dockspaceID, ImVec2(1280.0f, 720.0f));

			m_sidePanelID = ImGui::DockBuilderSplitNode(m_dockspaceID, ImGuiDir_Right, 0.2f, nullptr, &m_dockspaceID);
			m_consolePanelID = ImGui::DockBuilderSplitNode(m_dockspaceID, ImGuiDir_Down, 0.25f, nullptr, &m_dockspaceID);
			ImGui::DockBuilderDockWindow("SidePanel", m_sidePanelID);
			ImGui::DockBuilderDockWindow("ConsolePanel", m_consolePanelID);

			ImGui::DockBuilderFinish(m_dockspaceID);
		}
	}

	auto Layout::viewport_begin() -> void {
		ImGui::SetNextWindowDockID(m_dockspaceID, ImGuiCond_Once);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Begin("Viewport", 0, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		m_viewportSize = ImGui::GetWindowSize();
		if (m_viewportSize.x != m_lastViewportSize.x || m_viewportSize.y != m_lastViewportSize.y) {
			m_lastViewportSize = m_viewportSize;
			m_viewportSizeChanged = 1;
		}
	}

	auto Layout::viewport_end() -> void {
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(3);
	}

	auto Layout::side_panel_begin() -> void {
		ImGui::SetNextWindowDockID(m_sidePanelID, ImGuiCond_Once);
		ImGui::Begin("Settings");
	}

	auto Layout::side_panel_end() -> void {
		ImGui::End();
	}

	auto Layout::console_begin() -> void {
		ImGui::SetNextWindowDockID(m_consolePanelID, ImGuiCond_Once);
		ImGui::Begin("Console");
	}

	auto Layout::console_end() -> void {
		ImGui::End();
	}

	auto Layout::has_viewport_size_changed() -> bool {
		if (m_viewportSizeChanged && m_lastViewportSizeChangedDelay.getElapsedTime().asMilliseconds() > 200) {
			m_lastViewportSizeChangedDelay.restart();
			m_viewportSizeChanged = 0;
			return 1;
		}
		return 0;
	}
}
#pragma once
#include "Tool.h"
#include "Utility.h"
#include "Events/Event.h"

struct EmulationImageTool : public Tool
{
public:
	EmulationImageTool();
	~EmulationImageTool() override;

	void Render() override;
	inline std::string Name() const override { return "Image"; }
	inline Type GetType() const override { return _GetType(); }
	static inline Type _GetType() { return EMU_IMG; }

	void OnOutput(bf_mem_t out);
	void EmulationWantsInput();
	void EmulationInput(bf_mem_t in);

	void SetMemory(const std::vector<bf_mem_t>* memory) { m_Memory = memory; }

	listener_id SubscribeEmulationInput(consumer<bf_mem_t> cb) { return m_InputEvent.Subscribe(cb); }
	listener_id SubscribeToggleRendering(consumer<bool> cb) { return m_ToggleRenderingEvent.Subscribe(cb); }

	bool IsRendering() const { return m_Rendering; }

private:
	const std::vector<bf_mem_t>* m_Memory = nullptr;
	std::array<RGBA, 256> m_Buffer;

	bool m_332 = false, m_Rendering = true, m_AdvanceFrame = false;
	bf_mem_t m_LastOutput;
	
	enum Smoothing
	{
		NONE, BOX, GAUSS,
		MAX
	};
	Smoothing m_Smoothing = NONE;
	static inline const std::vector<std::string> m_SmoothingLabels = { "None"s, "Box"s, "Gauss"s };

	Event<bf_mem_t> m_InputEvent;
	Event<bool> m_ToggleRenderingEvent;
};
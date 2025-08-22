#pragma once
#include "Tool.h"
#include "Utility.h"
#include "Events/Event.h"


class EmulationIOTool : public Tool
{
public:
	EmulationIOTool() = default;
	~EmulationIOTool() override = default;

	void Reset();

	void Render() override;

	inline std::string Name() const override { return "Emulation I/O"; }
	inline Type GetType() const override { return _GetType(); }
	static inline Type _GetType() { return EMU_IO; }

	void SetOutput(const std::string& out);
	void AllowInput(bool allow);

	listener_id SubscribeInput(consumer<bf_mem_t> cb) { return m_InputEvent.Subscribe(cb); }

	void OnEmulationOutput(bf_mem_t o);
	void OnEmulationInputRequested();
	void OnEmulationTerminated();
	void OnEmulationStarted();

private:
	bf_mem_t m_EmuInput = 0;
	std::string m_EmuOutput;
	std::mutex m_EmuMutex;
	bool m_EmuWantsInput = false;
	bool m_EmuFocusInput = false;
	bool m_InputAllowed = true;

	Event<bf_mem_t> m_InputEvent;
};
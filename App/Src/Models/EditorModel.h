#pragma once
#include "Utility.h"
#include "Events/EventBus.h"

struct Document
{
	using idt = u32;
	static constexpr idt InvalidID = 0;

	std::string Name;
	fs::path Path;
	idt Id;
	bool Dirty = false;
	std::string Content;
	u32 CursorPos = 0;

	Document(const fs::path& path);
	void operator=(const Document& other)
	{
		Name = other.Name;
		Path = other.Path;
		Dirty = other.Dirty;
		Id = other.Id;
		Content = other.Content;
	}

	static idt NextId;
};

std::ostream& operator<<(std::ostream& out, const Document& doc);


class EditorModel
{
public:
	using idt = Document::idt;
	//using bus_t = EventBus<std::monostate>;
	//using bus_payload_t = bus_t::payload_t;
	static constexpr i32 InvalidIndex = -1;
	static constexpr u32 RecentOpenSize = 10, RecentCloseSize = 10;

	enum Prop : prop_id
	{
		FOCUS, CLOSE
	};



public:
	EditorModel() { LOG_APP("EditorModel Created\n"); }
	~EditorModel() { LOG_APP("EditorModel Destroyed\n"); }

	void Lock(bool lock) { m_Locked = lock; }

	bool Close(std::vector<u32> inds, bool save);
	void OpenOrFocus(const fs::path& path);

	const std::vector<fs::path>& GetRecentOpen()  const { return m_RecOpen; }
	const std::vector<fs::path>& GetRecentClose() const { return m_RecClose; }
	std::vector<Document>& GetDocuments()         { return m_Documents; }

	void PerformSave(Document& doc) const;
	void PerformRename(const idt id, const std::string& name);

	bool ChangeFile(const idt id);

	void MoveCursor(Document* doc, const i32 pos);
	void Edited(Document* doc, const char change);

	const Document* GetFocusedFile() const { return m_FocusInd == InvalidIndex ? nullptr : &(m_Documents[m_FocusInd]); }

	void OnPathDeleted(const fs::path& path);

	//CREATE_BUS_FORWARDING_SUB(bus_t, Prop, m_EventBus)
	/*listener_id Subscribe(const Prop prop, consumer<bus_payload_t> callable) { return m_EventBus.Subscribe(prop, callable); }
	template <typename C> requires InVariant<C, bus_payload_t>
	listener_id Subscribe(const Prop prop, consumer<C> callable) { return m_EventBus.Subscribe<C>(prop, callable); }
	listener_id Subscribe(const Prop prop, callable callable) { return m_EventBus.Subscribe(prop, callable); }
	bool Unsubscribe(Prop prop, listener_id id) { m_EventBus.Unsubscribe(prop, id); }*/
	// u64  Unsubscribe(listener_id id) { m_EventBus.Unsubscribe(id); }

	listener_id SubscribeFocus(consumer<const Document&> cb) { return m_FocusEvent.Subscribe(cb); }
	listener_id SubscribeClose(callable cb) { return m_CloseEvent.Subscribe(cb); }



private:
	/*void Notify(const Prop prop) { m_EventBus.Notify(prop); }
	template <typename C> requires InVariant<C, bus_payload_t>
	void Notify(const Prop prop, const C& data) { m_EventBus.Notify(prop, data); }*/


private:
	prop_id m_NextId = 0;

	std::vector<Document> m_Documents;
	std::vector<fs::path> m_RecOpen, m_RecClose;

	bool m_Locked = false;
	i32 m_FocusInd = InvalidIndex;

	//bus_t m_EventBus;
	Event<const Document&> m_FocusEvent;
	Event<void> m_CloseEvent;
	// std::unordered_map<Prop, std::unordered_map<listener_id, callable>> m_Observers;
};
#pragma once
#include "Utility.h"


template <typename T>
class Event {
public:
	template <typename U>
	struct callback_type {
		using type = consumer<const U&>;
	};
	template <>
	struct callback_type<void> {
		using type = callable;
	};

	using callback = typename callback_type<T>::type;

private:
	std::unordered_map<listener_id, callback> m_Listeners;

	static listener_id NextID;

public:
	listener_id Subscribe(callback cb)
	{
		m_Listeners[NextID] = cb;
		return NextID++;
	}

	bool Unsubscribe(listener_id id)
	{
		return m_Listeners.erase(id) == 1;
	}


	template <typename U = T>
	void Notify(const U& data) requires not_void<U>
	{
		for (const auto& [_, callable] : m_Listeners)
			callable(data);
	}
	void Notify() requires std::is_void_v<T>
	{
		for (const auto& [_, callable] : m_Listeners)
			callable();
	}
};


template <typename T>
listener_id Event<T>::NextID = 0;